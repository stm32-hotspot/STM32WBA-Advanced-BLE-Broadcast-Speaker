/**
******************************************************************************
* @file          livetune_pipe.h
* @author        MCD Application Team
* @brief         manage the audio pipe
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


#ifndef livetune_pipe_h
#define livetune_pipe_h


#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "st_registry.h"
#include "st_json.h"
#include "livetune_db.h"
#include "acSdk.h"
//#include "st_ac_types.h"


/* Exported defines --------------------------------------------------------*/

#ifndef ST_STACK_MAX
#define ST_STACK_MAX 4
#endif

#ifndef LIVETUNE_VAR_NAME_MAX
#define LIVETUNE_VAR_NAME_MAX 100
#endif

#ifndef ST_PIPE_GEN_CODE_BLK_SIZE
#define ST_PIPE_GEN_CODE_BLK_SIZE (2UL * 1024UL)
#endif

#define ST_PIPE_HEADER_LIST_MAX  (10U)
#define UINT_TO_STRING(a)        a, #a
#define PTR_TO_STRING(a)         ((uint32_t)a), #a
#define LIVETUNE_STRINGIFY(a) #a


/* Exported types ------------------------------------------------------------*/
typedef struct livetune_pipe_state
{
  int32_t iNbStack;
  int8_t  tStack[ST_STACK_MAX];
} livetune_pipe_state;
typedef enum livetune_pipe_log_type
{
  LIVETUNE_LOG_CODE,
} livetune_pipe_log_type;

/**
* @brief Key value converter
*
*/
typedef struct st_pipe_key_value
{
  union
  {
    uint32_t ivalue;
    void    *pValue;
  } value;
  const char *pKey;
} st_pipe_key_value;


typedef enum st_pipe_builder_cmd
{
  LIVETUNE_PIPE_CONSTRUCTOR,
  LIVETUNE_PIPE_START,
  LIVETUNE_PIPE_PROLOG,
  LIVETUNE_PIPE_PRE_INIT_GRAPH,
  LIVETUNE_PIPE_POST_INIT_GRAPH,
  LIVETUNE_PIPE_INIT_GRAPH_ELEMENT,
  LIVETUNE_PIPE_INIT_GRAPH_ATTACH_CNX,
  LIVETUNE_PIPE_DESTRUCTOR
} st_pipe_builder_cmd;

struct livetune_helper_builder;

typedef struct livetune_pipe
{
  st_mutex               hLock;
  livetune_db        *pDataBase;
  int8_t                 bPipeState;
  int8_t                bBuilding;
  livetune_pipe_state hStateStack;
  livetune_db_list   *pProcessList;
  acPipe                 hPipe;
  int8_t                 bGenerateCode;
  char_t                *pGenerateCode;
  uint32_t               szGenerateCode;
  uint32_t               szGenerateCodeMax;
  char_t                 tScratch[1024];
  uint32_t               buildErrorCount;

  const char_t *tHeaderList[ST_PIPE_HEADER_LIST_MAX];
  uint32_t      szHeaderList;
  int8_t        bRtGraphConstruction;
  livetune_builder_result(*livetune_helper_builder_default_cb)(struct livetune_helper_builder *pBuilder);



} livetune_pipe;


/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
ST_Result                  livetune_pipe_create(livetune_pipe *pHandle, livetune_db *pDataBase);
ST_Result                  livetune_pipe_delete(livetune_pipe *pHandle);
ST_Result                  livetune_pipe_state_pop(livetune_pipe *pHandle, livetune_db_pipe_cmd newState);
ST_Result                  livetune_pipe_state_push(livetune_pipe *pHandle);
ST_Result                  livetune_pipe_start(livetune_pipe *pHandle);
ST_Result                  livetune_pipe_stop(livetune_pipe *pHandle);
int8_t                     livetune_pipe_is_started(livetune_pipe *pHandle);
void                       livetune_pipe_log(livetune_pipe *pHandle, livetune_pipe_log_type type, char_t *pFormat, ...);
void                       livetune_pipe_add_log(livetune_pipe *pHandle, livetune_pipe_log_type type, char_t *pText);
void                       livetune_pipe_add_printf(livetune_pipe *pHandle, livetune_pipe_log_type type, char_t *pFormat, ...);
ST_Result                  livetune_pipe_enable_generate_code(livetune_pipe *pHandle, int8_t state);
ST_Result                  livetune_pipe_build_order(livetune_pipe *pHandle);
livetune_builder_result    livetune_pipe_notify(livetune_pipe *pHandle, st_pipe_builder_cmd cmd);
const char_t              *livetune_pipe_find_ivalue_string(const st_pipe_key_value **pList, uint32_t ivalue);
uint32_t                   livetune_pipe_get_ivalue(const st_pipe_key_value **pList, uint32_t listIndex, int32_t index);
const char_t              *livetune_pipe_get_ivalue_string(const st_pipe_key_value **pList, uint32_t listIndex, int32_t index);
int8_t                     livetune_pipe_set_rt_graph_construction(livetune_pipe *pHandle, int8_t bState);
int8_t                     livetune_pipe_get_rt_graph_construction(livetune_pipe *pHandle);

#ifdef __cplusplus
};
#endif
#endif

