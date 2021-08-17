#define BOOST_TEST_MODULE IntegrationTest
#include <boost/test/unit_test.hpp>

// For IO
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <unistd.h>
#include <bitset>

// Plotting
#include <boost/tuple/tuple.hpp>
#include "gnuplot-iostream.h"
#include <utility>
#include <matplot/matplot.h>

// For measuring elapsed time
#include <chrono>

// For Random Float Generator
#include <time.h>

// For object under test
#include "trx.h"
#include "common.h"


void PlotFFT(double *symbol, size_t nFFTPoints) 
{
    using namespace matplot;

    fftw_complex *in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * nFFTPoints );
    fftw_complex *out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * nFFTPoints);
    fftw_plan fftplan = fftw_plan_dft_1d(nFFTPoints, in, out, FFTW_FORWARD, FFTW_ESTIMATE); 

    memcpy(in, symbol, nFFTPoints * 2 * sizeof(double) );
    fftw_execute(fftplan);

    // Normalise 
    for(size_t i = 0; i < nFFTPoints; i++) 
    {
        out[i][0] /= nFFTPoints;
        out[i][1] /= nFFTPoints;
    }

    // Cast & Compute abs
    std::vector<double> fftBuffer(nFFTPoints);
    for(size_t i = 0; i < nFFTPoints; i++) 
    {
        std::complex<double> x(out[i][0],out[i][1]);
        fftBuffer.at(i) = std::abs(x);
    }


    std::vector<double> x = linspace(0, nFFTPoints, 1);
    plot(x, fftBuffer)->color({0.f, 0.7f, 0.9f});
    title("2-D Line Plot");
    xlabel("x");
    ylabel("FFT Spectrum");

    show();

    fftw_destroy_plan(fftplan);
    fftw_free(in); fftw_free(out);
}


void PlotRealFFT(double *data, size_t nFFTPoints)
{
    using namespace matplot;

    fftw_complex *in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * nFFTPoints );
    fftw_complex *out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * nFFTPoints);
    fftw_plan fftplan = fftw_plan_dft_1d(nFFTPoints, in, out, FFTW_FORWARD, FFTW_ESTIMATE); 

    // Copy data to input buffer as purley real data
    for(size_t i = 0; i < nFFTPoints; i++ )
    {
        in[i][0] = data[i];
    }

    // Compute FFT transform
    fftw_execute(fftplan);


    // Normalise 
    for(size_t i = 0; i < nFFTPoints; i++) 
    {
        out[i][0] /= nFFTPoints;
        out[i][1] /= nFFTPoints;
    }

    std::vector<double> fftBuffer(nFFTPoints );
    // Cast & Compute absolute value
    for(size_t i = 0; i < nFFTPoints; i++) 
    {
        std::complex<double> x(out[i][0],out[i][1]);
        fftBuffer.at(i) = std::abs(x);
    }

    std::vector<double> x = linspace(0, nFFTPoints, 1);
    plot(x, fftBuffer)->color({0.f, 0.7f, 0.9f});
    title("2-D Line Plot");
    xlabel("Normalised Freq");
    ylabel("FFT Real Spectrum");

    show();

    fftw_destroy_plan(fftplan);
    fftw_free(in); fftw_free(out);
}

/*
void Plotbuffer(double *data, size_t nFFTPoints)
{
    using namespace matplot;

    // Plot Rx waveform
    std::vector<double> vecBuffer(nFFTPoints);
    for(size_t i = 0; i < nFFTPoints; i++) 
    {
        vecBuffer.at(i) = data[i];
    }

    std::vector<double> x = linspace(0, nFFTPoints, nFFTPoints);
    plot(x, vecBuffer)->color({0.f, 0.7f, 0.9f});
    title("2-D Line Plot");
    xlabel("Normalised Freq");
    ylabel("Buffer");

    show();
}*/


void Plotbuffer(double *data, size_t nFFTPoints)
{
        // Plot Rx waveform
    std::vector<double> vecBuffer(nFFTPoints);
    for(size_t i = 0; i < nFFTPoints; i++) 
    {
        vecBuffer.at(i) = data[i];
    }

    Gnuplot gp;
    gp << "plot '-' with line title 'Record Buffer'\n";
    gp.send1d(vecBuffer);
}

