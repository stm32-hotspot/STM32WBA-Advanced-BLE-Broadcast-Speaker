/**
******************************************************************************
* @file    stm32_audio.c
* @author  MCD Application Team
* @brief   Manage audio start stop etc...
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

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include "stm32_audio.h"
#include "stm32_audio_tasks.h"
#include "audio_chain_instance_conf.h"

#ifdef UTIL_AUDIO_DFSDM_USED
  #include "stm32_audio_dfsdm.h"
#endif

/* Global variables ----------------------------------------------------------*/

/* Private defines -----------------------------------------------------------*/

/* Sizing the static buffer for microphone capture, allocation in cache safe
*  memory zone*/

#ifdef UTIL_AUDIO_IN_PDM_USED /* Microphones connected to SAIPDM or I2S*/
#if USE_PDM2PCM_LIB
  #include "pdm2pcm_glo.h"
#endif /* USE_PDM2PCM_LIB */

  #ifndef UTIL_AUDIO_IN_PDM2PCM_USED
    #define UTIL_AUDIO_IN_PDM2PCM_USED 1U
  #endif

  #ifndef UTIL_AUDIO_IN_PDM_MSB_LSB
    #define UTIL_AUDIO_IN_PDM_MSB_LSB ABUFF_FORMAT_PDM_LSB_FIRST
  #endif


  #if (UTIL_AUDIO_MAX_IN_FREQUENCY == 96000UL)
    #ifndef UTIL_AUDIO_MICS_PDM2PCM_RATIO
      #define UTIL_AUDIO_MICS_PDM2PCM_RATIO     32UL
    #endif
    #define UTIL_AUDIO_MICS_PDM2PCM_RATIO_MAX 32UL

  #elif  (UTIL_AUDIO_MAX_IN_FREQUENCY == 48000UL)
    #ifndef UTIL_AUDIO_MICS_PDM2PCM_RATIO
      #define UTIL_AUDIO_MICS_PDM2PCM_RATIO     64UL
    #endif
    #define UTIL_AUDIO_MICS_PDM2PCM_RATIO_MAX 64UL

  #elif  (UTIL_AUDIO_MAX_IN_FREQUENCY == 16000UL)
    #ifndef UTIL_AUDIO_MICS_PDM2PCM_RATIO
      #define UTIL_AUDIO_MICS_PDM2PCM_RATIO     128UL
    #endif
    #define UTIL_AUDIO_MICS_PDM2PCM_RATIO_MAX 128UL

  #elif  (UTIL_AUDIO_MAX_IN_FREQUENCY == 8000UL)
    /* Take good care with this setup as it expects a pdm clock for the MEMs ~1024kHz;
    * some microphone may not support such low frequency;            */
    #ifndef UTIL_AUDIO_MICS_PDM2PCM_RATIO
      #define UTIL_AUDIO_MICS_PDM2PCM_RATIO     128UL
    #endif
    #define UTIL_AUDIO_MICS_PDM2PCM_RATIO_MAX 128UL
  #endif

  #define UTIL_AUDIO_MICS_PDM_BUFF_IS_PINGPONG  1
  #define UTIL_AUDIO_MICS_PDM_CLK_kHz           ((UTIL_AUDIO_MAX_IN_FREQUENCY / 1000UL) * UTIL_AUDIO_MICS_PDM2PCM_RATIO)
  #define UTIL_AUDIO_MICS_PDM_CLK_kHz_MAX       ((UTIL_AUDIO_MAX_IN_FREQUENCY / 1000UL) * UTIL_AUDIO_MICS_PDM2PCM_RATIO_MAX)
  #define UTIL_AUDIO_IN_PDM_DMA_BUFF_SIZE_BYTES (((1+UTIL_AUDIO_MICS_PDM_BUFF_IS_PINGPONG) * UTIL_AUDIO_N_MS_PER_INTERRUPT * UTIL_AUDIO_MICS_PDM_CLK_kHz_MAX/8 * UTIL_AUDIO_MAX_IN_CH_NB)/UTIL_AUDIO_N_MS_DIV)
#else

  #define UTIL_AUDIO_IN_PDM_DMA_BUFF_SIZE_BYTES       0UL
#endif

#ifdef UTIL_AUDIO_IN_PCM_USED /* Microphones connected to DFSDM, MDF*/

  #ifndef UTIL_AUDIO_IN_IS_DMA_DATA_INTERLEAVED
    #define UTIL_AUDIO_IN_IS_DMA_DATA_INTERLEAVED 0U // BY default no, matches DFSDM which doesn't support interleaving of microphones. In case of LineIn or MDF, it should be set to TRUE.
  #endif

  #ifndef UTIL_AUDIO_IN_DMA_BIT_RESOLUTION
    #define UTIL_AUDIO_IN_DMA_BIT_RESOLUTION 32U // MDF directly writes 16bit PCM samples, DFSDM writes 32bit samples (default setting is for dfsdm)
  #endif

  #ifndef UTIL_AUDIO_IN_MAX_DMA_BYTES_RESOLUTION
    #define UTIL_AUDIO_IN_MAX_DMA_BYTES_RESOLUTION 4U // MDF directly writes 16bit PCM samples, DFSDM writes 32bit samples (default setting is for dfsdm)
  #endif

  #define UTIL_AUDIO_IN_SCRATCH_SPLES_NB              ((2UL *  (UTIL_AUDIO_N_MS_PER_INTERRUPT * (UTIL_AUDIO_MAX_IN_FREQUENCY / 1000UL)) )/UTIL_AUDIO_N_MS_DIV) /* x2 for ping pong */
  #define UTIL_AUDIO_IN_SCRATCH_SIZE_BYTES            (UTIL_AUDIO_IN_SCRATCH_SPLES_NB * UTIL_AUDIO_IN_MAX_DMA_BYTES_RESOLUTION )
  #define UTIL_AUDIO_IN_PCM_DMA_BUFF_SIZE_BYTES       (UTIL_AUDIO_IN_SCRATCH_SIZE_BYTES * UTIL_AUDIO_MAX_IN_CH_NB)
#else

  #define UTIL_AUDIO_IN_PCM_DMA_BUFF_SIZE_BYTES        0UL
#endif

/* Buffer size in bytes*/
//#define MAX(a,b) (((a)>(b))?(a):(b))
#define UTIL_AUDIO_IN_DMA_BUFF_SIZE_BYTES              (((UTIL_AUDIO_IN_PDM_DMA_BUFF_SIZE_BYTES)>(UTIL_AUDIO_IN_PCM_DMA_BUFF_SIZE_BYTES))?(UTIL_AUDIO_IN_PDM_DMA_BUFF_SIZE_BYTES):(UTIL_AUDIO_IN_PCM_DMA_BUFF_SIZE_BYTES)) //(UTIL_AUDIO_IN_PDM_DMA_BUFF_SIZE_BYTES+UTIL_AUDIO_IN_PCM_DMA_BUFF_SIZE_BYTES)

/* Sizing the static buffer for playback, allocation in cache safe
*  memory zone*/

#ifndef UTIL_AUDIO_OUT_DMA_BUFF_BYTES_RESOLUTION
  #define UTIL_AUDIO_OUT_DMA_BUFF_BYTES_RESOLUTION     4U // default is 32-bit samples but an be set to 2 if only 16-bit is needed
#endif

/* Buffer size in bytes*/
#define UTIL_AUDIO_OUT_DMA_BUFF_SIZE_BYTES            ((2UL * UTIL_AUDIO_OUT_DMA_BUFF_BYTES_RESOLUTION *  (UTIL_AUDIO_N_MS_PER_INTERRUPT * (UTIL_AUDIO_MAX_OUT_FREQUENCY / 1000UL)*UTIL_AUDIO_MAX_OUT_CH_NB))/UTIL_AUDIO_N_MS_DIV) /* x2 for ping pong */


/* Private macros ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/



typedef void (txCompleteCb)(uint32_t const offsetSpleBytes, int32_t const instance);

#ifdef UTIL_AUDIO_IN_PDM_USED
typedef struct
{
  uint16_t                      libRatio;
  uint32_t                      ratio;
  uint32_t                      clk;
  audio_buffer_t                buffInfo; /*Used for microphones connected to SAIPDM or I2S interfaces */

} UTIL_AUDIO_PDM_context_t;
#endif

typedef struct
{
  bool                          isMono2Stereo;
  uint8_t                       isDmaInterleaved;
  uint8_t                       nbCh;
  uint8_t                       nbHwCh;
  uint32_t                      device;
  uint32_t                      nbSamples;
  uint32_t                      halfOffsetBytes;
  uint32_t                      sizeBytes;
  uint16_t                      dmaResoBytes;
  audio_buffer_t                buffInfo;

  uint8_t                      *pBaseAddr;
  txCompleteCb                 *txCpltCb;

  #ifdef UTIL_AUDIO_IN_PDM_USED
  UTIL_AUDIO_PDM_context_t      pdm; /*Used for microphones connected to SAIPDM or I2S interfaces */
  #endif

  #ifdef UTIL_AUDIO_IN_PCM_USED
  /* This table is storing base & half address per microphones. It is needed
  ** when using mutlimicrophone started through DFSDM to pass as pBuf argument
  ** of legacy API such as:
  ** int32_t BSP_AUDIO_IN_RecordEx(uint32_t Instance, uint8_t **pBuf, uint32_t NbrOfBytes)
  */
  int32_t                     *pPcmCacheSafeAddrList[UTIL_AUDIO_MAX_IN_CH_NB];
  #endif


} UTIL_AUDIO_IN_context_t;

