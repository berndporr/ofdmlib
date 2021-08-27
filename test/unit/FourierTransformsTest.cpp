#define BOOST_TEST_MODULE FourierTransformTest
#include <boost/test/unit_test.hpp>

// For IO
#include <stdlib.h>
#include <math.h>
#include <cmath> 
#include <iostream>
#include <unistd.h>

// For measuring elapsed time
#include <chrono>

// For Random Float Generator
#include <time.h>

// For object under test
#include "fft.h"

#define FFT_DIFFERENCE_THRESHOLD 0.0000000000001
#define FFT_NUMERICAL_THRESHOLD 0.0000000001

// Test FFT & IFFT
/**
* Test FFT & IFFT
* @todo: Unify the printing standard and formating
*/
BOOST_AUTO_TEST_SUITE(FOURIER_TRANSFORMS)



/**
* Generate random data (floats) Put it throguh IFFT.
* Copy the time domain samples into FFT input buffer.
* Execute and check the input and output are within a
* threshold value.
* 
*/
BOOST_AUTO_TEST_CASE(IFFTtoFFT)
{
    printf("\nTesting IFFT to IFFT...\n");
    printf("\nInverse Fourier transform:\n");

    // Setup random float generator
    srand( (unsigned)time( NULL ) );

    OFDMSettingsStruct encoderSettingsStruct;
    encoderSettingsStruct.type = FFTW_BACKWARD;
    encoderSettingsStruct.EnergyDispersalSeed = 10;
    encoderSettingsStruct.nFFTPoints = 1024; 
    encoderSettingsStruct.PilotToneDistance = 16; 
    encoderSettingsStruct.PilotToneAmplitude = 2.0; 
    encoderSettingsStruct.QAMSize = 2; 
    encoderSettingsStruct.PrefixSize = (size_t) ((encoderSettingsStruct.nFFTPoints*2)/4); // 1/4th of symbol

    OFDMSettingsStruct decoderSettingsStruct = encoderSettingsStruct;
    decoderSettingsStruct.type = FFTW_FORWARD;

    OFDMSettings encoderSettings(encoderSettingsStruct);
    OFDMSettings decoderSettings(decoderSettingsStruct);

    FFT ifft(encoderSettings);
    FFT fft(decoderSettings);

    // Generate random floats 
    for (size_t i = 0; i < encoderSettings.m_nFFTPoints; i++)
    {
        ifft.in[i][0] = (double) rand()/RAND_MAX;
        ifft.in[i][1] = (double) rand()/RAND_MAX;
    }
    
    // Measure wall time of the ifft execution.
    auto start = std::chrono::steady_clock::now();
    ifft.ComputeTransform();
    auto end = std::chrono::steady_clock::now();
 
    std::cout << "ifft Elapsed time in nanoseconds: "
        << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
        << " ns" << std::endl;

    /*
    for (size_t i = 0; i < encoderSettings.nFFTPoints; i++)
    {
        printf("Time: %3d %+9.5f j%+9.5f \n", i, ifft.out[i][0], ifft.out[i][1]);
    }
    */

    printf("\nFourier Transform:\n");
    // Assign the output of ifft to the input of fft
    for (size_t i = 0; i < encoderSettings.m_nFFTPoints ; i++)
    {
        fft.in[i][0] = ifft.out[i][0];
        fft.in[i][1] = ifft.out[i][1];
    }

    // Measure wall time of the fft execution.
    start = std::chrono::steady_clock::now();
    fft.ComputeTransform();
    end = std::chrono::steady_clock::now();
 
    std::cout << "fft Elapsed time in nanoseconds: "
        << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
        << " ns" << std::endl;

    // Normalize samples
    fft.Normalise();  // TODO: Check this works

    // Print input and output buffers
    for (size_t i = 0; i < encoderSettings.m_nFFTPoints ; i++)
    {
        /*
        printf("Recovered Sample(time domain) %3d %+9.5f j%+9.5f Input to IFFT vs. %+9.5f j%+9.5f Output of FFT\n",
        i, ifft.in[i][0], ifft.in[i][1], fft.out[i][0], fft.out[i][1]);
        */

        // Check if real and complex element match within defined precision.
        BOOST_CHECK_MESSAGE(
         ( (std::abs( ifft.in[i][0] - fft.out[i][0] ) <= FFT_DIFFERENCE_THRESHOLD ) ||
         (  std::abs( ifft.in[i][1] - fft.out[i][1] ) <= FFT_DIFFERENCE_THRESHOLD )), 
         "Values vary more than threshold! - Occured at index: " << i );  

    }
    fftw_cleanup();
}



