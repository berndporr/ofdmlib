/**
* @file trx.cpp
* @author Kamil Rog
* 
*/

#include "trx.h"
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>

// Debug callbacks //

int PlaybackRaw( void *outputBuffer, void * /*inputBuffer*/, unsigned int nBufferFrames,
            double /*streamTime*/, RtAudioStreamStatus /*status*/, void *data )
{
  OutputData *oData = (OutputData*) data;

  // Not good idea to read from file in the callback
  unsigned int count = fread( outputBuffer, oData->channels * sizeof( MY_TYPE ), nBufferFrames, oData->fd);
  if ( count < nBufferFrames )
  {
    unsigned int bytes = (nBufferFrames - count) * oData->channels * sizeof( MY_TYPE );
    unsigned int startByte = count * oData->channels * sizeof( MY_TYPE );
    memset( (char *)(outputBuffer)+startByte, 0, bytes );
    return 1;
  }

  return 0;
}


// Recording Callbacks //

int Record( void * /*outputBuffer*/, void *inputBuffer, unsigned int nBufferFrames,
           double /*streamTime*/, RtAudioStreamStatus status, void *data )
{
  if ( status )
  {
      std::cout << "Error: RxCallback: Stream over/underflow detected." << std::endl;
  }

  // Recover callback data struct
  RecordData *recordCallbackData = (RecordData *) data;
  // Copy Rx Data 
  memcpy( &recordCallbackData->rxCopy[recordCallbackData->counter*nBufferFrames], inputBuffer, nBufferFrames*sizeof(double) );
  recordCallbackData->counter++;
  recordCallbackData->nRxFrames += nBufferFrames; 
  // Check if enough frames received 
  if (recordCallbackData->frameLimit <= recordCallbackData->nRxFrames)
  {
    std::cout << "Recording Finished\n" << std::endl;
    return 1;
  }
    
  return 0;
}


int Playback( void *outputBuffer, void * /*inputBuffer*/, unsigned int nBufferFrames,
            double /*streamTime*/, RtAudioStreamStatus /*status*/, void *data )
{
  // Recover callback data struct
  PlaybackData *playbackData = (PlaybackData *) data;
  // Copy Rx Data 
  memcpy(outputBuffer, &playbackData->rxCopy[playbackData->counter*nBufferFrames], nBufferFrames*sizeof(double) );
  playbackData->counter++;
  playbackData->nTxFrames += nBufferFrames; 
  // Check if enough frames received 
  if (playbackData->frameLimit <= playbackData->nTxFrames)
  {
    std::cout << "Playback Finished\n" << std::endl;
    return 1;
  }
    
  return 0;
}

// Real-Time operation mode callbacks //

int TxCallback( void *outputBuffer, void* /*inputBuffer*/, unsigned int /*nBufferFrames*/,
                double /*streamTime*/, RtAudioStreamStatus status, void *data )
{
    if ( status )
    {
      std::cout << "Error: TxCallback: Stream over/underflow detected." << std::endl;
    }

    // Recover callback data struct
    TxCallbackData *txCallbackData = (TxCallbackData *) data;
    
    // Check if there is data to transmit
    if(txCallbackData->nBytes > 0)
    {
        // Enough bytes to tx to fill ofdm symbol with nMaxBytesPerSymbol
        if(txCallbackData->nBytes >= txCallbackData->nBytesPerSymbol)
        {
          txCallbackData->pCodec->ProcessTxBuffer(&txCallbackData->txBuffer[txCallbackData->nTxByteCounter],
                                  (double *) outputBuffer);
          txCallbackData->nBytes -= txCallbackData->nBytesPerSymbol;
          txCallbackData->nTxByteCounter += txCallbackData->nBytesPerSymbol;
        }
    }
    // No more data to Tx 
    else
    {
      // Stop Stream
      return 1;
    }
    // More data to tx
    return 0;
}


