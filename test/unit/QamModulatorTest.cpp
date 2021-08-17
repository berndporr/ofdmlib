#define BOOST_TEST_MODULE QamModulatorTest
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

    // OFDM Codec Settings
    OFDMSettings encoderSettings;
    encoderSettings.type = FFTW_BACKWARD;
    encoderSettings.EnergyDispersalSeed = 10;
    encoderSettings.nFFTPoints = 1024; 
    encoderSettings.PilotToneDistance = 16; 
    encoderSettings.PilotToneAmplitude = 2.0; 
    encoderSettings.QAMSize = 2; 
    encoderSettings.PrefixSize = (int) ((encoderSettings.nFFTPoints*2)/4); // 1/4th of symbol

    size_t nAvaiableifftPoints = (encoderSettings.nFFTPoints - (size_t)(encoderSettings.nFFTPoints/encoderSettings.PilotToneDistance));
    size_t nMaxEncodedBytes = (int)((nAvaiableifftPoints *  BITS_PER_FREQ_POINT)  / BITS_IN_BYTE);
    size_t nData = nMaxEncodedBytes;

    std::cout << "nMaxEncodedBytes = " << nMaxEncodedBytes << std::endl;
    // Setup random float generator
    srand( (unsigned)time( NULL ) );

    uint8_t * TxCharArray = (uint8_t*) calloc(nData, sizeof(uint8_t));
    uint8_t * RxCharArray = (uint8_t*) calloc(nData, sizeof(uint8_t));

    for(size_t i = 0; i < nData; i++ )
    {
        TxCharArray[i] = (unsigned char) rand() % 255;
    }

    DoubleVec QamOutput(encoderSettings.nFFTPoints*2);

    QamModulator qam(encoderSettings);

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
