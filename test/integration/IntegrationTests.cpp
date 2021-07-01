#define BOOST_TEST_MODULE FourierTransformTest
#include <boost/test/unit_test.hpp>

// For IO
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <unistd.h>

// For measuring elapsed time
#include <chrono>

// For Random Float Generator
#include <time.h>

// For object under test
#include "ofdmcodec.h"

#define FFT_DIFFERENCE_THRESHOLD 0.0000000000001

// Integration Tests 
BOOST_AUTO_TEST_SUITE(IntegrationTests)

/**
*  This test simulates entire encoding and decoding process
*  of only one symbol, this test does not transmit the data 
*  through a physical medium.
* 
*/
BOOST_AUTO_TEST_CASE(EncodeDecode)
{
    printf("Testing OFDM Coder Object...\n");

    // Initialize ofdm coder setting structs and objects

    OFDMSettings encoderSettings; 
    encoderSettings.type = FFTW_BACKWARD;
	encoderSettings.complexTimeSeries = false;
    encoderSettings.EnergyDispersalSeed = 0;
    encoderSettings.nPoints = 512; 
	encoderSettings.pilotToneStep = 16; 
    encoderSettings.pilotToneAmplitude = 2.0; 
    encoderSettings.guardInterval = 0; 
    encoderSettings.QAMSize = 4; 
    encoderSettings.cyclicPrefixSize = 128; 

    OFDMSettings decoderSettings = encoderSettings;
    decoderSettings.type = FFTW_FORWARD;

    double txData[encoderSettings.nPoints*2];
    double decoderOutput[encoderSettings.nPoints*2];

    OFDMCodec encoder(encoderSettings, txData);
    OFDMCodec decoder(decoderSettings, txData);

    // Setup random float generator
    srand( (unsigned)time( NULL ) );

    // Generate Array of random floats
    float RandomInputArray[encoderSettings.nPoints*2];
    for (uint16_t i = 0; i < encoderSettings.nPoints*2; i++)
    {
        RandomInputArray[i] = (float) rand()/RAND_MAX;
    }

    // Encode
    auto start = std::chrono::steady_clock::now();
    encoder.Encode(RandomInputArray);
    auto end = std::chrono::steady_clock::now();

    std::cout << "Encode elapsed time: "
    << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
    << " ns" << std::endl;

    // Decode
    start = std::chrono::steady_clock::now();
    decoder.Decode();
    end = std::chrono::steady_clock::now();

    std::cout << "Decode elapsed time: "
    << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
    << " ns" << std::endl;

    // Copy the Output of the decoder after QAM Demodulation
    decoder.QAMDemodulatorPlaceholder(decoderOutput);

    // Check the input and output are within threshold
    for (uint16_t i = 0; i < encoderSettings.nPoints*2; i++)
    {
        /*
        printf("Recovered Sample: %3d %+9.5f vs. %+9.5f\n",
        i, RandomInputArray[i], decoderOutput[i]);
        */

        // Check if real and complex element match within defined precision of each other
        BOOST_CHECK_MESSAGE( (std::abs(RandomInputArray[i] - decoderOutput[i])  <= FFT_DIFFERENCE_THRESHOLD), "Values vary more than threshold!" ); 
    }

}
BOOST_AUTO_TEST_SUITE_END()



