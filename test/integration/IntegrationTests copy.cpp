#define BOOST_TEST_MODULE IntegrationTest
#include <boost/test/unit_test.hpp>

// For IO
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <unistd.h>

// For measuring elapsed time
#include <chrono>

// For Random Float Generator
#include <time.h>

// For object under test
#include "trx.h"
#include "common.h"


// Integration Tests 
BOOST_AUTO_TEST_SUITE(IntegrationTests)

/**
*  This test simulates entire encoding and decoding process
*  of only one symbol, this test does not transmit the data 
*  through a physical medium.
* 
*/
BOOST_AUTO_TEST_CASE(EncodeDecode)
{
    printf("Testing OFDM Encoder & Decoder Object...\n");
    // Initialize ofdm coder setting structs and objects
    OFDMSettings encoderSettings; 
    encoderSettings.type = FFTW_BACKWARD;
    encoderSettings.EnergyDispersalSeed = 0;
    encoderSettings.nPoints = 1024; 
	encoderSettings.pilotToneStep = 16; 
    encoderSettings.pilotToneAmplitude = 2.0; 
    encoderSettings.guardInterval = 0; 
    encoderSettings.QAMSize = 2; 
    encoderSettings.cyclicPrefixSize = 512; 

    OFDMSettings decoderSettings = encoderSettings;
    decoderSettings.type = FFTW_FORWARD;

    OFDMCodec encoder(encoderSettings);
    OFDMCodec decoder(decoderSettings);

    size_t symbolSize = (encoderSettings.nPoints*2);
    size_t symbolSizeWithPrefix = symbolSize + encoderSettings.cyclicPrefixSize;
    size_t rxSignalSize = symbolSizeWithPrefix * 10;
    size_t rxLastAllowedIndex = symbolSizeWithPrefix * 9;

    // Randomly generated prefix start position
    size_t prefixStart = rand() % rxLastAllowedIndex;
    printf("Randomly Generated Prefix Start = %lu\n",prefixStart);

    // Calculate max nBytes 
    size_t nAvaiablePoints = (encoderSettings.nPoints - ((size_t)(encoderSettings.nPoints / encoderSettings.pilotToneStep)));
    size_t nBytes = (nAvaiablePoints*encoderSettings.QAMSize) / 8;

    std::cout << "Encoding nBytes = " << nBytes << std::endl;

    // Byte input & output buffers
    //ByteVec txIn(nBytes);
    //ByteVec rxOut(nBytes);

    // Signal buffer 
    //DoubleVec txData(symbolSizeWithPrefix);
    //DoubleVec rxSignal(rxSignalSize);
    
    uint8_t * txIn = (uint8_t*) calloc(nBytes, sizeof(uint8_t));
    uint8_t * rxOut = (uint8_t*) calloc(nBytes, sizeof(uint8_t));

    double * txData = (double*) calloc(symbolSizeWithPrefix, sizeof(double));
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

    // Copy the symbol with prefix into Rx signal buffer
    //std::copy(txData.begin(), txData.begin()+symbolSizeWithPrefix, rxSignal.begin()+prefixStart);
    memcpy(&rxSignal[prefixStart], &txData[0], sizeof(double)*symbolSizeWithPrefix);

    // Check the smybol has been copied correctly
    for (size_t i = 0; i < encoderSettings.nPoints*2; i++)
    {
        //printf("Copied vs encoded sample: %lu %+9.5f vs. %+9.5f\n",
        //i, rxSignal[prefixStart+i], txData[i]);
        
        // Check if real and complex element match within defined precision of each other
        BOOST_CHECK_MESSAGE( (rxSignal[prefixStart+i]  == txData[i] ), "Copied Symbol differs!" ); 
    }

    // Decode
    size_t resut = 0;
    size_t counter = 0;
    start = std::chrono::steady_clock::now();
    //decoder.Decode(rxSignal, rxOut, nBytes);
    while(resut == 0)
    {
        resut = decoder.ProcessRxBuffer(&rxSignal[counter*symbolSizeWithPrefix], rxOut);
        counter++;
    }
    end = std::chrono::steady_clock::now();

    std::cout << "Counter Buffer " << counter << std::endl;

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

    start = std::chrono::steady_clock::now();
    end = std::chrono::steady_clock::now();

    std::cout << "start stop elapsed time: "
    << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
    << " ns" << std::endl;

    free(txIn);
    free(rxOut);
    free(txData);
    free(rxSignal);
}

