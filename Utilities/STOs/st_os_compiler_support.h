/**
******************************************************************************
* @file          st_os_compiler_support.h
* @author        MCD Application Team
* @brief         expose os memory and threading support
                 functions. the implementation is done according to the build toolchain

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


#ifndef st_os_compiler_support_h
#define st_os_compiler_support_h

/* Includes ------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C"
{
#endif
#include <stdint.h>
#include <stdbool.h>

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
struct st_os_mem_status;
bool st_os_mem_section_info(struct st_os_mem_status **ppSysInfo);
void  st_os_compiler_init(void);

#ifdef __cplusplus
};
#endif

#endif

