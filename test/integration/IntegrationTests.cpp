#define BOOST_TEST_MODULE IntegrationTest
#include <boost/test/unit_test.hpp>

// For IO
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <unistd.h>

// Plotting
#include <boost/tuple/tuple.hpp>
#include "gnuplot-iostream.h"
#include <utility>

// For measuring elapsed time
#include <chrono>

// For Random Float Generator
#include <time.h>

// For object under test
#include "trx.h"
#include "common.h"


void PlotFFT(fftw_complex *data, size_t nPoints)
{
    fftw_complex *in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * nPoints );
    fftw_complex *out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * nPoints);
    fftw_plan fftplan = fftw_plan_dft_1d(nPoints, in, out, FFTW_FORWARD, FFTW_ESTIMATE); 

    memcpy(in, data, nPoints * 2 * sizeof(double) );
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


void Plotbuffer(double *data, size_t nPoints)
{
        // Plot Rx waveform
    std::vector<double> vecBuffer(nPoints);
    for(size_t i = 0; i < nPoints; i++) 
    {
        vecBuffer.at(i) = data[i];
    }

    Gnuplot gp;
    gp << "plot '-' with line title 'Record Buffer'\n";
    gp.send1d(vecBuffer);
}


// Integration Tests 
BOOST_AUTO_TEST_SUITE(IntegrationTests)