typedef struct
{
  uint32_t                      device;
  bool                          isStarted;
  uint32_t                      nbSamples;
  audio_buffer_t                buffInfo;
  uint16_t                      volumeRequest;
  int8_t                       *pBasePtr;   /* BSP Audio Out buffer base address */
  int8_t                       *pHalfPtr;   /* BSP Audio Out buffer half address; computed at init to avoid re-computing every DMA IT */
} UTIL_AUDIO_OUT_context_t;

typedef struct
{
  UTIL_AUDIO_IN_context_t       rec;
  UTIL_AUDIO_OUT_context_t      play;
  uint8_t                       usbRecChId1;
  uint8_t                       usbRecChId2;
  UTIL_AUDIO_cbs_t              cbs;
} UTIL_AUDIO_context_t;


/* Private variables ---------------------------------------------------------*/
static UTIL_AUDIO_context_t ContextStatic;          /* used for main path  */
static UTIL_AUDIO_context_t ContextStaticAlternate; /* used for alternate path */

#ifdef UTIL_AUDIO_IN_PDM_USED
  static uint16_t tSamplesScratch[(UTIL_AUDIO_MAX_IN_FREQUENCY / 1000UL * UTIL_AUDIO_N_MS_PER_INTERRUPT) / UTIL_AUDIO_N_MS_DIV]; /* assume the max frequency is 48K */
#endif

static int32_t s_capture_init_pcm(UTIL_AUDIO_context_t *const pContext);
#ifdef UTIL_AUDIO_IN_PDM_USED
static int32_t s_capture_init_pdm(UTIL_AUDIO_context_t *const pContext);
#endif

/* Static allocation of microphones DMA scratch buffer and loudspeaker DMA buffer */
#ifdef STM32_AUDIO_USE_EXTERN_BUFF
extern int8_t* UtilAudio_InBuff;
extern int8_t* UtilAudio_OutBuff;

extern int8_t* UtilAudio_MicPDMScratchBuff;

extern int8_t* UtilAudio_OutBuffAlternate;
extern int8_t* UtilAudio_InBuffAlternate;
#else
VAR_DECLARE_ALIGN4_AT_SECTION(int8_t,  UtilAudio_InBuff[UTIL_AUDIO_IN_PCM_DMA_BUFF_SIZE_BYTES],             UTIL_AUDIO_MEMORY_SECTION_HW_BUFFERS);  /*cstat !MISRAC2012-Rule-1.4_a extended language features needed*/
VAR_DECLARE_ALIGN4_AT_SECTION(int8_t,  UtilAudio_OutBuff[UTIL_AUDIO_OUT_DMA_BUFF_SIZE_BYTES],               UTIL_AUDIO_MEMORY_SECTION_HW_BUFFERS);  /*cstat !MISRAC2012-Rule-1.4_a extended language features needed*/

VAR_DECLARE_ALIGN4_AT_SECTION(int8_t,  UtilAudio_MicPDMScratchBuff[UTIL_AUDIO_IN_PDM_DMA_BUFF_SIZE_BYTES],  UTIL_AUDIO_MEMORY_SECTION_HW_BUFFERS);

VAR_DECLARE_ALIGN4_AT_SECTION(int8_t,  UtilAudio_OutBuffAlternate[UTIL_AUDIO_OUT_DMA_BUFF_SIZE_BYTES],      UTIL_AUDIO_MEMORY_SECTION_HW_BUFFERS);
VAR_DECLARE_ALIGN4_AT_SECTION(int8_t,  UtilAudio_InBuffAlternate[UTIL_AUDIO_OUT_DMA_BUFF_SIZE_BYTES],       UTIL_AUDIO_MEMORY_SECTION_HW_BUFFERS);
#endif

/* Private function prototypes -----------------------------------------------*/
#ifdef UTIL_AUDIO_IN_PCM_USED
  static void s_inTxCplt_16bitDeInt_to_16bitInt_processCh(int16_t *pInSample, int16_t *pOutSample, uint8_t nbCh, uint8_t idxChannel, uint32_t nbSamples, uint16_t dmaResoBytes);

  static void s_inTxCplt_pcmbitInterleaved_cb(uint32_t                 const offsetSpleBytes, int32_t const instance);   /* if data from DMA is already pcm16-bit interleaved (LineIn or MDF)*/
  static void s_inTxCplt_pcm16bitInterleaved_m2s_cb(uint32_t           const offsetSpleBytes, int32_t const instance);   /* if data from DMA is already pcm16-bit interleaved (LineIn or MDF)*/
  static void s_inTxCplt_pcm32bitInterleaved_m2s_cb(uint32_t           const offsetSpleBytes, int32_t const instance);   /* if data from DMA is already pcm16-bit interleaved (LineIn or MDF)*/

  static void s_inTxCplt_16bitDeInt_to_16bitInt_cb(uint32_t            const offsetSpleBytes, int32_t const instance);
  static void s_inTxCplt_16bitDeInt_to_16bitInt_m2s_cb(uint32_t        const offsetSpleBytes, int32_t const instance);

  static void s_inTxCplt_32bitDeint_to_16bitInt_cb(uint32_t             const offsetSpleBytes, int32_t const instance);
  static void s_inTxCplt_32bitDeint_to_16bitInt_waitAllMics_cb(uint32_t const offsetSpleBytes, int32_t const instance);   /* Some early BSP used to send one cb per microphone; in this case we wait to get all the one requested to trigger the audioCapture task */

  static void s_inTxCplt_32bitDeint_to_32bitInt_cb(uint32_t             const offsetSpleBytes, int32_t const instance);
  static void s_inTxCplt_32bitDeint_to_32bitInt_waitAllMics_cb(uint32_t const offsetSpleBytes, int32_t const instance);   /* Some early BSP used to send one cb per microphone; in this case we wait to get all the one requested to trigger the audioCapture task */

#endif

#ifdef UTIL_AUDIO_IN_PDM_USED
  static void    s_inPdm2PcmTxCompleteCb(uint32_t const offsetSpleBytes, int32_t const instance);
  static void    s_inPdmTxCompleteCb(uint32_t const offsetSpleBytes, int32_t const instance);
#endif


/* Functions Definition ------------------------------------------------------*/
/**
  * @brief  Register BSP audio callbacks or other functions to control audio
  * @param  Handler of the callbacks
  * @retval None
  */
void UTIL_AUDIO_registerCallbacks(UTIL_AUDIO_cbs_t const *const pCbs, uint8_t path_id)
{
  if (path_id == MAIN_PATH)
  {
  ContextStatic.cbs = *pCbs;
  }
  else /* ALTERNATE_PATH */
  {
    ContextStaticAlternate.cbs = *pCbs;
  }
}

/**
  * @brief  Reset audio callbacks
  * @param  Handler of the callbacks
  * @retval None
  */
void UTIL_AUDIO_resetCallbacks(UTIL_AUDIO_cbs_t *const pCbs)
{
  memset(pCbs, 0, sizeof(UTIL_AUDIO_cbs_t));
}

/**
  * @brief  This utility code doesn't return error but call registered call back (while TRUE if no cb registered)
  * @param  None
  * @retval None
  */
void UTIL_AUDIO_error(void)
{
  if (ContextStatic.cbs.error != NULL)
  {
    ContextStatic.cbs.error();
  }
  else
  {
    while (1)
    {
    }
  }
}

/**
  * @brief  Init audio capture & rendering
  * @param  None
  * @retval None
  */
static uint8_t gEn_ALT = 0;
void UTIL_AUDIO_init(uint8_t enable_alt)
{
  /* Init of loudspeaker/headset streaming*/
  UTIL_AUDIO_RENDER_init(MAIN_PATH);

  /* Init of microphone recording */
  UTIL_AUDIO_CAPTURE_init(UTIL_AUDIO_LOG_TASK_QUEUE_LEVELS, MAIN_PATH);

#ifdef USE_WAVEFILE_IN
  /* init of audio buffer used for reading internal wav file (annoucement) */
  UTIL_AUDIO_WAVFILE_init();
#endif

  gEn_ALT = enable_alt;
  if (gEn_ALT)
  {
    UTIL_AUDIO_CAPTURE_init(UTIL_AUDIO_LOG_TASK_QUEUE_LEVELS, ALTERNATE_PATH);

    UTIL_AUDIO_RENDER_init(ALTERNATE_PATH);
  }
}

/**
  * @brief  DeInit audio capture & rendering
  * @param  None
  * @retval None
  */
void UTIL_AUDIO_deinit(void)
{
  UTIL_AUDIO_RENDER_deinit(MAIN_PATH);

  UTIL_AUDIO_CAPTURE_deinit(MAIN_PATH);

  if (gEn_ALT)
  {
   //UTIL_AUDIO_RENDER_deinit(ALTERNATE_PATH);

   //UTIL_AUDIO_CAPTURE_deinit(ALTERNATE_PATH);
    gEn_ALT = 0;
  }
}

/**
  * @brief  Start audio capture & rendering
  * @param  None
  * @retval None
  */