size_t FindAbsMaxIndex(double *buffer, size_t size)
{
    double maxValue = 0.0;
    size_t maxIndex = 0;

    for(size_t i = 0; i < size; i++)
    {
        if( maxValue < abs(buffer[i]) )
        {
            maxValue = abs(buffer[i]);
            maxIndex = i;
        }
    }

    return maxIndex;
}


double FindAbsMaxValue(double *buffer, size_t size)
{
    double maxValue = 0.0;
    for(size_t i = 0; i < size; i++)
    {
        if( maxValue < abs(buffer[i]) )
        {
            maxValue = abs(buffer[i]);
        }
    }

    return maxValue;
}

void NormaliseToValue(double *buffer, size_t size, double value)
{
    double factor = 1/value;
    for(size_t i = 0; i < size; i++)
    {
        buffer[i] *= factor;
    }
}

void ScaleToAbsMax(double *buffer, size_t size)
{
    double value = FindAbsMaxValue(buffer, size);
    //std::cout << "max value = " <<  value << std::endl;
    NormaliseToValue(buffer, size, value);
}


/*
void LoadGreyScaleImage(matplot::image_channels_t gray)
{
    using namespace matplot;
    auto [h, w] = size(gray);
    double mean_intensity = 0;
    for(const auto &row : gray)
    {
        for (const auto &pixel : row)
        {
            mean_intensity += pixel;
        }
    }
    
    mean_intensity /= (h * w);

    for (auto &row : gray)
    {
        for (auto &pixel : row)
        {
            pixel = pixel > mean_intensity ? 255 : 0;
        }
    }

    imshow(gray);

    show();     

}
*/


double ComputeBitErrorRatio(uint8_t *input, uint8_t *output, size_t nBytes)
{
    size_t nBits = nBytes * 8;
    size_t errorCount = 0;
    uint8_t xorResult = 0;

    // For each byte
    for(size_t i = 0; i < nBytes; i++)
    {
        // XOR corresponding two bytes to obtain the positions where bits differ
        xorResult = input[i] ^ output[i];
        // Create bit set from xor value
        std::bitset<8> bitSet(xorResult);
        // Update bit error count
        errorCount += bitSet.count();      
    }
    // Compute Ratio
    double ratio = errorCount/nBits;
    // Return
    return ratio;
}

// Integration Tests 
BOOST_AUTO_TEST_SUITE(IntegrationTests)


/**
*  This test simulates entire encoding and decoding process
*  of only one symbol, this test does not transmit the data 
*  through a physical medium.
* 
*/

/*
BOOST_AUTO_TEST_CASE(EncodeDecode)
{
    printf("Testing OFDM Encoder & Decoder Object...\n");
    // OFDM Codec Settings
    OFDMSettings encoderSettings;
    encoderSettings.type = FFTW_BACKWARD;
    encoderSettings.EnergyDispersalSeed = 10;
    encoderSettings.nFFTPoints = 1024; 
    encoderSettings.PilotToneDistance = 16; 
    encoderSettings.PilotToneAmplitude = 2.0; 
    encoderSettings.QAMSize = 2; 
    encoderSettings.PrefixSize = (size_t) ((encoderSettings.nFFTPoints*2)/4); // 1/4th of symbol

    OFDMSettings decoderSettings = encoderSettings;
    decoderSettings.type = FFTW_FORWARD;
    
    OFDMCodec encoder(encoderSettings);
    OFDMCodec decoder(decoderSettings);

    size_t symbolSize = (encoderSettings.nFFTPoints*2);
    size_t prefixedSymbolSize = symbolSize + encoderSettings.PrefixSize;
    size_t rxSignalSize = prefixedSymbolSize * 10;
    size_t rxLastAllowedIndex = prefixedSymbolSize * 9;

    // Randomly generated prefix start position
    size_t prefixStart = rand() % rxLastAllowedIndex;
    printf("Randomly Generated Prefix Start = %lu\n",prefixStart);

    // Calculate max nBytes 
    size_t nAvaiablePoints = (encoderSettings.nFFTPoints - ((size_t)(encoderSettings.nFFTPoints / encoderSettings.PilotToneDistance)));
    size_t nBytes = (nAvaiablePoints*encoderSettings.QAMSize) / 8;

    std::cout << "Encoding nBytes = " << nBytes << std::endl;
    
    uint8_t * txIn = (uint8_t*) calloc(nBytes, sizeof(uint8_t));
    uint8_t * rxOut = (uint8_t*) calloc(nBytes, sizeof(uint8_t));

    double * txData = (double*) calloc(prefixedSymbolSize, sizeof(double));
    double * rxSignal = (double*) calloc(rxSignalSize, sizeof(double));


    // Setup random byte generator
    srand( (unsigned)time( NULL ) );

    // Generate array of random bytes
    for (size_t i = 0; i < nBytes; i++)
    {
        txIn[i] = rand() % 255;
    }

    // Encode 1 ofdm symbol
    auto start = std::chrono::steady_clock::now();
    encoder.Encode(txIn,txData, nBytes);
    auto end = std::chrono::steady_clock::now();

    std::cout << "Encode elapsed time: "
    << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
    << " ns" << std::endl;

    // Copy Encoded data to Rx Signal 
    memcpy(&rxSignal[prefixStart], &txData[0], sizeof(double)*prefixedSymbolSize);

    // Decode
    start = std::chrono::steady_clock::now();
    decoder.Decode(&rxSignal[prefixStart], rxOut, nBytes);
    end = std::chrono::steady_clock::now();

    std::cout << "Decode elapsed time: "
    << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
    << " ns" << std::endl;
    
    // Check the input and output are within threshold
    for (size_t i = 0; i < nBytes; i++)
    {
        //printf("Recovered Sample: %lu %d vs. %d\n", i, txIn[i], rxOut[i]);
        
        // Check if real and complex element match within defined precision of each other
        BOOST_CHECK_MESSAGE( (txIn[i] == rxOut[i]), "Bytes difffer!" ); 
    }

    std::cout << "start stop elapsed time: "
    << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
    << " ns" << std::endl;

    free(txIn);
    free(rxOut);
    free(txData);
    free(rxSignal);
}
*/

