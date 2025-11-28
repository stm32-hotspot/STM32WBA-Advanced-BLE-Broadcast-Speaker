/**
******************************************************************************
* @file    stm32_term.c
* @author  MCD Application Team
* @brief   Manage terminal communication
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

/* Includes ------------------------------------------------------------------*/
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "stm32_term.h"
#include "stm32_term_tasks.h"

/* Global variables ----------------------------------------------------------*/

/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
#define UTIL_TERM_KEY_DEL          ((char)0x7FU)        /* teraterm send 7F rather than an "ESC[3~"*/
#define UTIL_TERM_KEY_ESC_KEYBOARD ((char)0xFEU)
#define UTIL_TERM_KEY_ESC          ((char)0x1BU)
#define UTIL_TERM_KEY_BACK         ((char)0x08U)
#define UTIL_TERM_KEY_UP           "[A"
#define UTIL_TERM_KEY_DN           "[B"
#define UTIL_TERM_KEY_LF           "[D"
#define UTIL_TERM_KEY_RG           "[C"
#define UTIL_TERM_KEY_END          "[4~"
#define UTIL_TERM_KEY_HOME         "[1~"
#define UTIL_TERM_KEY_DEL_CSI      "[3~"
#define UTIL_TERM_KEY_INSERT       "[2~"
#define UTIL_TERM_KEY_BEL          "\x7"
#ifndef UTIL_TERM_SIZE_HISTORY
  #define UTIL_TERM_SIZE_HISTORY    5U
#endif
#define UTIL_CHAR_EOF               0xF0U
#ifndef UTIL_TERM_PROMPT_PROLOGUE
  #define UTIL_TERM_PROMPT_PROLOGUE "cli> "
#endif
/* Private typedef -----------------------------------------------------------*/
/* key overload for Csi support */
typedef struct key_support_cb
{
  uint8_t maxHistory;
  void (*add_history)(void);
  void (*add_char_at_position)(char theChar);
  void (*set_cursor_pos)(uint32_t pos);
  bool (*is_key_del_char)(char theChar);
  bool (*is_key_escape)(char theChar);
  bool (*is_key_enter_exit_cli)(char theChar);
  bool (*is_key_back)(char theChar);
  bool (*is_key_csi)(void);
} key_support_cb;
static const key_support_cb cKeySupport;

/* Private variables ---------------------------------------------------------*/
static UTIL_TERM_t *gContext;

/* Private function prototypes -----------------------------------------------*/
static void                             s_enable_cli(uint8_t bEnable);
static void                             s_rx_error_cb(UART_HandleTypeDef *pHdleUart);
static void                             s_dma_uart_event_cb(struct __UART_HandleTypeDef *huart, uint16_t Pos);
static bool                             s_search_end_of_word(char **const ppLine, const char end_of_word_char);
static void                             s_do_line_cmd(char *pLine);
static UTIL_TERM_cmd_str_entry_t const *s_find_entry(char *command);
static void                             s_dma_uart_rx_half_cb(struct __UART_HandleTypeDef *huart);
static void                             s_dma_uart_rx_cpl_cb(struct __UART_HandleTypeDef *huart);

/* Allocator overload */
static UTIL_TERM_realloc_t s_realloc = (UTIL_TERM_realloc_t)realloc;
static UTIL_TERM_free_t    s_free    = (UTIL_TERM_free_t)free;



/* Functions Definition ------------------------------------------------------*/

__weak void UTIL_TERM_cycleMeasure_Init(void)
{
}


__weak void UTIL_TERM_cycleMeasure_Reset(void)
{
}


__weak void UTIL_TERM_cycleMeasure_Start(void)
{
}


__weak void UTIL_TERM_cycleMeasure_Stop(void)
{
}


/**
* @brief Terminal init
*
*/

/**
* @brief Return the cli buffer
* @param index history buffer
*
*/
static char *UTIL_TERM_get_cli_buffer(uint32_t index)
{
  return &gContext->pBuffer[index * gContext->lenBuffer];
}


/**
* @brief Overload allocator functions
*
* @param pRealloc callback realloc
* @param pFree callback free
*/
void UTIL_TERM_set_alloc_funcs(UTIL_TERM_realloc_t pRealloc, UTIL_TERM_free_t pFree)
{
  s_realloc = pRealloc;
  s_free = pFree;
}



#ifdef UTIL_TERM_USE_CSI

/**
 * @brief Insert a char in the current buffer
 *
 * @param theChar
 */
static void s_csi_insert_char(char theChar)
{
  uint32_t lenMove = gContext->lenBuffer - gContext->iPosCursor - 1U;
  memmove(&UTIL_TERM_get_cli_buffer(0)[gContext->iPosCursor + 1U], &UTIL_TERM_get_cli_buffer(0)[gContext->iPosCursor], lenMove);
  UTIL_TERM_get_cli_buffer(0)[gContext->iPosCursor] = theChar;
  gContext->iPosCursor++;
  char strChar[6] = {'\x1B', '[', '1', '@', theChar, '\0'};
  UTIL_TERM_puts(strChar); /* echo on the console  */
}


/**
 * @brief  Insert a char in the cursor position
 *
 * @param theChar
 */
static void s_csi_add_char_at_position(char theChar)
{
  if (gContext->iPosCursor < (gContext->lenBuffer - 1U))
  {
    /* if the char is ascii, add it to the Cli buffer */
    if ((theChar >= ' ') && (theChar <= (char)127U))
    {
      s_csi_insert_char(theChar);
    }
  }
  else
  {
    UTIL_TERM_puts(UTIL_TERM_KEY_BEL);
  }
}


/**
 * @brief Set the cursor to a specific position in the line
 *
 * @param pos
 */