void UTIL_AUDIO_start(void)
{
  UTIL_AUDIO_DISABLE_IRQ();
  /* WARNING: since irq are disabled, tick is not incremented and thus timeout can't be detected during audio start
              => in case of audio start issue, we will be blocked in an infinite loop */
  UTIL_AUDIO_CAPTURE_start(MAIN_PATH);
  UTIL_AUDIO_RENDER_start(MAIN_PATH);

  if (gEn_ALT)
  {
    UTIL_AUDIO_CAPTURE_start(ALTERNATE_PATH);
    UTIL_AUDIO_RENDER_start(ALTERNATE_PATH);
  }
  UTIL_AUDIO_ENABLE_IRQ();
}



/**
  * @brief  Start audio capture & rendering
  * @param  None
  * @retval None
  */
void UTIL_AUDIO_stop(void)
{
  UTIL_AUDIO_DISABLE_IRQ();
  UTIL_AUDIO_CAPTURE_stop(MAIN_PATH);
  UTIL_AUDIO_RENDER_stop(MAIN_PATH);

  if (gEn_ALT)
  {
    UTIL_AUDIO_CAPTURE_stop(ALTERNATE_PATH);
    UTIL_AUDIO_RENDER_stop(ALTERNATE_PATH);
  }

  UTIL_AUDIO_ENABLE_IRQ();
}

/***********************************************  WAV FILE  ***********************************************/
#ifdef USE_WAVEFILE_IN
  /**
  * @brief  Init WAV file input
  * @param  None
  * @retval None
  */
audio_buffer_t Wavbuff;
void UTIL_AUDIO_WAVFILE_init(void)
{
  int32_t             error     = AUDIO_ERR_MGNT_NONE;

  if (AudioError_isOk(error))
  {
    uint16_t nbSamples = ((AC_SYSIN_WAVFILE_FS / 1000UL) * UTIL_AUDIO_N_MS_PER_INTERRUPT) / UTIL_AUDIO_N_MS_DIV;

    /* Initialize structure for wav file reading signals*/
    error =  AudioBuffer_create(&Wavbuff,
                                AC_SYSIN_WAVFILE_CHANNELS_NB,
                                AC_SYSIN_WAVFILE_FS,
                                nbSamples,
                                ABUFF_FORMAT_TIME,
                                ABUFF_FORMAT_FIXED16,
                                AC_SYSIN_WAVFILE_INTERL,
                                UTIL_AUDIO_MEMPOOL);
  }

  if (AudioError_isOk(error))
  {

  }
}

audio_buffer_t *UTIL_AUDIO_WAVFILE_CAPTURE_getAudioBuffer(void)
{
  return &Wavbuff;
}

uint8_t *UTIL_AUDIO_WAVFILE_getSampleAddr(void)
{
  return (uint8_t *)AudioBuffer_getSampleAddress(&Wavbuff, 0U, 0U);
}
#endif /* USE_WAVEFILE_IN */

/***********************************************  PDM CAPTURE  ***********************************************/

/**
  * @brief  Get microphone clk
  * @param  None
  * @retval None
  */
uint32_t UTIL_AUDIO_CAPTURE_getMicClk(void)
{
  #ifdef UTIL_AUDIO_IN_PDM_USED
  return ContextStaticAlternate.rec.pdm.clk;
  #else
  return 0UL;
  #endif
}


/**
  * @brief  Get ratio of conversion used in pdm2pcm library
  * @param  None
  * @retval None
  */
uint32_t UTIL_AUDIO_CAPTURE_getPdm2PcmLibRatio(void)
{
  #ifdef UTIL_AUDIO_IN_PDM_USED
  return ContextStaticAlternate.rec.pdm.libRatio;
  #else
  return 0UL;
  #endif
}

/***********************************************  CAPTURE ***********************************************/

/**
  * @brief  Init BSP_AUDIO_IN or other registered Audio in function. Also allocate microphone buffer
  * @param  None
  * @retval None
  */
void UTIL_AUDIO_CAPTURE_init(bool log_enable, Path_id_t path_id)
{
  UTIL_AUDIO_context_t *pContext;
  int32_t             error     = AUDIO_ERR_MGNT_NONE;
  uint32_t const      inChNb    = UTIL_AUDIO_IN_CH_NB;
  uint32_t const      maxInChNb = UTIL_AUDIO_MAX_IN_CH_NB;
  uint32_t const      inHwChNb  = UTIL_AUDIO_IN_HW_CH_NB;
  UTIL_AUDIO_params_t conf;

  conf.Device        = UTIL_AUDIO_IN_DEVICE_SELECT;
  conf.ChannelsNbr   = inHwChNb;
  conf.SampleRate    = UTIL_AUDIO_IN_FREQUENCY;
  conf.BitsPerSample = UTIL_AUDIO_IN_BIT_RESOLUTION;
  conf.Volume        = UTIL_AUDIO_IN_VOLUME;

  if (path_id == MAIN_PATH)
  {
    pContext = &ContextStatic;
  }
  else
  {
    pContext = &ContextStaticAlternate;
#ifdef UTIL_AUDIO_IN_PDM_USED //For alternate path
    //overwrite config
    conf.Device        = 0;
    conf.ChannelsNbr   = AC_SYSIN_PDM_CHANNELS_NB;
    conf.SampleRate    = AC_SYSIN_PDM_FREQ;
    conf.BitsPerSample = AUDIO_RESOLUTION_8B;
    conf.Volume        = 50;
#endif
  }

  if (UTIL_AUDIO_IN_FREQUENCY > UTIL_AUDIO_MAX_IN_FREQUENCY)
  {
    error = AUDIO_ERR_MGNT_CONFIG;
  }

  if (AudioError_isOk(error))
  {
    if ((inChNb > maxInChNb) || (inHwChNb > maxInChNb))
    {
      error = AUDIO_ERR_MGNT_CONFIG;
    }
  }

  if (AudioError_isOk(error))
  {
    pContext->rec.isMono2Stereo = ((inChNb == 2UL) && (inHwChNb == 1UL));
    pContext->rec.nbCh          = (uint8_t)inChNb;
    pContext->rec.nbHwCh        = (uint8_t)inHwChNb;
    pContext->rec.nbSamples     = ((conf.SampleRate / 1000UL) * UTIL_AUDIO_N_MS_PER_INTERRUPT) / UTIL_AUDIO_N_MS_DIV;
  }

#ifdef UTIL_AUDIO_IN_PDM_USED //For alternate path
  if (AudioError_isOk(error) && (path_id == ALTERNATE_PATH))
  {
      error = s_capture_init_pdm(pContext);
  }else
#endif
  if (AudioError_isOk(error))
  {
      error = s_capture_init_pcm(pContext);
  }


  if (AudioError_isOk(error))
  {
    UTIL_AUDIO_INOUT_IRQ_cycleMeasure_Init();
    UTIL_AUDIO_INOUT_IRQ_cycleMeasure_Reset();
    if (pContext->cbs.initIn != NULL)
    {
      if (pContext->cbs.initIn(pContext->rec.device, &conf) != BSP_ERROR_NONE)
      {
        error = AUDIO_ERR_MGNT_ERROR;
      }
    }
  }

  if (AudioError_isOk(error))
  {
    if (pContext->cbs.getStateIn != NULL)
    {
      uint32_t InState = 0UL;

      if (pContext->cbs.getStateIn(pContext->rec.device, &InState) != BSP_ERROR_NONE)
      {
        error = AUDIO_ERR_MGNT_ERROR;
      }
      if (AudioError_isOk(error))
      {
        if (InState == 0UL) /* Still reset state ==> error */
        {
          error = AUDIO_ERR_MGNT_ERROR;
        }
      }
    }
  }

  if (AudioError_isOk(error))
  {
    /* cycles measure initialization */
    UTIL_AUDIO_cycleMeasure_Init();
    UTIL_AUDIO_cycleMeasure_Reset();

    /* AudioIn task is high prio but lower than DMA; it copies samples */
    UTIL_AUDIO_createTask(log_enable);
  }
  else
  {
    UTIL_AUDIO_error();
  }
}


/**
  * @brief  free output buffer
  * @param  None
  * @retval None
  */
void UTIL_AUDIO_CAPTURE_deinit(Path_id_t path_id)
{
  if (path_id == MAIN_PATH)
  {
    AudioBuffer_deinit(&ContextStatic.rec.buffInfo);
  }
  else
  {
#ifdef UTIL_AUDIO_IN_PDM_USED //For alternate path
    AudioBuffer_deinit(&ContextStaticAlternate.rec.pdm.buffInfo);
#else
    AudioBuffer_deinit(&ContextStaticAlternate.rec.buffInfo);
#endif
  }
}


/**
  * @brief  Get offset value to reach half of ping pong buffer
  * @param  None
  * @retval None
  */
uint32_t UTIL_AUDIO_CAPTURE_getHalfBuffOffsetBytes(Path_id_t path_id)
{
  if (path_id == MAIN_PATH)
    return ContextStatic.rec.halfOffsetBytes;
  else
    return ContextStaticAlternate.rec.halfOffsetBytes;
}

/**
  * @brief  Get capture SAI buffer pointer if any
  * @param  None
  * @retval None
  */
