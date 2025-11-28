/**
******************************************************************************
* @file          livetune_ac_wrapper.h
* @author        MCD Application Team
* @brief         implements a conditional call to the acSDK
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



#ifndef LIVETUNE_AC_WRAPPER
#define LIVETUNE_AC_WRAPPER

#ifdef __cplusplus
extern "C"
{
#endif
/* Includes ------------------------------------------------------------------*/
#include "livetune_helper.h"
#include "livetune_ac_json.h"
//#include "st_ac_types.h"
#include "livetune_ac_factory.h"
#include "acSdk.h"


/* Exported constants --------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported macros ------------------------------------------------------------*/

#define ST_HELPER_GET_RT_STATE() livetune_pipe_get_rt_graph_construction(&livetune_get_instance()->hAudioPipe)

/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void    livetune_ac_wrapper_acPipeConnectPinOut(struct livetune_helper_builder *pBuilder, uint32_t pinID, acChunk hChunk);
void    livetune_ac_wrapper_acPipeConnectPinIn(struct livetune_helper_builder *pBuilder, uint32_t pinID, acChunk hChunk);
void    livetune_ac_wrapper_acPipeConnect(struct livetune_helper_builder *pBuilder, uint32_t srcPinID, acChunk hChunk, acAlgo hAlgoDst, uint32_t dstPinID);
int32_t livetune_ac_wrapper_acPipePlay_start(livetune_pipe *pPipe);
int32_t livetune_ac_wrapper_acPipePlay_stop(livetune_pipe *pPipe);
int32_t livetune_ac_wrapper_acPipeIsPlaying(livetune_pipe *pPipe);
void    livetune_ac_wrapper_acAlgoSetCommonConfig(struct livetune_helper_builder *pBuilder, const char_t *pKey, void *pOpaque);
void    livetune_ac_wrapper_acAlgoSetConfig(acAlgo pAlgo, const char_t *pName, const char_t *pValue, int8_t bForceUpdate);
void    livetune_ac_wrapper_acAlgoSetConfig_ptr(acAlgo hAlgo, const char_t *pName, const void *pValue, int8_t bForceUpdate);
void    livetune_ac_wrapper_acChunkCreate(struct livetune_helper_builder *pBuilder, livetune_db_instance_pins_def **pPinDef, uint32_t pinDef);
void    livetune_ac_wrapper_acPipeCreate(livetune_pipe *pPipe);
void    livetune_ac_wrapper_acAlgoCreate(struct livetune_helper_builder *pBuilder);
void    livetune_ac_wrapper_acEnvSetConfig(const char_t *pKey, uint32_t value);
void    livetune_ac_wrapper_acCyclesDump(void);
void    livetune_ac_check_system_error(uint32_t level);
int8_t  livetune_ac_is_factory_algo(const char_t *pAlgoName);

#ifdef __cplusplus
};
#endif


#endif



