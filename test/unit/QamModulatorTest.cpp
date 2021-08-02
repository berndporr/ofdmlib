#define BOOST_TEST_MODULE FourierTransformTest
#include <boost/test/unit_test.hpp>

// For IO
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <cmath> 
#include <iostream>
#include <unistd.h>
#include <vector>

// For measuring elapsed time
#include <chrono>

// For Random Float Generator
#include <time.h>

// For object under test
#include "qam-modulator.h"
#include "common.h"
#include "fftw3.h"


/**
* Test NYQUIST MODULATOR
* 
*/
BOOST_AUTO_TEST_SUITE(QAM_MODULATOR)


/**
* Generate random data execute QAM Modulator and demodulator.
* 
*/
BOOST_AUTO_TEST_CASE(QamModToDemod)
{
    printf("\nTesting QAM Modulation to Demodulation...\n");
    printf("\nMdoulator:\n");

    size_t nPoints = 512;
    size_t pilotToneStep = 8;
    size_t energyDispersalSeed = 10;
    size_t bitsPerSymbol = 2;
    size_t nAvaiableifftPoints = (nPoints - (int)(nPoints/pilotToneStep));
    size_t nMaxEncodedBytes = (int)((nAvaiableifftPoints *  bitsPerSymbol)  / 8);
    size_t nData = nMaxEncodedBytes;
    double pilotToneAmplitude = 2.0;

    std::cout << "nMaxEncodedBytes = " << nMaxEncodedBytes << std::endl;
    // Setup random float generator
    srand( (unsigned)time( NULL ) );

    //std::vector<unsigned char> TxCharArray(nData);
    //std::vector<unsigned char> RxCharArray(nData);

    uint8_t * TxCharArray = (uint8_t*) calloc(nData, sizeof(uint8_t));
    uint8_t * RxCharArray = (uint8_t*) calloc(nData, sizeof(uint8_t));
    //double * QamOutput = (double*) calloc(nPoints*2, sizeof(double));
    //double * rxSignal = (double*) calloc(symbolSize*10, sizeof(double));


    for(size_t i = 0; i < nData; i++ )
    {
        TxCharArray[i] = (unsigned char) rand() % 255;
    }

    DoubleVec QamOutput(nPoints*2);

    QamModulator qam(nPoints, pilotToneStep, pilotToneAmplitude, energyDispersalSeed, bitsPerSymbol);

    auto start = std::chrono::steady_clock::now();
    qam.Modulate(TxCharArray, QamOutput, nData);
    auto end = std::chrono::steady_clock::now();

    std::cout << "QAM Modulator elapsed time: "
    << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
    << " ns" << std::endl;

    printf("\nDemodulator:\n");

    start = std::chrono::steady_clock::now();
    qam.Demodulate(QamOutput, RxCharArray, nData);
    end = std::chrono::steady_clock::now();

    std::cout << "QAM Demodulator elapsed time: "
    << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
    << " ns" << std::endl;
    
    for(size_t i = 0; i < nData; i++)
    {
        //printf("TxCharArray[%lu] = %d ,RxCharArray[%lu] = %d\n"
        //,i,(int)TxCharArray[i] ,i, (int)RxCharArray[i] );

        BOOST_CHECK_MESSAGE( (TxCharArray[i] == RxCharArray[i] ), 
        "Elements differ! - Occured at index: " << i );
    }

}

BOOST_AUTO_TEST_SUITE_END()
