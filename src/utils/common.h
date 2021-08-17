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

struct OFDMSettings
{
    int type; 
    size_t EnergyDispersalSeed; 
    size_t nFFTPoints;              // Total number of FFT & IFFT coefficients 
	size_t PilotToneDistance;       // The distance between the pilot tones in a symbol
    double PilotToneAmplitude;      // The amplitude of the pilot tones
    size_t PrefixSize;              // Cyclic prefix size in time-domain samples
    size_t QAMSize;
    // InputDataType  
};


struct DetectorSettings
{
    size_t Threshold;
    size_t FineSearchRange; 
};


// Maths Functions //



// Plotting Functions // 


#endif
