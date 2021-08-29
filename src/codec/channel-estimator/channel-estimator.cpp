/**
* @file channel-estimator.cpp
* @author Kamil Rog
*
* 
*/

#include "channel-estimator.h"
#include <complex>
#include <cmath>

/**
* Constructor
*
*/
ChannelEstimator::ChannelEstimator(const OFDMSettings settings) :
    m_Settings(settings)
{

}


/**
* Destructor 
*
*/
ChannelEstimator::~ChannelEstimator()
{
    
}

void PlotQAM(fftw_complex *FFT, size_t nPoints)
{
    using namespace matplot;
    DoubleVec x;
    DoubleVec y;
    for(size_t i = 0; i < nPoints; i++ )
    {
        x.push_back( FFT[i][0] );
        y.push_back( FFT[i][1] );
    }

    scatter(x, y);
    show();
}



/**
* Estimates the channel in frequency domain for a comb-type
* pilot tone arrangment by computing the gradient between the 
* a pair of pilot tones and subtracting the value of the interpolated 
* point from the actuals
*
* @param pFFTOutput pointer to the the symbol in frequency domain samples (FFT Output)
* TODO: Deal with wrap around
*/ 
void ChannelEstimator::FrequencyDomainInterpolation(fftw_complex *pFFTOutput)
{
    // y = mx + c
    // Gradient m = (y(n) - y(n-1)/(x(n) - x(n-1))
    // Intercept c = y(n-1) - mx(n-1)
    //intercept = y1 - slope * x1; // which is same as y2 - slope * x2
    double deltaX = (double)m_Settings.m_PilotToneDistance; 
    double deltaYphase = 0.0;
    double phaseGradient = 0.0;
    //double deltaYAmplitude = 0.0;
    //double AmplitudeGradient = 0.0;  
    double InterpolatedPhaseFactors[] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
    double interpolatedPhase = 0;
    double phaseIntercept = 0;
    std::complex<double> complexPoint(0, 1);
    // Interpolate -ve frequencies 
    size_t pointCounter = 0;
    // Plot QAM Before correction
    PlotQAM(pFFTOutput,m_Settings.m_nFFTPoints);
    for(size_t pilotCounter = 1; pilotCounter < m_Settings.m_PilotToneLocations.size(); pilotCounter++)
    {
        // Calculate delta between current and previous pilot tone
        //deltaYAmplitude = pFFTOutput[pilotCounter][0] -  pFFTOutput[pilotCounter-1][0];
        phaseIntercept = pFFTOutput[pilotCounter-1][1];
        deltaYphase = pFFTOutput[pilotCounter][1] - pFFTOutput[pilotCounter-1][1];
        // Calculate gradient based on the delta and pilot distance
        phaseGradient = deltaYphase/deltaX;
        //AmplitudeGradient = deltaYAmplitude/deltaX;
        pointCounter = 0;
        for(size_t c = m_Settings.m_DataSubCarrierLocations[(pilotCounter-1)*m_Settings.m_PilotToneDistance]; c < m_Settings.m_DataSubCarrierLocations[(pilotCounter)*m_Settings.m_PilotToneDistance]; c++)
        {
            // From the data sub-carrier phase subtract the interpolated phase value y(i)
            // y(i) = y0(The first pilot phase) + m(gradient)*x(position between the first pilot and the second 1-to-15)
            interpolatedPhase = ( phaseIntercept + (phaseGradient * InterpolatedPhaseFactors[pointCounter] ) );
            complexPoint = std::complex<double>(pFFTOutput[c][0], pFFTOutput[c][1]);
            // Rotate Complex point
            complexPoint *= std::exp( std::complex<double>(0, -interpolatedPhase));
            //complexPoint *= std::complex<double>(0, -interpolatedPhase);
            // Insert point back into the fft window
            pFFTOutput[c][0] = complexPoint.real();
            pFFTOutput[c][1] = complexPoint.imag();
            //pFFTOutput[c][1] -=interpolatedPhase;
            pointCounter++;

        }
    }
    PlotQAM(pFFTOutput,m_Settings.m_nFFTPoints);
    // Interpolate +ve frequencies
}