static void s_csi_set_cursor_pos(uint32_t pos)
{
  gContext->iPosCursor = pos;
  char tChar[10];
  snprintf(tChar, sizeof(tChar) - 1U, "\x1B[%d`", 1U + pos + strlen(UTIL_TERM_PROMPT_PROLOGUE));
  UTIL_TERM_puts(tChar);
}


/**
 * @brief Check the Key DEL and do the action
 *
 * @param theChar
 * @return false
 */

static bool s_csi_is_key_del_char(char theChar)
{
  bool bDone = false;
  if ((gContext->bCliEnable == TRUE) && (theChar == UTIL_TERM_KEY_DEL))
  {
    if (gContext->iPosCursor < strlen(UTIL_TERM_get_cli_buffer(0)))
    {
      uint32_t szMove = gContext->lenBuffer - gContext->iPosCursor - 1U;
      memmove(UTIL_TERM_get_cli_buffer(0), UTIL_TERM_get_cli_buffer(0) + 1, szMove);
      cKeySupport.set_cursor_pos(gContext->iPosCursor);
      UTIL_TERM_puts("\x1B[1P"); /* delete char */
    }
    else
    {
      UTIL_TERM_puts(UTIL_TERM_KEY_BEL);
    }
    bDone = true;
  }
  return bDone;
}


/**
 * @brief Check the Key BACK  and do the action
 *
 * @param theChar
 * @return false
 */
static bool s_csi_is_key_back(char theChar)
{
  bool bDone = false;
  if ((gContext->bCliEnable == true) && (theChar == UTIL_TERM_KEY_BACK))
  {
    /* check the backspace and remove the last char */
    if (gContext->iPosCursor)
    {
      uint32_t lenLine = strlen(UTIL_TERM_get_cli_buffer(0)) + 1U; /* +1 for the 0 */
      gContext->iPosCursor--;                                     /* remove previous char */
      memmove(UTIL_TERM_get_cli_buffer(0) + gContext->iPosCursor, UTIL_TERM_get_cli_buffer(0) + gContext->iPosCursor + 1U, lenLine - gContext->iPosCursor - 1U);
      cKeySupport.set_cursor_pos(gContext->iPosCursor);
      UTIL_TERM_puts("\x1B[1P"); /* delete char */
    }
    else
    {
      UTIL_TERM_puts(UTIL_TERM_KEY_BEL);
    }

    bDone = true;
  }
  return bDone;
}


/**
 * @brief Check and manage a CSI sequence, mainly ARROWS
 *
 * @param theChar
 * @return false
 */
static bool s_csi_check_key(void)
{
  bool bDone = true;

  if (strcmp(gContext->tCsi, UTIL_TERM_KEY_HOME) == 0)
  {
    cKeySupport.set_cursor_pos(0);
  }
  else if (strcmp(gContext->tCsi, UTIL_TERM_KEY_END) == 0)
  {
    cKeySupport.set_cursor_pos(strlen(UTIL_TERM_get_cli_buffer(0)));
  }
  else if (strcmp(gContext->tCsi, UTIL_TERM_KEY_RG) == 0)
  {
    if (gContext->iPosCursor < strlen(UTIL_TERM_get_cli_buffer(0)))
    {
      cKeySupport.set_cursor_pos(gContext->iPosCursor + 1U);
    }
    else
    {
      UTIL_TERM_puts(UTIL_TERM_KEY_BEL);
    }
  }
  else if (strcmp(gContext->tCsi, UTIL_TERM_KEY_LF) == 0)
  {
    if (gContext->iPosCursor)
    {
      cKeySupport.set_cursor_pos(gContext->iPosCursor - 1U);
    }
    else
    {
      UTIL_TERM_puts(UTIL_TERM_KEY_BEL);
    }
  }
  else if (strcmp(gContext->tCsi, UTIL_TERM_KEY_UP) == 0)
  {
    if (gContext->iHistory < gContext->szHistory)
    {
      gContext->iHistory++;
      strcpy(UTIL_TERM_get_cli_buffer(0), UTIL_TERM_get_cli_buffer(gContext->iHistory));
      UTIL_TERM_puts("\x1B[M\x1B[1G");
      UTIL_TERM_puts(UTIL_TERM_PROMPT_PROLOGUE);
      UTIL_TERM_puts(UTIL_TERM_get_cli_buffer(0));
      gContext->iPosCursor = strlen(UTIL_TERM_get_cli_buffer(0));
      cKeySupport.set_cursor_pos(gContext->iPosCursor);
    }
    else
    {
      UTIL_TERM_puts(UTIL_TERM_KEY_BEL);
    }
  }
  else if (strcmp(gContext->tCsi, UTIL_TERM_KEY_DN) == 0)
  {
    if (gContext->iHistory >= 1U)
    {
      gContext->iHistory--;
      strcpy(UTIL_TERM_get_cli_buffer(0), UTIL_TERM_get_cli_buffer(gContext->iHistory));
      UTIL_TERM_puts("\x1B[M\x1B[1G");
      UTIL_TERM_puts(UTIL_TERM_PROMPT_PROLOGUE);
      UTIL_TERM_puts(UTIL_TERM_get_cli_buffer(0));
      gContext->iPosCursor = strlen(UTIL_TERM_get_cli_buffer(0));
      cKeySupport.set_cursor_pos(gContext->iPosCursor);
    }
    else
    {
      UTIL_TERM_puts(UTIL_TERM_KEY_BEL);
    }
  }
  else
  {
    bDone = false;
  }

  return bDone;
}


/* Overload key interface */
static const key_support_cb cKeySupport =
{
  .maxHistory           = UTIL_TERM_SIZE_HISTORY,
  .is_key_del_char      = s_csi_is_key_del_char,
  .set_cursor_pos       = s_csi_set_cursor_pos,
  .add_char_at_position = s_csi_add_char_at_position,
  .is_key_back          = s_csi_is_key_back,
  .is_key_csi           = s_csi_check_key,
};
#else
/**
 * @brief key  not supported
 *
 * @param theChar
 * @return true
 * @return false
 */
