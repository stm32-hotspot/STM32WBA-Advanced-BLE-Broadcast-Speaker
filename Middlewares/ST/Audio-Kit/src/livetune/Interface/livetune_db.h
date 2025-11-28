/**
******************************************************************************
* @file          livetune_db.h
* @author        MCD Application Team
* @brief         manage the db
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


#ifndef livetune_db_h
#define livetune_db_h


#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/


#include "st_base.h"
#include "st_message.h"
#include "st_os.h"
#include "st_os_mem.h"
#include "livetune_db_conf.h"
#include <math.h>




/* Exported defines   --------------------------------------------------------*/


#define LIVETUNE_CHUNK_SCRATCH_SIZE  4096
#define CNX_PARAM_SKIP_MARKER           "NO_CHANGE"
#define LIVETUNE_AC_BUS              "acChunk"


#ifndef LIVETUNE_INSTANCE_EXTRA_STRUCT
#define LIVETUNE_INSTANCE_EXTRA_STRUCT
#endif

#ifndef LIVETUNE_INTERFACE_EXTRA_STRUCT
#define LIVETUNE_INTERFACE_EXTRA_STRUCT
#endif

#ifndef LIVETUNE_CNX_EXTRA_STRUCT
#define LIVETUNE_CNX_EXTRA_STRUCT
#endif

#ifndef LIVETUNE_BASE_EVT
#define LIVETUNE_BASE_EVT 100U
#endif




/* Exported macro------------------------------------------------------------*/

#define LIVETUNE_VALUE(p, k)            (strtod(livetune_db_list_get(&(p)->pParams, ((uint32_t)k))->pInstParam->pParamValue, NULL))
#define LIVETUNE_INT(p, k)              ((int32_t)atoi(livetune_db_list_get(&(p)->pParams, ((uint32_t)k))->pInstParam->pParamValue))
#define LIVETUNE_STRING(p, k)           (livetune_db_list_get(&(p)->pParams, ((uint32_t)k))->pInstParam->pParamValue)
#define LIVETUNE_NAME(p, k)             (livetune_db_list_get(&(p)->pParams, ((uint32_t)k))->pInstParam->pName)
#define LIVETUNE_SET(p, k, format, ...) (livetune_db_param_change(&(p)->tParams[(k)], (format), __VA_ARGS__))
#define LIVETUNE_PARAM_TYPE(p, k)       (livetune_db_list_get(&(p)->pParams, ((uint32_t)k))->pInstParam->iType)


#define ST_INFO_TYPE_ELEMENT (0U)
#define ST_INFO_TYPE_PINOUT  (1U)
#define ST_INFO_TYPE_PININ   (2U)
#define ST_PIN_IN            (0U)
#define ST_PIN_OUT           (1U)


/* Exported types ------------------------------------------------------------*/

/* pre-definitions */

struct json_instance_t;
struct livetune_pipe;
struct livetune_db_element;
struct livetune_db_element_params;
struct livetune_db_instance;
struct livetune_db;
struct livetune_db_instance_param;
struct livetune_db_instance_cnx;
struct livetune_db_list;

typedef struct livetune_db_list livetune_db_instance_pins_def;
typedef struct livetune_db_list livetune_db_instance_cnx_list;
typedef struct livetune_db_list livetune_db_element_pin_def;
typedef int16_t                    st_audio_render_sample;
typedef char_t                    *livetune_type;


typedef enum livetune_db_pipe_reason
{
  ST_PIPE_REASON_CREATE,
  ST_PIPE_REASON_DELETE,
  ST_PIPE_REASON_UPDATE,
  ST_PIPE_REASON_CLEANUP,
} livetune_db_pipe_reason;

typedef enum livetune_db_pipe_cmd
{
  ST_PIPE_PUSH_STOP,
  ST_PIPE_POP,
  ST_PIPE_NO_CHANGE
} livetune_db_pipe_cmd;

typedef union livetune_info_param
{
  struct
  {
    uint8_t indexParam;
    struct
    {
      unsigned infoType : 6;
      unsigned updateType : 2;
    };
    uint8_t indexPinDef;
    uint8_t indexCnx;
  } byField;
  uint32_t byValue;
} livetune_info_param;

#define ST_EVT_UPDATE_PARAM        (LIVETUNE_BASE_EVT)
#define ST_EVT_REGISTER_ELEMENTS   (LIVETUNE_BASE_EVT + 1U)
#define ST_EVT_REGISTERED_ELEMENTS (LIVETUNE_BASE_EVT + 2U)
#define ST_EVT_LOAD                (LIVETUNE_BASE_EVT + 3U)
#define ST_EVT_SAVE                (LIVETUNE_BASE_EVT + 4U)
#define ST_EVT_NEW                 (LIVETUNE_BASE_EVT + 5U)
#define ST_EVT_START_PIPE          (LIVETUNE_BASE_EVT + 6U)
#define ST_EVT_STOP_PIPE           (LIVETUNE_BASE_EVT + 7U)
#define ST_EVT_PIPE_BUILT          (LIVETUNE_BASE_EVT + 8U)
#define ST_EVT_MAX                 (LIVETUNE_BASE_EVT + 9U)
#define LIVETUNE_DB_VERSION              "v1.0.0"



