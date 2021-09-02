/**
* @file nyquist-modulator.cpp
* @author Kamil Rog
*
* 
*/


#include "nyquist-modulator.h"
#include <cstddef>

// Ring Buffer Increments

/**
* Increments the specified variable and wraps around
* if neccessary.
* 
* @param variable reference to the variable being incremented
*
* @param Limit the boundary upon reach the variable is set to zero
*
*/
void LimitIncrement(size_t &variable, size_t Limit)
{
    // Less than boundary
    if( (variable + 1) < Limit )
    {
        variable++;
    }
    // Wrap around
    else
    {
        variable = 0;
    }
}


/**
* Constructor
* 
* @param settings
*
*/
NyquistModulator::NyquistModulator(const OFDMSettings &settings) :
    m_ofdmSettings(settings)
{
    m_RingBufferBoundary = m_ofdmSettings.m_PrefixedSymbolSize*3;
}


/**
* Destructor 
* Runs close function.
*
*/
NyquistModulator::~NyquistModulator()
{
    free(m_TempBuffer);
}


/**
* Modulates the output of IFFT buffer by upsampling
* at factor 2 and interleaving the I and Q signals.
* This Function assumes the ifftOutput points to the first
* element of the symbol (complex-time series).
* 
* @param ifftOutput pointer to the IFFT output buffer, 
* this must point to the first element of the symbol
*
* @return 0 on success, else error number
* 
*/  
void NyquistModulator::Modulate(double *ifftOutput)
{

    // If the nPoints is even
    if( (m_ofdmSettings.m_nFFTPoints % 2) == 0)
    {      
        // Initialize double buffer counter
        // For each every other real img pair skipping first fft point which remains unchanged
        for(size_t i = 2; i < m_ofdmSettings.m_SymbolSize; i+=4)
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
        for(size_t i = 0; i < m_ofdmSettings.m_nFFTPoints; i++) 
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
* @param rxBuffer Band-pass rx samples
* @param pFFTInput Pointer to the FFT input buffer
* @param symbolStart Start of the actual first sample of the symbol in the Rx signal buffer. 
*
*/   
void NyquistModulator::Demodulate(const double *pRxBuffer, fftw_complex *pFFTInput, const size_t symbolStart)
{
    // Initialize double buffer counter
    size_t j = symbolStart;

    // If the nPoints is even
    if(m_ofdmSettings.m_nFFTPoints % 2 == 0)
    {
        // For each expected FFT sample point
        for (size_t i = 0; i < m_ofdmSettings.m_nFFTPoints; i += 2)
        {
            // Copy first sample's real and img respectivley
            pFFTInput[i][0] = pRxBuffer[j];
            LimitIncrement(j, m_RingBufferBoundary);
            pFFTInput[i][1] = pRxBuffer[j];
            LimitIncrement(j, m_RingBufferBoundary);
            // Copy and and make negative real and img respectivley of next the sample
            pFFTInput[i+1][0] = -pRxBuffer[j];
            LimitIncrement(j, m_RingBufferBoundary);
            pFFTInput[i+1][1] = -pRxBuffer[j];
            LimitIncrement(j, m_RingBufferBoundary);
        }
    }
    // nPoints must be odd
    else
    {
        // Initialize +1 / -1 multiplier
        int s = 1;
        // For each expected FFT sample point
        for(size_t i = 0; i < m_ofdmSettings.m_nFFTPoints; i++)
        {
            // Copy the real part of the sample and multiply by s factor
            pFFTInput[i][0] = s * pRxBuffer[j];
            // Increment double buffer counter
            LimitIncrement(j, m_RingBufferBoundary);
            //j++;
            // Copy the imag part of the sample and multiply by s factor
            pFFTInput[i][1] = s * pRxBuffer[j];
            // Increment double buffer counter
            LimitIncrement(j, m_RingBufferBoundary);
            //j++;
            // Compute factor for next sample
            s *= -1;
        }
    }
}