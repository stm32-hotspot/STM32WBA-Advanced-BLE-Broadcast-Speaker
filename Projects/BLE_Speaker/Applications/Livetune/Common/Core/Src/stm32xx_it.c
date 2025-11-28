/**
******************************************************************************
* @file    stm32xx_it.c
* @author  MCD Application Team
* @brief   Implement remaining it handlers
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
#include "stm32xx_it.h"
#include "stm32_audio.h"

/*cstat -MISRAC2012-Rule-10.4_a third-party code*/
#if defined(USE_FREERTOS) && defined(USE_CMSISOS_V2) /* RTOS + cmsis os2 */
  #include "FreeRTOSConfig.h"
  #include "cmsis_os2.h"
  #include "FreeRTOS.h"
  #include "task.h"
#elif  defined(USE_FREERTOS)
  #include "cmsis_os.h"
  extern void osSystickHandler(void);
#endif
/*cstat +MISRAC2012-Rule-10.4_a */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*           Cortex Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
  while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Pre-fetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}





#ifndef USE_THREADS
/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}


/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}
#endif // !USE_THREADS


#ifdef USE_FREERTOS

#if (USE_CUSTOM_SYSTICK_HANDLER_IMPLEMENTATION == 1)

#define IMPLEMENT_CUSTOM_SYSTICK_HANDLER

static inline void s_SysTick_Handler(void);

/**
  * @brief  This function handles SysTick Handler according to RTOS config
  * @param  None
  * @retval None
  */
#if defined(USE_CMSISOS_V2) /* RTOS + cmsis os2 */

void xPortSysTickHandler(void);

static inline void s_SysTick_Handler(void)
{
  /* Clear overflow flag */
  SysTick->CTRL;

  if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
  {
    /* Call tick handler */
    xPortSysTickHandler();
  }
}

#else  /* RTOS + cmsis os */

static inline void s_SysTick_Handler(void)
{
  osSystickHandler();
}

#endif /* CMSIS OS v1 or v2 */

#endif /* (USE_CUSTOM_SYSTICK_HANDLER_IMPLEMENTATION == 1) */

#else  /* not USE_FREERTOS => Bare metal */

#define IMPLEMENT_CUSTOM_SYSTICK_HANDLER

static inline void s_SysTick_Handler(void);

static inline void s_SysTick_Handler(void)
{
  HAL_IncTick();
}

#endif /* USE_FREERTOS */


#ifdef IMPLEMENT_CUSTOM_SYSTICK_HANDLER

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
  s_SysTick_Handler();
  UTIL_AUDIO_USB_PLAY_incTick();
}

#endif


