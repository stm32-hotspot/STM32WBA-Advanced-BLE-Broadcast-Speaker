/**
******************************************************************************
* @file    audio_descriptor.h
* @author  MCD Application Team
* @brief   algo tuning.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __AUDIO_DESCRIPTOR_H
#define __AUDIO_DESCRIPTOR_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <math.h>
#include <stddef.h>
#include <stdarg.h>

/* Exported constants --------------------------------------------------------*/
#define AUDIO_ALGO_FORMAT_UNDEF                   (0U) /* used to check if algo capabilities defined by wrapper */
#define AUDIO_ALGO_SIGNATURE                      0x4F474C41U /* 'ALGO' */

#define AUDIO_ALGO_FLAGS_MISC_INIT_DEFAULT        (1U << 0)
#define AUDIO_ALGO_FLAGS_MISC_IGNORE_PININ        (1U << 1)
#define AUDIO_ALGO_FLAGS_MISC_IGNORE_PINOUT       (1U << 2)
#define AUDIO_ALGO_FLAGS_MISC_OPTIONAL_INPUTS     (1U << 3)
#define AUDIO_ALGO_FLAGS_MISC_IGNORE_ALGO_PARAMS  (1U << 4)     /* params already present in the json plugin and are stronger than the algo exposed to (to allow devel overload) */
#define AUDIO_ALGO_FLAGS_MISC_DISABLE_AUTO_MOUNT  (1U << 5)     /* The element must have a be mounted by a specific livetune plugin */
#define AUDIO_ALGO_FLAGS_MISC_SINGLE_INSTANCE     (1U << 6)     /* The Element must be present only once in the graph */

/* Exported types ------------------------------------------------------------*/
typedef uint32_t audio_descriptor_param_flag_t;

/* These flags can be combined to form a parameter behaviours */
#define AUDIO_DESC_PARAM_TYPE_FLAG_TRANSLATE_KEY_VALUE (1UL << 0)   /* this means the final value must be translated using the pKeyValue pointer */
#define AUDIO_DESC_PARAM_TYPE_FLAG_AS_KEY_VALUE        (1UL << 1)   /* this means the control must build a drop list of key integer values */
#define AUDIO_DESC_PARAM_TYPE_FLAG_AS_ARRAY            (1UL << 2)   /* this means the control must build a array of struct  definition */
#define AUDIO_DESC_PARAM_TYPE_FLAG_DISABLED            (1UL << 3)   /* the value is disabled, and must be set programaticaly only, concontril will be grayed and not modifiable */
#define AUDIO_DESC_PARAM_TYPE_FLAG_STOP_GRAPH          (1UL << 4)   /* the control will stop the graph, as soon as a value is changed */
#define AUDIO_DESC_PARAM_TYPE_FLAG_WANT_APPLY          (1UL << 5)   /* some controls need to apply the modification at once, if it is not present, the control will apply changes as soon as a value is changed */
#define AUDIO_DESC_PARAM_TYPE_FLAG_TOOL                (1UL << 6)   /* Notify, the control is not a value and will not produce a set config  , it is a tool that will apply a setting on other parameters */
#define AUDIO_DESC_PARAM_TYPE_FLAG_PRIVATE             (1UL << 7)   /* Notify, the control is not an audio_chain control and must not be exported as static or dynamic param, mainly used for FBF*/
#define AUDIO_DESC_PARAM_TYPE_FLAG_AS_KEY_STRING       (1UL << 8)   /* this means the control must build a drop list of key string preset values */
#define AUDIO_DESC_PARAM_TYPE_FLAG_ALWAYS_DEFAULT      (1UL << 9)   /* this means the control value is ignored and use automaicaly the default value, it is a workaround for livetune pointed (such as soundfont, that change according to the board, the generator will generate always the default value  */

/* Here are some flags that define the control behaviours */
#define AUDIO_DESC_PARAM_TYPE_FLAG_DEFINE_KEY          (AUDIO_DESC_PARAM_TYPE_FLAG_TRANSLATE_KEY_VALUE | AUDIO_DESC_PARAM_TYPE_FLAG_AS_KEY_VALUE)   /* the control has a drop list and translates the key in value for audio chain. */
#define AUDIO_DESC_PARAM_TYPE_FLAG_COMBO_KEY           (AUDIO_DESC_PARAM_TYPE_FLAG_AS_KEY_VALUE)                                                    /* the control has a drop list numeric, there is not translation */
#define AUDIO_DESC_PARAM_TYPE_FLAG_STRUCT_ARRAY        (AUDIO_DESC_PARAM_TYPE_FLAG_AS_ARRAY)                                                        /* the control is an ARRAY and must use the pDefArray */

