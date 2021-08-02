/**
* @file detector.h
* @author Kamil Rog
*
* 
*/
#ifndef DETECTOR__H
#define DETECTOR__H

#include <string>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <math.h>
#include <fftw3.h>
#include <cstring>

#include "ofdmfft.h"
#include "nyquist-modulator.h"
#include "common.h"

#include <cstddef>


/**
 * Creates cycli prefix by copying the symbols end to memory
 * preceding the start of the symbol
 * 
 * @param symbolStart pointer to the start of the symbol, including prefix
 * 
 * @param symbolSize size of the symbol excluding prefix
 * 
 * @param prefixSize number of samples included in the prefix
 * 
 * @return 0 on success, else error number
 * 
 */
inline void AddCyclicPrefix(double *symbol, size_t symbolSize, size_t prefixSize)
{
	//std::copy(symbol.begin()+symbolSize, symbol.end(), symbol.begin() );
	memcpy(&symbol[0], &symbol[symbolSize], sizeof(double)*prefixSize);
}


/**
 * @brief Detector object repsonsible for calculating correlation
 * between signal and it's delayed version, where the expected prefix of 
 * a symbol should be.
 * 
 */
class Detector {

public:

	/**
	* Constructor runs setup function and sets the setup flag.
	* 
	* @param nPoints Number of FFT / IFFT coefficients
	* @param prefixSize size of the cyclic prefix in samples.
	* @param pDouble pointer to data to do the correlation on, the Rx sampled signal.
	* @param buffSize size of the buffer 
	*
	*/
	Detector(size_t nPoints, size_t prefixSize, ofdmFFT *fft, NyquistModulator *nyquist);

	/**
	* Destructor 
	* Runs close function.
	*
	*/
	~Detector();

	long int CoarseSearch(const double *input);

	double ExecuteCorrelator(const double *input, size_t Offset);
	long int FindSymbolStart(const double *input, size_t nbytes);
	size_t FineSearch(const double *input, size_t coarseStart, size_t nbytes);

private:

	int m_configured = 0;
	size_t m_nPrefix;
	double m_threshold; // TODO: Calibration function which listens to the noise and sets this value
	size_t m_startOffset;
	size_t m_symbolSize;
	size_t m_SearchRange;
	ofdmFFT *pFFT;
	NyquistModulator* pNyquistModulator;
	size_t m_prefixedSymbolSize;


};

#endif
