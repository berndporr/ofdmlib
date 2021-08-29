
#include <accumulator.h>
    
/**
* Constructor
* 
* @param settings
* @param fft reference to the fft objecct
* @param nyquist reference to the nyquist modulator objecct
*
*/
Accumulator::Accumulator(const size_t size) :
    m_nSamples(size),
    m_LastSampleIndex(0)
{
    pData = (double*) calloc(m_nSamples, sizeof(double));
}

/**
* Default Destructor 
*
*/
Accumulator::~Accumulator()
{
    free(pData);
}   


/**
* Process number of samples equal to the size of the buffer 
*
*/
double Accumulator::ProcessFullSet(double *buffer, size_t prefix, size_t signal)
{
    /*
    // Buffer size
    size_t p = prefix;
    size_t s = prefix;
    if( + )
    {

    }

    for(size_t i = 0; i < m_nSamples; i++)
    {
        ProcessSample(buffer[p] * buffer[s]);
    }
    */
    return m_Sum;
}


/**
* Process new sample by subtracting the last sample 
*
*@param sample new sample to insert into the array in place of the last in the sequence
*
*
*/
double Accumulator::ProcessSample(double sample)
{
    // Substract the last sample from the sum
    m_Sum -= pData[m_LastSampleIndex];
    // Insert new element in place of the last sample
    pData[m_LastSampleIndex] = sample;
    // Add new sample to the sum
    m_Sum += sample; 
    // Increment last sample index tracker
    // If Increment is not going to exceed the boundary
    if( (m_LastSampleIndex+1) <= m_nSamples)
    {
        // Safe to increment
        m_LastSampleIndex++;
    }
    // Wrap around
    else
    {
        m_LastSampleIndex = 0;
    }
    // Return Sum
    return m_Sum;
}


/**
* Clears the sum and samples of the accumulator
*
*
*/
void Accumulator::Reset()
{   
    // Set Sum to zero
    m_Sum = 0;
    // Each element 
    for(size_t elementCounter = 0; elementCounter < m_nSamples; elementCounter++)
    {   
        // Set element to zero
        pData[elementCounter] = 0;
    }
    // Reset Last sample index to default
    m_LastSampleIndex = 0;
}