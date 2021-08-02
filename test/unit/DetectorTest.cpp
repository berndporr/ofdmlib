#define BOOST_TEST_MODULE FourierTransformTest
#include <boost/test/unit_test.hpp>

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

    size_t nPoints = 1024;
    size_t symbolSize = nPoints*2;
    size_t prefixSize = (int) (symbolSize / 8);
    size_t symbolSizeWithPrefx = symbolSize + prefixSize;
    size_t pilotToneStep = 16;
    size_t bitsPerSymbol = 2;
    double pilotToneAmplitude = 2.0;
    size_t energyDispersalSeed = 10;
    size_t nAvaiableifftPoints = (nPoints - (int)(nPoints/pilotToneStep));
    size_t nMaxEncodedBytes = (int)((nAvaiableifftPoints *  bitsPerSymbol)  / 8);
    size_t nData = nMaxEncodedBytes;

    // Setup random float generator
    srand( (unsigned)time( NULL ) );

    // Create rx signal array to capable of holding 20 upsampled symbols with prefix
    size_t rxSignalSize = (symbolSizeWithPrefx * 10);
    size_t rxLastAllowedIndex = (symbolSizeWithPrefx * 9);

    uint8_t * txBytes = (uint8_t*) calloc(nData, sizeof(uint8_t));
    double * modulatorOutput = (double*) calloc(symbolSizeWithPrefx, sizeof(double));
    double * corellatorOutput = (double*) calloc(rxSignalSize, sizeof(double));
    double * rxSignal = (double*) calloc(rxSignalSize, sizeof(double));

    //double modulatorOutput[symbolSizeWithPrefx];
    //DoubleVec modulatorOutput;
    //modulatorOutput.resize(symbolSizeWithPrefx);
    //DoubleVec rxSignal(rxSignalSize);
    //DoubleVec corellatorOutput(rxSignalSize);
    //ByteVec txBytes(nData);

    // Initialize Encoder objects
    QamModulator qam(nPoints, pilotToneStep, pilotToneAmplitude, energyDispersalSeed, bitsPerSymbol);
    ofdmFFT ifft(nPoints, FFTW_BACKWARD, pilotToneStep);
    NyquistModulator nyquistModulator(nPoints, ifft.out);
    
    // Initialize Decoder objects
    ofdmFFT fft(nPoints, FFTW_FORWARD, pilotToneStep);
    NyquistModulator nyquistDemodulator(nPoints, fft.in);    
    Detector detector(nPoints, prefixSize, &fft, &nyquistDemodulator);


    // Generate Random Bytes
    for(size_t i = 0; i < nData; i++)
    {
        txBytes[i] = rand() % 255;
    }

    // Generate random index value for symbol start
    size_t  symbolStart = rand() % rxLastAllowedIndex;
    printf("Randomly Generated Symbol Start = %lu\n",symbolStart);

    // Encode one symbol 
    qam.Modulate(txBytes, (DoubleVec &) ifft.in, nData);
    ifft.ComputeTransform( (fftw_complex *) &modulatorOutput[prefixSize]);
    nyquistModulator.Modulate( modulatorOutput, prefixSize);
    AddCyclicPrefix(modulatorOutput, symbolSize , prefixSize);
    // Check Prefix Has been added correctly
    for(size_t i = 0 ; i < prefixSize; i++)
    {
        //modulatorOutput[i] = modulatorOutput[symbolSize+i];

        if(modulatorOutput[i] != modulatorOutput[symbolSize + i])
        {
            printf("Missmatch symbolwithprefix[%lu]  = %f, modulatorOutput[%lu] = %f \n" , i , modulatorOutput[i], (symbolSize + i) , modulatorOutput[(symbolSize + i)]);
        }
    }

    // Copy the symbol with prefix to Rx signal array
    //std::copy(modulatorOutput.begin(), modulatorOutput.begin()+symbolSizeWithPrefx, rxSignal.begin()+symbolStart);
    memcpy(&rxSignal[symbolStart], &modulatorOutput[0], sizeof(double)*symbolSizeWithPrefx);

    // Check if the symbol with prefix has been copied correctly
    for(size_t i = 0 ; i < symbolSizeWithPrefx ; i++)
    {
        if(rxSignal[symbolStart+i] != modulatorOutput[i])
        {
            printf("Missmatch rxSignal[%lu]  = %f, symbolwithprefix[%lu] = %f \n" , symbolStart+i , rxSignal[symbolStart+i], i , modulatorOutput[i]);
        }
    }

    auto start = std::chrono::steady_clock::now();
    for(size_t i = 0; i < rxLastAllowedIndex; i++)
    {
        corellatorOutput[i] = detector.ExecuteCorrelator(rxSignal, i);
    }
    auto end = std::chrono::steady_clock::now();

    std::cout << "Cross-Corellator elapsed time: "
    << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
    << " ns" << std::endl;   

    /*
    // Plot modulator output, symbol
    std::vector<double> modOutput(symbolSizeWithPrefx);
    for(uint32_t i = 0; i < symbolSizeWithPrefx; i++) 
    {
        modOutput.at(i) = modulatorOutput[i];
    }

    // Plot Corellator output
    std::vector<double> corOutput(rxLastAllowedIndex);
    // Copy Correlator output to vector
    for(uint32_t i = 0; i < rxLastAllowedIndex; i++) 
    {
        // Square Result to reduce noise
        corOutput.at(i) = corellatorOutput[i] * corellatorOutput[i];
    }


    Gnuplot gp;
    gp << "plot '-' with line title 'Correlation'\n";
    gp.send1d(corOutput);

    Gnuplot gp1;
    gp1 << "plot '-' with line title 'Symbol with prefix'\n";
    gp1.send1d(modOutput);
    */
    
    // Find Peak in correlation
    size_t peakIndex = 0;
    double max = 0.0;
    start = std::chrono::steady_clock::now();
    for (size_t i = 0; i < rxSignalSize; i++)
    {
        if(corellatorOutput[i] > max)
        {
            max = corellatorOutput[i];
            peakIndex = i;
        }
    }
    end = std::chrono::steady_clock::now();

    std::cout << "Peak Search elapsed time: "
    << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
    << " ns" << std::endl;   

    // Check if the correlator max output is where the symbol was inserted
    BOOST_CHECK_MESSAGE( (peakIndex == symbolStart), 
    "Symbol start has not been detected correctly, The max correlation occurs at index =  " << peakIndex );  


    start = std::chrono::steady_clock::now();
    for(size_t i = 0; i < symbolSize; i++)
    {
        corellatorOutput[i] = detector.ExecuteCorrelator(rxSignal, i);
    }
    end = std::chrono::steady_clock::now();

    std::cout << "symbolSize Cross-Corellator elapsed time: "
    << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
    << " ns" << std::endl;   
}

