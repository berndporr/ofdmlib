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
* Generate random data (doubles) which simulate output
* of ifft. Copy the time domain samples for every possible
* ring buffer location. Demodulate and check if the output and
* matches the input
* 
*/
BOOST_AUTO_TEST_CASE(ModToDemod)
{
    printf("\nTesting Modulation to Demodulation...\n");

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

    OFDMSettingsStruct decoderSettingsStruct = encoderSettingsStruct;
    decoderSettingsStruct.type = FFTW_FORWARD;

    OFDMSettings encoderSettings(encoderSettingsStruct);
    OFDMSettings decoderSettings(decoderSettingsStruct);

    NyquistModulator modulator(encoderSettings); 
    NyquistModulator demodulator(decoderSettings);

    // Setup random float generator
    srand( (unsigned)time( NULL ) );

    double *ifftOutput = (double*) calloc(encoderSettings.m_SymbolSize, sizeof(double));
    double *modulatorOutput = (double*) calloc(encoderSettings.m_SymbolSize, sizeof(double));
    double *rxSignal = (double*) calloc(modulator.m_RingBufferBoundary, sizeof(double)); // This simulates block ring buffer
    fftw_complex *demodulatorOutput = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * decoderSettings.m_nFFTPoints);
    
    // For every possible location the symbol can be pasted in
    for(size_t symbolStart = 0; symbolStart < modulator.m_RingBufferBoundary; symbolStart++)
    {   
        //std::cout << "Nyquist Mod -> Demod @ symbolStart = " << symbolStart << std::endl;

        // Popilate ifft output double vec with random values
        for (size_t i = 0; i < encoderSettings.m_SymbolSize; i++)
        {
            ifftOutput[i] = (double) rand()/RAND_MAX;
        }

        // Modulator //

        // Copy the data into buffer which is going to be manipulated by modulator
        memcpy(&modulatorOutput[0], &ifftOutput[0], sizeof(double)*encoderSettings.m_SymbolSize);

        // Measure wall time of the modulator execution.
        auto start = std::chrono::steady_clock::now();
        modulator.Modulate(modulatorOutput);
        auto end = std::chrono::steady_clock::now();


        /*
        std::cout << "Digital quadrature modulator elapsed time: "
        << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
        << " ns" << std::endl;
        */

        // Demodulator //

        // Copy the symbol somewhere in the ring buffer,
        // Check if the index suggests wrapping around the buffer is needed
        if(modulator.m_RingBufferBoundary - symbolStart > encoderSettings.m_SymbolSize)
        {
            memcpy(&rxSignal[symbolStart], &modulatorOutput[0], sizeof(double)*encoderSettings.m_SymbolSize);
        }
        // wrap the object around
        else
        {
            // Calculate the number elements of the symbol untill boundary of the ring buffer
            size_t nToBoundary = modulator.m_RingBufferBoundary - symbolStart;
            // Copy samples
            memcpy(&rxSignal[symbolStart], &modulatorOutput[0], nToBoundary*sizeof(double));
            // Calculate the number elements of the symbol over boundary of the ring buffer
            size_t nOverBoundary = encoderSettings.m_SymbolSize - nToBoundary;
            // Copy samples
            memcpy(&rxSignal[0], &modulatorOutput[nToBoundary], nOverBoundary*sizeof(double));
        }

        // Measure wall time of the demodulator execution.
        start = std::chrono::steady_clock::now();
        demodulator.Demodulate(rxSignal, demodulatorOutput, symbolStart);
        end = std::chrono::steady_clock::now();

        /*
        std::cout << "Digital quadrature demodulator elapsed time: "
        << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
        << " ns" << std::endl;
        */

        // Print input and output buffer
        for (size_t i = 0; i < encoderSettings.m_nFFTPoints; i++)
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
}

BOOST_AUTO_TEST_SUITE_END()
