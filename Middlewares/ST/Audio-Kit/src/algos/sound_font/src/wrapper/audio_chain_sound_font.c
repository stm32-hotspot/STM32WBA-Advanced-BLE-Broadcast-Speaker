/**
******************************************************************************
* @file    audio_chain_sound_font.c
* @author  MCD Application Team
* @brief   decode an SF2 file
*******************************************************************************
* @attention
*
* Copyright (c) 2019(-2022) STMicroelectronics.
* All rights reserved.
*
* This software is licensed under terms that can be found in the LICENSE file
* in the root directory of this software component.
* If no LICENSE file comes with this software, it is provided AS-IS.
*
********************************************************************************
*/

/*
Play sound fonts in format sf2 or sf2
Same filesare avaiables for free at :
https://musical-artifacts.com/artifacts?formats=sf2&tags=florestan
https://archive.org/download/free-soundfonts-sf2-2019-04
https://sites.google.com/site/soundfonts4u/
to play  perfectly the .sf2 must be GM ( general midi) compliant
the file must be packaged using soundFont script in the soundFont folder
*/

/* Includes ------------------------------------------------------------------*/
#include "audio_chain_conf.h"
#ifdef USE_SOUND_FONT
#include "sound_font/audio_chain_sound_font.h"
#include "st_os_mem.h"
#include "common/commonMath.h"
#include "tinyusb.h"


/* Private defines -----------------------------------------------------------*/
#define SF2_SIGNATURE                     0x42324653U
#define SOUND_FONT_FRAME_LATENCY          (3U)
#define  TSF_NO_STDIO                     1
#define  TSF_IMPLEMENTATION               1
#define  SF_ALLOC_ST_OS                   1
#define  SF_SIZE_MIDI_BUFFER              (1024U)
#define  SF_NB_CHANELS                    (2U)

/* Possible math optimization */
#define SOUND_FONT_MATH_OPTIM
#ifdef SOUND_FONT_MATH_OPTIM
  #define TSF_POW(f,y)       pow(f,y)
  #define TSF_POWF(f,y)      powf(f,y)
  #define TSF_LOG10(f)       log10(f)
  #define TSF_LOG(f)         log(f)
  #define TSF_EXPF(f)        expf(f)
  #define TSF_SQRT(f)        sqrt(f)
  #define TSF_SQRTF(f)       sqrt_f32(f)
  #define TSF_TAN(f)         tan(f)
#endif

/* tmp implementation waiting the official one */
#if SF_ALLOC_ST_OS == 1
  static memPool_t tsfMemPool = AUDIO_MEM_RAMEXT;
  #define  TSF_MALLOC(size)       AudioAlgo_malloc(size,tsfMemPool)
  #define  TSF_FREE(p)            if(AudioMallocCheckPtr(p,ST_Mem_Type_ANY_FAST)){AudioAlgo_free(p,tsfMemPool);} /* We need to check the pointer before, because it can belong to the flash ext if an .sff file is used */
  #define  TSF_REALLOC(p,size)    AudioAlgo_realloc(p,size,tsfMemPool)
#else
  #define  TSF_MALLOC(size)       malloc(size)
  #define  TSF_FREE(p)            if(st_os_mem_check_ptr(ST_Mem_Type_ANY_FAST,p)){free(p);} /* We need to check the pointer before, because it can belong to the flash ext if an .sff file is used */
  #define  TSF_REALLOC(p,size)    realloc(p,size)
#endif

/*cstat -MISRAC2012-* sound font not misra compliant */
#include "tsf.h"
/*cstat +MISRAC2012-* */

/* Private typedef -----------------------------------------------------------*/