typedef enum livetune_builder_result
{
  ST_BUILDER_OK,
  ST_BUILDER_ERROR,
  ST_BUILDER_NOT_IMPLEMENTED,
  ST_BUILDER_DEFAULT
} livetune_builder_result;


typedef uint32_t (*LIVETUNE_DB_MSG_CB)(struct livetune_db *pHandle, uint32_t evt, st_message_param lparam);
typedef void (*LIVETUNE_DB_REGISTER_UPDATE_CB)(struct json_instance_t *pJsonInst, uint16_t rootID, struct livetune_db_element *pElement);


typedef struct livetune_db_list
{
  union
  {
    void                                 *pObject;
    struct livetune_db_instance       *pInstance;
    struct livetune_db_element_params *pElemParam;
    struct livetune_db_instance_param *pInstParam;
    struct livetune_db_element_pin    *pElementPin;
    struct livetune_db_list           *pCnxList;
    struct livetune_db_instance_cnx   *pCnxInstance;
  };
  struct livetune_db_list *pNext;
  struct livetune_db_list *pPrev;
} livetune_db_list;

typedef struct livetune_interface
{
  void (*constructor_cb)(struct livetune_db_instance *pInstance);
  void (*destructor_cb)(struct livetune_db_instance *pInstance);
  void (*transform_cb)(struct livetune_db_instance *pInstance);
  void (*parameter_change_cb)(struct livetune_db_instance *pInstance, livetune_info_param type);
  livetune_builder_result(*pipe_builder_cb)(struct livetune_db_instance *pInstance, uint32_t cmd);
  uint32_t iCookie;
  LIVETUNE_INTERFACE_EXTRA_STRUCT
} livetune_interface;


typedef struct livetune_db_instance_param
{
  char_t       *pParamValue;
  const char_t *pName;
} livetune_db_instance_param;


typedef struct livetune_db_instance_cnx
{
  struct livetune_db_instance *pInstance;
  uint32_t                        iPinDef;
  char_t                         *pName;
  livetune_db_list            *pParams;
  LIVETUNE_CNX_EXTRA_STRUCT
} livetune_db_instance_cnx;

typedef struct livetune_db_instance
{
  char_t                           *pMeta;
  float_t                          *pScratchBuffer;
  int8_t                            bResolved;
  void                             *pBuilderData;
  void                             *pUserData;
  uint32_t                          iUserIndex;
  char_t                           *pInstanceName;
  char_t                           *pDescription;
  uint8_t                           iUpdateType;
  livetune_db_list              *pParams;
  livetune_db_instance_pins_def *pPinOut;
  livetune_db_instance_pins_def *pPinIn;
  struct livetune_db_element    *pElements;
  struct livetune_pipe          *pPipe;
  LIVETUNE_INSTANCE_EXTRA_STRUCT
} livetune_db_instance;


typedef struct livetune_db_element_params
{
  struct livetune_db_element *pElements;
  char_t                        *pParamDefault;
  char_t                        *pParamType;
  uint32_t                       iIndex;
  uint32_t                       iType;
  char_t                        *pName;
} livetune_db_element_params;


typedef struct livetune_db_element_pin
{
  livetune_type               pType;
  char_t                        *pPinName;
  struct livetune_db_element *pElem;
  uint32_t                       iIndexPin;
  livetune_db_list           *pParams;
} livetune_db_element_pin;



typedef struct livetune_db_element
{
  const char_t                   *pJsonDefinition;
  char_t                         *pName;
  livetune_db_list            *pParams;
  livetune_db_element_pin_def *pPinIn;
  livetune_db_element_pin_def *pPinOut;
  livetune_interface           hInterface;
  const void                     *pRefBuilder;

} livetune_db_element;



typedef struct livetune_db
{
  uint32_t                          bParsingError;
  livetune_db_list              *pElements;
  char_t                           *pMeta;
  livetune_db_list              *pInstances;
  uint8_t                           bStorageErr;
  LIVETUNE_DB_MSG_CB             cbMsg;
  LIVETUNE_DB_REGISTER_UPDATE_CB cbRegisterUpdate;
} livetune_db;


/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */


