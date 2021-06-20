#define BOOST_TEST_MODULE FourierTransformTest
#include <boost/test/unit_test.hpp>

// For IO
#include <stdlib.h>
#include <math.h>
#include <cmath> 
#include <iostream>
#include <unistd.h>

// For measuring elapsed time
#include <chrono>

// For Random Float Generator
#include <time.h>

// For object under test
#include "ofdmfft.h"

#define FFT_DIFFERENCE_THRESHOLD 0.0000000000001

// Test FFT & IFFT
BOOST_AUTO_TEST_SUITE(FOURIER_TRANSFORMS)

/**
* 
* 
*/
BOOST_AUTO_TEST_CASE(FFTtoIFFT)
{
    printf("\nTesting FFT to IFFT...\n");
    printf("\nFourier transform:\n");

    // Setup random float generator
    srand( (unsigned)time( NULL ) );

    uint16_t nPoints = 512;

    ofdmFFT forwardfft(nPoints, FFTW_FORWARD);
    ofdmFFT backwardfft(nPoints, FFTW_BACKWARD);

    // Generate random floats 
    for (uint16_t i = 0; i < nPoints; i++)
    {
        forwardfft.in[i][0] = (float) rand()/RAND_MAX;
        forwardfft.in[i][1] = (float) rand()/RAND_MAX;
    }

    auto start = std::chrono::steady_clock::now();
    forwardfft.ComputeTransform();
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
    backwardfft.ComputeTransform();
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

        // Check if real and complex element match within defined precision of each other
        BOOST_CHECK_MESSAGE(
         ( (std::abs( forwardfft.in[i][0] - backwardfft.out[i][0] ) <= FFT_DIFFERENCE_THRESHOLD ) ||
         (  std::abs( forwardfft.in[i][1] - backwardfft.out[i][1] ) <= FFT_DIFFERENCE_THRESHOLD )), 
         "Values vary more than threshold!" );  

    }
    fftw_cleanup();
}


BOOST_AUTO_TEST_CASE(FFTtoIFFTCosine)
{
    printf("\nTesting FFT to IFFT...\n");
    printf("\nFourier transform:\n");

    // Setup random float generator
    srand( (unsigned)time( NULL ) );

    uint16_t nPoints = 512;

    ofdmFFT forwardfft(nPoints, FFTW_FORWARD);
    ofdmFFT backwardfft(nPoints, FFTW_BACKWARD);

    // Generate Cosine wave in time domain
    
    for (uint16_t j = 0; j < nPoints; j++)
    {
        forwardfft.in[j][0] = cos(3 * 2*M_PI*j/nPoints);
        forwardfft.in[j][1] = 0; 
    }
    auto start = std::chrono::steady_clock::now();
    forwardfft.ComputeTransform();
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
    backwardfft.ComputeTransform();
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

        // Check if real and complex element match within defined precision of each other
        BOOST_CHECK_MESSAGE(
         ( (std::abs( forwardfft.in[i][0] - backwardfft.out[i][0] ) <= FFT_DIFFERENCE_THRESHOLD ) ||
         (  std::abs( forwardfft.in[i][1] - backwardfft.out[i][1] ) <= FFT_DIFFERENCE_THRESHOLD )), 
         "Values vary more than threshold!" );   
        

    }
    fftw_cleanup();
}


BOOST_AUTO_TEST_CASE(Reconfiguration)
{
    printf("\nTesting Object Reconfiguration...\n");
    printf("\nFourier transform:\n");

    // Setup random float generator
    srand( (unsigned)time( NULL ) );

    uint16_t nPoints = 1;

    ofdmFFT forwardfft(nPoints, FFTW_FORWARD);
    ofdmFFT backwardfft(nPoints, FFTW_BACKWARD);

    int PowOf2Arr[] = { 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384 };
    int nTestSizes = sizeof(PowOf2Arr) / sizeof(PowOf2Arr[0]);

    for(uint16_t j = 0; j < nTestSizes; j++)
    {
        printf("\nTesting %d Point FFT\n",PowOf2Arr[j]);
        forwardfft.Configure(PowOf2Arr[j], FFTW_FORWARD);
        backwardfft.Configure(PowOf2Arr[j], FFTW_BACKWARD);
        // Generate random floats 

        for (uint16_t i = 0; i < PowOf2Arr[j]; i++)
        {
            forwardfft.in[i][0] = (float) rand()/RAND_MAX;
            forwardfft.in[i][1] = (float) rand()/RAND_MAX;
        }

        auto start = std::chrono::steady_clock::now();
        forwardfft.ComputeTransform();
        auto end = std::chrono::steady_clock::now();
    
        std::cout << "forwardfft Elapsed time in nanoseconds: "
            << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
            << " ns" << std::endl;

        // Print Data output of forward FFT
        /*
        for (uint16_t i = 0; i < PowOf2Arr[j]; i++)
        {
            printf("freq: %3d %+9.5f j%+9.5f \n", i, forwardfft.out[i][0], forwardfft.out[i][1]);
        }
        */
    
        printf("\nInverse Foruier Transform:\n");
        // Assign the output of forward fft to the input of backward fft
        for (uint16_t i = 0; i < PowOf2Arr[j]; i++)
        {
            backwardfft.in[i][0] = forwardfft.out[i][0];
            backwardfft.in[i][1] = forwardfft.out[i][1];
        }

        start = std::chrono::steady_clock::now();
        backwardfft.ComputeTransform();
        end = std::chrono::steady_clock::now();
    
        std::cout << "backwardfft Elapsed time in nanoseconds: "
            << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
            << " ns" << std::endl;

        // Normalize 
        for (uint16_t i = 0; i < PowOf2Arr[j]; i++)
        {
            backwardfft.out[i][0] *= 1./PowOf2Arr[j];
            backwardfft.out[i][1] *= 1./PowOf2Arr[j];
        }

        // Print input and output buffers
        for (uint16_t i = 0; i < PowOf2Arr[j]; i++)
        {
            //printf("Recovered Sample(time domain) Amplitude: %3d %+9.5f j%+9.5f Input to fft vs. %+9.5f j%+9.5f Output of IFFT\n",
            //i, forwardfft.in[i][0], forwardfft.in[i][1], backwardfft.out[i][0], backwardfft.out[i][1]);

            // Check if real and complex element match within defined precision of each other
            BOOST_CHECK_MESSAGE(
            ( (std::abs( forwardfft.in[i][0] - backwardfft.out[i][0] ) <= FFT_DIFFERENCE_THRESHOLD ) ||
            (  std::abs( forwardfft.in[i][1] - backwardfft.out[i][1] ) <= FFT_DIFFERENCE_THRESHOLD )), 
            "Values vary more than threshold!" );  

        }
    }
    fftw_cleanup();
}
BOOST_AUTO_TEST_SUITE_END()
