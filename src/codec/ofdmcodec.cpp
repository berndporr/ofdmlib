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
    m_fft.ComputeTransform( (fftw_complex *) &output[GetSettings().cyclicPrefixSize]);
    // Run nyquist modulator
    m_NyquistModulator.Modulate( output, GetSettings().cyclicPrefixSize);
    // Add cyclic prefix
    AddCyclicPrefix(output, GetSettings().nPoints*2 , GetSettings().cyclicPrefixSize);
}

void OFDMCodec::ProcessTxBuffer(const uint8_t *input, double *txBuffer, size_t nBytes)
{
    // QAM Encode data block
    m_qam.Modulate(input, (DoubleVec &) m_fft.in, nBytes);
    // Transform data and put into the 
    m_fft.ComputeTransform( (fftw_complex *) &txBuffer[GetSettings().cyclicPrefixSize]);
    // Run nyquist modulator
    m_NyquistModulator.Modulate( txBuffer, GetSettings().cyclicPrefixSize);
    // Add cyclic prefix
    AddCyclicPrefix(txBuffer, GetSettings().nPoints*2 , GetSettings().cyclicPrefixSize);
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
        m_NyquistModulator.Demodulate(input, symbolStart);
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
size_t OFDMCodec::ProcessRxBuffer(const double *input, uint8_t *output)
{    
    // Create output vector
    size_t nBytes = 120;        // !!!!!!!!!!! Make this variable
    long int symbolStart = -1;
    // Copy Rx Buffer
    memcpy(&rxBuffer[prefixedSymbolSize], &input[0], prefixedSymbolSize*sizeof(double));
    // Find Symbol Start
    symbolStart = m_detector.FindSymbolStart(rxBuffer, nBytes);
    // If symbol start detected
    if (symbolStart >= 0)
    {
        std::cout << "Symbol Start Found = " << symbolStart << std::endl;
        // Run Data thrgough nyquist demodulator
        m_NyquistModulator.Demodulate(rxBuffer, symbolStart);
        // Compute FFT & Normalise
        m_fft.ComputeTransform();
        // Normalise FFT
        m_fft.Normalise();
        // Decode QAM encoded fft points and place in the destination buffer
        m_qam.Demodulate( (DoubleVec &) m_fft.out, output, nBytes);
        // Copy Residue for next itteration
        memcpy(&rxBuffer[0], &rxBuffer[prefixedSymbolSize], prefixedSymbolSize*sizeof(double));
        return 120;
    }
    // Copy Residue for next itteration
    memcpy(&rxBuffer[0], &rxBuffer[prefixedSymbolSize], prefixedSymbolSize*sizeof(double));
    return 0;
}