audio_buffer_t *UTIL_AUDIO_CAPTURE_getAudioBuffer(Path_id_t path_id)
{
  if (path_id == MAIN_PATH)
  {
    return &ContextStatic.rec.buffInfo;
  }
  else
  {
#ifdef UTIL_AUDIO_IN_PDM_USED //For alternate path
    return &ContextStaticAlternate.rec.pdm.buffInfo;
#else
    return &ContextStaticAlternate.rec.buffInfo;
#endif
  }
}


/**
  * @brief  Clear buffer that will be fed by microphones.
  * @param  None
  * @retval None
  */
void UTIL_AUDIO_CAPTURE_clearBuffer(Path_id_t path_id)
{
  if (path_id == MAIN_PATH)
  {
    if (AudioError_isError(AudioBuffer_resetData(&ContextStatic.rec.buffInfo)))
    {
      UTIL_AUDIO_error();
    }
  }
  else
  {
    if (AudioError_isError(AudioBuffer_resetData(&ContextStaticAlternate.rec.buffInfo)))
    {
      UTIL_AUDIO_error();
    }
  }
}

/**
  * @brief  Starts BSP_AUDIO_IN or other registered Audio in function. Handles different cases (dfsdm & SAI)
  * @param  None
  * @retval None
  */
void UTIL_AUDIO_CAPTURE_start(Path_id_t path_id)
{
  UTIL_AUDIO_context_t *pContext;
  int32_t error     = AUDIO_ERR_MGNT_NONE;

  if (path_id == MAIN_PATH)
  {
    pContext = &ContextStatic;
  }
  else
  {
    pContext = &ContextStaticAlternate;
  }

  if (pContext->cbs.recordIn != NULL)
  {
      /* Here we are in the MDF case where we produce PCM but all channels in a single buffer.
      add ping pong and add channels , notice mics could be 1 or to ifthe flags MonoToSetreo is set
      */
    uint32_t szBuffer = pContext->rec.halfOffsetBytes * 2UL; /* 2 x for the ping pong */
    if (pContext->cbs.recordIn(pContext->rec.device, pContext->rec.pBaseAddr, szBuffer) != UTIL_AUDIO_ERROR_NONE)
    {
      error = AUDIO_ERR_MGNT_ERROR;
    }
  }
  else
  {
    //error = AUDIO_ERR_MGNT_ERROR; not an error, callback may not have been registered for application purpose
  }


  if (AudioError_isError(error))
  {
    UTIL_AUDIO_error();
  }
}




/**
  * @brief  Starts BSP_AUDIO_IN or other registered Audio in function. Handles different cases (dfsdm & SAI)
  * @param  None
  * @retval None
  */
void UTIL_AUDIO_CAPTURE_stop(Path_id_t path_id)
{
  UTIL_AUDIO_context_t *pContext;

  if (path_id == MAIN_PATH)
  {
    pContext = &ContextStatic;
  }
  else
  {
    pContext = &ContextStaticAlternate;
  }

  if (pContext->cbs.stopIn != NULL)
  {
    if (pContext->cbs.stopIn(pContext->rec.device) != UTIL_AUDIO_ERROR_NONE)
    {
      UTIL_AUDIO_error();
    }
  }
}

/**
  * @brief  Initialize the device corresponding to main and alterate path
  * @param
  * @retval None
  */
void UTIL_AUDIO_CAPTURE_setDevice(Path_id_t path_id)
{
  if (path_id == MAIN_PATH)
    ContextStatic.rec.device = MAIN_PATH;
  else
    ContextStaticAlternate.rec.device = ALTERNATE_PATH;
}


/**
 * @brief UTIL_AUDIO_CAPTURE_TxComplete_cb; called by every microphone DMA IT
 * @param  offsetSpleBytes (to manage half, full or more)
 * @param  instance (to manage type of data sent, one mic only or multiple; or other info if needed)
 * @retval None
*/
void UTIL_AUDIO_CAPTURE_TxComplete_cb(uint32_t const offsetSpleBytes, Path_id_t path_id)
{
  if (path_id == MAIN_PATH)
  {
    ContextStatic.rec.txCpltCb(offsetSpleBytes, path_id);
  }
  else
  {
    ContextStaticAlternate.rec.txCpltCb(offsetSpleBytes, path_id);
  }
}


/*********************************************   RENDER  *********************************************/

/**
  * @brief  Init BSP_AUDIO_OUT or other registered Audio in function. Also allocate output buffer
  * @param  None
  * @retval None
  */
static bool alreadyDone = false;
static bool alreadyDoneAlternate = false;

void UTIL_AUDIO_RENDER_init(Path_id_t path_id)
{
  UTIL_AUDIO_context_t *pContext;

  if (path_id == MAIN_PATH)
  {
    pContext = &ContextStatic;
  }
  else
  {
    pContext = &ContextStaticAlternate;
  }

  if ((!alreadyDone && path_id == MAIN_PATH) ||
      (!alreadyDoneAlternate && path_id == ALTERNATE_PATH))
  {
    int32_t             error      = AUDIO_ERR_MGNT_NONE;
    uint32_t const      outChNb    = UTIL_AUDIO_OUT_CH_NB;
    uint32_t const      maxOutChNb = UTIL_AUDIO_MAX_OUT_CH_NB;
    uint32_t const      outFreq    = UTIL_AUDIO_OUT_FREQUENCY;
    uint32_t const      maxOutFreq = UTIL_AUDIO_MAX_OUT_FREQUENCY;
    UTIL_AUDIO_params_t conf;

    conf.Device        = 0; /* not used */
    conf.SampleRate    = outFreq;
    conf.BitsPerSample = UTIL_AUDIO_OUT_BIT_RESOLUTION;
    conf.ChannelsNbr   = outChNb;
    conf.Volume        = UTIL_AUDIO_OUT_VOLUME;

    pContext->play.volumeRequest = 0U;

    if ((outChNb > maxOutChNb) || (outFreq > maxOutFreq))
    {
      error = AUDIO_ERR_MGNT_CONFIG;
    }

    if (AudioError_isOk(error))
    {
      pContext->play.nbSamples = (((outFreq / 1000UL) * UTIL_AUDIO_N_MS_PER_INTERRUPT) / UTIL_AUDIO_N_MS_DIV);
      if ((((outFreq / 1000UL) * UTIL_AUDIO_N_MS_PER_INTERRUPT) % UTIL_AUDIO_N_MS_DIV) != 0UL)
      {
        error = AUDIO_ERR_MGNT_CONFIG;
      }
    }

    if (AudioError_isOk(error))
    {
      audio_buffer_type_t bufferType;

      switch (UTIL_AUDIO_OUT_BIT_RESOLUTION)
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

      error = AudioBuffer_create(&pContext->play.buffInfo,
                                 (uint8_t)outChNb,
                                 outFreq,
                                 pContext->play.nbSamples,
                                 ABUFF_FORMAT_TIME,
                                 bufferType,
                                 ABUFF_FORMAT_INTERLEAVED,
                                 UTIL_AUDIO_MEMPOOL);
    }
    if (AudioError_isOk(error))
    {
      /* set pContext->play.buffInfo pData to NULL while it is not ready
      * will be set later in BSP_AUDIO_OUT_TransferComplete_CallBack()
      * or BSP_AUDIO_OUT_HalfTransfer_CallBack()
      * to first or second part of UtilAudio_OutBuff ping pong buffer*/
      error = AudioBuffer_setPdata(&pContext->play.buffInfo, NULL);
    }

    if (AudioError_isOk(error))
    {
      UTIL_AUDIO_INOUT_IRQ_cycleMeasure_Init();
      UTIL_AUDIO_INOUT_IRQ_cycleMeasure_Reset();
      if (pContext->cbs.initOut != NULL)
      {
        error = pContext->cbs.initOut(pContext->play.device, &conf);
      }
    }

    if (AudioError_isOk(error))
    {
      if (path_id == MAIN_PATH)
        alreadyDone = true;
      else
        alreadyDoneAlternate = true;
    }
    else
    {
      UTIL_AUDIO_error();
    }
  }
}


/**
  * @brief  free output buffer
  * @param  None
  * @retval None
  */
void UTIL_AUDIO_RENDER_deinit(Path_id_t path_id)
{
  if (path_id == MAIN_PATH)
  {
  AudioBuffer_deinit(&ContextStatic.play.buffInfo);
  alreadyDone = false;
  }
  else
  {
    AudioBuffer_deinit(&ContextStaticAlternate.play.buffInfo);
    alreadyDoneAlternate = false;
  }
}

/**
  * @brief  Get rendering audio_buffer_t pointer (with all meta data)
  * @param  None
  * @retval None
  */
audio_buffer_t *UTIL_AUDIO_RENDER_getAudioBuffer(Path_id_t path_id)
{
  if (path_id == MAIN_PATH)
    return &ContextStatic.play.buffInfo;
  else
    return &ContextStaticAlternate.play.buffInfo;
}

/**
  * @brief  Get rendering sample buffer base address
  * @param  None
  * @retval None
  */
void  *UTIL_AUDIO_RENDER_getBaseAddr(Path_id_t path_id)
{
  if (path_id == MAIN_PATH)
    return ContextStatic.play.pBasePtr;
  else
    return ContextStaticAlternate.play.pBasePtr;

}

/**
  * @brief  Get rendering sample buffer half address
  * @param  None
  * @retval None
  */
