/**
******************************************************************************
* @file          livetuneConf.h
* @author        MCD Application Team
* @brief         designer config
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


#ifndef _LIVETUNE_CONF_H
#define _LIVETUNE_CONF_H

#ifdef USE_LIVETUNE_DESIGNER

#include "st_base.h"
#include "irq_utils.h"
#include "analyze_dbg_conf.h" /* Support for the debug realtime using the logic analyzer DigiView
*                                4 pins are pre-programmed   ARDUINO:CN6:D0-D3 */

#define LIVETUNE_DISABLE_IRQ    disable_irq_with_cnt
#define LIVETUNE_ENABLE_IRQ     enable_irq_with_cnt


#ifdef ST_USE_DEBUG
  #define ST_AC_DEBUG_INSTRUMENTATION
#endif

#endif /* USE_LIVETUNE_DESIGNER */

#endif /* _LIVETUNE_CONF_H */


