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
#include "cyclicprefix.h"

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
    printf("\nTesting Correlator...\n");

    uint16_t nPoints = 512;
    uint32_t symbolSize = nPoints*2;
    uint32_t prefixSize = (int) (symbolSize / 8);
    uint32_t symbolSizeWithPrefx = symbolSize + prefixSize;

    // Setup random float generator
    srand( (unsigned)time( NULL ) );

    double modulatorOutput[symbolSizeWithPrefx];

    // Create rx signal array to capable of holding 20 upsampled symbols with prefix
    uint32_t rxSignalSize = (symbolSizeWithPrefx * 10);
    uint32_t rxLastAllowedIndex = (symbolSizeWithPrefx * 9);
    double rxSignal[rxSignalSize];
    double corellatorOutput[rxSignalSize];
    
    Correlator correlator(nPoints, prefixSize, rxSignal, rxSignalSize);

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

    // Simulate modulator output
    for (uint32_t i = prefixSize; i < symbolSizeWithPrefx; i++)
    {
        modulatorOutput[i] = (float) rand()/RAND_MAX;
    }

    // Create Cyclic Prefix
    AddCyclicPrefix(&modulatorOutput[0], symbolSize, prefixSize);
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
    memcpy( &rxSignal[symbolStart], &modulatorOutput[0], (sizeof(double)*symbolSizeWithPrefx));
    // Check if the symbol with prefix has been copied correctly
    for(uint32_t i = 0 ; i < symbolSizeWithPrefx ; i++)
    {
        if(rxSignal[symbolStart+i] != modulatorOutput[i])
        {
            printf("Missmatch rxSignal[%d]  = %f, symbolwithprefix[%d] = %f \n" , symbolStart+i , rxSignal[symbolStart+i], i , modulatorOutput[i]);
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
    for (uint32_t i = 0; i < rxSignalSize; i++)
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
    "Symbol start has not been detected correctly, The peak occurs at index: " << peakIndex );  
        
}


BOOST_AUTO_TEST_CASE(CoarseSearchTest)
{
    printf("\nTesting Coarse Search...\n");
    uint16_t nPoints = 512;
    uint32_t symbolSize = nPoints*2;
    uint32_t prefixSize = (int) (symbolSize / 8);
    uint32_t symbolSizeWithPrefx = symbolSize + prefixSize;

    // Setup random float generator
    srand( (unsigned)time( NULL ) );

    double modulatorOutput[symbolSizeWithPrefx];

    // Create rx signal array to capable of holding 20 upsampled symbols with prefix
    uint32_t rxSignalSize = (symbolSizeWithPrefx * 10);
    uint32_t rxLastAllowedIndex = (symbolSizeWithPrefx * 9);
    double rxSignal1[rxSignalSize] = {0};
    double corellatorOutput[rxSignalSize];
    
    Correlator correlator(nPoints, prefixSize, rxSignal1, rxSignalSize);

    // Populate all Rx signal with random values
    /*
    for (uint16_t i = 0; i < rxSignalSize; i++)
    {
        rxSignal1[i] = (float) rand()/RAND_MAX;
    }
    */

    // Generate random index value for symbol start
    uint32_t symbolStart = rand() % rxLastAllowedIndex;
    printf("Random Symbol Start = %d\n",symbolStart);

    // Simulate modulator output
    for (uint32_t i = prefixSize; i < symbolSizeWithPrefx; i++)
    {
        modulatorOutput[i] = (float) rand()/RAND_MAX;
    }

    // Create Cyclic Prefix
    AddCyclicPrefix(&modulatorOutput[0], symbolSize, prefixSize);
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
    memcpy( &rxSignal1[symbolStart], &modulatorOutput[0], (sizeof(double)*symbolSizeWithPrefx));
    // Check if the symbol with prefix has been copied correctly
    for(uint32_t i = 0 ; i < symbolSizeWithPrefx ; i++)
    {
        if(rxSignal1[symbolStart+i] != modulatorOutput[i])
        {
            printf("Missmatch rxSignal1[%d]  = %f, symbolwithprefix[%d] = %f \n" , symbolStart+i , rxSignal1[symbolStart+i], i , modulatorOutput[i]);
        }
    }

    uint32_t correlatorCoarseSearchIndex = 0;
    auto start = std::chrono::steady_clock::now();
        correlatorCoarseSearchIndex = correlator.CoarseSearch();
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
