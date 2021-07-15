/**
* @file ofdmfft.h
* @author Kamil Rog
*
* 
*/
#ifndef NYQUIST_MODULATOR_H
#define NYQUIST_MODULATOR_H

#include <string>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstddef>
#include <iostream>
#include <math.h>
#include <fftw3.h>

#include "common.h"

/**
 * @brief Digital nyquist quadrature modulator & demodulaor class
 * which upsamples/downsamples the signal at a factor of 2
 * The modulation esentially inteleaves the inphase and 
 * quadrature signals in the folllwing fashion:
 * 
 * +Real(c(n)), +Imag(c(n)), -Real(c(n+1), -Imag(c(n+1))
 * 
 * The demodulator is the inverse of this process and combines
 * the real and imaginary pairs back to form IFFT output.
 * 
 */
class NyquistModulator {

public:


	/**
	* Constructor runs setup function and sets the setup flag.
	* 
	* @param nPoints Number of FFT / IFFT coefficients
	* @param pComplex pointer to the complex buffer output of IFFT or input to FFT
	* @param pDouble pointer to double array
	*
	*/
	NyquistModulator(uint32_t nPoints, fftw_complex *pComplex) :
	nPoints(nPoints),
	complexBuffer(pComplex)
	{
		//Configure(nPoints, pComplex);
	}
	

	/**
	* Destructor 
	* Runs close function.
	*
	*/
	~NyquistModulator()
	{
		Close();
	}

	int Configure(uint16_t fftPoints, fftw_complex *pComplex);
	int Close();
	void Modulate(DoubleVec &ifftOutput, uint32_t prefixSize);
	void Demodulate(const DoubleVec &vectorBuffer, uint32_t offset);

private:

	int configured = 0;
	size_t nPoints;

	/// Input buffer for the modulator / Output buffer for demodulator.
	/// This must point to the Output of IFFT for modulator and the
	/// input of FFT for demodulator.
	fftw_complex *complexBuffer; 


	/// Output buffer for the Demodulator / Input buffer for demodulator.
	/// This must point to the desired tx destination buffer for modulator
	///  and the sampled signal for demodulator.
	//DoubleVec &vectorBuffer; 

};

#endif
