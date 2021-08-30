/**
* @file ofdmcodec.h
* @author Kamil Rog
*
* @section DESCRIPTION
* 
*/
#ifndef CHANNEL_ESTIMATOR
#define CHANNEL_ESTIMATOR

#include "ofdm-settings.h"
#include <string>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <math.h>
#include <fftw3.h>

#include "common.h"


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
	ChannelEstimator(const OFDMSettings settings);

    /**
	* Destructor 
	*
	*/
	~ChannelEstimator();

    void FrequencyDomainInterpolation(fftw_complex *pFFTOutput);
	double LinearInterpolation(double a, double b, double t);
	void PlotQAM(fftw_complex *FFT, fftw_complex *Corrected);

private:
	
	fftw_complex *m_Temp; // Temporary plot buffer, saves the rx QAM
    const OFDMSettings m_Settings;
	DoubleVec m_InterpolationFactors;;

};



#endif
