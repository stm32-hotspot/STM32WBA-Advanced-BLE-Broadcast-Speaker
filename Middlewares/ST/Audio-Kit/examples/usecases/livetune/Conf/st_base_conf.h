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


#ifndef _ST_BASE_CONF_H_
#define _ST_BASE_CONF_H_

#if defined(USE_LIVETUNE_DESIGNER) || defined(USE_LIVETUNE_TUNER)
  #define ST_TRACE_LVL_DESIGNER (ST_TRACE_LVL_USER1)

  #define TRACE_COLOR_USER1 "\033[35;1m"
  /* Overload the default traces for this project */
  #define ST_TRACE_LVL_DEFAULT (ST_TRACE_LVL_COLORIZE | ST_TRACE_LVL_ERROR | ST_TRACE_LVL_DESIGNER | ST_TRACE_LVL_INFO) /*!< Default traces */

#endif /* USE_LIVETUNE_DESIGNER || USE_LIVETUNE_TUNER */

#endif /* _ST_BASE_CONF_H_ */


