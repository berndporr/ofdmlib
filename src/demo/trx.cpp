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



// Callbacks //

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
  //size_t copiedFrames = ;
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



// TODO: delete this in next commit
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
        if(txCallbackData->nBytes >= txCallbackData->nMaxBytesPerSymbol)
        {
          txCallbackData->pCodec->ProcessTxBuffer(&txCallbackData->txBuffer[txCallbackData->nTxByteCounter],
                                  (double *) outputBuffer, 
                                  txCallbackData->nMaxBytesPerSymbol);
          txCallbackData->nBytes -= txCallbackData->nMaxBytesPerSymbol;
          txCallbackData->nTxByteCounter += txCallbackData->nMaxBytesPerSymbol;
        }
        // Last symbol to tx
        else
        {
          txCallbackData->pCodec->ProcessTxBuffer(
                                  &txCallbackData->txBuffer[txCallbackData->nTxByteCounter], 
                                  (double *) outputBuffer, txCallbackData->nBytes);
          txCallbackData->nBytes = 0;
          txCallbackData->nTxByteCounter += txCallbackData->nBytes;
          return 2;
        }
    }
    // No more data to Tx 
    else
    {
      // Stop Stream
      return 2;
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
  // Process Data 
  rxCallbackData->nRxBytes += rxCallbackData->pCodec->ProcessRxBuffer( (double *) inputBuffer, &rxCallbackData->rxBuffer[rxCallbackData->nRxBytes]);
  return 0;
}


// Trx //


/**
* Constructor
* 
* @param audioSettings 
*
*/
AudioTrx::AudioTrx(rtAudioSettings audioSettings, OFDMSettings encoderSettings, OFDMSettings decoderSettings) :
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

      // Handle additional options
      //RtAudio::StreamOptions options; 
      
      // Test Case 1 - Records & plays nTotalBuffers  (pass Record & playback when opening stream)
      // Record & Playback data structs
      /*
      size_t nTotalBuffers = 20;
      rxCopy = (double*) calloc((m_rtAudioSettings.BufferFrames * nTotalBuffers * audioSettings.nChannels), sizeof(double));
      m_recordData.rxCopy = rxCopy;
      m_recordData.frameLimit = m_rtAudioSettings.BufferFrames * nTotalBuffers * audioSettings.nChannels; 
      m_recordData.counter = 0;
      m_recordData.nRxFrames = 0;
      m_recordData.nChannels = audioSettings.nChannels;

      m_playbackData.rxCopy = rxCopy;
      m_playbackData.frameLimit = m_recordData.frameLimit;
      m_playbackData.counter = 0;
      m_playbackData.nTxFrames = 0;
      m_playbackData.nChannels = audioSettings.nChannels;
      */


      // Test Case 2 Tx ofdm encoded symbols, Record everything & save then decode(pass Record & TxCallback)
      // Setup the Tx callback data struct

      // Calculate max nBytes 
      size_t nAvaiablePoints = (encoderSettings.nPoints - ((size_t)(encoderSettings.nPoints / encoderSettings.pilotToneStep)));
      size_t nMaxBytesPerSymbol = (nAvaiablePoints*encoderSettings.QAMSize) / 8;
      size_t nSymbols = 1;

      m_TxCallbackData.pCodec = &m_encoder;
      m_TxCallbackData.txBuffer = txIn;
      m_TxCallbackData.nMaxBytesPerSymbol = nMaxBytesPerSymbol;
      m_TxCallbackData.nBytes = nMaxBytesPerSymbol*nSymbols;
      m_TxCallbackData.nTxByteCounter = 0;
  

      size_t nTotalBuffers = 10;
      rxCopy = (double*) calloc((m_rtAudioSettings.BufferFrames * nTotalBuffers), sizeof(double));
      m_recordData.rxCopy = rxCopy;
      m_recordData.frameLimit = m_rtAudioSettings.BufferFrames * nTotalBuffers; 
      m_recordData.counter = 0;
      m_recordData.nRxFrames = 0;
      m_recordData.nChannels = audioSettings.nChannels;


      // Normal Operation Setup 
      m_RxCallbackData.cbCounter = 0;
      //txIn = (uint8_t*) calloc(nMaxBytesPerSymbol*nSymbols, sizeof(uint8_t));
      //rxOut = (uint8_t*) calloc(nMaxBytesPerSymbol*nSymbols, sizeof(uint8_t));


      // Set up the Rx callback data struct  
      m_RxCallbackData.pCodec =  &m_decoder;
      m_RxCallbackData.nMaxBytesPerSymbol = nMaxBytesPerSymbol;
      m_RxCallbackData.rxBuffer = rxOut;
      m_RxCallbackData.nRxBytes = 0;


      // Setup the Tx callback data struct
      m_TxCallbackData.pCodec = &m_encoder;
      m_TxCallbackData.txBuffer = txIn;
      m_TxCallbackData.nMaxBytesPerSymbol = nMaxBytesPerSymbol;
      m_TxCallbackData.nBytes = nMaxBytesPerSymbol*nSymbols;
      m_TxCallbackData.nTxByteCounter = 0;


      // Try opening the rx stream
      try
      {
          adc.openStream( NULL, &m_InputParams, RTAUDIO_FLOAT64, m_rtAudioSettings.SampleRate, &m_RxBufferFrames, Record, reinterpret_cast<void *>(&m_recordData) );
      }
      catch ( RtAudioError& e )
      {
          e.printMessage();
          exit( 0 );
      }
      

      // Try opening the tx stream
      try
      {
          dac.openStream( &m_OutputParams, NULL, RTAUDIO_FLOAT64, m_rtAudioSettings.SampleRate, &m_TxBufferFrames, Playback, reinterpret_cast<void *>(&m_playbackData) );
      }
      catch ( RtAudioError& e )
      {
          e.printMessage();
          exit( 0 );
      }
}


/**
* Destructor 
*
*/
AudioTrx::~AudioTrx()
{

}


/**
*
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
*
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
*
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
*
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
