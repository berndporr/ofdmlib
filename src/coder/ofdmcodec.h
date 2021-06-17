/**
* @file ofdmcodec.h
* @author Kamil Rog
*
* @section DESCRIPTION
* 
*/
#ifndef OFDM_CODEC_H
#define OFDM_CODEC_H

#include <string>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <math.h>
#include <fftw3.h>

#include "ofdmfft.h"


/*
	// Total number of FFT & IFFT coefficients 
    uint16_t m_nPoints;

    // Complexity
    bool m_complexTimeSeries = false;

    // Upsampled size of the symbol without the cyclic prefix
    uint16_t m_symbolSize;
    uint16_t m_totalSymbolSize; // After upsampling

    // Carriers
    uint16_t m_nDataCarriers;

    // Pilot Tones
	uint16_t m_pilotToneStep;
    //uint16_t pilotTonesIndicies[] = { 10, 20, 30 };
    //uint16_t nPilotTones = sizeof(pilotTonesPositions) / sizeof(pilotTonesPositions[0]);

    // The time between the current and consecutive ofdm symbol
    uint16_t m_guardInterval;

    // QAM Modulator 
    uint16_t m_QAMSize;

    // Cyclic-Prefix
    uint32_t m_cyclicPrefixSize;
*/


struct OFDMSettings
{
	bool complexTimeSeries = false; // Complexity
    uint16_t nPoints; // Total number of FFT & IFFT coefficients 
	uint16_t pilotToneStep; // Pilot Tones
    uint16_t guardInterval; // The time between the current and consecutive ofdm symbol
    uint16_t QAMSize; // QAM Modulator 
    uint32_t cyclicPrefixSize; // Cyclic-Prefix
} ;



/**
* Return codes for get & set functions for the settings 
*
*/
enum SET_SETTINGS_RETURN_STATUS
{
	OK	                = 0,
	EXCEEDS_UPPER_LIMIT = 1,
	EXCEEDS_LOWER_LIMIT = 2
};


/**
 * @brief OFDM codec object class
 * This object encapsulates the encoding 
 * and decoding functionlaity and settings for each
 * ofdm related object. It is esentially a wrapper around
 * all elements that make up ofdm modulation scheme.
 * 
 */
class OFDMCodec {

public: 

	OFDMCodec(int type, uint16_t nPoints, bool complexTimeSeries, uint16_t pilotToneStep, uint16_t qamSize)
    {
    	m_Settings.nPoints             = nPoints;
        m_Settings.complexTimeSeries   = complexTimeSeries;
        m_Settings.pilotToneStep       = pilotToneStep;
        m_Settings.QAMSize             = qamSize;
        m_fft.Setup(nPoints, type);
        //nPilotTones         = ; // This needs to be an array
        //pilotTonesPositions = 0;
        //m_guardInterval       = guardInterval;
	}

    // Get & Set Functions for the settings variables

    uint16_t SetFFTDirection(int direction);
    uint16_t GetFFTDirection();

    uint16_t SetnPoints(uint16_t newNPoints);
    uint16_t GetnPoints();

    uint16_t SetTimeComplexity(bool newComplexity);
    uint16_t GetTimeComplexity();

    uint16_t GetPilotTonesIndicies(); // This should probably return an array of the pilotTones
    uint16_t SetPilotTones(uint16_t newPilotToneStep);
    uint16_t SetPilotTones(uint16_t newPilotToneSequence[], uint16_t nPilots); // nPilots might not be needed.

    uint16_t GetGuardInterval();
    uint16_t SetGuardInterval(uint16_t newGuardInterval);

    uint16_t GetQAMSize();
    uint16_t SetQAMSize(uint16_t newQAMSize);

    uint16_t GetCyclicPrefixSize();
    uint16_t SetCyclicPrefixSize(uint16_t newCyclicPrefixSize);

private:
    // Settings

    // Objects
	ofdmFFT m_fft;

    OFDMSettings m_Settings;

};

#endif
