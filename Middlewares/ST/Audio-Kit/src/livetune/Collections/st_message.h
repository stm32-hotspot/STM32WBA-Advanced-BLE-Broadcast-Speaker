/**
******************************************************************************
* @file          st_message.h
* @author        MCD Application Team
* @brief         dispatch message manager
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
#ifndef ST_MESSAGE_H
#define ST_MESSAGE_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/

#include "st_base.h"
#include "st_os.h"
#include "st_message_conf.h"
#include <stdarg.h>


/* Exported constants --------------------------------------------------------*/

#define ST_MESSAGE_REENTRANT_START 0x8000UL
#define ST_MESSAGE_REENTRANT_END   0xFFFFUL

#ifndef ST_MESSAGE_OK
#define ST_MESSAGE_OK ST_OS_OK
#endif

#ifndef ST_MESSAGE_ERROR
#define ST_MESSAGE_ERROR ST_OS_ERROR
#endif


#ifndef ST_MESSAGE_TRACE_ERROR
#define ST_MESSAGE_TRACE_ERROR(...) {printf(__VA_ARGS__);printf("\n");}
#endif

#ifndef ST_MESSAGE_VERIFY
#define ST_MESSAGE_VERIFY(a)  ((void)(a))
#endif

#ifndef ST_MESSAGE_ASSERT
#define ST_MESSAGE_ASSERT(a)
#endif



/* Exported types ------------------------------------------------------------*/


struct st_message_post_item;
struct st_message;
typedef uint64_t st_message_param;

typedef uint32_t (*st_message_poste_cb)(struct st_message_post_item *pPost);

/*!
@brief User event callback
@ingroup type
@param  pHandle  Instance handle
@param  pCookie  User cookie you can set in the factory
@param  evt      Event fired
@param  wparam   Event parameter
@param  lparam   Event parameter
@return Error code
*/

typedef uint32_t (*st_message_event_cb)(struct st_message *pHandle, void *pCookie, uint32_t evt, st_message_param wparam, st_message_param lParam);


/* message queue struct */

typedef enum st_message_type_post
{
  ST_POST_EVENT,
  ST_POST_CALLBACK,
  ST_POST_DISABLED,
} st_message_type_post;



typedef struct st_message_post_item
{
  st_message_type_post type;
  st_message_param     wparam;                          /* posted param 1*/
  st_message_param     lparam1;                         /* posted param 2*/
  st_message_param     lparam2;                         /* posted param 3*/
  void (*deleteCB)(struct st_message_post_item *pPost); /* if not null, will be called when the send is finished */
  void *pCookie;
} st_message_post_item;

typedef struct st_message
{
  st_message_event_cb pDispatcher;
  st_mutex            hLockMessage; /* Mutual exclusion  for message dispatch */
  st_queue            hPostMsg;     /* Handle of the message queue */
  uint32_t            iRunPostTaskFlag;
  st_task             hPostTask; /* Handle task post */
} st_message;

/* Exported macros ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

ST_OS_Result st_message_send(struct st_message *pHandle, void *pCookie, uint32_t evt, st_message_param wParam, st_message_param lParam);
ST_OS_Result st_message_post(struct st_message *pHandle, void *pCookie, uint32_t evt, st_message_param wParam, st_message_param lParam);
ST_OS_Result st_message_post_delete_cb(st_message *pHandle, void *pCookie, uint32_t evt, st_message_param wParam, st_message_param lParam, void (*deleteCB)(struct st_message_post_item *pPost));
ST_OS_Result st_message_create(struct st_message *pHandle, st_message_event_cb pDispatcher);
ST_OS_Result st_message_delete(struct st_message *pHandle);
ST_OS_Result st_message_internal(st_message *pHandle, void *pCookie, uint32_t evt, st_message_param wParam, st_message_param lParam);
ST_OS_Result st_message_cb_post(st_message *pHandle, uint32_t timeout, st_message_poste_cb cb, void *pCookie, st_message_param lparam1, st_message_param lparam2);
void st_message_cycleMeasure_Init(void);
void st_message_cycleMeasure_Reset(void);
void st_message_cycleMeasure_Start(void);
void st_message_cycleMeasure_Stop(void);

#ifdef __cplusplus
};
#endif


#endif


