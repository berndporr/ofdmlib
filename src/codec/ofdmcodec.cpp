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
* @param data pointer to input data array
*
* @return 0 On Success, else error number.
*
* @todo: Modify this so all data types can be handled not just the dobule.
*        Update for each new feature.
*/
int OFDMCodec::Encode(double *inputData)
{
    // QAM Encode data block
    QAMModulatorPlaceholder(inputData);
    // Transform data and put into the 
    m_fft.ComputeTransform();
    // Run band pass modulator
    m_bandPass.Modulate();
    // Add cyclic prefix
    //AddCyclicPrefix(outputData, m_Settings.nPoints*2 , m_Settings.cyclicPrefixSize);
    return 0;
}


/**
* Encodes one OFDM Symbol
* 
* @param data pointer to input data array
*
* @param data pointer to input data array
*
* @return 0 On Success, else error number.
*
* @todo: Modify this so all data types can be handled not just the dobule.
*        Update for each new feature.
*/
int OFDMCodec::Encode(double *inputData, double *outputData)
{
    // QAM Encode data block
    QAMModulatorPlaceholder(inputData);
    // Transform data and put into the 
    m_fft.ComputeTransform( (fftw_complex *) &outputData[m_Settings.cyclicPrefixSize]);
    // Run band pass modulator
    m_bandPass.Modulate(&outputData[m_Settings.cyclicPrefixSize]);
    // Add cyclic prefix
    AddCyclicPrefix(outputData, m_Settings.nPoints*2 , m_Settings.cyclicPrefixSize);
    return 0;
}


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
int OFDMCodec::QAMModulatorPlaceholder(double *data)
{
    // Copy Data
    for(uint16_t i = 0; i < m_Settings.nPoints; i++)
    {
        m_fft.in[i][0] = data[(i*2)];
        m_fft.in[i][1] = data[(i*2)+1];
    }
    return 0;
}


// Decoding Related Functions //


