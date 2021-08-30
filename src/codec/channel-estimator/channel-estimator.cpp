/**
* @file channel-estimator.cpp
* @author Kamil Rog
*
* 
*/

#include "channel-estimator.h"
#include <cmath>
#include <complex.h>
#include <iostream>

/**
* Constructor
*
*/
ChannelEstimator::ChannelEstimator(const OFDMSettings settings) :
    m_Settings(settings)
{
    m_Temp = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) *m_Settings.m_nFFTPoints);
    // For the pilot distance calculate
    for(size_t i = 1; i < m_Settings.m_PilotToneDistance+1; i++ )
    {
        // Compute factor from 0 to 1 based on the point
        m_InterpolationFactors.push_back((double)i);
    }

}


/**
* Destructor 
*
*/
ChannelEstimator::~ChannelEstimator()
{
    
}

void ChannelEstimator::PlotQAM(fftw_complex *unCorrected, fftw_complex *Corrected)
{
    using namespace matplot;
    DoubleVec x;
    DoubleVec y;
    DoubleVec xCorrected;
    DoubleVec yCorrected;
    

    for(size_t i = 0; i < m_Settings.m_DataSubCarrierLocations.size(); i++ )
    {
        x.push_back( unCorrected[  m_Settings.m_DataSubCarrierLocations[i]  ][0] );
        y.push_back( unCorrected[  m_Settings.m_DataSubCarrierLocations[i]  ][1] );
    }

    for(size_t i = 0; i < m_Settings.m_DataSubCarrierLocations.size(); i++ )
    {
        xCorrected.push_back( Corrected[  m_Settings.m_DataSubCarrierLocations[i]  ][0] );
        yCorrected.push_back( Corrected[  m_Settings.m_DataSubCarrierLocations[i]  ][1] );
    }

    auto RxQam = scatter(x, y);
    RxQam->display_name("Rx QAM");
    RxQam->marker_color({0.f, 0.0f, 1.0f});
    RxQam->marker_face_color({0.f, 0.0f, 1.0f});

    hold(on);
    auto CorrectedQam = scatter(xCorrected, yCorrected);
    CorrectedQam->display_name("Phase Corrected QAM");
    CorrectedQam->marker_color({1.0f, 0.0f, 0.0f});
    CorrectedQam->marker_face_color({1.0f, 0.0f, 0.0f});
    hold(off);
    legend();
    title("Phase Correction of Rx 4-QAM");
    show();

    /* 
    // Test
    std::complex<double> complexPoint = std::polar(1.0, 0.98);
    complexPoint *= std::exp(std::complex<double>(0, -0.43));
    std::cout << " Real = " << abs(complexPoint)  << " Imag = " <<  arg(complexPoint) << std::endl;
    */
    
}


double ChannelEstimator::LinearInterpolation(double a, double b, double t)
{
    return (a + (t * (b - a)));
}



/**
* Estimates the channel in frequency domain for a comb-type
* pilot tone arrangment by computing the gradient between the 
* a pair of pilot tones and subtracting the value of the interpolated 
* point from the actuals
*
* @param pFFTOutput pointer to the the symbol in frequency domain samples (FFT Output)
* TODO: Deal with wrap around by changing the nested for loop to while and using limit increment
*/ 
void ChannelEstimator::FrequencyDomainInterpolation(fftw_complex *pFFTOutput)
{
    // y = mx + c
    // Gradient m = (y(n) - y(n-1)/(x(n) - x(n-1))
    // Intercept c = y(n-1) - mx(n-1)
    //intercept = y1 - slope * x1; // which is same as y2 - slope * x2
    // Phase Interpolation Variables
    double deltaYphase = 0.0;
    double phaseGradient = 0.0;
    double interpolatedPhase = 0;
    double phaseIntercept = 0;
    double phaseAngle = 0;
    // Amplitude Interpolation Variables
    double deltaYamplitude = 0.0;
    double amplitudeGradient = 0.0;
    double interpolatedAmplitude = 0;
    double amplitudeIntercept = 0;
    std::complex<double> complexPoint(0, 0);

    // Save Constellation for later plotting
    //memcpy( m_Temp, pFFTOutput,  (sizeof(fftw_complex) *m_Settings.m_nFFTPoints) );

    size_t pointCounter = 0;
    // For each pilot tone skiping the first
    for(size_t pilotCounter = 1; pilotCounter < m_Settings.m_PilotToneLocations.size(); pilotCounter++)
    {
        // Save intercept of previous pilot tone
        phaseIntercept = pFFTOutput[pilotCounter-1][1];
        amplitudeIntercept = pFFTOutput[pilotCounter-1][0];
        // Calculate delta between current and previous pilot tone
        deltaYphase = pFFTOutput[pilotCounter][1] - pFFTOutput[pilotCounter-1][1];
        deltaYamplitude = pFFTOutput[pilotCounter][0] - pFFTOutput[pilotCounter-1][0];
        // Calculate gradient based on the delta and pilot distance
        phaseGradient = deltaYphase/(double)m_Settings.m_PilotToneDistance;
        amplitudeGradient = deltaYamplitude/(double)m_Settings.m_PilotToneDistance;
        // Reset subCarrier Counter
        pointCounter = 0;
        // Calculate the start & stop index valuex for the sub-carrier locations vector
        // Skip half pilot tones at the start, Add the
        size_t dataCarrierStartingIndex = (m_Settings.m_PilotToneDistance/2) +  ((pilotCounter-1)*m_Settings.m_PilotToneDistance);
        size_t dataCarrierStopingIndex = dataCarrierStartingIndex + m_Settings.m_PilotToneDistance -1;
        // For each consequtive data sub-carrier in the sequence
        for(size_t c = m_Settings.m_DataSubCarrierLocations[ dataCarrierStartingIndex]; 
        c < m_Settings.m_DataSubCarrierLocations[dataCarrierStopingIndex]; 
        c++)
        {
            // Calculate interpolated phase point 
            // y(i) = y0(The first pilot phase) + m(gradient)*x(position between the first pilot and the second 0-to-1)
            interpolatedPhase = ( phaseIntercept + (phaseGradient * m_InterpolationFactors[pointCounter] ) );
            interpolatedAmplitude = (amplitudeIntercept + ( (amplitudeGradient * m_InterpolationFactors[pointCounter] )) );
            // Calculate the angle o of the interpolated carrier and 
            phaseAngle = arg( std::complex<double>(interpolatedAmplitude,interpolatedPhase));
            // Rotate Complex sub-carrier point using phasor e^-jo
            complexPoint = std::complex<double>(pFFTOutput[c][0], pFFTOutput[c][1]);
            complexPoint *= std::exp(std::complex<double>(0, -phaseAngle ));
            // Insert point back into the fft window
            pFFTOutput[c][0] = complexPoint.real();
            pFFTOutput[c][1] = complexPoint.imag();
            // Move to the next data sub-carrier
            pointCounter++;
        }
    }
    // plot points after correction
    //PlotQAM(m_Temp,pFFTOutput);
}
