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
* 
*/
BOOST_AUTO_TEST_CASE(CorrelatorTest)
{
    printf("\nTesting Correlator...\n");

    // OFDM Codec Settings
    OFDMSettings encoderSettings;
    encoderSettings.type = FFTW_BACKWARD;
    encoderSettings.EnergyDispersalSeed = 10;
    encoderSettings.nFFTPoints = 1024; 
    encoderSettings.PilotToneDistance = 16; 
    encoderSettings.PilotToneAmplitude = 2.0; 
    encoderSettings.QAMSize = 2; 
    encoderSettings.PrefixSize = (size_t) ((encoderSettings.nFFTPoints*2)/4); // 1/4th of symbol

    OFDMSettings decoderSettings = encoderSettings;
    decoderSettings.type = FFTW_FORWARD;

    size_t symbolSize = decoderSettings.nFFTPoints*2;
    size_t prefixedSymbolSize = symbolSize + decoderSettings.PrefixSize;

    size_t nAvaiableifftPoints = (decoderSettings.nFFTPoints - (size_t)(decoderSettings.nFFTPoints/decoderSettings.PilotToneDistance));
    size_t nMaxEncodedBytes = (size_t)((nAvaiableifftPoints *  BITS_PER_FREQ_POINT)  / BITS_IN_BYTE);
    size_t nData = nMaxEncodedBytes;
    
    // Setup random float generator
    srand( (unsigned)time( NULL ) );

    // Create rx signal array to capable of holding 20 upsampled symbols with prefix
    size_t rxSignalSize = (prefixedSymbolSize * 10);
    size_t rxLastAllowedIndex = (prefixedSymbolSize * 9);

    uint8_t * txBytes = (uint8_t*) calloc(nData, sizeof(uint8_t));
    double * modulatorOutput = (double*) calloc(prefixedSymbolSize, sizeof(double));
    double * rxSignal = (double*) calloc(rxSignalSize, sizeof(double));

    // Initialize Encoder objects
    QamModulator qam(encoderSettings);
    FFT ifft(encoderSettings);
    NyquistModulator nyquistModulator(encoderSettings); // ifft.out
    
    // Initialize Decoder objects
    FFT fft(decoderSettings);
    NyquistModulator nyquistDemodulator(decoderSettings );    // fft.in
    Detector detector(decoderSettings, fft, nyquistDemodulator);

    // Generate Random Bytes
    for(size_t i = 0; i < nData; i++)
    {
        txBytes[i] = rand() % 255;
    }

    // Generate random index value for symbol start
    long int  symbolStart = rand() % rxLastAllowedIndex;
    printf("Randomly Generated Symbol Start = %lu\n",symbolStart);

    // Encode one symbol 
    qam.Modulate(txBytes, (DoubleVec &) ifft.in, nData);
    ifft.ComputeTransform( (fftw_complex *) &modulatorOutput[decoderSettings.PrefixSize]);
    nyquistModulator.Modulate(modulatorOutput);
    AddCyclicPrefix(modulatorOutput, symbolSize , decoderSettings.PrefixSize);
    // Check Prefix Has been added correctly
    for(size_t i = 0 ; i < decoderSettings.PrefixSize; i++)
    {
        //modulatorOutput[i] = modulatorOutput[symbolSize+i];

        if(modulatorOutput[i] != modulatorOutput[symbolSize + i])
        {
            printf("Missmatch symbolwithprefix[%lu]  = %f, modulatorOutput[%lu] = %f \n" , i , modulatorOutput[i], (symbolSize + i) , modulatorOutput[(symbolSize + i)]);
        }
    }

    // Copy the symbol with prefix to Rx signal array
    memcpy(&rxSignal[symbolStart], &modulatorOutput[0], sizeof(double)*prefixedSymbolSize);

    // Check if the symbol with prefix has been copied correctly
    for(size_t i = 0 ; i < prefixedSymbolSize ; i++)
    {
        if(rxSignal[symbolStart+i] != modulatorOutput[i])
        {
            printf("Missmatch rxSignal[%lu]  = %f, symbolwithprefix[%lu] = %f \n" , symbolStart+i , rxSignal[symbolStart+i], i , modulatorOutput[i]);
        }
    }

    long int foundSymbolStart =-1;
    auto start = std::chrono::steady_clock::now();
    size_t bufferCounter = 0;
    while(foundSymbolStart == -1)
    {
        foundSymbolStart = detector.FindSymbolStart(&rxSignal[bufferCounter*prefixedSymbolSize],nData);
        bufferCounter++;
    }
    auto end = std::chrono::steady_clock::now();
    std::cout << "bufferCounter = " << bufferCounter<<  std::endl;

    std::cout << "FindSymbolStart elapsed time: "
    << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
    << " ns" << std::endl;   

    long int ActualSymbolStart = symbolStart + 512;
    // Check if the correlator max output is where the symbol was inserted
    BOOST_CHECK_MESSAGE( foundSymbolStart == ActualSymbolStart, 
    "Symbol start has not been detected correctly, found indicates index =  " << foundSymbolStart << " Whereas the actual start = " << ActualSymbolStart  );  

}

