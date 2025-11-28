/**
******************************************************************************
* @file    audio_chain_mdrc_factory.c
* @author  MCD Application Team
* @brief   factory of MDRC (multi-band dynamic range compressor) algo to match usage inside audio_chain.c
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
#include "audio_chain_mdrc.h"

#if defined(AUDIO_CHAIN_ACSDK_USED) || defined(AUDIO_CHAIN_CONF_TUNING_CLI_USED)

static const audio_descriptor_key_string_t s_mdrcPreset[] =
{
  #if MDRC5B_SUBBAND_MAX > 1
  {"Bass-boost", "mode|3||numBands|2||compressorEnable0|1||attackTime0|100||releaseTime0|1000||dynamicResponse0|{'0':[-120,-120],'1':[-80,-80],'2':[-40,-15],'3':[0,0]}||compressorPostGain0|0||compressorFreqCutoff1|200||compressorEnable1|1||attackTime1|100||releaseTime1|1000||dynamicResponse1|{'0':[-120,-120],'1':[0,0]}||compressorPostGain1|0||compressorFreqCutoff2|500||compressorEnable2|1||attackTime2|100||releaseTime2|1000||dynamicResponse2|{}||compressorPostGain2|0||compressorFreqCutoff3|1000||compressorEnable3|1||attackTime3|100||releaseTime3|1000||dynamicResponse3|{}||compressorPostGain3|0||compressorFreqCutoff4|2000||compressorEnable4|1||attackTime4|100||releaseTime4|1000||dynamicResponse4|{}||compressorPostGain4|0||attackTime|1||releaseTime|100||limiterThreshold|-3"},
  #endif
  {0}
};

static const audio_descriptor_key_value_t s_mdrcMode[] =
{
  {"bypass",                   0UL},
  {"compressor only",          MDRC_COMPRESSOR_MODE},
  {"limiter only",             MDRC_LIMITER_MODE},
  {"compressor, then limiter", (uint32_t)MDRC_COMPRESSOR_MODE | (uint32_t)MDRC_LIMITER_MODE},
  {0}
};

/*cstat -MISRAC2012-Rule-1.4_a extended language features usage is needed to insure correct type in AUDIO_DESC_PARAM_XX macro*/
BEGIN_IGNORE_DIV0_WARNING

static const audio_descriptor_param_t s_mdrcCompDynResp[] =
{
  {
    .pDescription    = AUDIO_ALGO_OPT_STR("input level (dB)"),
    .pControl        = AUDIO_ALGO_OPT_STR("slider"),
    .pDefault        = "-10",
    .pName           = "inputLevel",
    AUDIO_DESC_PARAM_F(mdrc5b_dyn_resp_point_t, inputLevel, MDRC5B_LEVEL_MIN_DB, MDRC5B_LEVEL_MAX_DB)
  },
  {
    .pDescription    = AUDIO_ALGO_OPT_STR("output level (dB)"),
    .pControl        = AUDIO_ALGO_OPT_STR("slider"),
    .pDefault        = "-10",
    .pName           = "outputLevel",
    AUDIO_DESC_PARAM_F(mdrc5b_dyn_resp_point_t, outputLevel, MDRC5B_LEVEL_MIN_DB, MDRC5B_LEVEL_MAX_DB)
  },
  {0}
};