void  *UTIL_AUDIO_RENDER_getHalfAddr(Path_id_t path_id)
{
  if (path_id == MAIN_PATH)
    return ContextStatic.play.pHalfPtr;
  else
    return ContextStaticAlternate.play.pHalfPtr;
}

/**
  * @brief  Starts BSP_AUDIO_OUT or other registered Audio in function.
  * @param  None
  * @retval None
  */
void UTIL_AUDIO_RENDER_clearBuffer(void)
{
  uint32_t size = ContextStatic.play.nbSamples * UTIL_AUDIO_OUT_CH_NB * UTIL_AUDIO_OUT_BIT_RESOLUTION / 8UL * 2;
  if (size != 0)
  {
    memset(UtilAudio_OutBuff, 0,  size);
  }

  size = ContextStaticAlternate.play.nbSamples * UTIL_AUDIO_OUT_CH_NB * UTIL_AUDIO_OUT_BIT_RESOLUTION / 8UL * 2;
  if (size != 0)
  {
    memset(UtilAudio_OutBuffAlternate, 0,  size);
  }
}

/**
  * @brief  Starts BSP_AUDIO_OUT or other registered Audio in function.
  * @param  None
  * @retval None
  */
void UTIL_AUDIO_RENDER_start(Path_id_t path_id)
{
  UTIL_AUDIO_context_t *pContext;
  int8_t *pBuff;

  if (path_id == MAIN_PATH)
  {
    pContext = &ContextStatic;
    pBuff = UtilAudio_OutBuff;
  }
  else
  {
    pContext = &ContextStaticAlternate;
    pBuff = UtilAudio_OutBuffAlternate;
  }

  if (!pContext->play.isStarted)
  {
    uint32_t bspSamplesNb = pContext->play.nbSamples * UTIL_AUDIO_OUT_CH_NB;

    UTIL_AUDIO_RENDER_clearBuffer();
    pContext->play.pBasePtr = pBuff;
    pContext->play.pHalfPtr = pBuff + (bspSamplesNb * UTIL_AUDIO_OUT_BIT_RESOLUTION / 8UL);
    if (pContext->cbs.playOut != NULL)
    {
      int32_t error = pContext->cbs.playOut(0, (uint8_t *)pBuff, UTIL_AUDIO_OUT_BIT_RESOLUTION / 8UL * bspSamplesNb * 2UL); /* x2 for ping pong*/
      if (error != AUDIO_ERR_MGNT_NONE)
      {
        UTIL_AUDIO_error();
      }
    }
    pContext->play.isStarted = true;
  }
}

bool UTIL_AUDIO_RENDER_isStarted(Path_id_t path_id)
{
  if (path_id == MAIN_PATH)
  return ContextStatic.play.isStarted;
  else
    return ContextStaticAlternate.play.isStarted;
}

/**
* @brief  User function that is called to set all callbacks needed for audio
* @param  none
* @retval None
*/
__weak void UTIL_AUDIO_setCallbacks(Path_id_t path_id)
{
  return;
}


/**
 * @brief
 * @param  Volume linear
 * @retval None
 */
void UTIL_AUDIO_RENDER_setVolume(uint16_t const Volume)
{
  if (ContextStatic.play.isStarted)
  {
    if (ContextStatic.cbs.setVolumeOut != NULL)
    {
      ContextStatic.cbs.setVolumeOut(0, (uint32_t)Volume);
    }
  }
}

void UTIL_AUDIO_RENDER_requestVolumeChange(uint16_t const Volume)
{
  ContextStatic.play.volumeRequest = Volume;
}

void UTIL_AUDIO_RENDER_applyVolumeChange(void)
{
  static uint32_t prevVolume = 0UL;
  if (ContextStatic.play.isStarted)
  {
    if (prevVolume != ContextStatic.play.volumeRequest)
    {
      UTIL_AUDIO_RENDER_setVolume(ContextStatic.play.volumeRequest);
      prevVolume = ContextStatic.play.volumeRequest;
    }
  }
}

void UTIL_AUDIO_RENDER_stop(Path_id_t path_id)
{
  UTIL_AUDIO_context_t *pContext;

  if (path_id == MAIN_PATH)
  {
    pContext = &ContextStatic;
  }
  else
  {
    pContext = &ContextStaticAlternate;
  }

  if (pContext->play.isStarted)
  {
    if (pContext->cbs.stopOut != NULL)
    {
      pContext->cbs.stopOut(0);
    }
    pContext->play.isStarted = false;
  }
}


/**
  * @brief  Set the device
  * @param  device
  * @retval None
  */
void UTIL_AUDIO_RENDER_setDevice(uint32_t device)
{
  ContextStatic.play.device = device;
}


/*********************************************************************************************************/


__weak void UTIL_AUDIO_process(void)
{
  UTIL_AUDIO_cycleMeasure_Start();

  //UTIL_AUDIO_RENDER_start();
  UTIL_AUDIO_feed();
  //UTIL_AUDIO_RENDER_applyVolumeChange();

  UTIL_AUDIO_cycleMeasure_Stop();
}


/**
 * @brief UTIL_AUDIO_feed; weak function, by default only mic processing !
 * @param  None
 * @retval None
 */
__weak void UTIL_AUDIO_feed(void)
{
  return;
}

//#define UTIL_AUDIO_DUMP 1

#ifdef UTIL_AUDIO_DUMP

#define UTIL_AUDIO_DUMP_SEC 1U

void UTIL_AUDIO_DUMP_buffer(int8_t *buf, uint32_t size);


#if UTIL_AUDIO_DUMP > 2  // dump PDM data

#define UTIL_AUDIO_DUMP_SIZE_BYTES (UTIL_AUDIO_IN_DMA_BUFF_SIZE_BYTES * 400)
int8_t dump_audio_buff[UTIL_AUDIO_DUMP_SIZE_BYTES];


void UTIL_AUDIO_DUMP_buffer(int8_t *buf, uint32_t size)
{
  static uint32_t dump_len     = 0UL;
  static uint32_t dump_started = 0UL;
  static uint32_t wait_cnt     = 0UL;

  if (dump_started == 0UL)
  {
    wait_cnt++;
    if (wait_cnt > 1000UL) /* wait a second */
    {
      dump_started = 1UL;
    }
    return;
  }

  size = (dump_len + size > UTIL_AUDIO_DUMP_SIZE_BYTES) ? (UTIL_AUDIO_DUMP_SIZE_BYTES - dump_len) : size;
  memcpy(&dump_audio_buff[dump_len], buf, size);
  dump_len += size;
}


#else // dump PCM data
#define UTIL_AUDIO_DUMP_SAMPLES_NB (UTIL_AUDIO_DUMP_SEC*UTIL_AUDIO_IN_FREQUENCY*UTIL_AUDIO_IN_CH_NB)
int16_t dump_audio_buff[UTIL_AUDIO_DUMP_SAMPLES_NB]; // __attribute__((section(".Dump_Section")));


void UTIL_AUDIO_DUMP_buffer(int8_t *buf, uint32_t size)
{
  static uint32_t dump_len = 0UL;
  static uint32_t dump_started = 0UL;
  static uint32_t wait_cnt     = 0UL;

  if (dump_started == 0UL)
  {
    wait_cnt++;
    if (wait_cnt > 1000UL) /* wait a second */
    {
      dump_started = 1UL;
    }
    return;
  }

  size = (dump_len + size > UTIL_AUDIO_DUMP_SAMPLES_NB) ? (UTIL_AUDIO_DUMP_SAMPLES_NB - dump_len) : size;
  memcpy(&dump_audio_buff[dump_len], buf, size * sizeof(int16_t));
  dump_len += size;
}
#endif // PCM vs PDM data dumped
#else
void UTIL_AUDIO_DUMP_buffer(int8_t *buf, uint32_t size);
void UTIL_AUDIO_DUMP_buffer(int8_t *buf, uint32_t size)
{
}
#endif

/**
  * @brief This function returns nBbytes remaining in audio out DMA buffer.
           Should be redefined inside stm32_audio_boardxxname.c
  * @param None
  * @retval nb bytes
  */
__weak uint32_t UTIL_AUDIO_RENDER_getNbRemainingBytesDma(void)
{
  return 0;
}


void UTIL_AUDIO_RENDER_TxComplete_cb(void *const pData, uint8_t path_id)
{
  if (path_id == MAIN_PATH)
  {
    if (AudioError_isError(AudioBuffer_setPdata(&ContextStatic.play.buffInfo, pData)))
    {
      UTIL_AUDIO_error();
    }
    UTIL_AUDIO_triggerTask();
  }
  else
  {
    if (AudioError_isError(AudioBuffer_setPdata(&ContextStaticAlternate.play.buffInfo, pData)))
    {
      UTIL_AUDIO_error();
    }
  }
}

bool UTIL_AUDIO_CAPTURE_used(void)
{
  return (ContextStatic.cbs.initIn != NULL);
}

bool UTIL_AUDIO_RENDER_used(void)
{
  return (ContextStatic.cbs.initOut != NULL);
}



__weak void UTIL_AUDIO_USB_init(void)
{
  return;
}

__weak void UTIL_AUDIO_USB_deinit(void)
{
  return;
}

__weak void UTIL_AUDIO_USB_idle(void)
{
  return;
}

void UTIL_AUDIO_USB_REC_setChannelsId(uint8_t ch1, uint8_t ch2)
{
  ContextStatic.usbRecChId1 = ch1;
  ContextStatic.usbRecChId2 = ch2;
}

