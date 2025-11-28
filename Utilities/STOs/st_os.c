/**
******************************************************************************
* @file          st_os.c
* @author        MCD Application Team
* @brief         implement operating system abstraction for FreeRtos
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
#include <string.h>
#include <stdlib.h>

#include "st_os.h"
#include "st_os_mem.h"
#include "st_os_compiler_support.h"


#ifdef ST_USE_DEBUG
  #include <stdio.h>
#endif

/* Global variables ----------------------------------------------------------*/
static st_mutex hLockMemory;
static uint64_t systemTimeSec;
static uint64_t systemTickRef;
static uint32_t systemTimeOffset;


/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
typedef union
{
  struct
  {
    uint32_t iRead;
    uint32_t len;
  };
  uint64_t val;
} atomicPtr_t;

typedef struct st_os_stream_imp
{
  uint8_t             *pBuffer;
  uint32_t             szBuffer;
  volatile atomicPtr_t atomicPtr;
} st_os_stream_imp;



/**
 * @brief Initializes the kernel
 *
 * @return None
 */
void st_os_init(void)
{
  osKernelInitialize();
}


/**
 * @brief Starts the kernel
 *
 * @return error code
 */
ST_OS_Result st_os_start(void)
{
  st_os_mutex_create(&hLockMemory);
  st_os_compiler_init();
  return (osKernelStart() == osOK) ? ST_OS_OK : ST_OS_ERROR;
}



__weak void  st_os_system_time_changed(void)
{
}

__weak void  st_os_compiler_init(void)
{
}



void st_os_sytem_time_set(uint64_t time)
{
    systemTimeSec    = time;
    systemTickRef = st_os_sys_tick();
    st_os_system_time_changed();
}

uint64_t st_os_sytem_utc_get(void)
{
  
    uint64_t curTicks = st_os_sys_tick();
    return systemTimeSec +systemTimeOffset + ((curTicks - systemTickRef)/1000U);
}

void st_os_system_time_offset(uint32_t sec)
{
  systemTimeOffset= sec;
  
}





/**
* @brief overload RTOS unlock for memory
*
*/
void st_os_mem_unlock(void)
{
  st_os_mutex_unlock(&hLockMemory);
}


/**
* @brief  overload RTOS  lock for memory
*
*/
void st_os_mem_lock(void)
{
  st_os_mutex_lock(&hLockMemory);
}



/**
 * @brief Create a packet streaming interface, the user pushes a packet in a ring buffer a can read from an other task
 *
 * @param pHandle instance pointer
 * @param pBuffer ring buffer pointer
 * @param szBuffer buffer size
 * @param flags   some flags
 * @return ST_OS_Result
 */

ST_OS_Result st_os_stream_create(st_stream *pHandle, void *pBuffer, const uint32_t szBuffer)
{
  ST_OS_Result result = ST_OS_OK;
  ST_OS_ASSERT(pHandle != NULL);
  /* create the porting handler */
  st_os_stream_imp *pStream = (st_os_stream_imp *)st_os_mem_alloc(ST_Mem_Type_HEAP, sizeof(st_os_stream_imp)); /*cstat !MISRAC2012-Rule-22.1_a False Positive, the pointer is passed to a struct handle that will be freed later one in st_os_queue_delete */
  if (pStream)
  {
    pHandle->hStream = pStream;
    /* buffer init */
    memset(pStream, 0, sizeof(*pStream));
    pStream->pBuffer  = pBuffer;
    pStream->szBuffer = szBuffer;
    st_os_stream_reset(pHandle);
  }
  if ((result != ST_OS_OK) || (pStream == NULL))
  {
    /* free if error */
    st_os_mem_free(pStream);
    pHandle->hStream = NULL;
  }

  return result;
}

/**
 * @brief delete a os stream instance
 *
 * @param pHandle  instance pointer
 * @return ST_OS_Result
 */

ST_OS_Result st_os_stream_delete(st_stream *pHandle)
{
  ST_OS_Result result = ST_OS_OK;
  ST_OS_ASSERT((pHandle != NULL) && (pHandle->hStream != NULL));
  st_os_stream_imp *pStream = pHandle->hStream;
  /* assume delete function doesn't fail if already freed */
  st_os_mem_free(pStream);
  pHandle->hStream = NULL;
  return result;
}

/**
 * @brief reset the stream
 *
 * @param pHandle   instance pointer
 * @return error code
 */