/**
* Generate cosine of one frequency in time domain, put it throguh IFFT.
* Copy the time domain samples into FFT input buffer.
* Execute and check the input and output are within a
* threshold value. Each time changing the frequency of
* cosine. 
* 
*/
BOOST_AUTO_TEST_CASE(IFFTtoFFTCosine)
{
    printf("\nTesting IFFT to FFT Using Cosine...\n");

    // Setup random float generator
    srand( (unsigned)time( NULL ) );

    OFDMSettingsStruct encoderSettingsStruct;
    encoderSettingsStruct.type = FFTW_BACKWARD;
    encoderSettingsStruct.EnergyDispersalSeed = 10;
    encoderSettingsStruct.nFFTPoints = 1024; 
    encoderSettingsStruct.PilotToneDistance = 16; 
    encoderSettingsStruct.PilotToneAmplitude = 2.0; 
    encoderSettingsStruct.QAMSize = 2; 
    encoderSettingsStruct.PrefixSize = (size_t) ((encoderSettingsStruct.nFFTPoints*2)/4); // 1/4th of symbol

    OFDMSettingsStruct decoderSettingsStruct = encoderSettingsStruct;
    decoderSettingsStruct.type = FFTW_FORWARD;

    OFDMSettings encoderSettings(encoderSettingsStruct);
    OFDMSettings decoderSettings(decoderSettingsStruct);

    FFT ifft(encoderSettings);
    FFT fft(decoderSettings);
    
    for(size_t j = 0; j < encoderSettings.m_nFFTPoints ; j++)
    {
        // Generate Cosine wave in freq domain
        for(size_t i = 1; i < encoderSettings.m_nFFTPoints ; i++)
        {
            // Insert value at desired frequency
            if (i == j)
            {
                ifft.in[i][0] = encoderSettings.m_nFFTPoints /2;
                ifft.in[i][1] = 0; 
            }

            // Modify the negative frequency
            else if (i == encoderSettings.m_nFFTPoints -j)
            {
                ifft.in[i][0] = encoderSettings.m_nFFTPoints /2;
                ifft.in[i][1] = 0; 
            }
            // Fill with 0 otherwise
            else
            {
                ifft.in[i][0] = encoderSettings.m_nFFTPoints/2;
                ifft.in[i][1] = 0;     
            }

        }
        // Inverse Fourier transform
        //printf("\nInverse Fourier transform:\n");

        // Measure wall time of the ifft execution.
        auto start = std::chrono::steady_clock::now();
        ifft.ComputeTransform();
        auto end = std::chrono::steady_clock::now();
    
        // Print elapsed time
        /*
        std::cout << "ifft Elapsed time in nanoseconds: "
            << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
            << " ns" << std::endl;
        */

        // Print fft data
        /*
        for (size_t i = 0; i < encoderSettings.nFFTPoints; i++)
        {
            printf("Time: %3d %+9.5f j%+9.5f \n", i, ifft.out[i][0], ifft.out[i][1]);
        }
        */

        // Fourier Transform
        // printf("\nFourier Transform:\n");
        // Assign the output of forward fft to the input of backward fft
        for (size_t i = 0; i < encoderSettings.m_nFFTPoints ; i++)
        {
            fft.in[i][0] = ifft.out[i][0];
            fft.in[i][1] = ifft.out[i][1];
        }

        // Measure wall time of the fft execution.
        start = std::chrono::steady_clock::now();
        fft.ComputeTransform();
        end = std::chrono::steady_clock::now();

        // Print elapsed time
        /*
        std::cout << "fft Elapsed time in nanoseconds: "
            << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
            << " ns" << std::endl;
        */

        // Print input and output buffers
        for (size_t i = 0; i < encoderSettings.m_nFFTPoints ; i++)
        {
            // Check if real and complex element match within defined precision.
            BOOST_CHECK_MESSAGE(
            ( (std::abs( ifft.in[i][0] - fft.out[i][0] ) <= FFT_DIFFERENCE_THRESHOLD ) ||
            (  std::abs( ifft.in[i][1] - fft.out[i][1] ) <= FFT_DIFFERENCE_THRESHOLD )), 
            "Values vary more than threshold! - Occured at: index j = " << j << " & Index i = " << i );   
        }

    }  
    fftw_cleanup();
}


