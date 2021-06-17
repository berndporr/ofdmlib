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


// Integration Tests 
BOOST_AUTO_TEST_SUITE(IntegrationTests)

/**
*  This test simulates entire encoding and decoding process.
*
* 
*/
BOOST_AUTO_TEST_CASE(EncodeDecode)
{
    printf("Testing OFDM Coder Object...");
    //BOOST_TEST_MESSAGE( "Testing:" );
    //BOOST_TEST_MESSAGE( "Variable:" << variable );
    uint16_t nPoints = 512;
    bool complexTimeSeries = false;
    uint16_t pilotToneStep = 16;
    uint16_t qamSize = 4;

    // Initialize ofdm coder objects
    OFDMCodec encoder(-1, nPoints, complexTimeSeries, pilotToneStep, qamSize);
    OFDMCodec decoder(1, nPoints, complexTimeSeries, pilotToneStep, qamSize);

    //BOOST_CHECK_MESSAGE( sample != 0, "samples do not match:  " << sample 1 << " " << "  Initial Value Hasn't Changed" );

}
BOOST_AUTO_TEST_SUITE_END()



