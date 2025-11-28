/**
******************************************************************************
* @file    stm32_audio_tinyusb.c
* @author  MCD Application Team
* @brief   Encapsulate USB functions.
******************************************************************************
* @attention
*
* Copyright (c) 2018(-2022) STMicroelectronics.
* All rights reserved.
*
* This software is licensed under terms that can be found in the LICENSE file
* in the root directory of this software component.
* If no LICENSE file comes with this software, it is provided AS-IS.
*
******************************************************************************
*/
#if defined(TUD_AUDIO_IN_USED) || defined(TUD_AUDIO_OUT_USED)


/* Includes ------------------------------------------------------------------*/
#include "tinyusb.h"
#include "tusb_config.h"
#include "stm32_audio.h"


/* Global variables ----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
typedef int32_t (usbTx_cb_t)(audio_buffer_t *const pBuff);
typedef void (merge_cb_t)(audio_buffer_t const *const pBuffIn1, uint8_t ch1Id, audio_buffer_t const *const pBuffIn2, uint8_t ch2Id, audio_buffer_t *const pBuffOut);


typedef struct
{
  audio_buffer_t          usbBuffInfo;  /* final buffer sent to USB @ max freq */
  audio_buffer_t          appBuffInfo;  /* optional: only necessary if application sends buffer < max freq */
  audio_buffer_t         *pBuffInfo;    /* set at init as function of in & out sample rate frequencies */
  usbTx_cb_t             *pFuncTx;      /* Registered callback at init to avoid test at running time */
  merge_cb_t             *pFuncMerge;
}
UTIL_AUDIO_USB_stream_t;

typedef struct
{
  UTIL_AUDIO_USB_stream_t rec;           /* Record stream */
  UTIL_AUDIO_USB_stream_t play;          /* Playback stream */
}
UTIL_AUDIO_USB_context_t;

/* Private variables ---------------------------------------------------------*/
static UTIL_AUDIO_USB_context_t G_context;


/* Private function prototypes -----------------------------------------------*/
static void     s_play_init(void);
static void     s_play_deinit(void);
static void     s_rec_init(void);
static void     s_rec_deinit(void);

static void     s_initBuffers(UTIL_AUDIO_USB_stream_t *const pStream,
                              uint8_t                  const nbChannels,
                              uint32_t                 const appFs,
                              uint32_t                 const usbFs,
                              usbTx_cb_t                    *pFuncTx,
                              usbTx_cb_t                    *pFuncResampledTx);

static int32_t  s_play_receiveAudioBuffer(audio_buffer_t  *const pBuff);
static int32_t  s_rec_sendAudioBuffer(audio_buffer_t      *const pBuff);
static void     s_merge2Stereo_32bit(audio_buffer_t const *const pBuffIn1, uint8_t ch1Id, audio_buffer_t const *const pBuffIn2, uint8_t ch2Id, audio_buffer_t *const pBuffOut);
static void     s_merge2Stereo_16bit(audio_buffer_t const *const pBuffIn1, uint8_t ch1Id, audio_buffer_t const *const pBuffIn2, uint8_t ch2Id, audio_buffer_t *const pBuffOut);
static int32_t  s_merge2Stereo(audio_buffer_t       const *const pBuffIn1, uint8_t const ch1Id, audio_buffer_t const *const pBuffIn2, uint8_t const ch2Id, audio_buffer_t *const pBuffOut);

/* Functions Definition ------------------------------------------------------*/

