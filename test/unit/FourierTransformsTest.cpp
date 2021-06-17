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
#include "ofdmfft.h"


// Test FFT & IFFT
BOOST_AUTO_TEST_SUITE(FOURIER_TRANSFORMS)

/**
* 
* 
*/
BOOST_AUTO_TEST_CASE(FFTtoIFFT)
{
    // Setup timers
    /*
    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::duration;
    using std::chrono::milliseconds;
    */
    printf("Testing FFT to IFFT...");
    printf("\nFourier transform:\n");

    // Setup random float generator
    srand( (unsigned)time( NULL ) );

    uint16_t nPoints = 512;

    ofdmFFT forwardfft(nPoints, FFTW_FORWARD);
    ofdmFFT backwardfft(nPoints, FFTW_BACKWARD);

    // Generate Cosine wave in time domain
    /*
    for (uint16_t i = 0; i < nPoints; i++)
    {
        forwardfft.in[i][0] = cos(3 * 2*M_PI*i/nPoints);
        forwardfft.in[i][1] = 0;
    }
    */

    // Generate random floats 
    for (uint16_t i = 0; i < nPoints; i++)
    {
        forwardfft.in[i][0] = (float) rand()/RAND_MAX;
        forwardfft.in[i][1] = (float) rand()/RAND_MAX;
    }

    auto start = std::chrono::steady_clock::now();
    forwardfft.Execute();
    auto end = std::chrono::steady_clock::now();
 
    std::cout << "forwardfft Elapsed time in nanoseconds: "
        << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
        << " ns" << std::endl;

    for (uint16_t i = 0; i < nPoints; i++)
    {
        printf("freq: %3d %+9.5f j%+9.5f \n", i, forwardfft.out[i][0], forwardfft.out[i][1]);
    }
  
  
    printf("\nInverse Foruier Transform:\n");
    // Assign the output of forward fft to the input of backward fft
    for (uint16_t i = 0; i < nPoints; i++)
    {
        backwardfft.in[i][0] = forwardfft.out[i][0];
        backwardfft.in[i][1] = forwardfft.out[i][1];
    }

    start = std::chrono::steady_clock::now();
    backwardfft.Execute();
    end = std::chrono::steady_clock::now();
 
    std::cout << "backwardfft Elapsed time in nanoseconds: "
        << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
        << " ns" << std::endl;

    // Normalize 
    for (uint16_t i = 0; i < nPoints; i++)
    {
        backwardfft.out[i][0] *= 1./nPoints;
        backwardfft.out[i][1] *= 1./nPoints;
    }

    // Print input and output buffers
    for (uint16_t i = 0; i < nPoints; i++)
    {
        printf("Recovered Sample(time domain) Amplitude: %3d %+9.5f j%+9.5f Input to fft vs. %+9.5f j%+9.5f Output of IFFT\n",
        i, forwardfft.in[i][0], forwardfft.in[i][1], backwardfft.out[i][0], backwardfft.out[i][1]);

        // Check if real and complex element match

        /*BOOST_CHECK_MESSAGE(
         (forwardfft.in[i][0] == backwardfft.out[i][0]) ||
         (forwardfft.in[i][1] == backwardfft.out[i][1]), 
         "Value mismatch: " );   
        */

    }

    //fftw_destroy_plan(q);
    fftw_cleanup();
  
    //BOOST_TEST_MESSAGE( "Testing" );
}
BOOST_AUTO_TEST_SUITE_END()
