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
*  This test simulates entire encoding and decoding process.
*  Only one symbol, this test does not transmit the data 
*  through a physical medium.
* 
*/
BOOST_AUTO_TEST_CASE(EncodeDecode)
{
    printf("Testing OFDM Coder Object...\n");
    //BOOST_TEST_MESSAGE( "Testing:" );
    //BOOST_TEST_MESSAGE( "Variable:" << variable );
    uint16_t nPoints = 512;
    bool complexTimeSeries = false;
    uint16_t pilotToneStep = 16;
    float pilotToneAmplitude = 2.0;
    uint16_t qamSize = 4;

    // Initialize ofdm coder objects
    OFDMCodec encoder(FFTW_BACKWARD, nPoints, complexTimeSeries, pilotToneStep, pilotToneAmplitude, qamSize);
    OFDMCodec decoder(FFTW_FORWARD, nPoints, complexTimeSeries, pilotToneStep, pilotToneAmplitude, qamSize);

    // Setup random float generator
    srand( (unsigned)time( NULL ) );

    // Generate Array of random floats
    float RandomInputArray[nPoints*2];
    for (uint16_t i = 0; i < nPoints*2; i++)
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

    // Copy the encoded output
    for (uint16_t i = 0; i < nPoints; i++)
    {
        decoder.m_fft.in[i][0] = encoder.m_fft.out[i][0];
        decoder.m_fft.in[i][1] = encoder.m_fft.out[i][1];
    }

    // Decode
    start = std::chrono::steady_clock::now();
    decoder.Decode();
    end = std::chrono::steady_clock::now();

    std::cout << "Decode elapsed time: "
    << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
    << " ns" << std::endl;


    // Normalize 
    start = std::chrono::steady_clock::now();
    for (uint16_t i = 0; i < nPoints; i++)
    {
        decoder.m_fft.out[i][0] *= 1./nPoints;
        decoder.m_fft.out[i][1] *= 1./nPoints;
    }
    end = std::chrono::steady_clock::now();

    std::cout << "Normalize elapsed time: "
    << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
    << " ns" << std::endl;


    // Print input and output buffers
    for (uint16_t i = 0; i < nPoints; i++)
    {
        //printf("Recovered Sample: %3d %+9.5f j%+9.5f Input to ifft vs. %+9.5f j%+9.5f Output of IFFT\n",
        //i, encoder.m_fft.in[i][0], encoder.m_fft.in[i][1], decoder.m_fft.out[i][0], decoder.m_fft.out[i][1]);

        // Check if real and complex element match within defined precision of each other
        BOOST_CHECK_MESSAGE(
         ( (std::abs( encoder.m_fft.in[i][0] - decoder.m_fft.out[i][0] ) <= FFT_DIFFERENCE_THRESHOLD ) ||
         (  std::abs( encoder.m_fft.in[i][1] - decoder.m_fft.out[i][1] ) <= FFT_DIFFERENCE_THRESHOLD )), 
         "Values vary more than threshold!" );   
    }

}
BOOST_AUTO_TEST_SUITE_END()



