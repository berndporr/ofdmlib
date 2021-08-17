#define BOOST_TEST_MODULE NyquistModulatorTest
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

    // OFDM Codec Settings
    OFDMSettings encoderSettings;
    encoderSettings.type = FFTW_BACKWARD;
    encoderSettings.EnergyDispersalSeed = 10;
    encoderSettings.nFFTPoints = 1024; 
    encoderSettings.PilotToneDistance = 16; 
    encoderSettings.PilotToneAmplitude = 2.0; 
    encoderSettings.QAMSize = 2; 
    encoderSettings.PrefixSize = (size_t) ((encoderSettings.nFFTPoints*2)/4); // 1/4th of symbol

    OFDMSettings decoderSettings = encoderSettings;
    decoderSettings.type = FFTW_FORWARD;
    size_t symbolSize = encoderSettings.nFFTPoints*2;

    // Setup random float generator
    srand( (unsigned)time( NULL ) );

    double *ifftOutput = (double*) calloc(symbolSize, sizeof(double));
    double *modulatorOutput = (double*) calloc(symbolSize, sizeof(double));
    double *rxSignal = (double*) calloc(symbolSize*10, sizeof(double));

    size_t symbolStart = rand() % ((symbolSize)*9);
    printf("Random Symbol Start = %lu\n",symbolStart);

    fftw_complex *demodulatorOutput = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * decoderSettings.nFFTPoints);
 
    // Popilate ifft output double vec with random values
    for (size_t i = 0; i < symbolSize; i++)
    {
        ifftOutput[i] = (double) rand()/RAND_MAX;
    }

    memcpy(&modulatorOutput[0], &ifftOutput[0], sizeof(double)*symbolSize);

    NyquistModulator modulator(encoderSettings); 
    NyquistModulator demodulator(decoderSettings);

    // Measure wall time of the modulator execution.
    auto start = std::chrono::steady_clock::now();
    // Modulate data, set prefix size to 0 as it is not used in this test
    modulator.Modulate(modulatorOutput);
    auto end = std::chrono::steady_clock::now();
 
    std::cout << "Digital quadrature modulator elapsed time: "
    << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
    << " ns" << std::endl;

    printf("\nDemodulator:\n");

    // basically it works like this:
    //std::copy( src, src + size, dest );
    memcpy(&rxSignal[symbolStart], &modulatorOutput[0], sizeof(double)*symbolSize);

    // Measure wall time of the fft execution.
    start = std::chrono::steady_clock::now();
    demodulator.Demodulate(rxSignal, demodulatorOutput, symbolStart);
    end = std::chrono::steady_clock::now();
 
    std::cout << "Digital quadrature demodulator elapsed time: "
    << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
    << " ns" << std::endl;


    // Print input and output buffers
    for (size_t i = 0; i < encoderSettings.nFFTPoints; i++)
    {      
        //printf("Real Sample: %lu %+9.10f Input to Modulator vs. %+9.10f Output of Demodulator\n", i, ifftOutput[(i*2)], demodulatorOutput[i][0]);

        // Check if real and complex element match within defined precision.
        //BOOST_CHECK_MESSAGE( (std::abs( ifftOutput[(i*2)] -  demodulatorOutput[i][0] ) <= DIFFERENCE_THRESHOLD ), 
        //"Values vary more than threshold! - Occured at index: " << i );  

        // Check if real and complex element match within defined precision.
        BOOST_CHECK_MESSAGE( (ifftOutput[(i*2)] == demodulatorOutput[i][0] ), 
        "Values vary more than threshold! - Occured at index: " << i );  
        
        //printf("Imag Sample: %lu %+9.10f Input to Modulator vs. %+9.10f Output of Demodulator\n", i, ifftOutput[(i*2)+1], demodulatorOutput[i][1]);

        // Check if real and complex element match within defined precision.
        //BOOST_CHECK_MESSAGE( (std::abs( ifftOutput[(i*2)+1] - demodulatorOutput[i][1] ) <= DIFFERENCE_THRESHOLD ), 
        //"Values vary more than threshold! - Occured at index: " << i );  

        // Check if real and complex element match within defined precision.
        BOOST_CHECK_MESSAGE( (ifftOutput[(i*2)+1] == demodulatorOutput[i][1] ), 
        "Values vary more than threshold! - Occured at index: " << i );  
    }
}

BOOST_AUTO_TEST_SUITE_END()
