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
    OFDMSettingsStruct encoderSettingsStruct;
    encoderSettingsStruct.type = FFTW_BACKWARD;
    encoderSettingsStruct.EnergyDispersalSeed = 10;
    encoderSettingsStruct.nFFTPoints = 1024; 
    encoderSettingsStruct.PilotToneDistance = 16; 
    encoderSettingsStruct.PilotToneAmplitude = 2.0; 
    encoderSettingsStruct.QAMSize = 2; 
    encoderSettingsStruct.PrefixSize = (size_t) ((encoderSettingsStruct.nFFTPoints*2)/4); // 1/4th of symbol
    encoderSettingsStruct.nDataBytesPerSymbol = 100;

    OFDMSettings encoderSettings(encoderSettingsStruct);

    std::cout << "m_nDataBytesPerSymbol = " <<  encoderSettings.m_nDataBytesPerSymbol << std::endl;
    // Setup random float generator
    srand( (unsigned)time( NULL ) );

    uint8_t * TxCharArray = (uint8_t*) calloc( encoderSettings.m_nDataBytesPerSymbol, sizeof(uint8_t));
    uint8_t * RxCharArray = (uint8_t*) calloc( encoderSettings.m_nDataBytesPerSymbol, sizeof(uint8_t));

    for(size_t i = 0; i <  encoderSettings.m_nDataBytesPerSymbol; i++ )
    {
        TxCharArray[i] = (unsigned char) rand() % 255;
    }

    fftw_complex *QamDemodulatorOutput = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * encoderSettings.m_nFFTPoints);
    QamModulator qam(encoderSettings);

    auto start = std::chrono::steady_clock::now();
    qam.Modulate(TxCharArray, QamDemodulatorOutput);
    auto end = std::chrono::steady_clock::now();

    std::cout << "QAM Modulator elapsed time: "
    << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
    << " ns" << std::endl;

    printf("\nDemodulator:\n");

    start = std::chrono::steady_clock::now();
    qam.Demodulate(QamDemodulatorOutput, RxCharArray);
    end = std::chrono::steady_clock::now();

    std::cout << "QAM Demodulator elapsed time: "
    << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
    << " ns" << std::endl;
    
    for(size_t i = 0; i < encoderSettings.m_nDataBytesPerSymbol; i++)
    {
        //printf("TxCharArray[%lu] = %d ,RxCharArray[%lu] = %d\n"
        //,i,(int)TxCharArray[i] ,i, (int)RxCharArray[i] );

        BOOST_CHECK_MESSAGE( (TxCharArray[i] == RxCharArray[i] ), 
        "Elements differ! - Occured at index: " << i );
    }

}

BOOST_AUTO_TEST_SUITE_END()