int RxCallback( void * /*outputBuffer*/, void *inputBuffer, unsigned int nBufferFrames,
           double /*streamTime*/, RtAudioStreamStatus status, void *data )
{
  if ( status )
  {
      std::cout << "Error: RxCallback: Stream over/underflow detected." << std::endl;
  }

  // Recover callback data struct
  RxCallbackData *rxCallbackData = (RxCallbackData *) data;
  // Process data block 
  rxCallbackData->nRxBytes += rxCallbackData->pCodec->ProcessRxBuffer( (double *) inputBuffer,
                                                      &rxCallbackData->rxBuffer[rxCallbackData->nRxBytes],
                                                      rxCallbackData->nBytesPerSymbol);
  return 0;
}


// Trx //


/**
* Constructor
* 
* @param audioSettings 
*
*/
AudioTrx::AudioTrx(rtAudioSettings audioSettings, OFDMSettingsStruct encoderSettings, OFDMSettingsStruct decoderSettings, TRX_OPERATION_MODE operationMode) :
    m_encoder(encoderSettings),
    m_decoder(decoderSettings),
    m_rtAudioSettings(audioSettings)
{

    if ( dac.getDeviceCount() < 1 )
    {
        std::cout << "\nError: dac, No audio devices found!\n";
        exit( 0 );
    }
    
    if ( adc.getDeviceCount() < 1 )
    {
        std::cout << "\nError: adc, No audio devices found!\n";
        exit( 0 );
    }

    // Let RtAudio print messages to stderr.
    dac.showWarnings( true );
    adc.showWarnings( true );

    // Set the same number of channels for both input and output.
    m_InputParams.deviceId = m_rtAudioSettings.InputDevice;
    m_InputParams.nChannels = m_rtAudioSettings.nChannels;
    m_InputParams.firstChannel = m_rtAudioSettings.InputOffset;
    m_OutputParams.deviceId = m_rtAudioSettings.OutputDevice;
    m_OutputParams.nChannels = m_rtAudioSettings.nChannels;
    m_OutputParams.firstChannel = m_rtAudioSettings.OutputOffset;

    // Grab default device if none was specified 
    if ( m_rtAudioSettings.InputDevice == 0 )
    {
        m_InputParams.deviceId = adc.getDefaultInputDevice();
    }
    
    if ( m_rtAudioSettings.OutputDevice == 0 )
    {
        m_OutputParams.deviceId = dac.getDefaultOutputDevice();
    }
        
    m_RxBufferFrames = m_rtAudioSettings.BufferFrames;
    m_TxBufferFrames = m_rtAudioSettings.BufferFrames;

    // Set default settings for struct 
    size_t nAvaiablePoints = (encoderSettings.nFFTPoints - ((size_t)(encoderSettings.nFFTPoints / encoderSettings.PilotToneDistance)));
    size_t nMaxBytesPerSymbol = (nAvaiablePoints*encoderSettings.QAMSize) / 8;
    //restrict spectrum
    //nMaxBytesPerSymbol /= 2;
    nMaxBytesPerSymbol = 100;

    size_t nSymbols = 5;

    size_t nTotalBuffers = 10;
    rxCopy = (double*) calloc((m_rtAudioSettings.BufferFrames * nTotalBuffers), sizeof(double));
    m_recordData.rxCopy = rxCopy;
    m_recordData.frameLimit = m_rtAudioSettings.BufferFrames * nTotalBuffers; 
    m_recordData.counter = 0;
    m_recordData.nRxFrames = 0;
    m_recordData.nChannels = audioSettings.nChannels;
    
    // Set up the Rx callback data struct  
    m_RxCallbackData.pCodec = &m_decoder;
    m_RxCallbackData.nBytesPerSymbol = nMaxBytesPerSymbol;
    m_RxCallbackData.rxBuffer = rxOut;
    m_RxCallbackData.nRxBytes = 0;
    m_RxCallbackData.cbCounter = 0;

    // Setup the Tx callback data struct
    m_TxCallbackData.pCodec = &m_encoder;
    m_TxCallbackData.txBuffer = txIn;
    m_TxCallbackData.nBytesPerSymbol = nMaxBytesPerSymbol;
    m_TxCallbackData.nBytes = nMaxBytesPerSymbol*nSymbols;
    m_TxCallbackData.nTxByteCounter = 0;

    // Set stream options
    // Use default ALSA device
    m_StreamOptions.flags |= RTAUDIO_ALSA_USE_DEFAULT;
    // Set real-time round robin schedluing 
    m_StreamOptions.flags |= RTAUDIO_SCHEDULE_REALTIME;
    // Minimize latency
    m_StreamOptions.flags |= RTAUDIO_MINIMIZE_LATENCY;
    // Attempt exclusive use of the device
    m_StreamOptions.flags |= RTAUDIO_HOG_DEVICE;
    // Set the number of buffers for ALSA to use
    //m_StreamOptions.numberOfBuffers = 4;

    // Open Tx & Rx streams
    OpenStreams(operationMode);
}


