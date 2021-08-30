/**
* @file ofdmcodec.cpp
* @author Kamil Rog
*
* @section DESCRIPTION
* 
*/


#include <string>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "ofdmcodec.h"


// Encoding Related Functions //

/**
* Constructor
*
*/
OFDMCodec::OFDMCodec(OFDMSettingsStruct settingsStruct) :
    m_Settings(settingsStruct),
    m_fft(m_Settings),
    m_NyquistModulator(m_Settings),
    m_detector(m_Settings, m_fft, m_NyquistModulator),
    m_qam(m_Settings),
    m_Estimator(m_Settings)
{

}


/**
* Destructor 
*
*/
OFDMCodec::~OFDMCodec()
{

}


/**
* Encodes one OFDM Symbol
* 
* @param inputData reference to input data byte vector
*
* @param nBytes number of bytes encoded in the symbol
*
* @return vector containing encoded symbol 
*
*/
void OFDMCodec::Encode(const uint8_t *input, double *output)
{
    // QAM Encode data block
    m_qam.Modulate(input, m_fft.in);
    // Transform data and put into the 
    m_fft.ComputeTransform( (fftw_complex *) &output[GetSettings().m_PrefixSize]);
    // Run nyquist modulator
    m_NyquistModulator.Modulate(&output[GetSettings().m_PrefixSize]);
    // Add cyclic prefix
    AddCyclicPrefix(output, GetSettings().m_SymbolSize , GetSettings().m_PrefixSize);
}

/**
* Encodes one OFDM Symbol
* 
* @param inputData reference to input data byte vector
*
* @return vector containing encoded symbol 
*
*/
void OFDMCodec::ProcessTxBuffer(const uint8_t *input, double *txBuffer)
{
    // QAM Encode data block
    m_qam.Modulate(input, m_fft.in);
    // Transform data and put into the 
    m_fft.ComputeTransform( (fftw_complex *) &txBuffer[GetSettings().m_PrefixSize]);
    // Run nyquist modulator
    m_NyquistModulator.Modulate(&txBuffer[GetSettings().m_PrefixSize]);
    // Add cyclic prefix
    AddCyclicPrefix(txBuffer, GetSettings().m_SymbolSize, GetSettings().m_PrefixSize);
}


// Decoding Related Functions //


/**
* Decodes One OFDM Symbol
*
* @param inputData reference to input data double vector
*
* @param nBytes number of bytes encoded in the symbol
*
* @return byte vector containing decoded bytes
*
*/
void OFDMCodec::Decode(const double *input, uint8_t *output)
{
    // Time sync to first symbol start
    size_t symbolStart = -1;
    if( symbolStart >= 0)
    {
        symbolStart = m_detector.FindSymbolStart(input);
        // Run Data thrgough nyquist demodulator
        //m_NyquistModulator.Demodulate(input, m_fft.in, symbolStart);
        // Compute FFT & Normalise
        m_fft.ComputeTransform();
        // Normalise FFT
        m_fft.Normalise();
        // Decode QAM encoded fft points and place in the destination buffer
        m_qam.Demodulate(m_fft.out, output);
    }
}


/**
* Decodes One OFDM Symbol
*
* @param input pointer to the rx buffer
*
* @param output pointer to the buffer decoded bytes are stored to
*
* @param nBytes number of bytes encoded in the symbol
*
* @return Number of bytes decoded by the  //TODO: return bool if symbol has been decoded sucessfully
*
*/
size_t OFDMCodec::ProcessRxBuffer(const double *input, uint8_t *output , size_t nBytes)
{    
    long int symbolStart = -1;
    // Find Symbol Start
    symbolStart = m_detector.FindSymbolStart(input);
    // If symbol start detected
    if (symbolStart >= 0)
    {
        // Run Data thrgough nyquist demodulator
        m_NyquistModulator.Demodulate(m_detector.m_BlockRingBuffer, m_fft.in, symbolStart);
        // Compute FFT & Normalise
        m_fft.ComputeTransform();
        // Normalise FFT
        m_fft.Normalise();
        // Estmiate Channel
        //m_Estimator.FrequencyDomainInterpolation(m_fft.out);
        // Decode QAM encoded fft points and place in the destination buffer
        m_qam.Demodulate(m_fft.out, output);
        return nBytes;
    }
    else
    {
        return 0;
    }

}
