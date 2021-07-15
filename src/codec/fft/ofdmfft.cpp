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
int ofdmFFT::Configure(uint16_t nPoints, int type, uint32_t pilotStep)
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
double ofdmFFT::GetImagSum() 
{
    double sumOfImag = 0.0;
    uint32_t pilotToneCounter = m_pilotToneStep;
    uint32_t fftPointIndex = 0;
    while(fftPointIndex < m_nFFT)
    {
        if(pilotToneCounter == 0)
        {
            sumOfImag += out[fftPointIndex][1];
            pilotToneCounter = m_pilotToneStep;
        }
        else
        {
            pilotToneCounter--;
        }
        fftPointIndex++;
    }
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