ST_OS_Result st_os_stream_reset(st_stream *pHandle)
{
  ST_OS_Result result = ST_OS_OK;
  ST_OS_ASSERT((pHandle != NULL) && (pHandle->hStream != NULL));
  st_os_stream_imp *pStream = pHandle->hStream;
  /* for debug purpose, we start the buffer at 1/2 buffer */
  pStream->atomicPtr.iRead = pStream->szBuffer / 2U;
  pStream->atomicPtr.len   = 0UL;
  memset(pStream->pBuffer, 0, pStream->szBuffer);
  return result;
}



/**
 * @brief return the stream size to read available unlocked
 *
 * @param pHandle   instance pointer
 * @return uint32_t read size available
 */
static uint32_t st_os_stream_read_available_unlocked(st_stream *pHandle)
{
  st_os_stream_imp *pStream = pHandle->hStream;
  return pStream->atomicPtr.len;
}


/**
 * @brief return the stream size to write available unlocked
 *
 * @param pHandle   instance pointer
 * @return uint32_t write size available
 */
static uint32_t st_os_stream_write_available_unlocked(st_stream *pHandle)
{
  st_os_stream_imp *pStream = pHandle->hStream;
  return pStream->szBuffer - pStream->atomicPtr.len;
}


/**
 * @brief return the stream size to read available (safe)
 *
 * @param pHandle   instance  pointer
 * @return uint32_t read size available
 */
uint32_t st_os_stream_read_available(st_stream *pHandle)
{
  ST_OS_ASSERT((pHandle != NULL) && (pHandle->hStream != NULL));
  // mutex is useless since st_os_stream_read_available_unlocked reads only one variable field of stream buffer
  return st_os_stream_read_available_unlocked(pHandle);
}


/**
 * @brief return the stream size to write available (safe)
 *
 * @param pHandle   instance  pointer
 * @return uint32_t write size  available
 */
uint32_t st_os_stream_write_available(st_stream *pHandle)
{
  ST_OS_ASSERT((pHandle != NULL) && (pHandle->hStream != NULL));
  // mutex is useless since st_os_stream_read_available_unlocked reads only one variable field of stream buffer
  return st_os_stream_write_available_unlocked(pHandle);
}


/**
 * @brief  write a packet in the stream safe and manage called from ISR
 *
 * @param pHandle instance pointer
 * @param pData data pointer to write
 * @param szData size to write
 * @return uint32_t size written
 */

uint32_t st_os_stream_write(st_stream *pHandle, const void *pData, const uint32_t szData)
{
  ST_OS_ASSERT((pHandle != NULL) && (pHandle->hStream != NULL));
  uint32_t szCurBuff = szData;
  st_os_stream_imp *pStream = pHandle->hStream;
  uint32_t szWriteAvailable = st_os_stream_write_available_unlocked(pHandle);

  /* if the size asked is bigger than the available, we clamp it to size available */
  if (szCurBuff > szWriteAvailable)
  {
    szCurBuff = szWriteAvailable;
  }
  const uint8_t *pCurBuff = pData;
  atomicPtr_t atomicPtr;
  atomicPtr.val = pStream->atomicPtr.val; // insure atomic read of iRead and len
  uint32_t iWrite = atomicPtr.iRead + atomicPtr.len;
  if (iWrite >= pStream->szBuffer)
  {
    iWrite -= pStream->szBuffer;
  }
  if ((iWrite + szCurBuff) <= pStream->szBuffer)
  {
    memcpy(&pStream->pBuffer[iWrite], pCurBuff, szCurBuff);
  }
  else
  {
    uint32_t part1 = pStream->szBuffer - iWrite;
    uint32_t part2 = szCurBuff - part1;
    memcpy(&pStream->pBuffer[iWrite], pCurBuff, part1);
    memcpy(pStream->pBuffer, &pCurBuff[part1], part2);
  }
  if (st_os_is_irq() == ST_OS_FALSE)
  {
    st_os_enter_critical_section();
    pStream->atomicPtr.len += szCurBuff;
    st_os_exit_critical_section();
  }
  else
  {
    pStream->atomicPtr.len += szCurBuff;
  }
  return szCurBuff;
}


/**
 * @brief  read a packet in the stream (safe)
 *
 * @param pHandle instance pointer
 * @param pData data pointer to write
 * @param szData size  to write
 * @return uint32_t size read
 */

