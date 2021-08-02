#define BOOST_TEST_MODULE TrxTest
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


BOOST_AUTO_TEST_CASE(SelfReceiveing)
{
    // OFDM Codec Settings
    OFDMSettings encoderSettings;
    encoderSettings.type = FFTW_BACKWARD;
    encoderSettings.EnergyDispersalSeed = 0;
    encoderSettings.nPoints = 1024; 
    encoderSettings.pilotToneStep = 16; 
    encoderSettings.pilotToneAmplitude = 2.0; 
    encoderSettings.guardInterval = 0; 
    encoderSettings.QAMSize = 2; 
    encoderSettings.cyclicPrefixSize = (int) ((encoderSettings.nPoints*2)/4); // 1/4th of symbol

    OFDMSettings decoderSettings = encoderSettings;
    decoderSettings.type = FFTW_FORWARD;

    // rt audioSettings
    rtAudioSettings audioSettings;
    audioSettings.SampleRate = 8000;
    audioSettings.BufferFrames = 2560;
    audioSettings.nChannels = 1;
    audioSettings.InputDevice = 0;
    audioSettings.OutputDevice = 0;
    audioSettings.InputOffset = 0;
    audioSettings.OutputOffset = 0;

    // Initialize transceiver
    AudioTrx trx(audioSettings, encoderSettings, decoderSettings);

    // Start receiver
    trx.StartRxStream();

    // Sleep just to make sure receiver settles
    // This is most likley not needed
    sleep(8);

    // Transmit Random bytes
    trx.StartTxStream();

    sleep(8);

}


BOOST_AUTO_TEST_SUITE_END()
