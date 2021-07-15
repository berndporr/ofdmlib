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
int Detector::Configure(uint32_t fftPoints, uint32_t prefixSize, ofdmFFT *fft, NyquistModulator* nyquist)
{   
    // Set variables
    nPrefix = prefixSize;
    //input = pDouble;
    symbolSize = fftPoints*2;
    threshold = 0.8;
    configured = 1;
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
    size_t signalIndex = prefixOffset + symbolSize; 
    // For each sample length of the prefix
    for(size_t i = prefixOffset; i < nPrefix+prefixOffset; i++)
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
size_t Detector::FindSymbolStart(const DoubleVec &input)
{   
    size_t coarseStart = 0;
    size_t symbolStart = 0;

    // Coarse search
    coarseStart = CoarseSearch(input);
    coarseStart += nPrefix;

    if(coarseStart >= 0)
    {
        // Pilot Tone Search
        symbolStart = FineSearch(input, coarseStart);
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
size_t Detector::FineSearch(const DoubleVec &buff, size_t coarseStart)
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
        sumOfImag = abs(pFFT->GetImagSum());
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
size_t Detector::CoarseSearch(const DoubleVec &input)
{
    bool startNotFound = true;
    double correlation = 0;
    bool thresholdExceeded = false;

    double maxValue = 0.0;
    uint32_t maxValueIndex = 0;

    // While the start of the symbol has not been found
    while(startNotFound) // Maybe set maximum itterations?
    {
        // Calculate correlation for a given offset
        correlation = ExecuteCorrelator(input, startOffset);
        // If the correlation exceeds the threshold
        if(correlation >= threshold)
        {
            // Set the flag to true
            thresholdExceeded = true;
            // Check if the value is higher than current max
            if(maxValue <= correlation)
            {
                // Set the max value
                maxValue = correlation;
                // Set the index at which this occured
                maxValueIndex = startOffset;
            }

        }

        // If the sample value falls below threshold, and it was previously exceeded
        if((correlation <= threshold) && (thresholdExceeded  == true) )
        {
            // Symbol start has been found
            startNotFound = false;
            // return the index at which the max value has occured
            // This is most likley the max correlation point for the first symbol in the RxSignal
            return maxValueIndex;
        }

        // The coarse search for this offset value has not been sucessfull,
        // Increment offset
        startOffset++;
        // TODO: Need to add extra logic to tell the function when the whole buffer has been searched

    }
    // Whole buffer searched, no symbol has been detected
    return -1;
}


/**
* Sets the buffer pointer to null, variables and flasgs to zero
* 
* @return 0 on success, else error number
*
*/    
int Detector::Close()
{   
    nPrefix = 0;
    symbolSize = 0;
    configured = 0;
    return 0;
}
