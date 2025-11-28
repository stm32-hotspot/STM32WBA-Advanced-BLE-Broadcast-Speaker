/**
  ******************************************************************************
  * @file    speexAlloc.c
  * @author  MCD Application Team
  * @brief   speex alloc routines wrapper
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

#ifndef __SPEEX_ALLOC_H
#define __SPEEX_ALLOC_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "speex_aec/os_support_custom.h"
#include "audio_mem_mgnt.h"

/* Global variables ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Functions Definition ------------------------------------------------------*/
#if (defined(OVERRIDE_SPEEX_ALLOC) && !defined(OVERRIDE_SPEEX_FREE)) || (!defined(OVERRIDE_SPEEX_ALLOC) && defined(OVERRIDE_SPEEX_FREE))
#error "OVERRIDE_SPEEX_ALLOC and OVERRIDE_SPEEX_FREE must be defined both or none"
#endif

#ifdef OVERRIDE_SPEEX_ALLOC
void *speex_alloc(int size);
#endif

#ifdef OVERRIDE_SPEEX_FREE
void speex_free(void *ptr);
#endif

#if defined(OVERRIDE_SPEEX_ALLOC) && defined(OVERRIDE_SPEEX_FREE)
void speex_setMemPool(memPool_t const memPool);
#endif

#ifdef __cplusplus
}
#endif

#endif // __SPEEX_ALLOC_H
