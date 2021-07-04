/**
* @file cyclicprefix.cpp
* @author Kamil Rog
*
* @todo: DEAL WITH WRAPAROUND FOR INCOMING BLOCKS OF DATA
*/

#include "cyclicprefix.h"


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
int AutoCorrelator::Configure(uint16_t fftPoints, uint16_t prefixSize, double *pDouble, uint32_t buffSize)
{   
    // Set variables
    nPrefix = prefixSize;
    input = pDouble;
    symbolSize = fftPoints*2;
    threshold = 0.8;
    bufferSize = buffSize;
    configured = 1;
    return 0;
}

/**
* Computes correlation in time domain.
* This is done by accumulating the product of the flipped
* prefix and the expected end of the signal across the expected
* prefix lengths. 
* 
* @param Offset is the start of the prefix
* 
* @return correlation number.
*
*/
double AutoCorrelator::ExecuteCorrelator(uint32_t Offset)
{
    // Initialize output variable
    double correlation = 0;
    // Set new prefix end index
    uint32_t prefixIndex = nPrefix + Offset -1; // later on this variables can be calculated once and just incremented/decremented after function executes
    // Set new prefix start to the symbol's end
    uint32_t signalIndex = Offset + symbolSize; // later on this variables can be calculated once and just incremented/decremented after function executes
    // For each sample length of the prefix
    for(int i = 0; i < nPrefix; i++)
    {
        // Multiply the signal with the flipped delayed version of itself
        correlation += input[prefixIndex] * input[signalIndex];
        // Move indicies
        prefixIndex--;
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
int AutoCorrelator::FindSymbolStart()
{   
    uint32_t CoarseStart = 0;
    uint32_t FineStart = 0;
    // Coarse search
    CoarseStart = CoarseSearch();

    if(CoarseStart > 0)
    {
        // Pilot Tone Search
        FineStart = 0;
    }
    // Return Symbol start
    return FineStart;
}


/**
* Searches for the symbol start using correlator
* on rx signal to find expected prefix.
* 
* @return symbol start(integer) index, else -1
*
*/
int AutoCorrelator::CoarseSearch()
{
    bool startNotFound = true;
    double correlation = 0;
    int bufferCounter = bufferSize;
  
    //uint32_t startOffset = 0;
	//uint32_t offset = 0;

    // While the start of the symbol has not been found
    while(startNotFound) // Maybe set maximum itterations?
    {
        // Calculate correlation for an offset
        correlation = ExecuteCorrelator(startOffset);
        // If the correlation exceeds the threshold
        if(correlation >= threshold)
        {
            // Symbol start has been found
            startNotFound == false;
            // return the offset at which the symbol was found
            return startOffset;
        }
        // The coarse search has not been sucessfull,
        // Increment offset
        startOffset++;
    }
    return -1;
}


/**
* Sets the buffer pointer to null, variables and flasgs to zero
* 
* @return 0 on success, else error number
*
*/    
int AutoCorrelator::Close()
{   
    nPrefix = 0;
    input = nullptr;
    symbolSize = 0;
    bufferSize = 0;
    configured = 0;
    return 0;
}


