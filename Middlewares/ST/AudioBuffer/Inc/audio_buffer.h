/**
******************************************************************************
* @file    audio_buffer.h
* @author  MCD Application Team
* @brief   Header for audio_buffer description and basic audio process routines.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __AUDIO_BUFFER_H
#define __AUDIO_BUFFER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdlib.h>
#include "audio_mem_mgnt.h"
#include "audio_error_mgnt.h"

/* Exported types ------------------------------------------------------------*/

typedef enum
{
  ABUFF_FORMAT_NON_INTERLEAVED,
  ABUFF_FORMAT_INTERLEAVED,
  ABUFF_FORMAT_INTERLEAVED_UNKNOWN,
  ABUFF_FORMAT_INTERLEAVED_NO_CHANGE
} audio_buffer_interleaved_t;

typedef enum
{
  ABUFF_FORMAT_TIME                = 0,   /* used as left shift for sample size computation thus must remain equal to 0 ! */
  ABUFF_FORMAT_FREQ                = 1,   /* used as left shift for sample size computation thus must remain equal to 1 ! */
  ABUFF_FORMAT_TIME_FREQ_UNKNOWN   = 2,
  ABUFF_FORMAT_TIME_FREQ_NO_CHANGE = 0xF
} audio_buffer_time_freq_t;

typedef enum
{
  ABUFF_FORMAT_PDM_LSB_FIRST,
  ABUFF_FORMAT_PDM_MSB_FIRST,
  ABUFF_FORMAT_G711_ALAW,
  ABUFF_FORMAT_G711_MULAW,
  ABUFF_FORMAT_FIXED16,
  ABUFF_FORMAT_FIXED32,
  ABUFF_FORMAT_FLOAT,
  ABUFF_FORMAT_NB,
  ABUFF_FORMAT_UNKNOWN,
  ABUFF_FORMAT_TYPE_NO_CHANGE
} audio_buffer_type_t;

/*cstat -MISRAC2012-Rule-8.12 duplicated constant convenient to define ranges*/
typedef enum
{
  // PCM samples frequencies
  ABUFF_FS_8000,
  ABUFF_FS_12000,
  ABUFF_FS_16000,
  ABUFF_FS_24000,
  ABUFF_FS_32000,
  ABUFF_FS_48000,
  ABUFF_FS_96000,
  // PDM samples frequencies
  ABUFF_FS_256000,
  ABUFF_FS_384000,
  ABUFF_FS_512000,
  ABUFF_FS_576000,
  ABUFF_FS_640000,
  ABUFF_FS_768000,
  ABUFF_FS_960000,
  ABUFF_FS_1024000,
  ABUFF_FS_1152000,
  ABUFF_FS_1280000,
  ABUFF_FS_1536000,
  ABUFF_FS_1920000,
  ABUFF_FS_2048000,
  ABUFF_FS_2304000,
  ABUFF_FS_2560000,
  ABUFF_FS_3072000,
  ABUFF_FS_3840000,
  ABUFF_FS_4096000,
  // custom samples frequencies
  ABUFF_FS_CUSTOM,
  // number of standard samples frequencies
  ABUFF_FS_NB        = ABUFF_FS_CUSTOM,
  // start/stop/number of PCM samples frequencies
  ABUFF_FS_PCM_START = ABUFF_FS_8000,
  ABUFF_FS_PCM_STOP  = ABUFF_FS_96000,
  ABUFF_FS_PCM_NB    = ABUFF_FS_PCM_STOP - ABUFF_FS_PCM_START + 1,
  // start/stop/number of PDM samples frequencies
  ABUFF_FS_PDM_START = ABUFF_FS_256000,
  ABUFF_FS_PDM_STOP  = ABUFF_FS_4096000,
  ABUFF_FS_PDM_NB    = ABUFF_FS_PDM_STOP - ABUFF_FS_PDM_START + 1,
} audio_buffer_fs_t;
/*cstat +MISRAC2012-Rule-8.12 */

enum
{
  PARAM_FS,
  PARAM_CH,
  PARAM_TIME_FREQ,
  PARAM_INTERLEAVING,
  PARAM_TYPE,
  PARAM_NB_SAMPLES,
  PARAM_DURATION,
  PARAM_NB_ELEMENTS,
  NB_PARAMS
};