void UTIL_AUDIO_USB_init(void)
{
  #if defined(TUD_AUDIO_IN_USED) || defined(TUD_AUDIO_OUT_USED)
  tusb_device_conf_t hTusbConfig;
  tusb_device_default_conf(&hTusbConfig);

  hTusbConfig.uac2_audio.timeFrameMs = (float)UTIL_AUDIO_N_MS_PER_INTERRUPT / (float)UTIL_AUDIO_N_MS_DIV;
  hTusbConfig.uac2_audio.rec_szSple  = UTIL_AUDIO_USB_BIT_RESOLUTION / 8U;
  hTusbConfig.uac2_audio.rec_freq    = UTIL_AUDIO_IN_FREQUENCY;
  hTusbConfig.uac2_audio.rec_ch      = UTIL_AUDIO_IN_CH_NB;
  hTusbConfig.uac2_audio.play_szSple = UTIL_AUDIO_USB_BIT_RESOLUTION / 8U;
  hTusbConfig.uac2_audio.play_freq   = UTIL_AUDIO_OUT_FREQUENCY;
  hTusbConfig.uac2_audio.play_ch     = UTIL_AUDIO_OUT_CH_NB;

  #if !defined(TUD_AUDIO_IN_USED)
  hTusbConfig.play_enabled = false;
  #endif
  #if !defined(TUD_AUDIO_OUT_USED)
  hTusbConfig.rec_enabled = false;
  #endif
  tusb_device_init(&hTusbConfig);
  tusb_device_start();
  #endif
  s_play_init();
  s_rec_init();

}

void UTIL_AUDIO_USB_deinit(void)
{
  #if defined(TUD_AUDIO_IN_USED) || defined(TUD_AUDIO_OUT_USED)
  tusb_device_deinit();
  #endif

  s_play_deinit();
  s_rec_deinit();
}

void UTIL_AUDIO_USB_idle(void)
{
}


audio_buffer_t *UTIL_AUDIO_USB_REC_getAudioBuffer(void)
{
  return G_context.rec.pBuffInfo;
}


audio_buffer_t *UTIL_AUDIO_USB_PLAY_getAudioBuffer(void)
{
  return G_context.play.pBuffInfo;
}

void UTIL_AUDIO_USB_PLAY_clearBuffer(void)
{
  if (AudioError_isError(AudioBuffer_resetData(&G_context.play.usbBuffInfo)))
  {
    UTIL_AUDIO_error();
  }
  if (tud_audio_play_frequency() != UTIL_AUDIO_OUT_FREQUENCY)
  {
    if (AudioError_isError(AudioBuffer_resetData(&G_context.play.appBuffInfo)))
    {
      UTIL_AUDIO_error();
    }
  }
}

int32_t UTIL_AUDIO_USB_PLAY_get(audio_buffer_t const *const pBuff)
{
  int32_t        error      = AUDIO_ERR_MGNT_NONE;
  if (G_context.play.pFuncTx != NULL)
  {
    error = G_context.play.pFuncTx((audio_buffer_t *) pBuff);
  }
  else
  {
    error = AUDIO_ERR_MGNT_INIT;
  }

  if (AudioError_isError(error))
  {
    UTIL_AUDIO_error();
  }
  return error;
}

void UTIL_AUDIO_USB_REC_send(audio_buffer_t const *const pBuff)
{
  /* uint16_t const nbSamples  = (uint16_t)AudioBuffer_getNbSamples(pBuff); */
  uint16_t const nbChannels = (uint16_t)AudioBuffer_getNbChannels(pBuff);
  static uint8_t calledOnce = 1U;

  if (calledOnce)
  {
    calledOnce = 0U;
  }
  else
  {
    if (nbChannels == tud_audio_rec_channel_count())
    {
      if (G_context.rec.pFuncTx != NULL)
      {
        G_context.rec.pFuncTx((audio_buffer_t *) pBuff);
      }
      else
      {
        UTIL_AUDIO_error();
      }
    }
    else
    {
      UTIL_AUDIO_USB_REC_mergeAndSend(pBuff, pBuff);
    }
  }
}

