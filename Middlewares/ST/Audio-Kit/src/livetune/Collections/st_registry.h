/**
******************************************************************************
* @file          st_registry.h
* @author        MCD Application Team
* @brief         Header: registry  management
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



#ifndef ST_REGISTRY_H
#define ST_REGISTRY_H

/* Includes ------------------------------------------------------------------*/
#include "st_base.h"
#include "st_os.h"
#include "st_registry_conf.h"


#ifdef __cplusplus
extern "C"
{
#endif

/* Exported macros -----------------------------------------------------------*/
#ifndef ST_REGISTRY_CUSTOM
#define ST_REGISTRY_CUSTOM st_registry_config_default
#endif

#ifndef ST_Mem_Type_Registry
#define ST_Mem_Type_Registry ST_Mem_Type_ANY_FAST
#endif


#ifndef ST_REGISTRY_OK
#define   ST_REGISTRY_OK     1
#endif
#ifndef ST_REGISTRY_ERROR
#define   ST_REGISTRY_ERROR  0
#endif

#ifndef ST_REGISTRY_TRACE_ERROR
#define ST_REGISTRY_TRACE_ERROR(...)   {printf(__VA_ARGS__);printf("\n");}
#endif
#ifndef  ST_REGISTRY_TRACE_INFO
#define ST_REGISTRY_TRACE_INFO(...)    {printf(__VA_ARGS__);printf("\n");}
#endif

#ifndef  ST_REGISTRY_TRACE_DEBUG
#define ST_REGISTRY_TRACE_DEBUG(...)    {printf(__VA_ARGS__);printf("\n");}
#endif



#ifndef ST_REGISTRY_VERIFY
#define ST_REGISTRY_VERIFY(a)  ((void)(a))
#endif

#ifndef ST_REGISTRY_ASSERT
#define ST_REGISTRY_ASSERT(a)
#endif


typedef int32_t ST_Registry_Result ;


/* Exported types ------------------------------------------------------------*/


typedef struct st_registry_config_default
{
  uint32_t dummy;
} st_registry_config_default;



/*!
@brief Persistent items
@ingroup enum
Persistent items see also  ST_Persist_CB
*/
typedef enum st_persist_item
{
  ST_PERSIST_ITEM_SYSTEM,
  ST_PERSIST_ITEM_END,
  ST_PERSIST_ITEM_ALIGN = MAX_INT

} st_persist_item;


/*!
@brief Actions for Persistent items
@ingroup enum
Actions for Persistent  see also  ST_Persist_CB
*/
typedef enum st_persist_action
{
  ST_PERSIST_GET_POINTER /*!< Return a  direct pointer on the item */
  ,
  ST_PERSIST_SET /*!< Set the Item */
  ,
  ST_PERSIST_ALIGN = MAX_INT

} st_persist_action;

/*!
@brief Persistent storage READ/WRITE callback
@ingroup type
@param  action   Persistent  action
@param  item     Persistent   item
@param  pItem    Item pointer
@param  itemSize Item size
Item itemSize
@return Error code or a pointer
*/
#ifndef ST_PERSIST_SIGNATURE
#define ST_PERSIST_SIGNATURE 0x34D5453UL /* STM0*/
#endif

/*!
@brief Persistent object  network
@ingroup struct
Actions for Persistent  see also  ST_Persist_CB
*/
ST_ALIGN_START
typedef struct st_persist_sys
{
  uint32_t           iSignature;
  uint32_t           iLogLevel; /*!< log level   */
  ST_REGISTRY_CUSTOM hUser;     /*!< User registry field */
  uint32_t           iEndCheckSum;
  uint32_t           iChecksum; /*!< anti-corruption chks*/
} st_persist_sys;
ST_ALIGN_STOP


typedef struct t_st_registry
{
  st_persist_sys *pSystemRAM;   /* store the system persistent pointer to minimize the access to the callback decoded*/
  st_mutex        systemLock;   /* Mutex locking the system registry*/
  st_mutex        iotLock;      /* Mutex locking the iot registry*/
  void           *pCookie;

} st_registry;


/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

ST_Registry_Result       st_registry_create(st_registry *pHandle);
ST_Registry_Result       st_registry_delete(st_registry *pHandle);
st_persist_sys          *st_registry_lock_sys(st_registry *pHandle);
ST_Registry_Result       st_registry_unlock_sys(st_registry *pHandle, uint32_t bModified);
ST_Registry_Result       st_registry_reset_default(st_registry *pHandle, st_persist_sys *pRegistry);
ST_Registry_Result       st_registry_erase(st_registry *pHandle);
ST_Registry_Result       st_registry_is_valid(st_registry *pHandle);





#ifdef __cplusplus
};
#endif



#endif

