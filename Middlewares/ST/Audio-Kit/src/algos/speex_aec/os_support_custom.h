/**
  ******************************************************************************
  * @file    os_support_custom.h
  * @author  MCD Application Team
  * @brief   Speex third party OS support redefinition (alloc routines)
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2019(-2022) STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __OS_SUPPORT_CUSTOM_H
#define __OS_SUPPORT_CUSTOM_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define OVERRIDE_SPEEX_ALLOC
//#define OVERRIDE_SPEEX_ALLOC_SCRATCH
//#define OVERRIDE_SPEEX_REALLOC
#define OVERRIDE_SPEEX_FREE
//#define OVERRIDE_SPEEX_FREE_SCRATCH

/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
/** Speex wrapper for calloc. To do your own dynamic allocation, all you need to do is replace this function, speex_realloc and speex_free
    NOTE: speex_alloc needs to CLEAR THE MEMORY */
#ifdef OVERRIDE_SPEEX_ALLOC
void *speex_alloc(int size);
#endif

/** Same as speex_alloc, except that the area is only needed inside a Speex call (might cause problem with wideband though) */
#ifdef OVERRIDE_SPEEX_ALLOC_SCRATCH
void *speex_alloc_scratch(int size);
#endif

/** Speex wrapper for realloc. To do your own dynamic allocation, all you need to do is replace this function, speex_alloc and speex_free */
#ifdef OVERRIDE_SPEEX_REALLOC
void *speex_realloc(void *ptr, int size);
#endif

/** Speex wrapper for calloc. To do your own dynamic allocation, all you need to do is replace this function, speex_realloc and speex_alloc */
#ifdef OVERRIDE_SPEEX_FREE
void speex_free(void *ptr);
#endif

/** Same as speex_free, except that the area is only needed inside a Speex call (might cause problem with wideband though) */
#ifdef OVERRIDE_SPEEX_FREE_SCRATCH
void speex_free_scratch(void *ptr);
#endif


#ifdef __cplusplus
}
#endif

#endif /* __OS_SUPPORT_CUSTOM_H */

