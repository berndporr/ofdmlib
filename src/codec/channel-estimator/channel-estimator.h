/**
* @file ofdmcodec.h
* @author Kamil Rog
*
* @section DESCRIPTION
* 
*/
#ifndef CHANNEL_ESTIMATOR
#define CHANNEL_ESTIMATOR

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
 * @brief Channel Estimator Object
 * 
 */
class ChannelEstimator {

public: 

    /**
	* Constructor 
	*
	*/
	ChannelEstimator(OFDMSettings settingsStruct);

    /**
	* Destructor 
	*
	*/
	~ChannelEstimator();

    void PhaseCompenstator(fftw_complex *pFFTOutput, size_t nBytes);
    //void FrequencyDomainInterpolation(double *pFFTOutput, size_t nBytes);

private:

    OFDMSettings m_Settings;

};



#endif
