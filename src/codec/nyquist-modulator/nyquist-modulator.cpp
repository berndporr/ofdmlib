/**
* @file nyquist-modulator.cpp
* @author Kamil Rog
*
* 
*/


#include "nyquist-modulator.h"
#include <cstddef>


/**
* Constructor
* 
* @param settings
*
*/
NyquistModulator::NyquistModulator(OFDMSettings &settings) :
    m_ofdmSettings(settings)
{

}


/**
* Destructor 
* Runs close function.
*
*/
NyquistModulator::~NyquistModulator()
{

}


/**
* Modulates the output of IFFT buffer by upsampling
* at factor 2 and interleaving the I and Q signals.
* This Function assumes the ifftOutput point to the first
* element of the symbol.
* 
* @param ifftOutput pointer to the IFFT output buffer, this must point to the first element
*
* @return 0 on success, else error number
* 
*/  
void NyquistModulator::Modulate(double *ifftOutput)
{
    
    size_t symbolSize = m_ofdmSettings.nFFTPoints * 2;
    // If the nPoints is even
    if( (m_ofdmSettings.nFFTPoints % 2) == 0)
    {      
        // Initialize double buffer counter
        // For each every other real img pair skipping first fft point which remains unchanged
        for(size_t i = 2; i < symbolSize; i+=4)
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
        for(size_t i = 0; i < m_ofdmSettings.nFFTPoints; i++) 
        {
            // Copy the real part of the sample and multiply by s factor
            ifftOutput[j] *= s;
            // Increment double buffer counter
            j++;
            // Copy the imag part of the sample and multiply by s factor
            ifftOutput[j] *= s;
            // Increment double buffer counter
            j++;
            // Compute factor for next sample
            s *= -1;
        }
    }
}


/**
* Demodulates the Rx Samples into the complex fft input buffer
* This function combines the interleaved real and imaginary samples
* into an input buffer of the fft transform.
*
* @param rxBuffer Raw Rx Samples
* @param pFFTInput Pointer to the FFT input buffer
* @param symbolStart Start of the actual first sample of the symbol in the Rx signal buffer. 
*
*/   
void NyquistModulator::Demodulate(const double *rxBuffer, fftw_complex *pFFTInput, const size_t symbolStart)
{
    // If the nPoints is even
    if(m_ofdmSettings.nFFTPoints % 2 == 0)
    {
        // Initialize double buffer counter
        size_t j = symbolStart;
        // For each expected FFT sample point
        for (size_t i = 0; i < m_ofdmSettings.nFFTPoints; i += 2)
        {
            // Copy first sample's real and img respectivley
            pFFTInput[i][0] = rxBuffer[j];
            j++;
            pFFTInput[i][1] = rxBuffer[j];
            j++;
            // Copy and the minus real and img respectivley of next the sample
            pFFTInput[i+1][0] = -rxBuffer[j];
            j++;
            pFFTInput[i+1][1] = -rxBuffer[j];
            j++;
        }
    }
    // nPoints must be odd
    else
    {
        // Initialize +1 / -1 multiplier
        int s = 1;
        // Initialize double buffer counter
        size_t j = symbolStart;
        // For each expected FFT sample point
        for(size_t i = 0; i < m_ofdmSettings.nFFTPoints; i++)
        {
            // Copy the real part of the sample and multiply by s factor
            pFFTInput[i][0] = s * rxBuffer[j];
            // Increment double buffer counter
            j++;
            // Copy the imag part of the sample and multiply by s factor
            pFFTInput[i][1] = s * rxBuffer[j];
            // Increment double buffer counter
            j++;
            // Compute factor for next sample
            s *= -1;
        }
    }
}