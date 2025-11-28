/**
******************************************************************************
* @file          st_message_conf.h
* @author        MCD Application Team
* @brief         private messages
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


#ifndef ST_MESSAGE_CONF_H
#define ST_MESSAGE_CONF_H




#if defined(USE_LIVETUNE_DESIGNER) || defined(USE_LIVETUNE_TUNER)

  /* connect stmessage to st_base error mngs */
  #define ST_MESSAGE_TRACE_ERROR       ST_TRACE_ERROR
  #define ST_MESSAGE_VERIFY(arg)       ST_VERIFY(arg)
  #define ST_MESSAGE_ASSERT(arg)       ST_ASSERT(arg)

#endif /* USE_LIVETUNE_DESIGNER || USE_LIVETUNE_TUNER */

/*!

  @brief Standard events available for all platform. Some extra events may be added by the user application
  @ingroup enum
*/

typedef uint32_t ST_Message_Event;

#define ST_EVT_NULL               0U /*!< Notify First msg , event message never 0 */
#define ST_EVT_REBOOT             1U /*!< Notify the system will  reboot PARAM none */
#define ST_EVT_RESTART            2U /*!< Notify the system is restarting  PARAM none */
#define ST_EVT_SYSTEM_STARTED     3U /*!< Notify that the all mandatory tasks are started PARAM none */
#define ST_EVT_SYSTEM_READY       4U /*!< Notify that the system is ready to start and elements are installed*/
#define ST_EVT_SYSTEM_SHUTDOWN    5U /*!< Notify that the system will shutdown PARAM none */
#define ST_EVT_INIT_PERSISTENT    6U /*!< notify that the system will start to the use the persistent storage, the app need to initialize the persistent manager and set  its default value if it is blank PARAM = error*/
#define ST_EVT_TEST_REPORT        7U /*!< notify a test event*/
#define ST_EVT_MSG                8U /*!< Notify  just a message  PARAM= char * */
#define ST_EVT_START_ISSUE_MSG    9U /*!< Notify  an issue during the pipe start */
#define ST_EVT_UPDATE_CONFIG_MSG  10U /*!< Notify a config update */
#define ST_EVT_PIPE_STOP_ERROR    12U /*!< Notify the pipe is stopped due to and error */
#define ST_EVT_REENTRANT_END      13U
#define ST_EVT_NO_REENTRANT_START 20U
#define ST_EVT_NO_REENTRANT_END   21U /*!< End of the Re-entrant events */
#define ST_EVT_USER               30U /*!< Start of the user events */
#define ST_EVT_ALIGN              (ST_EVT_USER + 1)



#endif /* ST_MESSAGE_CONF_H */