/**
* Generate random data (floats) Put it throguh FFT.
* Copy the frequency domain samples into IFFT input buffer.
* Execute and check the input and output are within a
* threshold value.
* 
*/
BOOST_AUTO_TEST_CASE(FFTtoIFFT)
{
    printf("\nTesting FFT to IFFT...\n");
    printf("\nFourier transform:\n");

    OFDMSettingsStruct encoderSettingsStruct;
    encoderSettingsStruct.type = FFTW_BACKWARD;
    encoderSettingsStruct.EnergyDispersalSeed = 10;
    encoderSettingsStruct.nFFTPoints = 1024; 
    encoderSettingsStruct.PilotToneDistance = 16; 
    encoderSettingsStruct.PilotToneAmplitude = 2.0; 
    encoderSettingsStruct.QAMSize = 2; 
    encoderSettingsStruct.PrefixSize = (size_t) ((encoderSettingsStruct.nFFTPoints*2)/4); // 1/4th of symbol

    OFDMSettingsStruct decoderSettingsStruct = encoderSettingsStruct;
    decoderSettingsStruct.type = FFTW_FORWARD;

    OFDMSettings encoderSettings(encoderSettingsStruct);
    OFDMSettings decoderSettings(decoderSettingsStruct);

    // Setup random float generator
    srand( (unsigned)time( NULL ) );

    FFT forwardfft(decoderSettings);
    FFT backwardfft(encoderSettings);

    // Generate random floats 
    for (size_t i = 0; i < encoderSettings.m_nFFTPoints; i++)
    {
        forwardfft.in[i][0] = (double) rand()/RAND_MAX;
        forwardfft.in[i][1] = (double) rand()/RAND_MAX;
    }
    
    // Measure wall time of the ifft execution.
    auto start = std::chrono::steady_clock::now();
    forwardfft.ComputeTransform();
    auto end = std::chrono::steady_clock::now();
   
    // Print elapsed time
    std::cout << "forwardfft Elapsed time in nanoseconds: "
        << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
        << " ns" << std::endl;

    /*
    for (size_t i = 0; i < nFFTPoints; i++)
    {
        printf("freq: %3d %+9.5f j%+9.5f \n", i, forwardfft.out[i][0], forwardfft.out[i][1]);
    }
    */

    printf("\nInverse Fourier Transform:\n");
    // Assign the output of forward fft to the input of backward fft
    for (size_t i = 0; i < encoderSettings.m_nFFTPoints; i++)
    {
        backwardfft.in[i][0] = forwardfft.out[i][0];
        backwardfft.in[i][1] = forwardfft.out[i][1];
    }

    // Measure wall time of the fft execution.
    start = std::chrono::steady_clock::now();
    backwardfft.ComputeTransform();
    end = std::chrono::steady_clock::now();
 
    std::cout << "backwardfft Elapsed time in nanoseconds: "
        << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
        << " ns" << std::endl;

    // Normalize samples
    for (size_t i = 0; i < encoderSettings.m_nFFTPoints; i++)
    {
        backwardfft.out[i][0] *= 1./encoderSettings.m_nFFTPoints;
        backwardfft.out[i][1] *= 1./encoderSettings.m_nFFTPoints;
    }

    // Print input and output buffers
    for (size_t i = 0; i < encoderSettings.m_nFFTPoints; i++)
    {
        //printf("Recovered Sample(time domain) %3d %+9.5f j%+9.5f Input to fft vs. %+9.5f j%+9.5f Output of IFFT\n",
        //i, forwardfft.in[i][0], forwardfft.in[i][1], backwardfft.out[i][0], backwardfft.out[i][1]);

        // Check if real and complex element match within defined precision.
        BOOST_CHECK_MESSAGE(
         ( (std::abs( forwardfft.in[i][0] - backwardfft.out[i][0] ) <= FFT_DIFFERENCE_THRESHOLD ) ||
         (  std::abs( forwardfft.in[i][1] - backwardfft.out[i][1] ) <= FFT_DIFFERENCE_THRESHOLD )), 
         "Values vary more than threshold! - Occured at index: " << i );  

    }
    fftw_cleanup();
}

