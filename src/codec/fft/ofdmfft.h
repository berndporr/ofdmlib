/**
* @file ofdmfft.h
* @author Kamil Rog
*
* 
*/
#ifndef OFDM_FFT_H
#define OFDM_FFT_H

#include <string>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <math.h>
#include <fftw3.h>

#include "common.h"


/**
 * @brief Fourier and Inverse Fourier transform object class
 * This object is a wrapper of fftw3 library for ofdmlib.
 * Contains the 
 * 
 */
class ofdmFFT {

public:

	/**
	* Default constructor
	*/
	ofdmFFT()
	{

	}

	/**
	* Constructor runs configure function.
	* 
	* @param nPoints Number(uint16_t) of FFT / IFFT coefficients
	* @param type Specifies whether the object computes FFT or IFFT choices - FFTW_FORWARD(-1) FFTW_BACKWARD(+1)
	*
	* @return -
	*
	*/
	ofdmFFT(size_t nPoints, int type, uint32_t pilotStep)
	{
		Configure(nPoints, type, pilotStep);
	}

	/**
	* Destructor runs close function and clears the setup flag.
	*
	* @return -
	*
	*/
	~ofdmFFT()
	{
		Close();
	}

	int Configure(uint16_t nPoints, int type, uint32_t pilotStep);
	int Normalise();
	int Close();
	int ComputeTransform();
	int ComputeTransform(fftw_complex *dest);
	double GetImagSum();

public:

	fftw_complex *in; /// Input buffer for the (I)FFT algorithm.
	fftw_complex *out; // Output buffer, the results of fft execution is put into this after Exectue() call

private:

	uint16_t m_nFFT = 0;
	uint32_t m_pilotToneStep = 0;
	int m_configured = 0;
    fftw_plan m_fftplan; /// FFT plan 

};

#endif