typedef enum audio_descriptor_param_type
{
  AUDIO_DESC_PARAM_TYPE_INT8,
  AUDIO_DESC_PARAM_TYPE_UINT8,
  AUDIO_DESC_PARAM_TYPE_INT16,
  AUDIO_DESC_PARAM_TYPE_UINT16,
  AUDIO_DESC_PARAM_TYPE_INT32,
  AUDIO_DESC_PARAM_TYPE_UINT32,
  AUDIO_DESC_PARAM_TYPE_FLOAT,
  AUDIO_DESC_PARAM_TYPE_ADDRESS,
  AUDIO_DESC_PARAM_TYPE_OBJECT,
  AUDIO_ALGO_NB_PARAM_TYPES
} audio_descriptor_param_type_t;

typedef struct audio_descriptor_key_value
{
  const char                              *pKey;
  uint32_t                                 iValue;
} audio_descriptor_key_value_t;

typedef struct audio_descriptor_key_string
{
  const char                              *pKey;
  const char                              *pValue;
} audio_descriptor_key_string_t;

typedef union audio_descriptor_type_union
{
  int8_t                                   s8;
  uint8_t                                  u8;
  int16_t                                  s16;
  uint16_t                                 u16;
  int32_t                                  s32;
  uint32_t                                 u32;
  uint32_t                                 p;
  float                                    f;
} audio_descriptor_type_union_t;

typedef struct audio_algo_control_param
{
  const char                             *pDescription;
  const char                             *pName;
  uint32_t                                iOffset;
  audio_descriptor_param_type_t           paramType;
} audio_algo_control_param_t;

typedef struct audio_algo_control_params_descr
{
  uint8_t                                 nbParams;
  const audio_algo_control_param_t       *pParams;
} audio_algo_control_params_descr_t;


/******************************************************************************************************************
audio_descriptor_param_t:
  when (iParamFlag & AUDIO_DESC_PARAM_TYPE_FLAG_AS_ARRAY) != 0, param is a struct array which must follow this definition:
    struct
    {
      uint32_t <number of structure instances>;
      <struct type> <struct name>[<max number of structure instances>];
    } <struct array>;

  with:
    pDefArray must point to a descriptor of a <struct type> structure instance, let's call it <struct array descriptor>
    structDescr.structNbMaxInstances = <max number of structure instances>
    structDescr.structSize           = sizeof(<struct type>)
    ended with audio_descriptor_param_t element with pName set to NULL

******************************************************************************************************************/
typedef struct audio_descriptor_param
{
  const char                              *pName;
  const char                              *pDescription;
  const char                              *pControl;
  const char                              *pCustom;
  const char                              *pGroup;
  audio_descriptor_param_flag_t            iParamFlag;
  audio_descriptor_param_type_t            paramType;
  union
  {
    const audio_descriptor_key_value_t    *pKeyValue;
    const audio_descriptor_key_string_t   *pKeyString;
    const struct audio_descriptor_param   *pDefArray;
  };
  uint32_t                                 iOffset;
  const char                              *pDefault;
  union
  {
    struct
    {
      audio_descriptor_type_union_t        valMin, valMax;
    } limits;
    struct
    {
      uint32_t                             structNbMaxInstances;
      size_t                               structSize;
    } structDescr;
  };
} audio_descriptor_param_t;

typedef struct audio_descriptor_params
{
  uint8_t                                  nbParams;
  audio_descriptor_param_t                *pParam;
  uint32_t                                 szBytes;
} audio_descriptor_params_t;

/* Exported variables --------------------------------------------------------*/
extern const audio_descriptor_key_value_t tRamTypeKeyValue[];
#define AUDIO_ALGO_RAM_TYPES_NB 3U

/* Exported macros -----------------------------------------------------------*/

