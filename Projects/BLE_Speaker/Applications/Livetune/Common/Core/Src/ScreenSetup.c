/**
******************************************************************************
* @file    ScreenSetup.c
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
#include "stm32xxx_voice_bsp.h"

#ifdef USE_SCREEN
#include "ScreenSetup.h"
#include "st_os_mem_conf.h"

/* Global variables ----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/*cstat -MISRAC2012-* st_logo not misra compliant */
#if LCD_DEFAULT_WIDTH == 480
  #include "../logo/st_logo_74_54.h"
#elif LCD_DEFAULT_WIDTH == 240
  #include "../logo/st_logo_48_34.h"
#else
  #include "../logo/st_logo_150_106.h"
#endif
/*cstat +MISRAC2012-* */

/** aligned left **/
#define ST_LOGO_X ((uint32_t)LCD_DEFAULT_WIDTH - (ST_LOGO_WIDTH + (2UL * LOGOS_MARGIN)))
#define ST_LOGO_Y (ST_LOGO_Y_OFFSET + (2UL * LOGOS_MARGIN))

/* Private macros ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static uint8_t G_LcdInitDone = 0;

/* Private function prototypes -----------------------------------------------*/
static void s_manageGlitch(void);

/* Functions Definition ------------------------------------------------------*/

/**
* @brief  Return true if LCD init was done.
* @param  None
* @retval bool true if LCD init was done
*/
uint8_t LCDIsInitDone(void)
{
  return G_LcdInitDone;
}

/**
* @brief  Initialize LCD and required peripheral for extFlash support.
* @param  None
* @retval None
*/
void LCDInit(void)
{
  /* Initialize the extFlash if needed */
  LCDInitXspi();

  /* Initialize the LCD */
  int32_t lcd_status = BSP_LCD_Init(0, LCD_ORIENTATION_LANDSCAPE);
  while (lcd_status != BSP_ERROR_NONE);

  #ifdef STM32N6
  lcd_status = BSP_LCD_SetLayerAddress(0, 0, RAMEXT_LCD_LAYER_BASE);
  while (lcd_status != BSP_ERROR_NONE);
  #endif

  /* Enable the display */
  BSP_LCD_DisplayOn(0);
  s_manageGlitch();

  UTIL_LCD_SetFuncDriver(&LCD_Driver);

  UTIL_LCD_SetFont(&UTIL_LCD_DEFAULT_FONT);

  /* Clear LCD */
  UTIL_LCD_Clear(UTIL_LCD_COLOR_WHITE);

  /* Setting Title */
  //  UTIL_LCD_SetFont(&LCD_TXT_FONT_DEFAULT);
  UTIL_LCD_SetBackColor(UTIL_LCD_COLOR_WHITE);

  UTIL_LCD_DrawBitmap(ST_LOGO_X, ST_LOGO_Y, (uint8_t *)st_logo_bmp);

  LCDDisplayHeader();
  G_LcdInitDone = 1;
}

/**
* @brief  Initialize extFlash support.
* @param  None
* @retval None
*/
__weak void LCDInitXspi(void)
{
  /* Empty by default => no screen data in extFlash*/
}
__weak void LCDDisplayHeader(void)
{
  UTIL_LCD_SetFont(&LCD_TXT_FONT_INFO);
  UTIL_LCD_SetTextColor(LCD_TXT_COLOR_INFO);
  UTIL_LCD_DisplayStringAt(LOGOS_MARGIN, Y_POS_VERSION_ID, (uint8_t *)APP_VERSION, LEFT_MODE);
}

static void s_manageGlitch(void)
{
  #ifdef STM32H7
  /* Following line is necessary to avoid glitch on the LCD
  -- Reason is:
  -- THE CM7 is doing speculation. It does it well over small address range
  -- However it fails when range is too big;
  -- here the issue is with SDRAM, since the BSP uses it for bank2,
  -- the SDRAM bank1 gets enabled; field MBKEN of register BCR1
  -- However since we don't use it, the MPU doesn't configure this address range
  -- Another fix should be to configure MPU properly
  -- for that matter the AN4861 explains what to do especially on
  -- p.42 to p.52. For our case p.48/52 contains most useful information */
  __FMC_NORSRAM_DISABLE(FMC_NORSRAM_DEVICE, FMC_SDRAM_BANK1);
  #endif
}
#endif  // USE_SCREEN

#ifdef USE_QSPI

HAL_StatusTypeDef MX_SDRAM_Init(SDRAM_HandleTypeDef *hSdram)
{
  FMC_SDRAM_TimingTypeDef sdram_timing;

  /* SDRAM device configuration */
  hSdram->Instance = FMC_SDRAM_DEVICE;

  /* SDRAM handle configuration */
  hSdram->Init.SDBank             = FMC_SDRAM_BANK2;
  hSdram->Init.ColumnBitsNumber   = FMC_SDRAM_COLUMN_BITS_NUM_9;
  hSdram->Init.RowBitsNumber      = FMC_SDRAM_ROW_BITS_NUM_12;
  hSdram->Init.MemoryDataWidth    = FMC_SDRAM_MEM_BUS_WIDTH_32;
  hSdram->Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;
  hSdram->Init.CASLatency         = FMC_SDRAM_CAS_LATENCY_3;
  hSdram->Init.WriteProtection    = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
  hSdram->Init.SDClockPeriod      = FMC_SDRAM_CLOCK_PERIOD_3;
  hSdram->Init.ReadBurst          = FMC_SDRAM_RBURST_ENABLE;
  hsdram->Init.ReadPipeDelay      = FMC_SDRAM_RPIPE_DELAY_0;

  /* Timing configuration for 100Mhz as SDRAM clock frequency (System clock is up to 200Mhz) */
  sdram_timing.LoadToActiveDelay    = 2;
  sdram_timing.ExitSelfRefreshDelay = 7;
  sdram_timing.SelfRefreshTime      = 4;
  sdram_timing.RowCycleDelay        = 7;
  sdram_timing.WriteRecoveryTime    = 2;
  sdram_timing.RPDelay              = 2;
  sdram_timing.RCDDelay             = 2;

  /* SDRAM controller initialization */
  if (HAL_SDRAM_Init(hSdram, &sdram_timing) != HAL_OK)
  {
    return  HAL_ERROR;
  }
  return HAL_OK;
}
#endif
