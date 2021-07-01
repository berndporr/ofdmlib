/**
* @file bandpass.cpp
* @author Kamil Rog
*
* 
*/

#include "bandpass.h"

/**
* Configures the band pass modulator/demodulator object 
* by setting the number of expected points and pointers 
* to appropriate buffers.
* 
* @param nPoints 
* 
* @return 0 on success, else error number
*
*/
int BandPassModulator::Configure(uint16_t fftPoints, fftw_complex *pComplex,  double *pDouble)
{   
    // Set variables
    nPoints = fftPoints;
    complexBuffer = pComplex;
    doubleBuffer = pDouble;
    configured = 1;
    return 0;
}

/**
* Sets the buffer pointers to null, variables and flasgs to zero
* 
* @return 0 on success, else error number
*
*/    
int BandPassModulator::Close()
{   
    complexBuffer = nullptr;
    doubleBuffer = nullptr;    
    nPoints = 0;
    configured = 0;
    return 0;
}


/**
* Modulates the output of IFFT buffer by upsampling
* at factor 2 and interleaving the I and Q signals
* pointers to null 
* 
* @return 0 on success, else error number
* 
*/   
int BandPassModulator::Modulate()
{
    // If the nPoints is even
    if(nPoints % 2 == 0)
    {      
        int j = 0;
        // Initialize double buffer counter
        for(int i = 0; i < nPoints; i+=2)
        {
            // Copy first first sample's real and img respectivley
            doubleBuffer[j]   = complexBuffer[i][0];
            doubleBuffer[j+1] = complexBuffer[i][1];
            // Copy and the minus real and img respectivley of next the sample
            doubleBuffer[j+2] = -complexBuffer[i+1][0];
            doubleBuffer[j+3] = -complexBuffer[i+1][1];
            // Increment double buffer counter
            j += 4;
        }
    }
    // nPoints must be odd 
    else
    {
        // Initialize +1 / -1 multiplier
        int s = 1;
        // Initialize double buffer counter
        int j = 0;
        // For each IFFT sample
        for(int i = 0; i < nPoints; i++) 
        {
            // Copy the real part of the sample and multiply by s factor
            doubleBuffer[j]   = s * complexBuffer[i][0];
            // Increment double buffer counter
            j++;
            // Copy the imag part of the sample and multiply by s factor
            doubleBuffer[j+1] = s * complexBuffer[i][1];
            // Increment double buffer counter
            j++;
            // Compute factor for next sample
            s *= -1;
        }
    }
    return 0;
}


/**
* Demodulates the Rx Samples into the complex fft input buffer
* 
* @return 0 on success, else error number
*
*/   
int BandPassModulator::Demodulate()
{
    // If the nPoints is even 
    if(nPoints % 2 == 0)
    {
        // Initialize double buffer counter
        int j = 0;
        // For each expected FFT sample point
        for (int i = 0; i < nPoints; i += 2)
        {   
            // Copy first first sample's real and img respectivley
            complexBuffer[i][0] = doubleBuffer[j];
            complexBuffer[i][1] = doubleBuffer[j+1];
            // Copy and the minus real and img respectivley of next the sample
            complexBuffer[i+1][0] = -doubleBuffer[j+2];
            complexBuffer[i+1][1] = -doubleBuffer[j+3];
            // Increment double buffer counter
            j += 4;
        }
    }
    // nPoints must be odd 
    else
    {
        // Initialize +1 / -1 multiplier
        int s = 1;
        // Initialize double buffer counter
        int j = 0;
        // For each expected FFT sample point
        for(int i = 0; i < nPoints; i++) 
        {
            // Copy the real part of the sample and multiply by s factor
            complexBuffer[i][0] = s * doubleBuffer[j];
            // Increment double buffer counter
            j++;
            // Copy the imag part of the sample and multiply by s factor
            complexBuffer[i][1] = s * doubleBuffer[j+1];
            // Increment double buffer counter
            j++;
            // Compute factor for next sample
            s *= -1;
        }
    }
    return 0;
}
