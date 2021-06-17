/**
* @file ofdmfft.cpp
* @author Kamil Rog
*
* @section 
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
* @return
*
*/
int ofdmFFT::Setup(uint16_t nPoints, int type)
{
    in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * nPoints);
    out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * nPoints);
    p = fftw_plan_dft_1d(nPoints, in, out, type, FFTW_ESTIMATE);
    return 0;
}


/**
* Destroys fftw plan and frees up allocated memory for input and output buffers
* 
* @return 0 on sucess, else error number
*
*/    
int ofdmFFT::Close()
{
    fftw_destroy_plan(p);
    fftw_free(in); fftw_free(out);
    settingsSet = 0;
    return 0;
}


/**
* Computes FFT Based on the object's input (in) buffer and stores it in the object's output (out) buffer.
* 
*
* @return 0 on sucess, else error number
*
*/    
int ofdmFFT::Execute()
{

    fftw_execute(p); 
    return 0;
}
