#ifndef COMMON_H
#define COMMON_H    
    
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>

// Data types
#include <cstring>
#include <vector>
#include <string.h>
#include <complex.h>

// Maths
#include <cmath>
#include <math.h>

// FFT
#include <fftw3.h>

// Plotting
#include <matplot/matplot.h>

using DoubleVec = std::vector<double>;
using ByteVec = std::vector<uint8_t>;
using SizeTVec = std::vector<size_t>;

/**
* Modulation schemes for the data sub-carriers 
* The variable is equivelent to the bits per symbol 
*/
typedef enum {
	QAM_4 =  2,
} MODULATION_SCHEME;


struct OFDMSettingsStruct
{
    int type; 
    size_t EnergyDispersalSeed; 
    size_t nFFTPoints;              // Total number of FFT & IFFT coefficients 
	size_t PilotToneDistance;       // The distance between the pilot tones in a symbol
    double PilotToneAmplitude;      // The amplitude of the pilot tones
    size_t PrefixSize;              // Cyclic prefix size in time-domain samples
    size_t QAMSize;
    size_t nDataBytesPerSymbol;
    // InputDataType  
};


struct DetectorSettings
{
    double UpperThreshold;
    double LowerThreshold;
    size_t nSamplesLowLimit;
    size_t FineSearchRange; 
};


// Maths Functions //



// Plotting Functions // 


#endif