void UTIL_AUDIO_USB_REC_mergeAndSend(audio_buffer_t const *const pIn1Buffer, audio_buffer_t const *const pIn2Buffer)
{
  static audio_buffer_t UsbInBuffer;
  static uint8_t        G_isUsbInAllocated = 0U;

  if ((pIn1Buffer != NULL) && (pIn2Buffer != NULL))
  {
    int32_t error = AUDIO_ERR_MGNT_NONE;

    if (G_isUsbInAllocated == 0U)
    {
      /* initialize audio_buffer_t structure of microphone signals*/
      audio_buffer_type_t bufferType;

      switch (UTIL_AUDIO_USB_BIT_RESOLUTION)
      {
        case UTIL_AUDIO_RESOLUTION_8B:
        case UTIL_AUDIO_RESOLUTION_16B:
          bufferType = ABUFF_FORMAT_FIXED16;
          break;
        case UTIL_AUDIO_RESOLUTION_24B:
        case UTIL_AUDIO_RESOLUTION_32B:
          bufferType = ABUFF_FORMAT_FIXED32;
          break;
        default:
          bufferType = ABUFF_FORMAT_UNKNOWN;
          break;
      }

      error = AudioBuffer_create(&UsbInBuffer,
                                 UTIL_AUDIO_OUT_CH_NB,
                                 UTIL_AUDIO_OUT_FREQUENCY,
                                 AudioBuffer_getNbSamples(pIn1Buffer),
                                 ABUFF_FORMAT_TIME,
                                 bufferType,
                                 ABUFF_FORMAT_INTERLEAVED,
                                 UTIL_AUDIO_MEMPOOL);

      if (AudioError_isOk(error))
      {
        G_isUsbInAllocated = 1U;
      }
    }

    if (AudioError_isOk(error))
    {
      if (AudioBuffer_getPdata(&UsbInBuffer) != NULL)
      {
        error = s_merge2Stereo(pIn1Buffer, UTIL_AUDIO_USB_REC_getChannelsId1(), pIn2Buffer, UTIL_AUDIO_USB_REC_getChannelsId2(), &UsbInBuffer);
      }
    }

    if ((AudioError_isOk(error)) && (G_context.rec.pFuncTx != NULL))
    {
      G_context.rec.pFuncTx(&UsbInBuffer);
    }

    if (AudioError_isNok(error))
    {
      UTIL_AUDIO_error();
    }
  }
}


/* Static Functions Prototypes ************************************************/

/* Static Functions Prototypes for playback ***********************************/
static void s_play_init(void)
{
  s_initBuffers(&G_context.play,
                UTIL_AUDIO_IN_CH_NB,
                UTIL_AUDIO_IN_FREQUENCY,
                UTIL_AUDIO_IN_FREQUENCY,
                s_play_receiveAudioBuffer,
                NULL);
}


static void s_play_deinit(void)
{
  AudioBuffer_deinit(&G_context.play.usbBuffInfo);
  if (tud_audio_play_frequency() != UTIL_AUDIO_OUT_FREQUENCY)
  {
    AudioBuffer_deinit(&G_context.play.appBuffInfo);
  }
}




static int32_t s_play_receiveAudioBuffer(audio_buffer_t *const pBuff)
{
  int32_t error = AUDIO_ERR_MGNT_NONE;
  if (pBuff == NULL)
  {
    error = AUDIO_ERR_MGNT_ALLOCATION;
  }
  else
  {
    #if defined(TUD_AUDIO_IN_USED)
    void          *pDestData = (void *)AudioBuffer_getPdata(pBuff);
    uint16_t const szBytes   = (uint16_t)AudioBuffer_getBufferSize(pBuff);

    if ((UTIL_AUDIO_RENDER_isStarted()) && (pDestData != NULL))
    {
      tud_audio_play(pDestData, szBytes);
    }
    #endif
  }
  return error;
}

/* Static Functions Prototypes for record *************************************/

static void s_rec_init(void)
{
  s_initBuffers(&G_context.rec,
                UTIL_AUDIO_OUT_CH_NB,
                UTIL_AUDIO_OUT_FREQUENCY,
                UTIL_AUDIO_OUT_FREQUENCY,
                s_rec_sendAudioBuffer,
                NULL);

  switch (UTIL_AUDIO_USB_BIT_RESOLUTION)
  {
    case UTIL_AUDIO_RESOLUTION_8B:
    case UTIL_AUDIO_RESOLUTION_16B:
      G_context.rec.pFuncMerge = s_merge2Stereo_16bit;
      break;
    case UTIL_AUDIO_RESOLUTION_24B:
    case UTIL_AUDIO_RESOLUTION_32B:
      G_context.rec.pFuncMerge = s_merge2Stereo_32bit;
      break;
    default:
      G_context.rec.pFuncMerge = NULL;
      break;
  }
}

static void s_rec_deinit(void)
{
  AudioBuffer_deinit(&G_context.rec.usbBuffInfo);
  if (tud_audio_rec_frequency() != UTIL_AUDIO_IN_FREQUENCY)
  {
    AudioBuffer_deinit(&G_context.rec.appBuffInfo);
  }
}

