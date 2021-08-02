// For IO
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <unistd.h>

// Plotting
#include <boost/tuple/tuple.hpp>
#include "gnuplot-iostream.h"
#include <utility>

// FFT
#include <fftw3.h>



void PlotFFT(double *symbol, size_t nPoints)
{
    fftw_complex *in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * nPoints );
    fftw_complex *out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * nPoints);
    fftw_plan fftplan = fftw_plan_dft_1d(nPoints, in, out, FFTW_FORWARD, FFTW_ESTIMATE); 

    memcpy(in, symbol, nPoints * 2 * sizeof(double) );
    fftw_execute(fftplan);


    // Normalise 
    for(size_t i = 0; i < nPoints; i++) 
    {
        out[i][0] /= nPoints;
        out[i][1] /= nPoints;
    }

    // Cast & Compute abs
    std::vector<double> fftBuffer(nPoints);
    for(size_t i = 0; i < nPoints; i++) 
    {
        std::complex<double> x(out[i][0],out[i][1]);
        fftBuffer.at(i) = std::abs(x);
    }

    // Plot
    Gnuplot gplot;
    gplot << "plot '-' with line title 'Spectrum'\n";
    gplot.send1d(fftBuffer);

    fftw_destroy_plan(fftplan);
    fftw_free(in); fftw_free(out);
}



void PlotRealFFT(double *data, size_t nPoints)
{
    fftw_complex *in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * nPoints );
    fftw_complex *out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * nPoints);
    fftw_plan fftplan = fftw_plan_dft_1d(nPoints, in, out, FFTW_FORWARD, FFTW_ESTIMATE); 

    // Copy data to input buffer as purley real data
    for(size_t i = 0; i < nPoints; i++ )
    {
        in[i][0] = data[i];
    }

    // Compute FFT transform
    fftw_execute(fftplan);


    // Normalise 
    for(size_t i = 0; i < nPoints; i++) 
    {
        out[i][0] /= nPoints;
        out[i][1] /= nPoints;
    }


    std::vector<double> fftBuffer(nPoints );
    // Cast & Compute absolute value
    for(size_t i = 0; i < nPoints; i++) 
    {
        std::complex<double> x(out[i][0],out[i][1]);
        fftBuffer.at(i) = std::abs(x);
    }


    // Plot
    Gnuplot plot;
    plot << "plot '-' with impulses title 'Spectrum using data as real'\n";
    plot.send1d(fftBuffer);

    fftw_destroy_plan(fftplan);
    fftw_free(in); fftw_free(out);
}
