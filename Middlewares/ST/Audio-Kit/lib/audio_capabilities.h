/**
******************************************************************************
* @file    audio_capabilities.h
* @author  MCD Application Team
* @brief   Header for audio_capabilities.c module
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
#ifndef __AUDIO_CAPBILITIES_H
#define __AUDIO_CAPBILITIES_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "audio_buffer.h"


/* Exported constants --------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/

typedef enum
{
  AUDIO_CAPABILITY_INTERLEAVING_NO   = 1UL << ABUFF_FORMAT_NON_INTERLEAVED,
  AUDIO_CAPABILITY_INTERLEAVING_YES  = 1UL << ABUFF_FORMAT_INTERLEAVED,
  AUDIO_CAPABILITY_INTERLEAVING_BOTH = AUDIO_CAPABILITY_INTERLEAVING_YES | AUDIO_CAPABILITY_INTERLEAVING_NO
} audio_capability_interleaving_t;

typedef enum
{
  AUDIO_CAPABILITY_TIME      = 1UL << ABUFF_FORMAT_TIME,
  AUDIO_CAPABILITY_FREQ      = 1UL << ABUFF_FORMAT_FREQ,
  AUDIO_CAPABILITY_TIME_FREQ = AUDIO_CAPABILITY_TIME | AUDIO_CAPABILITY_FREQ
} audio_capability_time_freq_t;

typedef enum
{
  AUDIO_CAPABILITY_TYPE_PDM_LSB_FIRST              = 1UL << ABUFF_FORMAT_PDM_LSB_FIRST,
  AUDIO_CAPABILITY_TYPE_PDM_MSB_FIRST              = 1UL << ABUFF_FORMAT_PDM_MSB_FIRST,
  AUDIO_CAPABILITY_TYPE_G711_ALAW                  = 1UL << ABUFF_FORMAT_G711_ALAW,
  AUDIO_CAPABILITY_TYPE_G711_MULAW                 = 1UL << ABUFF_FORMAT_G711_MULAW,
  AUDIO_CAPABILITY_TYPE_FIXED16                    = 1UL << ABUFF_FORMAT_FIXED16,
  AUDIO_CAPABILITY_TYPE_FIXED32                    = 1UL << ABUFF_FORMAT_FIXED32,
  AUDIO_CAPABILITY_TYPE_FLOAT                      = 1UL << ABUFF_FORMAT_FLOAT,
  AUDIO_CAPABILITY_TYPE_PDM                        = AUDIO_CAPABILITY_TYPE_PDM_LSB_FIRST              | AUDIO_CAPABILITY_TYPE_PDM_MSB_FIRST,
  AUDIO_CAPABILITY_TYPE_G711                       = AUDIO_CAPABILITY_TYPE_G711_ALAW                  | AUDIO_CAPABILITY_TYPE_G711_MULAW,
  AUDIO_CAPABILITY_TYPE_FIXED16_FIXED32            = AUDIO_CAPABILITY_TYPE_FIXED16                    | AUDIO_CAPABILITY_TYPE_FIXED32,
  AUDIO_CAPABILITY_TYPE_FIXED16_FLOAT              = AUDIO_CAPABILITY_TYPE_FIXED16                    | AUDIO_CAPABILITY_TYPE_FLOAT,
  AUDIO_CAPABILITY_TYPE_FIXED32_FLOAT              = AUDIO_CAPABILITY_TYPE_FIXED32                    | AUDIO_CAPABILITY_TYPE_FLOAT,
  AUDIO_CAPABILITY_TYPE_FIXED16_FIXED32_FLOAT      = AUDIO_CAPABILITY_TYPE_FIXED16_FIXED32            | AUDIO_CAPABILITY_TYPE_FLOAT,
  AUDIO_CAPABILITY_TYPE_FIXED16_FIXED32_FLOAT_G711 = AUDIO_CAPABILITY_TYPE_FIXED16_FIXED32_FLOAT      | AUDIO_CAPABILITY_TYPE_G711,
  AUDIO_CAPABILITY_TYPE_FIXED16_FIXED32_FLOAT_PDM  = AUDIO_CAPABILITY_TYPE_FIXED16_FIXED32_FLOAT      | AUDIO_CAPABILITY_TYPE_PDM,
  AUDIO_CAPABILITY_TYPE_ALL                        = AUDIO_CAPABILITY_TYPE_FIXED16_FIXED32_FLOAT_G711 | AUDIO_CAPABILITY_TYPE_PDM,
} audio_capability_type_t;

typedef enum
{
  /* PCM freqs*/
  AUDIO_CAPABILITY_FS_8000                 = 1UL << ABUFF_FS_8000,
  AUDIO_CAPABILITY_FS_12000                = 1UL << ABUFF_FS_12000,
  AUDIO_CAPABILITY_FS_16000                = 1UL << ABUFF_FS_16000,
  AUDIO_CAPABILITY_FS_24000                = 1UL << ABUFF_FS_24000,
  AUDIO_CAPABILITY_FS_32000                = 1UL << ABUFF_FS_32000,
  AUDIO_CAPABILITY_FS_48000                = 1UL << ABUFF_FS_48000,
  AUDIO_CAPABILITY_FS_96000                = 1UL << ABUFF_FS_96000,
  /* PDM freqs*/
  AUDIO_CAPABILITY_FS_256000               = 1UL << ABUFF_FS_256000,
  AUDIO_CAPABILITY_FS_384000               = 1UL << ABUFF_FS_384000,
  AUDIO_CAPABILITY_FS_512000               = 1UL << ABUFF_FS_512000,
  AUDIO_CAPABILITY_FS_576000               = 1UL << ABUFF_FS_576000,
  AUDIO_CAPABILITY_FS_640000               = 1UL << ABUFF_FS_640000,
  AUDIO_CAPABILITY_FS_768000               = 1UL << ABUFF_FS_768000,
  AUDIO_CAPABILITY_FS_960000               = 1UL << ABUFF_FS_960000,
  AUDIO_CAPABILITY_FS_1024000              = 1UL << ABUFF_FS_1024000,
  AUDIO_CAPABILITY_FS_1152000              = 1UL << ABUFF_FS_1152000,
  AUDIO_CAPABILITY_FS_1280000              = 1UL << ABUFF_FS_1280000,
  AUDIO_CAPABILITY_FS_1536000              = 1UL << ABUFF_FS_1536000,
  AUDIO_CAPABILITY_FS_1920000              = 1UL << ABUFF_FS_1920000,
  AUDIO_CAPABILITY_FS_2048000              = 1UL << ABUFF_FS_2048000,
  AUDIO_CAPABILITY_FS_2304000              = 1UL << ABUFF_FS_2304000,
  AUDIO_CAPABILITY_FS_2560000              = 1UL << ABUFF_FS_2560000,
  AUDIO_CAPABILITY_FS_3072000              = 1UL << ABUFF_FS_3072000,
  AUDIO_CAPABILITY_FS_3840000              = 1UL << ABUFF_FS_3840000,
  AUDIO_CAPABILITY_FS_4096000              = 1UL << ABUFF_FS_4096000,

  AUDIO_CAPABILITY_FS_CUSTOM               = 1UL << ABUFF_FS_CUSTOM,   /* To allow custom defined sampling frequencies (1000, 2000, etc...) */

  /* Useful capability masks */
  AUDIO_CAPABILITY_FS_8000_16000           = AUDIO_CAPABILITY_FS_8000  | AUDIO_CAPABILITY_FS_16000,
  AUDIO_CAPABILITY_FS_8000_12000_16000     = AUDIO_CAPABILITY_FS_8000  | AUDIO_CAPABILITY_FS_12000 | AUDIO_CAPABILITY_FS_16000,
  AUDIO_CAPABILITY_FS_8000_48000           = AUDIO_CAPABILITY_FS_8000  | AUDIO_CAPABILITY_FS_48000,
  AUDIO_CAPABILITY_FS_16000_48000          = AUDIO_CAPABILITY_FS_16000 | AUDIO_CAPABILITY_FS_48000,
  AUDIO_CAPABILITY_FS_8000_16000_48000     = AUDIO_CAPABILITY_FS_8000  | AUDIO_CAPABILITY_FS_16000 | AUDIO_CAPABILITY_FS_48000,

  AUDIO_CAPABILITY_FS_PCM_ALL              = ((1UL << ABUFF_FS_PCM_NB) - 1UL) << ABUFF_FS_PCM_START,
  AUDIO_CAPABILITY_FS_PDM_ALL              = ((1UL << ABUFF_FS_PDM_NB) - 1UL) << ABUFF_FS_PDM_START,
  AUDIO_CAPABILITY_FS_ALL                  = AUDIO_CAPABILITY_FS_PCM_ALL | AUDIO_CAPABILITY_FS_PDM_ALL,

  /* Useful capability masks for custom sampling frequencies */
  AUDIO_CAPABILITY_FS_PCM_ALL_AND_CUSTOM   = AUDIO_CAPABILITY_FS_PCM_ALL | AUDIO_CAPABILITY_FS_CUSTOM,
  AUDIO_CAPABILITY_FS_ALL_AND_CUSTOM       = AUDIO_CAPABILITY_FS_ALL     | AUDIO_CAPABILITY_FS_CUSTOM,
} audio_capability_fs_t;

