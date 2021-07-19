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
inline void AddCyclicPrefix(DoubleVec &symbol, uint32_t symbolSize, uint32_t prefixSize)
{
	std::copy(symbol.begin()+symbolSize, symbol.end(), symbol.begin() );
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
	Detector(size_t nPoints, size_t prefixSize, ofdmFFT *fft, NyquistModulator *nyquist) :
			m_configured(0),
			m_nPrefix(prefixSize),
			m_threshold(30000.0), // TODO: Calibration function which listens to the noise and sets this value
			m_startOffset(0),
			m_symbolSize(nPoints*2),
			m_SearchRange(25),
			pFFT(fft),
			pNyquistModulator(nyquist)
	{
		//Configure(nPoints, prefixSize, fft, nyquist);
	}

	/**
	* Destructor 
	* Runs close function.
	*
	*/
	~Detector()
	{
		Close();
	}

	int Configure(size_t fftPoints, size_t prefixSize, ofdmFFT *fft, NyquistModulator *nyquist);
	int Close();
	size_t CoarseSearch(const DoubleVec &input);
		
	double ExecuteCorrelator(const DoubleVec &input, size_t Offset);
	size_t FindSymbolStart(const DoubleVec &input, size_t nbytes);
	size_t FineSearch(const DoubleVec &input, size_t coarseStart, size_t nbytes);

private:

	int m_configured = 0;
	size_t m_nPrefix;
	double m_threshold; // TODO: Calibration function which listens to the noise and sets this value
	size_t m_startOffset;
	size_t m_symbolSize;
	size_t m_SearchRange;
	ofdmFFT *pFFT;
	NyquistModulator* pNyquistModulator;
	//DoubleVec &input; 

};

#endif
