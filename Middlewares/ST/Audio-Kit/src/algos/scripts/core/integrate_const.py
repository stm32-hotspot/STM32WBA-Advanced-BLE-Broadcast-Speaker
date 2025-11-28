class IntegrateStr:
    """ This class contains all const strings needed to generate algo wrappers """

    HEADER = """/**
  ******************************************************************************
  * @file    audio_chain_key_replace.EXT_REPLACE
  * @author  MCD Application Team
  * @brief   wrapper of key_replace algo to match usage inside audio_chain.c
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2019(-2022) STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
"""
    H_HEADER = """/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __AUDIO_CHAIN_KEY_REPLACE_H
#define __AUDIO_CHAIN_KEY_REPLACE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "audio_chain.h"

"""

    H_HEADER_FOOTER ="""
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
extern const audio_algo_factory_t AudioChainWrp_key_replace_factory;
extern const audio_algo_common_t  AudioChainWrp_key_replace_common;
extern       audio_algo_cbs_t     AudioChainWrp_key_replace_cbs;

/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

"""
    H_HEADER_CPP_FOOTER ="""

#ifdef __cplusplus
}
#endif

#endif /* __AUDIO_CHAIN_KEY_REPLACE_H */
"""

    C_INCLUDES = """/* Includes ------------------------------------------------------------------*/
#include "key_replace/audio_chain_key_replace.h"
key_extra_include

"""

    C_TYPEDEF_COMMENT = "/* Private typedef -----------------------------------------------------------*/\n"
    C_DEFINES_COMMENT = "/* Private defines -----------------------------------------------------------*/\n"
    C_MACROS_COMMENT = "/* Private macros ------------------------------------------------------------*/\n"
    C_VAR_COMMENT = "/* Private variables ---------------------------------------------------------*/\n"
    C_FUNC_COMMENT = "/* Private function prototypes -----------------------------------------------*/\n"


    C_GLOBALS_COMMENT ="""
/* Global variables ----------------------------------------------------------*/
"""

    C_FUNC_DECLARE_COMMENT ="""

/* Private Functions Definition ------------------------------------------------------*/
"""
    C_PLUG_HEADER="""
#ifdef USE_LIVETUNE_DESIGNER


/* Includes ------------------------------------------------------------------*/
#include "livetune.h"
#include "livetune_helper.h"


/* Private defines -----------------------------------------------------------*/
#define ALGO_PLUGIN_CB plugin_key_replace_event_cb

enum plugin_pin_out
{
  kPinOutMsg,
};



/* Private macros ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
static livetune_helper_builder_def hBuilderDef =
{
  .pAlgoName = "key_replace"
};

"""
    C_PLUG_GLOBALS=r"""/* Declarations ----------------------------------------------------------*/
static void                    plugin_processing_constructor(livetune_db_instance *pInstance);
static livetune_builder_result plugin_processing_builder(livetune_db_instance *pInstance, uint32_t cmd);

/* Global variables ----------------------------------------------------------*/

static char_t tElementDescription[] = "{\"PinDefOut\":[{\"Type\":\"acMsg\",\"Name\":\"Msg\",\"Description\":\"Must be connected to an acMsg viewer\"}]}";

static livetune_interface plugin_interface =  // element instance
{
  .constructor_cb      = plugin_processing_constructor,
  .pipe_builder_cb     = plugin_processing_builder
};

"""
    C_PLUG_FUNC_BEG="""

/* Private functions ------------------------------------------------------- */

#define LOCAL_SCRATCH_MSG 256U

/* AC control callback */

static int32_t plugin_processing_control_key_replace(acAlgo hAlgo)
{
  if (hAlgo)
  {
    st_class_element_instance *pCls = NULL;
    acAlgoGetCommonConfig(hAlgo, "userData", &pCls);
    ST_ASSERT(pCls != NULL);
    st_plugin_element_ext *pClsExt = LIVETUNE_HELP_CLASS_EXT(pCls, st_plugin_element_ext *);
    /* if not connected, no events */
    if (pClsExt->pConnectedMsg)
    {
"""
    C_PLUG_FUNC_MID="""

      /* alloc the memory for the the msg body */

      char_t   tlocalScratch[LOCAL_SCRATCH_MSG];
      char_t   tlocalVar[LOCAL_SCRATCH_MSG] = "";
      uint32_t szScratchBody = LOCAL_SCRATCH_MSG ;
      char_t  *pScratchBody  = st_os_mem_alloc(ST_Mem_Type_Designer, szScratchBody);
      if (pScratchBody)
      {
        memset(pScratchBody, 0, szScratchBody);
        /* Example of text message commented here: */
        //        livetune_helper_format_clear(tlocalScratch);
        //        /* build message to be sent */
        //        snprintf(tlocalScratch, sizeof(tlocalScratch), "Blabla my var = %6.2f", pClsExt->myField);
        //        strcat(pScratchBody, tlocalScratch);
"""
    C_PLUG_FUNC_END=r"""
        /* Example of variable message activated for the first field of the control structure here. 
           Can be copied and applied for any field of the structure.
        */
        /* Enable a variable watcher */
        char *pVars = strchr(tlocalVar, 0);
        snprintf(pVars, sizeof(tlocalVar), "ch%d:%2.1fdB", 0, (double)pClsExt->key_replace); /* the syntax is Key:Value*/

        /* build mandatory json fields to identify the MsgViewer instance to send to */
        livetune_helper_format_clear(tlocalScratch);
        snprintf(tlocalScratch, sizeof(tlocalScratch), "\"From\":\"%s\",\"To\":\"%s\",\"Vars\":\"%s\"", pCls->hBuilder.pInstance->pInstanceName, pClsExt->pConnectedMsg->pInstance->pInstanceName, tlocalVar);

        /* Example given for the first field but can be copied for others */

        /* send the message */
        livetune_send_block_text_async(FALSE, "acMsg", "Text", tlocalScratch, pScratchBody);

        st_os_mem_free(pScratchBody);
      }
    }
  }
  return 1;
}
"""
    C_PLUG_FOOTER="""

/**
* @brief instantiate or generate code
* * @param pHandle the instance handle
*/

static livetune_builder_result plugin_processing_builder(livetune_db_instance *pInstance, uint32_t cmd)
{
  livetune_builder_result result = ST_BUILDER_DEFAULT;
  st_class_element_instance *pCls   = (st_class_element_instance *)pInstance->pUserData;

  switch (cmd)
  {
    case LIVETUNE_PIPE_INIT_GRAPH_ELEMENT:
    {
      /* Process Init Graph default behaviour */
      ST_VERIFY(livetune_helper_builder_pipe_init_graph_element_default(&pCls->hBuilder) == ST_BUILDER_OK);
      /* overload the default with a set control callback */

      if (pInstance->hAc.bValid)
      {
        /* Add the send Graph support control */
        livetune_ac_wrapper_acAlgoSetCommonConfig(&pCls->hBuilder, "controlCb", (void *)plugin_processing_control_key_replace); /*cstat !MISRAC2012-Rule-11.1 cast a callback in pointer is mandatory for this API */
        livetune_ac_wrapper_acAlgoSetCommonConfig(&pCls->hBuilder, "userData", pCls);
      }


      result = ST_BUILDER_OK;
      break;
    }
  }
  return result;
}

/**
* @brief Overload the construction because we derive the builder with an extension
*       This function is called before the pipeline start
* * @param pHandle the instance handle
*/


static void plugin_processing_constructor(livetune_db_instance *pInstance)
{
  pInstance->pUserData = livetune_ac_factory_builder_create(pInstance, sizeof(st_plugin_element_ext));
  if (pInstance->pUserData)
  {
    st_class_element_instance *pCls    = (st_class_element_instance *)pInstance->pUserData;
    st_plugin_element_ext     *pClsExt = LIVETUNE_HELP_CLASS_EXT(pCls, st_plugin_element_ext *);
    pClsExt->pConnectedMsg             = livetune_helper_cnx_get(&pInstance->pPinOut, (uint32_t)kPinOutMsg, 0);
  }
  else
  {
    ST_TRACE_ERROR("Memory alloc %d", sizeof(st_class_element_instance));
  }
}


/**
* @brief  Event handler
*
* @param handle the instance handle
* @param evt  the event
* @param wparam  the opaque param
* @param lparam  the opaque param
* @return ST_Result Error code
*/

static uint32_t ALGO_PLUGIN_CB(ST_Handle hInst, ST_Message_Event evt, st_message_param wParam, st_message_param lParam)
{
  if (evt == ST_EVT_REGISTER_ELEMENTS)
  {
    livetune_instance *pDesigner = (livetune_instance *)hInst;
    if (livetune_db_element_json_register(&pDesigner->hDesignerDB, tElementDescription, &hBuilderDef, plugin_interface) == 0)
    {
      ST_TRACE_ERROR("cannot register algo %s", hBuilderDef.pAlgoName);
    }

  }
  return 0;
}

/* plugin declaration, notify that this element must be add as a client */

ST_PLUGIN_DECLARE(ALGO_PLUGIN_CB);

#endif
    
"""
    C_FACTORY_HEADER="""
#if defined(AUDIO_CHAIN_ACSDK_USED) || defined(AUDIO_CHAIN_CONF_TUNING_CLI_USED)

/*cstat -MISRAC2012-Rule-1.4_a extended language features usage is needed to insure correct type in AUDIO_DESC_PARAM_XX macro*/
BEGIN_IGNORE_DIV0_WARNING
"""
    C_FACTORY_CONTOL_DECLARE="""
    
static const audio_algo_control_params_descr_t s_key_replace_controlParamTemplate =
{
  .pParams           = s_key_replace_controlParamsDesc,
  .nbParams          = sizeof(s_key_replace_controlParamsDesc) / sizeof(s_key_replace_controlParamsDesc[0])
};

"""
    C_FACTORY_PARAM_DECLARE="""

static const audio_descriptor_params_t s_key_replace_type_replaceParamTemplate =
{
  .pParam            = (audio_descriptor_param_t *)s_key_replace_type_replaceParamsDesc,
  .nbParams          = sizeof(s_key_replace_type_replaceParamsDesc) / sizeof(s_key_replace_type_replaceParamsDesc[0]),
  .szBytes           = sizeof(struct_replace)
};

"""
    C_FACTORY_FOOTER = """
#endif  // AUDIO_CHAIN_ACSDK_USED || AUDIO_CHAIN_CONF_TUNING_CLI_USED

const audio_algo_factory_t AudioChainWrp_key_replace_factory =
{
  .pStaticParamTemplate  = pointer_static_replace,
  .pDynamicParamTemplate = pointer_dynamic_replace,
  .pControlTemplate      = pointer_control_replace,
  .pCapabilities         = &AudioChainWrp_key_replace_common,
  .pExecutionCbs         = &AudioChainWrp_key_replace_cbs
};

ALGO_FACTORY_DECLARE(AudioChainWrp_key_replace_factory);

"""
    C_STATIC_CONF_CHECK = """  key_replace const *const pStaticConfig = AudioAlgo_getStaticConfig(pAlgo);
  if (pStaticConfig == NULL)
  {
    AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "missing static conf !");
    error = AUDIO_ERR_MGNT_INIT;
  }
"""
    C_DYNAMIC_CONF_CHECK = """  key_replace const *const pDynamicConfig = AudioAlgo_getDynamicConfig(pAlgo);
  if (pDynamicConfig == NULL)
  {
    AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "missing dynamic conf !");
    error = AUDIO_ERR_MGNT_INIT;
  }
"""
    C_ERROR_DECLARE = "  int32_t{} error{} = AUDIO_ERR_MGNT_NONE;\n"
