#include "stdio.h"
#include "string.h"
#include "assert.h"
#include "acSdk.h"

//#define CODE_GEN_SDKVERSION       "v1.6.3"                   // The code has been generated with this Firmware version
//#define CODE_GEN_BOARD            "STM32H573I-DK"            // The code has been generated with this board
//#define CODE_GEN_AUDIOCONFIG      "ID_02480248med080016FF"   // The code has been generated with this audio configuration

extern int32_t gAudio_Config;

static acPipe       hPipe;                        // Pipe Handle
static acChunk      hSysIn_BLE_LC3_cnx;               // hSysIn_BLE_LC3_cnx handle
static acAlgo       hFIR_equalizer_1;             // FIR-equalizer-1 handle
static acAlgo       hIIR_equalizer_1;             // IIR-equalizer-1 handle
static acChunk      hSysOut_dac_cnx;                  // hSysOut_dac_cnx handle


/* BLE_SPEAKER Board shows more complex functionnalities, while STM32WBA65I_DK1 demo uses only FIR and IIR
*/
#ifdef USE_BLE_SPEAKER
static acChunk      hSysIn_WavFile_cnx;         // hSysIn_WavFile_cnx handle
static acAlgo       hStereo2mono_1;               // stereo2mono-1 handle
static acChunk      hChunk_cnx_5;                 // hChunk_cnx_5 handle
static acAlgo       hResample_1;                  // resample-1 handle
static acChunk      hChunk_cnx_1;                 // hChunk_cnx_1 handle
static acChunk      hChunk_cnx_6;                 // hChunk_cnx_6 handle
static acAlgo       hMix_1;                       // mix-1 handle
static acChunk      hChunk_cnx_2;                 // hChunk_cnx_2 handle
static acAlgo       hMono2stereo_1;               // mono2stereo-1 handle
#endif /* USE_BLE_SPEAKER */

#if (!defined(USE_BLE_SPEAKER) && !defined(USE_STM32WBA65I_DK1))
#error "this file is prepared for either BLE_SPEAKER or STM32WBA65I-DK1"
#endif

// This function is called at the very beginning of the system initialization.
// It gives the opportunity to replace the default audio config by the configuration used during the code generation.
void AudioChainInstance_initSystem(void)
{
  if (gAudio_Config > -1)
  {
    audio_persist_set_config_index(gAudio_Config);
  }
  else
  {
    assert(0);
  }
}


// Implement the control callback for the algo FIR-equalizer-1
static int32_t cb_control_hFIR_equalizer_1(acAlgo hAlgo)
{
  int32_t error = 0;
  void   *pUserData = NULL;

  error = acAlgoGetCommonConfig(hAlgo, "userData", &pUserData);
  assert(error == 0);

  // number points of FIR response curve
  uint32_t nbBands;
  error = acAlgoGetControl(hAlgo, "nbBands", &nbBands);
  assert(error == 0);

  // sampling rate (Hz)
  float samplingRate;
  error = acAlgoGetControl(hAlgo, "samplingRate", &samplingRate);
  assert(error == 0);

  // array with the magnitude in dB of each band, nbBands points
  void * pResponse_dB;
  error = acAlgoGetControl(hAlgo, "pResponse_dB", &pResponse_dB);
  assert(error == 0);

  // min magnitude in dB of filter response
  float modMin_dB;
  error = acAlgoGetControl(hAlgo, "modMin_dB", &modMin_dB);
  assert(error == 0);

  // max magnitude in dB of filter response
  float modMax_dB;
  error = acAlgoGetControl(hAlgo, "modMax_dB", &modMax_dB);
  assert(error == 0);

  // your code here...
  // see acAlgoGetControlPtr for optimization

  return 0;
}

