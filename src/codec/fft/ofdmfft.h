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
	ofdmFFT(size_t nPoints, int type)
	{
		Configure(nPoints, type);
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

	int Configure(uint16_t nPoints, int type);
	int Normalise();
	int Close();
	int ComputeTransform();

public:

	fftw_complex *in; /// Input buffer for the (I)FFT algorithm.
	fftw_complex *out; // Output buffer, the results of fft execution is put into this after Exectue() call

private:

	uint16_t nFFTPoints = 0;
	int configured = 0;
    fftw_plan fftplan; /// FFT plan 

};

#endif