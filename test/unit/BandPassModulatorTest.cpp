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
#include "bandpass.h"

#define DIFFERENCE_THRESHOLD 0.0001


/**
* Test BANDPASS MODULATOR
* 
*/
BOOST_AUTO_TEST_SUITE(BAND_PASS_MODULATOR)


/**
* Generate random data (floats) Put it throguh IFFT.
* Copy the time domain samples into FFT input buffer.
* Execute and check the input and output are within a
* threshold value.
* 
*/
BOOST_AUTO_TEST_CASE(ModToDemod)
{
    printf("\nTesting Modulation to Demodulation...\n");
    printf("\nMdoulator:\n");

    uint16_t nPoints = 512;

    // Setup random float generator
    srand( (unsigned)time( NULL ) );

    // Generate Array of random floats
    double modulatorOutput[nPoints*2];

    fftw_complex *modulatorInput;
    fftw_complex *demodulatorOutput;

    modulatorInput = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * nPoints);
    demodulatorOutput = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * nPoints);

    for (uint16_t i = 0; i < nPoints; i++)
    {
        modulatorInput[i][0] = (float) rand()/RAND_MAX;
        modulatorInput[i][1] = (float) rand()/RAND_MAX;
    }

    BandPassModulator modulator(nPoints,  modulatorInput, modulatorOutput);
    BandPassModulator demodulator(nPoints, demodulatorOutput, modulatorOutput);

    // Measure wall time of the ifft execution.
    auto start = std::chrono::steady_clock::now();
    modulator.Modulate();
    auto end = std::chrono::steady_clock::now();
 
    std::cout << "Digital quadrature modulator elapsed time: "
        << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
        << " ns" << std::endl;

    printf("\nDemodulator:\n");

    // Measure wall time of the fft execution.
    start = std::chrono::steady_clock::now();
    demodulator.Demodulate();
    end = std::chrono::steady_clock::now();
 
    std::cout << "Digital quadrature demodulator elapsed time: "
        << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
        << " ns" << std::endl;


    // Print input and output buffers
    for (uint16_t i = 0; i < nPoints; i++)
    {
        
        /*
        printf("Sample: %3d %+9.5f j%+9.5f Input to Modulator vs. %+9.5f j%+9.5f Output of Demodulator\n",
        i, modulatorInput[i][0], modulatorInput[i][1], demodulatorOutput[i][0], demodulatorOutput[i][1]);
        */

        // Check if real and complex element match within defined precision.
        BOOST_CHECK_MESSAGE(
         ( (std::abs( modulatorInput[i][0] - demodulatorOutput[i][0] ) <= DIFFERENCE_THRESHOLD ) ||
         (  std::abs( modulatorInput[i][1] - demodulatorOutput[i][1] ) <= DIFFERENCE_THRESHOLD )), 
         "Values vary more than threshold! - Occured at index: " << i );  
        
    }

    fftw_free(modulatorInput); 
    fftw_free(demodulatorOutput);
    fftw_cleanup();
}

BOOST_AUTO_TEST_SUITE_END()
