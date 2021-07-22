#include "trx.h"
#include <iostream>
#include <cstdlib>
#include <cstring>


typedef double MY_TYPE;
#define FORMAT RTAUDIO_FLOAT64


int main( int argc, char *argv[] )
{
  // OFDM Codec Settings
  OFDMSettings settingsStruct;
  settingsStruct.type = FFTW_BACKWARD;
  settingsStruct.EnergyDispersalSeed = 0;
  settingsStruct.nPoints = 1024; 
  settingsStruct.pilotToneStep = 16; 
  settingsStruct.pilotToneAmplitude = 2.0; 
  settingsStruct.guardInterval = 0; 
  settingsStruct.QAMSize = 2; 
  settingsStruct.cyclicPrefixSize = (int) ((settingsStruct.nPoints*2)/4); 

  // rt audioSettings
  rtAudioSettings audioSettings;
  audioSettings.SampleRate = 44100;
  audioSettings.BufferFrames = 2056;
  audioSettings.nChannels = 2;
  audioSettings.Device = 0;
  audioSettings.offset = 0;
  audioSettings.InputDevice = 0;
  audioSettings.OutputDevice = 0;
  audioSettings.InputOffset = 0;
  audioSettings.OutputOffset = 0;

  // Initialize transceiver
  AudioTrx trx(audioSettings, settingsStruct);

  // Start Recording
  //trx.StartRxStream();

  // Start Transmitting 
  trx.StartTxStream();

  // Sleep

  std::cout << "\nPlaying raw file" << std::endl;
  while ( 1 ) 
  {

  }

  // Stop Transmitting
  trx.StopTxStream();

  // Stop Recording
  //trx.StopRxStream();

  // Start transmitting what has been recorded


  return 0;
}
