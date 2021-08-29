/**
* @file qam-modulator.h
* @author Kamil Rog
*
* @section DESCRIPTION
*
* 4-QAM Modulator/Demodulator
*
* TODO: Recursive encoding & decoding could be faster and could allow for multiple QAM schemes 
*/


#ifndef QAM_MODULATOR_H
#define QAM_MODULATOR_H

#include "common.h"
#include "ofdm-settings.h"
#include <fftw3.h>
#include <iostream>
#include <bits/stdc++.h>

#include <stdio.h>
#include <string.h>


#define BITS_IN_BYTE            8
#define BITS_PER_FREQ_POINT     2
#define BYTE_MAX                255
#define FREQ_POINTS_PER_BYTE    4



/**
 * @brief 4-QAM modulator object,
 * 
 */
class QamModulator {

public: 

	QamModulator(const OFDMSettings &settings) :
        m_ofdmSettings(settings)
    {

	}

    /**
	* Destructor 
	*
	*/
	~QamModulator()
	{

	} 
    void Modulate(const uint8_t *input, fftw_complex *output);
    void Demodulate(const fftw_complex *input, uint8_t *output); 
    void PlotQ(fftw_complex *FFT, size_t nPoints);

private:

    const OFDMSettings &m_ofdmSettings;
};


inline void QamModulator::PlotQ(fftw_complex *FFT, size_t nPoints)
{
    using namespace matplot;
    DoubleVec x;
    DoubleVec y;
    for(size_t i = 0; i < nPoints; i++ )
    {
        x.push_back( FFT[i][0] );
        y.push_back( FFT[i][1] );
    }

    scatter(x, y);
    show();
}


/**
* 4-QAM modulator function.
* Each fft point is capable of encoding 2bits.
* Firstly energy dispersal is performed of each byte,
* then a byte is encoded and placed into the ifft buffer.
* This is acheived by macro which performs logical AND operation
* on the data byte and mask which is subsequently shifted.
* In addition a counter is used to insert pilot tone at appropriate
* locations specified by the settings object.    
* 
* @param input reference to input data byte array to be encoded
*
* @param ifftIn pointer to the ifft buffer which the  buffer to output data array, the ifftIn
*
*/
inline void QamModulator::Modulate(const uint8_t *input, fftw_complex *output) 
{
    // Set-up pseudo-random number generator
    srand(m_ofdmSettings.m_EnergyDispersalSeed);

    uint8_t dataByte = 0;
    uint8_t bitMask = 0x01;
    size_t dataSubCarrierCounter = 0; 
    // For each byte to be encoded
    for(size_t byteCounter = 0; byteCounter <  m_ofdmSettings.m_nDataBytesPerSymbol;  byteCounter++)
    {
        // Perform energy dispersal by xor-ing the to be encoded data byte
        dataByte = input[byteCounter];
        dataByte ^= rand() % BYTE_MAX;
        // Reset bit mask
        bitMask = 0x01;
        // For each sub-carrier(complex frequency point) that composes this byte
        for(size_t i = 0; i < FREQ_POINTS_PER_BYTE; i++)
        {
            // TODO: Assert m_ofdmSettings.m_DataSubCarrierLocations[dataSubCarrierCounter] is within limits 
            // Set IFFT point to to appropriate value depending on the bit value
            ((bitMask & dataByte) > 0 ) ?  output[ m_ofdmSettings.m_DataSubCarrierLocations[dataSubCarrierCounter] ][0] = 1.0 : output[ m_ofdmSettings.m_DataSubCarrierLocations[dataSubCarrierCounter ]][0] = -1.0;
            // Shift mask for the phase(imaginary) component
            bitMask <<= 1;
            // TODO: Assert m_ofdmSettings.m_DataSubCarrierLocations[dataSubCarrierCounter] is within limits 
            // Set IFFT point to to appropriate value depending on the bit value
            ((bitMask & dataByte) > 0 ) ?  output[ m_ofdmSettings.m_DataSubCarrierLocations[dataSubCarrierCounter] ][1] = 1.0 : output[ m_ofdmSettings.m_DataSubCarrierLocations[dataSubCarrierCounter ]][1] = -1.0;
            // Shift mask for the phase(imaginary) component
            bitMask <<= 1;
            dataSubCarrierCounter++;
        }
    }

    // The loop below could be performed once, on the initialization of the fft object
    // But it is here as a sanity check
    // For each pilot tone location
    for(size_t i = 0; i < m_ofdmSettings.m_PilotToneLocations.size(); i++)
    {
        // Insert Pilot Tone
        // Amplitude(Real)
        output[ m_ofdmSettings.m_PilotToneLocations[i] ][0] = m_ofdmSettings.m_PilotToneAmplitude;
        // Phase(Imag)
        output[ m_ofdmSettings.m_PilotToneLocations[i] ][1] = 0;
    }
    //PlotQ(output,m_ofdmSettings.m_nFFTPoints);
    /*   
    for(size_t i = 0 ; i < m_ofdmSettings.m_nFFTPoints ; i++ )
    {
        std::cout << " Real = " << output[i][0] << " Imag = " << output[i][1] << std::endl;
    }
    */

}


/**
* 4-QAM demodulator function.
* Demodulator steps through the fft output and setting 
* bits of each expected byte omitting pilot tones. The
* bit setting is achieved through logixal OR operation.
* This is a simplistic hard decision algorithm, if the
* value of the fft component exceeding 0 is equivelent
* to bit being set. 
* 
* @param input reference to input data array, the output of the fft
*
* @param output reference to output data array
*
* @param nBytes The expected number of bytes to be decoded from the symbol
*
*/
inline void QamModulator::Demodulate(const fftw_complex *input, uint8_t *output)
{
    // Set-up pseudo-random number generator
    srand(m_ofdmSettings.m_EnergyDispersalSeed);

    uint8_t bitMask = 0x01;
    size_t dataSubCarrierCounter = 0; 
    // For each byte to be decoded byte
    for(size_t byteCounter = 0; byteCounter <  m_ofdmSettings.m_nDataBytesPerSymbol;  byteCounter++)
    {
        // Reset byte
        output[byteCounter] = 0;
        // Reset bit mask
        bitMask = 0x01;
        // For each sub-carrier(complex frequency point) that composes this byte
        for(size_t i = 0; i < FREQ_POINTS_PER_BYTE; i++)
        {
           
            // Real component hard decision decoding
            // If Real value is greater than 0
            if( (input[ m_ofdmSettings.m_DataSubCarrierLocations[dataSubCarrierCounter] ][0] > 0 ) )
            {
                // Set the bit by bitwise OR operation
                output[byteCounter] |= bitMask;
            }
            // Shift bit in the mask by 1 to the left
            bitMask <<= 1;
            // Imag component hard decision decoding
            // If Imag value is greater than 0
            if( (input[ m_ofdmSettings.m_DataSubCarrierLocations[dataSubCarrierCounter] ][1] > 0 ) )
            {
                // Set the bit by bitwise OR operation
                output[byteCounter] |= bitMask;
            }
            // Shift bit in the mask by 1 to the left
            bitMask <<= 1;
            dataSubCarrierCounter++;
        }
        // Recover original data by xor-ing input byte with random value
        output[byteCounter] ^= rand() % BYTE_MAX;
    }
}

#endif
