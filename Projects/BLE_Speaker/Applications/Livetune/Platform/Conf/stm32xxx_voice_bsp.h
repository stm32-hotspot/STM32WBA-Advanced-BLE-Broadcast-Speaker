/**
  ******************************************************************************
  * @file    stm32xxx_voice_bsp.h
  * @author  MCD Application Team
  * @brief   Header for cube_hal.c module
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
#ifndef __STM32XXX_VOICE_BSP__
#define __STM32XXX_VOICE_BSP__

#ifdef __cplusplus
extern "C" {
#endif


/* Conf ----------------------------------------------------------------------*/
//#define USE_UART /* needed only for livetune mode : at project conf level */

#define STM32_ID 573UL
#define APP_USB_PID ((STM32_ID * 10UL) + (USBD_AUDIO_IN_FREQ / 1000UL))
#define STM32_NAME_STRING  "STM32H573I-DK"

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include "stm32h5xx.h"
//#include "stm32h573i_discovery.h"
//#include "stm32h573i_discovery_bus.h"
#include "stm32h5xx_hal.h"
#include "stm32h5xx_hal_def.h"
#include "stm32h5xx_ll_usb.h"
#include "stm32h5xx_ll_cortex.h"
#include "stm32xx_it.h"


#ifdef USE_UART
#include "stm32_usart.h"
#endif

#ifdef USE_SCREEN
#include "stm32_lcd.h"
#include "stm32h5xx_ll_fmc.h"
//#include "stm32h573i_discovery_sdram.h"
#include "stm32h573i_discovery_lcd.h"

#ifdef HAL_QSPI_MODULE_ENABLED
#define USE_QSPI
#error "NO QSPI"
#endif

#ifdef USE_TOUCHSCREEN
#include "stm32h573i_discovery_ts.h"
#endif

#endif


#if defined(USBD_AUDIO_OUT_USED) || defined(USBD_AUDIO_IN_USED)
#include "usbd_conf.h"
#define APP_HID_IN_EP HID_EPIN_ADDR
#else
#define APP_HID_IN_EP 0
#endif



/* Exported types ------------------------------------------------------------*/



/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/

#define HAL_GetCpuClockFreq()     HAL_RCC_GetSysClockFreq()


#ifdef USE_USB_FS
#define USB_IRQHandler OTG_FS_IRQHandler
#define USB_IRQn       OTG_FS_IRQn
#else
#define USB_IRQHandler OTG_HS_IRQHandler
#define USB_IRQn       OTG_HS_IRQn
#endif

#define MEMORY_SECTION_HW_BUFFERS ".noncacheable"
#define MEMORY_SECTION_DTCM       ".DTCMRAM_Section"

#define LED_AUDIO_CAPTURE_MS      LED_GREEN
#define LED_RECO_FOUND            LED_RED
#define LED_RECO_MODE             LED_GREEN
#define LED_MICROPHONES_BYPASSED  LED_RED

/* Exported functions ------------------------------------------------------- */


#ifdef __cplusplus
}
#endif

#endif //__STM32XXX_VOICE_BSP__
