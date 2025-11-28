/**
******************************************************************************
* @file    audio_chain_sysIOs_typedefs.h
* @author  MCD Application Team
* @brief   Header for audio_chain_sysIOs.c module
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
#ifndef __AUDIO_CHAIN_SYSIOS_TYPEDEFS_H
#define __AUDIO_CHAIN_SYSIOS_TYPEDEFS_H

/* Includes ------------------------------------------------------------------*/
#include "audio_chunk.h"

/* Exported constants --------------------------------------------------------*/

typedef bool (*ac_sys_ios_cb_t)(void);

typedef struct
{
  audio_chunk_t       hdle;
  audio_chunk_conf_t  conf;
  ac_sys_ios_cb_t     availabilityCb;
} ac_io_descr_t;

typedef struct
{
  uint8_t             nb;
  ac_io_descr_t      *pIos;
} ac_ios_t;

typedef struct
{
  ac_ios_t in;
  ac_ios_t out;
} ac_sys_ios_t;

/* Exported macro ------------------------------------------------------------*/
/* Exported structures--------------------------------------------------------*/
/* Exported variables ------------------------------------------------------- */
/* Exported functions ------------------------------------------------------- */


#endif /* __AUDIO_CHAIN_SYSIOS_TYPEDEFS_H */