typedef enum TMLMessageType
{
  MIDI_MISC              = 0U,
  MIDI_CABLE_EVENT       = 1U,
  MIDI_SYSCOM_2BYTE      = 2U, // 2 byte system common message e.g MTC, SongSelect
  MIDI_SYSCOM_3BYTE      = 3U, // 3 byte system common message e.g SPP
  MIDI_SYSEX_START       = 4U, // SysEx starts or continue
  MIDI_SYSEX_END_1BYTE   = 5U, // SysEx ends with 1 data, or 1 byte system common message
  MIDI_SYSEX_END_2BYTE   = 6U, // SysEx ends with 2 data
  MIDI_SYSEX_END_3BYTE   = 7U, // SysEx ends with 3 data
  MIDI_NOTE_OFF          = 8U,
  MIDI_NOTE_ON           = 9U,
  MIDI_POLY_KEYPRESS     = 10U,
  MIDI_CONTROL_CHANGE    = 11U,
  MIDI_PROGRAM_CHANGE    = 12U,
  MIDI_CHANNEL_PRESSURE  = 13U,
  MIDI_PITCH_BEND_CHANGE = 14U,
  MIDI_1BYTE_DATA        = 15U
} midi_code_index_number_t;


typedef struct
{
  uint8_t                  nbChannels;
  uint32_t                 fs;
  const audio_chunk_t     *pChunkOut;
  tsf                     *pTsf;
  void                    *pSf2;
  uint32_t                 szSf2;
  bool                     fileValid;
  uint8_t                  lastGate;
  float                   *pScratchSample;
  uint32_t                 szScratch;
  audio_buffer_t           sf2AudioBuffer;
  sfcContext_t             sfcOutContext;
  audio_algo_t            *pAlgo;
  uint32_t                 nbSamples;
  uint8_t                  lastKey;
  bool                     bInitDone;
  ring_buff_t              hMidiStream;
  ring_buff_t              hSoundFontWav;
  uint32_t                 szSoundFontWav;
  float                   *pScratchSampleProcess;
  memPool_t                memPool;
} sound_fontCtx_t;


typedef struct
{
  uint32_t    signature;
  uint32_t    szSf2;
  uint32_t    offSf2;
} sf2_toc;


/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
//This is a minimal SoundFont with a single loopin saw-wave sample/instrument/preset (484 bytes)

const audio_algo_common_t AudioChainWrp_sound_font_common =
{
  .pName                     = "sound-font",
  .prio_level                = AUDIO_CAPABILITY_PROCESS_PRIO_LEVEL_NORMAL,
  .chunks_consistency.in     = ABUFF_PARAM_NOT_APPLICABLE,
  .chunks_consistency.out    = ABUFF_PARAM_NOT_APPLICABLE,
  .chunks_consistency.in_out = ABUFF_PARAM_ALL,

  .iosIn.nb                  = AUDIO_CAPABILITY_CHUNK_NONE,
  .iosIn.nbChan              = (audio_capability_channel_t)AUDIO_ALGO_FORMAT_UNDEF,
  .iosIn.fs                  = (audio_capability_fs_t)AUDIO_ALGO_FORMAT_UNDEF,
  .iosIn.interleaving        = (audio_capability_interleaving_t)AUDIO_ALGO_FORMAT_UNDEF,
  .iosIn.time_freq           = (audio_capability_time_freq_t)AUDIO_ALGO_FORMAT_UNDEF,
  .iosIn.type                = (audio_capability_type_t)AUDIO_ALGO_FORMAT_UNDEF,

  .iosOut.nb                 = AUDIO_CAPABILITY_CHUNK_ONE,
  .iosOut.nbChan             = AUDIO_CAPABILITY_1CH_2CH,
  .iosOut.fs                 = AUDIO_CAPABILITY_FS_PCM_ALL,
  .iosOut.interleaving       = AUDIO_CAPABILITY_INTERLEAVING_YES,
  .iosOut.time_freq          = AUDIO_CAPABILITY_TIME,
  .iosOut.type               = AUDIO_CAPABILITY_TYPE_FIXED16,
  .misc.pAlgoDesc            = AUDIO_ALGO_OPT_STR("Sound font Midi Player based on TinySoundFont @https://github.com/schellingb/TinySoundFont"),
  .misc.pAlgoHelp            = AUDIO_ALGO_OPT_STR("sound-font"),
  .misc.flags                = AUDIO_ALGO_FLAGS_MISC_SINGLE_INSTANCE
};


/* Private function prototypes -----------------------------------------------*/

