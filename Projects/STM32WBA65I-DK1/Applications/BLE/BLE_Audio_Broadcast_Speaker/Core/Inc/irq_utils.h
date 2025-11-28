/**
  ******************************************************************************
  * @file    irq_utils.h
  * @author  MCD Application Team
  * @brief   Header for irq_utils.c module
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
#ifndef _IRQ_UTILS_H_
#define _IRQ_UTILS_H_

#ifdef __cplusplus
extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void disable_irq_with_cnt(void);
void enable_irq_with_cnt(void);


#ifdef __cplusplus
}
#endif

#endif /* _IRQ_UTILS_H_ */