/* above AUDIO_DESC_PARAM_<type> macros have been defined for each param type (uint8_t, int8_t, uint16_t, int16_t, uint32_t, int32_t, float)
   they set paramType, iOffset and limits fields of param descriptor insuring consistency between all these fields
   if a wrong macro is used for a parameter description, there will be a build issue (division by 0)
   - type:    params struct typedef
   - member:  param field inside params structure
   - val_min: minimum value for this param
   - val_max: maximum value for this param

   remark: I couldn't find a way to use a unique macro which use the right limits field depending the member type,
           that's why there is a macro per type and the macro fails if we don't use the right one

   the AUDIO_DESC_PARAM_ADDR macro has also been added but it hasn't exactly the same purpose: it doesn't check
   if member is a pointer and it automatically sets min & max values to 0x00000000 and 0xffffffff respectively

   the AUDIO_DESC_PARAM_OBJ macro has also been added but it hasn't exactly the same puspose but it set the same fields
   for struct arrays (structDescr and limits are union between each others)

   same kind of macros AUDIO_DESC_CONTROL_<type> for control parameters (without val_min & val_max)
*/
#ifdef __GNUC__
#define BEGIN_IGNORE_DIV0_WARNING                     \
  _Pragma("GCC diagnostic push")                      \
  _Pragma("GCC diagnostic ignored \"-Wdiv-by-zero\"")
#define END_IGNORE_DIV0_WARNING                       \
  _Pragma("GCC diagnostic pop")
#else
#define BEGIN_IGNORE_DIV0_WARNING
#define END_IGNORE_DIV0_WARNING
#endif

#define AUDIO_DESC_PARAM_U8(type, member, val_min, val_max)                                                   \
  .paramType         = _Generic(((type *)0)->member, uint8_t  : AUDIO_DESC_PARAM_TYPE_UINT8,  default : 1/0), \
  .iOffset           = offsetof(type, member),                                                                \
  .limits.valMin.u8  = val_min,                                                                               \
  .limits.valMax.u8  = val_max

#define AUDIO_DESC_PARAM_S8(type, member, val_min, val_max)                                                   \
  .paramType         = _Generic(((type *)0)->member, int8_t   : AUDIO_DESC_PARAM_TYPE_INT8,   default : 1/0), \
  .iOffset           = offsetof(type, member),                                                                \
  .limits.valMin.s8  = val_min,                                                                               \
  .limits.valMax.s8  = val_max,

#define AUDIO_DESC_PARAM_U16(type, member, val_min, val_max)                                                  \
  .paramType         = _Generic(((type *)0)->member, uint16_t : AUDIO_DESC_PARAM_TYPE_UINT16, default : 1/0), \
  .iOffset           = offsetof(type, member),                                                                \
  .limits.valMin.u16 = val_min,                                                                               \
  .limits.valMax.u16 = val_max

#define AUDIO_DESC_PARAM_S16(type, member, val_min, val_max)                                                  \
  .paramType         = _Generic(((type *)0)->member, int16_t  : AUDIO_DESC_PARAM_TYPE_INT16,  default : 1/0), \
  .iOffset           = offsetof(type, member),                                                                \
  .limits.valMin.s16 = val_min,                                                                               \
  .limits.valMax.s16 = val_max

#define AUDIO_DESC_PARAM_U32(type, member, val_min, val_max)                                                  \
  .paramType         = _Generic(((type *)0)->member, uint32_t : AUDIO_DESC_PARAM_TYPE_UINT32, default : 1/0), \
  .iOffset           = offsetof(type, member),                                                                \
  .limits.valMin.u32 = val_min,                                                                               \
  .limits.valMax.u32 = val_max

#define AUDIO_DESC_PARAM_S32(type, member, val_min, val_max)                                                  \
  .paramType         = _Generic(((type *)0)->member, int32_t  : AUDIO_DESC_PARAM_TYPE_INT32,  default : 1/0), \
  .iOffset           = offsetof(type, member),                                                                \
  .limits.valMin.s32 = val_min,                                                                               \
  .limits.valMax.s32 = val_max

#define AUDIO_DESC_PARAM_F(type, member, val_min, val_max)                                                    \
  .paramType         = _Generic(((type *)0)->member, float    : AUDIO_DESC_PARAM_TYPE_FLOAT,  default : 1/0), \
  .iOffset           = offsetof(type, member),                                                                \
  .limits.valMin.f   = val_min,                                                                               \
  .limits.valMax.f   = val_max

