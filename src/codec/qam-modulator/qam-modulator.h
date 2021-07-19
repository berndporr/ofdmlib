/**
* @file qam-modulator.h
* @author Kamil Rog
*
* @section DESCRIPTION
*
* 4-QAM Encoder/Decoder
*
* TODO: Breaking the encoding and decoding process into two loops for +ve and -ve
        freq respectivley is going to speed up processing.
* TODO: Recursive encoding & decoding could be faster and could allow for multiple QAM schemes 
*/
#ifndef QAM_MODULATOR_H
#define QAM_MODULATOR_H

#include <iostream>
#include <bits/stdc++.h>
#include "common.h"
#include <stdio.h>
#include <string.h>

/**
 * @brief QAM modulator
 * 
 */
class QamModulator {

public: 

	QamModulator(size_t fftPoints, size_t pilotToneStep, double pilotToneAmplitude, size_t energyDispersalSeed, size_t QAM) :
        m_nFFT(fftPoints),
        m_pilotToneStep(pilotToneStep),
        m_pilotToneAmplitude(pilotToneAmplitude),
        m_EnergyDispersalSeed(energyDispersalSeed),
        m_BitsPerSymbol(QAM)
    {

	}

    /**
	* Destructor 
	*
	*/
	~QamModulator()
	{

	}
    //template<typename T, typename A> 
    void Modulate(const ByteVec &input, DoubleVec &output, size_t nBytes);
    void Demodulate(const DoubleVec &input, ByteVec &output, size_t nBytes); 

private:

    size_t m_nFFT;
    size_t m_pilotToneStep;
    double m_pilotToneAmplitude;
    size_t m_EnergyDispersalSeed;
    size_t m_BitsPerSymbol;

};


/**
* Template QAM modulator encoding function
* 
* @param input reference to input data array to be encoded
* @param output reference to output data array, this is most likley the ifft input
*
*/
//template<typename T, typename A> 
inline void QamModulator::Modulate(const ByteVec &input, DoubleVec &output, size_t nBytes) 
{
    // Compute avaiable points for ifft
    // This depends on the size of the ifft and pilot tone step
    size_t nAvaiableifftPoints = (m_nFFT - (int)(m_nFFT/m_pilotToneStep));
    // Compute the equivelent of avaiable data bytes per symbol
    size_t nMaxEncodedBytes = (int)((nAvaiableifftPoints *  m_BitsPerSymbol)  / 8);

    // Check if the the number of bytes expected be demodulated is within one symbol
    if(nMaxEncodedBytes < nBytes)
    {
        return;
    }

    // Compute frequency coefficient index used
    // Assume spectrum is centred symmetrically around DC and depends on nBytes
    size_t startIndex = (int) ((m_nFFT*2) - (m_nFFT*2) / m_pilotToneStep / 2 - nBytes * 4 / 2);
    startIndex = (int) startIndex / 2;

    // Start insertion with negative frequencies
    size_t ifftPointCounter = startIndex;
    size_t pilotCounter =  (int) m_pilotToneStep / 2 ; // divide this by to when starting with -ve frequencies

    // Set pseudo-random number generator
    srand(m_EnergyDispersalSeed);

    uint8_t dataByte = 0;
    uint8_t bitMask = 0x01;

    size_t insertionCounter = 0;
    size_t byteCounter = 0;
    while(byteCounter < nBytes)
    {
        dataByte = input[byteCounter];// ^ rand() % 255;
        bitMask = 0x01;
        insertionCounter = 0;
        // While byte is being encoded
        while(insertionCounter < 4)
        {
            // If pilot tone counter counted down
            if(pilotCounter == 0)
            {
                // Reset counter
                pilotCounter =  m_pilotToneStep;
                // Insert Pilot Tone         
                output[(ifftPointCounter*2)] = m_pilotToneAmplitude;
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
            if(ifftPointCounter == m_nFFT)
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
* QAM Demodulator function
* 
* @param input reference to input data array, the output of the fft
*
* @param output reference to output data array, this is the 
*
* @param nBytes The expected number of byte to be decoded from the symbol
*
*/
inline void QamModulator::Demodulate(const DoubleVec &input, ByteVec &output, size_t nBytes)
{
    size_t nAvaiableifftPoints = (m_nFFT - (int)(m_nFFT/m_pilotToneStep));
    size_t nMaxEncodedBytes = (int)((nAvaiableifftPoints *  m_BitsPerSymbol)  / 8);

    // Check if the the number of bytes expected be demodulated is within one symbol
    if(nMaxEncodedBytes < nBytes)
    {
        return;
    }

    // Compute frequency coefficient index used
    // Assume spectrum is centred symmetrically around DC and depends on nBytes
    size_t startIndex = (int) ((m_nFFT*2) - (m_nFFT*2) / m_pilotToneStep / 2 - nBytes * 4 / 2);
    startIndex = (int) startIndex / 2;

    // Start insertion with negative frequencies
    size_t fftPointCounter = startIndex;
    size_t pilotCounter =  (int) m_pilotToneStep / 2 ; // divide this by two when starting with -ve frequencies

    // Set pseudo-random number generator
    srand(m_EnergyDispersalSeed);
    uint8_t bitMask = 0x01;

    size_t insertionCounter = 0;
    // For each byte 
    for(size_t byteCounter = 0; byteCounter < nBytes; byteCounter++)
    {
        // Reset byte
        output[byteCounter] = 0;
        insertionCounter = 0;
        // Reset bit mask
        bitMask = 0x01;
        // Process 4 FFT points i.e 8 bits
        while(insertionCounter < 4)
        {
            // If pilot tone counter counted down
            // This point is pilot tone
            if(pilotCounter == 0)
            {
                // Reset Counter
                // No need to process this in any way
                pilotCounter = m_pilotToneStep;
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
            if(fftPointCounter == m_nFFT)
            {
                // Roll back to positive frequencies
                fftPointCounter = 0;
            }
        }

        // Energy dispersal by xor-ing input byte with random value
        //output[byteCounter] ^= rand() % 255;
    }

}
#endif
