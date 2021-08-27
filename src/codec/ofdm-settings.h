/**
* @file ofdm-settings.h
* @author Kamil Rog
*
* @section DESCRIPTION
* 
*/
#ifndef OFDM_SETTINGS_H
#define OFDM_SETTINGS_H

#include <string>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <math.h>

#include <common.h>


/**
 * @brief OFDM codec settings class
 * This object encapsulates all settings
 * related ofdm codec and objects. It is 
 * provides error checking which prevents
 * use of invalid ofdm scheme.
 * 
 */
class OFDMSettings {

public: 

    /**
	* Constructor 
	*
	*/
	OFDMSettings(OFDMSettingsStruct settingsStruct);

    /**
	* Destructor 
	*
	*/
	~OFDMSettings();

    void ComputeLocationVectors();
    void Configure(OFDMSettingsStruct settingsStruct);

public:

    int    m_type;
    size_t m_EnergyDispersalSeed;
    size_t m_nFFTPoints;
    size_t m_SymbolSize;
    size_t m_PrefixSize;
    size_t m_PrefixedSymbolSize;
    size_t m_PilotToneDistance;
    size_t m_nMaxDataSubCarriers;
    size_t m_nMaxPilots;                    // Depends on m_nDataBytesPerSymbol
    size_t m_nMaxDataBytesPerSymbol;
    size_t m_nDataBytesPerSymbol;
    size_t m_SubCarrierStartIndex;
    size_t m_QAMSize;
    double m_PilotToneAmplitude;
    SizeTVec m_PilotToneLocations;
    SizeTVec m_DataSubCarrierLocations;

};


#endif
