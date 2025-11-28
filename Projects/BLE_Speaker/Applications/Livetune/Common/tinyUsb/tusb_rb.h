/**
******************************************************************************
* @file    tusb_rb.h
* @author  MCD Application Team
* @brief   implemente a ring buffer
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
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>


/* Exported typedef -----------------------------------------------------------*/

typedef struct ring_buff_t
{
  uint8_t *pBuffer;
  uint32_t szBuffer;
  // ARM says, Load and Store instructions are atomic
  // and it's execution is guaranteed to be complet before interrupt handler executes
  // So, no lock code needed
  volatile uint32_t iWrite;
  volatile uint32_t iRead;
} ring_buff_t;


/* Exported defines -----------------------------------------------------------*/

#ifndef RING_BUFF_ALLOC
  #define RING_BUFF_ALLOC(size) malloc(size)
#endif

#ifndef RING_BUFF_FREE
  #define RING_BUFF_FREE(ptr)   free(ptr)
#endif


/* Exported functions ------------------------------------------------------- */

/**
 * @brief Init a byte ring buffer
 *
 * @param pRb   instance  pointer
 * @param  szRb  ring buffer size
 * @return true if OK, else false
 */
static inline bool s_rb_init(ring_buff_t *pRb, uint32_t szRb)
{
  bool ok = true;
  memset(pRb, 0, sizeof(*pRb));
  pRb->szBuffer = szRb;
  pRb->pBuffer = RING_BUFF_ALLOC(szRb);
  if (pRb->pBuffer != NULL)
  {
    memset(pRb->pBuffer, 0, szRb);
  }
  else
  {
    ok = false;
  }
  return ok;
}


/**
 * @brief terminate a byte ring buffer
 *
 * @param pRb   instance  pointer
 */

static inline  void s_rb_term(ring_buff_t *pRb)
{
  if (pRb->pBuffer)
  {
    RING_BUFF_FREE(pRb->pBuffer);
    pRb->pBuffer = NULL;
    pRb->szBuffer = 0UL;
  }
}


/**
 * @brief reset
 *
 * @param pRb   instance  pointer
 */

static inline  void s_rb_reset(ring_buff_t *pRb, uint32_t value)
{
  pRb->iRead = 0;
  pRb->iWrite = value;
  memset(pRb->pBuffer, 0, pRb->szBuffer);

}


/**
 * @brief return the read pointer ( consumer )
 *
 * @param pRb   instance  pointer
 * @return a ptr
 */
static inline  uint8_t *s_rb_read_ptr(ring_buff_t *pRb)
{
  return pRb->pBuffer + pRb->iRead;
}

/**
 * @brief return the write pointer ( producer)
 *
 * @param pRb   instance  pointer
 * @return a ptr
 */
static inline  uint8_t *s_rb_write_ptr(ring_buff_t *pRb)
{
  return pRb->pBuffer + pRb->iWrite;
}


/**
 * @brief return the  size to read available
 *
 * @param pRb   instance  pointer
 * @return uint32_t read size  available
 */
static inline  uint32_t s_rb_read_available(ring_buff_t *pRb)
{
  uint32_t const iRead  = pRb->iRead;
  uint32_t const iWrite = pRb->iWrite;
  return (iWrite >= iRead) ? (iWrite - iRead) : (pRb->szBuffer + iWrite - iRead);
}


/**
 * @brief resize the write count to the maximum aligned in the buffer
 *
 * @param pRb   instance  pointer
 * @param count count produced
 * @return uint32_t read size  available
 */

static inline  uint32_t s_rb_write_count_aligned(ring_buff_t *pRb, uint32_t count)
{
  uint32_t const iWrite = pRb->iWrite;
  if ((iWrite + count) > pRb->szBuffer)
  {
    count = pRb->szBuffer - iWrite;
  }
  return count;
}


/**
 * @brief move the read position
 *
 * @param pRb   instance  pointer
 * @param count count consumed
 * @return uint32_t read size  available
 */
static inline  void s_rb_write_move(ring_buff_t *pRb, uint32_t count)
{
  uint32_t iWrite = (pRb->iWrite + count) % pRb->szBuffer;
  pRb->iWrite = iWrite ;
}