static bool s_key_not_supported(char theChar)
{
  return false;
}


/**
 * @brief set the current cursor position
 *
 * @param pos
 */
static void s_set_cursor_pos(uint32_t pos)
{
  gContext->iPosCursor = pos;
}


/**
 * @brief set the current cursor position
 *
 * @param pos
 */
static void s_add_char_at_position(char theChar)
{
  if (gContext->iPosCursor < (gContext->lenBuffer - 1U))
  {
    /* if the char is ascii, add it to the Cli buffer */
    if ((theChar >= ' ') && (theChar <= (char)127U))
    {
      uint32_t lenMove = gContext->lenBuffer - gContext->iPosCursor - 1;
      memmove(&UTIL_TERM_get_cli_buffer(0)[gContext->iPosCursor + 1], &UTIL_TERM_get_cli_buffer(0)[gContext->iPosCursor], lenMove);
      UTIL_TERM_get_cli_buffer(0)[gContext->iPosCursor] = theChar;
      gContext->iPosCursor++;
      char tChar[2] = {theChar, '\0'};
      UTIL_TERM_puts(tChar);
    }
  }
}


/**
 * @brief Check the key BACK and do the action
 *
 * @param theChar
 * @return true
 * @return false
 */
static bool s_is_key_back(char theChar)
{
  bool bDone = false;
  if ((gContext->bCliEnable == true) && (theChar == UTIL_TERM_KEY_BACK))
  {
    /* check the backspace and remove the last char */
    if (gContext->iPosCursor)
    {
      uint32_t lenLine = strlen(UTIL_TERM_get_cli_buffer(0)) + 1; /* +1 for the 0 */
      gContext->iPosCursor--;                                     /* remove previous char */
      memmove(UTIL_TERM_get_cli_buffer(0) + gContext->iPosCursor, UTIL_TERM_get_cli_buffer(0) + gContext->iPosCursor + 1, lenLine - gContext->iPosCursor - 1);
      UTIL_TERM_puts("\x8 \x8");
    }
    bDone = true;
  }
  return bDone;
}


/* Overload key interface */
static const key_support_cb cKeySupport =
{
  .maxHistory           = 1,
  .is_key_del_char      = s_key_not_supported,
  .set_cursor_pos       = s_set_cursor_pos,
  .add_char_at_position = s_add_char_at_position,
  .is_key_back          = s_is_key_back,
  .is_key_csi           = NULL,
};


#endif


/**
 * @brief Check and manage CSI keys
 *
 * @param theChar
 * @return true
 * @return false
 */
static bool s_is_key_csi(char theChar)
{
  bool bDone = false;

  if ((gContext->bEsc == true) && (gContext->bCliEnable == true))
  {
    /* if too big, cancel the sequence */
    if (gContext->iIndexCsi > (uint8_t)(sizeof(gContext->tCsi) - 1U))
    {
      gContext->bEsc = false;
      bDone          = true;
    }
    /* accumulate to the buffer */
    gContext->tCsi[gContext->iIndexCsi++] = theChar;
    gContext->tCsi[gContext->iIndexCsi]   = '\0';
    /* read the Csi esc until the end */
    if ((theChar == '[') || (theChar == ';') || (isdigit((int)theChar) != 0)) /*cstat !MISRAC2012-Dir-4.11_h false positive, in this context theChar is an 8 bits and cannot be > 255 after the cast  */
    {
      bDone = true;
    }
    else
    {
      if (cKeySupport.is_key_csi)
      {
        cKeySupport.is_key_csi();
      }
      /* no interception of a csi key, ignore it */
      bDone = true;
      gContext->iIndexCsi = 0;
      gContext->bEsc      = false;

    }
  }
  if ((theChar == UTIL_TERM_KEY_ESC) && (gContext->bCliEnable))
  {
    gContext->iIndexCsi = 0;
    gContext->bEsc      = true;
    bDone               = true;
  }
  return bDone;
}


/**
 * @brief Add  the current line to the history
 *
 */
static void s_add_history(void)
{
  if (strlen(UTIL_TERM_get_cli_buffer(0)) != 0U)
  {
    /* scroll history */
    if ((gContext->szHistory + 1U) < gContext->maxHistory)
    {
      gContext->szHistory++;
    }
    uint32_t szMove = gContext->lenBuffer * gContext->szHistory;
    memmove(UTIL_TERM_get_cli_buffer(1), UTIL_TERM_get_cli_buffer(0), szMove);
  }
  /* reset ESC and history */
  gContext->iHistory    = 0;
  gContext->iIndexCsi = 0;
  gContext->bEsc        = false;
}


/**
 * @brief Reset the current line
 *
 */
static void s_reset_cursor(void)
{
  gContext->iPosCursor           = 0;
  UTIL_TERM_get_cli_buffer(0)[0] = '\0';
}


/**
 * @brief Check if we enter or exit from the CLI state
 *
 * @param theChar
 * @return true
 * @return false
 */