typedef enum
{
  /* base enums */
  ABUFF_PARAM_FS                  = 1 << PARAM_FS,
  ABUFF_PARAM_CH                  = 1 << PARAM_CH,
  ABUFF_PARAM_TIME_FREQ           = 1 << PARAM_TIME_FREQ,
  ABUFF_PARAM_INTERLEAVING        = 1 << PARAM_INTERLEAVING,
  ABUFF_PARAM_TYPE                = 1 << PARAM_TYPE,
  ABUFF_PARAM_NB_ELEMENTS         = 1 << PARAM_NB_ELEMENTS,
  ABUFF_PARAM_DURATION            = 1 << PARAM_DURATION,

  /* combination of all base enums */
  ABUFF_PARAM_ALL                 = (1 << NB_PARAMS) - 1,

  /* enums excluding few base enums */
  ABUFF_PARAM_NOT_FS              = ABUFF_PARAM_ALL ^ ABUFF_PARAM_FS,
  ABUFF_PARAM_NOT_CH              = ABUFF_PARAM_ALL ^ ABUFF_PARAM_CH,
  ABUFF_PARAM_NOT_TIME_FREQ       = ABUFF_PARAM_ALL ^ ABUFF_PARAM_TIME_FREQ,
  ABUFF_PARAM_NOT_INTERLEAVING    = ABUFF_PARAM_ALL ^ ABUFF_PARAM_INTERLEAVING,
  ABUFF_PARAM_NOT_TYPE            = ABUFF_PARAM_ALL ^ ABUFF_PARAM_TYPE,
  ABUFF_PARAM_NOT_DURATION        = ABUFF_PARAM_ALL ^ ABUFF_PARAM_DURATION,
  ABUFF_PARAM_NOT_NB_ELEMENTS     = ABUFF_PARAM_ALL ^ ABUFF_PARAM_NB_ELEMENTS,
  ABUFF_PARAM_NOT_NB_ELEMENTS_FS  = ABUFF_PARAM_ALL ^ ABUFF_PARAM_NB_ELEMENTS  ^ ABUFF_PARAM_FS,
  ABUFF_PARAM_NOT_INTERLEAVING_CH = ABUFF_PARAM_ALL ^ ABUFF_PARAM_INTERLEAVING ^ ABUFF_PARAM_CH,

  /* not applicable enum */
  ABUFF_PARAM_NOT_APPLICABLE      = 1 << NB_PARAMS
} audio_buffer_param_list_t;

typedef struct
{
  uint32_t                   fs;                    /* Hz */
  uint32_t                   nbElements;            /* number of samples for time buffer, number of bands for spectral buffer */
  uint8_t                    nbChannels;            /* number of channels */
  audio_buffer_interleaved_t interleaved;
  audio_buffer_time_freq_t   timeFreq;
  audio_buffer_type_t        type;
  void                      *pInternalMem;
} audio_buffer_t;

/* Exported constants --------------------------------------------------------*/
#define ABUFF_CONFIG_NO_CHANGE  0U   /* for nbChannels, fs, nbElements & sampleSize */

#define ABUFF_SAMPLES_SIZE(formatType) ((((formatType) == ABUFF_FORMAT_G711_ALAW) || ((formatType) == ABUFF_FORMAT_G711_MULAW)) ? 1U : (((formatType) == ABUFF_FORMAT_FIXED16) ? 2U : 4U))

