#define BOOST_TEST_MODULE FourierTransformTest
#include <boost/test/unit_test.hpp>

// For IO
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


#define DIFFERENCE_THRESHOLD 0.0001


template<typename T, typename A> void gietec(const std::vector<T,A>  &input )
{

    size_t variableTypeSize = sizeof(T);
    std::cout << "Encode variableTypeSize =  " << variableTypeSize; 

}


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

    size_t nPoints = 512;
    size_t symbolSize = nPoints*2;
    size_t pilotToneStep = 16;
    size_t energyDispersalSeed = 10;
    size_t QAM = 4;

    // Setup random float generator
    srand( (unsigned)time( NULL ) );

    std::vector<int> intarr(10);
    //intarr.resize(10);

    DoubleVec ifftOutput;
    ifftOutput.resize(symbolSize);

    DoubleVec modulatorOutput;
    modulatorOutput.resize(symbolSize);
    
    //fftw_complex *iFFTInput;
    //iFFTInput = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * nPoints);
 
    // Popilate ifft output double vec with random values
    /*
    for (size_t i = 0; i < symbolSize; i++)
    {
        ifftOutput[i] = (double) rand()/RAND_MAX;
    }
    
    */

    QamModulator modulator(nPoints, pilotToneStep, energyDispersalSeed, QAM);

    modulator.Encode(intarr, ifftOutput);

    /*
    // Measure wall time of the modulator execution.
    auto start = std::chrono::steady_clock::now();
    // Modulate data, set prefix size to 0 as it is not used in this test
    modulator.Modulate(modulatorOutput, 0);
    auto end = std::chrono::steady_clock::now();
 
    std::cout << "QAM modulator elapsed time: "
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
 
    std::cout << "QAM demodulator elapsed time: "
        << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
        << " ns" << std::endl;
    

    fftw_free(iFFTInput);
    */
}

BOOST_AUTO_TEST_SUITE_END()
