/**
* @file ofdmfft.cpp
* @author Kamil Rog
*
* 
*/

#include "fft.h"


/**
* Constructor 
*
*
*/
FFT::FFT(const OFDMSettings &settings) :
    m_ofdmSettings(settings),
    m_configured(0)
{
    Configure();
}


/**
* Destructor 
*
*
*/
FFT::~FFT()
{
    CleanUp();
}

/**
* Sets up FFT for specified size, 
* 
* @param nPoints Number of FFT / IFFT coefficients
* 
* @param type Specifies whether the object computes FFT or IFFT choices - FFTW_FORWARD(-1) FFTW_BACKWARD(+1)
*
* @return 0 on success, else error number
*
*/
void FFT::Configure()
{   
    // If object has been configured before
    if(m_configured)
    { 
        // Destroy fft plan and free allocated memory to buffers
        CleanUp();
    }
    // Allocate memory for the input & output buffers
    in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * m_ofdmSettings.m_nFFTPoints);
    out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * m_ofdmSettings.m_nFFTPoints);

    // Create a plan by measuring the fastest avaiable plan
    m_fftplan = fftw_plan_dft_1d(m_ofdmSettings.m_nFFTPoints, in, out, m_ofdmSettings.m_type, FFTW_MEASURE); 

    // Set configure flag
    m_configured = 1;
}


/**
* Destroys fftw plan and frees up allocated memory for input and output buffers
*
*/    
void FFT::CleanUp()
{
    fftw_destroy_plan(m_fftplan);
    fftw_free(in); fftw_free(out);
    //fftw_cleanup();
    m_configured = 0;
}


/**
* Normalises the output of the FFT 
*
*/  
void FFT::Normalise()
{
    // Calculate the multiplication factor
    double multiplicationFactor = 1./m_ofdmSettings.m_nFFTPoints;
    // For each complex frequency point
    for (size_t i = 0; i < m_ofdmSettings.m_nFFTPoints; i++)
    {
        out[i][0] *= multiplicationFactor;
        out[i][1] *= multiplicationFactor;
    }
}


/**
* Computes the sum of the imaginary points where 
* pilot tones are expected
* 
* @return sum of the phase(imaginary) compomenets of the pilot tones
*
*/
double FFT::GetImagSum() 
{
    double sumOfImag = 0.0;
    // For each pilot tone location
    for(size_t i = 0; i < m_ofdmSettings.m_PilotToneLocations.size(); i++)
    {
        // Add phase value of a pilot tone
        sumOfImag += abs( out[ m_ofdmSettings.m_PilotToneLocations[i] ][1] );
    }
    // Return sum
    return sumOfImag;
}


/**
* Computes FFT Based on the object's input (in) buffer and stores it in the object's output (out) buffer.
*
*/    
void FFT::ComputeTransform()
{
    fftw_execute(m_fftplan);
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
void FFT::ComputeTransform(fftw_complex *dest)
{
    fftw_execute_dft(m_fftplan, in, dest);
}