BOOST_AUTO_TEST_CASE(SelfReceiveing)
{
    std::cout << "\nSelf Receiveing\n" << std::endl;
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
    size_t guardInterval = 512;

    OFDMSettings decoderSettings = encoderSettings;
    decoderSettings.type = FFTW_FORWARD;

    // rt audioSettings
    rtAudioSettings audioSettings;
    audioSettings.SampleRate = 8000;
    audioSettings.BufferFrames = 3072; // + 512 guard interval = 3072
    audioSettings.nChannels = 1;
    audioSettings.InputDevice = 0;
    audioSettings.OutputDevice = 0;
    audioSettings.InputOffset = 0;
    audioSettings.OutputOffset = 0;

    // Initialize transceiver
    AudioTrx trx(audioSettings, encoderSettings, decoderSettings);

    // Calculate max nBytes 
    size_t nAvaiablePoints = (encoderSettings.nPoints - ((size_t)(encoderSettings.nPoints / encoderSettings.pilotToneStep)));
    size_t nBytes = (nAvaiablePoints*encoderSettings.QAMSize) / 8;
    size_t nSymbolsToTx = 10;
    size_t nSamplesPerSymbolIncludingGuard = (encoderSettings.nPoints*2) + encoderSettings.cyclicPrefixSize + guardInterval;

    // Create input & output array
    uint8_t *txIn = (uint8_t*) calloc(nBytes*nSymbolsToTx, sizeof(uint8_t));
    uint8_t *testRxOut = (uint8_t*) calloc(nBytes*nSymbolsToTx, sizeof(uint8_t));
    uint8_t *rxOut = (uint8_t*) calloc(nBytes*nSymbolsToTx, sizeof(uint8_t));

    // Create Tx Buffer
    double *encodedplayback = (double*) calloc(nSamplesPerSymbolIncludingGuard*nSymbolsToTx, sizeof(double));
    // Assign Buffer
    trx.m_playbackData.rxCopy = encodedplayback;
    trx.m_playbackData.nChannels = 1;
    trx.m_playbackData.frameLimit = nSymbolsToTx*nSamplesPerSymbolIncludingGuard;


    // Create Rx Buffer
    double *recorded = (double*) calloc((nSamplesPerSymbolIncludingGuard*nSymbolsToTx*2), sizeof(double));
    // Assign Buffer
    trx.m_recordData.rxCopy = recorded;
    trx.m_recordData.nChannels = 1;
    trx.m_recordData.frameLimit = nSamplesPerSymbolIncludingGuard*nSymbolsToTx*2;

    // Setup random
    srand( (unsigned)time( NULL ) );

    // Generate array of random bytes
    for (size_t i = 0; i < nBytes*nSymbolsToTx; i++)
    {
        txIn[i] = rand() % 255;
    }

    // Encode Symbols
    for (size_t i = 0; i < nSymbolsToTx; i++)
    {
        trx.m_TxCallbackData.pCodec->Encode(&txIn[i*nBytes], &encodedplayback[i*nSamplesPerSymbolIncludingGuard], nBytes);
    }

    // Decode 
    // testRxOut
    size_t bufferCounter = 0; 
    size_t symbolCounter = 0;
    size_t ndecodedBytes = 0;
    while(symbolCounter != nSymbolsToTx)
    {
        // Decode Symbols
        // They are alligned nicley here at should start at the index that is passed
        // However last symbol cannot be decoded normally 
        // The current method passes one instance outside of the boundaries of the array FIX THIS
        ndecodedBytes = trx.m_RxCallbackData.pCodec->ProcessRxBuffer(&encodedplayback[bufferCounter*nSamplesPerSymbolIncludingGuard], &testRxOut[symbolCounter*nBytes]);
        if(ndecodedBytes > 0)
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

    // Start receiver
    trx.StartRxStream();

    // Transmit Random bytes
    trx.StartTxStream();

    sleep(8);

     bufferCounter = 0; 
     symbolCounter = 0;
     ndecodedBytes = 0;
    std::cout << " Decoding From Recored!" << std::endl;
    while(symbolCounter != nSymbolsToTx)
    {
        ndecodedBytes = trx.m_RxCallbackData.pCodec->ProcessRxBuffer(&recorded[bufferCounter*nSamplesPerSymbolIncludingGuard], &testRxOut[symbolCounter*nBytes]);
        if(ndecodedBytes > 0)
        {
            symbolCounter++;
            std::cout << symbolCounter << " Symbol Decoded!" << std::endl;
        }
        bufferCounter++;
    }

    // Check the input and output are within threshold
    for (size_t i = 0; i < nBytes; i++)
    {
        //printf("Recovered Sample: %lu %d vs. %d\n", i, txIn[i], rxOut[i]);
        
        // Check if real and complex element match within defined precision of each other
        //BOOST_CHECK_MESSAGE( (txIn[i] == rxOut[i]), "Bytes difffer!" ); 
    }

    free(encodedplayback);
    free(recorded);

    free(txIn);
    free(testRxOut);
    free(rxOut);
}


BOOST_AUTO_TEST_SUITE_END()
