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
#include "bandpass.h"


struct OFDMSettings
{
    int type;
	bool complexTimeSeries = false; // Complexity
    uint16_t EnergyDispersalSeed;
    uint16_t nPoints; // Total number of FFT & IFFT coefficients 
	uint16_t pilotToneStep; // Pilot Tones
    float    pilotToneAmplitude; // Pilot Tones
    uint16_t guardInterval; // The time between the current and consecutive ofdm symbol
    uint16_t QAMSize; // QAM Modulator 
    uint32_t cyclicPrefixSize; // Cyclic-Prefix
};


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

	OFDMCodec(OFDMSettings settingsStruct, double *buffer)
    {
        Configure(settingsStruct, buffer);
	}

    // Encoding Related Functions
    int Encode(float *data);

    int Configure(OFDMSettings settingsStruct, double *buffer);

    int QAMModulatorPlaceholder(float *data);
    int QAMDemodulatorPlaceholder(double *data);

    // Decode
    int Decode();

    // Get & Set Functions for the settings variables
    uint16_t SetEnergyDispersalSeed(uint16_t seed);
    uint16_t GetEnergyDispersalSeed();
    
    uint16_t SetCodecType(int type);
    int GetCodecType();

    uint16_t SetnPoints(uint16_t newNPoints);
    uint16_t GetnPoints();

    uint16_t SetTimeComplexity(bool newComplexity);
    bool GetTimeComplexity();

    uint16_t GetPilotTonesIndicies(); // This should probably return an array of the pilotTones
    uint16_t GetPilotToneStep();
    uint16_t SetPilotTones(uint16_t newPilotToneStep);
    uint16_t SetPilotTones(uint16_t newPilotToneVector[], uint16_t nPilots); // nPilots might not be needed.

    uint16_t GetPilotTonesAmplitude();
    uint16_t SetPilotTonesAmplitude(float newPilotToneStep);

    uint16_t GetQAMSize();
    uint16_t SetQAMSize(uint16_t newQAMSize);

    uint16_t GetCyclicPrefixSize();
    uint16_t SetCyclicPrefixSize(uint16_t newCyclicPrefixSize);

    // Objects
	ofdmFFT m_fft; // TODO: In future releases this can possibly made private

private:
    // Settings
    OFDMSettings m_Settings;
    BandPassModulator m_bandPass;
    

};

#endif