static int32_t s_sound_font_deinit(audio_algo_t    *const pAlgo);
static int32_t s_sound_font_init(audio_algo_t      *const pAlgo);
static int32_t s_sound_font_configure(audio_algo_t *const pAlgo);
static int32_t s_sound_font_dataInOut(audio_algo_t *const pAlgo);
static int32_t s_sound_font_process(audio_algo_t   *const pAlgo);
static int32_t s_check_sf2_file(sound_fontCtx_t    *pHandle);


/* Global variables ----------------------------------------------------------*/
audio_algo_cbs_t AudioChainWrp_sound_font_cbs =
{
  .init                       = s_sound_font_init,
  .deinit                     = s_sound_font_deinit,
  .configure                  = NULL,
  .dataInOut                  = s_sound_font_dataInOut,
  .process                    = s_sound_font_process,
  .control                    = NULL,
  .checkConsistency           = NULL,
  .isDisabled                 = NULL,
  .isDisabledCheckConsistency = NULL
};


/* Private Functions Definition ------------------------------------------------------*/


#if CFG_TUD_MIDI_IN

#define MIDI_GET_STATUS(pMidiStream)        (pMidiStream[0] >> 4)
#define MIDI_GET_CHANNEL(pMidiStream)       (pMidiStream[0] & 0x0FU)
#define MIDI_GET_KEY(pMidiStream)           (pMidiStream[1] & 0x7FU)
#define MIDI_GET_VELOCITY(pMidiStream)      (pMidiStream[2] & 0x7FU)
#define MIDI_GET_BEND(pMidiStream)          ((pMidiStream[1] & 0x7FU) | ((pMidiStream[2] & 0x7FU) << 7)) // MSB 7 bits and LSB 7 bits 
#define MIDI_GET_CONTROL(pMidiStream)       (pMidiStream[1])
#define MIDI_GET_CONTROL_VALUE(pMidiStream) (pMidiStream[2])
#define MIDI_GET_PROGRAM(pMidiStream)       (pMidiStream[1]&0x7FU)


/**
* @brief return the event size
*
* @param status event status
*/

static uint32_t s_midi_event_size(uint8_t status)
{
  uint32_t szEvent = 3UL; // assume 3 by default
  switch (status)
  {
    case MIDI_PROGRAM_CHANGE:
      szEvent = 2UL;
      break;

    case MIDI_SYSEX_END_1BYTE:
    case MIDI_1BYTE_DATA:
      szEvent = 1UL;
      break;

    case MIDI_SYSCOM_2BYTE:
    case MIDI_SYSEX_END_2BYTE:
    case MIDI_CHANNEL_PRESSURE:
      szEvent = 2UL;
      break;

    default:
      // 3 by default
      break;
  }

  return szEvent;
}

/**
* @brief Called from USB midi interrupt to pass midi events stream
*
* @param port MIDI port
* @param pPacket stream buffer
* @param szPacket stream buffer  size
* @param pCookie cookie pass at the registration
*/

static void s_midi_player_event(uint32_t port, const void *pPacket, uint32_t szPacket, void *pCookie)
{
  (void)port;

  audio_algo_t *pAlgo = (audio_algo_t *)pCookie;
  sound_fontCtx_t *const pCtx  = (sound_fontCtx_t *)AudioAlgo_getWrapperContext(pAlgo);
  /* The tsf api use alloc/free/realloc, so , this event comes from an IT and allocs are forbidden, we need to defer the task in the process task*/

  if (pCtx->fileValid && (pCtx->pTsf != NULL))
  {
    s_rb_write(&pCtx->hMidiStream, (void *)pPacket, szPacket);
  }
}

#endif



/**
* @brief Parse the stream ring buffer and play midi stream
*
* @param pAlgo algo instance
*/

