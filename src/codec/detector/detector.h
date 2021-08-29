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

#include "common.h"
#include "fft.h"
#include "nyquist-modulator.h"
#include "accumulator.h"


#include <cstddef>


/**
 * Creates cycli prefix by copying the symbols end to memory
 * preceding the start of the symbol.
 * 
 * @param symbol pointer to the start of the symbol's prefix
 * 
 * @param symbolSize size of the symbol(excluding prefix)
 * 
 * @param prefixSize number of samples in the prefix
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
	Detector(const OFDMSettings &settings, FFT &fft, NyquistModulator &nyquist);

	/**
	* Destructor 
	* Runs close function.
	*
	*/
	~Detector();


	double ExecuteCorrelator();
	void ExecuteAccumulatorFullSet();
	long int CoarseSearch();
	
	double ComputeSumOfImag(size_t offset);
	size_t FineSearch(size_t coarseStart);

	long int FindSymbolStart(const double *input);

	void IncrementCorrelatorIndicies(size_t n);
	void Increment(size_t &counter);
	void IncrementByN(long int &variable, size_t n);

private:

	const OFDMSettings &m_ofdmSettings;
	FFT &rFFT;
	NyquistModulator &rNyquistModulator;
	Accumulator m_correlationAccumulator;

	// Detector Settings
	double m_UpperThreshold; // TODO: Calibration function which listens to the noise and sets this value or Normalisation
	double m_LowerThreshold;
	size_t m_SearchRange; // Search range of the fine search, this range is used to determine the indicies of search loop  
	size_t m_LastPeakCounter; // The number of samples since the last peak was detected 
	size_t m_nFromPeak;

    size_t m_PrefixIndex;
    size_t m_SignalIndex;
	size_t m_OffsetCounter;
	
	// Ring Block Buffer
    // Contains the ADC samples and must hold at least 3x prefixed symbol size doubles
	// This buffer size is m_nBlocks * prefixed symbol size
	size_t m_nBlocks;
	size_t m_RingBufferEdge;

	// Variables keeping track of the next position the data can be pasted in
	size_t m_BlockRingBufferDataPostion; // Track of the next aviable position the data can be pasted in
	size_t m_BlockRingBufferCorrelatorPostion; // This must lag by value of 1 to the, this indicates the starting block correlator function will execute

	bool m_ThresholdFlag;
	double m_CorrelatorMaxValue;
	size_t m_CorrelatorMaxValueIndex;

	// Debug Info
	DoubleVec corOutput; // DEBUG ONLY
	size_t plotCounter = 0;
	
	public:

	double *m_BlockRingBuffer;
};

#endif
