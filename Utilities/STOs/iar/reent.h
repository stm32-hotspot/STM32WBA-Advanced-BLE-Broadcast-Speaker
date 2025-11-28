/**
******************************************************************************
* @file          reent.h
* @author        MCD Application Team
* @brief         implementation task reentrance for newlib

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


#ifndef st_os_reent_h
#define st_os_reent_h

/* Includes ------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C"
{
#endif

#if defined(USE_FREERTOS) && defined(__ICCARM__)

/* Exported types ------------------------------------------------------------*/
// Adapt FreeRTOS's configUSE_NEWLIB_REENTRANT to work with the IAR C library

typedef struct _reent
{
  void *ptr;  /**< Pointer to thread-local storage area */
} _reent;

extern struct _reent *_impure_ptr;          /**< Pointer to struct _reent for current thread */

/** Frees resources associated with the struct _reent
*   weak function called from freertos
*   \param r struct _reent */

void _reclaim_reent(struct _reent *r);
/** Initializes a struct _reent */
/** \param r struct _reent */
void st_os_init_reent(struct _reent *r);


/** Initializes the struct _reent by pointer x */
#define _REENT_INIT_PTR(x) st_os_init_reent(x)

#endif // USE_FREERTOS && __ICCARM__


/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
#ifdef __cplusplus
};
#endif

#endif

