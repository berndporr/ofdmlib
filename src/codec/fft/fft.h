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

#define BITS_IN_BYTE            8
#define BITS_PER_FREQ_POINT     2
#define FREQ_POINTS_PER_BYTE    4
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
	FFT(OFDMSettings &settings);

	/**
	* Destructor runs close function and clears the setup flag.
	*
	*/
	~FFT();

	int Configure();
	int Normalise();
	int Close();
	int ComputeTransform();
	// void ComputeFFT();
	// void ComputeIFFT();
	// void ComputeFFT(fftw_complex *dest);
	// void ComputeIFFT(fftw_complex *dest)
	int ComputeTransform(fftw_complex *dest);
	double GetImagSum(const size_t nBytes);

public:

	fftw_complex *in; /// Input buffer for the (I)FFT algorithm.
	fftw_complex *out; // Output buffer, the results of fft execution is put into this after Exectue() call

	fftw_complex *IFFTin; /// Input buffer for the IFFT algorithm.
	fftw_complex *IFFTout; // Output buffer, the results of IFFT execution is put into this after Exectue() call

	fftw_complex *FFTin; /// Input buffer for the (I)FFT algorithm.
	fftw_complex *FFTout; // Output buffer, the results of fft execution is placed here

private:

	OFDMSettings &m_ofdmSettings;
	int m_configured = 0;
    fftw_plan m_fftplan; /// FFT plan 
	fftw_plan m_ifftplan;
	size_t m_PilotStartIndex;
	size_t m_nPilots;

};

#endif
