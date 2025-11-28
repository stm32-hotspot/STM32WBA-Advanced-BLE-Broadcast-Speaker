/**
  ******************************************************************************
  * @file    stm32_term.h
  * @author  MCD Application Team
  * @brief   Header for stm32_term.c module
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022(-2022) STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32_TERM_H
#define __STM32_TERM_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>

#include "stm32_term_conf.h"
#include "stm32_errors.h"
#include "stm32_usart.h"
/* Exported macros -----------------------------------------------------------*/
#ifndef UTIL_TERM_CLI_SIZE
#define UTIL_TERM_CLI_SIZE (256)
#endif
#ifndef UTIL_TERM_TIMEOUT_EOF
#define UTIL_TERM_TIMEOUT_EOF 500U /* read timeout threshold before detect an end of file for raw receive file */
#endif
#ifndef UTIL_TERM_MAX_PARAM
#define UTIL_TERM_MAX_PARAM 10U
#endif
#ifndef UTIL_TERM_MEM_PULL
#define UTIL_TERM_MEM_PULL 0
#endif

#ifndef UTIL_TERM_SYNC_TIMEOUT
#define UTIL_TERM_SYNC_TIMEOUT 10UL /* sychro read file timeout */
#endif

#ifndef UTIL_TERM_IDLE_TIMEOUT
#define UTIL_TERM_IDLE_TIMEOUT 1000
#endif

#ifndef UTIL_TERM_IDLE_CB_MAX
#define UTIL_TERM_IDLE_CB_MAX 3U
#endif

#ifndef UTIL_TERM_BUFFER_GROW
#define UTIL_TERM_BUFFER_GROW    (5U*1024U)
#endif


#ifndef UTIL_TERM_USE_CSI
/* use Control Sequence Introducer */
#define UTIL_TERM_USE_CSI
#endif


/* Exported types ------------------------------------------------------------*/

typedef enum
{
  UTIL_TERM_MUTE_LOG_FORCE_ON = 0,
  UTIL_TERM_MUTE_LOG_FORCE_OFF,
  UTIL_TERM_MUTE_RX_ON,
  UTIL_TERM_MUTE_RX_OFF,
} UTIL_TERM_MUTE_t;



/**
* @brief Command entry
*
*/
typedef struct t_str_cmd_entry
{
  const char *name;
  const char *params;
  const char *comments;
  void (*command)(int argc, char *argv[]);
  const uint16_t flags;
} UTIL_TERM_cmd_str_entry_t;


typedef void (*UTIL_TERM_IDLE_CB)(void);


typedef struct UTIL_TERM_t
{
  void               *pCookie;
  UART_HandleTypeDef *pHdleUart;
  bool                bInitDone;
  bool                bCliEnable;
  bool                rxMute;
  bool                txMute;
  char                tFormat[400];
  DMA_HandleTypeDef  *hRxDma;
  uint8_t            *pDmaBuffer;
  uint16_t            szDmaBlk;
  uint8_t             szIdleCb;
  UTIL_TERM_IDLE_CB   tIdleCb[UTIL_TERM_IDLE_CB_MAX];
  bool                bTerminalEnabled;
  bool                bRawTransfer;
  uint32_t            lenBuffer;
  uint8_t             szHistory;
  uint8_t             iHistory;
  uint8_t             maxHistory;
  uint32_t            iPosCursor;
  char               *pBuffer;
  bool                bEsc;
  uint8_t             iIndexCsi;
  uint8_t             *pDmaCurrent;
  uint32_t            iRxBufferOffset;
  char                tCsi[6]; /* sequence CSI no more of 4 chars*/
} UTIL_TERM_t;

/* allocator prototype if overloaded */
typedef void *(*UTIL_TERM_realloc_t)(void *ptr, size_t size);
typedef void (*UTIL_TERM_free_t)(void *ptr);



/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

int32_t UTIL_TERM_set_uart_rx(UART_HandleTypeDef *const huart);
int32_t UTIL_TERM_create(void);
int32_t UTIL_TERM_delete(void);
void    UTIL_TERM_printf(char *pFormat, ...);
void    UTIL_TERM_printf_cr(char *pFormat, ...);
void    UTIL_TERM_printf_forced(char *pFormat, ...);
void    UTIL_TERM_printf_forced_cr(char *pFormat, ...);
int32_t UTIL_TERM_receive_file(char_t **pBuffer, uint32_t  *pOutSzPaste, uint32_t const syncTmo);
void    UTIL_TERM_puts(char *const pString);
void    UTIL_TERM_parse_char(char const theChar);
int32_t UTIL_TERM_process(void);
void    UTIL_TERM_cmd_help(int argc, char *argv[]);
void    UTIL_TERM_cmd_reboot(int argc, char *argv[]);
bool    UTIL_TERM_IsLogMuted(void);
void    UTIL_TERM_Mute(UTIL_TERM_MUTE_t state);
int32_t UTIL_TERM_inject_rx(uint8_t *pBuffer, uint32_t const szBuffer);
int32_t UTIL_TERM_Enable(bool bEnable);
void    UTIL_TERM_set_alloc_funcs(UTIL_TERM_realloc_t pRealloc, UTIL_TERM_free_t pFree);

UTIL_TERM_t *UTIL_TERM_get_context(void);
uint8_t      UTIL_TERM_add_Idle_listener(UTIL_TERM_IDLE_CB cb);
void         UTIL_TERM_Initialized(UTIL_TERM_t *gContext);
void         UTIL_TERM_parse_in_recorder(uint8_t *pBuffer, uint32_t szBuffer);
void         UTIL_TERM_parse_out_recorder(uint8_t *pBuffer, uint32_t szBuffer);
int32_t      UTIL_TERM_receive_raw_file(uint8_t *pBuffer, uint32_t const szBuffer, uint32_t const syncTmo, uint32_t *pOutSzPaste);

/* User defined functions --------------------------------------------------- */
UTIL_TERM_cmd_str_entry_t const *UTIL_TERM_getCustomCommands(void);


/* Exported constants --------------------------------------------------------*/
/* Exported defines   --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/


#ifdef __cplusplus
}
#endif

#endif /* __STM32_TERM_H */
