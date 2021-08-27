#define BOOST_TEST_MODULE DetectorTest
#include <boost/test/unit_test.hpp>


#ifndef DEBUG_MODE_OFDM
#define DEBUG_MODE_OFDM
#endif


// For IO
#include <stdlib.h>
#include <math.h>
#include <cmath> 
#include <iostream>
#include <unistd.h>
#include <vector>

// Plotting library 
#include <boost/tuple/tuple.hpp>
#include "gnuplot-iostream.h"
#include <utility>

// For measuring elapsed time
#include <chrono>

// For Random Float Generator
#include <time.h>

// For object under test
#include "detector.h"
#include "qam-modulator.h"
#include "common.h"
#include "fftw3.h"

/**
* Test NYQUIST MODULATOR
* 
*/
BOOST_AUTO_TEST_SUITE(DetectorTest)


/**
* Generates random prefixed symbol and 
* stores it for every possible position
* in the ring buffer. The correlator values
* are calculated for the whole ring buffer and 
* an evaluation of the peak element is carried out 
* 
*/
BOOST_AUTO_TEST_CASE(CorrelatorTest)
{
    printf("\nTesting Correlator...\n");
    // OFDM Codec Settings
    OFDMSettingsStruct encoderSettingsStruct;
    encoderSettingsStruct.type = FFTW_BACKWARD;
    encoderSettingsStruct.EnergyDispersalSeed = 10;
    encoderSettingsStruct.nFFTPoints = 1024; 
    encoderSettingsStruct.PilotToneDistance = 16; 
    encoderSettingsStruct.PilotToneAmplitude = 2.0; 
    encoderSettingsStruct.QAMSize = 2; 
    encoderSettingsStruct.PrefixSize = (size_t) ((encoderSettingsStruct.nFFTPoints*2)/4); // 1/4th of symbol
    encoderSettingsStruct.nDataBytesPerSymbol = 100;

    OFDMSettingsStruct decoderSettingsStruct = encoderSettingsStruct;
    decoderSettingsStruct.type = FFTW_FORWARD;

    OFDMSettings encoderSettings(encoderSettingsStruct);
    OFDMSettings decoderSettings(decoderSettingsStruct);

    // Initialize Encoder objects
    QamModulator qam(encoderSettings);
    FFT ifft(encoderSettings);
    NyquistModulator nyquistModulator(encoderSettings);
    
    // Initialize Decoder objects
    FFT fft(decoderSettings);
    NyquistModulator nyquistDemodulator(decoderSettings );
    Detector detector(decoderSettings, fft, nyquistDemodulator);

    // Setup random float generator
    srand( (unsigned)time( NULL ) );

    // Create rx signal array to capable of holding 20 upsampled symbols with prefix
    uint8_t *txBytes = (uint8_t*) calloc(decoderSettings.m_nDataBytesPerSymbol, sizeof(uint8_t));
    double *modulatorOutput = (double*) calloc(decoderSettings.m_PrefixedSymbolSize, sizeof(double));
    
    DoubleVec corOutput; // DEBUG ONLY
    //auto start = std::chrono::steady_clock::now();
    //auto end = std::chrono::steady_clock::now();

    size_t MaxCorrelationIndex = 0;
    double MaxCorrelationValue = 0.0;
    double correlation = 0.0;

    // For every possible location the symbol can be pasted in
    for(size_t symbolStart = 0; symbolStart < nyquistModulator.m_RingBufferBoundary; symbolStart++) //
    {   
        //std::cout << "Correlator @ symbolStart = " << symbolStart << std::endl;
        // Clear Rx Signal
        //memset(detector.m_BlockRingBuffer, 0, nyquistModulator.m_RingBufferBoundary*sizeof(double) );
        for(size_t i = 0; i < nyquistModulator.m_RingBufferBoundary; i++)
        {
            detector.m_BlockRingBuffer[i] = 0;
        }
        MaxCorrelationIndex = 0;
        MaxCorrelationValue = 0.0;
        correlation = 0.0;

        // Generate Random Bytes
        for(size_t i = 0; i < decoderSettings.m_nDataBytesPerSymbol; i++)
        {
            txBytes[i] = rand() % 255;
        }
   
        // Encode one symbol 
        qam.Modulate(txBytes, ifft.in);
        ifft.ComputeTransform( (fftw_complex *) &modulatorOutput[decoderSettings.m_PrefixSize]);
        nyquistModulator.Modulate(modulatorOutput);
        AddCyclicPrefix(modulatorOutput, decoderSettings.m_SymbolSize, decoderSettings.m_PrefixSize);
    
        // Check Prefix Has been added correctly
        for(size_t i = 0 ; i < decoderSettings.m_PrefixSize; i++)
        {
            if(modulatorOutput[i] != modulatorOutput[decoderSettings.m_SymbolSize + i])
            {
                printf("Missmatch symbolwithprefix[%lu]  = %f, modulatorOutput[%lu] = %f \n" , i , modulatorOutput[i], (decoderSettings.m_SymbolSize + i) , modulatorOutput[(decoderSettings.m_SymbolSize + i)]);
            }
        }

        // Copy the prefixed symbol somewhere in the ring buffer,
        // Check if the index suggests wrapping around the buffer is needed
        if(nyquistModulator.m_RingBufferBoundary - symbolStart > encoderSettings.m_PrefixedSymbolSize)
        {
            memcpy(&detector.m_BlockRingBuffer[symbolStart], &modulatorOutput[0], sizeof(double)*encoderSettings.m_PrefixedSymbolSize);
        }
        // wrap the object around
        else
        {
            // Calculate the number elements of the symbol untill boundary of the ring buffer
            size_t nToBoundary = nyquistModulator.m_RingBufferBoundary - symbolStart;
            // Copy samples
            memcpy(&detector.m_BlockRingBuffer[symbolStart], &modulatorOutput[0], nToBoundary*sizeof(double));
            // Calculate the number elements of the symbol over boundary of the ring buffer
            size_t nOverBoundary = encoderSettings.m_PrefixedSymbolSize - nToBoundary;
            // Copy samples
            memcpy(&detector.m_BlockRingBuffer[0], &modulatorOutput[nToBoundary], nOverBoundary*sizeof(double));
        }
        // Check if the symbol with prefix has been copied correctly

        // Clear correlator output
        corOutput.clear();
        // Calculate correlation for the whole ring buffer.
        for(size_t i = 0; i < nyquistModulator.m_RingBufferBoundary; i++)
        {
            // Calculate correlation for a given offset
            correlation = detector.ExecuteCorrelator();
            // Square correlation to make the result +ve and further spearate peaks and noise
            correlation *= correlation;
            // Append correlation to plot buffer
            corOutput.push_back(correlation); 
            // Increment offset
            detector.IncrementCorrelatorIndicies(1);
        }

        // Find Max in Correlation
        for(size_t i = 0; i < nyquistModulator.m_RingBufferBoundary; i++)
        {
            if(MaxCorrelationValue <= corOutput[i])
            {
                MaxCorrelationIndex = i;
                MaxCorrelationValue = corOutput[i];
            }
               
        }

        // Plot Corellator output
        //Gnuplot gp;
        //gp << "plot '-' with line title 'Detector - Correlation'\n";
        //gp.send1d(corOutput);

        // Check if the correlator max output is where the symbol was inserted
        BOOST_CHECK_MESSAGE( symbolStart == MaxCorrelationIndex, 
        "Max correlation does not correspond to prefixed symbol start, actual =  " << symbolStart << " Max = " << MaxCorrelationIndex  );  
    }
}



