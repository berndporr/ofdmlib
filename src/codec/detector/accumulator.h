/**
* @file ofdm-settings.h
* @author Kamil Rog
*
* @section DESCRIPTION
* 
*/
#ifndef ACCUMULATOR_H
#define ACCUMULATOR_H

#include <common.h>
#include "ofdm-settings.h"
#include <string>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <math.h>



/**
 * @brief This Accumulator stores the sum of elements
 * 
 */
class Accumulator {

public: 

    /**
	* Constructor 
	*
	*/
	Accumulator(const OFDMSettings &settingsStruct);

    /**
	* Destructor 
	*
	*/
	~Accumulator();

    double ComputeFull(double *prefix, double *signal);
    double ProcessSample(double sample);

private:
	const OFDMSettings m_ofdmSettings;
    double *pData;
    double m_Output;
	double m_Sum;
	double m_lastElementValue;
	size_t m_lastIndex;


};


#endif
