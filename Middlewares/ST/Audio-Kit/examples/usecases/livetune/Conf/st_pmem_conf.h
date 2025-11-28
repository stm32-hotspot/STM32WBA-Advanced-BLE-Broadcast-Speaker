/**
  ******************************************************************************
  * @file    st_pmem_conf.h
  * @author  MCD Application Team
  * @brief   configuration file
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
********************************************************************************
*/

#ifndef __ST_PMEM_CONF_H
#define __ST_PMEM_CONF_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include  "stm32xxx_voice_bsp.h" /* Needed for the define identifying the family
*                                   such as STM32N6, STM32H7, etc... 
*                                   wiz cubeMx this define is removed from pre 
*                                   processor define list thus we need to get it 
*                                   from stm32n6xx.h, stm32h7xx.h, etc... 
*                                   In our audio projects, these includes are 
*                                   handled by stm32xxx_voice_bsp.h.
*/

#include "irq_utils.h"

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
#define PMEM_DISABLE_IRQ disable_irq_with_cnt
#define PMEM_ENABLE_IRQ  enable_irq_with_cnt

#if defined(USE_LIVETUNE_DESIGNER) && defined(ST_USE_DEBUG)
#if defined(STM32N6) || defined(STM32H7)
#define PMEM_INSTRUMENTATION
#endif

#ifdef STM32H5
//#define PMEM_INSTRUMENTATION
#define PMEM_PERFORMANCE_INDEX 0
#endif

#endif

/* Exported structures--------------------------------------------------------*/
/* Exported variables ------------------------------------------------------- */
/* Exported functions ------------------------------------------------------- */

#ifdef __cplusplus
}
#endif

#endif /* __ST_PMEM_CONF_H */


