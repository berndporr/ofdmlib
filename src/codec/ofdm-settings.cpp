/**
* @file ofdm-settings.cpp
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
#include "ofdm-settings.h"

#define BITS_IN_BYTE            8

// 4-QAM Parameters
#define BITS_PER_FREQ_POINT     2
#define FREQ_POINTS_PER_BYTE    4

/* Ranges for the OFDM settings that are set by the user
TYPE                NAME                    DESCRIPTION                                         RANGE                   REASONING
size_t              EnergyDispersalSeed     Seed used for rand                                  [Any +ve value]         As long as the value is kept constant it doesnt matter
size_t              nFFTPoints              Total number of FFT & IFFT coefficients             [ 2 < Power of two ]                  
size_t              PilotToneDistance       The distance between the pilot tones in a symbol    [ 0 < x < ]
double              PilotToneAmplitude      The amplitude of the pilot tones                    [Poistive]
size_t              PrefixSize              Cyclic prefix size in time-domain samples           [0 < x >= SymbolSize ]
MODULATION_SCHEME   QAMSize                 Modulation scheme used for sub-carriers             [Enum]
*/

/* Exit Error Codes   <--- TODO: This might not be best idea. maybe use error msg instead
STATUS      DESCRIPTION
0           No Errors
1           Invalid Settings
2           
*/

/**
* Constructor
*
*/
OFDMSettings::OFDMSettings(OFDMSettingsStruct settingsStruct)
{
    Configure(settingsStruct);
}


/**
* Check the validity of the settings specified
* and compute member variables which are
* needed for the ofdm objects.
*
*/
void OFDMSettings::Configure(OFDMSettingsStruct settingsStruct)
{
    //std::cout << "Configuring OFDM Settings Object!" << std::endl;
    // Do some basic error checking to prevent user from usinf useless codec 

    // Check if the type of codec is allowed
    if( (settingsStruct.type == FFTW_BACKWARD) || (settingsStruct.type == FFTW_FORWARD ) )
    {
        m_type = settingsStruct.type;
        //std::cout << "Codec type = " << m_type << std::endl;
    } 
    else
    {
        std::cout << "Error: Incorrect codec type!" << std::endl;
        exit(1);
    }
    
    // No error checking for the dispersal seed, it can take any value
    // TODO: check if this actually the case, see what rand can take in.
    m_EnergyDispersalSeed = settingsStruct.EnergyDispersalSeed; 

    // Check if the number of points is power of 2
    // In number is power of 2 in binary representation the count of 1 is one
    if(settingsStruct.nFFTPoints != 0 && (settingsStruct.nFFTPoints & (settingsStruct.nFFTPoints-1)) == 0)
    {
        m_nFFTPoints = settingsStruct.nFFTPoints;
        m_SymbolSize = m_nFFTPoints * 2;
        //std::cout << "N FFT Points = " << settingsStruct.nFFTPoints << std::endl;

    }
    else
    {
        std::cout << "Error: Number of FFT Points is not power of 2" << std::endl;
        exit(1);
    }

    // Check if the prefix size is greater than 0
    // AND Does no exceed the symbol size 
    if(settingsStruct.PrefixSize > 0 && settingsStruct.PrefixSize <=  m_SymbolSize )
    {
        m_PrefixSize = settingsStruct.PrefixSize;
        m_PrefixedSymbolSize = m_SymbolSize + m_PrefixSize;
        //std::cout << "Prefix Size = " << m_PrefixSize << std::endl;
        //std::cout << "Prefixed Symbol Size = " << m_PrefixedSymbolSize << std::endl;
    }
    else
    {
        std::cout << "Error: Prefix Size is not valid!" << std::endl;
        exit(1);
    }


    // Check if Pilot Distance is greater than 0
    // AND even
    // TODO: AND At least 2 Tones can be inserted into a symbol
    if( (settingsStruct.PilotToneDistance > 0) && ( (settingsStruct.PilotToneDistance % 2) == 0 ) )
    {
        m_PilotToneDistance = settingsStruct.PilotToneDistance; 
        //std::cout << "Pilot Tone Distance = " << m_PilotToneDistance << std::endl;
    }
    else
    {
        std::cout << "Error: Pilot Tone Distance is not valid, either all symbols " << std::endl;
        exit(1);
    }

    // TODO: Change this to use ENUM indicating the scheme 
    if( settingsStruct.QAMSize > 0 )
    {
        m_QAMSize = settingsStruct.QAMSize;
        //std::cout << "QAM - Bits Bytes Per Symbol = " <<  m_QAMSize << std::endl;
    }


    // Calculate number of avaiable sub-carriers for data
    // This depends on the size of the fft and pilot tone distance
    // TODO: Account for the DC point 
    m_nMaxDataSubCarriers = (m_nFFTPoints - (size_t)(m_nFFTPoints/m_PilotToneDistance));
    // Compute the equivelent of avaiable data bytes per symbol
    m_nMaxDataBytesPerSymbol = (size_t)((m_nMaxDataSubCarriers * BITS_PER_FREQ_POINT)  / BITS_IN_BYTE);

    // Check if user wants to encode a specific number of bytes in the symbol
    // AND the number of bytes is within the possible range
    if( (settingsStruct.nDataBytesPerSymbol > 0) && (settingsStruct.nDataBytesPerSymbol < m_nMaxDataBytesPerSymbol))
    {
        m_nDataBytesPerSymbol = settingsStruct.nDataBytesPerSymbol;
        //std::cout << " Data Bytes Per Symbol = " <<  m_nDataBytesPerSymbol << std::endl;
    }
    else
    {
        m_nDataBytesPerSymbol = m_nMaxDataBytesPerSymbol;
        std::cout << "Error: number of data bytes per symbol is not valid!" << std::endl;
        std::cout << "Using Maximum instead = " << m_nDataBytesPerSymbol << std::endl;
    }

    // Check if Pilot tone Amplitude is greater than 1
    if(settingsStruct.PilotToneAmplitude > 0)
    {
        m_PilotToneAmplitude = settingsStruct.PilotToneAmplitude;
        //std::cout << "Pilot Tone Amplitude = " << m_PilotToneAmplitude << std::endl;
    }
    else
    {
        std::cout << "Error: Pilot Tone Amplitude is not valid!" << std::endl;
        exit(1);
    }

    // Compute Sub-carrier locations of pilot tones & data
    ComputeLocationVectors();
}