static const audio_descriptor_param_t s_mdrc_dynamicParamsDesc[] =
{
  {
    .pDescription     = AUDIO_ALGO_OPT_STR("Applies a quick setting"),
    .pGroup           = AUDIO_ALGO_OPT_STR("General"),
    .pName            = "preset",
    .pControl         = AUDIO_ALGO_OPT_STR("preset"),
    .iParamFlag       = AUDIO_DESC_PARAM_TYPE_FLAG_AS_KEY_STRING | AUDIO_DESC_PARAM_TYPE_FLAG_PRIVATE,
    .pKeyString       = AUDIO_ALGO_OPT_TUNING(s_mdrcPreset),
    .pDefault         = "0", /* ignored for this control */
    AUDIO_DESC_PARAM_U8(mdrcDynamicConfig_t, preset, 0U, 0U)
  },
  {
    .pDescription     = AUDIO_ALGO_OPT_STR("Compressor/Limiter mode"),
    .pName            = "mode",
    .pControl         = AUDIO_ALGO_OPT_STR("droplist"),
    .iParamFlag       = AUDIO_DESC_PARAM_TYPE_FLAG_DEFINE_KEY,
    .pKeyValue        = s_mdrcMode,
    .pDefault         = AUDIOCHAINFACTORY_INT2STR((uint32_t)MDRC_COMPRESSOR_MODE | (uint32_t)MDRC_LIMITER_MODE),
    AUDIO_DESC_PARAM_U8(mdrcDynamicConfig_t, mode, 0U, 3U)
  },
  // remark: numBands and bandCompressors could be managed as a struct array if recursive struct array was managed
  //         today it is impossible because dynamicResponse is already a struct array inside bandCompressors
  {
    .pDescription     = AUDIO_ALGO_OPT_STR("number of compressor sub-bands (only Band 0 to Band {numBands-1} are active)"),
    .pName            = "numBands",
    #if MDRC5B_SUBBAND_MAX > 1
    .pControl         = AUDIO_ALGO_OPT_STR("slider"),
    #else
    .pControl         = AUDIO_ALGO_OPT_STR("static"),
    #endif
    .pDefault         = "1",
    AUDIO_DESC_PARAM_U32(mdrcDynamicConfig_t, compressor.numBands, 1UL, MDRC5B_SUBBAND_MAX)
  },
  {
    .pGroup           = AUDIO_ALGO_OPT_STR("Band 0"),
    .pDescription     = AUDIO_ALGO_OPT_STR("sub-band lower frequency (constant for first band: 0 Hz)"),
    .pControl         = AUDIO_ALGO_OPT_STR("static"),
    .pDefault         = "0",
    .pName            = "compressorFreqCutoff0",
    AUDIO_DESC_PARAM_U32(mdrcDynamicConfig_t, compressor.bandCompressors[0].freqCutoff, 0UL, 24000UL)
  },
  {
    .pDescription     = AUDIO_ALGO_OPT_STR("compressor enable"),
    .pControl         = AUDIO_ALGO_OPT_STR("checkbox"),
    .pDefault         = "1",
    .pName            = "compressorEnable0",
    AUDIO_DESC_PARAM_U8(mdrcDynamicConfig_t, compressor.bandCompressors[0].enable, 0U, 1U)
  },
  {
    .pDescription     = AUDIO_ALGO_OPT_STR("compressor attack time (milli-seconds)"),
    .pControl         = AUDIO_ALGO_OPT_STR("slider"),
    .pDefault         = "100",
    .pName            = "attackTime0",
    AUDIO_DESC_PARAM_F(mdrcDynamicConfig_t, compressor.bandCompressors[0].attackTime, MDRC5B_COM_AT_TIME_MIN, MDRC5B_COM_AT_TIME_MAX)
  },
  {
    .pDescription     = AUDIO_ALGO_OPT_STR("compressor release time (milli-seconds)"),
    .pControl         = AUDIO_ALGO_OPT_STR("slider"),
    .pDefault         = "1000",
    .pName            = "releaseTime0",
    AUDIO_DESC_PARAM_F(mdrcDynamicConfig_t, compressor.bandCompressors[0].releaseTime, MDRC5B_COM_RE_TIME_MIN, MDRC5B_COM_RE_TIME_MAX)
  },
  {
    .pDescription     = AUDIO_ALGO_OPT_STR("compressor curve: input level (dB) as x-axis, output level (dB) as y-axis; Shift click to create a point, Del to remove a point"),
    .pControl         = AUDIO_ALGO_OPT_STR("grapharray"),
    .pCustom          = AUDIO_ALGO_OPT_STR("{\"xAxis\":\"dB\",\"yAxis\":\"dB\"}"),
    .pDefArray        = s_mdrcCompDynResp, // first parameter of this link is number of struct instance (uint32_t), second is the table of these struct instances
    .pDefault         = "{}",
    .pName            = "dynamicResponse0",
    .iParamFlag       = AUDIO_DESC_PARAM_TYPE_FLAG_STRUCT_ARRAY,
    AUDIO_DESC_PARAM_OBJ(mdrcDynamicConfig_t, compressor.bandCompressors[0].dynamicResponse, MDRC5B_KNEEPOINTS_MAX, sizeof(mdrc5b_dyn_resp_point_t))
  },
  {
    .pDescription     = AUDIO_ALGO_OPT_STR("compressor post gain (dB)"),
    .pControl         = AUDIO_ALGO_OPT_STR("slider"),
    .pDefault         = "0",
    .pName            = "compressorPostGain0",
    AUDIO_DESC_PARAM_F(mdrcDynamicConfig_t, compressor.bandCompressors[0].postGain, MDRC5B_POSTGAIN_MIN, MDRC5B_POSTGAIN_MAX)
  },
  #if MDRC5B_SUBBAND_MAX > 1
  {
    .pGroup           = AUDIO_ALGO_OPT_STR("Band 1"),
    .pDescription     = AUDIO_ALGO_OPT_STR("sub-band lower frequency (Hz), equal to next sub-band higher frequency"),
    .pControl         = AUDIO_ALGO_OPT_STR("slider"),
    .pDefault         = "200",
    .pName            = "compressorFreqCutoff1",
    AUDIO_DESC_PARAM_U32(mdrcDynamicConfig_t, compressor.bandCompressors[1].freqCutoff, 0UL, 24000UL)
  },
  {
    .pDescription     = AUDIO_ALGO_OPT_STR("compressor enable"),
    .pControl         = AUDIO_ALGO_OPT_STR("checkbox"),
    .pDefault         = "1",
    .pName            = "compressorEnable1",
    AUDIO_DESC_PARAM_U8(mdrcDynamicConfig_t, compressor.bandCompressors[1].enable, 0U, 1U)
  },
  {
    .pDescription     = AUDIO_ALGO_OPT_STR("compressor attack time (milli-seconds)"),
    .pControl         = AUDIO_ALGO_OPT_STR("slider"),
    .pDefault         = "100",
    .pName            = "attackTime1",
    AUDIO_DESC_PARAM_F(mdrcDynamicConfig_t, compressor.bandCompressors[1].attackTime, MDRC5B_COM_AT_TIME_MIN, MDRC5B_COM_AT_TIME_MAX)
  },
  {
    .pDescription     = AUDIO_ALGO_OPT_STR("compressor release time (milli-seconds)"),
    .pControl         = AUDIO_ALGO_OPT_STR("slider"),
    .pDefault         = "1000",
    .pName            = "releaseTime1",
    AUDIO_DESC_PARAM_F(mdrcDynamicConfig_t, compressor.bandCompressors[1].releaseTime, MDRC5B_COM_RE_TIME_MIN, MDRC5B_COM_RE_TIME_MAX)
  },
  {
    .pDescription     = AUDIO_ALGO_OPT_STR("compressor curve: input level (dB) as x-axis, output level (dB) as y-axis"),
    .pControl         = AUDIO_ALGO_OPT_STR("grapharray"),
    .pCustom          = AUDIO_ALGO_OPT_STR("{\"xAxis\":\"dB\",\"yAxis\":\"dB\"}"),
    .pDefArray        = s_mdrcCompDynResp, // first parameter of this link is number of struct instance (uint32_t), second is the table of these struct instances
    .pDefault         = "{}",
    .pName            = "dynamicResponse1",
    .iParamFlag       = AUDIO_DESC_PARAM_TYPE_FLAG_STRUCT_ARRAY,
    AUDIO_DESC_PARAM_OBJ(mdrcDynamicConfig_t, compressor.bandCompressors[1].dynamicResponse, MDRC5B_KNEEPOINTS_MAX, sizeof(mdrc5b_dyn_resp_point_t))
  },
  {
    .pDescription     = AUDIO_ALGO_OPT_STR("compressor post gain (dB)"),
    .pControl         = AUDIO_ALGO_OPT_STR("slider"),
    .pDefault         = "0",
    .pName            = "compressorPostGain1",
    AUDIO_DESC_PARAM_F(mdrcDynamicConfig_t, compressor.bandCompressors[1].postGain, MDRC5B_POSTGAIN_MIN, MDRC5B_POSTGAIN_MAX)
  },
  #endif
  #if MDRC5B_SUBBAND_MAX > 2
  {
    .pGroup           = AUDIO_ALGO_OPT_STR("Band 2"),
    .pDescription     = AUDIO_ALGO_OPT_STR("sub-band lower frequency (Hz), equal to next sub-band higher frequency"),
    .pControl         = AUDIO_ALGO_OPT_STR("slider"),
    .pDefault         = "500",
    .pName            = "compressorFreqCutoff2",
    AUDIO_DESC_PARAM_U32(mdrcDynamicConfig_t, compressor.bandCompressors[2].freqCutoff, 0UL, 24000UL)
  },
  {
    .pDescription     = AUDIO_ALGO_OPT_STR("compressor enable"),
    .pControl         = AUDIO_ALGO_OPT_STR("checkbox"),
    .pDefault         = "1",
    .pName            = "compressorEnable2",
    AUDIO_DESC_PARAM_U8(mdrcDynamicConfig_t, compressor.bandCompressors[2].enable, 0U, 1U)
  },
  {
    .pDescription     = AUDIO_ALGO_OPT_STR("compressor attack time (milli-seconds)"),
    .pControl         = AUDIO_ALGO_OPT_STR("slider"),
    .pDefault         = "100",
    .pName            = "attackTime2",
    AUDIO_DESC_PARAM_F(mdrcDynamicConfig_t, compressor.bandCompressors[2].attackTime, MDRC5B_COM_AT_TIME_MIN, MDRC5B_COM_AT_TIME_MAX)
  },
  {
    .pDescription     = AUDIO_ALGO_OPT_STR("compressor release time (milli-seconds)"),
    .pControl         = AUDIO_ALGO_OPT_STR("slider"),
    .pDefault         = "1000",
    .pName            = "releaseTime2",
    AUDIO_DESC_PARAM_F(mdrcDynamicConfig_t, compressor.bandCompressors[2].releaseTime, MDRC5B_COM_RE_TIME_MIN, MDRC5B_COM_RE_TIME_MAX)
  },
  {
    .pDescription     = AUDIO_ALGO_OPT_STR("compressor curve: input level (dB) as x-axis, output level (dB) as y-axis"),
    .pControl         = AUDIO_ALGO_OPT_STR("grapharray"),
    .pCustom          = AUDIO_ALGO_OPT_STR("{\"xAxis\":\"dB\",\"yAxis\":\"dB\"}"),
    .pDefArray        = s_mdrcCompDynResp, // first parameter of this link is number of struct instance (uint32_t), second is the table of these struct instances
    .pDefault         = "{}",
    .pName            = "dynamicResponse2",
    .iParamFlag       = AUDIO_DESC_PARAM_TYPE_FLAG_STRUCT_ARRAY,
    AUDIO_DESC_PARAM_OBJ(mdrcDynamicConfig_t, compressor.bandCompressors[2].dynamicResponse, MDRC5B_KNEEPOINTS_MAX, sizeof(mdrc5b_dyn_resp_point_t))
  },
  {
    .pDescription     = AUDIO_ALGO_OPT_STR("compressor post gain (dB)"),
    .pControl         = AUDIO_ALGO_OPT_STR("slider"),
    .pDefault         = "0",
    .pName            = "compressorPostGain2",
    AUDIO_DESC_PARAM_F(mdrcDynamicConfig_t, compressor.bandCompressors[2].postGain, MDRC5B_POSTGAIN_MIN, MDRC5B_POSTGAIN_MAX)
  },
  #endif
  #if MDRC5B_SUBBAND_MAX > 3
  {
    .pGroup           = AUDIO_ALGO_OPT_STR("Band 3"),
    .pDescription     = AUDIO_ALGO_OPT_STR("sub-band lower frequency (Hz), equal to next sub-band higher frequency"),
    .pControl         = AUDIO_ALGO_OPT_STR("slider"),
    .pDefault         = "1000",
    .pName            = "compressorFreqCutoff3",
    AUDIO_DESC_PARAM_U32(mdrcDynamicConfig_t, compressor.bandCompressors[3].freqCutoff, 0UL, 24000UL)
  },
  {
    .pDescription     = AUDIO_ALGO_OPT_STR("compressor enable"),
    .pControl         = AUDIO_ALGO_OPT_STR("checkbox"),
    .pDefault         = "1",
    .pName            = "compressorEnable3",
    AUDIO_DESC_PARAM_U8(mdrcDynamicConfig_t, compressor.bandCompressors[3].enable, 0U, 1U)
  },
  {
    .pDescription     = AUDIO_ALGO_OPT_STR("compressor attack time (milli-seconds)"),
    .pControl         = AUDIO_ALGO_OPT_STR("slider"),
    .pDefault         = "100",
    .pName            = "attackTime3",
    AUDIO_DESC_PARAM_F(mdrcDynamicConfig_t, compressor.bandCompressors[3].attackTime, MDRC5B_COM_AT_TIME_MIN, MDRC5B_COM_AT_TIME_MAX)
  },
  {
    .pDescription     = AUDIO_ALGO_OPT_STR("compressor release time (milli-seconds)"),
    .pControl         = AUDIO_ALGO_OPT_STR("slider"),
    .pDefault         = "1000",
    .pName            = "releaseTime3",
    AUDIO_DESC_PARAM_F(mdrcDynamicConfig_t, compressor.bandCompressors[3].releaseTime, MDRC5B_COM_RE_TIME_MIN, MDRC5B_COM_RE_TIME_MAX)
  },
  {
    .pDescription     = AUDIO_ALGO_OPT_STR("compressor curve: input level (dB) as x-axis, output level (dB) as y-axis"),
    .pControl         = AUDIO_ALGO_OPT_STR("grapharray"),
    .pCustom          = AUDIO_ALGO_OPT_STR("{\"xAxis\":\"dB\",\"yAxis\":\"dB\"}"),
    .pDefArray        = s_mdrcCompDynResp, // first parameter of this link is number of struct instance (uint32_t), second is the table of these struct instances
    .pDefault         = "{}",
    .pName            = "dynamicResponse3",
    .iParamFlag       = AUDIO_DESC_PARAM_TYPE_FLAG_STRUCT_ARRAY,
    AUDIO_DESC_PARAM_OBJ(mdrcDynamicConfig_t, compressor.bandCompressors[3].dynamicResponse, MDRC5B_KNEEPOINTS_MAX, sizeof(mdrc5b_dyn_resp_point_t))
  },
  {
    .pDescription     = AUDIO_ALGO_OPT_STR("compressor post gain (dB)"),
    .pControl         = AUDIO_ALGO_OPT_STR("slider"),
    .pDefault         = "0",
    .pName            = "compressorPostGain3",
    AUDIO_DESC_PARAM_F(mdrcDynamicConfig_t, compressor.bandCompressors[3].postGain, MDRC5B_POSTGAIN_MIN, MDRC5B_POSTGAIN_MAX)
  },
  #endif
  #if MDRC5B_SUBBAND_MAX > 4
  {
    .pGroup           = AUDIO_ALGO_OPT_STR("Band 4"),
    .pDescription     = AUDIO_ALGO_OPT_STR("sub-band lower frequency (Hz), equal to next sub-band higher frequency"),
    .pControl         = AUDIO_ALGO_OPT_STR("slider"),
    .pDefault         = "2000",
    .pName            = "compressorFreqCutoff4",
    AUDIO_DESC_PARAM_U32(mdrcDynamicConfig_t, compressor.bandCompressors[4].freqCutoff, 0UL, 24000UL)
  },
  {
    .pDescription     = AUDIO_ALGO_OPT_STR("compressor enable"),
    .pControl         = AUDIO_ALGO_OPT_STR("checkbox"),
    .pDefault         = "1",
    .pName            = "compressorEnable4",
    AUDIO_DESC_PARAM_U8(mdrcDynamicConfig_t, compressor.bandCompressors[4].enable, 0U, 1U)
  },
  {
    .pDescription     = AUDIO_ALGO_OPT_STR("compressor attack time (milli-seconds)"),
    .pControl         = AUDIO_ALGO_OPT_STR("slider"),
    .pDefault         = "100",
    .pName            = "attackTime4",
    AUDIO_DESC_PARAM_F(mdrcDynamicConfig_t, compressor.bandCompressors[4].attackTime, MDRC5B_COM_AT_TIME_MIN, MDRC5B_COM_AT_TIME_MAX)
  },
  {
    .pDescription     = AUDIO_ALGO_OPT_STR("compressor release time (milli-seconds)"),
    .pControl         = AUDIO_ALGO_OPT_STR("slider"),
    .pDefault         = "1000",
    .pName            = "releaseTime4",
    AUDIO_DESC_PARAM_F(mdrcDynamicConfig_t, compressor.bandCompressors[4].releaseTime, MDRC5B_COM_RE_TIME_MIN, MDRC5B_COM_RE_TIME_MAX)
  },
  {
    .pDescription     = AUDIO_ALGO_OPT_STR("compressor curve: input level (dB) as x-axis, output level (dB) as y-axis"),
    .pControl         = AUDIO_ALGO_OPT_STR("grapharray"),
    .pCustom          = AUDIO_ALGO_OPT_STR("{\"xAxis\":\"dB\",\"yAxis\":\"dB\"}"),
    .pDefArray        = s_mdrcCompDynResp, // first parameter of this link is number of struct instance (uint32_t), second is the table of these struct instances
    .pDefault         = "{}",
    .pName            = "dynamicResponse4",
    .iParamFlag       = AUDIO_DESC_PARAM_TYPE_FLAG_STRUCT_ARRAY,
    AUDIO_DESC_PARAM_OBJ(mdrcDynamicConfig_t, compressor.bandCompressors[4].dynamicResponse, MDRC5B_KNEEPOINTS_MAX, sizeof(mdrc5b_dyn_resp_point_t))
  },
  {
    .pDescription     = AUDIO_ALGO_OPT_STR("compressor post gain (dB)"),
    .pControl         = AUDIO_ALGO_OPT_STR("slider"),
    .pDefault         = "0",
    .pName            = "compressorPostGain4",
    AUDIO_DESC_PARAM_F(mdrcDynamicConfig_t, compressor.bandCompressors[4].postGain, MDRC5B_POSTGAIN_MIN, MDRC5B_POSTGAIN_MAX)
  },
  #endif
  {
    .pGroup           = AUDIO_ALGO_OPT_STR("Limiter"),
    .pDescription     = AUDIO_ALGO_OPT_STR("attack time (milli-seconds)"),
    .pControl         = AUDIO_ALGO_OPT_STR("slider"),
    .pDefault         = "1",
    .pName            = "attackTime",
    AUDIO_DESC_PARAM_F(mdrcDynamicConfig_t, limiter.attackTime, MDRC5B_LIMITER_AT_TIME_MIN, MDRC5B_LIMITER_AT_TIME_MAX)
  },
  {
    .pDescription     = AUDIO_ALGO_OPT_STR("release time (milli-seconds)"),
    .pControl         = AUDIO_ALGO_OPT_STR("slider"),
    .pDefault         = "100",
    .pName            = "releaseTime",
    AUDIO_DESC_PARAM_F(mdrcDynamicConfig_t, limiter.releaseTime, MDRC5B_LIMITER_RE_TIME_MIN, MDRC5B_LIMITER_RE_TIME_MAX)
  },
  {
    .pDescription     = AUDIO_ALGO_OPT_STR("threshold (dB), i.e. max output level"),
    .pControl         = AUDIO_ALGO_OPT_STR("slider"),
    .pDefault         = "-3",
    .pName            = "limiterThreshold",
    AUDIO_DESC_PARAM_F(mdrcDynamicConfig_t, limiter.thresh, MDRC5B_LIMITER_THRESH_DB_MIN, MDRC5B_LIMITER_THRESH_DB_MAX)
  }
};