/**
* Generates random prefixed symbol and 
* stores it for every possible position
* in the ring buffer. Then a coarse search
* fucntion is used to find the coarse symbol
* start
*
* This test rquires a appropriate threshold to
* be set, play around with 
* 
*    m_UpperThreshold(35000000000)
*    m_LowerThreshold(10000000000)
*
* In the decoder settings if this fails
*
*/   
BOOST_AUTO_TEST_CASE(CoarseStartTest)
{
   // OFDM Codec Settings
    OFDMSettingsStruct encoderSettingsStruct;
    encoderSettingsStruct.type = FFTW_BACKWARD;
    encoderSettingsStruct.EnergyDispersalSeed = 10;
    encoderSettingsStruct.nFFTPoints = 1024; 
    encoderSettingsStruct.PilotToneDistance = 16; 
    encoderSettingsStruct.PilotToneAmplitude = 2.0; 
    encoderSettingsStruct.QAMSize = 2; 
    encoderSettingsStruct.PrefixSize = (size_t) ((encoderSettingsStruct.nFFTPoints*2)/4); // 1/4th of symbol
    encoderSettingsStruct.nDataBytesPerSymbol = 100;

    OFDMSettingsStruct decoderSettingsStruct = encoderSettingsStruct;
    decoderSettingsStruct.type = FFTW_FORWARD;

    OFDMSettings encoderSettings(encoderSettingsStruct);
    OFDMSettings decoderSettings(decoderSettingsStruct);

    // Initialize Encoder objects
    QamModulator qam(encoderSettings);
    FFT ifft(encoderSettings);
    NyquistModulator nyquistModulator(encoderSettings);
    
    // Initialize Decoder objects
    FFT fft(decoderSettings);
    NyquistModulator nyquistDemodulator(decoderSettings );
    Detector detector(decoderSettings, fft, nyquistDemodulator);

    // Setup random float generator
    srand( (unsigned)time( NULL ) );

    // Create rx signal array to capable of holding 20 upsampled symbols with prefix
    uint8_t *txBytes = (uint8_t*) calloc(decoderSettings.m_nDataBytesPerSymbol, sizeof(uint8_t));
    double *modulatorOutput = (double*) calloc(decoderSettings.m_PrefixedSymbolSize, sizeof(double));

    auto start = std::chrono::steady_clock::now();
    auto end = std::chrono::steady_clock::now();
    
    long int symbolStart = 0;
    // For every possible location the symbol can be pasted in
    for(size_t PrefixedsymbolStart = 0; PrefixedsymbolStart < nyquistModulator.m_RingBufferBoundary; PrefixedsymbolStart++) //
    {   
        // Compute the actual symbol start not the prefixed start
        // Start with prefixed symbol start
        symbolStart = PrefixedsymbolStart;
        // Increment the start by the size of the prefix
        detector.IncrementByN(symbolStart, decoderSettings.m_PrefixSize);
        //std::cout << "Coarse Start @ Prefixed Symbol Start  = " << PrefixedsymbolStart << std::endl;
        //std::cout << "Coarse Start @ Symbol Start  = " << symbolStart << std::endl;
        // Clear Rx Signal
        //memset(detector.m_BlockRingBuffer, 0, nyquistModulator.m_RingBufferBoundary*sizeof(double) );
        for(size_t i = 0; i < nyquistModulator.m_RingBufferBoundary; i++)
        {
            detector.m_BlockRingBuffer[i] = 0;
        }

        // Generate Random Bytes
        for(size_t i = 0; i < decoderSettings.m_nDataBytesPerSymbol; i++)
        {
            txBytes[i] = rand() % 255;
        }
   
        // Encode one symbol 
        qam.Modulate(txBytes, ifft.in);
        ifft.ComputeTransform( (fftw_complex *) &modulatorOutput[decoderSettings.m_PrefixSize]);
        nyquistModulator.Modulate(modulatorOutput);
        AddCyclicPrefix(modulatorOutput, decoderSettings.m_SymbolSize, decoderSettings.m_PrefixSize);

        // Copy the prefixed symbol somewhere in the ring buffer,
        // Check if the index suggests wrapping around the buffer is needed
        if( (nyquistModulator.m_RingBufferBoundary - PrefixedsymbolStart) > encoderSettings.m_PrefixedSymbolSize)
        {
            memcpy(&detector.m_BlockRingBuffer[PrefixedsymbolStart], &modulatorOutput[0], sizeof(double)*encoderSettings.m_PrefixedSymbolSize);
        }
        // wrap the object around
        else
        {
            // Calculate the number elements of the symbol untill boundary of the ring buffer
            size_t nToBoundary = nyquistModulator.m_RingBufferBoundary - PrefixedsymbolStart;
            // Copy samples
            memcpy(&detector.m_BlockRingBuffer[PrefixedsymbolStart], &modulatorOutput[0], nToBoundary*sizeof(double));
            // Calculate the number elements of the symbol over boundary of the ring buffer
            size_t nOverBoundary = encoderSettings.m_PrefixedSymbolSize - nToBoundary;
            // Copy samples
            memcpy(&detector.m_BlockRingBuffer[0], &modulatorOutput[nToBoundary], nOverBoundary*sizeof(double));
        }

        long int coarseStart = -1;
        while( coarseStart == -1)
        {
          start = std::chrono::steady_clock::now();
          coarseStart = detector.CoarseSearch();
          end = std::chrono::steady_clock::now();
        }

        
        //std::cout << "Coarse Search elapsed time: "
        //<< std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
        //<< " ns" << std::endl;  
        

        // Check if the correlator max output is where the symbol was inserted
        BOOST_CHECK_MESSAGE(  (long int)PrefixedsymbolStart == coarseStart, 
        "Coarse Search Not Successfull Prefixed symbolStart start, actual =  " << PrefixedsymbolStart << " found = " << coarseStart );   
    }
}


