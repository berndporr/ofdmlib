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



/**
* DESC
* 
* @param
*
* @return
*
* @todo:
*/
uint16_t OFDMCodec::SetFFTDirection(int direction)
{


    return OK;
}


/**
* DESC
* 
* @param
*
* @return
*
* @todo:
*/
uint16_t OFDMCodec::GetFFTDirection()
{



    return OK;
}


/**
* DESC
* 
* @param
*
* @return
*
* @todo:
*/
uint16_t OFDMCodec::SetnPoints(uint16_t newNPoints)
{


}


/**
* DESC
* 
* @param
*
* @return
*
* @todo:
*/
uint16_t OFDMCodec::GetnPoints()
{


}


/**
* DESC
* 
* @param
*
* @return
*
* @todo:
*/
uint16_t OFDMCodec::SetTimeComplexity(bool newComplexity)
{

}


/**
* DESC
* 
* @param
*
* @return
*
* @todo:
*/
uint16_t OFDMCodec::GetTimeComplexity()
{

}


/**
* DESC
* 
* @param
*
* @return
*
* @todo:
*/
uint16_t OFDMCodec::GetPilotTonesIndicies() // This should probably return an array of the pilotTones
{

}


/**
* DESC
* 
* @param
*
* @return
*
* @todo:
*/
uint16_t OFDMCodec::SetPilotTones(uint16_t newPilotToneStep)
{

}


/**
* DESC
* 
* @param
*
* @return
*
* @todo:
*/
uint16_t OFDMCodec::SetPilotTones(uint16_t newPilotToneSequence[], uint16_t nPilots) // nPilots might not be needed.
{

}


/**
* DESC
* 
* @param
*
* @return
*
* @todo:
*/
uint16_t OFDMCodec::GetGuardInterval()
{

}


/**
* DESC
* 
* @param
*
* @return
*
* @todo:
*/
uint16_t OFDMCodec::SetGuardInterval(uint16_t newGuardInterval)
{

}


/**
* DESC
* 
* @param
*
* @return
*
* @todo:
*/
uint16_t OFDMCodec::GetQAMSize()
{

}

/**
* DESC
* 
* @param
*
* @return
*
* @todo:
*/
uint16_t OFDMCodec::SetQAMSize(uint16_t newQAMSize)
{

}


/**
* DESC
* 
* @param
*
* @return
*
* @todo:
*/
uint16_t OFDMCodec::GetCyclicPrefixSize()
{

}


/**
* DESC
* 
* @param
*
* @return
*
* @todo:
*/
uint16_t OFDMCodec::SetCyclicPrefixSize(uint16_t newCyclicPrefixSize)
{

}
