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

// Ofdmlib objects
#include "ofdm-settings.h"
#include "channel-estimator.h"
#include "detector.h"             // Detector Contains the fft & nyquist definitions as include header
#include "qam-modulator.h"
#include "common.h"



/**
 * @brief OFDM codec object class
 * This object encapsulates the encoding 
 * and decoding functionlaity and settings for each
 * ofdm related object. It is esentially a wrapper 
 * which chains functions to create Tx & Rx chain.
 * for one symbol.
 * 
 */
class OFDMCodec {

public: 

    /**
	* Constructor 
	*
	*/
	OFDMCodec(OFDMSettingsStruct settingsStruct);

    /**
	* Destructor 
	*
	*/
	~OFDMCodec();

    void Encode(const uint8_t *input, double *output);

    void Decode(const double *input, uint8_t *output);

    size_t ProcessRxBuffer(const double *input, uint8_t *output, size_t nBytes);
    void ProcessTxBuffer(const uint8_t *input, double *txBuffer);

    //const OFDMSettingsStruct & GetSettings() const;
    const OFDMSettings & GetSettings() const;

private:

    OFDMSettings m_Settings;

public:

    FFT m_fft;

private:

	NyquistModulator m_NyquistModulator;
    Detector m_detector;
    QamModulator m_qam;
    ChannelEstimator m_Estimator;

};


/**
 * Returns constant reference to the OFDMSettings 
 * object
 */
inline const OFDMSettings & OFDMCodec::GetSettings() const
{
    return m_Settings;
}

#endif