static int32_t s_rec_sendAudioBuffer(audio_buffer_t *const pBuff)
{
  #if defined(TUD_AUDIO_OUT_USED)
  uint32_t const nbSamples  = AudioBuffer_getNbSamples(pBuff);
  uint32_t const nbChannels = AudioBuffer_getNbChannels(pBuff);
  tud_audio_rec(AudioBuffer_getPdata(pBuff), (uint32_t)(nbSamples * nbChannels * UTIL_AUDIO_USB_BIT_RESOLUTION / 8U));
  #endif
  return AUDIO_ERR_MGNT_NONE; /* Todo error mgnt */
}




static void s_initBuffers(UTIL_AUDIO_USB_stream_t *const pStream,
                          uint8_t                  const nbChannels,
                          uint32_t                 const appFs,
                          uint32_t                 const usbFs,
                          usbTx_cb_t                    *pFuncTx,
                          usbTx_cb_t                    *pFuncResampledTx)
{
  uint16_t const appNbSamples = (uint16_t)((UTIL_AUDIO_N_MS_PER_INTERRUPT * (appFs / 1000UL)) / UTIL_AUDIO_N_MS_DIV);
  uint16_t const usbNbSamples = (uint16_t)((UTIL_AUDIO_N_MS_PER_INTERRUPT * (usbFs / 1000UL)) / UTIL_AUDIO_N_MS_DIV);
  audio_buffer_type_t bufferType;

  switch (UTIL_AUDIO_USB_BIT_RESOLUTION)
  {
    case UTIL_AUDIO_RESOLUTION_8B:
    case UTIL_AUDIO_RESOLUTION_16B:
      bufferType = ABUFF_FORMAT_FIXED16;
      break;
    case UTIL_AUDIO_RESOLUTION_24B:
    case UTIL_AUDIO_RESOLUTION_32B:
      bufferType = ABUFF_FORMAT_FIXED32;
      break;
    default:
      bufferType = ABUFF_FORMAT_UNKNOWN;
      break;
  }

  if (AudioError_isError(AudioBuffer_create(&pStream->usbBuffInfo,
                                            nbChannels,
                                            usbFs,
                                            usbNbSamples,
                                            ABUFF_FORMAT_TIME,
                                            bufferType,
                                            ABUFF_FORMAT_INTERLEAVED,
                                            UTIL_AUDIO_MEMPOOL)))
  {
    UTIL_AUDIO_error();
  }

  if (usbFs != appFs)
  {
    pStream->pBuffInfo = &pStream->appBuffInfo;
    pStream->pFuncTx   = pFuncResampledTx;

    if (AudioError_isError(AudioBuffer_create(&pStream->appBuffInfo,
                                              nbChannels,
                                              appFs,
                                              appNbSamples,
                                              ABUFF_FORMAT_TIME,
                                              bufferType,
                                              ABUFF_FORMAT_INTERLEAVED,
                                              UTIL_AUDIO_MEMPOOL)))
    {
      UTIL_AUDIO_error();
    }
  }
  else
  {
    pStream->pBuffInfo = &pStream->usbBuffInfo;
    pStream->pFuncTx   = pFuncTx;
  }
}

