/**
******************************************************************************
* @file          plugin_terminal_cmd.h
* @author        MCD Application Team
* @brief         Manage terminal communication
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


#ifndef service_terminal_cmd_h
#define service_terminal_cmd_h

#ifdef __cplusplus
extern "C"
{
#endif
/* Includes ------------------------------------------------------------------*/
#include "stm32_term.h"
#include "stm32_term_cmd.h"

/* Exported types ------------------------------------------------------------*/
typedef void (*st_plugin_cmd_cb)(int argc, char_t *argv[]);

typedef struct t_state_flags
{
  const char_t *pName;
  uint32_t      msk;
} state_flags_t;

struct livetune_instance;
struct livetune_instance *UTIL_TERM_get_instance(void);
void                         plugin_terminal_cmd_ack(uint8_t result, char_t *pSucessCmdMsg, char_t *pErrorCmdMsg, char_t *pDesc, char_t *pExtra);
void                         plugin_terminal_cmd_ack_hook(uint8_t result, char_t *pSucessCmdMsg, char_t *pErrorCmdMsg, char_t *pDesc);
uint32_t                     plugin_terminal_cmd_parse_valid_event(ST_Handle hInst, ST_Message_Event evt, st_message_param wParam, st_message_param pParam);
uint32_t                     plugin_terminal_cmd_parse_common_event(ST_Handle hInst, uint32_t evt, st_message_param wParam, st_message_param pParam);
uint32_t                     plugin_terminal_cmd_parse_designer_event(ST_Handle hInst, uint32_t evt, st_message_param wParam, st_message_param pParam);
void                         plugin_terminal_cmd_stop_pipe(void);
int32_t                      plugin_terminal_cmd_add_event_delegation(st_plugin_cb cb);
const struct t_state_flags *plugin_terminal_cmd_get_flags(void);
uint8_t                      plugin_terminal_send_config(void);



/* Exported constants --------------------------------------------------------*/
/* Exported defines   --------------------------------------------------------*/
/* Exported macros ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

#ifdef __cplusplus
};
#endif
#endif