/**
* Destructor 
*
*/
OFDMSettings::~OFDMSettings()
{

}

/**
* Computes the index values within a fft windows 
* where pilot tones and data sub-carriers
* This simplifies and speeds up various algorithms
* in the OFDM objects
*
*/
void OFDMSettings::ComputeLocationVectors()
{
    // Reset vectors
    m_PilotToneLocations.clear();
    m_DataSubCarrierLocations.clear();

    // Calculate number of pilot tones used in the symbol
    m_nMaxPilots = (size_t) (((m_nDataBytesPerSymbol*BITS_IN_BYTE)/BITS_PER_FREQ_POINT)/m_PilotToneDistance);
    //std::cout << "Number of Max Pilot tones = " << m_nMaxPilots << std::endl;

    // Compute starting frequency coefficient index for the negative frequency
    /*

    Freq
    ^
    |
    |
    |
    |               
    |               
    |               
    |               
    |               |
    |_______________|_______________-> Time
        +ve Freq   N/2   -ve Freq
                   D C
    */
    // Assume spectrum is centred symmetrically around DC( which is m_nFFTPoints/2) and depends on m_nDataBytesPerSymbol
    // The more points, larger spectrum, more sub-carriers, the closer to the DC point the stating index is
    m_SubCarrierStartIndex = (size_t) ((m_nFFTPoints) - (m_nMaxPilots/ 2) - ((m_nDataBytesPerSymbol * 4) / 2));
    //std::cout << "Sub-Carrier Start Index = " << m_SubCarrierStartIndex << std::endl;
    //size_t m_SubCarrierStartIndex = (((BITS_IN_BYTE*m_nDataBytesPerSymbol)/m_QAMSize) + m_nMaxPilots)/2;

    // Start insertion with negative frequencies
    size_t fftPointCounter = m_SubCarrierStartIndex;
    // Set Pilot counter 
    size_t pilotCounter = (size_t)(m_PilotToneDistance / 2);

    // Counter for to make sure FREQ_POINTS_PER_BYTE sub-carriers
    // are used for the data before bits from subsequent bytes are used 
    size_t insertionCounter = 0;
    size_t byteCounter = 0;

    while(byteCounter < m_nDataBytesPerSymbol)
    {
        // Reset fft point insertion counter 
        insertionCounter = 0;
        // While byte is being encoded
        while(insertionCounter < FREQ_POINTS_PER_BYTE)
        {
            // If pilot tone location
            if(pilotCounter == 0)
            {
                // Reset counter
                pilotCounter = m_PilotToneDistance;
                // Add pilot tone index
                m_PilotToneLocations.push_back(fftPointCounter);
            }
            // QAM Encoded point
            else
            {
                // Add index to the vector 
                m_DataSubCarrierLocations.push_back(fftPointCounter);
                // Indicate 2 bits have been inserted
                insertionCounter++;
                // Decrement pilot tone counter
                pilotCounter--;               
            }
            // Increment fft point counter to indicate complex point has been used
            fftPointCounter++;
            // Check if the counter exceeds the total number of points
            if(fftPointCounter == m_nFFTPoints)
            {
                // Wrap to positive frequencies
                fftPointCounter = 0;
            }
        }
        // One full byte has been encoded
        // Move to the next one
        byteCounter++;
        
    }
    //std::cout << "n Pilot Tone Locations = " << m_PilotToneLocations.size() << std::endl;
    //std::cout << "n Data Sub-Carriers = " << m_DataSubCarrierLocations.size() << std::endl;
}