uint32_t                          livetune_db_list_nb(livetune_db_list **pList);
livetune_db_list                 *livetune_db_list_create(livetune_db_list **pList, void *pObject);
ST_Result                         livetune_db_list_delete(livetune_db_list **pList, uint32_t index, uint32_t bFreeObject);
livetune_db_list                 *livetune_db_list_get(livetune_db_list **pList, uint32_t index);
void                             *livetune_db_list_get_object(livetune_db_list **pList, uint32_t index);
void                              livetune_db_list_clear(livetune_db_list **pList, uint32_t bFreeObject);
int32_t                           livetune_db_list_find(livetune_db_list **pList, void *pObject);
ST_Result                         livetune_db_list_delete_item(livetune_db_list **pList, livetune_db_list *pCur, uint32_t bFreeObject);
livetune_db_element              *livetune_db_element_create(livetune_db *pHandle, const char_t *pJsonDesc, const void *pRefBuilder, livetune_interface cb);
ST_Result                         livetune_db_element_delete(livetune_db *pHandle, livetune_db_element *pElement);
uint32_t                          livetune_db_element_nb_get(livetune_db *pRenderHandle);
livetune_db_element              *livetune_db_element_get(livetune_db *pRenderHandle, uint32_t index);
const char_t                     *livetune_db_element_definition_get(livetune_db *pRenderHandle, uint32_t index);
void                              livetune_db_set_register_update_cb(livetune_db *pHandle, LIVETUNE_DB_REGISTER_UPDATE_CB cb);
ST_Result                         livetune_db_create(livetune_db *pRenderHandle);
ST_Result                         livetune_db_delete(livetune_db *pHandle);
ST_Result                         livetune_db_new(livetune_db *pHandle);
livetune_db_instance             *livetune_db_instance_create(livetune_db *pHandle, const char_t *pInstanceName, const char_t *pRefElement);
ST_Result                         livetune_db_instance_delete_from_name(livetune_db *pHandle, const char_t *pInstanceName);
ST_Result                         livetune_db_instance_delete(livetune_db *pHandle, livetune_db_instance *pInstance);
uint32_t                          livetune_db_instance_pin_def_nb(livetune_db_instance_pins_def **pPinDef);
livetune_db_instance_pins_def    *livetune_db_instance_pin_def_get(livetune_db_instance_pins_def **pPinDef, uint32_t index);
livetune_db_instance             *livetune_db_instance_find_from_name(livetune_db *pHandle, const char_t *pName);
void                              livetune_db_instance_cleanup(livetune_db *pHandle);
void                              livetune_db_instance_set_update_type(livetune_db_instance *pInstance, uint8_t type);
livetune_db_instance_cnx_list    *livetune_db_cnx_attach(livetune_db_instance_cnx_list **pPinList, livetune_db_instance *pHandleAttached, uint32_t pinAttached, livetune_db_element_pin *pPinElement);
uint32_t                          livetune_db_cnx_nb(livetune_db_instance_cnx_list **pList);
livetune_db_instance_cnx         *livetune_db_cnx_get(livetune_db_instance_cnx_list **pCnxList, uint32_t index);
ST_Result                         livetune_db_cnx_delete(livetune_db *pHandle, const char_t *pInstanceNameOut, uint32_t PinOut, const char_t *pInstanceNameIn, uint32_t pinIn);
ST_Result                         livetune_db_cnx_create(livetune_db *pHandle, const char_t *pInstanceNameOut, uint32_t pinOut, const char_t *pInstanceNameIn, uint32_t pinIn);
ST_Result                         livetune_db_param_change(livetune_db_instance_param *pParam, char_t *pFormat, ...);
int32_t                           livetune_db_param_find_from_name(livetune_db_list **pParams, const char_t *pName);
ST_Result                         livetune_db_param_set(livetune_db *pHandle, livetune_db_instance *pInstance, livetune_info_param paramType, livetune_db_instance_param *pParams, const char_t *pValue);
livetune_db_instance_cnx         *livetune_db_cnx_find(livetune_db_instance_pins_def **pHandle, uint32_t pinRef, const char_t *pInstanceNameDst, uint32_t pinDst);
int32_t                           livetune_db_cnx_find_from_name(livetune_db_instance_pins_def **pHandle, uint32_t pinRef, const char_t *pInstanceNameDst, uint32_t pinDst);
livetune_db_instance_cnx         *livetune_db_cnx_name_search(livetune_db_instance_pins_def **ppDef, const char_t *pCnxName, uint32_t pinDef);
int32_t                           livetune_db_cnx_name_search_index(livetune_db_instance_pins_def **ppDef, const char_t *pCnxName, uint32_t pinDef);
livetune_db_instance_cnx         *livetune_db_cnx_name_find(livetune_db_instance_pins_def **pHandle, int32_t pinRef, const char_t *pInstanceNameDst, uint32_t pinDst);
livetune_db_element              *livetune_db_element_find(livetune_db *pHandle, const char_t *pRef);
void                              livetune_db_cmd(livetune_db_pipe_cmd cmd, livetune_db_pipe_reason reason);
void                              livetune_db_send(livetune_db *pHandle, uint32_t evt, st_message_param lparam);
void                              livetune_db_msg_cb_set(livetune_db *pHandle, LIVETUNE_DB_MSG_CB cb);
void                              livetune_db_coherency(livetune_db *pHandle);


#ifdef __cplusplus
};
#endif




#endif



