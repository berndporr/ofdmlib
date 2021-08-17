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

#include "fft.h"
#include "nyquist-modulator.h"
#include "common.h"

#include <cstddef>


/**
 * Creates cycli prefix by copying the symbols end to memory
 * preceding the start of the symbol.
 * 
 * @param symbol pointer to the start of the symbol, including prefix
 * 
 * @param symbolSize size of the symbol excluding prefix
 * 
 * @param prefixSize number of samples included in the prefix
 * 
 */
inline void AddCyclicPrefix(double *symbol, size_t symbolSize, size_t prefixSize)
{
	memcpy(&symbol[0], &symbol[symbolSize], sizeof(double)*prefixSize);
}


/**
 * @brief Detector object repsonsible for 
 * calculating correlation
 * between signal and it's delayed version, where the expected prefix of 
 * a symbol should be.
 * 
 */
class Detector {

public:

	/**
	* Constructor
	* 
	* @param settings
	* @param fft reference to the fft objecct
	* @param nyquist reference to the nyquist modulator objecct
	*
	*/
	Detector(OFDMSettings &settings, FFT &fft, NyquistModulator &nyquist);

	/**
	* Destructor 
	* Runs close function.
	*
	*/
	~Detector();


	double ExecuteCorrelator(const double *input, size_t Offset);
	long int CoarseSearch(const double *input);
	
	double ComputeSumOfImag(const double *buff, size_t offset, size_t nbytes);
	size_t FineSearch(const double *input, size_t coarseStart, size_t nbytes);

	long int FindSymbolStart(const double *input, size_t nbytes);

private:

	OFDMSettings &m_ofdmSettings;
	FFT &rFFT;
	NyquistModulator &rNyquistModulator;

	// Detector Settings
	double m_threshold; // TODO: Calibration function which listens to the noise and sets this value or Normalisation
	size_t m_SearchRange; // Search range of the fine search, this range is used to determine the indicies of search loop  
	size_t m_StartOffset; // Coarse Search offset counter, this counter is used for 
	size_t m_LastPeakCounter; // The number of samples since the last peak was detected 

	// 
	size_t m_SymbolSize;
	size_t m_PrefixedSymbolSize;

	// Debug Info
	std::vector<double> corOutput; // DEBUG ONLY
	size_t plotCounter = 0;

};

#endif