static void s_midi_play(audio_algo_t *pAlgo)
{
  #if CFG_TUD_MIDI_IN
  sound_fontCtx_t *const pCtx = (sound_fontCtx_t *)AudioAlgo_getWrapperContext(pAlgo);
  uint8_t                midiStream[4];

  while (true)
  {
    uint8_t  status, midiChannel, midiProgram, midiKey, midiVelocity, midiBend, midiControl, midiControlValue;
    uint32_t availableBytes, eventSize;

    if (s_rb_read_fetch(&pCtx->hMidiStream, midiStream, 1UL) != 1UL)
    {
      break;
    }
    availableBytes = s_rb_read_available(&pCtx->hMidiStream);
    status         = MIDI_GET_STATUS(midiStream);
    eventSize      = s_midi_event_size(status);
    if (availableBytes < eventSize)
    {
      break;
    }

    s_rb_read(&pCtx->hMidiStream, midiStream, eventSize);
    switch (status)
    {
      case MIDI_PROGRAM_CHANGE: // apply an preset to a channel
      {
        const char *presetname = (midiChannel != 9U) ? tsf_get_presetname(pCtx->pTsf, tsf_get_presetindex(pCtx->pTsf, 0, (int)midiProgram)) : "Standard Drum Kit";
        midiChannel = MIDI_GET_CHANNEL(midiStream);
        midiProgram = MIDI_GET_PROGRAM(midiStream);
        AudioAlgo_trace(pAlgo, TRACE_LVL_LOG, NULL, 0, "Midi:Programme: Channel %d: %d:%s", midiChannel, midiProgram, (presetname == NULL) ? "" : presetname);
        tsf_channel_set_presetnumber(pCtx->pTsf, (int)midiChannel, (int)midiProgram, (int)(midiChannel == 9U)); //channel program (preset) change (special handling for 10th MIDI channel with drums)
        break;
      }

      case MIDI_NOTE_ON: //play a note
        midiChannel  = MIDI_GET_CHANNEL(midiStream);
        midiKey      = MIDI_GET_KEY(midiStream);
        midiVelocity = MIDI_GET_VELOCITY(midiStream);
        tsf_channel_note_on(pCtx->pTsf, (int)midiChannel, (int)midiKey, (float)midiVelocity / 127.0f);
        break;

      case MIDI_NOTE_OFF: //stop a note
        midiChannel = MIDI_GET_CHANNEL(midiStream);
        midiKey     = MIDI_GET_KEY(midiStream);
        tsf_channel_note_off(pCtx->pTsf, (int)midiChannel, (int)midiKey);
        break;

      case MIDI_PITCH_BEND_CHANGE: //pitch wheel modification
        midiChannel = MIDI_GET_CHANNEL(midiStream);
        midiBend    = MIDI_GET_BEND(midiStream);
        tsf_channel_set_pitchwheel(pCtx->pTsf, (int)midiChannel, (int)midiBend);
        break;

      case MIDI_CONTROL_CHANGE: //MIDI controller messages
        midiChannel      = MIDI_GET_CHANNEL(midiStream);
        midiControl      = MIDI_GET_CONTROL(midiStream);
        midiControlValue = MIDI_GET_CONTROL_VALUE(midiStream);
        tsf_channel_midi_control(pCtx->pTsf, (int)midiChannel, (int)midiControl, (int)midiControlValue);
        break;

      case MIDI_SYSEX_START:
        AudioAlgo_trace(pAlgo, TRACE_LVL_LOG, NULL, 0, "Midi Start");
        break;

      case MIDI_SYSEX_END_3BYTE:
      case MIDI_POLY_KEYPRESS:
        // not handled , assumes 3 bytes
        break;

      case MIDI_MISC:
      case MIDI_CABLE_EVENT:
        // These are reserved and unused, possibly issue somewhere, skip this packet
        return;
        break;

      case MIDI_SYSEX_END_1BYTE:
      case MIDI_1BYTE_DATA:
      case MIDI_SYSCOM_2BYTE:
      case MIDI_SYSEX_END_2BYTE:
      case MIDI_CHANNEL_PRESSURE:
        break;

      default:
        AudioAlgo_trace(pAlgo, TRACE_LVL_WARNING, NULL, 0, "Midi code ignored %02X:%02X:%02X", midiStream[0], midiStream[1], midiStream[1]);
        break;
    }

  }
  #endif
}



/**
* @brief Reset and open the sf2
*

* @param pHandle the algo instance
* @return int32_t error code
*/