// Implement the control callback for the algo IIR-equalizer-1
static int32_t cb_control_hIIR_equalizer_1(acAlgo hAlgo)
{
  int32_t error = 0;
  void   *pUserData = NULL;

  error = acAlgoGetCommonConfig(hAlgo, "userData", &pUserData);
  assert(error == 0);

  // number points of IIR response curve
  uint32_t nbBands;
  error = acAlgoGetControl(hAlgo, "nbBands", &nbBands);
  assert(error == 0);

  // sampling rate (Hz)
  float samplingRate;
  error = acAlgoGetControl(hAlgo, "samplingRate", &samplingRate);
  assert(error == 0);

  // array with the magnitude in dB of each band, nbBands points
  void * pResponse_dB;
  error = acAlgoGetControl(hAlgo, "pResponse_dB", &pResponse_dB);
  assert(error == 0);

  // min magnitude in dB of filter response
  float modMin_dB;
  error = acAlgoGetControl(hAlgo, "modMin_dB", &modMin_dB);
  assert(error == 0);

  // max magnitude in dB of filter response
  float modMax_dB;
  error = acAlgoGetControl(hAlgo, "modMax_dB", &modMax_dB);
  assert(error == 0);

  // your code here...
  // see acAlgoGetControlPtr for optimization

  return 0;
}

