/**
* @file ofdmfft.h
* @author Kamil Rog
*
* 
*/
#ifndef OFDM_FFT_H
#define OFDM_FFT_H

#include "common.h"
#include "ofdm-settings.h"
#include <string>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <math.h>
#include <fftw3.h>



/**
 * @brief Fourier transform object class
 * This object is a wrapper of fftw3 library function for ofdmlib.
 * Contains Fourier and inverse fourier transforms functions
 * 
 */
class FFT {

public:

	/**
	* Constructor runs configure function.
	* 
	* @param OFDMSettings Structure holding the ofdm configuration
	*/
	FFT(const OFDMSettings &settings);

	/**
	* Destructor runs close function and clears the setup flag.
	*
	*/
	~FFT();

	void Configure();
	void Normalise();
	void Close();
	void ComputeTransform();
	void ComputeTransform(fftw_complex *dest);
	double GetImagSum();

public:

	fftw_complex *in; /// Input buffer for the (I)FFT algorithm.
	fftw_complex *out; // Output buffer, the results of fft execution is put into this after Exectue() call

private:

	const OFDMSettings &m_ofdmSettings;
	int m_configured;
    fftw_plan m_fftplan;

};

#endif