typedef struct
{
  audio_buffer_param_list_t in;
  audio_buffer_param_list_t out;
  audio_buffer_param_list_t in_out;
} audio_capability_chunk_consistency_t;

typedef enum
{
  _1CH = 1,      // enum starting at 1 needed for acChunk config through liveTune
  _2CH,
  _3CH,
  _4CH,
  _5CH,
  _6CH,
  _7CH,
  _8CH,
} audio_channel_t;

typedef enum
{
  AUDIO_CAPABILITY_1CH                             = 1UL << _1CH,
  AUDIO_CAPABILITY_2CH                             = 1UL << _2CH,
  AUDIO_CAPABILITY_3CH                             = 1UL << _3CH,
  AUDIO_CAPABILITY_4CH                             = 1UL << _4CH,
  AUDIO_CAPABILITY_5CH                             = 1UL << _5CH,
  AUDIO_CAPABILITY_6CH                             = 1UL << _6CH,
  AUDIO_CAPABILITY_7CH                             = 1UL << _7CH,
  AUDIO_CAPABILITY_8CH                             = 1UL << _8CH,
  AUDIO_CAPABILITY_1CH_2CH                         = AUDIO_CAPABILITY_1CH | AUDIO_CAPABILITY_2CH,
  AUDIO_CAPABILITY_1CH_2CH_3CH                     = AUDIO_CAPABILITY_1CH_2CH | AUDIO_CAPABILITY_3CH,
  AUDIO_CAPABILITY_1CH_2CH_3CH_4CH                 = AUDIO_CAPABILITY_1CH_2CH_3CH | AUDIO_CAPABILITY_4CH,
  AUDIO_CAPABILITY_1CH_2CH_3CH_4CH_5CH             = AUDIO_CAPABILITY_1CH_2CH_3CH_4CH | AUDIO_CAPABILITY_5CH,
  AUDIO_CAPABILITY_1CH_2CH_3CH_4CH_5CH_6CH         = AUDIO_CAPABILITY_1CH_2CH_3CH_4CH_5CH | AUDIO_CAPABILITY_6CH,
  AUDIO_CAPABILITY_1CH_2CH_3CH_4CH_5CH_6CH_7CH     = AUDIO_CAPABILITY_1CH_2CH_3CH_4CH_5CH_6CH | AUDIO_CAPABILITY_7CH,
  AUDIO_CAPABILITY_1CH_2CH_3CH_4CH_5CH_6CH_7CH_8CH = AUDIO_CAPABILITY_1CH_2CH_3CH_4CH_5CH_6CH_7CH | AUDIO_CAPABILITY_8CH,
  AUDIO_CAPABILITY_CH_ALL                          = AUDIO_CAPABILITY_1CH_2CH_3CH_4CH_5CH_6CH_7CH_8CH,
} audio_capability_channel_t;