/*
BOOST_AUTO_TEST_CASE(TheorethicalSelfReceiving)
{
    std::cout << "\n Theorethical Self-Receiveing Test\n" << std::endl;
    // OFDM Codec Settings
    OFDMSettings encoderSettings;
    encoderSettings.type = FFTW_BACKWARD;
    encoderSettings.EnergyDispersalSeed = 10;
    encoderSettings.nFFTPoints = 1024; 
    encoderSettings.PilotToneDistance = 16; 
    encoderSettings.PilotToneAmplitude = 2.0; 
    encoderSettings.QAMSize = 2; 
    encoderSettings.PrefixSize = (size_t) ((encoderSettings.nFFTPoints*2)/4); // 1/4th of symbol

    OFDMSettings decoderSettings = encoderSettings;
    decoderSettings.type = FFTW_FORWARD;

    // Calculate max nBytes 
    //size_t nAvaiablePoints = (encoderSettings.nFFTPoints - ((size_t)(encoderSettings.nFFTPoints / encoderSettings.PilotToneDistance)));
    size_t nBytes = 120; // (nAvaiablePoints*encoderSettings.QAMSize) / 8;
    size_t nSymbolsToTx = 5;
    size_t prefixedSymbolSize = (encoderSettings.nFFTPoints*2) + encoderSettings.PrefixSize;

    // rt audioSettings
    rtAudioSettings audioSettings;
    audioSettings.SampleRate = 8000;
    audioSettings.BufferFrames = prefixedSymbolSize;
    audioSettings.nChannels = 1;
    audioSettings.InputDevice = 0;
    audioSettings.OutputDevice = 0; // Use Speaker
    audioSettings.InputOffset = 0;
    audioSettings.OutputOffset = 0;

    // Initialize transceiver
    AudioTrx trx(audioSettings, encoderSettings, decoderSettings);

    // Create input & output array
    uint8_t *txIn = (uint8_t*) calloc(nBytes*nSymbolsToTx, sizeof(uint8_t));
    uint8_t *rxOut = (uint8_t*) calloc(nBytes*nSymbolsToTx, sizeof(uint8_t));

    // Create Tx Buffer
    // But make sure it is one prefixedSymbolSize larger to allow for appropriate decoding
    double *encodedplayback = (double*) calloc( prefixedSymbolSize * (nSymbolsToTx + 1), sizeof(double));
    double *OffsetEncodedPlayback = (double*) calloc( prefixedSymbolSize * (nSymbolsToTx + 1), sizeof(double));

    // Create Rx Buffer
    size_t recordBufferSize = prefixedSymbolSize*nSymbolsToTx*2; // Make the rec buffer x2 larger for good measure
    double *recorded = (double*) calloc((recordBufferSize), sizeof(double));

    // Setup random seed
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

    // Randomly Offset the start of the symbols
    size_t randomOffset =  rand() % prefixedSymbolSize;
    randomOffset = 2347;
    // Copy encoded symbols to random position in the buffer
    memcpy(&OffsetEncodedPlayback[randomOffset], encodedplayback, sizeof(double)*(prefixedSymbolSize * nSymbolsToTx ));

    size_t bufferCounter = 0; 
    size_t symbolCounter = 0;
    size_t nDecodedBytes = 0;
    
    // Decode the encoded symbols to see if decoding functions work
    // Until all encoded symbols have been decoded
    while(symbolCounter != nSymbolsToTx)
    {
        // Decode Symbols
        // They are alligned nicley here and start at the index that is passed
        // However there is a delay of one buffer so last symbol cannot be decoded normally because of this the encode playback buffer is greater by one symbol 
        nDecodedBytes = trx.m_RxCallbackData.pCodec->ProcessRxBuffer(&OffsetEncodedPlayback[bufferCounter*prefixedSymbolSize], &rxOut[symbolCounter*nBytes], nBytes);
        // If the number of decoded bytes is greater than 0
        if(nDecodedBytes > 0)
        {   
            // Symbol presumeably decoded correctly
            // Increment counter to indicate this
            symbolCounter++;
        }
        // Increment counter which indicates the 
        bufferCounter++;
    }
    
    // Check the theorethical real-time encoding and decoding is correct 
    for (size_t i = 0; i < nBytes*nSymbolsToTx; i++)
    {
        //printf("%lu Sample: %d vs. %d\n", i, txIn[i], rxOut[i]);
        
        // Check if bytes are same
        BOOST_CHECK_MESSAGE( (txIn[i] == rxOut[i]), "RxOut: Bytes Differ!" ); 
    }

    // Free allocated memory
    free(encodedplayback);
    free(recorded);
    free(txIn);
    free(rxOut);
}
*/


