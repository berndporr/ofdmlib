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


/**
 * @brief AutoCorrelator
 * 
 */
class AutoCorrelator {

public:

	/**
	* Default constructor
	*/
	AutoCorrelator()
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
	AutoCorrelator(uint16_t nPoints, uint16_t prefixSize, double *pDouble, uint32_t buffSize)
	{
		Configure(nPoints, prefixSize, pDouble, buffSize);
	}

	/**
	* Destructor 
	* Runs close function.
	*
	*/
	~AutoCorrelator()
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
	float threshold = 0;
	uint32_t startOffset = 0;
	uint32_t offset = 0;
	uint32_t bufferSize = 0;
	uint32_t symbolSize = 0;

	double *input; 

	uint16_t searchRange; // This number should be odd 

};

#endif
