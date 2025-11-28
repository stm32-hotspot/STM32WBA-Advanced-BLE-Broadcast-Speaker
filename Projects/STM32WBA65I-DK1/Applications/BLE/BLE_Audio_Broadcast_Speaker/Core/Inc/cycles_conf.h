/**
  ******************************************************************************
  * @file    cycles_conf.h
  * @author  MCD Application Team
  * @brief   Header for configuration of cycles.c module
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017(-2022) STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CYCLES_CONF_H
#define __CYCLES_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
//#include  "stm32xxx_voice_bsp.h" /* Needed for the define identifying the family
/*                                   such as STM32N6, STM32H7, etc...
*                                   wiz cubeMx this define is removed from pre
*                                   processor define list thus we need to get it
*                                   from stm32n6xx.h, stm32h7xx.h, etc...
*                                   In our audio projects, these includes are
*                                   handled by stm32xxx_voice_bsp.h.
*/
#include "stm32wbaxx.h"
#include "irq_utils.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
//#define CYCLES_CNT_PARALLEL_MEASURE_MAX_NB 20U /*Default value is 10U */
#define CYCLES_CNT_TRACES_USED
//#define CYCLES_CNT_PRINTF_USED  /* CYCLES_CNT_TRACES_USED & CYCLES_CNT_PRINTF_USED are exclusive; please select one or the other*/

#define CYCLES_CNT_DISABLE_IRQ disable_irq_with_cnt
#define CYCLES_CNT_ENABLE_IRQ  enable_irq_with_cnt

/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

#ifdef __cplusplus
}
#endif

#endif /* __CYCLES_CONF_H */