BOOST_AUTO_TEST_CASE(RecordedSelfReceiveing)
{
    std::cout << "\n Recorded Self-Receiveing Test\n" << std::endl;
    // OFDM Codec Settings
    OFDMSettings encoderSettings;
    encoderSettings.type = FFTW_BACKWARD;
    encoderSettings.EnergyDispersalSeed = 10;
    encoderSettings.nFFTPoints = 1024; 
    encoderSettings.PilotToneDistance = 16; 
    encoderSettings.PilotToneAmplitude = 2.0; 
    encoderSettings.QAMSize = 2; 
    encoderSettings.PrefixSize = (size_t) ((encoderSettings.nFFTPoints*2)/4); // 1/4th of symbol

    OFDMSettings decoderSettings = encoderSettings;
    decoderSettings.type = FFTW_FORWARD;

    size_t nSymbolsToTx = 5;

    // Calculate max avaiable points in the symbol for data
    size_t nAvaiablePoints = (encoderSettings.nFFTPoints - ((size_t)(encoderSettings.nFFTPoints / encoderSettings.PilotToneDistance)));
    // Calculate number of bytes that can be encoded per symbol
    size_t nBytes = ((nAvaiablePoints*encoderSettings.QAMSize) / 8);
    // But restrict The spectrum to half the number of avaiable bytes to account for poor speaker frequency response 
    //nBytes /= 2;
    // or Some specified value
    nBytes = 100;
    // Calculate prefixed symbol size 
    size_t prefixedSymbolSize = (encoderSettings.nFFTPoints*2) + encoderSettings.PrefixSize;

    // rt audioSettings
    rtAudioSettings audioSettings;
    audioSettings.SampleRate = 8000;
    audioSettings.BufferFrames = prefixedSymbolSize;
    audioSettings.nChannels = 1;
    audioSettings.InputDevice = 0;
    audioSettings.OutputDevice = 0; // Use Speaker
    audioSettings.InputOffset = 0;
    audioSettings.OutputOffset = 0;

    // Initialize transceiver
    AudioTrx trx(audioSettings, encoderSettings, decoderSettings, RECORDING);

    // Create input & output array
    uint8_t *txIn = (uint8_t*) calloc(nBytes*nSymbolsToTx, sizeof(uint8_t));
    uint8_t *testRxOut = (uint8_t*) calloc(nBytes*nSymbolsToTx, sizeof(uint8_t));
    uint8_t *rxOut = (uint8_t*) calloc(nBytes*nSymbolsToTx, sizeof(uint8_t));

    // Create Tx Buffer
    double *encodedplayback = (double*) calloc(prefixedSymbolSize*nSymbolsToTx, sizeof(double));
    // Configure playback data
    trx.m_playbackData.rxCopy = encodedplayback;
    trx.m_playbackData.nChannels = 1;
    trx.m_playbackData.nTxFrames = 0;
    trx.m_playbackData.counter = 0;
    trx.m_playbackData.frameLimit = nSymbolsToTx*prefixedSymbolSize;

    // Create Rx Buffer
    size_t recordBufferSize = prefixedSymbolSize*(nSymbolsToTx+10); // Make the rec buffer x2 larger for good measure
    double *recorded = (double*) calloc((recordBufferSize), sizeof(double));
    // Configure record data
    trx.m_recordData.rxCopy = recorded;
    trx.m_recordData.nChannels = 1;
    trx.m_recordData.counter = 0;
    trx.m_recordData.nRxFrames = 0;
    trx.m_recordData.frameLimit = prefixedSymbolSize*nSymbolsToTx*2;

    // Setup random seed
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
        // Scale Output to be within +- 1;
        // Scaling this makes transmission inaudibly quiet
        //ScaleToAbsMax(&encodedplayback[i*prefixedSymbolSize], prefixedSymbolSize);
    }

    // Execute transform for last symbol to put data in out buffer
    trx.m_encoder.m_fft.ComputeTransform();
    // Compute & Plot FFTs for ofdm symbol and tx symbol spectrum
    //PlotFFT((double *)trx.m_encoder.m_fft.out, encoderSettings.nFFTPoints);   // This plots the last symbol in the sequence
    //PlotRealFFT(encodedplayback, prefixedSymbolSize);            // This plots the first prefixed symbol in the sequence

    size_t bufferCounter = 0; 
    size_t symbolCounter = 0;
    size_t nDecodedBytes = 0;

    // Start receiver
    trx.StartRxStream();

    // Transmit Encoded Symbols bytes
    trx.StartTxStream();

    //calculate sleep time
    size_t secToSleep = ((nSymbolsToTx*prefixedSymbolSize) / 8000) + 1;
    // Wait to receive 
    sleep(secToSleep);
    std::cout << "Woke up from sleep!" << std::endl;

    bufferCounter = 0; 
    symbolCounter = 0;
    nDecodedBytes = 0;

    // Decode symbols from recored audio
    // Until the # symbols decoded is not equal to the # of transmitted symbols
    while(symbolCounter != nSymbolsToTx) 

    {
        // Process Rx buffer
        //ScaleToAbsMax(&recorded[bufferCounter*prefixedSymbolSize], prefixedSymbolSize);
        nDecodedBytes = trx.m_RxCallbackData.pCodec->ProcessRxBuffer(&recorded[bufferCounter*prefixedSymbolSize], &rxOut[symbolCounter*nBytes], nBytes);
        // Increment buffer counter
        bufferCounter++;
        if(nDecodedBytes > 0)
        {
            // Increment decoded symbol counter
            symbolCounter++;
            //std::cout << symbolCounter << " Symbol Decoded!" << std::endl;
            //std::cout << "bufferCounter = " << bufferCounter << std::endl;
        }

        // whole Rx Buffer has been processed
        if(bufferCounter >= nSymbolsToTx*2)
        {
            // Break while loop
            break;
        }
    }


    size_t byteErrorCount = 0;
    std::vector<double> symbolErrorCounter;
    for(size_t j = 0; j < nSymbolsToTx; j++)
    {
        byteErrorCount = 0;
        // Check if Decoding has been Successfull
        for(size_t i = j*nBytes; i < ((j+1)*nBytes); i++)
        {
            // Print Tx and decoded Rx bytes
            //printf("%lu Byte: %d vs. %d\n", i, txIn[i], rxOut[i]);
            if(txIn[i] != rxOut[i])
            {
                byteErrorCount++;
            }
            
            // Check if real and complex element match within defined precision of each other
            //BOOST_CHECK_MESSAGE( (txIn[i] == rxOut[i]), "Bytes difffer! Index = " << i ); 
        }
        symbolErrorCounter.push_back(byteErrorCount);
        std::cout <<  j  << " Symbol Error Count = " << byteErrorCount << std::endl;
    }

    // Plot recorded & transmitted waveforms
    Plotbuffer(recorded, 20000);
    //Plotbuffer(encodedplayback, prefixedSymbolSize*nSymbolsToTx);

    // Free allocated memory for buffers
    free(encodedplayback);
    free(recorded);
    free(txIn);
    free(testRxOut);
    free(rxOut);
}


