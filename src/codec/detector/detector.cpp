/**
* @file detector.cpp
* @author Kamil Rog
*
* @todo: DEAL WITH WRAPAROUND FOR INCOMING BLOCKS OF DATA
*/

// Plotting library 
#include <vector>
#include "gnuplot-iostream.h"
#include "detector.h"


Detector::Detector(OFDMSettings &settings, FFT &fft, NyquistModulator &nyquist) :
    m_ofdmSettings(settings),
    rFFT(fft),
    rNyquistModulator(nyquist),
    m_threshold(250),
    m_SearchRange(25),
    m_StartOffset(0),
    m_LastPeakCounter(0)
{
    m_SymbolSize = m_ofdmSettings.nFFTPoints * 2;
    m_PrefixedSymbolSize = m_SymbolSize + m_ofdmSettings.PrefixSize;
}

/**
* Default Destructor 
*
*/
Detector::~Detector()
{

}


/**
* Computes correlation in time domain.
* This is done by accumulating the product of the prefix
* and the end of the expected symbol location across the expected
* prefix length. 
* 
* @param prefixOffset An index of the start of the prefix
* 
* @return correlation result
*
*/
double Detector::ExecuteCorrelator(const double *input, size_t prefixOffset)
{
    // Initialize output correlation variable
    double correlation = 0;
    // Set the signal index to the symbol's end start, 
    // which in theory coresponds to the prefix 
    size_t signalIndex = prefixOffset + m_SymbolSize; 
    // For each sample length of the prefix size
    for(size_t i = prefixOffset; i < m_ofdmSettings.PrefixSize+prefixOffset; i++)
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
long int Detector::FindSymbolStart(const double *input, size_t nBytes)
{   
    long int  coarseStart = 0;
    size_t symbolStart = 0;

    // Coarse search
    coarseStart = CoarseSearch(input);
    if(coarseStart >= 0)
    {
        // Add prefix size to get the actual symbol start 
        coarseStart += m_ofdmSettings.PrefixSize;
        // Pilot Tone Search
        symbolStart = FineSearch(input, coarseStart, nBytes);
        // Return symbol start
        return symbolStart;
    }
    return -1;
}


/**
* Computes the value of the imaginary part of the pilot tones
* locations. This is a wrapper for the routine that 
* for given buffer, demodulates the rx samples into
* fft input array, computes the transform, normalises
* the result and computes the sum of the imaginary
* compoment of the pilot tone locations.
* 
* @param buff start of the symbol found by coarse search
* @param offset the expected start of the symbol in the rx samples (NOTE This is not prefixed symbol start)
* @param nbytes expected number of bytes in the symbol
*
* @return Sum of imaginary components of the pilot locations in the ofdm symbol
*
*/
double Detector::ComputeSumOfImag(const double *buff, size_t offset, size_t nbytes)
{
    // Use Nyquist Demodulator to put samples into fft input buffer
    rNyquistModulator.Demodulate(buff, rFFT.in, offset);
    // Compute FFT
    rFFT.ComputeTransform();
    // Normalise FFT
    rFFT.Normalise();
    // Compute & return the sum of imaginary components
    return rFFT.GetImagSum(nbytes);
}


/**
* Searches for the symbol start by assessing the 
* value of the imaginary components of the pilot tone
* locations. Firstly restrict the search to the boundaries
* of the provided buffer, then for the calculated search range
* compute the sum of the imaginary components of the pilot
* tone locations and return the index at which the lowest value
* occurs.
* 
* @param coarseStart start of the symbol found by coarse search
*
* @return fine symbol start index
* TODO: The symbol start can be in the last Rx buffer 
*/
size_t Detector::FineSearch(const double *buff, size_t coarseStart, size_t nbytes)
{
    // Set min to the max value of double
    double min = std::numeric_limits<double>::max();
    // Initialize variables to zero
    double sumOfImag = 0.0;
    size_t lowestImgIndex = 0;
    size_t startIndex = 0;

    // For DEBUG ONLY: Create vector to store sums
    std::vector<double> SumOfImagVec; 

    // For given Coarse start index 
    // Restric start index of fine search to 0th element of the buffer
    // If the result of the start index is greater or equal to zero 
    if( ((m_SearchRange-1) / 2) >= 0)
    {
        // Set the start index as intended
        startIndex = coarseStart - (size_t)((m_SearchRange-1) / 2);
    }
    // Start index outside the boundaries of the array
    else
    {
        // Set the start index to zero
        startIndex = 0;
    }

    // TODO: Handle an edge case where the stop index is outside the boundaries 
    size_t stopIndex = coarseStart + (int)((m_SearchRange-1) / 2); 
    
    // For each offset from start to stop index 
    for(size_t i = startIndex; i < stopIndex; i++)
    {
        // Compute sum of imaginary components of the expected pilot tone locations
        sumOfImag = ComputeSumOfImag(buff, i, nbytes);
        SumOfImagVec.push_back(sumOfImag); // For DEBUG ONLY: Append sum to the end of the vector to plot later
        // If calculated Sum of imaginary components is less than current minimum
        if( sumOfImag < min )
        {
            // Save minimum value
            min = sumOfImag;
            // Save index at which this occured
            lowestImgIndex = i;
        }
    }

    // For DEBUG ONLY
    // Plot Sum of Imag for the calculated search range
    //Gnuplot gp;
    //gp << "plot '-' with line title 'Detector - Fine Search - Sum of Imag'\n";
    //gp.send1d(SumOfImagVec);   
    
    // Return the index where the sum of the imaginary parts is the lowest
    return lowestImgIndex;
}


/**
* Searches for the prefixed symbol start using correlator
* on the specified input buffer to find expected prefix.
* 
* @return start of the prefix index, else -1
*
*/
long int Detector::CoarseSearch(const double *input)
{
    // Initialize return value to indicate coarse search has not found the symbol start
    long int returnValue = -1; 
    
    bool startNotFound = true;
    bool thresholdExceeded = false;

    double correlation = 0;
    double maxValue = 0.0;

    // Check if the last peak counter indicates the peak has been recently found
    if((m_LastPeakCounter > 0) )
    {
        // Check if the symbol peak can occur in this buffer
        if((m_LastPeakCounter - m_PrefixedSymbolSize ) <= 0 )
        {
            // Skip This buffer
            m_StartOffset = m_PrefixedSymbolSize;
            // Update last peak counter
            m_LastPeakCounter -= m_PrefixedSymbolSize;
        }
        // Start with the offset where correlation can be calculated
        else
        {
            // Update offset
            m_StartOffset = m_PrefixedSymbolSize - m_LastPeakCounter;
            // Reset last peak counter
            m_LastPeakCounter = 0;
        }
    }

    
    // While the start of the symbol has not been found
    while(startNotFound)
    {
        // Check if the whole allowed buffer has been searched
        if(m_StartOffset == m_PrefixedSymbolSize) // NOTE: If the real-time buffer size changes this must too
        {
            // Reset buffer offset variable 
            m_StartOffset = 0;
            // Break out of while loop
            break;
        }

        // Check if a reasonable quantity of samples elapes since the last detected peak
        if(m_LastPeakCounter > 0)
        {
            // Decrement Counter
            m_LastPeakCounter--;
        }
        // Sufficient number of samples passed
        // Can compute correlation
        else
        {
            // Calculate correlation for a given offset
            correlation = ExecuteCorrelator(input, m_StartOffset);
            // Square correlation to make the result +ve and further spearate peaks and noise
            correlation *= correlation;
            // DEBUG ONLY: Append correlation to plot buffer
            corOutput.push_back(correlation); 
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
                    //maxValueIndex = m_StartOffset;
                    // Update retrun value
                    returnValue = m_StartOffset;
                }

            }

            // If the sample value falls below threshold, and it was previously exceeded
            if((correlation <= m_threshold) && (thresholdExceeded == true) )
            {
                // Reset flag
                thresholdExceeded = false;
                // Reset offset
                m_StartOffset = 0;
                // Symbol start has been found
                startNotFound = false;
                // break while loop
                break; 
            }
        }

        // Increment offset
        m_StartOffset++;
    }

        // Debug only
        plotCounter++;
        if(plotCounter == 6)
        {
            // Plot Corellator output
            Gnuplot gp;
            gp << "plot '-' with line title 'Detector - Correlation'\n";
            gp.send1d(corOutput);
        }

    // If symbol start has been found
    if(returnValue >= 0 )
    {
        // Set Last Peak counter
        // Compute how many points since the peak occured
        // If the return value is greater than symbol size
        if( (m_PrefixedSymbolSize - returnValue) <= 0)
        {
            // Restrict last peak counter value to prefixed symbol size
            // TODO: But allow quarter size of the prefix samples 
            m_LastPeakCounter = m_PrefixedSymbolSize;
        }
        else
        {
            // Compute the number of samples left since the peak in the buffer occured
            m_LastPeakCounter = m_PrefixedSymbolSize - returnValue; 
        }
    }


    return returnValue;
}