/**
* Generate cosine of one frequency in time domain, put it throguh FFT.
* Copy the frequency domain samples into IFFT input buffer.
* Execute and check the input and output are within a
* threshold value. Each time changing the frequency of
* cosine. 
* 
*/
BOOST_AUTO_TEST_CASE(FFTtoIFFTCosine)
{
    printf("\nTesting FFT to IFFT Using Cosine...\n");

    // Setup random float generator
    srand( (unsigned)time( NULL ) );

    // OFDM Codec Settings
    OFDMSettingsStruct encoderSettingsStruct;
    encoderSettingsStruct.type = FFTW_BACKWARD;
    encoderSettingsStruct.EnergyDispersalSeed = 10;
    encoderSettingsStruct.nFFTPoints = 1024; 
    encoderSettingsStruct.PilotToneDistance = 16; 
    encoderSettingsStruct.PilotToneAmplitude = 2.0; 
    encoderSettingsStruct.QAMSize = 2; 
    encoderSettingsStruct.PrefixSize = (size_t) ((encoderSettingsStruct.nFFTPoints*2)/4); // 1/4th of symbol

    OFDMSettingsStruct decoderSettingsStruct = encoderSettingsStruct;
    decoderSettingsStruct.type = FFTW_FORWARD;

    OFDMSettings encoderSettings(encoderSettingsStruct);
    OFDMSettings decoderSettings(decoderSettingsStruct);


    FFT forwardfft(decoderSettings);
    FFT backwardfft(encoderSettings);

    for(size_t j = 0; j < encoderSettings.m_nFFTPoints; j++)
    {
        // Generate Cosine wave in time domain
        for(size_t i = 0; i < encoderSettings.m_nFFTPoints; i++)
        {
            forwardfft.in[i][0] = cos(j * 2*M_PI*i/encoderSettings.m_nFFTPoints);
            forwardfft.in[i][1] = 0; 
        }

        // Fourier transform:
        //printf("\nFourier transform:\n");

        // Measure wall time of the ifft execution.
        auto start = std::chrono::steady_clock::now();
        forwardfft.ComputeTransform();
        auto end = std::chrono::steady_clock::now();
        
        // Print elapsed time
        /*
        std::cout << "forwardfft Elapsed time in nanoseconds: "
            << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
            << " ns" << std::endl;
        */

        // Print fft data
        for (size_t i = 1; i < encoderSettings.m_nFFTPoints; i++)
        {
            //printf("Freq: %3d %+9.5f j%+9.5f \n", i, forwardfft.out[i][0], forwardfft.out[i][1]);
            
            if(i==j)
            {
                // Check if real and complex element match within defined precision.
                BOOST_CHECK_MESSAGE( ( forwardfft.out[i][0] > FFT_NUMERICAL_THRESHOLD ), 
                "Value is below threshold! - Occured at: index j = " << j << " & Index i = " << i );  
            }
            else if(i == (encoderSettings.m_nFFTPoints -j))
            {
                // Check if real and complex element match within defined precision.
                BOOST_CHECK_MESSAGE( ( forwardfft.out[i][0] > FFT_NUMERICAL_THRESHOLD ), 
                "Value is below threshold! - Occured at: index j = " << j << " & Index i = " << i);  
            }         
            else
            {
                // Check if real and complex element match within defined precision.
                BOOST_CHECK_MESSAGE( ( forwardfft.out[i][0] < FFT_NUMERICAL_THRESHOLD ), 
                "Value is above threshold! - Occured at: index j = " << j << " & Index i = " << i );  
            }
         }
        
        // Inverse Fourier Transform
        // printf("\nInverse Fourier Transform:\n");
        // Assign the output of forward fft to the input of backward fft
        for (size_t i = 0; i < encoderSettings.m_nFFTPoints; i++)
        {
            backwardfft.in[i][0] = forwardfft.out[i][0];
            backwardfft.in[i][1] = forwardfft.out[i][1];
        }

        // Measure wall time of the fft execution.
        start = std::chrono::steady_clock::now();
        backwardfft.ComputeTransform();
        end = std::chrono::steady_clock::now();
    
        // Print elapsed time
        /*
        std::cout << "backwardfft Elapsed time in nanoseconds: "
            << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
            << " ns" << std::endl;
        */

        // Normalize 
        for (size_t i = 0; i < encoderSettings.m_nFFTPoints; i++)
        {
            backwardfft.out[i][0] *= 1./encoderSettings.m_nFFTPoints;
            backwardfft.out[i][1] *= 1./encoderSettings.m_nFFTPoints;
        }

        // Print input and output buffers
        for (size_t i = 0; i < encoderSettings.m_nFFTPoints; i++)
        {
            // Check if real and complex element match within defined precision.
            BOOST_CHECK_MESSAGE(
            ( (std::abs( forwardfft.in[i][0] - backwardfft.out[i][0] ) <= FFT_DIFFERENCE_THRESHOLD ) ||
            (  std::abs( forwardfft.in[i][1] - backwardfft.out[i][1] ) <= FFT_DIFFERENCE_THRESHOLD )), 
            "Values vary more than threshold! - Occured at: index j = " << j << " & Index i = " << i );   
        }

    }  
    fftw_cleanup();
}

