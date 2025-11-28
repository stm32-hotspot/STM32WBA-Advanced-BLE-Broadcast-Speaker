/**
******************************************************************************
* @file          st_os_compiler_support.c
* @author        MCD Application Team
* @brief         expose os memory and threading support

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

/* Includes ------------------------------------------------------------------*/

#include <time.h>

#ifdef USE_FREERTOS
  /*cstat -MISRAC2012-Rule-10.4_a  A third-party produces a violation message we cannot fix*/
  #include "FreeRTOS.h"
  /*cstat +MISRAC2012-Rule-10.4_a */
  #include "task.h"
#endif

#include "st_os.h"
#include "st_os_compiler_support.h"
#include "st_os_mem.h"
#include "cmsis_compiler.h"
#include "assert.h"

#include <stdbool.h>
#include <string.h>

/* Global variables ----------------------------------------------------------*/
/* Private variables ----------------------------------------------------------*/
static st_os_mem_status sysInfo;
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private functions ------------------------------------------------------- */

#if  defined ( __GNUC__  )
#include <malloc.h>
extern uint32_t HAL_GetTick(void);


extern char _sheap;
extern char _eheap;
extern char _sdata;
extern char _edata;
extern char _sbss;
extern char _ebss;
extern char _sflash;
extern char _eflash;


/**
* @brief Checks if ptr is a heap block.   ptr must be the pointer returned by malloc.
 * @param pHeap a pointer
 * @return bool
 */
bool st_os_mem_check_heap_ptr(void *pHeap)
{
  return ((pHeap >= (void *)&_sheap) && (pHeap <= (void *)&_eheap));
}



/**
 * @brief * GCC manages malloc info  differently, arena represents the current maximum heap and not the system linker size allocated for the heap
          * so , we need to extract the heap len from link script info

 *
 * @param pSysInfo  sysinfo instance
 * @return bool
 */

bool st_os_mem_section_info(st_os_mem_status **ppSysInfo)
{
  static bool            done     = false;
  uint32_t               szHeap   = &_eheap  - &_sheap;
  uint32_t               szdata   = &_edata  - &_sdata;
  uint32_t               szbss    = &_ebss   - &_sbss;
  uint32_t               szflash  = &_eflash - &_sflash;
  st_os_mem_status      *pSysInfo = &sysInfo;
  static struct mallinfo mi;

  if (!done)
  {
    memset(pSysInfo, 0, sizeof(*pSysInfo));

    pSysInfo->iMemBssSpace   = szbss;
    pSysInfo->iMemFlashSpace = szflash;
    pSysInfo->iMemDataSpace  = szdata;

    pSysInfo->tPools[ST_Mem_Type_HEAP].iPoolSize     = szHeap;
    pSysInfo->tPools[ST_Mem_Type_HEAP].iPoolEmbedded = 0;
    pSysInfo->tPools[ST_Mem_Type_HEAP].pName         = "HEAP";
    pSysInfo->tPools[ST_Mem_Type_HEAP].iPoolFrag     = 0;
    pSysInfo->tPools[ST_Mem_Type_HEAP].iFlags        = 0;

    done = true;
  }

  mi = mallinfo();
  pSysInfo->tPools[ST_Mem_Type_HEAP].iPoolFreeSize = szHeap - mi.uordblks;
  pSysInfo->tPools[ST_Mem_Type_HEAP].iPoolMaxAlloc = szHeap - mi.uordblks;

  *ppSysInfo = pSysInfo;

  return true;
}

int _gettimeofday(struct timeval *tp, void *tzvp)
{
  time_t curTime = HAL_GetTick() / 1000;
  struct timezone *tzp = tzvp;
  if (tp)
  {
    tp->tv_sec = curTime;
  }
  tp->tv_usec = 0;
  /* Return fixed data for the timezone.  */
  if (tzp)
  {
    //    tzp->tz_dsttime = 0;
    //    tzp->tz_minuteswest = 0;
  }

  return 0;
}


