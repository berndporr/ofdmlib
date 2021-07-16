/**
* @file qam-modulator.h
* @author Kamil Rog
*
* @section DESCRIPTION
* 
*/
#ifndef QAM_MODULATOR_H
#define QAM_MODULATOR_H

#include <iostream>
#include "common.h"

/**
 * @brief QAM modulator
 * 
 */
class QamModulator {

public: 

	QamModulator(size_t fftPoints, size_t pilotToneStep, size_t energyDispersalSeed, size_t QAM) :
        m_nFFT(fftPoints),
        m_pilotToneStep(pilotToneStep),
        m_EnergyDispersalSeed(energyDispersalSeed),
        m_QAMsize(QAM)
    {

	}

    /**
	* Destructor 
	*
	*/
	~QamModulator()
	{

	}

    template<typename T, typename A> void Encode(const std::vector<T,A>  &input, DoubleVec &output);
    template<typename T, typename A> void Decode(DoubleVec &input, std::vector<T,A> &output);


private:

    size_t m_nFFT;
    size_t m_pilotToneStep;
    size_t m_EnergyDispersalSeed;
    size_t m_QAMsize;

};

/**
* Template QAM modulator encoding function
* 
* @param input reference to input data array to be encoded
* @param output reference to output data array, this is most likley the ifft input
*
*/
template<typename T, typename A> 
inline void QamModulator::Encode(const std::vector<T,A>  &input, DoubleVec &output)
{

    size_t typeBitSize = sizeof(T);
    /*
    size_t nAvaiableifftPoints =  (m_nFFT - (int)(m_nFFT/m_pilotToneStep));
    size_t nFullEncodedElements = (int)((nAvaiableifftPoints *  m_QAMsize)  / typeBitSize);
    size_t nLastElementBitsNotEncoded =  ((nAvaiableifftPoints *  m_QAMsize)  % typeBitSize);
    */

    std::cout << "Encode Type Size =  " << typeBitSize << std::endl;
}


/**
* Template QAM Modulator decoding function
* 
* @param input reference to input data array
* @param output reference to output data array
*
*/
template<typename T, typename A> 
inline void QamModulator::Decode(DoubleVec &input, std::vector<T,A> &output)
{

    size_t typeBitSize = sizeof(T);
    /*
    size_t nAvaiableifftPoints =  (m_nFFT - (int)(m_nFFT/m_pilotToneStep));
    size_t nFullEncodedElements = (int)((nAvaiableifftPoints *  m_QAMsize)  / typeBitSize);
    size_t nLastElementBitsNotEncoded =  ((nAvaiableifftPoints *  m_QAMsize)  % typeBitSize);
    */
    std::cout << "Decode Type Size =  " << typeBitSize << std::endl; 
}
#endif
