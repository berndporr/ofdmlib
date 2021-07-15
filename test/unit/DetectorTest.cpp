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

    uint32_t nPoints = 512;
    uint32_t symbolSize = nPoints*2;
    uint32_t prefixSize = (int) (symbolSize / 8);
    uint32_t symbolSizeWithPrefx = symbolSize + prefixSize;
    uint32_t pilotToneStep = 16;
    int type = FFTW_FORWARD;

    // Setup random float generator
    srand( (unsigned)time( NULL ) );

    //double modulatorOutput[symbolSizeWithPrefx];
    DoubleVec modulatorOutput;
    modulatorOutput.resize(symbolSizeWithPrefx);

    // Create rx signal array to capable of holding 20 upsampled symbols with prefix
    uint32_t rxSignalSize = (symbolSizeWithPrefx * 10);
    uint32_t rxLastAllowedIndex = (symbolSizeWithPrefx * 9);

    DoubleVec rxSignal;
    rxSignal.resize(rxSignalSize);

    DoubleVec dummyModulatorOutput;
    dummyModulatorOutput.resize(symbolSizeWithPrefx);


    DoubleVec corellatorOutput;
    corellatorOutput.resize(rxSignalSize);

    ofdmFFT fft(nPoints, type, pilotToneStep);
    NyquistModulator nyquistModulator(nPoints, ( type == -1 ) ?  fft.out : fft.in);    
    Detector detector(nPoints, prefixSize, &fft, &nyquistModulator);

    // Generate random index value for symbol start
    uint32_t symbolStart = rand() % rxLastAllowedIndex;
    printf("Randomly Generated Symbol Start = %d\n",symbolStart);

    // Simulate modulator output
    for (uint32_t i = prefixSize; i < symbolSizeWithPrefx; i++)
    {
        modulatorOutput[i] = (double) rand()/RAND_MAX;
    }

    // Create Cyclic Prefix
    AddCyclicPrefix(modulatorOutput, symbolSize, prefixSize);
    // Check Prefix Has been added correctly
    for(uint32_t i = 0 ; i < prefixSize; i++)
    {
        //modulatorOutput[i] = modulatorOutput[symbolSize+i];

        if(modulatorOutput[i] != modulatorOutput[symbolSize + i])
        {
            printf("Missmatch symbolwithprefix[%d]  = %f, modulatorOutput[%d] = %f \n" , i , modulatorOutput[i], (symbolSize + i) , modulatorOutput[(symbolSize + i)]);
        }
    }

    // Copy the symbol with prefix to Rx signal array
    //memcpy( &rxSignal[symbolStart], &modulatorOutput[0], (sizeof(double)*symbolSizeWithPrefx));
    std::copy(modulatorOutput.begin(), modulatorOutput.begin()+symbolSizeWithPrefx, rxSignal.begin()+symbolStart);

    // Check if the symbol with prefix has been copied correctly
    for(uint32_t i = 0 ; i < symbolSizeWithPrefx ; i++)
    {
        if(rxSignal[symbolStart+i] != modulatorOutput[i])
        {
            printf("Missmatch rxSignal[%d]  = %f, symbolwithprefix[%d] = %f \n" , symbolStart+i , rxSignal[symbolStart+i], i , modulatorOutput[i]);
        }
    }

    auto start = std::chrono::steady_clock::now();
    for(uint32_t i = 0; i < rxLastAllowedIndex; i++)
    {
        corellatorOutput[i] = detector.ExecuteCorrelator(rxSignal, i);
    }
    auto end = std::chrono::steady_clock::now();

    std::cout << "Cross-Corellator elapsed time: "
    << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
    << " ns" << std::endl;   

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

    /*
    Gnuplot gp;
    gp << "plot '-' with line title 'Correlation'\n";
    gp.send1d(corOutput);

    Gnuplot gp1;
    gp1 << "plot '-' with line title 'Symbol with prefix'\n";
    gp1.send1d(modOutput);
    */
    
    // Find Peak in correlation
    uint32_t peakIndex = 0;
    double max = 0.0;
    start = std::chrono::steady_clock::now();
    for (uint32_t i = 0; i < corellatorOutput.size(); i++)
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
        
}


