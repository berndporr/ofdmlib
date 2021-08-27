/**
* @file detector.cpp
* @author Kamil Rog
*
*/

// Plotting library
#include <vector>
#include "gnuplot-iostream.h"
#include "detector.h"

/**
* Constructor
* 
* @param settings
* @param fft reference to the fft objecct
* @param nyquist reference to the nyquist modulator objecct
*
*/
Detector::Detector(const OFDMSettings &settings, FFT &fft, NyquistModulator &nyquist) :
    m_ofdmSettings(settings),
    rFFT(fft),
    rNyquistModulator(nyquist),
    m_UpperThreshold(1), // Value for Tests 35000000000
    m_LowerThreshold(0.3), // Value for Tests  10000000000
    m_SearchRange(25),
    m_LastPeakCounter(0)
{
    // Error checking for the threshold

    // Initialize correlator indicies
    m_PrefixIndex = 0;
    m_SignalIndex = m_ofdmSettings.m_SymbolSize;
    m_OffsetCounter = 0;
    m_nFromPeak = 0;

    m_nBlocks = 3; // Must be at least 3
    // Allocate memory for ring buffer
    m_BlockRingBuffer = (double*) calloc(m_ofdmSettings.m_PrefixedSymbolSize*m_nBlocks, sizeof(double));
    // Initialize track variables for the buffer
    m_BlockRingBufferCorrelatorPostion = 0; 
    m_BlockRingBufferDataPostion = 1; 

    m_RingBufferEdge = m_nBlocks*m_ofdmSettings.m_PrefixedSymbolSize;

	m_ThresholdFlag = false;
	m_CorrelatorMaxValue = -1;
	m_CorrelatorMaxValueIndex = 0;
}

/**
* Default Destructor 
*
*/
Detector::~Detector()
{
    free(m_BlockRingBuffer);
}


/**
* Increments the specified variable by and wraps around
* if neccessary.
* 
* @param variable reference to the variable being incremented
*
*/
void Detector::Increment(size_t &variable)
{
    // Less than boundary
    if( (variable + 1) < m_RingBufferEdge )
    {
        variable++;
    }
    // Wrap around
    else
    {
        variable = 0;
    }
}


/**
* Increments the specified variable by N and wraps around
* if neccessary.
* 
* @param variable reference to the variable being incremented
*
* @param n number of incrementations
*
*/
void Detector::IncrementByN(long int &variable, size_t n)
{
    size_t newIndex = 0;
    // Safe to increment by n;
    if( variable + n < m_RingBufferEdge )
    {
        variable += n;
    }
    // Check if m_PrefixIndex exceeds the boundary of ring buffer
    else
    {
        // Wrap around
        newIndex = ((variable + n ) - m_RingBufferEdge);
        variable = newIndex;
    }
}