BOOST_AUTO_TEST_CASE(RecordedImageSelfReceiveing)
{
    using namespace matplot;
    std::cout << "\n Recorded Self-Receiveing Test\n" << std::endl;
    // OFDM Codec Settings
    OFDMSettings encoderSettings;
    encoderSettings.type = FFTW_BACKWARD;
    encoderSettings.EnergyDispersalSeed = 10;
    encoderSettings.nFFTPoints = 1024; 
    encoderSettings.PilotToneDistance = 16; 
    encoderSettings.PilotToneAmplitude = 2.0; 
    encoderSettings.QAMSize = 2; 
    encoderSettings.PrefixSize = (size_t) ((encoderSettings.nFFTPoints*2)/4); // 1/4th of symbol

    OFDMSettings decoderSettings = encoderSettings;
    decoderSettings.type = FFTW_FORWARD;

    size_t nSymbolsToTx = 5;

    // Calculate max avaiable points in the symbol for data
    size_t nAvaiablePoints = (encoderSettings.nFFTPoints - ((size_t)(encoderSettings.nFFTPoints / encoderSettings.PilotToneDistance)));
    // Calculate number of bytes that can be encoded per symbol
    size_t nBytes = ((nAvaiablePoints*encoderSettings.QAMSize) / 8);
    // But restrict The spectrum to half the number of avaiable bytes to account for poor speaker frequency response 
    //nBytes /= 2;
    // or some specified value
    nBytes = 100;
    // Calculate prefixed symbol size 
    size_t prefixedSymbolSize = (encoderSettings.nFFTPoints*2) + encoderSettings.PrefixSize;

    // rt audioSettings
    rtAudioSettings audioSettings;
    audioSettings.SampleRate = 8000;
    audioSettings.BufferFrames = prefixedSymbolSize;
    audioSettings.nChannels = 1;
    audioSettings.InputDevice = 0;
    audioSettings.OutputDevice = 0;
    audioSettings.InputOffset = 0;
    audioSettings.OutputOffset = 0;

    // Initialize transceiver
    AudioTrx trx(audioSettings, encoderSettings, decoderSettings, REAL_TIME);

    // Create input & output array
    uint8_t *txIn = (uint8_t*) calloc(nBytes*nSymbolsToTx, sizeof(uint8_t));
    uint8_t *testRxOut = (uint8_t*) calloc(nBytes*nSymbolsToTx, sizeof(uint8_t));
    uint8_t *rxOut = (uint8_t*) calloc(nBytes*nSymbolsToTx, sizeof(uint8_t));

    // Configure callbacks data struct
    // Tx callbacks
    trx.m_TxCallbackData.nMaxBytesPerSymbol = nBytes;
    trx.m_TxCallbackData.txBuffer = txIn;

    // Rx callback
    trx.m_RxCallbackData.rxBuffer = rxOut;

    // Start receiver
    trx.StartRxStream();

    // Transmit Encoded Symbols bytes
    trx.StartTxStream();

    //calculate sleep time
    size_t secToSleep = ((nSymbolsToTx*prefixedSymbolSize) / 8000) + 1;
    // Wait to receive 
    sleep(secToSleep);
    std::cout << "Woke up from sleep!" << std::endl;

    size_t byteErrorCount = 0;
    std::vector<double> symbolErrorCounter;
    for(size_t j = 0; j < nSymbolsToTx; j++)
    {
        byteErrorCount = 0;
        // Check if Decoding has been Successfull
        for(size_t i = j*nBytes; i < ((j+1)*nBytes); i++)
        {
            // Print Tx and decoded Rx bytes
            //printf("%lu Byte: %d vs. %d\n", i, txIn[i], rxOut[i]);
            if(txIn[i] != rxOut[i])
            {
                byteErrorCount++;
            }
            
            // Check if real and complex element match within defined precision of each other
            //BOOST_CHECK_MESSAGE( (txIn[i] == rxOut[i]), "Bytes difffer! Index = " << i ); 
        }
        symbolErrorCounter.push_back(byteErrorCount);
        std::cout <<  j  << " Symbol Error Count = " << byteErrorCount << std::endl;
    }

    free(txIn);
    free(testRxOut);
    free(rxOut);
}


BOOST_AUTO_TEST_SUITE_END()