static const audio_descriptor_param_t s_mdrc_staticParamsDesc[] =
{
  {
    .pDescription     = AUDIO_ALGO_OPT_STR("RAM type"),
    .pControl         = AUDIO_ALGO_OPT_STR("droplist"),
    .pKeyValue        = tRamTypeKeyValue,
    .pDefault         = AUDIOCHAINFACTORY_INT2STR(AUDIO_MEM_RAMINT),
    .iParamFlag       = AUDIO_DESC_PARAM_TYPE_FLAG_DEFINE_KEY,
    .pName            = "ramType",
    AUDIO_DESC_PARAM_U8(mdrcStaticConfig_t, ramType, 0U, AUDIO_ALGO_RAM_TYPES_NB - 1U)
  }
};

END_IGNORE_DIV0_WARNING
/*cstat +MISRAC2012-Rule-1.4_a*/

static const audio_descriptor_params_t s_mdrc_staticParamTemplate =
{
  .pParam   = (audio_descriptor_param_t *)s_mdrc_staticParamsDesc,
  .nbParams = sizeof(s_mdrc_staticParamsDesc) / sizeof(s_mdrc_staticParamsDesc[0]),
  .szBytes  = sizeof(mdrcStaticConfig_t),
};

static const audio_descriptor_params_t s_mdrc_dynamicParamTemplate =
{
  .pParam   = (audio_descriptor_param_t *)s_mdrc_dynamicParamsDesc,
  .nbParams = sizeof(s_mdrc_dynamicParamsDesc) / sizeof(s_mdrc_dynamicParamsDesc[0]),
  .szBytes  = sizeof(mdrcDynamicConfig_t),
};

#endif  // AUDIO_CHAIN_ACSDK_USED || AUDIO_CHAIN_CONF_TUNING_CLI_USED

const audio_algo_factory_t AudioChainWrp_mdrc_factory =
{
  .pStaticParamTemplate  = AUDIO_ALGO_OPT_TUNING(&s_mdrc_staticParamTemplate),
  .pDynamicParamTemplate = AUDIO_ALGO_OPT_TUNING(&s_mdrc_dynamicParamTemplate),
  .pControlTemplate      = AUDIO_ALGO_OPT_TUNING(NULL),
  .pCapabilities         = &AudioChainWrp_mdrc_common,
  .pExecutionCbs         = &AudioChainWrp_mdrc_cbs
};

// ALGO_FACTORY_DECLARE(AudioChainWrp_mdrc_factory);
