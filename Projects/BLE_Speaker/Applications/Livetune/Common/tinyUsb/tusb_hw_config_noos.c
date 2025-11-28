/**
******************************************************************************
* @file    tusb_hw_config_noos.c
* @author  MCD Application Team
* @brief   tinyusb HW implementation no os
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

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
/*cstat -MISRAC2012-* tinyusb not misra compliant */
#include "tusb.h"
#include "tusb_hw_config.h"
#include "assert.h"
/*cstat +MISRAC2012-* */

/* Private defines  ---------------------------------------------------------*/
void configure_usb_clock(void);
/* Private variables ---------------------------------------------------------*/
static                    tusb_idle_cb       hIdle_cb;
static                    tusb_idle_cb       schedule_cb;
static bool               bUsbStarted;

/* Functions Definition ------------------------------------------------------*/




uint8_t tusb_hw_device_start(tusb_idle_cb cb)
{
  bUsbStarted = true;
  schedule_cb = cb;
  tusb_hw_enable_schedule(TRUE);
  return true;
}

uint8_t tusb_hw_device_stop(void)
{
  bUsbStarted = false;
  hIdle_cb = NULL;
  return true;
}


/**
  * @brief EXTI line detection callbacks
  * @param GPIO_Pin: Specifies the pins connected EXTI line
  * @retval None
  */


void GPIO_TUSB_EXTI_CALLBACK(uint16_t GPIO_Pin)
{
  if (GPIO_Pin == GPIO_TUSB_EXTI_PIN)
  {
    /* reset the GPIO */
    HAL_GPIO_WritePin(GPIO_TUSB_EXTI_PORT, GPIO_TUSB_EXTI_PIN, GPIO_PIN_RESET);
    if (hIdle_cb && bUsbStarted)
    {
      hIdle_cb();
    }
  }
}


/**
  * @brief  This function handles external lines 15 to 10 interrupt request.
  * @param  None
  * @retval None
  */
void GPIO_TUSB_EXTI_IRQHandler(void);
void GPIO_TUSB_EXTI_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(GPIO_TUSB_EXTI_PIN);
}




/**
 * @brief Overloaded from the coreInit() usb clock setup for tinyusb
 *
 */




void configure_usb_clock(void)
{
  configure_hw_usb_clock();

}

/**
* @brief Enable clock according to its address
*
*/


static void tusb_hw_config_clock_setup(GPIO_TypeDef *pPort)
{
  if (pPort == GPIOA)
  {
    __HAL_RCC_GPIOA_CLK_ENABLE();
  }
  else if (pPort == GPIOB)
  {
    __HAL_RCC_GPIOB_CLK_ENABLE();
  }

  else if  (pPort == GPIOC)
  {
    __HAL_RCC_GPIOC_CLK_ENABLE();
  }

  else if  (pPort == GPIOD)
  {
    __HAL_RCC_GPIOD_CLK_ENABLE();
  }

  else if  (pPort == GPIOE)
  {
    __HAL_RCC_GPIOE_CLK_ENABLE();
  }
  else if  (pPort == GPIOF)
  {
    __HAL_RCC_GPIOF_CLK_ENABLE();
  }
  else if  (pPort == GPIOH)
  {
    __HAL_RCC_GPIOH_CLK_ENABLE();
  }
  #ifdef GPIOI
  else if (pPort == GPIOI)
  {
    __HAL_RCC_GPIOI_CLK_ENABLE();
  }
  #endif
  #ifdef GPIOM
  else if  (pPort == GPIOM)
  {
    __HAL_RCC_GPIOM_CLK_ENABLE();
  }
  else
  {
    assert(0); //IO not mapped 
  }
  #endif

}

void tusb_hw_enable_schedule(int8_t state)
{
  if (state) { hIdle_cb = schedule_cb; }
  else { hIdle_cb = NULL; }
}


/**
 * @brief Init the USB IP HW, GPIO clock etc....
 *
 */
void tusb_hw_config(void)
{
  
  GPIO_InitTypeDef GPIO_InitStruct;
  memset(&GPIO_InitStruct, 0, sizeof(GPIO_InitStruct));

  struct
  {
    GPIO_TypeDef *port;
    uint32_t pin;
    uint32_t mode;
    uint32_t alternate;
    uint32_t pull;
    
  } const gpio_pins[] =
  {
    GPIO_INIT
  };

  for (uint8_t i = 0; i < sizeof(gpio_pins) / sizeof(gpio_pins[0]); i++)
  {
    GPIO_InitStruct.Pin       = gpio_pins[i].pin;
    GPIO_InitStruct.Mode      = gpio_pins[i].mode;
    GPIO_InitStruct.Pull      = gpio_pins[i].pull;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = gpio_pins[i].alternate;
    tusb_hw_config_clock_setup(gpio_pins[i].port);
    HAL_GPIO_Init(gpio_pins[i].port, &GPIO_InitStruct);
  }
  
  USB_ENABLE_HW_USB();
   
  
}

/**
 * @brief USB it Handler
 *
 */
void USB_IT_IRQHandler(void)
{
  tud_int_handler(BOARD_TUD_RHPORT);
  if (bUsbStarted)
  {
    __HAL_GPIO_EXTI_GENERATE_SWIT(GPIO_TUSB_EXTI_PIN);  // Triggers a lower priority ISR to simulate a Task
  }

}
