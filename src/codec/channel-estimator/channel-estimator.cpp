/**
* @file channel-estimator.cpp
* @author Kamil Rog
*
* 
*/

#include "channel-estimator.h"


/**
* Constructor
*
*/
ChannelEstimator::ChannelEstimator(const OFDMSettings settings) :
    m_Settings(settings)
{

}


/**
* Destructor 
*
*/
ChannelEstimator::~ChannelEstimator()
{
    
}

/**
* Estimates the channel in frequency domain for a comb-type
* pilot tone arrangment by subtracting the phase of the pilots
* from adjacent data-sub-carriers
*
* @param pFFTOutput pointer to the Rx Samples
* @param nBytes size of the to the FFT input buffer
*
*/ 
void ChannelEstimator::PhaseCompenstator(fftw_complex *pFFTOutput)
{


}

/**
* Estimates the channel in frequency domain for a comb-type
* pilot tone arrangment by computing the gradient between the 
* a pair of pilot tones and implementing 
* from adjacent data-sub-carriers
*
* @param pFFTOutput pointer to the Rx Samples
* @param nBytes size of the to the FFT input buffer
*
*/ 
void FrequencyDomainInterpolation(double *pFFTOutput)
{

    
}