/**
* Generate random data (floats) Put it throguh FFT.
* Copy the frequency domain samples into IFFT input buffer.
* Execute and check the input and output are within a
* threshold value. Each time reconfiguring the size of 
* fft.
* 
*/
BOOST_AUTO_TEST_CASE(Reconfiguration)
{
    printf("\nTesting Object Reconfiguration...\n");

    // Setup random float generator
    srand( (unsigned)time( NULL ) );

    // OFDM Codec Settings
    OFDMSettingsStruct encoderSettingsStruct;
    encoderSettingsStruct.type = FFTW_BACKWARD;
    encoderSettingsStruct.EnergyDispersalSeed = 10;
    encoderSettingsStruct.nFFTPoints = 1024; 
    encoderSettingsStruct.PilotToneDistance = 16; 
    encoderSettingsStruct.PilotToneAmplitude = 2.0; 
    encoderSettingsStruct.QAMSize = 2; 
    encoderSettingsStruct.PrefixSize = (size_t) ((encoderSettingsStruct.nFFTPoints*2)/4); // 1/4th of symbol

    OFDMSettingsStruct decoderSettingsStruct = encoderSettingsStruct;
    decoderSettingsStruct.type = FFTW_FORWARD;

    OFDMSettings encoderSettings(encoderSettingsStruct);
    OFDMSettings decoderSettings(decoderSettingsStruct);

    // Initialize fft object with size of 1 
    FFT forwardfft(decoderSettings);
    FFT backwardfft(encoderSettings);
    
    size_t PowOf2Arr[] = { 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384 };
    size_t nTestSizes = sizeof(PowOf2Arr) / sizeof(PowOf2Arr[0]);

    // For each array size 
    for(size_t j = 0; j < nTestSizes; j++)
    {
        printf("\nTesting %lu Point FFT\n",PowOf2Arr[j]);
        encoderSettings.m_nFFTPoints = PowOf2Arr[j];
        decoderSettings.m_nFFTPoints = PowOf2Arr[j];
        forwardfft.Configure();
        backwardfft.Configure();
        // Generate random floats 

        for (size_t i = 0; i < PowOf2Arr[j]; i++)
        {
            forwardfft.in[i][0] = (double) rand()/RAND_MAX;
            forwardfft.in[i][1] = (double) rand()/RAND_MAX;
        }

        // Measure wall time of the ifft execution.
        auto start = std::chrono::steady_clock::now();
        forwardfft.ComputeTransform();
        auto end = std::chrono::steady_clock::now();
    
        std::cout << "Fourier transform elapsed time: "
            << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
            << " ns" << std::endl;

        // Print Data output of forward FFT
        /*
        for (size_t i = 0; i < PowOf2Arr[j]; i++)
        {
            printf("Freq: %3d %+9.5f j%+9.5f \n", i, forwardfft.out[i][0], forwardfft.out[i][1]);
        }
        */

        // Assign the output of forward fft to the input of backward fft
        for (size_t i = 0; i < PowOf2Arr[j]; i++)
        {
            backwardfft.in[i][0] = forwardfft.out[i][0];
            backwardfft.in[i][1] = forwardfft.out[i][1];
        }


        // Measure wall time of the fft execution.
        start = std::chrono::steady_clock::now();
        backwardfft.ComputeTransform();
        end = std::chrono::steady_clock::now();
    
        std::cout << "Inverse Fourier Transform elapsed time: "
            << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
            << " ns" << std::endl;

        // Normalize 
        for (size_t i = 0; i < PowOf2Arr[j]; i++)
        {
            backwardfft.out[i][0] *= 1./PowOf2Arr[j];
            backwardfft.out[i][1] *= 1./PowOf2Arr[j];
        }

        // Print input and output buffers
        for (size_t i = 0; i < PowOf2Arr[j]; i++)
        {
            //printf("Recovered Sample: %3d %+9.5f j%+9.5f Input to fft vs. %+9.5f j%+9.5f Output of IFFT\n",
            //i, forwardfft.in[i][0], forwardfft.in[i][1], backwardfft.out[i][0], backwardfft.out[i][1]);

            // Check if real and complex element match within defined precision of each other
            BOOST_CHECK_MESSAGE(
            ( (std::abs( forwardfft.in[i][0] - backwardfft.out[i][0] ) <= FFT_DIFFERENCE_THRESHOLD ) ||
            (  std::abs( forwardfft.in[i][1] - backwardfft.out[i][1] ) <= FFT_DIFFERENCE_THRESHOLD )), 
            "Values vary more than threshold! - Occured at: index j = " << j << " & Index i = " << i );  

        }
    }
    fftw_cleanup();
}
BOOST_AUTO_TEST_SUITE_END()