uint32_t st_os_stream_read(st_stream *pHandle, const void *pData, const uint32_t szData, uint32_t timeout)
{
  ST_OS_ASSERT((pHandle != NULL) && (pHandle->hStream != NULL));
  ST_OS_Result      result      = ST_OS_OK;
  st_os_stream_imp *pStream     = pHandle->hStream;
  uint32_t          szAvailable = st_os_stream_read_available_unlocked(pHandle);
  uint32_t          szRead      = 0UL;

  if (szAvailable < szData)
  {
    /*
    there is not enough space to satisfy the request
    so we give a chance to get the remaining data before the timeout
    */
    uint32_t timeoutTicks = (timeout * osKernelGetTickFreq()) / 1000UL;
    result                = ST_OS_BUSY;
    while (timeoutTicks > 0UL)
    {
      /* check the size */
      szAvailable = st_os_stream_read_available_unlocked(pHandle);
      if (szAvailable >= szData)
      {
        /* ok there is enough data in the buffer to return without error */
        result = ST_OS_OK;
        break;
      }
      else
      {
        st_os_task_tdelay(1); /* sleep a bit (1 tick) */
        timeoutTicks--;
      }
    }
  }

  uint32_t szCurBuff = szData;
  if ((result == ST_OS_OK) && (szCurBuff != 0U))
  {
    /* if the size asked is bigger than the available, we clamp it to size available */
    if (szCurBuff > szAvailable)
    {
      szCurBuff = szAvailable;
    }
    uint8_t *pCurBuff = (uint8_t *)pData;
    atomicPtr_t atomicPtr;
    atomicPtr.val = pStream->atomicPtr.val; // insure atomic read of iRead and len
    if ((atomicPtr.iRead + szCurBuff) <= pStream->szBuffer)
    {
      memcpy(pCurBuff, &pStream->pBuffer[atomicPtr.iRead], szCurBuff);
      atomicPtr.iRead += szCurBuff;
      if (atomicPtr.iRead == pStream->szBuffer)
      {
        atomicPtr.iRead = 0UL;
      }
    }
    else
    {
      /* the buffer is split in 2 parts, reconstitute it in aligned.*/
      uint32_t part1 = pStream->szBuffer - atomicPtr.iRead;
      uint32_t part2 = szCurBuff - part1;
      memcpy(pCurBuff, &pStream->pBuffer[atomicPtr.iRead], part1);
      memcpy(&pCurBuff[part1], pStream->pBuffer, part2);
      atomicPtr.iRead = part2;
    }
    if (st_os_is_irq() == ST_OS_FALSE)
    {
      st_os_enter_critical_section();
      pStream->atomicPtr.iRead = atomicPtr.iRead;
      pStream->atomicPtr.len  -= szCurBuff;
      st_os_exit_critical_section();
    }
    else
    {
      pStream->atomicPtr.iRead = atomicPtr.iRead;
      pStream->atomicPtr.len  -= szCurBuff;
    }
    szRead = szCurBuff;
  }
  return szRead;
}



/* other RTOS here ... */

/**
 * @brief Write a file using the IAR debugger, use it only for debug
 *
 * @return error code
 */
#ifdef ST_USE_DEBUG
int32_t st_os_write_debug_file(char *pName, void *pBuffer, uint32_t szBuffer)
{
  int32_t bret  = 1;
  FILE   *pFile = fopen(pName, "wb");
  if (pFile)
  {
    while ((szBuffer != 0U) && (bret != 0))
    {
      uint32_t blk = 1000U;
      if (blk > szBuffer)
      {
        blk = szBuffer;
      }
      if (fwrite(pBuffer, 1, blk, pFile) == blk)
      {
        bret = 1;
      }
      szBuffer -= blk;
      pBuffer = ((uint8_t *)pBuffer) + blk;
    }
    fclose(pFile);
  }
  return bret;
}

int32_t st_os_read_debug_file(char *pName, void *pBuffer, uint32_t szBuffer)
{
  uint32_t szRead = 0;
  FILE    *pFile  = fopen(pName, "rb");
  if (pFile)
  {
    while ((szBuffer != 0U))
    {
      uint32_t blk = 1000U;
      if (blk > szBuffer)
      {
        blk = szBuffer;
      }
      blk = fread(pBuffer, 1, blk, pFile);
      if (blk == 0U)
      {
        break;
      }
      szBuffer -= blk;
      szRead += blk;
      pBuffer = ((uint8_t *)pBuffer) + blk;
    }
    fclose(pFile);
  }
  return (int32_t)szRead;
}

#endif



