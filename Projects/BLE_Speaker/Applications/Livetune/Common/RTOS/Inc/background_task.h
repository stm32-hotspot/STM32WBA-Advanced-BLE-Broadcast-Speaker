/**
******************************************************************************
* @file    background_task.h
* @author  MCD Application Team
* @brief   background task
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


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BACKGROUND_TASK_H
#define __BACKGROUND_TASK_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "st_os_hl.h"

/* Exported macros -----------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
bool st_os_hl_task_background_create(st_os_hl_task_func_cb task_func);

#ifdef __cplusplus
}
#endif

#endif /* __BACKGROUND_TASK_H */
