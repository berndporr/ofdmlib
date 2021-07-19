/**
* @file ofdmcodec.cpp
* @author Kamil Rog
*
* @section DESCRIPTION
* 
*/

#define UINT16_T_UPPER_LIMIT 65535

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
* @param inputData reference to input data array
*
* @return 0 On Success, else error number.
*
* @todo: Modify this so all data types can be handled not just the dobuleVec.
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
* @return 0 On Success, else error number.
*
* @todo: Update for each new feature.
*/
ByteVec OFDMCodec::Decode(const DoubleVec &input, size_t nBytes)
{
    // Data size is the number of points minus number of pilot tones 
    //size_t dataSize = (2 * (m_Settings.nPoints - (size_t)(m_Settings.nPoints/m_Settings.cyclicPrefixSize)));
    // nBytes
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