#     C_CHUNK_DECLARE = """  audio_chunk_t  const *const pChunkkey_replace          = AudioAlgo_getChunkPtrkey_replace(pAlgo, 0U);
#   audio_buffer_t const *const pBuffkey_replace           = AudioChunk_getBuffInfo(pChunkkey_replace);
# """
    C_CHUNK_DECLARE = "  audio_chunk_t  {}const *const pChunk{} = AudioAlgo_getChunkPtr{}(pAlgo, 0U);\n"
    C_BUFF_DECLARE = "  audio_buffer_t {}const *const pBuff{} = AudioChunk_getBuffInfo(pChunk{});\n"

    C_MEM_POOL = "#define KEY_REPLACE_MEM_POOL AUDIO_MEM_RAMINT\n"
    C_CTXT_FREE = """  if (pContext != NULL)
  {
    /* disconnect context from Algo first: insure algo won't be executed during deinit */
    AudioAlgo_setWrapperContext(pAlgo, NULL);
    if (pContext->user.pReserved != NULL)
    {
      /* Free anything that needs to be */
    }

    AudioAlgo_free(pContext, KEY_REPLACE_MEM_POOL);
  }
"""
    C_CTXT_ALLOCATE = """  key_replace_context_t            *pContext          = NULL;
  size_t                      allocSize         = sizeof(key_replace_context_t);

  pContext   = (key_replace_context_t *)AudioAlgo_malloc(allocSize, KEY_REPLACE_MEM_POOL);
  if (pContext == NULL)
  {
    AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "Alloc failed !");
    error = AUDIO_ERR_MGNT_ALLOCATION;
  }

  if (AudioError_isOk(error))
  {
    memset(pContext, 0, allocSize);
    AudioAlgo_setWrapperContext(pAlgo, pContext);
  }
"""
    C_CTXT_USAGE_COMMENT ="""
/*
 * This code is generated to provide several ways of integrating an algorithm. Therefore, there will be some useless
 * variables that can be removed when wrapper is completed. 
 * Input & output data are provided using audio_chunk_t which is used by AudioChain to maintain all data pointers
 * automatically. As a matter of fact it is a slightly complex structure one may not want to use for simple algorithm.
 * AudioChain also uses a simpler structure called audio_buffer_t which hosts fields for data description plus a data 
 * pointer. 
 * 
 * In this generated code, we instantiate a private context structure with several fields that may come in handy to 
 * process audio. 
 * Chunks are saved at init to avoid getting them every calls of dataInOut or Process callbacks. 
 * The context also includes audio_buffer_t fields for those who may use this structure for data manipulation. 
 * They can be removed, if not wanted.
 * 
 * Method 1: Working with data pointers
 *  The generated code shows how to get these pointers from the audio_chunk_t through both routines:
 *  + void *AudioChunk_getWritePtr(audio_chunk_t const *const pChunk,   uint8_t  const chId, uint32_t const spleId);
 *  + void *AudioChunk_getReadPtr(audio_chunk_t  const *const pChunk,   uint8_t  const chId, uint32_t const spleId);
 *  Note: Necessary info such as number of channels, samples, etc... are saved in the context structure. Of course,
 *       if not needed they can be removed.
 * 
 * Method 2: Working with audio_buffer_t
 *  The generated code shows how to set audio_buffer_t from audio_chunk_t through both routines:
 *  + for input:  int32_t AudioChunk_setAudioBufPtrFromRdPtr(audio_chunk_t const *const pChunk,   audio_buffer_t *const pBuff);
 *  + for output: int32_t AudioChunk_setAudioBufPtrFromWrPtr(audio_chunk_t const *const pChunk,   audio_buffer_t *const pBuff); 
*/    
"""

    C_ERROR_OK_CHECK = "  if (AudioError_isOk(error))\n  {\n"
    C_ERROR_NOK_CHECK = "  if (AudioError_isError(error))\n  {\n"
    C_STRUCT_CTXT_BUFF = """
typedef struct
{
  audio_buffer_t            buff;
  audio_chunk_t            *pChunk;
}
key_replace_buffer_context_t;"""

    C_TYPEDEF_START = "\ntypedef struct\n{\n"
    C_TYPEDEF_END = "\n{}_context_t;\n"
    C_CONTROL_MALLOC = """  void *pCtrl = NULL;
  if (AudioError_isOk(error))
  {
    pCtrl = AudioAlgo_malloc(sizeof(struct_replace), KEY_REPLACE_MEM_POOL); /*cstat !MISRAC2012-Rule-22.1_a pCtrl is stored in context through AudioAlgo_setCtrlData and freed in s_sam_deinit*/
    if (pCtrl == NULL)
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "Alloc for control data failed !");
      error = AUDIO_ERR_MGNT_ALLOCATION;
    }
    else
    {
      memset(pCtrl, 0, sizeof(struct_replace));
      AudioAlgo_setCtrlData(pAlgo, pCtrl);
    }
  }

"""
    C_CONTROL_FREE="""
  if (pCtrl != NULL)
  {
    AudioAlgo_setCtrlData(pAlgo, NULL);
    AudioAlgo_free(pCtrl, KEY_REPLACE_MEM_POOL);
  }
  
"""
    HELP_DATAINOUT_PROCESS = """
  + dataInOut:
    - retrieves input data from the input IOs
    - provides output data the output IOs.
    - mandatory routine but can be empty of any audio task. However, in order to trigger the processing callback,
      AudioAlgo_incReadyForProcess(pAlgo); MUST BE called. In case where no processing callback is needed, calling
      AudioAlgo_incReadyForProcess is not necessary.
    - Doesn't need to be called at the same pace as processing callback. For instance, let's assume dataInOut @ 1ms but
      processing must be called @ 8ms; then developer will have to ensure that dataInOut calls
      AudioAlgo_incReadyForProcess every 8 calls.
    - In a low-latency driven system it can host the complete processing, otherwise,
      it is better to delegate the main part of the processing to the process callback
    - Automatically triggered when input IOs had at least a frame written
    - the dataInOut callback of all algorithms in a chain are called within a single task/thread

  + process:
    - optional routines; everything can be done in dataInOut as explained.
    - different priority than dataInOut, MUST BE lower
    - two levels available: "normal" vs "low"
    - the process callback of all algorithms in a chain are called within a single task/thread which priority can be
      tuned lower than the thread of the dataInOut
 """
    C_JUSTIFY_VAR_NAME = 31
    C_JUSTIFY_VAR_TYPE = 10
    C_JUSTIFY_VAR_VAL = 10
    C_JUSTIFY_VAR_CAPA = 26
    C_CTXT_VAR_LIST = {
        "sampleSize": "uint8_t",
        "nbChannels": "uint8_t",
        "nbSamples" : "uint32_t",
        "fs" : "uint32_t",
        "bufferSize" : "uint32_t",
        "nbCells": "uint32_t",
        "nbElements": "uint32_t",
        "channelsOffset" : "uint32_t",
        "samplesOffset": "uint32_t",
        "samplesOffset0": "uint32_t",
        "interleaved": "audio_buffer_interleaved_t",
        "timeFreq": "audio_buffer_time_freq_t",
        "type": "audio_buffer_type_t"
    }

