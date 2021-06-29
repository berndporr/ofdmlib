/**
* @file ofdmfft.h
* @author Kamil Rog
*
* 
*/
#ifndef OFDM_BAND_PASS_H
#define OFDM_BAND_PASS_H

#include <string>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <math.h>
#include <fftw3.h>


/**
 * @brief Digital quadrature modulator & demodulaor class
 * which upsamples/downsamples the signal at a factor of 2
 * The modulation esentially inteleaves the inphase and 
 * quadrature signals in the folllwing fashion:
 * 
 * +Real(c(n)), +Imag(c(n)), -Real(c(n+1), -Imag(c(n+1))
 * 
 * The demodulator is the inverse of this process and combines
 * the real and imaginary pairs back to form that IFFT outputs.
 * 
 */
class BandPassModulator {

public:

	/**
	* Default constructor
	*/
	BandPassModulator()
	{

	}

	/**
	* Constructor runs setup function and sets the setup flag.
	* 
	* @param nPoints Number(uint16_t) of FFT / IFFT coefficients
	* @param type Specifies whether the object is demodulator(-1) or modulator(+1)
	* @param pComplex pointer to the complex buffer output of IFFT or input to FFT
	* @param pDouble pointer to double array
	*
	*/
	BandPassModulator(size_t nPoints, int type, fftw_complex *pComplex, double *pDouble)
	{
		Configure(nPoints, type, pComplex, pDouble);
	}

	/**
	* Destructor runs close function.
	*
	* @return -
	*
	*/
	~BandPassModulator()
	{
		Close();
	}

	int Configure(uint16_t fftPoints, int type, fftw_complex *pComplex, double *pDouble);
	int Close();
	int Modulate();
	int Demodulate();

private:

	int configured = 0;
    int type; /// FFT plan 
	uint16_t nPoints = 0;

	fftw_complex *complexBuffer; /// Input buffer for the Demodulator / Output buffer for demodulator.
	double *doubleBuffer;


};

#endif
