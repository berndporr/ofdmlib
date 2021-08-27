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
NyquistModulator::NyquistModulator(const OFDMSettings &settings) :
    m_ofdmSettings(settings)
{
    m_RingBufferBoundary = m_ofdmSettings.m_PrefixedSymbolSize*3;
    // Allocate memory for a temporary buffer
    // For the edge case where the symbol spans across 
    // end and start of the ring buffer
    // TODO: Asses whether the if statements are faster
    m_TempBuffer = (double*) calloc((m_ofdmSettings.m_SymbolSize), sizeof(double));
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
* @param rxBuffer Raw Rx Samples
* @param pFFTInput Pointer to the FFT input buffer
* @param symbolStart Start of the actual first sample of the symbol in the Rx signal buffer. 
*
*/   
void NyquistModulator::Demodulate(double *rxBuffer, fftw_complex *pFFTInput, const size_t symbolStart)
{
    
    double *pRxBuffer = nullptr;
    // Initialize double buffer counter
    size_t j = symbolStart;
    // If symbol lies across the end edge of the buffer.
    // Use temporary buffer to allign the symbol for demodulation
    if( symbolStart >= (m_RingBufferBoundary - m_ofdmSettings.m_SymbolSize))
    {
        //std::cout << " Demodulator Edge Case!" << std::endl;
        // Calculate the number elements of the symbol untill boundary of the ring buffer
        size_t nToBoundary = m_RingBufferBoundary - symbolStart;
        // Copy samples
        memcpy( &m_TempBuffer[0], &rxBuffer[symbolStart], nToBoundary*sizeof(double));
        // Calculate the number elements of the symbol over boundary of the ring buffer
        size_t nOverBoundary = m_ofdmSettings.m_SymbolSize - nToBoundary;
        // Copy samples
        memcpy(&m_TempBuffer[nToBoundary], &rxBuffer[0], nOverBoundary*sizeof(double)); // This breaks
        //std::cout << " Demodulator Addition = " << nOverBoundary + nToBoundary << std::endl;
        // Assign Pointer
        pRxBuffer = m_TempBuffer;
        // Update counter;
        j = 0;
        
    }
    // Can demodulate straight from rxBuffer
    else
    {
        pRxBuffer = rxBuffer;
    }
    

    // If the nPoints is even
    if(m_ofdmSettings.m_nFFTPoints % 2 == 0)
    {
        // For each expected FFT sample point
        for (size_t i = 0; i < m_ofdmSettings.m_nFFTPoints; i += 2)
        {
            // Copy first sample's real and img respectivley
            pFFTInput[i][0] = pRxBuffer[j];
            j++;
            pFFTInput[i][1] = pRxBuffer[j];
            j++;
            // Copy and the minus real and img respectivley of next the sample
            pFFTInput[i+1][0] = -pRxBuffer[j];
            j++;
            pFFTInput[i+1][1] = -pRxBuffer[j];
            j++;
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
            j++;
            // Copy the imag part of the sample and multiply by s factor
            pFFTInput[i][1] = s * pRxBuffer[j];
            // Increment double buffer counter
            j++;
            // Compute factor for next sample
            s *= -1;
        }
    }
}