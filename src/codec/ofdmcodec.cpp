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
OFDMCodec::OFDMCodec(OFDMSettings settingsStruct) :
    m_Settings(settingsStruct),
    m_fft(m_Settings),
    m_NyquistModulator(m_Settings),
    m_detector(m_Settings, m_fft, m_NyquistModulator),
    m_qam(m_Settings),
    m_Estimator(m_Settings)
{
    m_PrefixedSymbolSize = ((m_Settings.nFFTPoints * 2) + m_Settings.PrefixSize);
    // Create Rx Buffer capable of holding maximum of twice the prefixed symbol size samples
    rxBuffer = (double*) calloc((m_PrefixedSymbolSize*2),sizeof(double));
}


/**
* Destructor 
*
*/
OFDMCodec::~OFDMCodec()
{
    free(rxBuffer);
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
void OFDMCodec::Encode(const uint8_t *input, double *output, size_t nBytes)
{
    // QAM Encode data block
    m_qam.Modulate(input, (DoubleVec &) m_fft.in, nBytes);
    // Transform data and put into the 
    m_fft.ComputeTransform( (fftw_complex *) &output[GetSettings().PrefixSize]);
    // Run nyquist modulator
    m_NyquistModulator.Modulate(&output[GetSettings().PrefixSize]);
    // Add cyclic prefix
    AddCyclicPrefix(output, GetSettings().nFFTPoints*2 , GetSettings().PrefixSize);
}

void OFDMCodec::ProcessTxBuffer(const uint8_t *input, double *txBuffer, size_t nBytes)
{
    // QAM Encode data block
    m_qam.Modulate(input, (DoubleVec &) m_fft.in, nBytes);
    // Transform data and put into the 
    m_fft.ComputeTransform( (fftw_complex *) &txBuffer[GetSettings().PrefixSize]);
    // Run nyquist modulator
    m_NyquistModulator.Modulate(&txBuffer[GetSettings().PrefixSize]);
    // Add cyclic prefix
    AddCyclicPrefix(txBuffer, GetSettings().nFFTPoints*2 , GetSettings().PrefixSize);
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
void OFDMCodec::Decode(const double *input, uint8_t *output, size_t nBytes)
{
    // Time sync to first symbol start
    size_t symbolStart = -1;
    if( symbolStart >= 0)
    {
        symbolStart = m_detector.FindSymbolStart(input, nBytes);
        // Run Data thrgough nyquist demodulator
        m_NyquistModulator.Demodulate(input, m_fft.in, symbolStart);
        // Compute FFT & Normalise
        m_fft.ComputeTransform();
        // Normalise FFT
        m_fft.Normalise();
        // Decode QAM encoded fft points and place in the destination buffer
        m_qam.Demodulate( (DoubleVec &) m_fft.out, output, nBytes);
    }
}


/**
* Decodes One OFDM Symbol
*
* @param input pointer to the rx buffer
*
* @param nBytes number of bytes encoded in the symbol
*
* @return byte vector containing decoded bytes
*
*/
size_t OFDMCodec::ProcessRxBuffer(const double *input, uint8_t *output , size_t nBytes)
{    
    long int symbolStart = -1;
    // Copy Rx Buffer
    memcpy(&rxBuffer[m_PrefixedSymbolSize], &input[0], m_PrefixedSymbolSize*sizeof(double));
    // Find Symbol Start
    symbolStart = m_detector.FindSymbolStart(rxBuffer, nBytes);
    // If symbol start detected
    if (symbolStart >= 0)
    {
        //std::cout << "Symbol Start Found = " << symbolStart << std::endl;
        // Run Data thrgough nyquist demodulator
        m_NyquistModulator.Demodulate(rxBuffer, m_fft.in, symbolStart);
        // Compute FFT & Normalise
        m_fft.ComputeTransform();
        // Normalise FFT
        m_fft.Normalise();
        // Estmiate Channel
        //m_Estimator.PhaseCompenstator(m_fft.out, nBytes);
        // Decode QAM encoded fft points and place in the destination buffer
        m_qam.Demodulate( (DoubleVec &) m_fft.out, output, nBytes);
        // Copy Residue for next itteration
        memcpy(&rxBuffer[0], &rxBuffer[m_PrefixedSymbolSize], m_PrefixedSymbolSize*sizeof(double));
        return nBytes;
    }
    else
    {
        // Copy Residue for next itteration
        memcpy(&rxBuffer[0], &rxBuffer[m_PrefixedSymbolSize], m_PrefixedSymbolSize*sizeof(double));
        return 0;
    }

}