static int32_t s_check_sf2_file(sound_fontCtx_t *pHandle)
{
  int32_t  error = AUDIO_ERR_MGNT_NONE;
  const sound_font_static_config_t *const pStaticConfig = (sound_font_static_config_t const *)AudioAlgo_getStaticConfig(pHandle->pAlgo);

  sf2_toc *pToc  = (sf2_toc *)pStaticConfig->soundFontPtr;

  pHandle->fileValid = false;
  /* at this point we have a risk of crash if the ptr is not properly set, we do some check but .... */
  if ((pToc != NULL) && ((((uint32_t)(pToc)) % 4U) == 0U) && (pToc->signature == SF2_SIGNATURE) && (pToc->offSf2 != 0U))
  {
    pHandle->fileValid = true;
    pHandle->pSf2      = (void *)(((uint8_t *)pToc) + pToc ->offSf2);
    pHandle->szSf2     = pToc->szSf2;
  }
  else
  {
    AudioAlgo_trace(pHandle->pAlgo, TRACE_LVL_ERROR, NULL, 0, "No Sound font SF2 flashed at 0x%p", pStaticConfig->soundFontPtr);
    error = AUDIO_ERR_MGNT_CONFIG;
  }
  return error;
}



/**
* @brief Init the algo
*
* @param pAlgo the algo instance
* @return int32_t error code
*/
static int32_t s_sound_font_init(audio_algo_t *const pAlgo)
{
  int32_t                                  error          = AUDIO_ERR_MGNT_NONE;
  const sound_font_static_config_t  *const pStaticConfig  = (sound_font_static_config_t const *)AudioAlgo_getStaticConfig(pAlgo);
  const sound_font_dynamic_config_t *const pDynamicConfig = (sound_font_dynamic_config_t const *)AudioAlgo_getDynamicConfig(pAlgo);
  audio_chunk_t *const                     pChunkOut      = AudioAlgo_getChunkPtrOut(pAlgo, 0U);
  audio_buffer_t const *const              pBuffOut       = AudioChunk_getBuffInfo(pChunkOut);
  audio_chain_utilities_t *const           pUtilsHandle   = AudioAlgo_getUtilsHdle(pAlgo);
  memPool_t                                memPool        = AUDIO_MEM_UNKNOWN;
  sound_fontCtx_t                         *pCtx           = NULL;

  if ((pStaticConfig == NULL) || (pDynamicConfig == NULL))
  {
    AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "static or dynamic config is missing!");
    error = AUDIO_ERR_MGNT_INIT;
  }

  if (AudioError_isOk(error))
  {
    memPool = (memPool_t)pStaticConfig->ramType;
    pCtx    = (sound_fontCtx_t *)AudioAlgo_malloc(sizeof(sound_fontCtx_t), memPool);
    if (pCtx == NULL)
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "context malloc failed !");
      error = AUDIO_ERR_MGNT_ALLOCATION;
    }
    else
    {
      memset(pCtx, 0, sizeof(*pCtx));
      AudioAlgo_setWrapperContext(pAlgo, pCtx);
      pCtx->memPool          = memPool;
      pCtx->fs               = AudioBuffer_getFs(pBuffOut);
      pCtx->nbChannels       = SF_NB_CHANELS;
      pCtx->nbSamples        = AudioBuffer_getNbSamples(pBuffOut);
      pCtx->pChunkOut        = pChunkOut;
      pCtx->pAlgo            = pAlgo;
    }
  }

  if (AudioError_isOk(error))
  {
    // create pCtx->faustAudioBuffer (for sfcSetContext purpose and for FAUST processing purpose) to be compliant with pScratchSample samples format (float, non-interleaved, stereo)
    error = AudioBuffer_create(&pCtx->sf2AudioBuffer, pCtx->nbChannels, pCtx->fs, pCtx->nbSamples, ABUFF_FORMAT_TIME, ABUFF_FORMAT_FLOAT, ABUFF_FORMAT_INTERLEAVED, memPool);
    if (AudioError_isError(error))
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "AudioBuffer_create(&pCtx->sf2udioBuffer, ...) error !");
      error = AUDIO_ERR_MGNT_ALLOCATION;
    }
    else
    {
      pCtx->pScratchSample = AudioBuffer_getPdataCh(&pCtx->sf2AudioBuffer, 0);
      pCtx->szScratch      = pCtx->nbChannels * pCtx->nbSamples * sizeof(float);
    }
  }

  if (AudioError_isOk(error))
  {
    pCtx->pScratchSampleProcess = (float *)AudioAlgo_malloc(pCtx->szScratch, memPool);
    if (pCtx->pScratchSampleProcess == NULL)
    {
      error = AUDIO_ERR_MGNT_ALLOCATION;
    }
  }

  if (AudioError_isOk(error))
  {
    error = sfcSetContext(&pCtx->sfcOutContext, &pCtx->sf2AudioBuffer, pBuffOut, false, 1.0f, pUtilsHandle);
    if (AudioError_isError(error))
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "sfcSetContext(&pCtx->SfcInContext, ...) error !");
      error = AUDIO_ERR_MGNT_ALLOCATION;
    }
  }

  if (AudioError_isOk(error))
  {
    pCtx->szSoundFontWav = pCtx->szScratch * SOUND_FONT_FRAME_LATENCY;
    error = s_rb_init(&pCtx->hSoundFontWav, pCtx->szSoundFontWav) ? AUDIO_ERR_MGNT_NONE : AUDIO_ERR_MGNT_ALLOCATION;
  }
  if (AudioError_isOk(error))
  {
    error = s_rb_init(&pCtx->hMidiStream, SF_SIZE_MIDI_BUFFER) ? AUDIO_ERR_MGNT_NONE : AUDIO_ERR_MGNT_ALLOCATION;
  }

  if (AudioError_isOk(error))
  {
    tsfMemPool = memPool; // save memPool in global variable tsfMemPool for sound-fond allocation
    error      = s_sound_font_configure(pAlgo);
  }
  if (AudioError_isOk(error))
  {
    tsf_set_output(pCtx->pTsf, (pCtx->nbChannels == 1U) ? TSF_MONO : TSF_STEREO_INTERLEAVED, (int)pCtx->fs, 0.0f);
  }


  #if CFG_TUD_MIDI_IN
  if (AudioError_isOk(error))
  {
    usb_midi_listener_register(s_midi_player_event, pAlgo);
  }
  #endif


  if (AudioError_isError(error))
  {
    s_sound_font_deinit(pAlgo);
  }
  else
  {
    pCtx->bInitDone = true;
  }

  return error;
}


