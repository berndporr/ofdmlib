/**
* @file qam-modulator.h
* @author Kamil Rog
*
* @section DESCRIPTION
*
* 4-QAM Modulator/Demodulator
*
* TODO: Breaking the encoding and decoding process into two loops for +ve and -ve
        freq respectivley is going to speed up processing.
* TODO: Recursive encoding & decoding could be faster and could allow for multiple QAM schemes 
*/


/*
    // Precise starting point calculation
    size_t nPilots = (size_t) ((m_nFFT - ((nBytes*BITS_IN_BYTE)/BITS_PER_FREQ_POINT))/m_pilotToneStep);
    // Compute frequency coefficient index used
    // Assume spectrum is centred symmetrically around DC and depends on nBytes
    size_t startIndex = (size_t) ((m_nFFT) - (nPilots/ 2) - ((nBytes * 4) / 2)); // Pilot tones are set incorrectly 
*/

#ifndef QAM_MODULATOR_H
#define QAM_MODULATOR_H

#include <iostream>
#include <bits/stdc++.h>
#include "common.h"
#include <stdio.h>
#include <string.h>

#define BITS_IN_BYTE            8
#define BITS_PER_FREQ_POINT     2
#define BYTE_MAX                255
#define FREQ_POINTS_PER_BYTE    4

/**
 * @brief 4-QAM modulator object,
 *
 * 
 */
class QamModulator {

public: 

	QamModulator(OFDMSettings &settings) :
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
    void Modulate(const uint8_t *input, DoubleVec &output, size_t nBytes);
    void Demodulate(const DoubleVec &input, uint8_t *output, size_t nBytes); 

private:

    OFDMSettings &m_ofdmSettings;
};


