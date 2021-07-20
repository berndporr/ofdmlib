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
DoubleVec OFDMCodec::Encode(const ByteVec &input, size_t nBytes)
{
    DoubleVec output;
    output.resize(m_Settings.nPoints*2 + m_Settings.cyclicPrefixSize);
    // QAM Encode data block
    m_qam.Modulate(input, (DoubleVec &) m_fft.in, nBytes);
    // Transform data and put into the 
    m_fft.ComputeTransform( (fftw_complex *) &output[GetSettings().cyclicPrefixSize]);
    // Run nyquist modulator
    m_NyquistModulator.Modulate( output, GetSettings().cyclicPrefixSize);
    // Add cyclic prefix
    AddCyclicPrefix(output, GetSettings().nPoints*2 , GetSettings().cyclicPrefixSize);
    return output;
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
ByteVec OFDMCodec::Decode(const DoubleVec &input, size_t nBytes)
{
    // Create output vector
    ByteVec output(nBytes);
    // Time sync to first symbol start
    size_t symbolStart = 0;
    symbolStart = m_detector.FindSymbolStart(input, nBytes);
    // Run Data thrgough nyquist demodulator
    m_NyquistModulator.Demodulate(input, symbolStart);
    // Compute FFT & Normalise
    m_fft.ComputeTransform();
    // Normalise FFT
    m_fft.Normalise();
    // Decode QAM encoded fft points and place in the destination buffer
    m_qam.Demodulate( (DoubleVec &) m_fft.out, output, nBytes);
    return output;
}
