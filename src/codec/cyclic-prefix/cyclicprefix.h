/**
* @file ofdmfft.h
* @author Kamil Rog
*
* 
*/
#ifndef CYCLIC_PREFIX_H
#define CYCLIC_PREFIX_H

#include <string>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <math.h>
#include <fftw3.h>
#include <cstring>


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
static int AddCyclicPrefix(double *symbol, uint32_t symbolSize, uint32_t prefixSize)
{
	// Copy the Cyclic Prefix
	memcpy(&symbol[0], &symbol[symbolSize], (sizeof(double)*prefixSize));
	return 0;
}


/**
 * @brief Correlator object repsonsible for calculating correlation
 * between signal and it's delayed version, where the expected prefix of 
 * a symbol should be.
 * 
 */
class Correlator {

public:

	/**
	* Default constructor
	*/
	Correlator()
	{

	}

	/**
	* Constructor runs setup function and sets the setup flag.
	* 
	* @param nPoints Number(uint16_t) of FFT / IFFT coefficients
	* @param prefixSize size(uint16_t) of the cyclic prefix in samples.
	* @param pDouble pointer to data to do the correlation on, the Rx sampled signal.
	* @param buffSize size(uint32_t) of the buffer 
	*
	*/
	Correlator(uint16_t nPoints, uint16_t prefixSize, double *pDouble, uint32_t buffSize)
	{
		Configure(nPoints, prefixSize, pDouble, buffSize);
	}

	/**
	* Destructor 
	* Runs close function.
	*
	*/
	~Correlator()
	{
		Close();
	}

	int Configure(uint16_t fftPoints, uint16_t prefixSize, double *pDouble, uint32_t buffSize);
	int Close();
	int CoarseSearch();
		
	double ExecuteCorrelator(uint32_t Offset);
	int FindSymbolStart();

private:

	int configured = 0;
	uint16_t nPrefix = 0;
	double threshold = 30.0; // TODO: Calibration function which listens to the noise and sets this value
	uint32_t startOffset = 0;
	uint32_t offset = 0;
	uint32_t bufferSize = 0;
	uint32_t symbolSize = 0;

	double *input; 

	uint16_t searchRange; // This number should be odd 

};

#endif