/**
* 4-QAM modulator function.
* Each fft point is capable of encoding 2bits.
* Firstly energy dispersal is performed of each byte,
* then a byte is encoded and placed into the ifft buffer.
* This is acheived by macro which performs logical AND operation
* on the data byte and mask which is subsequently shifted.
* In addition a counter is used to insert pilot tone at appropriate
* locations specified by the object parameters.    
* 
* @param input reference to input data array to be encoded
*
* @param output reference to output data array, the ifft input
*
*/
inline void QamModulator::Modulate(const uint8_t *input, DoubleVec &output, size_t nBytes) 
{
    // Compute avaiable points for ifft
    // This depends on the size of the ifft and pilot tone step
    size_t nAvaiableifftPoints = (m_ofdmSettings.nFFTPoints - (size_t)(m_ofdmSettings.nFFTPoints/m_ofdmSettings.PilotToneDistance));
    // Compute the equivelent of avaiable data bytes per symbol
    size_t nMaxEncodedBytes = (size_t)((nAvaiableifftPoints *  BITS_PER_FREQ_POINT)  / BITS_IN_BYTE);

    // Check if the the number of bytes expected be demodulated is within one symbol
    if(nMaxEncodedBytes < nBytes)
    {
        std::cout << "QamModulator: Modulate: Error: Expected # of Bytes in this symbol exceeds the possible max! = " << nBytes << std::endl;
        return;
    }
    size_t nPilots = (size_t) (((nBytes*BITS_IN_BYTE)/BITS_PER_FREQ_POINT)/m_ofdmSettings.PilotToneDistance);
    // Compute frequency coefficient index used
    // Assume spectrum is centred symmetrically around DC and depends on nBytes
    size_t startIndex = (size_t) ((m_ofdmSettings.nFFTPoints) - (nPilots/ 2) - ((nBytes * 4) / 2));
    //size_t startIndex = (int) ((m_nFFT) - (((m_nFFT) / m_pilotToneStep) / 2) - ((nBytes * 4) / 2)); // Pilot tones are set incorrectly 

    // Start insertion with negative frequencies
    size_t ifftPointCounter = startIndex;
    size_t pilotCounter =  (int) (m_ofdmSettings.PilotToneDistance / 2); // divide this by to when starting with -ve frequencies

    // Set-up pseudo-random number generator
    srand(m_ofdmSettings.EnergyDispersalSeed);

    uint8_t dataByte = 0;
    uint8_t bitMask = 0x01;

    size_t insertionCounter = 0;
    size_t byteCounter = 0;
    while(byteCounter < nBytes)
    {
        // Perform energy dispersal by xor-ing the to be encoded data byte
        dataByte = input[byteCounter];
        dataByte ^= rand() % BYTE_MAX;
        // Reset bit mask
        bitMask = 0x01;
        // Reset fft point insertion counter 
        insertionCounter = 0;
        // While byte is being encoded
        while(insertionCounter < FREQ_POINTS_PER_BYTE)
        {
            // If pilot tone counter counted down
            if(pilotCounter == 0)
            {
                // Reset counter
                pilotCounter =  m_ofdmSettings.PilotToneDistance;
                // Insert Pilot Tone         
                output[(ifftPointCounter*2)] = m_ofdmSettings.PilotToneAmplitude;
                output[(ifftPointCounter*2)+1] = 0;

            }
            // Insert QAM Encoded point
            else
            {
                // Set IFFT point to to appropriate value depending on the bit value
                ((bitMask & dataByte) > 0 ) ?  output[(ifftPointCounter*2)] = 1.0 : output[(ifftPointCounter*2)] = -1.0;
                // move bit in the mask by 1 to the left
                bitMask <<= 1;
                // Set IFFT point to to appropriate value depending on the bit value
                ((bitMask & dataByte) > 0 ) ?  output[(ifftPointCounter*2)+1] = 1.0 : output[(ifftPointCounter*2)+1] = -1.0;
                // move bit in the mask by 1 to the left
                bitMask <<= 1;
                // Indicate 2 bits have been inserted
                insertionCounter++;
                // Decrement pilot tone counter
                pilotCounter--;               
            }
            // Increment ifft point counter to indicate complex point has been used
            ifftPointCounter++;
            // Check if the counter exceeds the total number of points
            if(ifftPointCounter == m_ofdmSettings.nFFTPoints)
            {
                // wrap to positive frequencies
                ifftPointCounter = 0;
            }
        }
        // One full byte has been encoded
        // Move to the next one
        byteCounter++;
        
    }
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
inline void QamModulator::Demodulate(const DoubleVec &input, uint8_t *output, size_t nBytes)
{
    size_t nAvaiableifftPoints = (m_ofdmSettings.nFFTPoints - (size_t)(m_ofdmSettings.nFFTPoints/m_ofdmSettings.PilotToneDistance));
    size_t nMaxEncodedBytes = (size_t)((nAvaiableifftPoints *  BITS_PER_FREQ_POINT)  / BITS_IN_BYTE);

    // Check if the the number of bytes expected be demodulated is within one symbol
    if(nMaxEncodedBytes < nBytes)
    {
        std::cout << "QamModulator: Demodulate: Error: Expected # of Bytes in this symbol exceeds the possible max! = " << nBytes << std::endl;
        return;
    }

    size_t nPilots = (size_t) (((nBytes*BITS_IN_BYTE)/BITS_PER_FREQ_POINT)/m_ofdmSettings.PilotToneDistance);
    // Compute frequency coefficient index used
    // Assume spectrum is centred symmetrically around DC and depends on nBytes
    size_t startIndex = (size_t) ((m_ofdmSettings.nFFTPoints) - (nPilots/ 2) - ((nBytes * 4) / 2));
    //size_t startIndex = (int) ((m_nFFT) - (((m_nFFT) / m_pilotToneStep) / 2) - ((nBytes * 4 )/ 2));

    // Start insertion with negative frequencies
    size_t fftPointCounter = startIndex;
    size_t pilotCounter = (size_t) (m_ofdmSettings.PilotToneDistance / 2); // divide this by two when starting with -ve frequencies

    // Set pseudo-random number generator
    srand(m_ofdmSettings.EnergyDispersalSeed);
    uint8_t bitMask = 0x01;

    size_t insertionCounter = 0;
    // For each byte 
    for(size_t byteCounter = 0; byteCounter < nBytes; byteCounter++)
    {
        // Reset byte
        output[byteCounter] = 0;
        // Reset insertion counter pointer
        insertionCounter = 0;
        // Reset bit mask
        bitMask = 0x01;
        // Process 4 FFT points i.e 8 bits
        while(insertionCounter < FREQ_POINTS_PER_BYTE)
        {
            // If pilot tone counter counted down
            // This point is pilot tone
            if(pilotCounter == 0)
            {
                // Reset Counter
                // No need to process this in any way
                pilotCounter = m_ofdmSettings.PilotToneDistance;
            }
            // This point is not pilot tone
            // Decode QAM encoded complex point
            else
            {
                // Real component hard decision decoding
                // If Real value is greater than 0
                if( (input[fftPointCounter*2] > 0 ) )
                {
                    // Set the bit by bitwise OR operation
                    output[byteCounter] |= bitMask;
                }
                // Shift bit in the mask by 1 to the left
                bitMask <<= 1;
                // Imag component hard decision decoding
                // If Imag value is greater than 0
                if( (input[fftPointCounter*2+1] > 0 ) )
                {
                    // Set the bit by bitwise OR operation
                    output[byteCounter] |= bitMask;
                }
                // Shift bit in the mask by 1 to the left
                bitMask <<= 1;
                // 1/4 (2bits) of a byte has been processed
                // Update counters
                insertionCounter++;
                pilotCounter--;
            }
            // Increment fft point counter
            fftPointCounter++;
            // Check if fft exceeds the limit of points
            if(fftPointCounter == m_ofdmSettings.nFFTPoints)
            {
                // Roll back to positive frequencies
                fftPointCounter = 0;
            }
        }
        // Recover original data by xor-ing input byte with random value
        output[byteCounter] ^= rand() % BYTE_MAX;
    }
}
#endif
