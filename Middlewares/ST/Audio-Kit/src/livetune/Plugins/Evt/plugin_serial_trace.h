/**
******************************************************************************
* @file          plugin_serial_trace.h
* @author        MCD Application Team
* @brief         console trace support
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



#ifndef plugin_serial_trace_h
#define plugin_serial_trace_h


#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdarg.h>
#include "st_base.h"
#include "st_os.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void plugin_serial_msg(ST_Handle hInstance, const char_t *pFormat, ...);

#ifdef __cplusplus
};
#endif

#endif