/**
* @brief deInit the algo
*
* @param pAlgo the algo instance
* @return int32_t error code
*/
static int32_t s_sound_font_deinit(audio_algo_t *const pAlgo)
{
  int32_t                error = AUDIO_ERR_MGNT_NONE;
  sound_fontCtx_t *const pCtx  = (sound_fontCtx_t *)AudioAlgo_getWrapperContext(pAlgo);

  #if CFG_TUD_MIDI_IN
  usb_midi_listener_unregister(s_midi_player_event);
  #endif

  if (pCtx != NULL)
  {
    /* disconnect context from Algo first: insure algo won't be executed during deinit */
    memPool_t const memPool = pCtx->memPool;

    AudioAlgo_setWrapperContext(pAlgo, NULL);

    if (pCtx->pTsf != NULL)
    {
      tsfMemPool = memPool; // save memPool in global variable tsfMemPool for sound-fond deallocation
      tsf_close(pCtx->pTsf);
    }
    if (pCtx->pScratchSampleProcess != NULL)
    {
      AudioAlgo_free(pCtx->pScratchSampleProcess, memPool);  /*cstat !MISRAC2012-Rule-20.7 don't know why MISRAC complains whereas it doesn't complain for the other AudioAlgo_free a few lines after*/
    }
    s_rb_term(&pCtx->hSoundFontWav);
    s_rb_term(&pCtx->hMidiStream);

    AudioBuffer_deinit(&pCtx->sf2AudioBuffer);

    /* close the API */
    AudioAlgo_free(pCtx, memPool);
  }

  return error;
}


