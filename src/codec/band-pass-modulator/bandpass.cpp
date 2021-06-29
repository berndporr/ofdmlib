/**
* @file bandpass.cpp
* @author Kamil Rog
*
* 
*/

#include "bandpass.h"

/**
* 
* 
* @param nPoints 
* 
*
* @return 0 on success, else error number
*
*/
int BandPassModulator::Configure(uint16_t fftPoints, int type, fftw_complex *pComplex,  double *pDouble)
{   
    // Set variables
    nPoints = fftPoints;
    complexBuffer = pComplex;
    doubleBuffer = pDouble;
    configured = 1;
    return 0;
}

/**
* Sets the buffer pointers to null 
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
* @todo: Figure out the odd npoint loops and posibly make the even loop to work for 2 point 
*/   
int BandPassModulator::Modulate()
{
    // If the nPoints is even and n points is 4 or greater 
    if(nPoints % 2 == 0 )
    {      
        int j = 0;
        for(int i = 0; i < nPoints; i+=2) // This doesnt execute for some reason
        {
            doubleBuffer[j]   = complexBuffer[i][0];
            doubleBuffer[j+1] = complexBuffer[i][1];
            doubleBuffer[j+2] = -complexBuffer[i+1][0];
            doubleBuffer[j+3] = -complexBuffer[i+1][1];
            j += 4;
        }
    }
    else
    {

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
    // If the nPoints is even and n points is 
    if(nPoints % 2 == 0 )
    {
        int j = 0;
        for (int i = 0; i < nPoints; i += 2)
        {
            complexBuffer[i][0] = doubleBuffer[j];
            complexBuffer[i][1] = doubleBuffer[j+1];
            complexBuffer[i+1][0] = -doubleBuffer[j+2];
            complexBuffer[i+1][1] = -doubleBuffer[j+3];
            j += 4;
        }
    }
    else
    {

    }

    return 0;
}
