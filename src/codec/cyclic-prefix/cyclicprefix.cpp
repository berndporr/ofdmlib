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
int Correlator::Configure(uint16_t fftPoints, uint16_t prefixSize, double *pDouble, uint32_t buffSize)
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
double Correlator::ExecuteCorrelator(uint32_t prefixOffset)
{
    // Initialize output variable
    double correlation = 0;
    // Set new prefix start to the symbol's end
    uint32_t signalIndex = prefixOffset + symbolSize; 
    // For each sample length of the prefix
    for(int i = prefixOffset; i < nPrefix+prefixOffset; i++)
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
int Correlator::FindSymbolStart()
{   
    uint32_t CoarseStart = 0;
    uint32_t FineStart = 0;
    uint32_t symbolStart = -1;

    // Coarse search
    CoarseStart = CoarseSearch();

    if(CoarseStart >= 0)
    {
        // Pilot Tone Search
        //  symbolStart = PilotToneSearch();
        symbolStart = CoarseStart;
    }
    // Return Symbol start
    return symbolStart;
}


/**
* Searches for the symbol start using correlator
* on rx signal to find expected prefix.
* 
* @return symbol start(integer) index, else -1
*
*/
int Correlator::CoarseSearch()
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
        correlation = ExecuteCorrelator(startOffset);
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
            printf("Correlator::CoarseSearch() maxValue = %f\n", maxValue);
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
int Correlator::Close()
{   
    nPrefix = 0;
    input = nullptr;
    symbolSize = 0;
    bufferSize = 0;
    configured = 0;
    return 0;
}