void AudioChainInstance_initGraph(void)
{
  int32_t error = 0;

  // Initialize acSDK
  error = acInitialize();
  assert(error == 0);

  // Enable the background cycle count
  error = acEnvSetConfig("bLogCycles", AC_FALSE);
  assert(error == 0);
  // Enable the default cycleCount handler, the default handler prints cycleCount in Mhz in the console
  error = acEnvSetConfig("bDefaultCyclesMgntCb", AC_TRUE);
  assert(error == 0);
  // Set the cycleCount call-back delay to 5 secs
  error = acEnvSetConfig("cyclesMgntCbTimeout", 5000UL);
  assert(error == 0);
  // Set the cycleCount measure delay to 500 msecs
  error = acEnvSetConfig("cyclesMgntMeasureTimeout", 500UL);
  assert(error == 0);

  // Set the chunks' memory pool
  error = acEnvSetConfig("chunkMemPool", 0UL);
  assert(error == 0);
  // Set the algos' handle & config memory pool
  error = acEnvSetConfig("algoMemPool", 0UL);
  assert(error == 0);

  // Create the pipe
  error = acPipeCreate(&hPipe);
  assert(error == 0);

  // Create the chunk hSysIn_BLE_LC3_cnx
  error = acChunkCreate(hPipe, AC_SYSIN_BLE_LC3_NAME, &hSysIn_BLE_LC3_cnx);
  assert(error == 0);
  // Create the chunk hSysOut_dac_cnx
  error = acChunkCreate(hPipe, AC_SYSOUT_CODEC_DAC_NAME, &hSysOut_dac_cnx);
  assert(error == 0);

#ifdef USE_BLE_SPEAKER
  // Create the chunk hSysIn_WavFile_cnx
  error = acChunkCreate(hPipe, AC_SYSIN_WAVFILE_NAME, &hSysIn_WavFile_cnx);
  assert(error == 0);
  // Create the chunk hChunk_cnx_5
  error = acChunkCreate(hPipe, "hChunk_cnx_5", &hChunk_cnx_5);
  assert(error == 0);
  // Chunk type: user or system; source (input), sink (output) or both; i.e. AUDIO_CHUNK_TYPE_SYS_IN, AUDIO_CHUNK_TYPE_SYS_OUT, AUDIO_CHUNK_TYPE_USER_SRC, AUDIO_CHUNK_TYPE_USER_SINK or AUDIO_CHUNK_TYPE
  error = acChunkSetConfig(hChunk_cnx_5, "chunkType", "AUDIO_CHUNK_TYPE_USER_INOUT");
  assert(error == 0);
  // Number of channels
  error = acChunkSetConfig(hChunk_cnx_5, "nbChannels", "1");
  assert(error == 0);

  // Sampling frequency (unit Hz)
  if (gAudio_Config == audio_persist_get_config_index_from_name("media48kHz"))
  {
    error = acChunkSetConfig(hChunk_cnx_5, "fs", "48000");
    assert(error == 0);
    // Number of elements per frame, an element is a sample in time, a band/bin in frequency domain
    error = acChunkSetConfig(hChunk_cnx_5, "nbElements", "384");
    assert(error == 0);
  }
  else
  {
    error = acChunkSetConfig(hChunk_cnx_5, "fs", "24000");
    assert(error == 0);
    // Number of elements per frame, an element is a sample in time, a band/bin in frequency domain
    error = acChunkSetConfig(hChunk_cnx_5, "nbElements", "192");
    assert(error == 0);
  }

  // Number of audio frames
  error = acChunkSetConfig(hChunk_cnx_5, "nbFrames", "2");
  assert(error == 0);
  // Time/frequency sample type: ABUFF_FORMAT_TIME or ABUFF_FORMAT_FREQ
  error = acChunkSetConfig(hChunk_cnx_5, "timeFreq", "ABUFF_FORMAT_TIME");
  assert(error == 0);
  // Sample internal coding type: ABUFF_FORMAT_PDM_LSB_FIRST, ABUFF_FORMAT_PDM_MSB_FIRST, ABUFF_FORMAT_G711_ALAW, ABUFF_FORMAT_G711_MULAW, ABUFF_FORMAT_FIXED16, ABUFF_FORMAT_FIXED32 or ABUFF_FORMAT_FLO
  error = acChunkSetConfig(hChunk_cnx_5, "bufferType", "ABUFF_FORMAT_FIXED16");
  assert(error == 0);
  // Channels interleaving type: ABUFF_FORMAT_NON_INTERLEAVED or ABUFF_FORMAT_INTERLEAVED
  error = acChunkSetConfig(hChunk_cnx_5, "interleaved", "ABUFF_FORMAT_INTERLEAVED");
  assert(error == 0);


  // downmix stereo to mono
  // Create the instance stereo2mono-1
  error = acAlgoCreate(hPipe, "stereo2mono", "stereo2mono-1", &hStereo2mono_1, 0, "stereo2mono-1");
  assert(error == 0);
  // Create the chunk hChunk_cnx_1
  error = acChunkCreate(hPipe, "hChunk_cnx_1", &hChunk_cnx_1);
  assert(error == 0);
  // Chunk type: user or system; source (input), sink (output) or both; i.e. AUDIO_CHUNK_TYPE_SYS_IN, AUDIO_CHUNK_TYPE_SYS_OUT, AUDIO_CHUNK_TYPE_USER_SRC, AUDIO_CHUNK_TYPE_USER_SINK or AUDIO_CHUNK_TYPE
  error = acChunkSetConfig(hChunk_cnx_1, "chunkType", "AUDIO_CHUNK_TYPE_USER_INOUT");
  assert(error == 0);
  // Number of channels
  error = acChunkSetConfig(hChunk_cnx_1, "nbChannels", "1");
  assert(error == 0);
  // Sampling frequency (unit Hz)
  if (gAudio_Config == audio_persist_get_config_index_from_name("media48kHz"))
  {
    error = acChunkSetConfig(hChunk_cnx_1, "fs", "48000");
    assert(error == 0);
    // Number of elements per frame, an element is a sample in time, a band/bin in frequency domain
    error = acChunkSetConfig(hChunk_cnx_1, "nbElements", "384");
    assert(error == 0);
  }
  else
  {
    error = acChunkSetConfig(hChunk_cnx_1, "fs", "24000");
    assert(error == 0);
    // Number of elements per frame, an element is a sample in time, a band/bin in frequency domain
    error = acChunkSetConfig(hChunk_cnx_1, "nbElements", "192");
    assert(error == 0);
  }
  // Number of audio frames
  error = acChunkSetConfig(hChunk_cnx_1, "nbFrames", "2");
  assert(error == 0);
  // Time/frequency sample type: ABUFF_FORMAT_TIME or ABUFF_FORMAT_FREQ
  error = acChunkSetConfig(hChunk_cnx_1, "timeFreq", "ABUFF_FORMAT_TIME");
  assert(error == 0);
  // Sample internal coding type: ABUFF_FORMAT_PDM_LSB_FIRST, ABUFF_FORMAT_PDM_MSB_FIRST, ABUFF_FORMAT_G711_ALAW, ABUFF_FORMAT_G711_MULAW, ABUFF_FORMAT_FIXED16, ABUFF_FORMAT_FIXED32 or ABUFF_FORMAT_FLO
  error = acChunkSetConfig(hChunk_cnx_1, "bufferType", "ABUFF_FORMAT_FIXED16");
  assert(error == 0);
  // Channels interleaving type: ABUFF_FORMAT_NON_INTERLEAVED or ABUFF_FORMAT_INTERLEAVED
  error = acChunkSetConfig(hChunk_cnx_1, "interleaved", "ABUFF_FORMAT_INTERLEAVED");
  assert(error == 0);


  // sampling frequency adaptation, available resampling ratios r=n1/n2 with n1 and n2 in {1, 2, 3, 4, 6, 8}
  // Create the instance resample-1
  error = acAlgoCreate(hPipe, "resample", "resample-1", &hResample_1, 0, "resample-1");
  assert(error == 0);
  // Filter type
  error = acAlgoSetConfig(hResample_1, "filterType", "RESAMPLE_TYPE_NO_FILTERING");
  assert(error == 0);
  // Create the chunk hChunk_cnx_6
  error = acChunkCreate(hPipe, "hChunk_cnx_6", &hChunk_cnx_6);
  assert(error == 0);
  // Chunk type: user or system; source (input), sink (output) or both; i.e. AUDIO_CHUNK_TYPE_SYS_IN, AUDIO_CHUNK_TYPE_SYS_OUT, AUDIO_CHUNK_TYPE_USER_SRC, AUDIO_CHUNK_TYPE_USER_SINK or AUDIO_CHUNK_TYPE
  error = acChunkSetConfig(hChunk_cnx_6, "chunkType", "AUDIO_CHUNK_TYPE_USER_INOUT");
  assert(error == 0);
  // Number of channels
  error = acChunkSetConfig(hChunk_cnx_6, "nbChannels", "1");
  assert(error == 0);

  if (gAudio_Config == audio_persist_get_config_index_from_name("media48kHz"))
  {
    // Sampling frequency (unit Hz)
    error = acChunkSetConfig(hChunk_cnx_6, "fs", "48000");
    assert(error == 0);
    // Number of elements per frame, an element is a sample in time, a band/bin in frequency domain
    error = acChunkSetConfig(hChunk_cnx_6, "nbElements", "384");
    assert(error == 0);
  }
  else
  {
    // Sampling frequency (unit Hz)
    error = acChunkSetConfig(hChunk_cnx_6, "fs", "24000");
    assert(error == 0);
    // Number of elements per frame, an element is a sample in time, a band/bin in frequency domain
    error = acChunkSetConfig(hChunk_cnx_6, "nbElements", "192");
    assert(error == 0);
  }

  // Number of audio frames
  error = acChunkSetConfig(hChunk_cnx_6, "nbFrames", "2");
  assert(error == 0);
  // Time/frequency sample type: ABUFF_FORMAT_TIME or ABUFF_FORMAT_FREQ
  error = acChunkSetConfig(hChunk_cnx_6, "timeFreq", "ABUFF_FORMAT_TIME");
  assert(error == 0);
  // Sample internal coding type: ABUFF_FORMAT_PDM_LSB_FIRST, ABUFF_FORMAT_PDM_MSB_FIRST, ABUFF_FORMAT_G711_ALAW, ABUFF_FORMAT_G711_MULAW, ABUFF_FORMAT_FIXED16, ABUFF_FORMAT_FIXED32 or ABUFF_FORMAT_FLO
  error = acChunkSetConfig(hChunk_cnx_6, "bufferType", "ABUFF_FORMAT_FIXED16");
  assert(error == 0);
  // Channels interleaving type: ABUFF_FORMAT_NON_INTERLEAVED or ABUFF_FORMAT_INTERLEAVED
  error = acChunkSetConfig(hChunk_cnx_6, "interleaved", "ABUFF_FORMAT_INTERLEAVED");
  assert(error == 0);
#endif /* USE_BLE_SPEAKER */

  if (gAudio_Config == audio_persist_get_config_index_from_name("media48kHz"))
  {
    // FIR (Finite Impulse Response) equalizer design
    // Create the instance FIR-equalizer-1
    error = acAlgoCreate(hPipe, "FIR-equalizer", "FIR-equalizer-1", &hFIR_equalizer_1, 0, "FIR-equalizer-1");
    assert(error == 0);
    // number of FIR coefficients (the bigger, the most accurate filter response at the expense of increased CPU load)
    error = acAlgoSetConfig(hFIR_equalizer_1, "firSize", "50");
    assert(error == 0);
    // FIR response curve (frequency, gain in dB); Shift click to create a point, Del to remove a point
    error = acAlgoSetConfig(hFIR_equalizer_1, "firResponseCurve", "{0:[0,0],1:[60,3],2:[300,0],3:[2000,-10],4:[8000,-15],5:[20000,-20]}");
    assert(error == 0);

    // Local instance data you can pass to the control callback.
    error = acAlgoSetCommonConfig(hFIR_equalizer_1, "userData", "MyCustomData");
    assert(error == 0);
    // The control callback is called after the dataflow processing, some algorithms such as rms expose a result you can retrieve using a function callback.
    error = acAlgoSetCommonConfig(hFIR_equalizer_1, "controlCb", (void *)&cb_control_hFIR_equalizer_1);
    assert(error == 0);
  }
  else
  {
    // IIR (Infinite Impulse Response) equalizer design, see: https://webaudio.github.io/Audio-EQ-Cookbook/audio-eq-cookbook.html
    // Create the instance IIR-equalizer-1
    error = acAlgoCreate(hPipe, "IIR-equalizer", "IIR-equalizer-1", &hIIR_equalizer_1, 0, "IIR-equalizer-1");
    assert(error == 0);
    // IIR config (cascade of biquad filters)
    error = acAlgoSetConfig(hIIR_equalizer_1, "iirEqStructArray", "{0:[2,0,0.507,6000,10],1:[3,0,0.5,200,4]}");
    assert(error == 0);

    // Local instance data you can pass to the control callback.
    error = acAlgoSetCommonConfig(hIIR_equalizer_1, "userData", "MyCustomData");
    assert(error == 0);
    // The control callback is called after the dataflow processing, some algorithms such as rms expose a result you can retrieve using a function callback.
    error = acAlgoSetCommonConfig(hIIR_equalizer_1, "controlCb", (void *)&cb_control_hIIR_equalizer_1);
  }

#ifdef USE_BLE_SPEAKER
  // Create the chunk hChunk_cnx_2
  error = acChunkCreate(hPipe, "hChunk_cnx_2", &hChunk_cnx_2);
  assert(error == 0);
  // Chunk type: user or system; source (input), sink (output) or both; i.e. AUDIO_CHUNK_TYPE_SYS_IN, AUDIO_CHUNK_TYPE_SYS_OUT, AUDIO_CHUNK_TYPE_USER_SRC, AUDIO_CHUNK_TYPE_USER_SINK or AUDIO_CHUNK_TYPE
  error = acChunkSetConfig(hChunk_cnx_2, "chunkType", "AUDIO_CHUNK_TYPE_USER_INOUT");
  assert(error == 0);
  // Number of channels
  error = acChunkSetConfig(hChunk_cnx_2, "nbChannels", "1");
  assert(error == 0);

  if (gAudio_Config == audio_persist_get_config_index_from_name("media48kHz"))
  {
    // Sampling frequency (unit Hz)
    error = acChunkSetConfig(hChunk_cnx_2, "fs", "48000");
    assert(error == 0);
    // Number of elements per frame, an element is a sample in time, a band/bin in frequency domain
    error = acChunkSetConfig(hChunk_cnx_2, "nbElements", "384");
    assert(error == 0);
  }
  else
  {
    // Sampling frequency (unit Hz)
    error = acChunkSetConfig(hChunk_cnx_2, "fs", "24000");
    assert(error == 0);
    // Number of elements per frame, an element is a sample in time, a band/bin in frequency domain
    error = acChunkSetConfig(hChunk_cnx_2, "nbElements", "192");
    assert(error == 0);
  }

  // Number of audio frames
  error = acChunkSetConfig(hChunk_cnx_2, "nbFrames", "2");
  assert(error == 0);
  // Time/frequency sample type: ABUFF_FORMAT_TIME or ABUFF_FORMAT_FREQ
  error = acChunkSetConfig(hChunk_cnx_2, "timeFreq", "ABUFF_FORMAT_TIME");
  assert(error == 0);
  // Sample internal coding type: ABUFF_FORMAT_PDM_LSB_FIRST, ABUFF_FORMAT_PDM_MSB_FIRST, ABUFF_FORMAT_G711_ALAW, ABUFF_FORMAT_G711_MULAW, ABUFF_FORMAT_FIXED16, ABUFF_FORMAT_FIXED32 or ABUFF_FORMAT_FLO
  error = acChunkSetConfig(hChunk_cnx_2, "bufferType", "ABUFF_FORMAT_FIXED16");
  assert(error == 0);
  // Channels interleaving type: ABUFF_FORMAT_NON_INTERLEAVED or ABUFF_FORMAT_INTERLEAVED
  error = acChunkSetConfig(hChunk_cnx_2, "interleaved", "ABUFF_FORMAT_INTERLEAVED");
  assert(error == 0);


  // Mix entries
  // Create the instance mix-1
  error = acAlgoCreate(hPipe, "mix", "mix-1", &hMix_1, 0, "mix-1");
  assert(error == 0);
  // gain in dB for entry 1
  error = acAlgoSetConfig(hMix_1, "gain1", "-4");
  assert(error == 0);


  // upmix mono to stereo
  // Create the instance mono2stereo-1
  error = acAlgoCreate(hPipe, "mono2stereo", "mono2stereo-1", &hMono2stereo_1, 0, "mono2stereo-1");
  assert(error == 0);
  // left/right balance
  error = acAlgoSetConfig(hMono2stereo_1, "balance", "-1.0");
  assert(error == 0);

  error = acPipeConnectPinOut(hPipe, hStereo2mono_1, 0, hChunk_cnx_5);
  assert(error == 0);
  error = acPipeConnectPinIn(hPipe, hStereo2mono_1, 0, hSysIn_BLE_LC3_cnx);
  assert(error == 0);
  error = acPipeConnectPinOut(hPipe, hResample_1, 0, hChunk_cnx_1);
  assert(error == 0);
  error = acPipeConnectPinIn(hPipe, hResample_1, 0, hSysIn_WavFile_cnx);
  assert(error == 0);
  error = acPipeConnectPinOut(hPipe, hMix_1, 0, hChunk_cnx_2);
  assert(error == 0);
  error = acPipeConnectPinIn(hPipe, hMix_1, 0, hChunk_cnx_6);
  assert(error == 0);
  error = acPipeConnectPinIn(hPipe, hMix_1, 1, hChunk_cnx_1);
  assert(error == 0);
  error = acPipeConnectPinIn(hPipe, hMono2stereo_1, 0, hChunk_cnx_2);
  assert(error == 0);
  error = acPipeConnectPinOut(hPipe, hMono2stereo_1, 0, hSysOut_dac_cnx);
  assert(error == 0);


  if (gAudio_Config == audio_persist_get_config_index_from_name("media48kHz"))
  {
    error = acPipeConnectPinOut(hPipe, hFIR_equalizer_1, 1, hChunk_cnx_6);
    assert(error == 0);
    error = acPipeConnectPinIn(hPipe, hFIR_equalizer_1, 0, hChunk_cnx_5);
    assert(error == 0);
  }
  else
  {
    error = acPipeConnectPinOut(hPipe, hIIR_equalizer_1, 1, hChunk_cnx_6);
    assert(error == 0);
    error = acPipeConnectPinIn(hPipe, hIIR_equalizer_1, 0, hChunk_cnx_5);
    assert(error == 0);
  }
#else /* USE_BLE_SPEAKER */
  if (gAudio_Config == audio_persist_get_config_index_from_name("media48kHz"))
  {
    error = acPipeConnectPinIn(hPipe, hFIR_equalizer_1, 0, hSysIn_BLE_LC3_cnx);
    assert(error == 0);
    error = acPipeConnectPinOut(hPipe, hFIR_equalizer_1, 0, hSysOut_dac_cnx);
    assert(error == 0);
  }
  else
  {
    error = acPipeConnectPinIn(hPipe, hIIR_equalizer_1, 0, hSysIn_BLE_LC3_cnx);
    assert(error == 0);
    error = acPipeConnectPinOut(hPipe, hIIR_equalizer_1, 0, hSysOut_dac_cnx);
    assert(error == 0);
  }
#endif

  // Start the pipe
  error = acPipePlay(hPipe, AC_START);
  if (error != 0)
  {
    acTrace("Ac play error\n");
  }
  else
  {
    acTrace("Ac playing\n");
  }
}