BOOST_AUTO_TEST_CASE(SelfReceiveing)
{
    std::cout << "\nSelf Receiveing Test\n" << std::endl;
    // OFDM Codec Settings
    OFDMSettings encoderSettings;
    encoderSettings.type = FFTW_BACKWARD;
    encoderSettings.EnergyDispersalSeed = 5;
    encoderSettings.nPoints = 1024; 
    encoderSettings.pilotToneStep = 16; 
    encoderSettings.pilotToneAmplitude = 2.0; 
    encoderSettings.guardInterval = 0; 
    encoderSettings.QAMSize = 2; 
    encoderSettings.cyclicPrefixSize = (int) ((encoderSettings.nPoints*2)/4); // 1/4th of symbol

    OFDMSettings decoderSettings = encoderSettings;
    decoderSettings.type = FFTW_FORWARD;

    // Calculate max nBytes 
    //size_t nAvaiablePoints = (encoderSettings.nPoints - ((size_t)(encoderSettings.nPoints / encoderSettings.pilotToneStep)));
    size_t nBytes = 120; // (nAvaiablePoints*encoderSettings.QAMSize) / 8;
    size_t nSymbolsToTx = 10;
    size_t prefixedSymbolSize = (encoderSettings.nPoints*2) + encoderSettings.cyclicPrefixSize;

    // rt audioSettings
    rtAudioSettings audioSettings;
    audioSettings.SampleRate = 8000;
    audioSettings.BufferFrames = prefixedSymbolSize;
    audioSettings.nChannels = 1;
    audioSettings.InputDevice = 0;
    audioSettings.OutputDevice = 1; // Use Speaker
    audioSettings.InputOffset = 0;
    audioSettings.OutputOffset = 0;

    // Initialize transceiver
    AudioTrx trx(audioSettings, encoderSettings, decoderSettings);

    // Create input & output array
    uint8_t *txIn = (uint8_t*) calloc(nBytes*nSymbolsToTx, sizeof(uint8_t));
    uint8_t *testRxOut = (uint8_t*) calloc(nBytes*nSymbolsToTx, sizeof(uint8_t));
    uint8_t *rxOut = (uint8_t*) calloc(nBytes*nSymbolsToTx, sizeof(uint8_t));

    // Create Tx Buffer
    double *encodedplayback = (double*) calloc(prefixedSymbolSize*nSymbolsToTx, sizeof(double));
    // Assign Buffer
    trx.m_playbackData.rxCopy = encodedplayback;
    trx.m_playbackData.nChannels = 1;
    trx.m_playbackData.nTxFrames = 0;
    trx.m_playbackData.counter = 0;
    trx.m_playbackData.frameLimit = nSymbolsToTx*prefixedSymbolSize;

    // Create Rx Buffer
    size_t recordBufferSize = prefixedSymbolSize*nSymbolsToTx*2; // Make the rec buffer x2 larger
    double *recorded = (double*) calloc((recordBufferSize), sizeof(double));
    // Assign Buffer
    trx.m_recordData.rxCopy = recorded;
    trx.m_recordData.nChannels = 1;
    trx.m_recordData.counter = 0;
    trx.m_recordData.nRxFrames = 0;
    trx.m_recordData.frameLimit = prefixedSymbolSize*nSymbolsToTx*2;

    // Setup random
    srand( (unsigned)time( NULL ) );

    // Generate array of random bytes
    for (size_t i = 0; i < nBytes*nSymbolsToTx; i++)
    {
        txIn[i] = rand() % 255;
    }

    // Encode Symbols & Place them in buffer
    for (size_t i = 0; i < nSymbolsToTx; i++)
    {
        trx.m_TxCallbackData.pCodec->Encode(&txIn[i*nBytes], &encodedplayback[i*prefixedSymbolSize], nBytes);
    }

    trx.m_encoder.m_fft.ComputeTransform();
    // Compute & Plot FFTs for ofdm symbol and tx symbol spectrum
    PlotFFT(trx.m_encoder.m_fft.out, encoderSettings.nPoints);
    PlotRealFFT(encodedplayback, prefixedSymbolSize);

    size_t bufferCounter = 0; 
    size_t symbolCounter = 0;
    size_t nDecodedBytes = 0;

    /*
    // Decode the encoded symbols to see if decoding functions work
    // THIS NEEDS CHANGE OF THE THRESHOLD IN THE DECODER
    while(symbolCounter != nSymbolsToTx)
    {
        // Decode Symbols
        // They are alligned nicley here at should start at the index that is passed
        // However last symbol cannot be decoded normally 
        // The current method passes one instance outside of the boundaries of the array FIX THIS
        nDecodedBytes = trx.m_RxCallbackData.pCodec->ProcessRxBuffer(&encodedplayback[bufferCounter*prefixedSymbolSize], &testRxOut[symbolCounter*nBytes]);
        if(nDecodedBytes > 0)
        {
            symbolCounter++;
        }
        bufferCounter++;
    }
    
    // Check the Buffer to transmit can be decoded 
    for (size_t i = 0; i < nBytes*nSymbolsToTx; i++)
    {
        //printf("Recovered Sample: %lu %d vs. %d\n", i, txIn[i], rxOut[i]);
        
        // Check if real and complex element match within defined precision of each other
        BOOST_CHECK_MESSAGE( (txIn[i] == testRxOut[i]), "testRxOut: Cannot Decode Tx Buffer, Bytes Differ!" ); 
    }
    */

    // Start receiver
    trx.StartRxStream();

    // Transmit Encoded Symbols bytes
    trx.StartTxStream();

    // Wait to receive 
    sleep(8);

    bufferCounter = 0; 
    symbolCounter = 0;
    nDecodedBytes = 0;

    // Decode symbols from recored audio
    while(symbolCounter != nSymbolsToTx)
    {
        nDecodedBytes = trx.m_RxCallbackData.pCodec->ProcessRxBuffer(&recorded[bufferCounter*2560], &rxOut[symbolCounter*nBytes]);
        if(nDecodedBytes > 0)
        {
            symbolCounter++;
            std::cout << symbolCounter << " Symbol Decoded!" << std::endl;
        }
        bufferCounter++;
        
        if(bufferCounter >= 23)
        {
            break;
        }
    }

    // Check if Decoding has been Successfull
    for (size_t i = 0; i < nBytes; i++)
    {
        // Print 
        printf("Byte: %lu %d vs. %d\n", i, txIn[i], rxOut[i]);
        
        // Check if real and complex element match within defined precision of each other
        BOOST_CHECK_MESSAGE( (txIn[i] == rxOut[i]), "Bytes difffer! Index = " << i ); 
    }

    // Plot recorded & transmitted waveforms
    Plotbuffer(recorded, 30000);
    Plotbuffer(encodedplayback, prefixedSymbolSize*nSymbolsToTx);

    // Free Buffers
    free(encodedplayback);
    free(recorded);
    free(txIn);
    free(testRxOut);
    free(rxOut);
}

BOOST_AUTO_TEST_SUITE_END()