uint8_t UTIL_AUDIO_USB_REC_getChannelsId1(void)
{
  return ContextStatic.usbRecChId1;
}

uint8_t UTIL_AUDIO_USB_REC_getChannelsId2(void)
{
  return ContextStatic.usbRecChId2;
}

__weak audio_buffer_t *UTIL_AUDIO_USB_PLAY_getAudioBuffer(void)
{
  return NULL;
}

__weak void UTIL_AUDIO_USB_PLAY_clearBuffer(void)
{
  return;
}


__weak void  UTIL_AUDIO_USB_PLAY_incTick(void)
{
  return;
}

__weak audio_buffer_t *UTIL_AUDIO_USB_REC_getAudioBuffer(void)
{
  return NULL;
}

__weak void UTIL_AUDIO_USB_REC_clearBuffer(void)
{
  return;
}

__weak int32_t UTIL_AUDIO_USB_PLAY_get(audio_buffer_t const *const pBuff)
{
  return AUDIO_ERR_MGNT_NONE;
}

__weak void UTIL_AUDIO_USB_REC_send(audio_buffer_t const *const pBuff)
{
  return;
}

__weak void UTIL_AUDIO_USB_REC_mergeAndSend(audio_buffer_t const *const pIn1Buffer, audio_buffer_t const *const pIn2Buffer)
{
  return;
}

/**
* @brief  Called upon audio usb streaming start.
* @param  None
* @retval None
*/
__weak void UTIL_AUDIO_USB_PLAY_start_cb(void)
{
  return;
}

/**
* @brief  Called upon audio usb Mute.
* @param  None
* @retval None
*/
__weak void UTIL_AUDIO_USB_PLAY_mute_cb(void)
{
  return;
}

/**
* @brief  Called upon audio usb stops
* @param  none
* @retval None
*/
__weak void UTIL_AUDIO_USB_PLAY_stop_cb(void)
{
  return;
}

/**
* @brief  Called upon audio usb pause
* @param  none
* @retval None
*/
__weak void UTIL_AUDIO_USB_PLAY_pause_cb(void)
{
  return;
}

/**
* @brief  Called upon audio usb resume
* @param  none
* @retval None
*/
__weak void UTIL_AUDIO_USB_PLAY_resume_cb(void)
{
  return;
}


__weak void UTIL_AUDIO_INOUT_IRQ_cycleMeasure_Init(void)
{
  return;
}


__weak void UTIL_AUDIO_INOUT_IRQ_cycleMeasure_Reset(void)
{
  return;
}


__weak void UTIL_AUDIO_INOUT_IRQ_cycleMeasure_Start(void)
{
  return;
}


__weak void UTIL_AUDIO_INOUT_IRQ_cycleMeasure_Stop(void)
{
  return;
}


__weak void UTIL_AUDIO_cycleMeasure_Init(void)
{
  return;
}


__weak void UTIL_AUDIO_cycleMeasure_Reset(void)
{
  return;
}


__weak void UTIL_AUDIO_cycleMeasure_Start(void)
{
  return;
}


__weak void UTIL_AUDIO_cycleMeasure_Stop(void)
{
  return;
}


#ifdef UTIL_AUDIO_IN_PDM_USED
static int32_t s_capture_init_pdm(UTIL_AUDIO_context_t *const pContext)
{
  int32_t error = AUDIO_ERR_MGNT_NONE;
  #ifdef UTIL_AUDIO_IN_PDM_USED

  if (UTIL_AUDIO_IN_PDM2PCM_USED == 1U)
  {
    if (UTIL_AUDIO_IN_BIT_RESOLUTION != UTIL_AUDIO_RESOLUTION_16B)
    {
      error = AUDIO_ERR_MGNT_INIT; /* For now, only PDM2PCM library is used and it delivers 16-bit ; todo allow usage of CIC filtering or enhance PDM2PCM library configuration */
    }
    if (AudioError_isOk(error))
    {
      /* Initialize audio_buffer_t structure of microphone signals*/
      audio_buffer_type_t bufferType;

      switch (UTIL_AUDIO_OUT_BIT_RESOLUTION)
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

      error =  AudioBuffer_create(&pContext->rec.buffInfo,
                                  pContext->rec.nbCh,
                                  UTIL_AUDIO_IN_FREQUENCY,
                                  pContext->rec.nbSamples,
                                  ABUFF_FORMAT_TIME,
                                  bufferType,
                                  ABUFF_FORMAT_INTERLEAVED,
                                  UTIL_AUDIO_MEMPOOL);
    }
  }

  if (AudioError_isOk(error))
  {
    switch (AC_SYSIN_PDM_FREQ)
    {
#if USE_PDM2PCM_LIB
      case 96000UL:
        pContext->rec.pdm.libRatio = PDM_FILTER_DEC_FACTOR_32;
        pContext->rec.pdm.ratio    = 32UL;
        break;
      case 48000UL:
        pContext->rec.pdm.libRatio = PDM_FILTER_DEC_FACTOR_64;
        pContext->rec.pdm.ratio    = 64UL;
        break;
      case 16000UL:
        pContext->rec.pdm.libRatio = PDM_FILTER_DEC_FACTOR_128;
        pContext->rec.pdm.ratio    = 128UL;
        break;
      case 8000UL:
        pContext->rec.pdm.libRatio = PDM_FILTER_DEC_FACTOR_128;
        pContext->rec.pdm.ratio    = 128UL;
        break;
#endif /* USE_PDM2PCM_LIB */
      default:
        /* Not supported */
        error = AUDIO_ERR_MGNT_CONFIG;
        break;

    }
    pContext->rec.pdm.clk         = (AC_SYSIN_PDM_FREQ / 1000UL) * pContext->rec.pdm.ratio;
    pContext->rec.sizeBytes       = (2UL * (pContext->rec.nbSamples * pContext->rec.pdm.ratio) / 8UL * pContext->rec.nbHwCh);
    pContext->rec.halfOffsetBytes = pContext->rec.sizeBytes / 2UL;
  }

  if (AudioError_isOk(error))
  {
    /* initialize audio_buffer_t structure of microphone signals*/
    error =  AudioBuffer_create(&pContext->rec.pdm.buffInfo,
                                (uint8_t)pContext->rec.nbHwCh,
                                pContext->rec.pdm.clk * 1000UL,
                                (pContext->rec.pdm.clk * UTIL_AUDIO_N_MS_PER_INTERRUPT) / UTIL_AUDIO_N_MS_DIV,
                                ABUFF_FORMAT_TIME,
                                UTIL_AUDIO_IN_PDM_MSB_LSB,
                                ABUFF_FORMAT_INTERLEAVED,
                                UTIL_AUDIO_MEMPOOL);
  }

  pContext->rec.pBaseAddr = (uint8_t *)UtilAudio_MicPDMScratchBuff;
  if (UTIL_AUDIO_IN_PDM2PCM_USED == 1U)
  {
    pContext->rec.txCpltCb = s_inPdm2PcmTxCompleteCb;
  }
  else
  {
    pContext->rec.txCpltCb = s_inPdmTxCompleteCb;
  }
  #endif
  return error;
}
#endif

