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
#include "bandpass.h"
#include "cyclicprefix.h"

#define DIFFERENCE_THRESHOLD 0.0001

/**
* Test BANDPASS MODULATOR
* 
*/
BOOST_AUTO_TEST_SUITE(CYCLIC_PREFIX)


/**
* 
*/
BOOST_AUTO_TEST_CASE(CorrelatorTest)
{
    printf("\nTesting Coarse Search Using Cross-Correlator...\n");
    printf("\nMdoulator:\n");

    uint16_t nPoints = 512;
    uint32_t symbolSize = nPoints*2;
    uint32_t prefixSize = (int) (symbolSize / 8);
    uint32_t symbolSizeWithPrefx = symbolSize + prefixSize;

    // Setup random float generator
    srand( (unsigned)time( NULL ) );

    double modulatorOutput[symbolSize] = {0};
    double symbolwithprefix[symbolSizeWithPrefx] = {0};

    fftw_complex *modulatorInput;
    modulatorInput = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * nPoints);

    // Create rx signal array to capable of holding 20 upsampled symbols with prefix
    uint32_t rxSignalSize = (symbolSizeWithPrefx * 20);
    uint32_t rxLastAllowedIndex = rxSignalSize - symbolSizeWithPrefx;
    double rxSignal[rxSignalSize]  = { 0 };
    double corellatorOutput[rxSignalSize] = { 0 };
    
    AutoCorrelator correlator(nPoints, prefixSize, rxSignal, rxSignalSize);

    // Populate all Rx signal with random values
    /*
    for (uint16_t i = 0; i < rxSignalSize; i++)
    {
        rxSignal[i] = (float) rand()/RAND_MAX;
    }
    */

    // Generate random index value for symbol start
    uint32_t symbolStart = rand() % rxLastAllowedIndex;
    printf("Random Symbol Start = %d\n",symbolStart);

    for (uint16_t i = 0; i < symbolSize; i++)
    {
        modulatorOutput[i] = (float) rand()/RAND_MAX;
    }

    // Create Cyclic Prefix
    memcpy( &symbolwithprefix[0], &modulatorOutput[symbolSize-prefixSize], (sizeof(double)*prefixSize));
    // Check Prefix Has been added correctly
    for(int i = 0 ; i < prefixSize; i++)
    {
        if(symbolwithprefix[i] != modulatorOutput[symbolSize - prefixSize + i])
        {
            printf("Missmatch symbolwithprefix[%d]  = %f, modulatorOutput[%d] = %f \n" , i , symbolwithprefix[i], (symbolSize - prefixSize + i) , modulatorOutput[symbolSize - prefixSize + i]);
        }
    }
    // Copy the rest of the symbol
    memcpy( &symbolwithprefix[prefixSize], &modulatorOutput[0], (sizeof(double)*symbolSize));
    // Check symbol has been copied correctly
    for(int i = prefixSize ; i < symbolSize ; i++)
    {
        if(symbolwithprefix[prefixSize+i] != modulatorOutput[i])
        {
            printf("Missmatch symbolwithprefix[%d]  = %f, modulatorOutput[%d] = %f \n" , prefixSize+i , symbolwithprefix[prefixSize+i], i , modulatorOutput[i]);
        }
    }
    // Copy the symbol with prefix to Rx signal array
    memcpy( &rxSignal[symbolStart], &symbolwithprefix[0], (sizeof(double)*symbolSizeWithPrefx));
    // Check if the symbol with prefix has been copied correctly
    for(int i = 0 ; i < symbolSizeWithPrefx ; i++)
    {
        if(rxSignal[symbolStart+i] != symbolwithprefix[i])
        {
            printf("Missmatch rxSignal[%d]  = %f, symbolwithprefix[%d] = %f \n" , symbolStart+i , rxSignal[symbolStart+i], i , symbolwithprefix[i]);
        }
    }


    // Error occured at Random Symbol Start = 16077
    
    auto start = std::chrono::steady_clock::now();
    for(uint32_t i = 0; i < rxLastAllowedIndex; i++)
    {
        corellatorOutput[i] = correlator.ExecuteCorrelator(i);
    }
    auto end = std::chrono::steady_clock::now();

    std::cout << "Cross-Corellator elapsed time: "
    << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
    << " ns" << std::endl;   
    
    // Plot modulator output, symbol
    std::vector<double> modOutput(symbolSize);
    for(uint32_t i = 0; i < symbolSize; i++) 
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

    
    // Find Peak in correlation
    uint32_t peakIndex = 0;
    double max = 0.0;
    for (uint16_t i = 0; i < rxSignalSize; i++)
    {
        if(corellatorOutput[i] > max)
        {
            max = corellatorOutput[i];
            peakIndex = i;
        }
    }
    printf("Peak occurs at index = %d ,value %f", peakIndex, max);
    
}

BOOST_AUTO_TEST_SUITE_END()
