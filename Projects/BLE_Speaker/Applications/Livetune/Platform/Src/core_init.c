/**
******************************************************************************
* @file    core_init.c
* @author  MCD Application Team
* @brief   Clock, memory & cache configuration file for STM32H7
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
#include "core_init.h"
#include "main_hooks.h"
#include "main.h"
#include "platform_setup_conf.h"
#include "st_flash_storage.h"
#include "audio_persist_config.h"

/* Global variables ----------------------------------------------------------*/
int32_t gAudio_Config = 0; /* global audio configuration, defining flow, peripheral to be init and in/out frequencies */
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
DCACHE_HandleTypeDef hdcache1;

/* Private function prototypes -----------------------------------------------*/
static void MX_ICACHE_Init(void);
static void MX_DCACHE_Init(void);
static void MX_GPIO_Init(void);

void SystemHook_Init(void);
void configure_usb_clock(void);


__weak void SystemHook_Init(void)
{
}

__weak void MPU_Config_Hook(uint8_t regionNumber, uint8_t AttrbNumber);
__weak void MPU_Config_Hook(uint8_t regionNumber, uint8_t AttrbNumber)
{
}

__weak ST_Storage_Result st_flash_storage_pre_init(void);
__weak ST_Storage_Result st_flash_storage_pre_init(void)
{
  return ST_STORAGE_OK;
}



/**
* @brief  CoreInit
* @param  None
* @retval None
*/
void CoreInit(void)
{
  audio_mode_t audiomode;

  MPU_Config();

  /* Enable the CPU Cache */
  CPU_CACHE_Enable();
  /* Tick is handled by timer thru stm32h7xx_hal_timebase_tim_template.c,
  freertos uses Systick interrupt  */

  HAL_Init();
  HAL_InitTick(1);
  /* USER CODE BEGIN Init */
  // Configure the System clock
  SystemClock_Config();

  /* Specific GPIO for audio*/
  MX_GPIO_Init();

  audiomode = WBA_link_init();
  if (audiomode == MODE_MEDIA_48k)
  {
    gAudio_Config = audio_persist_get_config_index_from_name("media48kHz");
  }
  else if (audiomode == MODE_MEDIA_24k)
  {
    gAudio_Config = audio_persist_get_config_index_from_name("media24kHz");
  }
  else /* (audiomode == MODE_TELEPHONY) */
  {
    gAudio_Config = audio_persist_get_config_index_from_name("telephony24kHz");
  }

  SystemHook_Init();

}   // End CoreInit





/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
}


/**
  * @brief DCACHE Initialization Function
  * @param None
  * @retval None
  */
static void MX_DCACHE_Init(void)
{
  /* MPU should be used to disable local cacheability due to accessing Flash read-only (RO) area */
  /* Disable MPU before perloading and config update */
  LL_MPU_Disable();

  /* Define Not cacheable memory via MPU */
  LL_MPU_ConfigAttributes(LL_MPU_ATTRIBUTES_NUMBER1, LL_MPU_NOT_CACHEABLE);

  /* BaseAddress-LimitAddress configuration for RO area*/
  LL_MPU_EnableRegion(LL_MPU_REGION_NUMBER1);
  LL_MPU_ConfigRegion(LL_MPU_REGION_NUMBER1, LL_MPU_REGION_ALL_RO, LL_MPU_ATTRIBUTES_NUMBER1, 0x08FFF800UL, 0x08FFFFFFUL);

  /* Enable MPU */
  LL_MPU_Enable(LL_MPU_CTRL_HFNMI_PRIVDEF);
}


/**
  * @brief ICACHE Initialization Function
  * @param None
  * @retval None
  */
static void MX_ICACHE_Init(void)
{
  /** Enable instruction cache (default 2-ways set associative cache)
  */
  if (HAL_ICACHE_Enable() != HAL_OK)
  {
    main_hooks_error_handler();
  }
}

/**
* @brief  CPU L1-Cache enable.
* @param  None
* @retval None
*/
void CPU_CACHE_Enable(void)
{
  /* Enable the Cache */
  MX_ICACHE_Init();

  /* Attention DCACHE used only for external memories */
  MX_DCACHE_Init();
}


