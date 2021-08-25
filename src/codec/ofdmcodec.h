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
#include "channel-estimator.h"
#include "detector.h"             // Detector Contains the fft & nyquist definitions as include header
#include "qam-modulator.h"
#include "common.h"




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

    /**
	* Constructor 
	*
	*/
	OFDMCodec(OFDMSettings settingsStruct);

    /**
	* Destructor 
	*
	*/
	~OFDMCodec();

    void Encode(const uint8_t *input, double *output, size_t nBytes);

    void Decode(const double *input, uint8_t *output, size_t nBytes);

    size_t ProcessRxBuffer(const double *input, uint8_t *output, size_t nBytes);
    void ProcessTxBuffer(const uint8_t *input, double *txBuffer, size_t nBytes);

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

    size_t m_PrefixedSymbolSize;




};


/**
 * @brief OFDM codec object class
 * This object encapsulates the encoding 
 * and decoding functionlaity and settings for each
 * ofdm related object. It is esentially a wrapper around
 * all elements that make up ofdm modulation scheme.
 * 
 */
inline const OFDMSettings & OFDMCodec::GetSettings() const
{
    return m_Settings;
}

#endif