BOOST_AUTO_TEST_CASE(CoarseSearchTest)
{
    printf("\nTesting Coarse Search...\n");

    uint32_t nPoints = 512;
    uint32_t symbolSize = nPoints*2;
    uint32_t prefixSize = (int) (symbolSize / 8);
    uint32_t symbolSizeWithPrefx = symbolSize + prefixSize;
    uint32_t pilotToneStep = 16;
    int type = FFTW_FORWARD;

    // Setup random float generator
    srand( (unsigned)time( NULL ) );

    //double modulatorOutput[symbolSizeWithPrefx];
    DoubleVec modulatorOutput;
    modulatorOutput.resize(symbolSizeWithPrefx);

    // Create rx signal array to capable of holding 20 upsampled symbols with prefix
    uint32_t rxSignalSize = (symbolSizeWithPrefx * 10);
    uint32_t rxLastAllowedIndex = (symbolSizeWithPrefx * 9);

    DoubleVec rxSignal;
    rxSignal.resize(rxSignalSize);

    DoubleVec dummyModulatorOutput;
    dummyModulatorOutput.resize(symbolSizeWithPrefx);

    ofdmFFT fft(nPoints, type, pilotToneStep);
    NyquistModulator nyquistModulator(nPoints, ( type == -1 ) ?  fft.out : fft.in);    
    Detector detector(nPoints, prefixSize, &fft, &nyquistModulator);

    // Generate random index value for symbol start
    uint32_t symbolStart = rand() % rxLastAllowedIndex;
    printf("Random Symbol Start = %d\n",symbolStart);

    // Simulate modulator output
    for (uint32_t i = prefixSize; i < symbolSizeWithPrefx; i++)
    {
        modulatorOutput[i] = (double) rand()/RAND_MAX;
    }

    // Create Cyclic Prefix
    AddCyclicPrefix( modulatorOutput, symbolSize, prefixSize);
    // Check Prefix Has been added correctly
    for(uint32_t i = 0 ; i < prefixSize; i++)
    {
        if(modulatorOutput[i] != modulatorOutput[symbolSize + i])
        {
            printf("Missmatch symbolwithprefix[%d]  = %f, modulatorOutput[%d] = %f \n" , i , modulatorOutput[i], (symbolSize + i) , modulatorOutput[(symbolSize + i)]);
        }
    }

    // Copy the symbol with prefix to Rx signal array
    //memcpy( &rxSignal[symbolStart], &modulatorOutput[0], (sizeof(double)*symbolSizeWithPrefx));
    std::copy(modulatorOutput.begin(), modulatorOutput.begin()+symbolSizeWithPrefx, rxSignal.begin()+symbolStart);
    //std::copy()
    // Check if the symbol with prefix has been copied correctly
    for(uint32_t i = 0 ; i < symbolSizeWithPrefx ; i++)
    {
        if(rxSignal[symbolStart+i] != modulatorOutput[i])
        {
            printf("Missmatch rxSignal[%d]  = %f, symbolwithprefix[%d] = %f \n" , symbolStart+i , rxSignal[symbolStart+i], i , modulatorOutput[i]);
        }
    }

    uint32_t correlatorCoarseSearchIndex = 0;
    auto start = std::chrono::steady_clock::now();
        correlatorCoarseSearchIndex = detector.CoarseSearch(rxSignal);
    auto end = std::chrono::steady_clock::now();

    std::cout << "Coarse Search elapsed time: "
    << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
    << " ns" << std::endl;   
  
    printf("Coarse Search Index = %d\n", correlatorCoarseSearchIndex);

    // Check if the correlator max output is where the symbol was inserted
    BOOST_CHECK_MESSAGE( (correlatorCoarseSearchIndex == symbolStart), 
    "Symbol start has not been detected correctly, The peak occurs at index: " << correlatorCoarseSearchIndex );  
        
}

BOOST_AUTO_TEST_SUITE_END()
