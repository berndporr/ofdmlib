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
#include "detector.h" // this has fft & nyquist definitions as include header
#include "qam-modulator.h"
#include "common.h"

struct OFDMSettings
{
    int type;
    size_t EnergyDispersalSeed; 
    size_t nPoints; // Total number of FFT & IFFT coefficients 
	size_t pilotToneStep; // Pilot Tones
    double  pilotToneAmplitude; // Pilot Tones
    size_t guardInterval; // The time between the current and consecutive ofdm symbol
    size_t QAMSize; // QAM Modulator 
    size_t cyclicPrefixSize; // Cyclic-Prefix
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
        m_detector(settingsStruct.nPoints, settingsStruct.cyclicPrefixSize, &m_fft, &m_NyquistModulator),
        m_qam(settingsStruct.nPoints, settingsStruct.pilotToneStep,  settingsStruct.pilotToneAmplitude, settingsStruct.EnergyDispersalSeed, settingsStruct.QAMSize)
    {
        prefixedSymbolSize = ((m_Settings.nPoints * 2) + m_Settings.cyclicPrefixSize);
        rxBuffer = (double*) calloc((prefixedSymbolSize*2),sizeof(double));
        m_encodedSymbolVec.resize(prefixedSymbolSize);
	}

    /**
	* Destructor 
	*
	*/
	~OFDMCodec()
	{
        free(rxBuffer);
	}
    // Encoding Related Functions //
    void Encode(const uint8_t *input, double *output, size_t nBytes);
    // Decode Related Functions //
    void Decode(const double *input, uint8_t *output, size_t nBytes);

    size_t ProcessRxBuffer(const double *input, uint8_t *output);
    void ProcessTxBuffer(const uint8_t *input, double *txBuffer, size_t nBytes);

    const OFDMSettings & GetSettings() const;
    ofdmFFT m_fft;

private:
    // ofdm related objects
    OFDMSettings m_Settings;
	
    NyquistModulator m_NyquistModulator;
    Detector m_detector;
    QamModulator m_qam;

    size_t prefixedSymbolSize;
    DoubleVec m_encodedSymbolVec;
	double *rxBuffer; // This buffer contains the ADC samples and holds 2x symbolSizeWithPrefix samples

};

 inline const OFDMSettings & OFDMCodec::GetSettings() const
 {
     return m_Settings;
 }

#endif
