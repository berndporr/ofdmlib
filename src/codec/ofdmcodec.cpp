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
* A place holder function for QAM Modulator 
* It just copies over the data from one aray into fft object input
* 
* @param data pointer to float data array
*
* @return 0 On Success, else error number.
*
* @todo: This needs to be implemented for release v0.5
*/
int OFDMCodec::QAMModulatorPlaceholder(const DoubleVec &data)
{
    uint32_t pilotToneCounter = GetSettings().pilotToneStep;
    uint32_t dataIndex = 0;
    uint32_t fftPointIndex = 0;
    while(fftPointIndex < GetSettings().nPoints)
    {
        if(pilotToneCounter == 0)
        {
            m_fft.in[fftPointIndex][0] =  GetSettings().pilotToneAmplitude;
            m_fft.in[fftPointIndex][1] =  0;
            pilotToneCounter = GetSettings().pilotToneStep;
        }
        else
        {
            m_fft.in[fftPointIndex][0] = data[(dataIndex*2)];
            m_fft.in[fftPointIndex][1] = data[(dataIndex*2)+1];
            pilotToneCounter--; 
            dataIndex++;
        }
        fftPointIndex++;
    }
    return 0;
}


/**
* Encodes one OFDM Symbol
* 
* @param inputData pointer to input data array
*
* @return 0 On Success, else error number.
*
* @todo: Modify this so all data types can be handled not just the dobuleVec.
*/
DoubleVec OFDMCodec::Encode(const DoubleVec &inputData)
{
    DoubleVec output;
    output.resize(m_Settings.nPoints*2 + m_Settings.cyclicPrefixSize);
    // QAM Encode data block
    QAMModulatorPlaceholder(inputData);
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
* A place holder function for QAM demodulator 
* It just copies over the data from the FFT object output to specified destination
* 
* @param data pointer to float data array
*
* @return 0 On Success, else error number.
*
* @todo: This needs to be implemented for release v0.5
*/
int OFDMCodec::QAMDemodulatorPlaceholder(DoubleVec &data)
{
    uint32_t pilotToneCounter = GetSettings().pilotToneStep;
    uint32_t dataIndex = 0;
    uint32_t fftPointIndex = 0;
    while(fftPointIndex < GetSettings().nPoints)
    {
        if(pilotToneCounter == 0)
        {
            pilotToneCounter = GetSettings().pilotToneStep;
            
        }
        else
        {
            data[(dataIndex*2)] = m_fft.out[fftPointIndex][0];
            data[(dataIndex*2)+1] = m_fft.out[fftPointIndex][1];
            pilotToneCounter--;
            dataIndex++;
            
        }
        fftPointIndex++;
    }
    return 0;
}


/**
* Decodes One OFDM Symbol
*
* @return 0 On Success, else error number.
*
* @todo: Update for each new feature.
*/
DoubleVec OFDMCodec::Decode(const DoubleVec &rxSignal)
{
    DoubleVec output;
    // Data size is the number of points minus number of pilot tones 
    size_t dataSize = (2 * (m_Settings.nPoints - (size_t)(m_Settings.nPoints/m_Settings.cyclicPrefixSize)));
    output.resize(dataSize);
    uint32_t symbolStart = 0;
    symbolStart = m_detector.FindSymbolStart(rxSignal);
    // Run Data thrgough nyquist demodulator
    m_NyquistModulator.Demodulate(rxSignal, symbolStart);
    // Compute FFT & Normalise
    m_fft.ComputeTransform();
    // Normalise FFT
    m_fft.Normalise();
    // Demodulate result and place in the destination buffer
    QAMDemodulatorPlaceholder(output);
    return output;


}


// Settings Functions //


/**
* Update energy dispersal seed
* 
* @param OFDMSettings integer for pseudo number generator (uint16_t)
*
* @param buffer pointer to the modulator buffer.
*
* @return 0 On Success, else error number.
*
*/
/*
int  OFDMCodec::Configure(OFDMSettings settingsStruct, DoubleVec &buffer)
{
    m_Settings = settingsStruct;
    m_fft.Configure(settingsStruct.nPoints, settingsStruct.type, settingsStruct.pilotToneStep);

    if(settingsStruct.type == FFTW_BACKWARD)
    {
        m_NyquistModulator.Configure(settingsStruct.nPoints, m_fft.out, buffer);
    }

    if(settingsStruct.type == FFTW_FORWARD)
    {
        m_NyquistModulator.Configure(settingsStruct.nPoints, m_fft.in, buffer);
        m_detector.Configure(settingsStruct.nPoints, settingsStruct.cyclicPrefixSize, buffer, &m_fft, &m_NyquistModulator);
    } 

    return OK;
}
*/
