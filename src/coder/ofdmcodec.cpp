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

/**
 * @brief OFDM coder object class
 * This object encapsulates the encoding 
 * and decoding functionlaity. Esentially it
 * is a wrapper of all OFDM related objects with
 * configurable settings.
 * 
 */


// Encoding Related Functions


/**
* Encodes one OFDM Symbol
* 
* @param data pointer to float array
*
* @return 0 On Success, else error number.
*
* @todo: Modify this so all data types can be handled not just the float.
*/
int OFDMCodec::Encode(float *data)
{
    QAMModulatorPlaceholder(data);
    m_fft.ComputeTransform();
    return 0;
}


/**
* A place holder function for QAM Modulator 
* It just copies over the data from one aray into fft object input
* 
* @param
*
* @return 0 On Success, else error number.
*
* @todo: This needs to be implemented for release v0.5
*/
int OFDMCodec::QAMModulatorPlaceholder(float *data)
{
    // Generate random floats 
    for(uint16_t i = 0; i < m_Settings.nPoints; i++)
    {
        m_fft.in[i][0] = data[(i*2)];
        m_fft.in[i][1] = data[(i*2)+1];
    }
    return 0;
}


// Decoding Related Functions

/**
* Decodes One OFDM Symbol
* 
* @param
*
* @return 0 On Success, else error number.
*
* @todo: This needs to be implemented for release v0.5
*/
int OFDMCodec::Decode()
{
    m_fft.ComputeTransform();
    return 0;
}


// Settings Functions //

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
* @todo: Reconfiguration in later releases
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
*
* @return Currently configured number(uint16_t) of points for fft
*
* @todo: Reconfiguration in later releases
*/
uint16_t OFDMCodec::GetnPoints()
{
    return m_Settings.nPoints;
}


/**
* Sets the Time complexity for the FFT 
* 
* @param newComplexity (bool)
*
* @return 0 On Success, else error number.
*
* @todo: Reconfiguration in later releases
*/
uint16_t OFDMCodec::SetTimeComplexity(bool newComplexity)
{
    return OK;
}


/**
* DESC
* 
* @param
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
* @param
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
* DESC
* 
* @param
*
* @return 0 On Success, else error number.
*
* @todo:
*/
uint16_t OFDMCodec::GetPilotTonesIndicies() // This should probably return an array of the pilotTones
{

}


/**
* Set new pilot tone step.
* 
* @param newPilotToneStep pilot tone step (uint16_t)
*
* @return 0 On Success, else error number.
*
* @todo:
*/
uint16_t OFDMCodec::SetPilotTones(uint16_t newPilotToneStep)
{
    return OK;
}


/**
* DESC
* 
* @param
*
* @return 0 On Success, else error number.
*
* @todo:
*/
uint16_t OFDMCodec::SetPilotTones(uint16_t newPilotToneSequence[], uint16_t nPilots) // nPilots might not be needed.
{

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
* Set new pilot tone amplitude and reconfigure apropriate objects.
* 
* @param newPilotToneAmplitude
*
* @return 0 On Success, else error number.
*
* @todo:
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
* @todo:
*/
uint16_t OFDMCodec::GetQAMSize()
{
    return m_Settings.QAMSize;
}

/**
* Sets the QAM scheme for data modulator
* 
* @param newQAMSize (uint16_t) 
*
* @return  0 On Success, else error number. 
*
* @todo:
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
* @todo:
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
* @todo:
*/
uint16_t OFDMCodec::SetCyclicPrefixSize(uint16_t newCyclicPrefixSize)
{
    return OK;
}