/**
* Generates random prefixed symbol and 
* stores it for every possible position
* in the ring buffer. Then a coarse search
* fucntion is used to find the coarse symbol
* start to which offset is added randomly and
* fine search is used to correct the offset
*
* This test rquires a appropriate threshold to
* be set, play around with 
* 
*    m_UpperThreshold(35000000000)
*    m_LowerThreshold(10000000000)
*
* In the decoder settings if this fails
*
*/   
BOOST_AUTO_TEST_CASE(FineSearch)
{
   // OFDM Codec Settings
    OFDMSettingsStruct encoderSettingsStruct;
    encoderSettingsStruct.type = FFTW_BACKWARD;
    encoderSettingsStruct.EnergyDispersalSeed = 10;
    encoderSettingsStruct.nFFTPoints = 1024; 
    encoderSettingsStruct.PilotToneDistance = 16; 
    encoderSettingsStruct.PilotToneAmplitude = 2.0; 
    encoderSettingsStruct.QAMSize = 2; 
    encoderSettingsStruct.PrefixSize = (size_t) ((encoderSettingsStruct.nFFTPoints*2)/4); // 1/4th of symbol
    encoderSettingsStruct.nDataBytesPerSymbol = 100;

    OFDMSettingsStruct decoderSettingsStruct = encoderSettingsStruct;
    decoderSettingsStruct.type = FFTW_FORWARD;

    OFDMSettings encoderSettings(encoderSettingsStruct);
    OFDMSettings decoderSettings(decoderSettingsStruct);

    // Initialize Encoder objects
    QamModulator qam(encoderSettings);
    FFT ifft(encoderSettings);
    NyquistModulator nyquistModulator(encoderSettings);
    
    // Initialize Decoder objects
    FFT fft(decoderSettings);
    NyquistModulator nyquistDemodulator(decoderSettings );
    Detector detector(decoderSettings, fft, nyquistDemodulator);

    // Setup random float generator
    srand( (unsigned)time( NULL ) );

    // Create rx signal array to capable of holding 20 upsampled symbols with prefix
    uint8_t *txBytes = (uint8_t*) calloc(decoderSettings.m_nDataBytesPerSymbol, sizeof(uint8_t));
    double *modulatorOutput = (double*) calloc(decoderSettings.m_PrefixedSymbolSize, sizeof(double));

    auto start = std::chrono::steady_clock::now();
    auto end = std::chrono::steady_clock::now();
    
    long int symbolStart = 0;
    // For every possible location the symbol can be pasted in
    for(size_t PrefixedsymbolStart = 0; PrefixedsymbolStart < nyquistModulator.m_RingBufferBoundary; PrefixedsymbolStart++)
    {   
        // Compute the actual symbol start not the prefixed start
        // Start with prefixed symbol start
        symbolStart = PrefixedsymbolStart;
        // Increment the start by the size of the prefix
        detector.IncrementByN(symbolStart, decoderSettings.m_PrefixSize);
        //std::cout << "Coarse Start @ Prefixed Symbol Start  = " << PrefixedsymbolStart << std::endl;
        //std::cout << "Coarse Start @ Symbol Start  = " << symbolStart << std::endl;
        // Clear Rx Signal
        //memset(detector.m_BlockRingBuffer, 0, nyquistModulator.m_RingBufferBoundary*sizeof(double) );
        for(size_t i = 0; i < nyquistModulator.m_RingBufferBoundary; i++)
        {
            detector.m_BlockRingBuffer[i] = 0;
        }

        // Generate Random Bytes
        for(size_t i = 0; i < decoderSettings.m_nDataBytesPerSymbol; i++)
        {
            txBytes[i] = rand() % 255;
        }
   
        // Encode one symbol 
        qam.Modulate(txBytes, ifft.in);
        ifft.ComputeTransform( (fftw_complex *) &modulatorOutput[decoderSettings.m_PrefixSize]);
        nyquistModulator.Modulate(modulatorOutput);
        AddCyclicPrefix(modulatorOutput, decoderSettings.m_SymbolSize, decoderSettings.m_PrefixSize);

        // Copy the prefixed symbol somewhere in the ring buffer,
        // Check if the index suggests wrapping around the buffer is needed
        if( (nyquistModulator.m_RingBufferBoundary - PrefixedsymbolStart) > encoderSettings.m_PrefixedSymbolSize)
        {
            memcpy(&detector.m_BlockRingBuffer[PrefixedsymbolStart], &modulatorOutput[0], sizeof(double)*encoderSettings.m_PrefixedSymbolSize);
        }
        // wrap the object around
        else
        {
            // Calculate the number elements of the symbol untill boundary of the ring buffer
            size_t nToBoundary = nyquistModulator.m_RingBufferBoundary - PrefixedsymbolStart;
            // Copy samples
            memcpy(&detector.m_BlockRingBuffer[PrefixedsymbolStart], &modulatorOutput[0], nToBoundary*sizeof(double));
            // Calculate the number elements of the symbol over boundary of the ring buffer
            size_t nOverBoundary = encoderSettings.m_PrefixedSymbolSize - nToBoundary;
            // Copy samples
            memcpy(&detector.m_BlockRingBuffer[0], &modulatorOutput[nToBoundary], nOverBoundary*sizeof(double));
        }

        long int coarseStart = -1;
        while( coarseStart == -1)
        {
          start = std::chrono::steady_clock::now();
          coarseStart = detector.CoarseSearch();
          end = std::chrono::steady_clock::now();
        }

        // Check if the correlator max output is where the symbol was inserted
        BOOST_CHECK_MESSAGE(  (long int)PrefixedsymbolStart == coarseStart, 
        "Coarse Search Not Successfull Prefixed symbolStart start, actual =  " << PrefixedsymbolStart << " found = " << coarseStart );   
        
        // Add some offset to the coarse start 

        // Increment coarse start by prefix
        detector.IncrementByN(coarseStart, encoderSettings.m_PrefixSize);
        

        size_t fineSearch = 0; 
        fineSearch = detector.FineSearch(coarseStart);
    
        // Check if the correlator max output is where the symbol was inserted
        BOOST_CHECK_MESSAGE(  (size_t)symbolStart == fineSearch, 
        "Fine Search Not Successfull Symbol Start actual =  " << symbolStart << " found = " << fineSearch );   
    }
}