static int32_t s_capture_init_pcm(UTIL_AUDIO_context_t *const pContext)
{
  int32_t error = AUDIO_ERR_MGNT_NONE;
  int8_t *pBuff;

  if (pContext == &ContextStatic)
  {
    pBuff = UtilAudio_InBuff;
  }
  else
  {
    pBuff = UtilAudio_InBuffAlternate;
  }

  #ifdef UTIL_AUDIO_IN_PCM_USED
  /* Initialize audio_buffer_t structure of microphone signals*/
  audio_buffer_type_t bufferType;

  switch (UTIL_AUDIO_IN_BIT_RESOLUTION)
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

  error =  AudioBuffer_create(&pContext->rec.buffInfo,
                              pContext->rec.nbCh,
                              UTIL_AUDIO_IN_FREQUENCY,
                              pContext->rec.nbSamples,
                              ABUFF_FORMAT_TIME,
                              bufferType,
                              ABUFF_FORMAT_INTERLEAVED,
                              UTIL_AUDIO_MEMPOOL);

  if (AudioError_isOk(error))
  {

    pContext->rec.dmaResoBytes    = (uint16_t) UTIL_AUDIO_IN_DMA_BIT_RESOLUTION / 8U;
    pContext->rec.isDmaInterleaved = (uint8_t) UTIL_AUDIO_IN_IS_DMA_DATA_INTERLEAVED;
    pContext->rec.pBaseAddr = (uint8_t *)pBuff;
    uint32_t chFactor = (pContext->rec.isDmaInterleaved == 1U) ? pContext->rec.nbHwCh : 1UL;                /* to manage PCM interleaved (MDF or lineIn for instance) */
    pContext->rec.halfOffsetBytes = pContext->rec.nbSamples * pContext->rec.dmaResoBytes * chFactor;
    uint32_t chOffset = (pContext->rec.isDmaInterleaved == 1U) ? 1UL : (2UL * pContext->rec.halfOffsetBytes); /* 2 for ping pong*/
    for (uint8_t i = 0U; i < pContext->rec.nbHwCh; i++)
    {
      pContext->rec.pPcmCacheSafeAddrList[i] = (int32_t *)&pBuff[i * chOffset];
    }

    if ((pContext->rec.dmaResoBytes == 4U) && (pContext->rec.isDmaInterleaved == 0U)) /* DFSDM provides non interleaved 32 bit samples */
    {
      if (UTIL_AUDIO_IN_ALL_MIKES_PER_CB == 0U)
      {
        if (UTIL_AUDIO_IN_BIT_RESOLUTION == UTIL_AUDIO_RESOLUTION_16B)
        {
          pContext->rec.txCpltCb = s_inTxCplt_32bitDeint_to_16bitInt_waitAllMics_cb;
        }
        else
        {
          pContext->rec.txCpltCb = s_inTxCplt_32bitDeint_to_32bitInt_waitAllMics_cb;
        }
      }
      else
      {
        if (UTIL_AUDIO_IN_BIT_RESOLUTION == UTIL_AUDIO_RESOLUTION_16B)
        {
          pContext->rec.txCpltCb = s_inTxCplt_32bitDeint_to_16bitInt_cb;
        }
        else
        {
          pContext->rec.txCpltCb = s_inTxCplt_32bitDeint_to_32bitInt_cb;
        }
      }
    }
    else if ((pContext->rec.dmaResoBytes == 2U) && (pContext->rec.isDmaInterleaved == 1U)) /* MDF provides interleaved 16-bit samples */
    {
      if (pContext->rec.isMono2Stereo)
      {
        pContext->rec.txCpltCb = s_inTxCplt_pcm16bitInterleaved_m2s_cb;
      }
      else
      {
        pContext->rec.txCpltCb = s_inTxCplt_pcmbitInterleaved_cb;
      }
    }
    else if ((pContext->rec.dmaResoBytes == 4U) && (pContext->rec.isDmaInterleaved == 1U)) /* MDF provides interleaved 32-bit samples */
    {
      if (pContext->rec.isMono2Stereo)
      {
        pContext->rec.txCpltCb = s_inTxCplt_pcm32bitInterleaved_m2s_cb;
      }
      else
      {
        pContext->rec.txCpltCb = s_inTxCplt_pcmbitInterleaved_cb;
      }
    }
    else if ((pContext->rec.dmaResoBytes == 2U) && (pContext->rec.isDmaInterleaved == 0U)) /* non-interleaved 16-bit samples */
    {
      if (pContext->rec.isMono2Stereo)
      {
        pContext->rec.txCpltCb = s_inTxCplt_16bitDeInt_to_16bitInt_m2s_cb;
      }
      else
      {
        pContext->rec.txCpltCb = s_inTxCplt_16bitDeInt_to_16bitInt_cb;
      }
    }

    else /* Not supported*/
    {
      error = AUDIO_ERR_MGNT_NOT_DONE;
    }
  }
  #endif

  return error;
}


#ifdef UTIL_AUDIO_IN_PDM_USED
static void s_inPdm2PcmTxCompleteCb(uint32_t const offsetSpleBytes, int32_t const instance)
{
  uint8_t res = 0U;
  void   *pOutPdmSample = (uint16_t *)AudioBuffer_getSampleAddress(&ContextStaticAlternate.rec.pdm.buffInfo, 0U, 0U);

  #if (UTIL_AUDIO_CPU_CACHE_MAINTENANCE == 1)
  /* Invalidate Data Cache to get the updated content of the SRAM*/
  SCB_InvalidateDCache_by_Addr((uint32_t *)&UtilAudio_MicPDMScratchBuff[offsetSpleBytes], ContextStatic.rec.halfOffsetBytes);
  #endif
  if (pOutPdmSample != NULL)
  {
      memcpy(pOutPdmSample, &UtilAudio_MicPDMScratchBuff[offsetSpleBytes], ContextStaticAlternate.rec.halfOffsetBytes);
  }
  #if UTIL_AUDIO_DUMP == 3
    UTIL_AUDIO_DUMP_buffer((int8_t *)pOutPdmSample, ContextStaticAlternate.rec.halfOffsetBytes);
  #endif
    if (ContextStaticAlternate.cbs.preprocIn != NULL)
  {
    uint16_t *pOutSample    = (uint16_t *)AudioBuffer_getSampleAddress(&ContextStaticAlternate.rec.buffInfo, 0U, 0U);

      if (ContextStaticAlternate.rec.isMono2Stereo)
    {
      if (ContextStaticAlternate.rec.nbCh == 2U)
      {
        if (ContextStaticAlternate.cbs.preprocIn(instance, (uint8_t *)&UtilAudio_MicPDMScratchBuff[offsetSpleBytes], tSamplesScratch) != 0)
        {
          UTIL_AUDIO_error();
        }
        else
        {
          uint16_t *pSamples = pOutSample;
          for (uint32_t indexSmp = 0UL; indexSmp  < ContextStaticAlternate.rec.nbSamples ; indexSmp ++)
          {
            *pSamples++ = tSamplesScratch[indexSmp];
            *pSamples++ = tSamplesScratch[indexSmp];
          }
        }
      }
      else
      {
        UTIL_AUDIO_error();
      }
    }
    else
    {
      if (ContextStaticAlternate.cbs.preprocIn(instance, (uint8_t *)&UtilAudio_MicPDMScratchBuff[offsetSpleBytes], pOutSample) != 0)
      {
        UTIL_AUDIO_error();
      }
    }
    res = 1;
  }
  else
  {
    UTIL_AUDIO_error();
  }


  if (res)
  {
    #if UTIL_AUDIO_DUMP == 1
    UTIL_AUDIO_DUMP_buffer((int8_t *)pOutSample, UTIL_AUDIO_IN_CH_NB * ContextStatic.rec.nbSamples);
    #endif

    //UTIL_AUDIO_triggerTask();
  }
}

static void s_inPdmTxCompleteCb(uint32_t const offsetSpleBytes, int32_t const instance)
{
  void     *pOutPdmSample = (uint16_t *)AudioBuffer_getSampleAddress(&ContextStaticAlternate.rec.pdm.buffInfo, 0U, 0U);

  #if (UTIL_AUDIO_CPU_CACHE_MAINTENANCE == 1)
  /* Invalidate Data Cache to get the updated content of the SRAM*/
  SCB_InvalidateDCache_by_Addr((uint32_t *)&UtilAudio_MicPDMScratchBuff[offsetSpleBytes], ContextStaticAlternate.rec.halfOffsetBytes);
  #endif
  if (pOutPdmSample != NULL)
  {
    memcpy(pOutPdmSample, &UtilAudio_MicPDMScratchBuff[offsetSpleBytes], ContextStaticAlternate.rec.halfOffsetBytes);
  }
  #if UTIL_AUDIO_DUMP == 3
  UTIL_AUDIO_DUMP_buffer((int8_t *)pOutPdmSample, ContextStaticAlternate.rec.halfOffsetBytes);
  #endif

  #if UTIL_AUDIO_DUMP == 1
  UTIL_AUDIO_DUMP_buffer((int8_t *)pOutSample, UTIL_AUDIO_IN_CH_NB * ContextStaticAlternate.rec.nbSamples);
  #endif

  //UTIL_AUDIO_triggerTask();
}
#endif //UTIL_AUDIO_IN_PDM_USED


#ifdef UTIL_AUDIO_IN_PCM_USED
static void s_inTxCplt_32bitDeint_to_16bitInt_cb(uint32_t const offsetSpleBytes, int32_t const instance)
{
  #ifdef UTIL_AUDIO_DFSDM_USED
  UTIL_AUDIO_DFSDM_16bitInt_process(&ContextStatic.rec.buffInfo,
                                    ContextStatic.rec.pPcmCacheSafeAddrList,
                                    ContextStatic.rec.nbSamples,
                                    offsetSpleBytes);
  #endif

  #if UTIL_AUDIO_DUMP == 1
  int8_t *pOutSample   = (int8_t *)AudioBuffer_getSampleAddress(pBuffInfo, 0U, 0U);
  UTIL_AUDIO_DUMP_buffer(pOutSample, UTIL_AUDIO_IN_CH_NB * nbSamples);
  #endif
  //UTIL_AUDIO_triggerTask();
}


static void s_inTxCplt_32bitDeint_to_16bitInt_waitAllMics_cb(uint32_t const offsetSpleBytes, int32_t const instance)
{
  static uint8_t micTxCompleteId = 0U;
  micTxCompleteId = (micTxCompleteId + 1U) % UTIL_AUDIO_IN_CH_NB;
  if (micTxCompleteId == 0U)      /* all mics are available => we can get them */
  {
    #ifdef UTIL_AUDIO_DFSDM_USED
    UTIL_AUDIO_DFSDM_16bitInt_process(&ContextStatic.rec.buffInfo,
                                      ContextStatic.rec.pPcmCacheSafeAddrList,
                                      ContextStatic.rec.nbSamples,
                                      offsetSpleBytes);
    #endif
    #if UTIL_AUDIO_DUMP == 1
    int8_t *pOutSample   = (int8_t *)AudioBuffer_getSampleAddress(pBuffInfo, 0U, 0U);
    UTIL_AUDIO_DUMP_buffer(pOutSample, UTIL_AUDIO_IN_CH_NB * nbSamples);
    #endif
    //UTIL_AUDIO_triggerTask();
  }
}