static bool s_is_key_enter_exit_cli(char theChar)
{
  bool bDone = false;
  /* Check enter and exit from the cli */
  if (((theChar == '\r') || (theChar == '\n')))
  {
    UTIL_TERM_puts("\n"); /* echo on the console  */
    if (gContext->bCliEnable)
    {
      /* The CLI was enabled, it is an end of line */
      s_enable_cli(0U);
      /*
          default, logs are allowed in the command
          if a command needs to receive data from the serial
          it must call UTIL_TERM_Mute(UTIL_TERM_MUTE_LOG_FORCE_OFF;
          to prevent transmission issue
      */

      UTIL_TERM_Mute(UTIL_TERM_MUTE_LOG_FORCE_ON);
      s_add_history();
      s_do_line_cmd(UTIL_TERM_get_cli_buffer(0));
      UTIL_TERM_puts("\n");
    }
    else
    {
      /* The LOG is disabled, we enter in the prompt CLI mode  */
      UTIL_TERM_Mute(UTIL_TERM_MUTE_LOG_FORCE_OFF);
      s_enable_cli(1U);
      UTIL_TERM_puts(UTIL_TERM_PROMPT_PROLOGUE);
    }
    s_reset_cursor();
    bDone = true;
  }
  return bDone;
}


/**
 * @brief Check if we exit from the CLI
 *
 * @param theChar
 * @return true
 * @return false
 */
static bool s_is_key_escape(char theChar)
{
  bool bDone = false;
  if ((gContext->bCliEnable == true) && (theChar == UTIL_TERM_KEY_ESC_KEYBOARD))
  {
    /* cancel edition  */
    UTIL_TERM_puts("\n");
    s_reset_cursor();
    UTIL_TERM_Mute(UTIL_TERM_MUTE_LOG_FORCE_ON);
    s_enable_cli(0U);
    bDone = true;
  }
  return bDone;
}


/**
 * @brief Create line buffers according to the history depth
 *
 * @return uint8_t
 */
static int32_t s_create_line_buffer(void)
{
  int32_t error        = UTIL_ERROR_NONE;
  gContext->szHistory  = 0;
  gContext->iHistory   = 0;
  gContext->lenBuffer  = UTIL_TERM_CLI_SIZE;
  gContext->maxHistory = cKeySupport.maxHistory;
  gContext->pBuffer    = s_realloc(NULL, gContext->lenBuffer * gContext->maxHistory);
  if (gContext->pBuffer == NULL)
  {
    error = UTIL_ERROR;
  }

  if (error == UTIL_ERROR_NONE)
  {
    memset(gContext->pBuffer, 0, gContext->lenBuffer * gContext->maxHistory);
  }
  return error;
}


/**
* @brief returns the current buffer
*
*/
static int32_t s_delete_line_buffer(void)
{
  int32_t error = UTIL_ERROR_NONE;
  if (gContext->pBuffer)
  {
    s_free(gContext->pBuffer); /*cstat !MISRAC2012-Dir-4.13_h the pointer is allocated in the class constructor, it is a false positive */
    gContext->pBuffer = NULL;
  }
  return error;
}


/**
* @brief Starts the RX DMA pump
*
*/
static int32_t s_start_rx(void)
{
  gContext->pDmaCurrent     = gContext->pDmaBuffer;
  gContext->iRxBufferOffset = 0;
  return UTIL_UART_StartRxDmaToIdle(gContext->pDmaBuffer, gContext->szDmaBlk);
}


/**
* @brief terminal init
*
*/
int32_t UTIL_TERM_create(void)
{
  int32_t error = UTIL_ERROR_NONE;

  /* cycles measure initialization */
  UTIL_TERM_cycleMeasure_Init();
  UTIL_TERM_cycleMeasure_Reset();
  error = UTIL_TERM_TASK_create();

  UTIL_TERM_Mute(UTIL_TERM_MUTE_LOG_FORCE_ON);
  s_enable_cli(0U);
  if (error == UTIL_ERROR_NONE)
  {
    gContext->bInitDone                     = true;
    gContext->szDmaBlk                      = UTIL_UART_RX_BUFFER_SIZE;
    gContext->pHdleUart->RxHalfCpltCallback = s_dma_uart_rx_half_cb;
    gContext->pHdleUart->RxCpltCallback     = s_dma_uart_rx_cpl_cb;
    gContext->pHdleUart->RxEventCallback    = s_dma_uart_event_cb;
    error = s_start_rx();
  }

  if (error == UTIL_ERROR_NONE)
  {
    error = s_create_line_buffer();
  }

  if (error == UTIL_ERROR_NONE)
  {
    UTIL_TERM_Initialized(gContext);
    UTIL_TERM_Enable(true);
  }
  else
  {
    UTIL_TERM_printf("UTIL_TERM_TASK_create error\n");
    if (error == UTIL_ERROR_ALLOC)
    {
      UTIL_TERM_printf("ERROR! UART RX size two big. Please increase DMA UART buffer\n");
    }
  }

  return error;
}


/**
* @brief this weak function provides the opportunity record incoming data
* @param gContext  Term instance
*
*/
__weak void UTIL_TERM_Initialized(UTIL_TERM_t *gContext)
{
}


/**
* @brief this weak function provides the opportunity to record the terminal console in
*
*/
__weak void UTIL_TERM_parse_in_recorder(uint8_t *pBuffer, uint32_t szBuffer)
{
}


/**
* @brief this weak function provides the opportunity to record the terminal console out
*
*/
__weak void UTIL_TERM_parse_out_recorder(uint8_t *pBuffer, uint32_t szBuffer)
{
}


/**
* @brief return true if the terminal is muted
*
*/
bool UTIL_TERM_IsLogMuted(void)
{
  return UTIL_UART_IsLogMuted();
}


void UTIL_TERM_Mute(UTIL_TERM_MUTE_t state)
{
  switch (state)
  {
    default:
    case UTIL_TERM_MUTE_LOG_FORCE_ON:
      UTIL_UART_Mute(UTIL_UART_MUTE_FORCE_ON);
      break;
    case UTIL_TERM_MUTE_LOG_FORCE_OFF:
      UTIL_UART_Mute(UTIL_UART_MUTE_FORCE_OFF);
      break;
    case UTIL_TERM_MUTE_RX_ON:
      gContext->rxMute = false;
      break;
    case UTIL_TERM_MUTE_RX_OFF:
      gContext->rxMute = true;
      break;
  }
}


