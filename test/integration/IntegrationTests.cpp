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

#define FFT_DIFFERENCE_THRESHOLD 0.0000000000001

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
	encoderSettings.complexTimeSeries = false;
    encoderSettings.EnergyDispersalSeed = 0;
    encoderSettings.nPoints = 512; 
	encoderSettings.pilotToneStep = 16; 
    encoderSettings.pilotToneAmplitude = 2.0; 
    encoderSettings.guardInterval = 0; 
    encoderSettings.QAMSize = 4; 
    encoderSettings.cyclicPrefixSize = 128; 

    OFDMSettings decoderSettings = encoderSettings;
    decoderSettings.type = FFTW_FORWARD;

    uint32_t symbolSize = (encoderSettings.nPoints*2);
    uint32_t symbolSizeWithPrefix = symbolSize + encoderSettings.cyclicPrefixSize;
    uint32_t rxSignalSize = symbolSizeWithPrefix * 10;
    uint32_t rxLastAllowedIndex = symbolSizeWithPrefix * 9;

    DoubleVec txData;
    txData.resize(symbolSizeWithPrefix);

    DoubleVec rxSignal;
    rxSignal.resize(rxSignalSize);

    size_t dataSize = 2*(encoderSettings.nPoints - ((size_t)(encoderSettings.nPoints / encoderSettings.pilotToneStep)));

    DoubleVec txIn;
    txIn.resize(dataSize);
    DoubleVec rxOut;
    rxOut.resize(dataSize);

    OFDMCodec encoder(encoderSettings); // txData
    OFDMCodec decoder(decoderSettings); // rxSignal

    // Setup random float generator
    srand( (unsigned)time( NULL ) );

    // Generate Array of random floats
    for (size_t i = 0; i < dataSize; i++)
    {
        txIn[i] = (double) rand()/RAND_MAX;
    }

    // Encode
    auto start = std::chrono::steady_clock::now();
    txData = encoder.Encode(txIn);
    auto end = std::chrono::steady_clock::now();

    std::cout << "Encode elapsed time: "
    << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
    << " ns" << std::endl;

    // Generate random index value for symbol start
    uint32_t symbolStart = rand() % rxLastAllowedIndex;
    printf("Randomly Generated Prefix Start = %d\n",symbolStart);

    // Copy the symbol with prefix into Rx signal buffer
    std::copy(txData.begin(), txData.begin()+symbolSizeWithPrefix, rxSignal.begin()+symbolStart);

    // Check the smybol has been copied correctly
    for (size_t i = 0; i < encoderSettings.nPoints*2; i++)
    {
        
        //printf("Copied vs encoded sample: %3d %+9.5f vs. %+9.5f\n",
        //i, rxSignal[symbolStart+i], txData[i]);
        
        // Check if real and complex element match within defined precision of each other
        BOOST_CHECK_MESSAGE( (rxSignal[symbolStart+i]  == txData[i] ), "Copied Symbol differs!" ); 
    }

    // Decode
    start = std::chrono::steady_clock::now();
    rxOut = decoder.Decode(rxSignal);
    end = std::chrono::steady_clock::now();

    std::cout << "Decode elapsed time: "
    << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
    << " ns" << std::endl;
    
    // Check the input and output are within threshold
    for (uint32_t i = 0; i < txIn.size(); i++)
    {
        //printf("Recovered Sample: %3d %+9.5f vs. %+9.5f\n", i, txIn[i], rxOut[i]);
        
        // Check if real and complex element match within defined precision of each other
        BOOST_CHECK_MESSAGE( (std::abs((txIn[i] - rxOut[i]))  <= FFT_DIFFERENCE_THRESHOLD), "Values vary more than threshold!" ); 
    }
    
}
BOOST_AUTO_TEST_SUITE_END()
