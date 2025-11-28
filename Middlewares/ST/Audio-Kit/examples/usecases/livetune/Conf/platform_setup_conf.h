/**
******************************************************************************
* @file          platform_setup_conf.h
* @author        MCD Application Team
* @brief         Board HW setup
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


#ifndef PLATFORM_SETUP_H
#define PLATFORM_SETUP_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/

#ifdef STM32H735xx
#include "stm32h7xx_hal.h"
#include "stm32h735g_discovery.h"
#include "stm32h735g_discovery_ospi.h"

#elif defined(STM32H747xx)

#include "stm32h7xx_hal.h"
#include "stm32h747i_discovery.h"
#include "stm32h747i_discovery_qspi.h"
#include "stm32h747i_discovery_sdram.h"

#elif defined(STM32N657xx)

#include "stm32n6xx_hal.h"
#include "stm32n6570_discovery.h"
#include "stm32n6570_discovery_xspi.h"


#elif defined(STM32H573xx)

#include "stm32h5xx_hal.h"
//#include "stm32h573i_discovery.h"
//#include "stm32h573i_discovery_ospi.h"


#elif defined(STM32H7S7xx)

#include "stm32h7rsxx_hal.h"
#include "stm32h7s78_discovery.h"
#include "stm32h7s78_discovery_xspi.h"



#else
#error "Platform is not supported by LiveTune's Designer"
#endif
/* Exported types ------------------------------------------------------------*/
/* Exported defines --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
#endif