/**
* @brief terminal ending & deinit
*
*/
int32_t UTIL_TERM_delete(void)
{
  /* Stop Rx processing*/
  s_enable_cli(TRUE);
  s_delete_line_buffer();

  UTIL_TERM_TASK_terminate();
  /* Delete rx buffer*/
  s_free(gContext);

  return UTIL_ERROR_NONE;
}


/**
* @brief returns the context
*
*/
UTIL_TERM_t *UTIL_TERM_get_context(void)
{
  return gContext;
}


/**
* @brief add a listener to the task idle state
* @param cb idle state callback
*/
uint8_t UTIL_TERM_add_Idle_listener(UTIL_TERM_IDLE_CB cb)
{
  uint8_t result = FALSE;
  if (gContext->szIdleCb < UTIL_TERM_IDLE_CB_MAX)
  {
    gContext->tIdleCb[gContext->szIdleCb] = cb;
    gContext->szIdleCb++;
    result = TRUE;
  }
  return result;
}


/**
* @brief Calls listener to idle when there is no activity
*
* @param pCookie the opaque parameter
* @return error
*/
static void UTIL_TERM_Idle(void)
{
  for (uint8_t indexListener = 0U; indexListener < gContext->szIdleCb; indexListener++)
  {
    gContext->tIdleCb[indexListener]();
  }
}


/**
* @brief Uart Rx configuration setting
*
* @param huart the uart instance
*/
int32_t UTIL_TERM_set_uart_rx(UART_HandleTypeDef *const huart)
{
  int32_t error = UTIL_ERROR_NONE;

  gContext = s_realloc(NULL, sizeof(*gContext));
  if (gContext == NULL)
  {
    UTIL_TERM_printf("gContext allocation error\n");
    error = UTIL_ERROR_ALLOC;
  }
  if (error == UTIL_ERROR_NONE)
  {
    memset(gContext, 0, sizeof(*gContext));
    gContext->pHdleUart                = huart;
    gContext->pHdleUart->ErrorCallback = s_rx_error_cb;
    gContext->hRxDma                   = huart->hdmarx;
    gContext->pDmaBuffer               = UTIL_UART_GetRxBuffer();
    if (gContext->pDmaBuffer == NULL)
    {
      UTIL_TERM_printf("gContext->pDmaBuffer allocation error\n");
      error = UTIL_ERROR_ALLOC;
    }
  }

  return error;
}


/**
* @brief Put a string in the terminal
*
*/
void UTIL_TERM_puts(char *const pString)
{
  UTIL_UART_fputs_force(pString);
  UTIL_TERM_parse_out_recorder((uint8_t *)pString, strlen(pString));
}


/**
* @brief Direct terminal printf
* @param printf format and arguments.
* @retval None.
*/
void UTIL_TERM_printf(char *pFormat, ...)
{
  va_list args;
  va_start(args, pFormat);
  if (vsnprintf(gContext->tFormat, sizeof(gContext->tFormat), pFormat, args) > 0)
  {
    UTIL_TERM_puts(gContext->tFormat);
  }
  va_end(args);
}


/**
* @brief Direct terminal printf with ending carriage-return
* @param printf format and arguments.
* @retval None.
*/
void UTIL_TERM_printf_cr(char *pFormat, ...)
{
  va_list args;
  va_start(args, pFormat);
  if (vsnprintf(gContext->tFormat, sizeof(gContext->tFormat), pFormat, args) > 0)
  {
    UTIL_TERM_puts(gContext->tFormat);
  }
  UTIL_TERM_puts("\n");
  va_end(args);
}


/**
* @brief print a text even if the mute is set, mainly used for errors
* @param printf format and arguments.
* @retval None.
*/
void UTIL_TERM_printf_forced(char *pFormat, ...)
{
  va_list args;
  va_start(args, pFormat);
  UTIL_TERM_Mute(UTIL_TERM_MUTE_LOG_FORCE_ON);
  if (vsnprintf(gContext->tFormat, sizeof(gContext->tFormat), pFormat, args) > 0)
  {
    UTIL_TERM_puts(gContext->tFormat);
  }
  va_end(args);
}


/**
* @brief print a text with ending carriage-return even if the mute is set, mainly used for errors
* @param printf format and arguments.
* @retval None.
*/
void UTIL_TERM_printf_forced_cr(char *pFormat, ...)
{
  va_list args;
  va_start(args, pFormat);
  UTIL_TERM_Mute(UTIL_TERM_MUTE_LOG_FORCE_ON);
  if (vsnprintf(gContext->tFormat, sizeof(gContext->tFormat), pFormat, args) > 0)
  {
    UTIL_TERM_puts(gContext->tFormat);
  }
  UTIL_TERM_puts("\n");
  va_end(args);
}


/* we try to detect  an isolated ESC that doesn't exist in terminal VT100, but it it sent as an unique ESC keyboard by them
   we use this key to exit the Cli mode
   if the buffer has an unique ESC we translate the char to a pseudo code handling the key ESC of the keyboard
   if a sequence ESC is not followed by '[' that is the open Sequence, we translate the key too
  */
static void UTIL_TERM_TASK_translate_esc(uint8_t *pBuffer, uint32_t szCopy)
{
  if (gContext->bCliEnable)
  {
    if (pBuffer[0] == (uint8_t)UTIL_TERM_KEY_ESC)
    {
      if ((szCopy == 1U) || ((szCopy >= 2U) && (pBuffer[1] != (uint8_t)'[')))
      {
        *pBuffer = (uint8_t)UTIL_TERM_KEY_ESC_KEYBOARD;
      }
    }
  }
}