int32_t s_merge2Stereo(audio_buffer_t const *const pBuffIn1, uint8_t ch1Id, audio_buffer_t const *const pBuffIn2, uint8_t ch2Id, audio_buffer_t *const pBuffOut)
{
  int32_t error = AUDIO_ERR_MGNT_NONE;

  if ((pBuffIn1 == NULL) || (pBuffIn1->pInternalMem == NULL) || (pBuffIn2 == NULL) || (pBuffIn2->pInternalMem == NULL) || (pBuffOut == NULL) || (pBuffOut->pInternalMem == NULL))
  {
    error = AUDIO_ERR_MGNT_ALLOCATION;
  }
  else
  {

    if ((pBuffIn1->timeFreq    != ABUFF_FORMAT_TIME)        ||
        (pBuffIn2->timeFreq    != ABUFF_FORMAT_TIME)        ||
        (pBuffOut->timeFreq    != ABUFF_FORMAT_TIME)        ||
        (pBuffOut->interleaved != ABUFF_FORMAT_INTERLEAVED) ||
        (pBuffIn2->type        != pBuffIn1->type)           ||
        (pBuffOut->type        != pBuffIn1->type)           ||
        (ch1Id                 >= pBuffIn1->nbChannels)     ||
        (ch2Id                 >= pBuffIn2->nbChannels)     ||
        (pBuffIn1->interleaved != pBuffIn2->interleaved)    ||
        (pBuffOut->nbChannels  != 2U)                       ||
        (pBuffIn2->nbElements  != pBuffIn1->nbElements)     ||
        (pBuffOut->nbElements  != pBuffIn1->nbElements))
    {
      error = AUDIO_ERR_MGNT_ERROR;
    }
    else
    {
      if (G_context.rec.pFuncMerge != NULL)
      {
        G_context.rec.pFuncMerge(pBuffIn1, ch1Id, pBuffIn2, ch2Id, pBuffOut);
      }
      else
      {
        error = AUDIO_ERR_MGNT_ERROR;
      }
    }
  }

  return error;
}

void s_merge2Stereo_16bit(audio_buffer_t const *const pBuffIn1, uint8_t ch1Id, audio_buffer_t const *const pBuffIn2, uint8_t ch2Id, audio_buffer_t *const pBuffOut)
{

  uint32_t const  nbSamples        = (uint32_t)AudioBuffer_getNbElements(pBuffOut);
  /* uint8_t  const  nbCh1            = AudioBuffer_getNbChannels(pBuffIn1);
  uint8_t  const  nbCh2            = AudioBuffer_getNbChannels(pBuffIn2); */
  uint16_t const  in1SamplesOffset = (uint16_t)AudioBuffer_getSamplesOffset(pBuffIn1);
  uint16_t const  in2SamplesOffset = (uint16_t)AudioBuffer_getSamplesOffset(pBuffIn2);
  int16_t  const *pIn1Int16        = (int16_t const *)AudioBuffer_getPdataCh(pBuffIn1, ch1Id);
  int16_t  const *pIn2Int16        = (int16_t const *)AudioBuffer_getPdataCh(pBuffIn2, ch2Id);
  int16_t        *pOutInt16        = (int16_t *)      AudioBuffer_getPdata(pBuffOut);

  for (uint32_t spl = 0; spl < nbSamples; spl++)
  {
    *pOutInt16 = *pIn1Int16; pOutInt16 ++; pIn1Int16  += in1SamplesOffset;
    *pOutInt16 = *pIn2Int16; pOutInt16 ++; pIn2Int16  += in2SamplesOffset;
  }
}


void s_merge2Stereo_32bit(audio_buffer_t const *const pBuffIn1, uint8_t ch1Id, audio_buffer_t const *const pBuffIn2, uint8_t ch2Id, audio_buffer_t *const pBuffOut)
{

  uint32_t const  nbSamples        = (uint32_t)AudioBuffer_getNbElements(pBuffOut);
  /* uint8_t  const  nbCh1            = AudioBuffer_getNbChannels(pBuffIn1);
  uint8_t  const  nbCh2            = AudioBuffer_getNbChannels(pBuffIn2); */
  uint32_t const  in1SamplesOffset = (uint32_t)AudioBuffer_getSamplesOffset(pBuffIn1);
  uint32_t const  in2SamplesOffset = (uint32_t)AudioBuffer_getSamplesOffset(pBuffIn2);
  int32_t  const *pIn1Int32        = (int32_t const *)AudioBuffer_getPdataCh(pBuffIn1, ch1Id);
  int32_t  const *pIn2Int32        = (int32_t const *)AudioBuffer_getPdataCh(pBuffIn2, ch2Id);
  int32_t        *pOutInt32        = (int32_t *)      AudioBuffer_getPdata(pBuffOut);

  for (uint32_t spl = 0; spl < nbSamples; spl++)
  {
    *pOutInt32 = *pIn1Int32; pOutInt32 ++; pIn1Int32  += in1SamplesOffset;
    *pOutInt32 = *pIn2Int32; pOutInt32 ++; pIn2Int32  += in2SamplesOffset;
  }
}
#endif