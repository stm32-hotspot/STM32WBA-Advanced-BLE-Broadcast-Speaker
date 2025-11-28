/**
******************************************************************************
* @file    tusb_hw_config.h
* @author  MCD Application Team
* @brief   tinyusb HW implementation
*
*
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

#ifndef _TUSB_HW_CONFIG_H_
#define _TUSB_HW_CONFIG_H_

/* Includes ------------------------------------------------------------------*/

#include <stdint.h>
#include "stm32h5xx_hal.h"


#ifdef __cplusplus
extern "C" {
#endif

#define  CFG_TUSB_MCU                    OPT_MCU_STM32H5
#define  CFG_TUD_MAX_SPEED               OPT_MODE_FULL_SPEED


// VBUS Sense detection
#define OTG_FS_VBUS_SENSE     1
#define OTG_HS_VBUS_SENSE     0
#define RCC_USB2_OTG_CLK_ENABLE          __HAL_RCC_USB1_OTG_HS_CLK_ENABLE
#define USB_IT_PRIORITY                   3
#define USB_IRQx                          USB_DRD_FS_IRQn
#define USB_IT_IRQHandler                USB_DRD_FS_IRQHandler


#define  GPIO_TUSB_EXTI_IRQHandler       EXTI14_IRQHandler
#define  GPIO_TUSB_EXTI_CLK_ENABLE()     __HAL_RCC_GPIOC_CLK_ENABLE()
#define  GPIO_TUSB_EXTI_IRQn             EXTI14_IRQn
#define  GPIO_TUSB_EXTI_PIN              GPIO_PIN_14

#define  GPIO_TUSB_EXTI_PORT             GPIOC                          //  PCX free not mapped for alternate functions
#define  GPIO_TUSB_EXTI_PRIORITY         4                              // important: the priority must be lower than the USB IT
#define  GPIO_TUSB_EXTI_CALLBACK         HAL_GPIO_EXTI_Rising_Callback





#define GPIO_INIT \
/* EXTI GPIO      */  {GPIO_TUSB_EXTI_PORT, GPIO_TUSB_EXTI_PIN,GPIO_MODE_IT_FALLING}, \
/*  DM DP Pins    */  {GPIOA, (GPIO_PIN_11 | GPIO_PIN_12),GPIO_MODE_AF_PP,GPIO_AF10_USB}

#define USB_ENABLE_HW_USB() \
/* Enable USB       */ __HAL_RCC_USB_CLK_ENABLE();\
/* Enable IT        */ HAL_NVIC_SetPriority(GPIO_TUSB_EXTI_IRQn, GPIO_TUSB_EXTI_PRIORITY, 0);\
            HAL_NVIC_EnableIRQ(GPIO_TUSB_EXTI_IRQn);\
            HAL_NVIC_SetPriority(USB_IRQx, USB_IT_PRIORITY,0);\
            HAL_NVIC_EnableIRQ(USB_IRQx);\
/* Enable VBUS     */ HAL_PWREx_EnableVddUSB();

/* Keep it in case of ... but normally useless  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);*/





#define USB_OTG_FS_PERIPH_BASE             1


static __inline void configure_hw_usb_clock(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_CRSInitTypeDef RCC_CRSInitStruct = {0};

  /* Enable HSI48 */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInitStruct.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;
  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    while (1);
  }
  /*Configure the clock recovery system (CRS)**********************************/

  /*Enable CRS Clock*/
  __HAL_RCC_CRS_CLK_ENABLE();
  /* Default Synchro Signal division factor (not divided) */
  RCC_CRSInitStruct.Prescaler = RCC_CRS_SYNC_DIV1;
  /* Set the SYNCSRC[1:0] bits according to CRS_Source value */
  #ifdef RCC_CRS_SYNC_SOURCE_USB1
  RCC_CRSInitStruct.Source = RCC_CRS_SYNC_SOURCE_USB1;
  #endif
  #ifdef RCC_CRS_SYNC_SOURCE_USB
  RCC_CRSInitStruct.Source = RCC_CRS_SYNC_SOURCE_USB;
  #endif
  /* HSI48 is synchronized with USB SOF at 1KHz rate */
  RCC_CRSInitStruct.ReloadValue =  RCC_CRS_RELOADVALUE_DEFAULT;
  RCC_CRSInitStruct.ErrorLimitValue = RCC_CRS_ERRORLIMIT_DEFAULT;
  RCC_CRSInitStruct.Polarity = RCC_CRS_SYNC_POLARITY_RISING;

  /* Set the TRIM[5:0] to the default value */
  RCC_CRSInitStruct.HSI48CalibrationValue = RCC_CRS_HSI48CALIBRATION_DEFAULT;

  /* Start automatic synchronization */
  HAL_RCCEx_CRSConfig(&RCC_CRSInitStruct);
}




/* Exported typedef -----------------------------------------------------------*/

typedef void (*tusb_idle_cb)(void);


/* Exported functions ------------------------------------------------------- */


void    tusb_hw_config(void);
uint8_t tusb_hw_device_start(tusb_idle_cb timer_cb);
uint8_t tusb_hw_device_stop(void);
void    tusb_hw_enable_schedule(int8_t state);


void    GPIO_TUSB_EXTI_IRQHandler(void);
void    USB_DRD_FS_IRQHandler(void);
void    USB_IT_IRQHandler(void);



#ifdef __cplusplus
}
#endif

#endif /* _TUSB_HW_CONFIG_H_ */