static bool UTIL_TERM_parse_key_action(char theChar)
{
  bool bDone = s_is_key_csi(theChar);

  if (bDone == false)
  {
    bDone = s_is_key_escape(theChar);
  }
  if (bDone == false)
  {
    bDone = s_is_key_enter_exit_cli(theChar);
  }

  if (bDone == false)
  {
    bDone = cKeySupport.is_key_del_char(theChar);
  }

  /* check the back space to delete a char */
  if (bDone == false)
  {
    bDone = cKeySupport.is_key_back(theChar);
  }
  return bDone;
}


/**
* @brief Add a char in the buffer or manage it
*
*/
void UTIL_TERM_parse_char(char const theChar)
{
  bool bDone = UTIL_TERM_parse_key_action(theChar);
  if (gContext->bCliEnable && (bDone == false))
  {
    cKeySupport.add_char_at_position(theChar);
  }
}


/**
* @brief MUST be defined by user ; must return pointer on custom commands
*
* @param argc  num args
* @param argv  args list
*/
__weak UTIL_TERM_cmd_str_entry_t const *UTIL_TERM_getCustomCommands(void)
{
  UTIL_TERM_printf_cr("ERROR! calling the weak function UTIL_TERM_getCustomCommands; it must be user defined");
  return NULL;
}


/**
* @brief Display terminal help
*
* @param argc  num args
* @param argv  args list
*/
void UTIL_TERM_cmd_help(int argc, char *argv[])
{
  UTIL_TERM_cmd_str_entry_t const *pCmds = UTIL_TERM_getCustomCommands();
  for (int cptCmd = 0; pCmds[cptCmd].name != NULL; cptCmd++)
  {
    UTIL_TERM_printf_cr("%-018s : %-40s : %s", pCmds[cptCmd].name, (pCmds[cptCmd].params == NULL) ? "" : pCmds[cptCmd].params, pCmds[cptCmd].comments);
  }
}


/**
* @brief Receive a raw file from the terminal
*
* @param pBuffer the buffer
* @param szBuffer the buffer size
*/
int32_t UTIL_TERM_receive_raw_file(uint8_t *pBuffer, uint32_t const szBuffer, uint32_t const syncTmo, uint32_t *pOutSzPaste)
{
  int32_t  ret         = UTIL_ERROR_NONE;
  uint32_t maxSyncTime = syncTmo / UTIL_TERM_SYNC_TIMEOUT; /* sync time*/
  uint32_t szPaste     = 0;
  uint32_t szBlock     = 0;
  bool     bContinue   = true;
  size_t   szRemaining;

  /* wait a transmission, log off */
  UTIL_TERM_Mute(UTIL_TERM_MUTE_LOG_FORCE_OFF);

  if (pOutSzPaste)
  {
    *pOutSzPaste = 0;
  }
  gContext->bRawTransfer = true;
  if (ret == UTIL_ERROR_NONE)
  {
    ret = UTIL_ERROR_TIMEOUT;
    /* wait for the transmission start */

    while ((maxSyncTime != 0U) && (ret == UTIL_ERROR_TIMEOUT))
    {
      ret = UTIL_TERM_TASK_wait_data(pBuffer, 1, UTIL_TERM_SYNC_TIMEOUT, &szRemaining);
      if (ret != UTIL_ERROR_TIMEOUT)
      {
        /* 1 byte read, the transmission is started */
        szPaste++;
        pBuffer++;
        break;
      }
      /* send an echo */
      UTIL_TERM_puts("C");
    }
    maxSyncTime--;
  }

  if (ret == UTIL_ERROR_NONE)
  {
    if (maxSyncTime == 0U)
    {
      /* timeout , exit */
      ret = UTIL_ERROR_TIMEOUT;
    }
  }

  if (ret == UTIL_ERROR_NONE)
  {
    /* Now receive the remaining until no char during x  ms that mark the end of the file */
    bContinue = true;
    while (bContinue)
    {
      ret = UTIL_TERM_TASK_wait_data(pBuffer, 1, UTIL_TERM_TIMEOUT_EOF, &szRemaining);
      if (ret == UTIL_ERROR_NONE)
      {
        szPaste++;
        pBuffer++;
        /* check the buffer limit */
        if ((szPaste + szBlock + 1U) > szBuffer)
        {
          ret       = UTIL_ERROR_ALLOC;
          bContinue = false;
        }
      }
      else
      {
        if (ret == UTIL_ERROR_TIMEOUT)
        {
          ret       = UTIL_ERROR_NONE;
          bContinue = false;
        }
      }
    }
  }

  if (pOutSzPaste)
  {
    *pOutSzPaste = szPaste;
  }
  gContext->bRawTransfer = false;
  UTIL_TERM_Mute(UTIL_TERM_MUTE_LOG_FORCE_ON);

  return ret;
}


/**
* @brief Reset board
*
* @param argc  num args
* @param argv  args list
*/
void UTIL_TERM_cmd_reboot(int argc, char *argv[])
{
  if (argc != 1)
  {
    UTIL_TERM_printf("Error:Wrong Command parameters\n");
  }
  else
  {
    UTIL_TERM_printf("reboot board\n");
    NVIC_SystemReset();
  }
}


/**
* @brief Add the char to the Receive buffer, realloc the buffer if needed
*
*/
static int32_t UTIL_TERM_add_buffer(uint8_t incoming, char_t **ppBuffer, uint32_t *pcountBuffer, uint32_t *pSzBuffer)
{
  char_t *pBuffer = *ppBuffer;

  if ((*pcountBuffer) >= (*pSzBuffer))
  {
    *pSzBuffer = (*pcountBuffer) + UTIL_TERM_BUFFER_GROW;
    char_t *pNewBuff = s_realloc(pBuffer, *pSzBuffer);
    if (pNewBuff == NULL)
    {
      s_free(pBuffer);
      pBuffer = NULL;
      return UTIL_ERROR_ALLOC;
    }
    pBuffer = pNewBuff;
  }

  pBuffer[*pcountBuffer] = (char_t)incoming;
  (*pcountBuffer)++;
  *ppBuffer = pBuffer;

  return UTIL_ERROR_NONE;
}


