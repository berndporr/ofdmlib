/**
* @file ofdmfft.h
* @author Kamil Rog
*
* @section DESCRIPTION
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
	* Constructor runs setup function and sets the setup flag.
	* 
	* @param nPoints Number(uint16_t) of FFT / IFFT coefficients
	* @param type Specifies whether the object computes FFT or IFFT choices - FFTW_FORWARD(-1) FFTW_BACKWARD(+1)
	*
	* @return -
	*
	*/
	ofdmFFT(size_t nPoints, int type)
	{
		Setup(nPoints, type);
		settingsSet = 1;
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
		settingsSet = 0;
	}

	int Setup(uint16_t nPoints, int type);
	int Close();
	int Execute();

public:

	fftw_complex *in; /// Input buffer for the (I)FFT algorithm.
	fftw_complex *out; // Output buffer, the results of fft execution is put into this after Exectue() call

private:

	int settingsSet = 0;
    fftw_plan p; /// FFT plan 

};

#endif