/* Provides a clock disconnected from the debugger */
clock_t clock(void)
{
  return HAL_GetTick();
}

#endif // __GNUC__


#if  defined ( __CC_ARM )

/**
* @brief * GCC manages malloc info  differently, arena represents the current maximum heap and not the system linker size allocated for the heap
          * so , we need to extract the heap len from link script info
 *
 * @param pSysInfo sys info instance
 * @return bool
 */
bool st_os_mem_section_info(st_os_mem_status **ppSysInfo)
{
  /* not implemented */
  static bool       done     = false;
  st_os_mem_status *pSysInfo = &sysInfo;

  if (!done)
  {
    memset(pSysInfo, 0, sizeof(*pSysInfo));
    done = true;
  }

  *ppSysInfo = pSysInfo;

  return true;
}



/**
* @brief Checks if ptr is a heap block.   ptr must be the pointer returned by malloc.
 * @param pHeap a pointer
 * @return bool
 */

bool st_os_mem_check_heap_ptr(void *pHeap)
{
  return true;
}


/**
 * @brief GCC  assert  overload
 *
 */
__attribute__((weak, noreturn))
void __aeabi_assert(const char *expr, const char *file, int line)
{
  char str[12], *p;

  fputs("*** assertion failed: ", stderr);
  fputs(expr, stderr);
  fputs(", file ", stderr);
  fputs(file, stderr);
  fputs(", line ", stderr);

  p = str + sizeof(str);
  *--p = '\0';
  *--p = '\n';
  while (line > 0)
  {
    *--p = '0' + (line % 10);
    line /= 10;
  }
  fputs(p, stderr);

  abort();
}


int _gettimeofday(struct timeval *tp, void *tzvp)
{
  time_t curTime = HAL_GetTick() / 1000;
  struct timezone *tzp = tzvp;
  if (tp)
  {
    tp->tv_sec = curTime;
  }
  tp->tv_usec = 0;
  /* Return fixed data for the timezone.  */
  if (tzp)
  {
    tzp->tz_minuteswest = 0;
    tzp->tz_dsttime = 0;
  }

  return 0;
}



int _gettimeofday(struct timeval *tp, void *tzvp)
{
  time_t curTime;
  struct timezone *tzp = tzvp;
  if (stvs_core_get_instance())
  {
    curTime = (time_t) st_os_sytem_utc_get();
    if (tp)
    {
      tp->tv_sec = curTime;
    }
    tp->tv_usec = 0;
    /* Return fixed data for the timezone.  */
    if (tzp)
    {
      tzp->tz_minuteswest = 0;
      tzp->tz_dsttime = 0;
    }
  }
  return 0;
}

#endif // __CC_ARM



#if defined ( __ICCARM__ ) /*!< IAR Compiler */

#include "iar_dlmalloc.h"

extern uint32_t HAL_GetTick(void);


/*
  IAR STDLIB doesn't support threads  by default
 use --threaded_lib to switch the lib
 the following lines implements the Mutex management

*/
#pragma section = "HEAP"
#pragma section = ".bss"
#pragma section = ".text"
#pragma section = ".data"
#pragma section = ".heap"
#pragma section = "ROM_CONTENT"

const size_t ROM_CONTENT_size @ "ROM_length_used" = __section_size("ROM_CONTENT");


/**
* @brief Checks if ptr is a heap block.   ptr must be the pointer returned by malloc.
 * @param pHeap a pointer
 * @return int
 */

bool st_os_mem_check_heap_ptr(void *pHeap)
{
  void *heap_start = __section_begin("HEAP");
  void *heap_limit = __section_end("HEAP");
  return ((pHeap >= heap_start) && (pHeap <= heap_limit));
}



/**
 * @brief return the section info
 *
 * @param pSysInfo the sysinfo instance
 * @return int
 */