BOOST_AUTO_TEST_CASE(SymbolStartTest)
{
    printf("\nTesting Symbol Start Search...\n");

    size_t nPoints = 1024;
    size_t symbolSize = nPoints*2;
    size_t prefixSize = (int) (symbolSize / 4);
    size_t symbolSizeWithPrefx = symbolSize + prefixSize;
    size_t pilotToneStep = 16;
    double pilotToneAmplitude = 2.0;
    size_t energyDispersalSeed = 10;
    size_t bitsPerSymbol = 2;

    // Setup random float generator
    srand( (unsigned)time( NULL ) );

    // Create rx signal array to capable of holding 20 upsampled symbols with prefix
    size_t  rxSignalSize = (symbolSizeWithPrefx * 10);
    size_t  rxLastAllowedIndex = (symbolSizeWithPrefx * 9);

    // Generate random index value for symbol start
    size_t  symbolStart = rand() % rxLastAllowedIndex;
    printf("Randomly Generated Symbol Start = %lu\n",symbolStart);

    size_t nAvaiableifftPoints = (nPoints - (int)(nPoints/pilotToneStep));
    size_t nMaxEncodedBytes = (int)((nAvaiableifftPoints *  bitsPerSymbol)  / 8);
    size_t nData = nMaxEncodedBytes;

    //ByteVec txBytes(nData);
    //DoubleVec EncoderOutput(symbolSizeWithPrefx);
    //DoubleVec rxSignal(rxSignalSize);

    uint8_t * txBytes = (uint8_t*) calloc(nData, sizeof(uint8_t));
    double * EncoderOutput = (double*) calloc(symbolSizeWithPrefx, sizeof(double));
    //double * corellatorOutput = (double*) calloc(rxSignalSize, sizeof(double));
    double * rxSignal = (double*) calloc(rxSignalSize, sizeof(double));


    // Initialize Encoder objects
    QamModulator qam(nPoints, pilotToneStep, pilotToneAmplitude, energyDispersalSeed, bitsPerSymbol);
    ofdmFFT ifft(nPoints, FFTW_BACKWARD, pilotToneStep);
    NyquistModulator nyquistModulator(nPoints, ifft.out);
       

    // Initialize Decoder objects
    ofdmFFT fft(nPoints, FFTW_FORWARD, pilotToneStep);
    NyquistModulator nyquistDemodulator(nPoints, fft.in);    
    Detector detector(nPoints, prefixSize, &fft, &nyquistDemodulator);

    // Randomly fill ifft input
    for(size_t i = 0; i < nData; i++)
    {
        txBytes[i] = rand() % 255;
    }

    // Encode one symbol 
    // (fftw_complex *) &output[GetSettings().cyclicPrefixSize]);
    qam.Modulate(txBytes, (DoubleVec &) ifft.in, nData);
    ifft.ComputeTransform( (fftw_complex *) &EncoderOutput[prefixSize]);
    nyquistModulator.Modulate( EncoderOutput, prefixSize);
    AddCyclicPrefix(EncoderOutput, symbolSize , prefixSize);

    // Copy the symbol with prefix to Rx signal array
    //std::copy(EncoderOutput.begin(), EncoderOutput.begin()+symbolSizeWithPrefx, rxSignal.begin()+symbolStart);
    memcpy(&rxSignal[symbolStart], &EncoderOutput[0], sizeof(double)*symbolSizeWithPrefx);
    // Check if the symbol with prefix has been copied correctly
    for(size_t i = 0 ; i < symbolSizeWithPrefx ; i++)
    {
        if(rxSignal[symbolStart+i] != EncoderOutput[i])
        {
            printf("Missmatch rxSignal[%lu]  = %f, symbolwithprefix[%lu] = %f \n" , symbolStart+i , rxSignal[symbolStart+i], i , EncoderOutput[i]);
        }
    }

    long int CoarseSearchIndex = -1;
    size_t counter = 0;
    auto start = std::chrono::steady_clock::now();
    while(CoarseSearchIndex == -1)
    {
        CoarseSearchIndex = detector.CoarseSearch(&rxSignal[counter*symbolSizeWithPrefx]);
        counter++;
        if(counter == 9)
        {
            break;
        }
    }
    
    auto end = std::chrono::steady_clock::now();
    std::cout << "counter = " << counter << std::endl;   
    CoarseSearchIndex += ((counter-1)*symbolSizeWithPrefx);

    std::cout << "Coarse Search elapsed time: "
    << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
    << " ns" << std::endl;   
  
    // Check if the correlator max output is where the symbol was inserted
    BOOST_CHECK_MESSAGE( ((size_t)CoarseSearchIndex == symbolStart), 
    "Symbol start has not been detected correctly, The peak occurs at index: " << CoarseSearchIndex );

    printf("Coarse Search Index = %lu\n", CoarseSearchIndex);

    // Skip the prefix 
    size_t symbolStartIndex = CoarseSearchIndex + prefixSize;
    printf("Symbol Start Index = %lu\n", symbolStartIndex);
    // Randomly change the Coarse Start by up to +/- 9 indexes
    size_t symbolStartOffsetIndex = symbolStartIndex + (rand() % 19 + (-9));

    start = std::chrono::steady_clock::now();
    size_t FineSearchSymbolIndex = detector.FineSearch(rxSignal, symbolStartOffsetIndex, nData);
    end = std::chrono::steady_clock::now();

    std::cout << "Fine Search elapsed time: "
    << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
    << " ns" << std::endl;   

    printf("Fine Search Index = %lu\n", FineSearchSymbolIndex);

    // Check if the fine search corresponds to symbol start
    BOOST_CHECK_MESSAGE( (FineSearchSymbolIndex == symbolStart+prefixSize), 
    "Symbol start has not been detected correctly, The lowest img sum occurs at start index: " << FineSearchSymbolIndex );  
        
}

BOOST_AUTO_TEST_SUITE_END()
