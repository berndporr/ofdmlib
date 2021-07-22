/**
* @file TRX.h
* @author Kamil Rog
*
* 
*/
#ifndef TRX_H

#include <string>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <math.h>
#include <fftw3.h>
#include <cstring>

#include <cstddef>
#include<rtaudio/RtAudio.h>

#include "ofdmcodec.h"
#include "common.h"

typedef double  MY_TYPE;
#define FORMAT RTAUDIO_FLOAT64
#define SCALE  1.0;

/**
 * 
 * 
 */
struct rtAudioSettings
{
    unsigned int SampleRate;
    unsigned int BufferFrames;
    unsigned int nChannels = 1;
    unsigned int Device = 0;
    unsigned int offset = 0;
    unsigned int InputDevice = 0;
    unsigned int OutputDevice = 0;
    unsigned int InputOffset = 0;
    unsigned int OutputOffset = 0;
};

/**
 * 
 * 
 */
struct CallbackData
{
  MY_TYPE* buffer; // This is going to be a ByteVec in the next adaptation
  OFDMCodec *pCodec; // Pointer to the ofdm codec 
  unsigned long bufferBytes;
  unsigned long totalFrames;
  unsigned long frameCounter;
};


struct OutputData
{
  FILE *fd;
  unsigned int channels;
};



/**
 * @brief TRX object, wrapper around the rtaudio API for ofdmlib example use
 * 
 */
class AudioTrx {

public:


	/**
	* Constructor
	* 
	* @param audioSettings 
    * @param settingsStruct 
	*
	*/
	AudioTrx(rtAudioSettings audioSettings, OFDMSettings settingsStruct);


	/**
	* Destructor 
	*
	*/
	~AudioTrx();

    void StartTxStream();
    void StopTxStream();

    void StartRxStream();
    void StopRxStream();


private:
    OFDMCodec m_ofdmCodec;
    rtAudioSettings m_rtAudioSettings;

    // Tx //
    // RtAudio Object
    RtAudio dac; // Digital-to-analog (Tx)
    RtAudio::StreamParameters m_OutputParams;
    CallbackData m_TxCallbackData;
    unsigned int m_TxBufferFrames;
    OutputData rawPlaybackData;


    // Rx //
    // RtAudio Object
    RtAudio adc; // Analog-to-digital (Rx) 
    RtAudio::StreamParameters m_InputParams;
    CallbackData m_RxCallbackData;    
    unsigned int m_RxBufferFrames;

};


#endif
