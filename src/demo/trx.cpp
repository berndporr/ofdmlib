/**
* @file trx.cpp
* @author Kamil Rog
* 
*/

#include "trx.h"
#include <unistd.h>

// Callbacks //


int TxCallback( void *outputBuffer, void* /*inputBuffer*/, unsigned int /*nBufferFrames*/,
                double /*streamTime*/, RtAudioStreamStatus status, void *data )
{
    if ( status )
    {
      std::cout << "Error: TxCallback: Stream over/underflow detected." << std::endl;
    }

    // Recover callback data struct
    //CallbackData *callbackData = (CallbackData *) data;
  
    /*
      if(iData->nTxSymbolsCounter)
      {
        //Encode 
      }
      // No more symbols to Tx 
      else
      {
        return 1; // TODO: stop stream?
      }
    */

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


int RxCallback( void * /*outputBuffer*/, void *inputBuffer, unsigned int nBufferFrames,
           double /*streamTime*/, RtAudioStreamStatus status, void *data )
{
  if ( status )
  {
      std::cout << "Error: TxCallback: Stream over/underflow detected." << std::endl;
  }

  // Recover callback data struct
  //CallbackData *callbackData = (CallbackData *) data;
  
  // Find symbol start
  // If symbol start found
  // Decode
 
  return 0;
}




// Trx //


/**
* Constructor
* 
* @param audioSettings 
*
*/
AudioTrx::AudioTrx(rtAudioSettings audioSettings, OFDMSettings codecSettings) :
    m_ofdmCodec(codecSettings),
    m_rtAudioSettings(audioSettings)
 
{
      m_TxCallbackData.pCodec =  &m_ofdmCodec;
      m_RxCallbackData.pCodec =  &m_ofdmCodec;

      rawPlaybackData.channels = m_rtAudioSettings.nChannels;
      rawPlaybackData.fd = fopen( "test.raw", "rb" );
      if ( !rawPlaybackData.fd )
      {
          std::cout << "Unable to find or open file!\n";
          exit( 1 );
      }


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

      //RtAudio::StreamOptions options;

      // Try opening the rx stream
      try
      {
          adc.openStream( NULL, &m_InputParams, RTAUDIO_FLOAT64, m_rtAudioSettings.SampleRate, &m_RxBufferFrames, RxCallback, reinterpret_cast<void *>(&m_RxCallbackData) );
      }
      catch ( RtAudioError& e )
      {
          e.printMessage();
          exit( 0 );
      }

      // Try opening the tx stream
      try
      {
          dac.openStream( &m_OutputParams, NULL, RTAUDIO_FLOAT64, m_rtAudioSettings.SampleRate, &m_TxBufferFrames, PlaybackRaw, reinterpret_cast<void *>(&rawPlaybackData) );
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

