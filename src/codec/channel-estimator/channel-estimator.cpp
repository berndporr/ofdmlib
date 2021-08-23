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
ChannelEstimator::ChannelEstimator(OFDMSettings settingsStruct) :
    m_Settings(settingsStruct)
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
* pilot tone arrangment by interpolating the 
*
* @param fftOutput pointer  Rx Samples
* @param nBytes size of the  to the FFT input buffer
* @param symbolStart Start of the actual first sample of the symbol in the Rx signal buffer. 
*
*/ 
void ChannelEstimator::PhaseCompenstator(fftw_complex *pFFTOutput, size_t nBytes)
{
    double pilotPhaseCompensation = 0;

    // Compute number of pilot tones based on the bytes in the symbol
    size_t nPilots = (size_t) (((nBytes*BITS_IN_BYTE)/BITS_PER_FREQ_POINT)/m_Settings.PilotToneDistance);
    // Compute frequency coefficient index used
    // Assume spectrum is centred symmetrically around DC and depends on nBytes
    size_t pilotToneCounter = (size_t) m_Settings.PilotToneDistance / 2 ; // divide this by to when starting with -ve frequencies
    size_t fftPointIndex = (size_t) ((m_Settings.nFFTPoints) - (nPilots/ 2) - ((nBytes * FREQ_POINTS_PER_BYTE) / 2));
    size_t insertionCounter = 0;
    pilotPhaseCompensation = pFFTOutput[fftPointIndex-pilotToneCounter][1];

     // For expected byte 
    for(size_t byteCounter = 0; byteCounter < nBytes; byteCounter++)
    {
        insertionCounter = 0;
        // Process 4 FFT points i.e 8 bits
        while(insertionCounter < 4)
        {
            // If pilot tone counter counted down
            // This point is the pilot tone
            if(pilotToneCounter == 0)
            {
                // Reset Counter
                pilotToneCounter = m_Settings.PilotToneDistance;
                // Obtain new Phase 
                pilotPhaseCompensation = pFFTOutput[fftPointIndex][1];
    
            }
            // This point is QAM encoded complex point
            else
            {
                pFFTOutput[fftPointIndex][1] -= pilotPhaseCompensation;
                insertionCounter++;
                pilotToneCounter--;
            }
            // Increment fft point counter
            fftPointIndex++;
            // Check if fft exceeds the limit of points
            if(fftPointIndex == m_Settings.nFFTPoints)
            {
                // Roll back to positive frequencies
                fftPointIndex = 0;
            }
        }
    }

    /*
    for(size_t i = 0; i < nBytes; i++)
    {
        //pFFTOutput[+1] exp(-pilotPhase);
    }
    */
    

}