/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
int32_t                    AudioBuffer_reset(audio_buffer_t                                *const pBuff);
int32_t                    AudioBuffer_init(audio_buffer_t                                 *const pBuff, memPool_t const memPool);
int32_t                    AudioBuffer_deinit(audio_buffer_t                               *const pBuff);
int32_t                    AudioBuffer_allocate(audio_buffer_t                             *const pBuff);
int32_t                    AudioBuffer_config(audio_buffer_t                               *const pBuff, uint8_t const nbChannels, uint32_t const fs, uint32_t const nbElements, audio_buffer_time_freq_t const timeFreq, audio_buffer_type_t const type, audio_buffer_interleaved_t const interleaved);
int32_t                    AudioBuffer_configAndAllocate(audio_buffer_t                    *const pBuff, uint8_t const nbChannels, uint32_t const fs, uint32_t const nbElements, audio_buffer_time_freq_t const timeFreq, audio_buffer_type_t const type, audio_buffer_interleaved_t const interleaved);
int32_t                    AudioBuffer_create(audio_buffer_t                               *const pBuff, uint8_t const nbChannels, uint32_t const fs, uint32_t const nbElements, audio_buffer_time_freq_t const timeFreq, audio_buffer_type_t const type, audio_buffer_interleaved_t const interleaved, memPool_t const memPool);
uint32_t                   AudioBuffer_getSamplesOffset0(audio_buffer_t              const *const pBuff); /* not including *2 for complex (spectral) samples */
uint32_t                   AudioBuffer_getSamplesOffset(audio_buffer_t               const *const pBuff); /* including *2 for complex (spectral) samples */
uint32_t                   AudioBuffer_getChannelsOffset(audio_buffer_t              const *const pBuff); /* including *2 for complex (spectral) samples */
uint8_t                    AudioBuffer_getSampleSize(audio_buffer_t                  const *const pBuff);
uint8_t                    AudioBuffer_getSampleFullSize(audio_buffer_t              const *const pBuff); /* including *2 for complex (spectral) samples */
uint32_t                   AudioBuffer_getNbCells(audio_buffer_t                     const *const pBuff); /* including *2 for complex (spectral) samples */
uint32_t                   AudioBuffer_getNbSamples(audio_buffer_t                   const *const pBuff);
uint32_t                   AudioBuffer_getNbElements(audio_buffer_t                  const *const pBuff); /* doesn't include*2 for complex (spectral) samples */
int32_t                    AudioBuffer_getDurationNs(audio_buffer_t                  const *const pBuff, uint32_t *const pDuration);
uint32_t                   AudioBuffer_getBufferSize(audio_buffer_t                  const *const pBuff);
int32_t                    AudioBuffer_checkAudioBuffersCompatibility(audio_buffer_t const *const pBuff1,   audio_buffer_t const *const pBuff2, audio_buffer_param_list_t const toBeTested);
int32_t                    AudioBuffer_copyConfig(audio_buffer_t                     const *const pBuffIn,                                        audio_buffer_t *const pBuffOut);
int32_t                    AudioBuffer_merge2Stereo(audio_buffer_t                   const *const pBuffIn1, audio_buffer_t const *const pBuffIn2, audio_buffer_t *const pBuffOut);
void                      *AudioBuffer_getSampleAddress(audio_buffer_t               const *const pBuff, uint8_t const chId, uint32_t const sampleId);
int32_t                    AudioBuffer_setPdata(audio_buffer_t                             *const pBuff, void *const pData);
void                       AudioBuffer_setAllocStatsPtr(memAllocStat_t                     *const pAllocStats);

audio_buffer_interleaved_t AudioBuffer_getInterleaved(audio_buffer_t  const *const pBuff);
audio_buffer_time_freq_t   AudioBuffer_getTimeFreq(audio_buffer_t     const *const pBuff);
audio_buffer_type_t        AudioBuffer_getType(audio_buffer_t         const *const pBuff);
uint32_t                   AudioBuffer_convTypeToBit(audio_buffer_type_t     const type);
bool                       AudioBuffer_isPdmType(audio_buffer_t       const *const pBuff);
bool                       AudioBuffer_isG711Type(audio_buffer_t      const *const pBuff);
uint8_t                    AudioBuffer_getNbChannels(audio_buffer_t   const *const pBuff);
uint32_t                   AudioBuffer_getFs(audio_buffer_t           const *const pBuff);
void                      *AudioBuffer_getPdata(audio_buffer_t        const *const pBuff);
void                      *AudioBuffer_getPdataCh(audio_buffer_t      const *const pBuff, uint8_t const ch);
int32_t                    AudioBuffer_resetData(audio_buffer_t       const *const pBuff);

int32_t                    AudioBuffer_setInterleaved(audio_buffer_t *const pBuff, audio_buffer_interleaved_t const interleaved);
int32_t                    AudioBuffer_setTimeFreq(audio_buffer_t    *const pBuff, audio_buffer_time_freq_t const timeFreq);
int32_t                    AudioBuffer_setType(audio_buffer_t        *const pBuff, audio_buffer_type_t const type);
int32_t                    AudioBuffer_setNbChannels(audio_buffer_t  *const pBuff, uint8_t const nbChannels);

int32_t                    AudioBuffer_setPdataCh(audio_buffer_t                  *const pBuff, uint8_t const chId, void *const pData);
void                       AudioBuffer_updateChannelsOffsetFromPtr(audio_buffer_t *const pBuff);
uint8_t                    AudioBuffer_getSilenceFillByte(audio_buffer_t    const *const pBuff);
char const                *AudioBuffer_getTimeFreqString(audio_buffer_t     const *const pBuff);
char const                *AudioBuffer_getTypeString(audio_buffer_t         const *const pBuff);
char const                *AudioBuffer_getInterleavedString(audio_buffer_t  const *const pBuff);
int                        AudioBuffer_getDescriptionString(audio_buffer_t  const *const pBuff, char *const pString, size_t const n);
uint8_t                    AudioBuffer_getRealComplexFactor(audio_buffer_time_freq_t const timeFreq);


#ifdef __cplusplus
}
#endif

#endif /* __AUDIO_BUFFER_H */