/**
* Computes correlation in time domain.
* This is done by accumulating the product of the prefix
* which should correspond to the expected symbol for the 
* length of the prefix. 
* 
* @param prefixOffset An index of the start of the prefix
* 
* @return correlation result
*
*/
double Detector::ExecuteCorrelator()
{
    // Initialize output correlation variable
    double correlation = 0;
    // Set the signal index to the symbol's end start, 
    // which in theory coresponds to the prefix 
    // For each sample length of the prefix size
    size_t p = m_PrefixIndex;
    size_t s = m_SignalIndex;
    // Initialize element counter
    size_t i = 0;
    // Compute the sum of a product untill the whole length of the prefix
    while(i != m_ofdmSettings.m_PrefixSize)
    {
        // Multiply the samples of the signal which correspond to the prefix and symbol
        correlation += m_BlockRingBuffer[p] * m_BlockRingBuffer[s];
        // Increment indicies
        Increment(p);
        Increment(s);
        // Increment element counter
        i++;
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
long int Detector::FindSymbolStart(const double *input)
{   
    long int  coarseStart = 0;
    long int returnValue = -1;

    // Insert data block into block ring buffer
    memcpy(&m_BlockRingBuffer[m_BlockRingBufferDataPostion*m_ofdmSettings.m_PrefixedSymbolSize], &input[0], (m_ofdmSettings.m_PrefixedSymbolSize*sizeof(double)) );

    // Search for a peak in the correlation in the data blocks
    coarseStart = CoarseSearch();
    // If Coarse Search was succesfull 
    if(coarseStart >= 0)
    {
        // Add prefix size to get the actual symbol start 
        IncrementByN(coarseStart, m_ofdmSettings.m_PrefixSize);
        // Pilot Tone Search
        returnValue = FineSearch(coarseStart);
    }

    // Increment position tracking variables for the next block
    m_BlockRingBufferDataPostion++;
    m_BlockRingBufferCorrelatorPostion++;
    // Check if next block position lies outside the boundaries of the array
    if ( m_BlockRingBufferDataPostion >= m_nBlocks)
    {
        m_BlockRingBufferDataPostion = 0;
    }
    // Check if next correlator start offset position lies outside the boundaries of the array
    if(m_BlockRingBufferCorrelatorPostion >= m_nBlocks )
    {
        m_BlockRingBufferCorrelatorPostion = 0;
    }

    return returnValue;
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
*
* @return Sum of imaginary components of the pilot locations in the ofdm symbol
*
*/
double Detector::ComputeSumOfImag(size_t offset)
{
    // Use Nyquist Demodulator to put samples into fft input buffer
    rNyquistModulator.Demodulate(m_BlockRingBuffer, rFFT.in, offset);
    // Compute FFT
    rFFT.ComputeTransform();
    // Normalise FFT
    rFFT.Normalise();
    // Compute & return the sum of imaginary components
    return rFFT.GetImagSum();
}


/**
* Searches for the symbol start by assessing the 
* value of the imaginary components of the pilot tone
* locations. Firstly restrict the search to the boundaries
* of the block ring buffer, then for the specified search range
* compute the sum of the imaginary components of the pilot
* tone locations and return the index at which the lowest value
* occurs.
* 
* @param coarseStart start of the symbol found by coarse search
*
* @return Fine symbol start index
*/
size_t Detector::FineSearch(size_t coarseStart)
{
    // Set min to the max value of double
    double min = std::numeric_limits<double>::max();
    // Initialize variables to zero
    double sumOfImag = 0.0;
    size_t lowestImgIndex = 0;
    size_t startIndex = 0;

    // For DEBUG ONLY: Create vector to store sums
    //std::vector<double> SumOfImagVec; 

    // For given Coarse start index 
    // Restric start index of fine search to 0th element of the buffer
    // If the result of the start index is greater or equal to zero 
    if(   (int)(coarseStart - ((m_SearchRange-1) / 2) ) > 0)   //     TODO: For this type of check where the variable becomes negative it must be casted!!!!! 
    {
        // Set the start index as intended
        startIndex = coarseStart - (size_t)((m_SearchRange-1) / 2);
    }
    // Start index outside the lower boundaries of the array
    else
    {
        // Wrap to the end of the ring buffer 
        startIndex = ( m_RingBufferEdge + ((coarseStart - ((m_SearchRange-1) / 2))) - 1);
    }

    /*
    std::cout << "" << std::endl;
    std::cout << "Coarse Symbol Start = " << coarseStart << std::endl;
    std::cout << "Start Index = " << startIndex << std::endl;
    std::cout << "Search Range = " << m_SearchRange << std::endl;
    */

    // Initialize Index variable
    size_t index = startIndex;
    // For each offset from start to stop index 
    for(size_t j = 0; j < m_SearchRange; j++) 
    {
        // Compute sum of imaginary components of the expected pilot tone locations
        sumOfImag = ComputeSumOfImag(index);
        //SumOfImagVec.push_back(sumOfImag); // For DEBUG ONLY: Append sum to the end of the vector to plot later
        // If calculated Sum of imaginary components is less than current minimum
        if( sumOfImag < min )
        {
            // Save minimum value
            min = sumOfImag;
            // Save index at which this occured
            lowestImgIndex = index;
        }
        Increment(index);
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
* Increments the indicies of the correlator by specified size and
* deals with the wrap around 
*
*/
void  Detector::IncrementCorrelatorIndicies(size_t n)
{
    size_t newIndex = 0;
    // Check if m_PrefixIndex  is going to exceed the boundary of ring buffer
    if( m_PrefixIndex + n < m_RingBufferEdge )
    {
        // Safe to increment by n;
        m_PrefixIndex += n;

    }
    else
    {
        // Wrap around
        newIndex = ((m_PrefixIndex + n ) - m_RingBufferEdge);
        m_PrefixIndex = newIndex;
    }

    // Check if m_SignalIndex exceed the boundary of ring buffer
    if( (m_SignalIndex + n) < m_RingBufferEdge )
    {
        // Safe to increment by n
        m_SignalIndex += n;
    }
    else
    {
        // Wrap around
        newIndex = ((m_SignalIndex + n ) - m_RingBufferEdge);
        m_SignalIndex = newIndex;
    }
}


/**
* Searches for the prefixed symbol start by computing
* and evaluating the correlator output.
* 
* @return Index of the start of the prefix
* else -1 if the search was not successful
*
*/
long int Detector::CoarseSearch()
{
    // Initialize return value, by default indicate coarse search has not found the start of the prefix
    long int returnValue = -1; 
    double correlation = 0;

    // Calculate the interations required to reach the end of the block 
    // By adding the size of the new block 
    m_OffsetCounter += (m_ofdmSettings.m_PrefixedSymbolSize); // m_PrefixedSymbolSize-1?; TODO: This keeps on iterating so bound this within the max to the ring buffer data position 
    
    // If previous search was successful the  
    // Speed up the peak search 
    // Skip to the possible peak once
    /*
    if((m_LastPeakCounter > 0) )
    {
        // Check if the symbol peak can occur before the end of the block
        if( (m_OffsetCounter - m_LastPeakCounter) <= 0) 
        {
            // Update indicies
            IncrementCorrelatorIndicies(m_OffsetCounter);
            // Update last peak counter
            m_LastPeakCounter -= m_OffsetCounter;
            m_OffsetCounter = 0;
        }
        // Set the indicies to where m_LastPeakCounter = 0 i.e indicates correlation can be calculated
        else
        {
            // update Couter
            IncrementCorrelatorIndicies(m_LastPeakCounter); 
            m_OffsetCounter -= m_LastPeakCounter; //TODO THIS EXCEEDS the limits
            // Reset last peak counter
            m_LastPeakCounter = 0;
        }
    }
   */
   
    // If the loop breaks before the offset counter counts down 
    // It will keep its value and m_PrefixedSymbolSize is added above 
    // to calculate the total itterations before the end of the block
    // This means the correlator will continue where it left off when
    // the symbol was found
    while(m_OffsetCounter > 0) 
    {
        // Calculate correlation for a given offset
        correlation = ExecuteCorrelator();
        // Square correlation to make the result +ve and further spearate peaks and noise
        correlation *= correlation;
        // DEBUG ONLY: Append correlation to plot buffer
        //corOutput.push_back(correlation); 

        // If the correlation exceeds the threshold
        if(correlation >= m_UpperThreshold)
        {
            // Set the flag to true
            m_ThresholdFlag = true;
            // Check if the value is higher than current max
            if(m_CorrelatorMaxValue <= correlation)
            {
                // Update the max value
                m_CorrelatorMaxValue = correlation;
                // Update index at which this occured
                m_CorrelatorMaxValueIndex = m_PrefixIndex;
                //m_nFromPeak = 0;
            }
        
        }

        // TODO: Jitter -> nSamplesLow 
    
        // If the sample value falls below lower threshold, and upper threshold was previosuly exceeded
        // Peak has been found
        if((correlation < m_LowerThreshold) && (m_ThresholdFlag == true) )
        {
            // Reset flag
            m_ThresholdFlag = false;
            // Update Return Value
            returnValue = m_CorrelatorMaxValueIndex;

            // Reset Correlator Max Value
            m_CorrelatorMaxValue = 0;
            // Reset Index 
            m_CorrelatorMaxValueIndex = 0;
            // break while loop
            break; 
        }

        //m_nFromPeak++;
        // Decrement points left before end of the block 
        m_OffsetCounter--;
        // Increment prefix & signal indicies 
        m_PrefixIndex++;
        m_SignalIndex++;
        // Check if indicies exceed the boundary of ring buffer
        if( m_PrefixIndex >= m_RingBufferEdge )
        {
            // Wrap around
            m_PrefixIndex = 0;
        }
        // Check if indicies exceed the boundary of ring buffer
        if( m_SignalIndex >= m_RingBufferEdge )
        {
            // Wrap around
            m_SignalIndex = 0;
        }
    }

    // Debug only
    /*
    plotCounter++;
    if(plotCounter == 6)
    {
        // Plot Corellator output
        Gnuplot gp;
        gp << "plot '-' with line title 'Detector - Correlation'\n";
        gp.send1d(corOutput);
    }
    */

    // If prefix start has been found
    if(returnValue >= 0 )
    {
        // Set Last Peak counter
        // use symbol size but subtract the number since the actual peak has occured
        m_LastPeakCounter = m_ofdmSettings.m_SymbolSize; //- m_nFromPeak;
        //m_nFromPeak = 0;
    }

    return returnValue;
}