/*
TODO:
BOOST_AUTO_TEST_CASE(SymbolStartTest)
{
    // OFDM Codec Settings
    OFDMSettingsStruct encoderSettingsStruct;
    encoderSettingsStruct.type = FFTW_BACKWARD;
    encoderSettingsStruct.EnergyDispersalSeed = 10;
    encoderSettingsStruct.nFFTPoints = 1024; 
    encoderSettingsStruct.PilotToneDistance = 16; 
    encoderSettingsStruct.PilotToneAmplitude = 2.0; 
    encoderSettingsStruct.QAMSize = 2; 
    encoderSettingsStruct.PrefixSize = (size_t) ((encoderSettingsStruct.nFFTPoints*2)/4); // 1/4th of symbol
    encoderSettingsStruct.nDataBytesPerSymbol = 100;

    OFDMSettingsStruct decoderSettingsStruct = encoderSettingsStruct;
    decoderSettingsStruct.type = FFTW_FORWARD;

    OFDMSettings encoderSettings(encoderSettingsStruct);
    OFDMSettings decoderSettings(decoderSettingsStruct);

    // Initialize Encoder objects
    QamModulator qam(encoderSettings);
    FFT ifft(encoderSettings);
    NyquistModulator nyquistModulator(encoderSettings);
    
    // Initialize Decoder objects
    FFT fft(decoderSettings);
    NyquistModulator nyquistDemodulator(decoderSettings );
    Detector detector(decoderSettings, fft, nyquistDemodulator);

    // Setup random float generator
    srand( (unsigned)time( NULL ) );

    // Create rx signal array to capable of holding 20 upsampled symbols with prefix
    uint8_t *txBytes = (uint8_t*) calloc(decoderSettings.m_nDataBytesPerSymbol, sizeof(uint8_t));
    double *modulatorOutput = (double*) calloc(decoderSettings.m_PrefixedSymbolSize, sizeof(double));
    
    size_t nBlocksRxSignal = 10;
    size_t nMaxAllowedBlock = nBlocksRxSignal -1;
    size_t  rxSignalSize = (decoderSettings.m_PrefixedSymbolSize * nBlocksRxSignal);
    double * rxSignal = (double*) calloc(rxSignalSize, sizeof(double));
    size_t  rxLastAllowedIndex = (decoderSettings.m_PrefixedSymbolSize * nMaxAllowedBlock);

    auto start = std::chrono::steady_clock::now();
    auto end = std::chrono::steady_clock::now();
    
    long int symbolStart = 0;
    // For every possible location the symbol can be pasted in
    for(size_t PrefixedsymbolStart = decoderSettings.m_PrefixedSymbolSize; PrefixedsymbolStart < rxLastAllowedIndex; PrefixedsymbolStart++)
    {   
        
        // Compute the actual symbol start not the prefixed start
        // Start with prefixed symbol start
        symbolStart = PrefixedsymbolStart;
        // Increment the start by the size of the prefix
        detector.IncrementByN(symbolStart, decoderSettings.m_PrefixSize);
        std::cout << "Symbol Start Test @ Prefixed Symbol Start  = " << PrefixedsymbolStart << std::endl;
        std::cout << "Symbol Start Test @ Symbol Start  = " << symbolStart << std::endl;
        // Clear Rx Signal
        //memset(rxSignal, 0, rxSignalSize*sizeof(double) );
        for(size_t i = 0; i < rxSignalSize; i++)
        {
            rxSignal[i] = 0;
        }

        // Generate Random Bytes
        for(size_t i = 0; i < decoderSettings.m_nDataBytesPerSymbol; i++)
        {
            txBytes[i] = rand() % 255;
        }
   
        // Encode one symbol 
        qam.Modulate(txBytes, ifft.in);
        ifft.ComputeTransform( (fftw_complex *) &modulatorOutput[decoderSettings.m_PrefixSize]);
        nyquistModulator.Modulate(modulatorOutput);
        AddCyclicPrefix(modulatorOutput, decoderSettings.m_SymbolSize, decoderSettings.m_PrefixSize);

        long int detectedSymbolStart = -1;
        size_t BufferCounter = 0;
        while( detectedSymbolStart == -1)
        {
            start = std::chrono::steady_clock::now();
            detectedSymbolStart = detector.FindSymbolStart(&rxSignal[decoderSettings.m_PrefixedSymbolSize*BufferCounter]);
            if( (BufferCounter+1) <= nMaxAllowedBlock )
            {
                BufferCounter++;
            }  
            else
            {
                BufferCounter = 0;
            }
            end = std::chrono::steady_clock::now();
        }
    
        // Check if the correlator max output is where the symbol was inserted
        BOOST_CHECK_MESSAGE( symbolStart == detectedSymbolStart, 
        "Coarse Search Not Successfull Prefixed symbolStart start, actual =  " << symbolStart << " found = " << detectedSymbolStart );   
    }
}
*/

BOOST_AUTO_TEST_SUITE_END()