/**
* @brief configure the algo (dynamic parameters)
*
* @param pAlgo the algo instance
* @return int32_t error code
*/
static int32_t s_sound_font_configure(audio_algo_t *const pAlgo)
{
  int32_t                error = AUDIO_ERR_MGNT_NONE;
  sound_fontCtx_t *const pCtx  = (sound_fontCtx_t *)AudioAlgo_getWrapperContext(pAlgo);

  if (pCtx != NULL)
  {
    error = s_check_sf2_file(pCtx);
    if (AudioError_isOk(error))
    {
      pCtx->pTsf = tsf_load_memory(pCtx->pSf2, (int)pCtx->szSf2);
      if (pCtx->pTsf == NULL)
      {
        error = AUDIO_ERR_MGNT_CONFIG;
      }
      else
      {
        // Initialize preset on special 10th MIDI channel to use percussion sound bank (128) if available
        tsf_channel_set_bank_preset(pCtx->pTsf, 9, 128, 0);
        pCtx->fileValid = true;
      }
    }
  }

  return error;
}


/**
* @brief Manage the data in out
*
* @param pAlgo the algo instance
* @return int32_t error code
*/

static int32_t s_sound_font_dataInOut(audio_algo_t *const pAlgo)
{
  int32_t                                  error          = AUDIO_ERR_MGNT_NONE;
  sound_fontCtx_t                   *const pCtx           = (sound_fontCtx_t *)AudioAlgo_getWrapperContext(pAlgo);
  const sound_font_dynamic_config_t *const pDynamicConfig = (sound_font_dynamic_config_t const *)AudioAlgo_getDynamicConfig(pAlgo);

  if (pCtx->fileValid && pCtx->bInitDone)
  {
    int channel = 0;
    if (pDynamicConfig->drums)
    {
      channel = 9;
      pCtx->lastKey = pDynamicConfig->preset;
    }
    else
    {
      channel = 0;
      pCtx->lastKey = 48;
    }

    if (pCtx->lastGate != pDynamicConfig->gate)
    {
      if (pDynamicConfig->gate != 0U)
      {
        const char *presetname = tsf_get_presetname(pCtx->pTsf, (int)pDynamicConfig->preset);
        tsf_note_off_all(pCtx->pTsf);
        tsf_channel_set_presetnumber(pCtx->pTsf, channel, (int)pDynamicConfig->preset, (int)(channel == 9));
        tsf_channel_note_on(pCtx->pTsf, channel, (int)pCtx->lastKey, 1.0F);//C2
        AudioAlgo_trace(pAlgo, TRACE_LVL_LOG, NULL, 0, "Midi:Programme: %d:%s", pDynamicConfig->preset, (presetname == NULL) ? "" : presetname);
      }
      else
      {
        tsf_channel_note_off(pCtx->pTsf, channel, (int)pCtx->lastKey);
      }
    }
    pCtx->lastGate = pDynamicConfig->gate;
    s_rb_read(&pCtx->hSoundFontWav, (void *)pCtx->pScratchSample, pCtx->szScratch);
    sfcSampleBufferConvert(&pCtx->sfcOutContext, pCtx->pScratchSample, AudioChunk_getWritePtr0(pCtx->pChunkOut), (int)pCtx->nbChannels, (int)pCtx->nbSamples);
    /* mandatory to call the process cb */
    AudioAlgo_incReadyForProcess(pAlgo);
  }
  return error;
}

/**
* @brief decoder Midi buffer
*
*
* @param pAlgo   instance  pointer algo
* @return error code
*/

static int32_t s_sound_font_process(audio_algo_t *const pAlgo)
{
  sound_fontCtx_t *const pCtx = (sound_fontCtx_t *)AudioAlgo_getWrapperContext(pAlgo);
  /*
  According to the midi file, and the number of channels played in same time and the CPU power.
  The rendering time could be important, so, the rendering task is deferred in the process callback to not lock the system when the rendering time exceed the data in out period.
  In this case, the rendering will be wrong but will not crash the board
  */
  /* parse the meidi stream until it is empty */
  s_midi_play(pAlgo);
  /* render the wave */
  tsf_render_float(pCtx->pTsf, pCtx->pScratchSampleProcess, (int)pCtx->nbSamples, 0);
  s_rb_write(&pCtx->hSoundFontWav, (void *)pCtx->pScratchSampleProcess, pCtx->szScratch);
  return AUDIO_ERR_MGNT_NONE;
}
#endif
