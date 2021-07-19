/**
* @file detector.cpp
* @author Kamil Rog
*
* @todo: DEAL WITH WRAPAROUND FOR INCOMING BLOCKS OF DATA
*/

#include "detector.h"
#include <cstddef>


/**
* Configures the AutoCorrelator object 
* by setting the number of expected sizes and pointers 
* to the Rx signal.
* 
* @param nPoints 
* 
* @return 0 on success, else error number
*
*/
int Detector::Configure(size_t fftPoints, size_t prefixSize, ofdmFFT *fft, NyquistModulator* nyquist)
{   
    // Set variables
    m_nPrefix = prefixSize;
    m_symbolSize = fftPoints*2;
    m_threshold = 0.8;
    m_configured = 1;
    m_SearchRange = 25;
    pFFT = fft;
	pNyquistModulator = nyquist;
    return 0;
}

/**
* Computes correlation in time domain.
* This is done by accumulating the product of the prefix
* and the end of the expected symbol location across the expected
* prefix location. 
* 
* @param prefixOffset An index of the start of the prefix
* 
* @return correlation result
*
*/
double Detector::ExecuteCorrelator(const DoubleVec &input, size_t prefixOffset)
{
    // Initialize output variable
    double correlation = 0;
    // Set new prefix start to the symbol's end
    size_t signalIndex = prefixOffset + m_symbolSize; 
    // For each sample length of the prefix
    for(size_t i = prefixOffset; i < m_nPrefix+prefixOffset; i++)
    {
        // Multiply the signal with delayed version of itself
        correlation += input[i] * input[signalIndex];
        // Move indicies
        signalIndex++;
    }
    // Return result
    return correlation;
}


/**
* Computes the first symbol start in the provided input buffer
* Firstly perform a coarse search (on prefix) and then fine search
* (on pilot tones) 
* 
* @param Offset is the start of the prefix
* 
* @return symbol start(integer) index.
*
*/
size_t Detector::FindSymbolStart(const DoubleVec &input, size_t nBytes)
{   
    size_t coarseStart = 0;
    size_t symbolStart = 0;

    // Coarse search
    coarseStart = CoarseSearch(input);
    coarseStart += m_nPrefix;

    if(coarseStart >= 0)
    {
        // Pilot Tone Search
        symbolStart = FineSearch(input, coarseStart, nBytes);
        //symbolStart = CoarseStart;
    }
    // Return Symbol start
    return symbolStart;
}


/**
* Searches for the symbol start by assessing the 
* value of the imaginary part of the pilot tones
* locations.
* 
* @param coarseStart start of the symbol found by coarse search
*
* @return fine symbol start index, else -1
*
*/
size_t Detector::FineSearch(const DoubleVec &buff, size_t coarseStart, size_t nbytes)
{
    double min = 100000;
    double sumOfImag = 0.0;
    int lowestImgIndex = 0;

    size_t startIndex = 0;
    // Restric start index of fine search to 0th element
    if( startIndex >= 0)
    {
        startIndex = coarseStart - (int)((m_SearchRange-1) / 2);
    }
    else
    {
        startIndex = 0;
    }

    // TODO: Handle an exception where the stop index is outside the boundaries 
    size_t stopIndex = coarseStart + (int)((m_SearchRange-1) / 2); 

    for(size_t i = startIndex; i < stopIndex; i++)
    {
        // Demodulate
        pNyquistModulator->Demodulate(buff, i);
        // Compute FFT
        pFFT->ComputeTransform();
        // Normalise
        pFFT->Normalise();
        // SUM imag parts
        sumOfImag = abs(pFFT->GetImagSum(nbytes));
        if( sumOfImag < min )
        {
            min = sumOfImag;
            lowestImgIndex = i;
        }
    }
    return lowestImgIndex;
}


/**
* Searches for the symbol start using correlator
* on rx signal to find expected prefix.
* 
* @return symbol start(integer) index, else -1
*
*/
size_t Detector::CoarseSearch(const DoubleVec &input) // change return type to 
{
    bool startNotFound = true;
    double correlation = 0;
    bool thresholdExceeded = false;

    double maxValue = 0.0;
    size_t maxValueIndex = 0;

    // While the start of the symbol has not been found
    while(startNotFound) // Maybe set maximum itterations?
    {
        // Calculate correlation for a given offset
        correlation = ExecuteCorrelator(input, m_startOffset);
        // If the correlation exceeds the threshold
        if(correlation >= m_threshold)
        {
            // Set the flag to true
            thresholdExceeded = true;
            // Check if the value is higher than current max
            if(maxValue <= correlation)
            {
                // Set the max value
                maxValue = correlation;
                // Set the index at which this occured
                maxValueIndex = m_startOffset;
            }

        }

        // If the sample value falls below threshold, and it was previously exceeded
        if((correlation <= m_threshold) && (thresholdExceeded  == true) )
        {
            // Symbol start has been found
            startNotFound = false;
            // return the index at which the max value has occured
            // This is most likley the max correlation point for the first symbol in the RxSignal
            return maxValueIndex;
        }

        // The coarse search for this offset value has not been sucessfull,
        // Increment offset
        m_startOffset++;

        // TODO: Need to add extra logic to tell the function when the whole buffer has been searched
        if( m_startOffset == input.size() )
        {
            m_startOffset = 0;
            // Whole buffer searched, no symbol has been detected
            return -1;
        }
    }
    return 0;
}


/**
* Sets the buffer pointer to null, variables and flasgs to zero
* 
* @return 0 on success, else error number
*
*/    
int Detector::Close()
{   
    m_nPrefix = 0;
    m_symbolSize = 0;
    m_configured = 0;
    return 0;
}