/**
* @brief Receive a raw file from the terminal
*
* @param pBuffer the buffer
* @param szBuffer the buffer size
*/
int32_t UTIL_TERM_receive_file(char_t **pFileBuffer, uint32_t *pSzFileBuffer, uint32_t const syncTmo)
{
  int32_t  ret          = UTIL_ERROR_NONE;
  uint32_t szBuffer     = 0U;
  uint32_t countBuffer  = 0U;
  char_t  *pBuffer      = NULL;
  uint32_t maxSyncTime  = syncTmo / UTIL_TERM_SYNC_TIMEOUT; /* sync time*/
  bool     bContinue    = true;
  size_t   szRemaining  = 0UL;
  uint8_t  incomingChar = 0U;

  if ((pFileBuffer == NULL) || (pSzFileBuffer == NULL))
  {
    return UTIL_ERROR;
  }
  *pFileBuffer   = NULL;
  *pSzFileBuffer = 0UL;

  /* wait a transmission, log off */
  UTIL_TERM_Mute(UTIL_TERM_MUTE_LOG_FORCE_OFF);

  gContext->bRawTransfer = true;
  ret = UTIL_ERROR_TIMEOUT;
  /* wait for the transmission start (at least 1 char) */
  while ((maxSyncTime != 0U) && (ret == UTIL_ERROR_TIMEOUT))
  {
    ret = UTIL_TERM_TASK_wait_data(&incomingChar, 1, UTIL_TERM_SYNC_TIMEOUT, &szRemaining);
    if (ret != UTIL_ERROR_TIMEOUT)
    {
      break;
    }
    maxSyncTime--;
  }

  if (ret == UTIL_ERROR_NONE)
  {
    if (maxSyncTime == 0U)
    {
      /* timeout , exit */
      ret = UTIL_ERROR_TIMEOUT;
    }
  }

  if (ret == UTIL_ERROR_NONE)
  {
    /* the char read during the sync must be added to the stream  */
    ret = UTIL_TERM_add_buffer(incomingChar, &pBuffer, &countBuffer, &szBuffer);
  }

  if (ret == UTIL_ERROR_NONE)
  {
    bContinue = true;
    /* Now receive the remaining until timeout that mark the end of the file or the max size reached*/
    while (bContinue)
    {
      ret = UTIL_TERM_TASK_wait_data(&incomingChar, 1, UTIL_TERM_TIMEOUT_EOF, &szRemaining);
      if (ret == UTIL_ERROR_NONE)
      {
        if (incomingChar == UTIL_CHAR_EOF)
        {
          /* max char reached, stop the stream */
          ret       = UTIL_ERROR_NONE;
          bContinue = false;
          break;
        }

        ret       = UTIL_TERM_add_buffer(incomingChar, &pBuffer, &countBuffer, &szBuffer);
        bContinue = (ret == UTIL_ERROR_NONE);
      }
      else
      {
        if (ret == UTIL_ERROR_TIMEOUT)
        {
          ret       = UTIL_ERROR_NONE;
          bContinue = false;
        }
      }
    }
  }

  if (ret == UTIL_ERROR_NONE)
  {
    UTIL_TERM_add_buffer(0, &pBuffer, &countBuffer, &szBuffer);
    (*pFileBuffer)   = pBuffer;
    (*pSzFileBuffer) = countBuffer - 1UL;
  }

  gContext->bRawTransfer = false;
  UTIL_TERM_Mute(UTIL_TERM_MUTE_LOG_FORCE_ON);

  return ret;
}


/* Static Functions Definition ------------------------------------------------------*/

/**
* @brief Look for an command instance
*
* @param command the command name
* @return UTIL_TERM_cmd_str_entry_t*  the command instance
*/
static const UTIL_TERM_cmd_str_entry_t *s_find_entry(char *command)
{
  UTIL_TERM_cmd_str_entry_t const *pCmd = UTIL_TERM_getCustomCommands();

  while (pCmd != NULL)
  {
    if (pCmd->name == NULL)
    {
      /* end of custom commands => command not found */
      pCmd = NULL;
    }
    else if (strcmp(command, pCmd->name) == 0)
    {
      /* command found */
      break;
    }
    else
    {
      /* next command */
      pCmd++;
    }
  }

  return pCmd;
}


static bool s_search_end_of_word(char **const ppLine, const char end_of_word_char)
{
  char *pLine = *ppLine;
  bool  eol   = false;

  while (*pLine != end_of_word_char)
  {
    if ((*pLine == '\0') || (*pLine == '\r') || (*pLine == '\n'))
    {
      eol = true;
      break;
    }
    pLine++;
  }
  *pLine = '\0'; // set end of word
  pLine++;       // skip end_of_word_char or eol (already replaced with end of word: see line above)
  *ppLine = pLine;

  return eol;
}