static void s_inTxCplt_32bitDeint_to_32bitInt_cb(uint32_t const offsetSpleBytes, int32_t const instance)
{
  #ifdef UTIL_AUDIO_DFSDM_USED
  UTIL_AUDIO_DFSDM_32bitInt_process(&ContextStatic.rec.buffInfo,
                                    ContextStatic.rec.pPcmCacheSafeAddrList,
                                    ContextStatic.rec.nbSamples,
                                    offsetSpleBytes);
  #endif

  #if UTIL_AUDIO_DUMP == 1
  int8_t *pOutSample   = (int8_t *)AudioBuffer_getSampleAddress(pBuffInfo, 0U, 0U);
  UTIL_AUDIO_DUMP_buffer(pOutSample, UTIL_AUDIO_IN_CH_NB * nbSamples);
  #endif
  //UTIL_AUDIO_triggerTask();
}


static void s_inTxCplt_32bitDeint_to_32bitInt_waitAllMics_cb(uint32_t const offsetSpleBytes, int32_t const instance)
{
  static uint8_t micTxCompleteId = 0U;
  micTxCompleteId = (micTxCompleteId + 1U) % UTIL_AUDIO_IN_CH_NB;
  if (micTxCompleteId == 0U)      /* all mics are available => we can get them */
  {
    #ifdef UTIL_AUDIO_DFSDM_USED
    UTIL_AUDIO_DFSDM_32bitInt_process(&ContextStatic.rec.buffInfo,
                                      ContextStatic.rec.pPcmCacheSafeAddrList,
                                      ContextStatic.rec.nbSamples,
                                      offsetSpleBytes);
    #endif
    #if UTIL_AUDIO_DUMP == 1
    int8_t *pOutSample   = (int8_t *)AudioBuffer_getSampleAddress(pBuffInfo, 0U, 0U);
    UTIL_AUDIO_DUMP_buffer(pOutSample, UTIL_AUDIO_IN_CH_NB * nbSamples);
    #endif
    //UTIL_AUDIO_triggerTask();
  }
}


static void s_inTxCplt_pcmbitInterleaved_cb(uint32_t const offsetSpleBytes, int32_t const instance)
{
  UTIL_AUDIO_context_t *pContext;

  if (instance == 0)
  {
    pContext = &ContextStatic;
  }
  else
  {
    pContext = &ContextStaticAlternate;
  }

  uint8_t *pInSample_u8 = (uint8_t *)pContext->rec.pBaseAddr + offsetSpleBytes;

  uint8_t *pOutSample_u8 = (uint8_t *)AudioBuffer_getSampleAddress(&pContext->rec.buffInfo, 0U, 0U);
  memcpy(pOutSample_u8, pInSample_u8, pContext->rec.halfOffsetBytes);
}

static void s_inTxCplt_pcm16bitInterleaved_m2s_cb(uint32_t const offsetSpleBytes, int32_t const instance)
{
  uint32_t nbMicConnected = UTIL_AUDIO_IN_HW_CH_NB;
  uint32_t nbCh           = ContextStatic.rec.nbCh;
  uint8_t *pInSample_u8   = (uint8_t *)ContextStatic.rec.pBaseAddr + offsetSpleBytes;

  #ifdef SEND_TO_USB_DEBUG
  tud_audio_rec(pInSample_u8, (uint32_t)(ContextStatic.rec.nbSamples * ContextStatic.rec.nbCh * sizeof(int16_t)));
  #endif

  /* if isMono2Stereo Channel 0 to all other Channels */
  for (uint8_t idxChannel = 0U; idxChannel < ContextStatic.rec.nbCh; idxChannel++)
  {
    int16_t *pInSample    = (int16_t *)pInSample_u8; /* pInSample_u8 is always addr of channel 0 */
    int16_t *pOutSample   = (int16_t *)AudioBuffer_getSampleAddress(&ContextStatic.rec.buffInfo, idxChannel, 0U);

    #if (UTIL_AUDIO_CPU_CACHE_MAINTENANCE == 1)
    SCB_InvalidateDCache_by_Addr((void *)pInSample, ContextStatic.rec.nbSamples * ContextStatic.rec.dmaResoBytes);
    #endif /* UTIL_AUDIO_CPU_CACHE_MAINTENANCE */

    for (uint32_t idxPcmBuff = 0UL; idxPcmBuff < ContextStatic.rec.nbSamples; idxPcmBuff++) // half of the ping pong buffer
    {
      pOutSample[nbCh  * idxPcmBuff] = pInSample[idxPcmBuff * nbMicConnected];
    }
  }
  //UTIL_AUDIO_triggerTask();
}


static void s_inTxCplt_pcm32bitInterleaved_m2s_cb(uint32_t const offsetSpleBytes, int32_t const instance)
{
  uint32_t nbMicConnected = UTIL_AUDIO_IN_HW_CH_NB;
  uint32_t nbCh           = ContextStatic.rec.nbCh;
  uint8_t *pInSample_u8   = (uint8_t *)ContextStatic.rec.pBaseAddr + offsetSpleBytes;

  #ifdef SEND_TO_USB_DEBUG
  tud_audio_rec(pInSample_u8, (uint32_t)(ContextStatic.rec.nbSamples * ContextStatic.rec.nbCh * sizeof(int32_t)));
  #endif

  /* if isMono2Stereo Channel 0 to all other Channels */
  for (uint8_t idxChannel = 0U; idxChannel < ContextStatic.rec.nbCh; idxChannel++)
  {
    int32_t *pInSample    = (int32_t *)pInSample_u8; /* pInSample_u8 is always addr of channel 0 */
    int32_t *pOutSample   = (int32_t *)AudioBuffer_getSampleAddress(&ContextStatic.rec.buffInfo, idxChannel, 0U);

    #if (UTIL_AUDIO_CPU_CACHE_MAINTENANCE == 1)
    SCB_InvalidateDCache_by_Addr((void *)pInSample, ContextStatic.rec.nbSamples * ContextStatic.rec.dmaResoBytes);
    #endif /* UTIL_AUDIO_CPU_CACHE_MAINTENANCE */

    for (uint32_t idxPcmBuff = 0UL; idxPcmBuff < ContextStatic.rec.nbSamples; idxPcmBuff++) // half of the ping pong buffer
    {
      pOutSample[nbCh  * idxPcmBuff] = pInSample[idxPcmBuff * nbMicConnected];
    }
  }
  //UTIL_AUDIO_triggerTask();
}


static void s_inTxCplt_16bitDeInt_to_16bitInt_processCh(int16_t *pInSample, int16_t *pOutSample, uint8_t nbCh, uint8_t idxChannel, uint32_t nbSamples, uint16_t dmaResoBytes)
{

  #if (UTIL_AUDIO_CPU_CACHE_MAINTENANCE == 1)
  SCB_InvalidateDCache_by_Addr((void *)pInSample, nbSamples * dmaResoBytes);
  #endif /* UTIL_AUDIO_CPU_CACHE_MAINTENANCE */

  for (uint32_t idxPcmBuff = 0UL; idxPcmBuff < nbSamples; idxPcmBuff++) // half of the ping pong buffer
  {
    pOutSample[nbCh * idxPcmBuff] = pInSample[idxPcmBuff];
  }
}

static void s_inTxCplt_16bitDeInt_to_16bitInt_cb(uint32_t const offsetSpleBytes, int32_t const instance)
{
  uint8_t nbCh = ContextStatic.rec.nbCh;

  for (uint8_t idxChannel = 0U; idxChannel < ContextStatic.rec.nbCh; idxChannel++)
  {
    uint8_t *pChBaseAddr  = (uint8_t *) ContextStatic.rec.pPcmCacheSafeAddrList[idxChannel];
    uint8_t *pInSample_u8 = pChBaseAddr + offsetSpleBytes;
    int16_t *pOutSample   = (int16_t *)AudioBuffer_getSampleAddress(&ContextStatic.rec.buffInfo, idxChannel, 0U);

    s_inTxCplt_16bitDeInt_to_16bitInt_processCh((int16_t *)pInSample_u8, pOutSample, nbCh, idxChannel, ContextStatic.rec.nbSamples, ContextStatic.rec.dmaResoBytes);
  }
  //UTIL_AUDIO_triggerTask();
}


static void s_inTxCplt_16bitDeInt_to_16bitInt_m2s_cb(uint32_t const offsetSpleBytes, int32_t const instance)
{
  uint8_t nbCh = ContextStatic.rec.nbCh;

  for (uint8_t idxChannel = 0U; idxChannel < ContextStatic.rec.nbCh; idxChannel++)
  {
    /* if isMono2Stereo Channel 0 to all other Channels */
    uint8_t *pChBaseAddr  = (uint8_t *) ContextStatic.rec.pPcmCacheSafeAddrList[0];
    uint8_t *pInSample_u8 = pChBaseAddr + offsetSpleBytes;
    int16_t *pOutSample   = (int16_t *)AudioBuffer_getSampleAddress(&ContextStatic.rec.buffInfo, idxChannel, 0U);

    s_inTxCplt_16bitDeInt_to_16bitInt_processCh((int16_t *)pInSample_u8, pOutSample, nbCh, idxChannel, ContextStatic.rec.nbSamples, ContextStatic.rec.dmaResoBytes);
  }
  //UTIL_AUDIO_triggerTask();
}


#endif /* UTIL_AUDIO_IN_PCM_USED */
