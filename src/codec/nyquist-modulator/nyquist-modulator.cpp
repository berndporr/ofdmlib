/**
* @file nyquist-modulator.cpp
* @author Kamil Rog
*
* 
*/

#include "nyquist-modulator.h"
#include <cstddef>

/**
* Configures the nyquist modulator/demodulator object 
* by setting the number of expected points and pointers 
* to appropriate buffers.
* 
* @param nPoints 
* 
* @return 0 on success, else error number
*
*/
int NyquistModulator::Configure(size_t fftPoints, fftw_complex *pComplex)
{   
    // Set variables
    m_nPoints = fftPoints;
    pComplexBuffer = pComplex;
    m_configured = 1;
    return 0;
}


/**
* Sets the buffer pointers to null, variables and flasgs to zero
* 
* @return 0 on success, else error number
*
*/    
int NyquistModulator::Close()
{   
    m_nPoints = 0;
    m_configured = 0;
    pComplexBuffer = nullptr;
    return 0;
}


/**
* Modulates the output of IFFT buffer by upsampling
* at factor 2 and interleaving the I and Q signals
* 
* @return 0 on success, else error number
* 
*/   
/*
int NyquistModulator::Modulate(DoubleVec &vectorBuffer)
{
    // If the nPoints is even
    if(nPoints % 2 == 0)
    {      
        int j = 0;
        // Initialize double buffer counter
        for(size_t i = 0; i < nPoints; i+=2)
        {
            // Copy first first sample's real and img respectivley
            vectorBuffer[j]   = complexBuffer[i][0];
            vectorBuffer[j+1] = complexBuffer[i][1];
            // Copy and the minus real and img respectivley of next the sample
            vectorBuffer[j+2] = -complexBuffer[i+1][0];
            vectorBuffer[j+3] = -complexBuffer[i+1][1];
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
        for(size_t i = 0; i < nPoints; i++) 
        {
            // Copy the real part of the sample and multiply by s factor
            vectorBuffer[j]   = s * complexBuffer[i][0];
            // Increment double buffer counter
            j++;
            // Copy the imag part of the sample and multiply by s factor
            vectorBuffer[j] = s * complexBuffer[i][1];
            // Increment double buffer counter
            j++;
            // Compute factor for next sample
            s *= -1;
        }
    }
    return 0;
}
*/


/**
* Modulates the output of IFFT buffer by upsampling
* at factor 2 and interleaving the I and Q signals 
* 
* @param ifftOutput reference to the IFFT output buffer
*
* @return 0 on success, else error number
* 
*/  
void NyquistModulator::Modulate(DoubleVec &ifftOutput, const size_t prefixSize)
{
    // If the nPoints is even
    if( (m_nPoints % 2) == 0)
    {      
        // Initialize double buffer counter
        // For each every other real img pair skipping first fft point
        for(size_t i = prefixSize+2; i < prefixSize+(m_symbolSize); i+=4)
        {
            // Negate the value
            ifftOutput[i] = -ifftOutput[i];
            ifftOutput[i+1] = -ifftOutput[i+1];
        }
    }
    // nPoints must be odd 
    else
    {
        // Initialize +1 / -1 multiplier
        int s = 1;
        // Initialize double buffer counter
        size_t j = 0;
        for(size_t i = 0; i < m_nPoints; i++) 
        {
            // Copy the real part of the sample and multiply by s factor
            ifftOutput[j]   = s * ifftOutput[j];
            // Increment double buffer counter
            j++;
            // Copy the imag part of the sample and multiply by s factor
            ifftOutput[j] = s * ifftOutput[j];
            // Increment double buffer counter
            j++;
            // Compute factor for next sample
            s *= -1;
        }
    }
}


/**
* Demodulates the Rx Samples into the complex fft input buffer
*
* @param offset Points to start of the symbol in the Rx signal buffer. 
* 
* @return 0 on success, else error number
*
*/   
void NyquistModulator::Demodulate(const DoubleVec &vectorBuffer, size_t offset)
{
    // If the nPoints is even
    if(m_nPoints % 2 == 0)
    {
        // Initialize double buffer counter
        size_t j = offset;
        // For each expected FFT sample point
        for (size_t i = 0; i < m_nPoints; i += 2)
        {   
            // Copy first first sample's real and img respectivley
            pComplexBuffer[i][0] = vectorBuffer[j];
            j++;
            pComplexBuffer[i][1] = vectorBuffer[j];
            j++;
            // Copy and the minus real and img respectivley of next the sample
            pComplexBuffer[i+1][0] = -vectorBuffer[j];
            j++;
            pComplexBuffer[i+1][1] = -vectorBuffer[j];
            j++;
        }
    }
    // nPoints must be odd
    else
    {
        // Initialize +1 / -1 multiplier
        int s = 1;
        // Initialize double buffer counter
        size_t j = offset;
        // For each expected FFT sample point
        for(size_t i = 0; i < m_nPoints; i++)
        {
            // Copy the real part of the sample and multiply by s factor
            pComplexBuffer[i][0] = s * vectorBuffer[j];
            // Increment double buffer counter
            j++;
            // Copy the imag part of the sample and multiply by s factor
            pComplexBuffer[i][1] = s * vectorBuffer[j];
            // Increment double buffer counter
            j++;
            // Compute factor for next sample
            s *= -1;
        }
    }
}