bool st_os_mem_section_info(st_os_mem_status **ppSysInfo)
{
  static bool       done       = false;
  char             *heap_start = __section_begin("HEAP");
  char             *heap_limit = __section_end("HEAP");
  uint32_t          maxHeap    = (uint32_t)heap_limit - (uint32_t)heap_start;
  st_os_mem_status *pSysInfo   = &sysInfo;
  struct mallinfo   mi         = __iar_dlmallinfo();

  if (!done)
  {
    memset(pSysInfo, 0, sizeof(*pSysInfo));

    pSysInfo->iMemFlashSpace = ROM_CONTENT_size;
    pSysInfo->iMemBssSpace   = __section_size(".bss");
    pSysInfo->iMemDataSpace  = __section_size(".data");

    pSysInfo->tPools[ST_Mem_Type_HEAP].iPoolSize     = maxHeap;
    pSysInfo->tPools[ST_Mem_Type_HEAP].iPoolEmbedded = 0;
    pSysInfo->tPools[ST_Mem_Type_HEAP].pName         = "HEAP";
    pSysInfo->tPools[ST_Mem_Type_HEAP].iPoolFrag     = 0;
    pSysInfo->tPools[ST_Mem_Type_HEAP].iFlags        = 0;

    done = true;
  }

  pSysInfo->tPools[ST_Mem_Type_HEAP].iPoolFreeSize = maxHeap  - mi.uordblks;
  pSysInfo->tPools[ST_Mem_Type_HEAP].iPoolMaxAlloc = mi.uordblks; /* Max allocated not available */

  *ppSysInfo = pSysInfo;

  return true;
}


/* Provides a clock disconnected from the debugger */
clock_t clock(void)
{
  return HAL_GetTick();
}

/* Provides a time disconnected from the debugger */
time_t __time64(time_t *p)
{
  time_t  time = (time_t)st_os_sytem_utc_get();
  if (p != NULL)
  {
    *p = time;
  }
  return time;
}


/* Make sure time() is defined, it is used by the system */

#endif // __ICCARM__


#if defined(__CC_ARM) || defined(__GNUC__)

  #ifdef __GNUC__
    #include <malloc.h>
  #endif

  //void __malloc_lock(struct _reent *r)
  //{
  //  configASSERT(!xPortIsInsideInterrupt()); // Make damn sure no more mallocs inside ISRs
  //  vTaskSuspendAll();
  //}
  //
  //
  //void __malloc_unlock(struct _reent *r)
  //{
  //  (void)xTaskResumeAll();
  //}

  // newlib also requires implementing locks for the application's environment memory space,
  // accessed by newlib's setenv() and getenv() functions.
  // As these are trivial functions, momentarily suspend task switching (rather than semaphore).
  // Not required (and trimmed by linker) in applications not using environment variables.
  // ToDo: Move __env_lock/unlock to a separate newlib helper file.

  //void __env_lock()
  //{
  //  vTaskSuspendAll();
  //}
  //
  //void __env_unlock()
  //{
  //  (void)xTaskResumeAll();
  //}

#endif // __CC_ARM || __GNUC__


#if defined ( __ICCARM__ ) /*!< IAR Compiler */

#include "DLib_Threads.h"
/* some IAR overload */


#ifdef USE_FREERTOS

void st_os_compiler_init(void)
{
  __iar_Initlocks();
}


#ifndef ST_OS_SUPPORT_IAR_USE_MUTEX
  #define ST_OS_SUPPORT_IAR_USE_MUTEX   0
#endif

void __iar_system_Mtxinit(__iar_Rmtx *m)
{
  #if ST_OS_SUPPORT_IAR_USE_MUTEX == 1
  static st_mutex tLocks[_MAX_LOCK];
  static int32_t  iLocks = 0;
  assert(iLocks < _MAX_LOCK);
  ST_OS_Result ret = st_os_mutex_create(&tLocks[iLocks]);
  *m = &tLocks[iLocks];
  iLocks++;
  assert(ret == ST_OS_OK);
  #endif
}