#ifdef ALGO_USE_LIST
  // This code allows to reduce the flash footprint for the release firmware
  // Only algos mandatory for the audio pipeline will be linked
  #include "audio_chain_factory.h"
#ifdef USE_BLE_SPEAKER
  /* 00 stereo2mono */
  extern audio_algo_factory_t AudioChainWrp_stereo2mono_factory;
  ALGO_FACTORY_DECLARE_FORCE(AudioChainWrp_stereo2mono_factory);
  /* 01 resample */
  extern audio_algo_factory_t AudioChainWrp_resample_factory;
  ALGO_FACTORY_DECLARE_FORCE(AudioChainWrp_resample_factory);
  /* 02 mix */
  extern audio_algo_factory_t AudioChainWrp_mix_factory;
  ALGO_FACTORY_DECLARE_FORCE(AudioChainWrp_mix_factory);
  /* 03 mono2stereo */
  extern audio_algo_factory_t AudioChainWrp_mono2stereo_factory;
  ALGO_FACTORY_DECLARE_FORCE(AudioChainWrp_mono2stereo_factory);
#endif /* USE_BLE_SPEAKER */
  /* 04 FIR-equalizer */
  extern audio_algo_factory_t AudioChainWrp_fir_equalizer_factory;
  ALGO_FACTORY_DECLARE_FORCE(AudioChainWrp_fir_equalizer_factory);
  /* 05 IIR-equalizer */
  extern audio_algo_factory_t AudioChainWrp_iir_equalizer_factory;
  ALGO_FACTORY_DECLARE_FORCE(AudioChainWrp_iir_equalizer_factory);
#endif

void AudioChainInstance_reset_states(void);
void AudioChainInstance_reset_states(void)
{
  int32_t error = 0;

  error = acPipeDelete(hPipe);
  assert(error == 0);

  error = acTerminate();
  assert(error == 0);

  acTrace("Ac reset\n");
}