static void s_do_line_cmd(char *pLine)
{
  char    *tListWord[UTIL_TERM_MAX_PARAM];
  uint32_t wordcpt = 0U;
  bool     eol     = false;

  while ((!eol) && (wordcpt < UTIL_TERM_MAX_PARAM))
  {
    /* Eliminate starting spaces */
    while (*pLine == ' ')
    {
      pLine++;
    }

    if (*pLine == '\0')
    {
      eol = true;
    }
    else if (*pLine == '"')
    {
      // word starting with '"'
      pLine++; // skip starting '"'
      tListWord[wordcpt] = pLine;
      wordcpt++;
      eol = s_search_end_of_word(&pLine, '"');
      if (!eol) // not eol => ending '"' found => is there an eol after ending '"' ?
      {
        eol = (*pLine == '\0') || (*pLine == '\r') || (*pLine == '\n');
      }
    }
    else
    {
      // word not starting with "'"
      tListWord[wordcpt] = pLine;
      wordcpt++;
      eol = s_search_end_of_word(&pLine, ' ');
    }
  }

  if (wordcpt > 0U)
  {
    UTIL_TERM_cmd_str_entry_t const *pEntry = s_find_entry(tListWord[0]);

    if (pEntry == NULL)
    {
      UTIL_TERM_printf("Command \"%s\" not found\n", tListWord[0]);
    }
    else
    {
      pEntry->command(wordcpt, tListWord);
    }
  }
}


/**
* @brief Uart error callback
*
* @param pHdleUart the uart instance
*/
static void s_rx_error_cb(UART_HandleTypeDef *pHdleUart)
{
  if (gContext->bInitDone)
  {
    if (pHdleUart == gContext->pHdleUart)
    {
      /*  Just signal it for debug */
      UTIL_TERM_printf_cr("UART Error: %d", pHdleUart->ErrorCode);
      /* on error the dma is stopped, so, restart it */
      s_start_rx();

    }
  }
}


/**
* @brief Inject a payload in the Rx queue
*
**/

int32_t UTIL_TERM_inject_rx(uint8_t *pBuffer, uint32_t const szBuffer)
{
  return UTIL_TERM_TASK_send_data(pBuffer, szBuffer);
}


/* @brief Inject a payload in the Rx queue
*
**/
static void s_dma_uart_inject(uint8_t *pData, uint32_t szData)
{
  if (gContext->rxMute == 0U)
  {
    UTIL_TERM_TASK_translate_esc(pData, szData);
    UTIL_TERM_TASK_send_data_from_isr(pData, szData);
  }
  UTIL_TERM_parse_in_recorder(pData, szData);
}


/**
* @brief Signal a full DMA
*
* @param pHdleUart
**/
static void s_dma_uart_rx_cpl_cb(struct __UART_HandleTypeDef *huart)
{
  if (gContext->bInitDone)
  {
    uint8_t  *pBuffer = gContext->pDmaCurrent + gContext->iRxBufferOffset;
    uint32_t szBuffer = ((uint32_t)gContext->szDmaBlk / 2U) - gContext->iRxBufferOffset;
    if (szBuffer)
    {
      s_dma_uart_inject(pBuffer, szBuffer);
    }
    gContext->iRxBufferOffset = 0;
    gContext->pDmaCurrent     = gContext->pDmaBuffer;
  }
}


/**
* @brief Signal a Half DMA
*
* @param pHdleUart
**/
static void s_dma_uart_rx_half_cb(struct __UART_HandleTypeDef *huart)
{
  if (gContext->bInitDone)
  {
    uint8_t   *pBuffer = gContext->pDmaCurrent + gContext->iRxBufferOffset;
    uint32_t szBuffer = ((uint32_t)gContext->szDmaBlk / 2U) - gContext->iRxBufferOffset;
    if (szBuffer)
    {
      s_dma_uart_inject(pBuffer, szBuffer);
    }
    gContext->pDmaCurrent     = gContext->pDmaBuffer + (gContext->szDmaBlk / 2U);
    gContext->iRxBufferOffset = 0U;
  }
}


/**
* @brief Signal a full DMA or IDLE state
*
* @param pHdleUart
* @param position position  in the full buffer
**/
static void s_dma_uart_event_cb(struct __UART_HandleTypeDef *huart, uint16_t position)
{
  if (gContext->bInitDone)
  {
    /* compute the position in the circular buffer */

    if (position  >= (gContext->szDmaBlk / 2U))
    {
      /* if the position is > sz/2, the target is in second part of the buffer */
      position -= gContext->szDmaBlk / 2U;
    }

    uint8_t *pBuffer = gContext->pDmaCurrent + gContext->iRxBufferOffset;;
    /* compute the size produced since the last dma it our UART Idle IT */
    uint32_t szBuffer = position - gContext->iRxBufferOffset;
    if (szBuffer)
    {
      /* if there are data, fill it */
      s_dma_uart_inject(pBuffer, szBuffer);
      gContext->iRxBufferOffset += szBuffer;
    }
  }
}


/**
* @brief enable cli
*
* @param bEnable  true or false
*/
static void s_enable_cli(uint8_t bEnable)
{
  gContext->bCliEnable = (bool)bEnable;
}


/**
* @brief Command processing thread
*
* @return error
*/
int32_t UTIL_TERM_process(void)
{
  int32_t error = UTIL_ERROR_NONE;

  if (gContext->bTerminalEnabled)
  {
    bool    done = false;
    uint8_t data;
    size_t  szRemaining;

    if (gContext->bRawTransfer == false)
    {
      error = UTIL_TERM_TASK_wait_data(&data, 1UL, UTIL_TERM_IDLE_TIMEOUT, &szRemaining);
      if (error == UTIL_ERROR_NONE)
      {
        UTIL_TERM_cycleMeasure_Start();
        UTIL_TERM_parse_char((char)data);
        UTIL_TERM_cycleMeasure_Stop();
        done = true;
      }
    }
    if (done == false)
    {
      UTIL_TERM_Idle();
    }
  }
  else
  {
    error = UTIL_ERROR;
  }

  return error;
}


/**
* @brief Enable the terminal
*
* @param bEnable true or false
* @return error
*/
int32_t UTIL_TERM_Enable(bool bEnable)
{
  int32_t error = UTIL_ERROR;
  if (gContext)
  {
    gContext->bTerminalEnabled = bEnable;
    error                      = UTIL_ERROR_NONE;
  }
  return error;
}