void __iar_system_Mtxlock(__iar_Rmtx *m)
{
  #if ST_OS_SUPPORT_IAR_USE_MUTEX == 1
  if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING)
  {
    st_os_mutex_lock((st_mutex *) *m);
  }
  #else
  if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
  {
    vTaskSuspendAll();
  }
  #endif
}


void __iar_system_Mtxunlock(__iar_Rmtx *m)
{
  #if ST_OS_SUPPORT_IAR_USE_MUTEX == 1
  if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING)
  {
    st_os_mutex_unlock((st_mutex *) *m);
  }
  #else
  if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
  {
    (void)xTaskResumeAll();
  }
  #endif

}


void __iar_file_Mtxinit(__iar_Rmtx *p)
{
  #if ST_OS_SUPPORT_IAR_USE_MUTEX == 1
  static st_mutex tFileLocks[FOPEN_MAX];
  static int32_t  iFileLocks = 0;
  assert(iFileLocks < FOPEN_MAX);
  ST_OS_Result ret = st_os_mutex_create(&tFileLocks[iFileLocks]);
  *p = &tFileLocks[iFileLocks];
  iFileLocks++;
  assert(ret == ST_OS_OK);
  #endif
}


void __iar_system_Mtxdst(__iar_Rmtx *m)
{
  /* Simply delete the mutex.  */
  st_os_mutex_delete((st_mutex *) *m);
}


void __iar_file_Mtxdst(__iar_Rmtx *p)
{
  /* Simply delete the mutex.  */
  st_os_mutex_delete((st_mutex *) *p);
}


void __iar_file_Mtxlock(__iar_Rmtx *p)
{
  #if ST_OS_SUPPORT_IAR_USE_MUTEX == 1
  if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING)
  {
    st_os_mutex_lock((st_mutex *) *p);
  }
  #else
  if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
  {
    vTaskSuspendAll();
  }
  #endif
}

void __iar_file_Mtxunlock(__iar_Rmtx *p)
{
  #if ST_OS_SUPPORT_IAR_USE_MUTEX == 1
  if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING)
  {
    st_os_mutex_unlock((st_mutex *) *p);
  }
  #else
  if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
  {
    (void)xTaskResumeAll();
  }
  #endif
}



#include "iar/reent.h"

#pragma section="__iar_tls$$DATA"
struct _reent  iar_global_impure = { __section_begin("__iar_tls$$DATA") };
struct _reent *_impure_ptr          = &iar_global_impure;


// __aeabi_read_tp functions is defined in the tx_iar.c port provided by ThreadX
void *__aeabi_read_tp(void)
{
  return _impure_ptr->ptr;
}

/** Initializes a struct _reent */
/** \param r struct _reent */
void st_os_init_reent(struct _reent *r)
{
  size_t tls_size = __iar_tls_size();
  if (0UL == tls_size)
  {
    tls_size = sizeof(void *);
  }
  r->ptr = pvPortMalloc(tls_size);
  if (NULL == r->ptr)
  {
    __BKPT(0);  // Failed to allocate memory for TLS
  }
  __iar_tls_init(r->ptr);
}


//--------------------------------------------------------------------------------------------------
// _reclaim_reent
//--------------------------------------------------------------------------------------------------
void _reclaim_reent(struct _reent *r)
{
  // __call_thread_dtors must be called from the thread that is being destroyed. This will not
  // work with FreeRTOS.
  // The effect of omitting it is that destructors for thread-local C++ objects will not be
  // executed.
  //__call_thread_dtors();
  if (r != &iar_global_impure)
  {
    vPortFree(r->ptr);
    r->ptr = NULL;
  }
}
#endif // USE_FREERTOS


#endif // __ICCARM__






