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
#include <rtaudio/RtAudio.h>

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
    unsigned int InputDevice = 0;
    unsigned int OutputDevice = 0;
    unsigned int InputOffset = 0;
    unsigned int OutputOffset = 0;
};


/**
 * 
 * 
 */
struct TxCallbackData
{
  uint8_t *txBuffer; 
  OFDMCodec *pCodec;
  size_t nBytes;
  size_t nMaxBytesPerSymbol; // This doesn't have to be actual max, user can define anything from 1 up to the max here
  size_t nTxByteCounter; // Number of transmitted bytes used to specify the offset for ofdm encoding function
};


/**
 * 
 * 
 */
struct RxCallbackData
{
  size_t cbCounter; 
  double * rxCopy;
  uint8_t *rxBuffer; 
  OFDMCodec *pCodec;
  size_t nRxBytes;
  size_t nMaxBytesPerSymbol; // This doesn't have to be actual max, user can define anything from 1 up to the max here
};


/**
 * Used for test purposes
 * 
 */
struct RecordData
{
  size_t counter; 
  size_t frameLimit;
  size_t nRxFrames;
  size_t nChannels;
  double *rxCopy;
};


/**
 * Used for test purposes
 * 
 */
struct PlaybackData
{
  size_t counter; 
  size_t frameLimit;
  size_t nTxFrames;
  size_t nChannels;
  double *rxCopy;
};


struct OutputData
{
  FILE *fd;
  unsigned int channels;
};


/**
* Operation mode of the transciever  
*
*/
typedef enum {
	REAL_TIME	=  0,
	RECORDING =  1,
} TRX_OPERATION_MODE;



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
	AudioTrx(rtAudioSettings audioSettings, OFDMSettings encoderSettings, OFDMSettings decoderSettings, TRX_OPERATION_MODE operationMode);


	/**
	* Destructor 
	*
	*/
	~AudioTrx();

    void StartTxStream();
    void StopTxStream();

    void StartRxStream();
    void StopRxStream();

    void OpenStreams(size_t mode);

    RxCallbackData m_RxCallbackData;   
    TxCallbackData m_TxCallbackData;

    PlaybackData m_playbackData;
    RecordData m_recordData;

    double *rxCopy;

    OFDMCodec m_encoder;
    OFDMCodec m_decoder;
private:

    rtAudioSettings m_rtAudioSettings;

    uint8_t *txIn;
    uint8_t *rxOut;

    // Tx //
    // RtAudio Object
    RtAudio dac; // Digital-to-analog (Tx)
    RtAudio::StreamParameters m_OutputParams;
    unsigned int m_TxBufferFrames;


    // Rx //
    // RtAudio Object
    RtAudio adc; // Analog-to-digital (Rx) 
    RtAudio::StreamParameters m_InputParams;
    unsigned int m_RxBufferFrames;

};

#endif