/**
* Decodes One OFDM Symbol
*
* @return 0 On Success, else error number.
*
* @todo: Update for each new feature.
*/
int OFDMCodec::Decode()
{
    uint32_t symbolStart = 0;
    int maxCorrelation = 0;
    maxCorrelation = m_correlator.FindSymbolStart();
    // if (maxCorrelationIndex >= 0)
    symbolStart = maxCorrelation +  m_Settings.cyclicPrefixSize;
    printf("OFDMCodec::Decode() maxCorrelation = %d\n", maxCorrelation);
    printf("OFDMCodec::Decode() symbolStart = %d\n", symbolStart);
    // Run Data thrgough bandpass demodulator
    m_bandPass.Demodulate(symbolStart);
    // Compute FFT & Normalise
    m_fft.ComputeTransform();
    m_fft.Normalise();
    return 0;
}


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
int OFDMCodec::QAMDemodulatorPlaceholder(double *data)
{
    for(uint32_t i = 0; i < m_Settings.nPoints; i++)
    {
        data[(i*2)] = m_fft.out[i][0];
        data[(i*2)+1] = m_fft.out[i][1];
    }  
    return 0;
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
int  OFDMCodec::Configure(OFDMSettings settingsStruct, double *buffer, uint32_t bufferSize)
{
    m_Settings = settingsStruct;
    m_fft.Configure(settingsStruct.nPoints, settingsStruct.type);

    if(settingsStruct.type == FFTW_BACKWARD)
    {
        m_bandPass.Configure(settingsStruct.nPoints, m_fft.out, buffer);
    }

    if(settingsStruct.type == FFTW_FORWARD)
    {
        m_correlator.Configure(settingsStruct.nPoints, settingsStruct.cyclicPrefixSize, buffer, bufferSize);
        m_bandPass.Configure(settingsStruct.nPoints, m_fft.in, buffer);
    } 

    return OK;
}


/**
* Update energy dispersal seed
* 
* @param seed integer for pseudo number generator (uint16_t)
*
* @return 0 On Success, else error number.
*
* @todo: Reconfiguration
*/
uint16_t SetEnergyDispersalSeed(uint16_t seed)
{
    
    return OK;
}


/**
* Get Current energy dispersal seed
*
* @return Currently configutred integer for pseudo number generator (uint16_t)
*
*/
uint16_t OFDMCodec::GetEnergyDispersalSeed()
{
    return m_Settings.EnergyDispersalSeed;
}


/**
* Sets the type of the codec
* 
* @param type Currently configured codec type(int)
*
* @return 0 On Success, else error number.
*
*/
uint16_t OFDMCodec::SetCodecType(int type)
{
    m_Settings.type = type;
    m_fft.Configure(m_Settings.nPoints, m_Settings.type);
    return OK;
}


/**
* Get codec type
*
* @return Currently configured codec type(int).
*
*/
int OFDMCodec::GetCodecType()
{
    return m_Settings.type;
}


/**
* Set points for the FFT 
* 
* @param newNPoints New number(uint16_t) of points for the FFT
*
* @return 0 On Success, else error number.
*
* @todo: Reconfiguration of appropriate objects in later releases
*/
uint16_t OFDMCodec::SetnPoints(uint16_t newNPoints)
{
    m_Settings.nPoints = newNPoints;
    m_fft.Configure(m_Settings.nPoints, m_Settings.type);
    return OK;
}


/**
* Get the FFT points number
*
* @return Currently configured number(uint16_t) of points for fft
*
*/
uint16_t OFDMCodec::GetnPoints()
{
    return m_Settings.nPoints;
}


/**
* Sets the time complexity for the FFT 
* 
* @param newComplexity time complexity(bool)
*
* @return 0 On Success, else error number.
*
* @todo: Reconfiguration of appropriate objects in later releases
*/
uint16_t OFDMCodec::SetTimeComplexity(bool newComplexity)
{
    return OK;
}


/**
* Get the time complexity for the FFT 
*
* @return 0 On Success, else error number.
*
* @todo:
*/
bool OFDMCodec::GetTimeComplexity()
{
    return m_Settings.complexTimeSeries;
}


/**
* Get Current pilot tone step
*
* @return Currently configured pilot tone step (uint16_t).
*
* @todo:
*/
uint16_t OFDMCodec::GetPilotToneStep()
{
    return m_Settings.pilotToneStep;
}

/**
* Obtains current pilot tone indicies vector.
*
* @return -
*
* @todo: Return pointer to the pilot tone vector
*/
uint16_t OFDMCodec::GetPilotTonesIndicies() // This should probably return an array of the pilotTones
{
    return OK;
}


/**
* Set new pilot tone step.
* 
* @param newPilotToneStep pilot tone step (uint16_t)
*
* @return 0 On Success, else error number.
*
* @todo: Reconfiguration of appropriate objects in later releases
*/
uint16_t OFDMCodec::SetPilotTones(uint16_t newPilotToneStep)
{
    return OK;
}


/**
* Sets the pilot tones, from a pilot tone position vector
* 
* @param newPilotToneVector new pilot tone positions
*
* @param nPilots number(uint16_t) of pilot tones in the vector 
*
* @return 0 On Success, else error number.
*
* @todo: Reconfiguration of appropriate objects in later releases
*/
uint16_t OFDMCodec::SetPilotTones(uint16_t newPilotToneVector[], uint16_t nPilots) // nPilots might not be needed.
{
    return OK;
}


/**
* Get pilot tone amplitude
*
* @return Currently configured pilot tone amplitude.
*
*/
uint16_t OFDMCodec::GetPilotTonesAmplitude()
{
    return m_Settings.pilotToneAmplitude;
} 


/**
* Set new pilot tone amplitude(for all tones) and reconfigure apropriate objects.
* 
* @param newPilotToneAmplitude
*
* @return 0 On Success, else error number.
*
* @todo: Reconfiguration of appropriate objects in later releases
*/
uint16_t OFDMCodec::SetPilotTonesAmplitude(float newPilotToneAmplitude)
{
    return OK;
}


/**
* Get the QAM scheme codec/modulator is using
*
* @return Currently configured QAM Size(uint16_t)
*
*/
uint16_t OFDMCodec::GetQAMSize()
{
    return m_Settings.QAMSize;
}


/**
* Sets the QAM scheme for data codec/modulator
* 
* @param newQAMSize (uint16_t) 
*
* @return  0 On Success, else error number. 
*
* @todo: Reconfiguration of appropriate objects in later releases
*/
uint16_t OFDMCodec::SetQAMSize(uint16_t newQAMSize)
{
    return OK;
}


/**
* Get currently configured cyclic prefix.
*
* @return Cyclic prefix size(uint16_t) 
*
*/
uint16_t OFDMCodec::GetCyclicPrefixSize()
{
    return m_Settings.cyclicPrefixSize;
}


/**
* Set new cyclic prefix and reconfigure all related objects
* 
* @param newCyclicPrefixSize Cyclic prefiz size in samples(uint16_t)
*
* @return 0 On Success, else error number.
*
* @todo: Reconfiguration of appropriate objects in later releases
*/
uint16_t OFDMCodec::SetCyclicPrefixSize(uint16_t newCyclicPrefixSize)
{
    return OK;
}