/**
* Destructor 
* Calls Stop Stream
* The streams should already be closed 
* if transcieiver is used correctly.
*
*/
AudioTrx::~AudioTrx()
{
  //StopTxStream();
  //StopRxStream();
}

/**
* Opens Tx & Rx audio streams 
*
*@param mode 
*
*/
void AudioTrx::OpenStreams(TRX_OPERATION_MODE mode)
{



  switch(mode)
  { 
    
    case REAL_TIME:
        std::cout << "\nREAL-TIME TRX OPERATION!" << std::endl;
        // Try opening the rx stream
        try
        {
            adc.openStream( NULL, &m_InputParams, RTAUDIO_FLOAT64, m_rtAudioSettings.SampleRate, &m_RxBufferFrames, RxCallback, reinterpret_cast<void *>(&m_RxCallbackData), &m_StreamOptions );
        }
        catch ( RtAudioError& e )
        {
            e.printMessage();
            exit( 0 );
        }
        

        // Try opening the tx stream
        try
        {
            dac.openStream( &m_OutputParams, NULL, RTAUDIO_FLOAT64, m_rtAudioSettings.SampleRate, &m_TxBufferFrames, TxCallback, reinterpret_cast<void *>(&m_TxCallbackData), &m_StreamOptions );
        }
        catch ( RtAudioError& e )
        {
            e.printMessage();
            exit( 0 );
        }
    break;

    case RECORDING:
        std::cout << "RECORDING TRX OPERATION!" << std::endl;
        // Try opening the rx stream
        try
        {
            adc.openStream( NULL, &m_InputParams, RTAUDIO_FLOAT64, m_rtAudioSettings.SampleRate, &m_RxBufferFrames, Record, reinterpret_cast<void *>(&m_recordData), &m_StreamOptions );
        }
        catch ( RtAudioError& e )
        {
            e.printMessage();
            exit( 0 );
        }

        // Try opening the tx stream
        try
        {
            dac.openStream( &m_OutputParams, NULL, RTAUDIO_FLOAT64, m_rtAudioSettings.SampleRate, &m_TxBufferFrames, Playback, reinterpret_cast<void *>(&m_playbackData), &m_StreamOptions );
        }
        catch ( RtAudioError& e )
        {
            e.printMessage();
            exit( 0 );
        }
    break;

    default:
      std::cout << "Error: OpenStreams: Unidentified Operation Mode!" << std::endl;
      exit( 0 );
  }



}


/**
* Starts the Transmiter (Playback) Stream
*
*/
void AudioTrx::StartTxStream( )
{
    try
    {
      dac.startStream();
    }
    catch ( RtAudioError& e )
    {
      e.printMessage();
      exit( 0 );
    }

}


/**
* Stop the Transmiter (Playback) Stream
*
*/
void AudioTrx::StopTxStream()
{
    try
    {
      dac.stopStream();
    }
    catch (RtAudioError& e)
    {
      e.printMessage();
      exit( 0 );
    }
}


/**
* Starts the Reciever (Recording) Stream
*
*/
void AudioTrx::StartRxStream()
{
    try
    {
      adc.startStream();
    }
    catch ( RtAudioError& e )
    {
      e.printMessage();
      exit( 0 );
    }
}


/**
* Stop the Receiver (Recording) Stream
*
*/
void AudioTrx::StopRxStream()
{
    try
    {
      adc.stopStream();
    }
    catch (RtAudioError& e)
    {
      e.printMessage();
      exit( 0 );
    }
}
