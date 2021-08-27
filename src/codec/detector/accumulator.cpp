
#include <accumulator.h>
    
/**
* Constructor
* 
* @param settings
* @param fft reference to the fft objecct
* @param nyquist reference to the nyquist modulator objecct
*
*/
Accumulator::Accumulator(const OFDMSettings &settings) :
    m_ofdmSettings(settings)
{
    pData = (double*) calloc((m_ofdmSettings.m_PrefixSize), sizeof(double));
    m_lastIndex = 0;
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
double Accumulator::ComputeFull(double *prefix, double *signal)
{
    for(size_t i = 0; i < m_ofdmSettings.m_PrefixSize; i++)
    {
        ProcessSample(prefix[i] * signal[i]);
    }
    return m_Sum;
}


/**
* Process new sample
*
*@param sample new sample to insert into the array in place of the last in the sequence
*
*/
double Accumulator::ProcessSample(double sample)
{
    // Substract the last sample
    m_Sum -= pData[m_lastIndex];
    // Insert Element the new element in its place
    pData[m_lastIndex] = sample;
    // Add new sample
    m_Sum += sample; 
    // Increment last index tracker
    // If Increment is not going to exceed the boundary
    if( (m_lastIndex+1) < m_ofdmSettings.m_PrefixSize)
    {
        m_lastIndex++;
    }
    // Wrap around
    else
    {
        m_lastIndex = 0;
    }
    // Return Sum
    return m_Sum;
}
