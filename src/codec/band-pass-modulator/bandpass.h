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
 * the real and imaginary pairs back to form IFFT output.
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
	* @param pComplex pointer to the complex buffer output of IFFT or input to FFT
	* @param pDouble pointer to double array
	*
	*/
	BandPassModulator(size_t nPoints, fftw_complex *pComplex, double *pDouble)
	{
		Configure(nPoints, pComplex, pDouble);
	}

	/**
	* Destructor 
	* Runs close function.
	*
	*/
	~BandPassModulator()
	{
		Close();
	}

	int Configure(uint16_t fftPoints, fftw_complex *pComplex, double *pDouble);
	int Close();
	int Modulate();
	int Modulate(double *pDouble);
	int Demodulate(uint32_t offset);

private:

	int configured = 0;
	uint16_t nPoints = 0;

	/// Input buffer for the modulator / Output buffer for demodulator.
	/// This must point to the Output of IFFT for modulator and the
	/// input of FFT for demodulator.
	fftw_complex *complexBuffer; 


	/// Output buffer for the Demodulator / Input buffer for demodulator.
	/// This must point to the desired tx destination buffer for modulator
	///  and the sampled signal for demodulator.
	double *doubleBuffer; 

};

#endif
