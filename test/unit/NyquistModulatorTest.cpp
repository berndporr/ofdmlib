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
#include "nyquist-modulator.h"
#include "fftw3.h"

#define DIFFERENCE_THRESHOLD 0.0001


/**
* Test NYQUIST MODULATOR
* 
*/
BOOST_AUTO_TEST_SUITE(NYQUIST_MODULATOR)


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

    uint32_t nPoints = 512;
    uint32_t symbolSize = nPoints*2;
    //uint32_t prefixSize = 128;

    // Setup random float generator
    srand( (unsigned)time( NULL ) );

    DoubleVec ifftOutput;
    ifftOutput.resize(symbolSize);

    DoubleVec modulatorOutput;
    modulatorOutput.resize(symbolSize);

    DoubleVec rxSignal;
    rxSignal.resize(symbolSize*10);

    uint32_t symbolStart = rand() % ((nPoints*2)*9);
    printf("Random Symbol Start = %d\n",symbolStart);
    
    fftw_complex *demodulatorOutput;
    demodulatorOutput = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * nPoints);
 
    // Popilate ifft output double vec with random values
    for (size_t i = 0; i < symbolSize; i++)
    {
        ifftOutput[i] = (double) rand()/RAND_MAX;
    }

    std::copy(ifftOutput.begin(), ifftOutput.begin()+symbolSize, modulatorOutput.begin());
  
    NyquistModulator modulator(nPoints, (fftw_complex *) &modulatorOutput);
    NyquistModulator demodulator(nPoints, demodulatorOutput); 

    // Measure wall time of the modulator execution.
    auto start = std::chrono::steady_clock::now();
    // Modulate data, set prefix size to 0 as it is not used in this test
    modulator.Modulate(modulatorOutput, 0);
    auto end = std::chrono::steady_clock::now();
 
    std::cout << "Digital quadrature modulator elapsed time: "
        << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
        << " ns" << std::endl;

    printf("\nDemodulator:\n");

    // basically it works like this:
    //std::copy( src, src + size, dest );
    std::copy( modulatorOutput.begin(), modulatorOutput.end(), rxSignal.begin()+symbolStart);

    // Measure wall time of the fft execution.
    start = std::chrono::steady_clock::now();
    demodulator.Demodulate(rxSignal, symbolStart);
    end = std::chrono::steady_clock::now();
 
    std::cout << "Digital quadrature demodulator elapsed time: "
        << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
        << " ns" << std::endl;


    // Print input and output buffers
    for (size_t i = 0; i < nPoints; i++)
    {
                
        //printf("Real Sample: %lu %+9.5f Input to Modulator vs. %+9.5f Output of Demodulator\n",
        //i, ifftOutput[(i*2)], demodulatorOutput[i][0]);

        // Check if real and complex element match within defined precision.
        BOOST_CHECK_MESSAGE( 
        (std::abs( ifftOutput[(i*2)] -  demodulatorOutput[i][0] ) <= DIFFERENCE_THRESHOLD ), 
        "Values vary more than threshold! - Occured at index: " << i );  
        

        //printf("Imag Sample: %lu %+9.5f Input to Modulator vs. %+9.5f Output of Demodulator\n",
        //i, ifftOutput[(i*2)+1], demodulatorOutput[i][1]);

        // Check if real and complex element match within defined precision.
        BOOST_CHECK_MESSAGE( 
        (std::abs( ifftOutput[(i*2)+1] - demodulatorOutput[i][1] ) <= DIFFERENCE_THRESHOLD ), 
        "Values vary more than threshold! - Occured at index: " << i );  
    
    }
}

BOOST_AUTO_TEST_SUITE_END()
