/**
* @file ofdmfft.cpp
* @author Kamil Rog
*
* 
*/

#include "fft.h"


/**
* Constructor 
*
*
*/
FFT::FFT(OFDMSettings &settings) :
    m_ofdmSettings(settings)
{
    Configure();
}


/**
* Destructor 
*
*
*/
FFT::~FFT()
{
    Close();
}

/**
* Sets up FFT for specified size, 
* 
* @param nPoints Number of FFT / IFFT coefficients
* 
* @param type Specifies whether the object computes FFT or IFFT choices - FFTW_FORWARD(-1) FFTW_BACKWARD(+1)
*
* @return 0 on success, else error number
*
*/
int FFT::Configure()
{   
    // If object has been configured before
    if(m_configured)
    { 
        // Destroy fft plan and free allocated memory to buffers
        Close();
    }
    // Allocate memory for the input & output buffers
    in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * m_ofdmSettings.nFFTPoints);
    out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * m_ofdmSettings.nFFTPoints);

    FFTin = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * m_ofdmSettings.nFFTPoints);
    FFTout = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * m_ofdmSettings.nFFTPoints);
    
    IFFTin = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * m_ofdmSettings.nFFTPoints);
    IFFTout = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * m_ofdmSettings.nFFTPoints);

    // Use the fastes avaiable plan for the size and type of the transform specified by measuring  
    m_fftplan = fftw_plan_dft_1d(m_ofdmSettings.nFFTPoints, in, out, m_ofdmSettings.type, FFTW_MEASURE); 
    m_ifftplan = fftw_plan_dft_1d(m_ofdmSettings.nFFTPoints, IFFTin, IFFTout, FFTW_BACKWARD, FFTW_MEASURE); 

    // Set configure flag
    m_configured = 1;
    //m_ofdmSettings = settings;
    return 0;
}


/**
* Destroys fftw plan and frees up allocated memory for input and output buffers
* 
* @return 0 on success, else error number
*
*/    
int FFT::Close()
{
    fftw_destroy_plan(m_fftplan);
    fftw_destroy_plan(m_ifftplan);
    fftw_free(in); fftw_free(out);
    m_configured = 0;
    return 0;
}


/**
* Normalises the output of the FFT 
* 
* @return 0 on success, else error number
*
*/  
int FFT::Normalise()
{
    double multiplicationFactor = 1./m_ofdmSettings.nFFTPoints;
    for (size_t i = 0; i < m_ofdmSettings.nFFTPoints; i++)
    {
        out[i][0] *= multiplicationFactor;
        out[i][1] *= multiplicationFactor;
    }
    return 0;
}


/**
* Computes the sum of the imaginary points where 
* pilot tones are expected
* 
* @return symbol start(integer) index, else -1
*
*/
double FFT::GetImagSum(const size_t nBytes) 
{
    // Compute number of pilot tones based on the bytes in the symbol
    size_t nPilots = (size_t) (((nBytes*BITS_IN_BYTE)/BITS_PER_FREQ_POINT)/m_ofdmSettings.PilotToneDistance);
    // Compute frequency coefficient index used
    // Assume spectrum is centred symmetrically around DC and depends on nBytes
    double sumOfImag = 0.0;
    size_t pilotToneCounter = (size_t) m_ofdmSettings.PilotToneDistance / 2 ; // divide this by to when starting with -ve frequencies
    size_t fftPointIndex = (size_t) ((m_ofdmSettings.nFFTPoints) - (nPilots/ 2) - ((nBytes * FREQ_POINTS_PER_BYTE) / 2));
    size_t insertionCounter = 0;
    // For expected byte 
    for(size_t byteCounter = 0; byteCounter < nBytes; byteCounter++)
    {
        insertionCounter = 0;
        // Process 4 FFT points i.e 8 bits
        while(insertionCounter < 4)
        {
            // If pilot tone counter counted down
            // This point is the pilot tone
            if(pilotToneCounter == 0)
            {
                // Reset Counter
                pilotToneCounter = m_ofdmSettings.PilotToneDistance;
                sumOfImag += abs(out[fftPointIndex][1]);
    
            }
            // This point is QAM encoded complex point
            else
            {
                insertionCounter++;
                pilotToneCounter--;
            }
            // Increment fft point counter
            fftPointIndex++;
            // Check if fft exceeds the limit of points
            if(fftPointIndex == m_ofdmSettings.nFFTPoints)
            {
                // Roll back to positive frequencies
                fftPointIndex = 0;
            }
        }
    }
    // Return sum
    return sumOfImag;

    // Skip to the first pilot
    for(size_t i = 0; i < nPilots; i++)
    {

    }
}


/**
* Computes FFT Based on the object's input (in) buffer and stores it in the object's output (out) buffer.
* 
*
* @return 0 on success, else error number
*
*/    
int FFT::ComputeTransform()
{
    fftw_execute(m_fftplan);
    return 0;
}


/**
* Computes FFT using object's configured plan and input(in) buffer
* and stores it in specified destination.
* 
* @param dest pointer to the fftw_complex array
*
*
* @return 0 on success, else error number
*
*/   
int FFT::ComputeTransform(fftw_complex *dest)
{
    fftw_execute_dft(m_fftplan, in, dest);
    return 0;
}
