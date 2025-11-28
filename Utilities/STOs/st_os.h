/**
******************************************************************************
* @file          st_os.h
* @author        MCD Application Team
* @brief         Header: implement operating system abstraction
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



#ifndef ST_OS_H
#define ST_OS_H

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
/*cstat -MISRAC2012-Rule-10.4_a issue in FreeRTOS*/
#include "st_os_conf.h"
/*cstat +MISRAC2012-Rule-10.4_a*/
#ifdef __cplusplus
extern "C"
{
#endif

/* Exported types ------------------------------------------------------------*/
/* Os wrappers */

#define ST_OS_TRUE       1
#define ST_OS_FALSE      0

typedef enum st_task_priority
{
  ST_Priority_Idle = 1,
  ST_Priority_Low,
  ST_Priority_Low1,
  ST_Priority_Low2,
  ST_Priority_Low3,
  ST_Priority_Low4,
  ST_Priority_Low5,
  ST_Priority_Low6,
  ST_Priority_Low7,

  ST_Priority_Below_Normal,
  ST_Priority_Below_Normal1,
  ST_Priority_Below_Normal2,
  ST_Priority_Below_Normal3,
  ST_Priority_Below_Normal4,
  ST_Priority_Below_Normal5,
  ST_Priority_Below_Normal6,
  ST_Priority_Below_Normal7,

  ST_Priority_Normal,
  ST_Priority_Normal1,
  ST_Priority_Normal2,
  ST_Priority_Normal3,
  ST_Priority_Normal4,
  ST_Priority_Normal5,
  ST_Priority_Normal6,
  ST_Priority_Normal7,

  ST_Priority_Above_Normal,
  ST_Priority_Above_Normal1,
  ST_Priority_Above_Normal2,
  ST_Priority_Above_Normal3,
  ST_Priority_Above_Normal4,
  ST_Priority_Above_Normal5,
  ST_Priority_Above_Normal6,
  ST_Priority_Above_Normal7,

  ST_Priority_High,
  ST_Priority_High1,
  ST_Priority_High2,
  ST_Priority_High3,
  ST_Priority_High4,
  ST_Priority_High5,
  ST_Priority_High6,
  ST_Priority_High7,

  ST_Priority_Realtime,
  ST_Priority_Realtime1,
  ST_Priority_Realtime2,
  ST_Priority_Realtime3,
  ST_Priority_Realtime4,
  ST_Priority_Realtime5,
  ST_Priority_Realtime6,
  ST_Priority_Realtime7,
  ST_Priority_Max
} st_task_priority;

typedef struct t_st_task
{
  void *hTask;
} st_task;

typedef struct
{
  void *hMutex;
} st_mutex;


typedef struct
{
  void *hQueue;
} st_queue;


typedef struct
{
  void *hEvent;
} st_event;


typedef struct
{
  void *hStream;
} st_stream;


typedef void (*st_task_cb)(const void *pCookie); /*!< Task callback*/

#ifndef ST_OS_DISABLE_IRQ
#define ST_OS_DISABLE_IRQ() __disable_irq()
#endif

#ifndef ST_OS_ENABLE_IRQ
#define ST_OS_ENABLE_IRQ() __enable_irq()
#endif

/* Exported constants --------------------------------------------------------*/
#define st_task_running       (1U)
#define st_task_closed        (2U)
#define st_task_force_exit    (4U)
#define st_task_about_closing (8U)

#ifndef RTOS_INDEX_TLS
#define RTOS_INDEX_TLS 0
#endif
#ifndef RTOS_INDEX_TASK_HANDLE
#define RTOS_INDEX_TASK_HANDLE 1
#endif

/* Exported macros  --------------------------------------------------------*/


#ifndef ST_OS_PRINT
#define ST_OS_PRINT(...) printf(__VA_ARGS__)
#endif

#ifndef st_infinit_delay
#error "please define st_infinit_delay inside st_os_conf.h"
#endif

#ifndef ST_OS_Result
#error "please define ST_OS_Result inside st_os_conf.h"
#endif

#ifndef ST_OS_OK
#error "please define ST_OS_OK inside st_os_conf.h"
#endif

#ifndef ST_OS_ERROR
#error "please define ST_OS_ERROR inside st_os_conf.h"
#endif

#ifndef ST_OS_BUSY
#error "please define ST_OS_BUSY inside st_os_conf.h"
#endif

#ifndef ST_OS_ERROR_TIMEOUT
#error "please define ST_OS_ERROR_TIMEOUT inside st_os_conf.h"
#endif


#ifndef ST_OS_ASSERT
#define ST_OS_ASSERT(a)
#endif

#ifndef ST_TRACE_OS_ERROR
#define ST_TRACE_OS_ERROR(...) ST_OS_PRINT(__VA_ARGS__)
#endif

#ifndef ST_Mem_Type_Os
#define ST_Mem_Type_Os ST_Mem_Type_ANY_FAST
#endif



/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
struct st_os_mem_info;
/* OS abstraction */

void         st_os_init(void);
ST_OS_Result st_os_start(void);
ST_OS_Result st_os_enter_critical_section(void);
ST_OS_Result st_os_exit_critical_section(void);
void         st_os_switch_isr_required(int8_t xSwitchRequired);
int8_t       st_os_is_irq(void);

ST_OS_Result st_os_event_create_named(st_event *event, const char *pName);
ST_OS_Result st_os_event_delete(st_event *event);
ST_OS_Result st_os_event_set(st_event *event);
ST_OS_Result st_os_event_set_isr(st_event *event);
ST_OS_Result st_os_event_wait(st_event *event, uint32_t timeout);
uint32_t     st_os_event_get_count(st_event *event);
void         st_os_mutex_unlock(st_mutex *mutex);
void         st_os_mutex_lock(st_mutex *mutex);
ST_OS_Result st_os_mutex_trylock(st_mutex *mutex);
ST_OS_Result st_os_mutex_create_named(st_mutex *mutex, const char *pName);
void         st_os_mutex_delete(st_mutex *mutex);

ST_OS_Result st_os_task_create(st_task *task, const char *pName, st_task_cb pCb, void *pCookie, uint32_t stackSize, st_task_priority priority);
void         st_os_task_delete(st_task *task);
void         st_os_task_exit(st_task *task);
ST_OS_Result st_os_task_join(st_task *task,uint32_t timeout);
ST_OS_Result st_os_task_priority_set(st_task *task,st_task_priority priority);

void         st_os_task_delay(uint32_t ms);
void         st_os_task_udelay(uint32_t micro);
void         st_os_task_tdelay(uint32_t nbTicks);
ST_OS_Result st_os_task_info_print(void);
void         st_os_lock_tasks(void);
void         st_os_unlock_tasks(void);
void         st_os_task_end(void);
unsigned long st_os_task_get_stack_high_water_mark(st_task *task);



uint32_t     st_os_sys_tick(void);
uint32_t     st_os_sys_tick_freq(void);
uint64_t     st_os_sys_time(void);
uint64_t     st_os_sys_utime(void);


ST_OS_Result st_os_queue_get(st_queue *pQueue, void *pElement, uint32_t timeout);
ST_OS_Result st_os_queue_create_named(st_queue *pQueue, uint32_t nbElement, uint32_t itemSize, const char *pName);
ST_OS_Result st_os_queue_delete(st_queue *pQueue);
ST_OS_Result st_os_queue_put(st_queue *pQueue, void *pElement, uint32_t timeout);


ST_OS_Result st_os_stream_create(st_stream *pHandle, void *pBuffer, const uint32_t szBuffer);
ST_OS_Result st_os_stream_delete(st_stream *pHandle);
uint32_t     st_os_stream_read_available(st_stream *pHandle);
uint32_t     st_os_stream_write_available(st_stream *pHandle);
uint32_t     st_os_stream_write(st_stream *pHandle, const void *pData, const uint32_t szData);
uint32_t     st_os_stream_read(st_stream *pHandle, const void *pData, const uint32_t szData, uint32_t timeout);
ST_OS_Result st_os_stream_reset(st_stream *pHandle);

int32_t      st_os_write_debug_file(char *pName, void *pBuffer, uint32_t szBuffer);
int32_t      st_os_read_debug_file(char *pName, void *pBuffer, uint32_t szBuffer);
void         st_os_sytem_time_set(uint64_t time);
uint64_t     st_os_sytem_utc_get(void);
void         st_os_system_time_changed(void);
void         st_os_system_time_offset(uint32_t sec);



/* Hook for debug, add the object name for tracealyzer */

#if defined(ST_USE_DEBUG) && configQUEUE_REGISTRY_SIZE > 0
#define st_os_mutex_create(mutex) st_os_mutex_create_named((mutex), (#mutex))
#define st_os_event_create(evt)   st_os_event_create_named((evt), (#evt))
#else
#define st_os_mutex_create(mutex) st_os_mutex_create_named((mutex), NULL)
#define st_os_event_create(evt)   st_os_event_create_named((evt), NULL)
#endif

#ifdef __cplusplus
};
#endif



#endif