/*
BOOST_AUTO_TEST_CASE(SymbolStartTest)
{
    printf("\nTesting Symbol Start Search...\n");

    // OFDM Codec Settings
    OFDMSettings encoderSettings;
    encoderSettings.type = FFTW_BACKWARD;
    encoderSettings.EnergyDispersalSeed = 10;
    encoderSettings.nFFTPoints = 1024; 
    encoderSettings.PilotToneDistance = 16; 
    encoderSettings.PilotToneAmplitude = 2.0; 
    encoderSettings.QAMSize = 2; 
    encoderSettings.PrefixSize = (size_t) ((encoderSettings.nFFTPoints*2)/4); // 1/4th of symbol

    OFDMSettings decoderSettings = encoderSettings;
    decoderSettings.type = FFTW_FORWARD;

    size_t symbolSize = decoderSettings.nFFTPoints * 2;
    size_t prefixedSymbolSize = symbolSize + decoderSettings.PrefixSize ;

    // Setup random float generator
    srand( (unsigned)time( NULL ) );

    // Create rx signal array to capable of holding 20 upsampled symbols with prefix
    size_t  rxSignalSize = (prefixedSymbolSize * 10);
    size_t  rxLastAllowedIndex = (prefixedSymbolSize * 9);

    // Generate random index value for symbol start
    size_t  symbolStart = rand() % rxLastAllowedIndex;
    symbolStart = 2347; // 2347 Edge case where detector fails
    printf("Randomly Generated Symbol Start = %lu\n",symbolStart);
 
    size_t nAvaiableifftPoints = (decoderSettings.nFFTPoints - (size_t)(decoderSettings.nFFTPoints/decoderSettings.PilotToneDistance));
    size_t nMaxEncodedBytes = (size_t)((nAvaiableifftPoints *  BITS_PER_FREQ_POINT)  / BITS_IN_BYTE);
    size_t nData = nMaxEncodedBytes;

    uint8_t * txBytes = (uint8_t*) calloc(nData, sizeof(uint8_t));
    double * EncoderOutput = (double*) calloc(prefixedSymbolSize, sizeof(double));
    double * rxSignal = (double*) calloc(rxSignalSize, sizeof(double));

    // Initialize Encoder objects
    QamModulator qam(encoderSettings);
    FFT ifft(encoderSettings);
    NyquistModulator nyquistModulator(encoderSettings); //  ifft.out

    // Initialize Decoder objects
    FFT fft(decoderSettings);
    NyquistModulator nyquistDemodulator(decoderSettings);    // fft.in
    Detector detector(decoderSettings, fft, nyquistDemodulator);

    // Generate Random bytes to encode
    for(size_t i = 0; i < nData; i++)
    {
        txBytes[i] = rand() % 255;
    }

    // Encode one symbol 
    // Start with QAM Modulation
    qam.Modulate(txBytes, (DoubleVec &) ifft.in, nData);    
    // Compute transform & put it into buffer, make sure to make space for prefix
    ifft.ComputeTransform( (fftw_complex *) &EncoderOutput[decoderSettings.PrefixSize]);
    // Modulate the tx buffer
    nyquistModulator.Modulate(&EncoderOutput[decoderSettings.PrefixSize]);
    // Add prefix 
    AddCyclicPrefix(EncoderOutput, symbolSize , decoderSettings.PrefixSize);

    // Copy the symbol with prefix to Rx signal array
    memcpy(&rxSignal[symbolStart], &EncoderOutput[0], sizeof(double)*prefixedSymbolSize);
    // Check if the symbol with prefix has been copied correctly
    for(size_t i = 0 ; i < prefixedSymbolSize ; i++)
    {
        if(rxSignal[symbolStart+i] != EncoderOutput[i])
        {
            printf("Missmatch rxSignal[%lu]  = %f, symbolwithprefix[%lu] = %f \n" , symbolStart+i , rxSignal[symbolStart+i], i , EncoderOutput[i]);
        }
    }

    long int CoarseSearchIndex = -1;
    size_t counter = 0;
    size_t vectorCounter = 0;
    auto start = std::chrono::steady_clock::now();
    //  While Coarse Search index has not been found
    while(CoarseSearchIndex == -1)
    {
        //CoarseSearchIndex = detector.CoarseSearch(&rxSignal[counter*prefixedSymbolSize]); //TODO:
        // If whole buffer searched or Corase Search successfull
        if(counter == 9)
        {
            // Break the while loop
            break;
        }
        // Process next data block
        counter++;
    }
    
    auto end = std::chrono::steady_clock::now();
    std::cout << "counter = " << counter << std::endl;   
    if(counter > 0 )
    {
        CoarseSearchIndex += ((counter-1) *prefixedSymbolSize);
    }
    

    std::cout << "Coarse Search elapsed time: "
    << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
    << " ns" << std::endl;   
  
    // Check if the correlator max output is where the symbol was inserted
    BOOST_CHECK_MESSAGE( ((size_t)CoarseSearchIndex == symbolStart), 
    "Symbol start has not been detected correctly, The peak occurs at index: " << CoarseSearchIndex );

    printf("Coarse Search Index = %lu\n", CoarseSearchIndex);

    // Skip the prefix 
    size_t symbolStartIndex = CoarseSearchIndex + decoderSettings.PrefixSize;
    printf("Symbol Start Index = %lu\n", symbolStartIndex);
    // Randomly change the Coarse Start by up to +/- 9 indexes
    size_t symbolStartOffsetIndex = symbolStartIndex + (rand() % 19 + (-9));
    

        double min = 100000000.0;
        double sumOfImag = 0.0;
        size_t searchRange = 25;

        // Plot fine search output
        std::vector<double> fineSearchOutput;

        size_t startIndex = 0;
        // Restric start index of fine search to 0th element
        if( (symbolStartOffsetIndex - ((searchRange-1) / 2)) >= 0)
        {
            startIndex = symbolStartOffsetIndex - (size_t)((searchRange-1) / 2);
        }
        else
        {
            startIndex = 0;
        }

        size_t FineSearchSymbolIndex = 0;
        start = std::chrono::steady_clock::now();
        // TODO: Handle an exception where the stop index is outside the boundaries 
        size_t stopIndex = symbolStartOffsetIndex + (size_t)((searchRange-1) / 2); 

        vectorCounter = 0;
        for(size_t i = startIndex; i < stopIndex; i++)
        {
            //sumOfImag = detector.ComputeSumOfImag(rxSignal, i, nData); TODO:
            fineSearchOutput.push_back(sumOfImag);
            vectorCounter++;
            if( sumOfImag < min )
            {
                min = sumOfImag;
                FineSearchSymbolIndex = i;
            }
        }
        end = std::chrono::steady_clock::now();

    //
    //    start = std::chrono::steady_clock::now();
    //    size_t FineSearchSymbolIndex = detector.FineSearch(rxSignal, symbolStartOffsetIndex,nData);
    //    end = std::chrono::steady_clock::now();
    //

    Gnuplot gp;
    gp << "plot '-' with line title 'Fine Search'\n";
    gp.send1d(fineSearchOutput);

    std::cout << "Fine Search elapsed time: "
    << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
    << " ns" << std::endl;   

    printf("Fine Search Index = %lu\n", FineSearchSymbolIndex);

    // Check if the fine search corresponds to symbol start
    BOOST_CHECK_MESSAGE( (FineSearchSymbolIndex == symbolStart+decoderSettings.PrefixSize), 
    "Symbol start has not been detected correctly, The lowest img sum occurs at start index: " << FineSearchSymbolIndex );  
        
}
*/

BOOST_AUTO_TEST_SUITE_END()
