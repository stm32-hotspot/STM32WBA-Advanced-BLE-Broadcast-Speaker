/**
******************************************************************************
* @file    BoardSetup.c
* @author  MCD Application Team
* @brief   Manage start stop , clock conf, meme conf, etc...
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

/* Includes ------------------------------------------------------------------*/
#include "stm32xxx_voice_bsp.h"
#include "irq_utils.h"
#include "BoardSetup.h"

#include "stm32_audio.h"
#include "stm32_usart.h"
#include "wba_link.h"

/* Global variables ----------------------------------------------------------*/

#ifdef USE_UART
  UART_HandleTypeDef G_hdleUart;
#endif

extern int32_t gAudio_Config;
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/* Functions Definition ------------------------------------------------------*/


#ifdef  USE_FULL_ASSERT
/**
* @brief  Reports the name of the source file and the source line number
*         where the assert_param error has occurred.
* @param  file: pointer to the source file name
* @param  line: assert_param error line source number
* @retval None
*/
void assert_failed(uint8_t *file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
  ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  trace_print(TRACE_OUTPUT_UART, TRACE_LVL_ERROR_FATAL, "assert failed in file %s, line %d\n", file, line);

  /* Infinite loop */
  disable_irq_with_cnt();
  while (1);
}
#endif



/**
* @brief  allow to overload the default audio configuration
*/
void AudioChainInstance_initSystem(void);
__weak void AudioChainInstance_initSystem(void)
{
}


#ifdef USE_UART_OVER_USB
#include "stm32_term_tasks.h"
#include "st_os.h"
#include "tinyusb.h"
#include "tusb.h"

static int8_t usb_rx_hook(uint8_t *pBuffer, uint16_t pszBuffer)
{
  int32_t status;
  size_t buff_size = pszBuffer;
  status = UTIL_TERM_TASK_send_data_from_isr(pBuffer, buff_size);
  return (int8_t)(status);
}

uint8_t USB_Write_Flag = 0;
int32_t usb_tx_hook(UART_HandleTypeDef *ptr, uint8_t *pBuffer, uint32_t pszBuffer);
int32_t usb_tx_hook(UART_HandleTypeDef *ptr, uint8_t *pBuffer, uint32_t pszBuffer)
{
  uint8_t status = 0;
  if (USB_Write_Flag == 0)
{
    USB_Write_Flag = 1;

    int32_t Sentsize = 0;
    int32_t remainingSize = pszBuffer;
    uint8_t *plocalBlock = pBuffer;

    NVIC_DisableIRQ(GPIO_TUSB_EXTI_IRQn);
    Sentsize = tusb_cdc_vpc0_write(plocalBlock, remainingSize, 1);

    remainingSize -= Sentsize;
    plocalBlock += Sentsize;
    NVIC_EnableIRQ(GPIO_TUSB_EXTI_IRQn);

    //HAL_NVIC_SetPendingIRQ(GPIO_TUSB_EXTI_IRQn); // force execution of the stack
    USB_Write_Flag = 0;

  }else{
    status = 1;
  }
  return (int32_t)(status);
}
#endif

extern void av_usb_init(void);
void BoardPreInit(void)
{
  #ifdef USE_UART
  UTIL_UART_t UartConf;
  UartConf.dma   = CONF_UART_DMA;
  UartConf.dma_mode_rx  = UTIL_UART_DMA_MODE_RX;
  UartConf.dma_mode_tx  = UTIL_UART_DMA_MODE_TX;
  UartConf.pHdle = &G_hdleUart;
  UTIL_UART_Init(&UartConf);
  #endif

  #ifdef USE_UART_OVER_USB
  av_usb_init();

  tusb_cdc_vpc0_set_rx_callback(usb_rx_hook);

  UTIL_UART_cbs_t util_uart_cbs;

  UTIL_UART_resetCallbacks(&util_uart_cbs);
  util_uart_cbs.transmit = usb_tx_hook;
  UTIL_UART_registerCallbacks(&util_uart_cbs);
  #endif
}




/**
* @brief  BoardInit
* @param  None
* @retval None
*/
void BoardInit(void)
{
  uint8_t enable_alternate = 0;

  UTIL_AUDIO_setCallbacks(MAIN_PATH);

  /* Init of Audio services */
  if (gAudio_Config == audio_persist_get_config_index_from_name("telephony24kHz"))
  {
    UTIL_AUDIO_setCallbacks(ALTERNATE_PATH);
    enable_alternate = 1;
  }

  UTIL_AUDIO_init(enable_alternate);
}   // End BoardInit
