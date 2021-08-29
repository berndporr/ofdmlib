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
	Accumulator(const size_t size);

    /**
	* Destructor 
	*
	*/
	~Accumulator();

    double ProcessFullSet(double *buffer, size_t prefix, size_t signal);
    double ProcessSample(double sample);
	void Reset();

private:
    double *pData;
	double m_Sum;
	size_t m_nSamples;
	size_t m_LastSampleIndex;


};


#endif