typedef enum
{
  CHUNK_NONE,
  CHUNK_ONE,
  CHUNK_TWO,
  CHUNK_THREE,
  CHUNK_FOUR,
  CHUNK_FIVE,
  CHUNK_SIX,
  CHUNK_SEVEN,
  CHUNK_EIGHT,
  CHUNK_NINE,
  CHUNK_TEN,
  CHUNK_MULTIPLE,
} audio_chunk_nb_t;

typedef enum
{
  AUDIO_CAPABILITY_CHUNK_NONE                                             = 1UL << CHUNK_NONE,
  AUDIO_CAPABILITY_CHUNK_ONE                                              = 1UL << CHUNK_ONE,
  AUDIO_CAPABILITY_CHUNK_TWO                                              = 1UL << CHUNK_TWO,
  AUDIO_CAPABILITY_CHUNK_THREE                                            = 1UL << CHUNK_THREE,
  AUDIO_CAPABILITY_CHUNK_FOUR                                             = 1UL << CHUNK_FOUR,
  AUDIO_CAPABILITY_CHUNK_FIVE                                             = 1UL << CHUNK_FIVE,
  AUDIO_CAPABILITY_CHUNK_SIX                                              = 1UL << CHUNK_SIX,
  AUDIO_CAPABILITY_CHUNK_SEVEN                                            = 1UL << CHUNK_SEVEN,
  AUDIO_CAPABILITY_CHUNK_EIGHT                                            = 1UL << CHUNK_EIGHT,
  AUDIO_CAPABILITY_CHUNK_NINE                                             = 1UL << CHUNK_NINE,
  AUDIO_CAPABILITY_CHUNK_TEN                                              = 1UL << CHUNK_TEN,
  AUDIO_CAPABILITY_CHUNK_MULTIPLE                                         = 1UL << CHUNK_MULTIPLE,
  AUDIO_CAPABILITY_CHUNK_NONE_ONE                                         = AUDIO_CAPABILITY_CHUNK_NONE | AUDIO_CAPABILITY_CHUNK_ONE,
  AUDIO_CAPABILITY_CHUNK_ONE_TWO                                          = AUDIO_CAPABILITY_CHUNK_ONE | AUDIO_CAPABILITY_CHUNK_TWO,
  AUDIO_CAPABILITY_CHUNK_ONE_TWO_THREE                                    = AUDIO_CAPABILITY_CHUNK_ONE_TWO | AUDIO_CAPABILITY_CHUNK_THREE,
  AUDIO_CAPABILITY_CHUNK_ONE_TWO_THREE_FOUR                               = AUDIO_CAPABILITY_CHUNK_ONE_TWO_THREE | AUDIO_CAPABILITY_CHUNK_FOUR,
  AUDIO_CAPABILITY_CHUNK_ONE_TWO_THREE_FOUR_FIVE                          = AUDIO_CAPABILITY_CHUNK_ONE_TWO_THREE_FOUR | AUDIO_CAPABILITY_CHUNK_FIVE,
  AUDIO_CAPABILITY_CHUNK_ONE_TWO_THREE_FOUR_FIVE_SIX                      = AUDIO_CAPABILITY_CHUNK_ONE_TWO_THREE_FOUR_FIVE | AUDIO_CAPABILITY_CHUNK_SIX,
  AUDIO_CAPABILITY_CHUNK_ONE_TWO_THREE_FOUR_FIVE_SIX_SEVEN                = AUDIO_CAPABILITY_CHUNK_ONE_TWO_THREE_FOUR_FIVE_SIX | AUDIO_CAPABILITY_CHUNK_SEVEN,
  AUDIO_CAPABILITY_CHUNK_ONE_TWO_THREE_FOUR_FIVE_SIX_SEVEN_EIGHT          = AUDIO_CAPABILITY_CHUNK_ONE_TWO_THREE_FOUR_FIVE_SIX_SEVEN | AUDIO_CAPABILITY_CHUNK_EIGHT,
  AUDIO_CAPABILITY_CHUNK_ONE_TWO_THREE_FOUR_FIVE_SIX_SEVEN_EIGHT_NINE     = AUDIO_CAPABILITY_CHUNK_ONE_TWO_THREE_FOUR_FIVE_SIX_SEVEN | AUDIO_CAPABILITY_CHUNK_EIGHT | AUDIO_CAPABILITY_CHUNK_NINE,
  AUDIO_CAPABILITY_CHUNK_ONE_TWO_THREE_FOUR_FIVE_SIX_SEVEN_EIGHT_NINE_TEN = AUDIO_CAPABILITY_CHUNK_ONE_TWO_THREE_FOUR_FIVE_SIX_SEVEN | AUDIO_CAPABILITY_CHUNK_EIGHT | AUDIO_CAPABILITY_CHUNK_NINE | AUDIO_CAPABILITY_CHUNK_TEN,
  AUDIO_CAPABILITY_CHUNK_ONE_MULTIPLE                                     = AUDIO_CAPABILITY_CHUNK_ONE | AUDIO_CAPABILITY_CHUNK_MULTIPLE,
} audio_capability_chunk_t;

