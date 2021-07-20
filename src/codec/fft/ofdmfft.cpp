/**
* @file ofdmfft.cpp
* @author Kamil Rog
*
* 
*/


#include "ofdmfft.h"


/**
* Sets up FFT for specified size, 
* 
* @param nPoints Number(uint16_t) of FFT / IFFT coefficients
* 
* @param type Specifies whether the object computes FFT or IFFT choices - FFTW_FORWARD(-1) FFTW_BACKWARD(+1)
*
* @return 0 on success, else error number
*
*/
int ofdmFFT::Configure(size_t nPoints, int type, size_t pilotStep)
{   
    // If object has been configured before
    if(m_configured)
    { 
        // Destroy fft plan and free allocated memory to buffers
        Close();
    }
    in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * nPoints);
    out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * nPoints);
    // Measure if many of the transforms of the siza are going to be performed
    // otherwise use FFTW_ESTIMATE 
    m_fftplan = fftw_plan_dft_1d(nPoints, in, out, type, FFTW_MEASURE); 

    m_nFFT = nPoints;
    m_pilotToneStep = pilotStep;

    // Set configure flag
    m_configured = 1;
    return 0;
}


/**
* Destroys fftw plan and frees up allocated memory for input and output buffers
* 
* @return 0 on success, else error number
*
*/    
int ofdmFFT::Close()
{
    fftw_destroy_plan(m_fftplan);
    fftw_free(in); fftw_free(out);
    m_configured = 0;
    return 0;
}


/**
* Normalises the output of the FFT 
* 
* @return 0 on success, else error number
*
*/  
int ofdmFFT::Normalise()
{
    double multiplicationFactor = 1./m_nFFT;
    for (uint32_t i = 0; i < m_nFFT; i++)
    {
        out[i][0] *= multiplicationFactor;
        out[i][1] *= multiplicationFactor;
    }
    return 0;
}


/**
* Computes the sum of the imaginary points where 
* pilot tones are expected
* 
* @return symbol start(integer) index, else -1
*
*/
double ofdmFFT::GetImagSum(size_t nBytes) 
{
    double sumOfImag = 0.0;
    size_t pilotToneCounter =  (int) m_pilotToneStep / 2 ; // divide this by to when starting with -ve frequencies
    size_t fftPointIndex = (int) ((m_nFFT*2) - (m_nFFT*2) / m_pilotToneStep / 2 - nBytes * 4 / 2);
    fftPointIndex = (int) fftPointIndex / 2;
    size_t insertionCounter = 0;
    // For expected byte 
    for(size_t byteCounter = 0; byteCounter < nBytes; byteCounter++)
    {
        insertionCounter = 0;
        // Process 4 FFT points i.e 8 bits
        while(insertionCounter < 4)
        {
            // If pilot tone counter counted down
            // This point is the pilot tone
            if(pilotToneCounter == 0)
            {
                // Reset Counter
                pilotToneCounter = m_pilotToneStep;
                sumOfImag += out[fftPointIndex][1];
    
            }
            // This point is QAM encoded complex point
            else
            {
                insertionCounter++;
                pilotToneCounter--;
            }
            // Increment fft point counter
            fftPointIndex++;
            // Check if fft exceeds the limit of points
            if(fftPointIndex == m_nFFT)
            {
                // Roll back to positive frequencies
                fftPointIndex = 0;
            }
        }

    }
    // Return sum
    return sumOfImag;
}


/**
* Computes FFT Based on the object's input (in) buffer and stores it in the object's output (out) buffer.
* 
*
* @return 0 on success, else error number
*
*/    
int ofdmFFT::ComputeTransform()
{
    fftw_execute(m_fftplan);
    return 0;
}


/**
* Computes FFT using object's configured plan and input(in) buffer
* and stores it in specified destination.
* 
* @param dest pointer to the fftw_complex array
*
*
* @return 0 on success, else error number
*
*/   
int ofdmFFT::ComputeTransform(fftw_complex *dest)
{
    fftw_execute_dft(m_fftplan, in, dest);
    return 0;
}
