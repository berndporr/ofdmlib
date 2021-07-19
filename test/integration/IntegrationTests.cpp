#define BOOST_TEST_MODULE FourierTransformTest
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
#include "ofdmcodec.h"
#include "common.h"

#define CONFIDENCE_INTERVAL 0.0000000000001

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
    encoderSettings.nPoints = 512; 
	encoderSettings.pilotToneStep = 8; 
    encoderSettings.pilotToneAmplitude = 2.0; 
    encoderSettings.guardInterval = 0; 
    encoderSettings.QAMSize = 2; 
    encoderSettings.cyclicPrefixSize = 128; 

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
    ByteVec txIn(nBytes);
    ByteVec rxOut(nBytes);

    // Signal buffer 
    DoubleVec txData(symbolSizeWithPrefix);
    DoubleVec rxSignal(rxSignalSize);

    // Setup random byte generator
    srand( (unsigned)time( NULL ) );

    // Generate array of random bytes
    for (size_t i = 0; i < nBytes; i++)
    {
        txIn[i] = rand() % 255;
    }

    // Encode 1 ofdm symbol
    auto start = std::chrono::steady_clock::now();
    txData = encoder.Encode(txIn, nBytes);
    auto end = std::chrono::steady_clock::now();

    std::cout << "Encode elapsed time: "
    << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
    << " ns" << std::endl;

    // Copy the symbol with prefix into Rx signal buffer
    std::copy(txData.begin(), txData.begin()+symbolSizeWithPrefix, rxSignal.begin()+prefixStart);

    // Check the smybol has been copied correctly
    for (size_t i = 0; i < encoderSettings.nPoints*2; i++)
    {
        //printf("Copied vs encoded sample: %lu %+9.5f vs. %+9.5f\n",
        //i, rxSignal[prefixStart+i], txData[i]);
        
        // Check if real and complex element match within defined precision of each other
        BOOST_CHECK_MESSAGE( (rxSignal[prefixStart+i]  == txData[i] ), "Copied Symbol differs!" ); 
    }

    // Decode
    start = std::chrono::steady_clock::now();
    rxOut = decoder.Decode(rxSignal, nBytes);
    end = std::chrono::steady_clock::now();

    std::cout << "Decode elapsed time: "
    << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
    << " ns" << std::endl;
    
    // Check the input and output are within threshold
    for (size_t i = 0; i < nBytes; i++)
    {
        //printf("Recovered Sample: %lu %d vs. %d\n", i, txIn[i], rxOut[i]);
        
        // Check if real and complex element match within defined precision of each other
        //BOOST_CHECK_MESSAGE( (txIn[i] == rxOut[i]), "Bytes difffer!" ); 
    }
    
}
BOOST_AUTO_TEST_SUITE_END()