#define AUDIO_DESC_PARAM_ADDR(type, member)                                                                   \
  .paramType         = AUDIO_DESC_PARAM_TYPE_ADDRESS,                                                         \
  .iOffset           = offsetof(type, member),                                                                \
  .limits.valMin.u32 = 0x00000000UL,                                                                          \
  .limits.valMax.u32 = 0xFFFFFFFFUL

#define AUDIO_DESC_PARAM_OBJ(type, member, nbMaxInst, objSize)                                                \
  .paramType                        = AUDIO_DESC_PARAM_TYPE_OBJECT,                                           \
  .iOffset                          = offsetof(type, member),                                                 \
  .structDescr.structNbMaxInstances = nbMaxInst,                                                              \
  .structDescr.structSize           = objSize


#define AUDIO_DESC_CONTROL_U8(type, member)                                                                   \
  .paramType         = _Generic(((type *)0)->member, uint8_t  : AUDIO_DESC_PARAM_TYPE_UINT8,  default : 1/0), \
  .iOffset           = offsetof(type, member)

#define AUDIO_DESC_CONTROL_S8(type, member)                                                                   \
  .paramType         = _Generic(((type *)0)->member, int8_t   : AUDIO_DESC_PARAM_TYPE_INT8,   default : 1/0), \
  .iOffset           = offsetof(type, member)

#define AUDIO_DESC_CONTROL_U16(type, member)                                                                  \
  .paramType         = _Generic(((type *)0)->member, uint16_t : AUDIO_DESC_PARAM_TYPE_UINT16, default : 1/0), \
  .iOffset           = offsetof(type, member)

#define AUDIO_DESC_CONTROL_S16(type, member)                                                                  \
  .paramType         = _Generic(((type *)0)->member, int16_t  : AUDIO_DESC_PARAM_TYPE_INT16,  default : 1/0), \
  .iOffset           = offsetof(type, member)

#define AUDIO_DESC_CONTROL_U32(type, member)                                                                  \
  .paramType         = _Generic(((type *)0)->member, uint32_t : AUDIO_DESC_PARAM_TYPE_UINT32, default : 1/0), \
  .iOffset           = offsetof(type, member)

#define AUDIO_DESC_CONTROL_S32(type, member)                                                                  \
  .paramType         = _Generic(((type *)0)->member, int32_t  : AUDIO_DESC_PARAM_TYPE_INT32,  default : 1/0), \
  .iOffset           = offsetof(type, member)

#define AUDIO_DESC_CONTROL_F(type, member)                                                                    \
  .paramType         = _Generic(((type *)0)->member, float    : AUDIO_DESC_PARAM_TYPE_FLOAT,  default : 1/0), \
  .iOffset           = offsetof(type, member)

#define AUDIO_DESC_CONTROL_ADDR(type, member)                                                                 \
  .paramType         = AUDIO_DESC_PARAM_TYPE_ADDRESS,                                                         \
  .iOffset           = offsetof(type, member)

#define AUDIO_DESC_CONTROL_OBJ(type, member)                                                                  \
  .paramType         = AUDIO_DESC_PARAM_TYPE_OBJECT,                                                          \
  .iOffset           = offsetof(type, member)

/* Exported functions ------------------------------------------------------- */
int32_t      AudioDescriptor_applyParamConfigStr(const audio_descriptor_param_t *const pParam, void *const pConfig, const char *const pStrVal, char **ppErrorString);
void         AudioDescriptor_getMinMax(const audio_descriptor_param_t           *const pParam, double *const pMin, double *const pMax);

/* Key <=> value routines */
int32_t      AudioDescriptor_getValueFromKey(const audio_descriptor_key_value_t *const pKeyValue, const char *const pKey, uint32_t *const pValue, char **ppErrorString);
const char  *AudioDescriptor_getKeyFromKeyId(const audio_descriptor_key_value_t *const pKeyValue, uint32_t const keyId);

int32_t      AudioDescriptor_getParam(const audio_descriptor_params_t *const pParamTemplate, const char *const pKey, const audio_descriptor_param_t **const ppParamDesc, char **ppErrorString);
void         AudioDescriptor_getParamValueString(const audio_descriptor_param_t *const pParam, void *const pData, char *const pString, uint32_t const szString);

#ifdef __cplusplus
}
#endif

#endif  /* __AUDIO_DESCRIPTOR_H */