enum
{
  PRIO_LEVEL_LOW,
  PRIO_LEVEL_NORMAL,
};

typedef enum
{
  AUDIO_CAPABILITY_PROCESS_PRIO_LEVEL_LOW    = 1UL << PRIO_LEVEL_LOW,
  AUDIO_CAPABILITY_PROCESS_PRIO_LEVEL_NORMAL = 1UL << PRIO_LEVEL_NORMAL,
  AUDIO_CAPABILITY_PROCESS_PRIO_LEVEL_ALL    = AUDIO_CAPABILITY_PROCESS_PRIO_LEVEL_LOW | AUDIO_CAPABILITY_PROCESS_PRIO_LEVEL_NORMAL,
} audio_capability_prio_level_t;

typedef struct
{
  audio_capability_chunk_t             nb;
  audio_capability_channel_t           nbChan;
  audio_capability_fs_t                fs;
  audio_capability_type_t              type;
  audio_capability_time_freq_t         time_freq;
  audio_capability_interleaving_t      interleaving;
  char                                *pPinNames;       /* actif if non null, One name by pindef separated by \0 and terminated by \0\0 */
  char const                          *pDescs;          /* actif if non null, One description by pindef separated by \0 and terminated by \0\0 */
} audio_capabilities_ios_descr_t;

typedef struct
{
  uint8_t                              icon;      /* 0 = default AC icon */
  uint8_t                              flags;     /* used for example by generator */
  char const                          *pAlgoDesc; /* no description if null*/
  char const                          *pAlgoHelp; /* no help if null*/
} audio_capabilities_misc_t;


/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */


#ifdef __cplusplus
}
#endif

#endif  /* __AUDIO_CAPBILITIES_H */