/* default USB clock  implementation */
__weak void configure_usb_clock(void)
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
  RCC_CRSInitStruct.Source = RCC_CRS_SYNC_SOURCE_USB;

  /* HSI48 is synchronized with USB SOF at 1KHz rate */
  RCC_CRSInitStruct.ReloadValue =  RCC_CRS_RELOADVALUE_DEFAULT;
  RCC_CRSInitStruct.ErrorLimitValue = RCC_CRS_ERRORLIMIT_DEFAULT;
  RCC_CRSInitStruct.Polarity = RCC_CRS_SYNC_POLARITY_RISING;

  /* Set the TRIM[5:0] to the default value */
  RCC_CRSInitStruct.HSI48CalibrationValue = RCC_CRS_HSI48CALIBRATION_DEFAULT;
  __HAL_RCC_USB_CLK_ENABLE();

  /* Start automatic synchronization */
  HAL_RCCEx_CRSConfig(&RCC_CRSInitStruct);
}


/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLL1_SOURCE_HSI; /* 64 MHz */
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 30;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 40; /* may be used for SPI1 : 12.288 MHz */
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1_VCIRANGE_2;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1_VCORANGE_WIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  RCC_OscInitStruct.PLL.PLLFRACN = 5900;
  /* PLL P output is at 245.76 Mhz, allowing to generate 12.288MHz audio clock from HSI (unprecise but suffiscient for annoucement) */
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    main_hooks_error_handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2
                                | RCC_CLOCKTYPE_PCLK3;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    main_hooks_error_handler();
  }

  MODIFY_REG(FLASH->ACR, 0x03<<4, 0x02<<4);

  /* USB clock setting*/
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInitStruct.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    main_hooks_error_handler();
  }
  /* Peripheral clock enable */
#ifdef USE_UART_OVER_USB
  configure_usb_clock();
#endif
  SystemCoreClockUpdate();

  /* Audio Clock Input
  PC9 as AUDIOCLK AF
  */
  __HAL_RCC_GPIOC_CLK_ENABLE();

  GPIO_InitTypeDef  gpio_init_structure;
  gpio_init_structure.Pin = GPIO_PIN_9;
  gpio_init_structure.Mode = GPIO_MODE_AF_PP;
  gpio_init_structure.Pull = GPIO_NOPULL;
  gpio_init_structure.Speed = GPIO_SPEED_FREQ_MEDIUM;
  gpio_init_structure.Alternate = GPIO_AF5_SPI1; /* AUDIOCLK AF*/
  HAL_GPIO_Init(GPIOC, &gpio_init_structure);
}


void MPU_Config(void)
{
  uint8_t regionNumber = MPU_REGION_NUMBER0;
  uint8_t AttrbNumber  = MPU_ATTRIBUTES_NUMBER0;

  #ifdef ST_USE_FLASHEXT
  /* QSPI initialized in the flash ext support */
  MPU_Attributes_InitTypeDef   attrFlashExt;
  MPU_Region_InitTypeDef       regionFlashExt;


  /* Define cacheable memory via MPU */
  attrFlashExt.Number             = AttrbNumber;
  attrFlashExt.Attributes         = MPU_WRITE_THROUGH ;
  HAL_MPU_ConfigMemoryAttributes(&attrFlashExt);

  regionFlashExt.Enable           = MPU_REGION_ENABLE;
  regionFlashExt.Number           = regionNumber; /* last one-1 */
  regionFlashExt.AttributesIndex  = AttrbNumber;
  regionFlashExt.BaseAddress      = ST_FLASH_SLOT0_BASE_ADDR;
  regionFlashExt.LimitAddress     = (ST_FLASH_SLOT2_BASE_ADDR + (ST_FLASH_SLOT2_BASE_SECTOR * ST_FLASH_SLOT2_BASE_SECTOR_SIZE * 1024U));
  regionFlashExt.AccessPermission = MPU_REGION_ALL_RW;
  regionFlashExt.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;
  regionFlashExt.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;

  HAL_MPU_ConfigRegion(&regionFlashExt);
  regionNumber++;
  AttrbNumber++;
  #endif

  HAL_MPU_Disable();
  MPU_Config_Hook(regionNumber, AttrbNumber);
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

  /*nothing in H5 */
}
