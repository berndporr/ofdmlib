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

// Ofdmlib objects
#include "detector.h"
#include "common.h"

struct OFDMSettings
{
    int type;
	bool complexTimeSeries; // Complexity
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

	OFDMCodec(OFDMSettings settingsStruct) :
        m_fft(settingsStruct.nPoints, settingsStruct.type, settingsStruct.pilotToneStep),
        m_Settings(settingsStruct),
        m_NyquistModulator(settingsStruct.nPoints, ( settingsStruct.type == +1 ) ?  m_fft.out : m_fft.in),
        m_detector(settingsStruct.nPoints, settingsStruct.cyclicPrefixSize, &m_fft, &m_NyquistModulator)
    {

	}

    /**
	* Destructor 
	*
	*/
	~OFDMCodec()
	{

	}


    // Encoding Related Functions
    DoubleVec Encode(const DoubleVec& inputData);

    //int Configure(OFDMSettings settingsStruct, DoubleVec &buffer);

    int QAMModulatorPlaceholder(const DoubleVec &data);
    int QAMDemodulatorPlaceholder(DoubleVec &data);

    // Decode
    DoubleVec Decode(const DoubleVec &rxOutput);

    const OFDMSettings & GetSettings() const;

    // Objects
	ofdmFFT m_fft; // TODO: In future releases this can possibly made private

private:
    // Settings
    OFDMSettings m_Settings;
    NyquistModulator m_NyquistModulator;
    Detector m_detector;

};

 inline const OFDMSettings & OFDMCodec::GetSettings() const
 {
     return m_Settings;
 }

#endif