/**
 * @brief move the write position
 *
 * @param pRb   instance  pointer
 * @param count count produced
 * @return uint32_t read size  available
 */

static inline  void s_rb_read_move(ring_buff_t *pRb, uint32_t count)
{
  uint32_t iRead = (pRb->iRead + count) % pRb->szBuffer;
  pRb->iRead = iRead ;
}


/**
 * @brief resize the read count to the maximum aligned in the buffer
 *
 * @param pRb   instance  pointer
 * @param count count produced
 * @return uint32_t read size  available
 */

static inline  uint32_t s_rb_read_count_aligned(ring_buff_t *pRb, uint32_t count)
{
  uint32_t const iRead = pRb->iRead;
  if ((iRead + count) > pRb->szBuffer)
  {
    count = pRb->szBuffer - iRead;
  }
  return count;
}



/**
 * @brief return the write size to write  available
 *
 * @param pHandle   instance  pointer
 * @return uint32_t write  size  available
 */

static inline  uint32_t s_rb_write_available(ring_buff_t *pRb)
{
  uint32_t const iRead  = pRb->iRead;
  uint32_t const iWrite = pRb->iWrite;
  return (iRead > iWrite) ? (iRead - iWrite - 1U) : (pRb->szBuffer + iRead - iWrite - 1U);
  /*
  remove 1 to the size to prevent read=write that could mean buffer empty or buffer full
  we assume read==write read empty
  */
}

static inline void s_rb_write_ensure_available(ring_buff_t *pRb, uint32_t size)
{
  // uint32_t sz = s_rb_write_available(pRb);
  s_rb_write_available(pRb);
  if (s_rb_write_available(pRb) < size)
  {
    uint32_t szSeek = size - s_rb_write_available(pRb);
    s_rb_read_move(pRb, szSeek);
    szSeek = s_rb_write_available(pRb);

  }
}



/**
* @brief produce data in the ring buffer
* @param pRb   instance  pointer
* @param pBuffer  buffer pointer
* @param szBuffer  buffer size
* @return count not written
*/
static inline uint32_t s_rb_write(ring_buff_t *pRb, uint8_t *pBuffer, uint32_t szBuffer)
{
  uint32_t szWritten = 0;

  if (s_rb_write_available(pRb) >= szBuffer)
  {
    while (szBuffer)
    {
      uint32_t countAligned = s_rb_write_count_aligned(pRb, szBuffer);
      if (countAligned > szBuffer)
      {
        countAligned = szBuffer;
      }
      memcpy(s_rb_write_ptr(pRb), pBuffer, countAligned);
      s_rb_write_move(pRb, countAligned);
      pBuffer += countAligned;
      szBuffer -= countAligned;
      szWritten += countAligned;
    }
  }
  return szWritten;
}



/**
* @brief consume data from the ring buffer
* @param pRb   instance  pointer
* @param pBuffer  buffer pointer
* @param szBuffer  buffer size
* @return count not read
*/
static inline uint32_t s_rb_read(ring_buff_t *pRb, uint8_t *pBuffer, uint32_t szBuffer)
{
  uint32_t szRead = 0;
  if (s_rb_read_available(pRb) >= szBuffer)
  {
    while (szBuffer)
    {
      uint32_t countAligned = s_rb_read_count_aligned(pRb, szBuffer);
      if (countAligned > szBuffer)
      {
        countAligned = szBuffer;
      }
      memcpy(pBuffer, s_rb_read_ptr(pRb), countAligned);
      s_rb_read_move(pRb, countAligned);
      pBuffer += countAligned;
      szBuffer -= countAligned;
      szRead += countAligned;


    }
  }
  return szRead;
}




/**
* @brief consume data from the ring buffer without moving the position
* @param pRb   instance  pointer
* @param pBuffer  buffer pointer
* @param szBuffer  buffer size
* @return count  read
*/
static inline uint32_t s_rb_read_fetch(ring_buff_t *pRb, uint8_t *pBuffer, uint32_t szBuffer)
{
  uint32_t index = 0UL;
  if (s_rb_read_available(pRb) >= szBuffer)
  {
    while (szBuffer)
    {
      *pBuffer ++ = pRb->pBuffer[(pRb->iRead + index) % pRb->szBuffer];
      index ++;
      szBuffer--;
    }
  }
  return index;
}
