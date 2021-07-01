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
int ofdmFFT::Configure(uint16_t nPoints, int type)
{   
    // If object has been configured before
    if(configured)
    { 
    // Destroy fft plan and free allocated memory to buffers
    Close();
    }
    in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * nPoints);
    out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * nPoints);
    // Measure if many of the transforms of the siza are going to be performed
    // otherwise use FFTW_ESTIMATE 
    fftplan = fftw_plan_dft_1d(nPoints, in, out, type, FFTW_MEASURE); 

    nFFTPoints = nPoints;

    // Set configure flag
    configured = 1;
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
    fftw_destroy_plan(fftplan);
    fftw_free(in); fftw_free(out);
    configured = 0;
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
    double multiplicationFactor = 1./nFFTPoints;
    for (uint16_t i = 0; i < nFFTPoints; i++)
    {
        out[i][0] *= multiplicationFactor;
        out[i][1] *= multiplicationFactor;
    }
    return 0;
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
    fftw_execute(fftplan); 
    return 0;
}
