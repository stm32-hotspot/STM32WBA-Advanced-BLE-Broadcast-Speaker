/**
  ******************************************************************************
  * @file    biquad.h
  * @author  MCD Application Team
  * @brief   biquad filtering algo
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

#ifndef __BIQUAD_H
#define __BIQUAD_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "audio_mem_mgnt.h"
#include "audio_buffer.h"
#include "common/biquad_typedef.h"

/* Exported constants --------------------------------------------------------*/
#define MAX_BIQUAD_CELLS       32
#define IIR_RESPONSE_ACCURACY 2048

/* Exported types ------------------------------------------------------------*/

/******************************/
/* structures used for design */
/******************************/
typedef enum
{
  NOTCH,
  PEAKING,
  LOWPASS,
  HIGHPASS,
  BANDPASS,
  LOWSHELF,
  HIGHSHELF,
  IIREQ_TYPE_NB
} iirEqType_t;

typedef enum
{
  STEEPNESS_Q,
  STEEPNESS_BW,
  IIREQ_STEEPNESS_NB
} iirEqQorBW_t;

typedef struct
{
  uint8_t              iirType;
  uint8_t              steepnessByQorBW;  /* biquad steepness defined by band-width or by Q factor */
  float                steepness;         /* Q factor or band-width (Hz) depending on steepnessByQorBW value */
  float                centerCutoffFrequency;
  float                gain_dB;
} biquadConfig_t;

typedef enum
{
  BIQUAD_COMPUTE_MODULE_ONLY,
  BIQUAD_COMPUTE_MODULE_AND_NORMALIZE
} biquadNormalize_t;

typedef enum
{
  BIQUAD_NO_SORT,
  BIQUAD_UP_SORT,   // see matlab zp2sos order parameter: order the sections so the first row of sos contains the poles farthest from the unit circle.
  BIQUAD_DOWN_SORT, // see matlab zp2sos order parameter: order the sections so the first row of sos contains the poles closest to the unit circle.
  BIQUAD_SORT_NB
} biquadSort_t;

typedef struct
{
  float                      q0, q1, q2;
} quad_t;

typedef struct
{
  float                      re, im;
} complex_t;

typedef struct complex_root
{
  complex_t                  root;
  float                      gain;
  float                      distanceToUnitCircle;
} complex_root_t;

typedef struct real_root
{
  float                      root;
  float                      gain;
  float                      distanceToUnitCircle;
} real_root_t;

typedef struct
{
  enum
  {
    NO_ROOT,
    REAL_ROOT,
    COMPLEX_ROOT
  }                          rootType;
  union
  {
    complex_root_t           complexRoot;
    real_root_t              realRoot;
  }                          root;
} root_t;

typedef struct biquad_cell_float
{
  quad_t                     num, den;
  root_t                     zeros[2], poles[2];
} biquad_cell_float_t;

typedef struct
{
  int                        nbCells;
  int                        nbPoles, nbComplexPoles, nbRealPoles;
  int                        nbZeros, nbComplexZeros, nbRealZeros;
  biquad_cell_float_t        biquadCells[MAX_BIQUAD_CELLS];
  root_t                    *pZeros[2 * MAX_BIQUAD_CELLS];
  root_t                    *pPoles[2 * MAX_BIQUAD_CELLS];
} IIR_filter_float_t;

typedef struct
{
  memPool_t                  memPool;
  audio_buffer_type_t        sampleType;
  union
  {
    void                    *pBiquadVoidCoef;
    biquadInt32_t           *pBiquadIntCoef;
    biquadFloat_t           *pBiquadFloatCoef;
  }                          biquadCoef;
} biquadDesignCtx_t;

/*********************************/
/* structures used for filtering */
/*********************************/
typedef struct biquadIntContextStruct
{
  memPool_t            memPool;
  biquadInt32_t const *pBiquad;
  int                  nbChannels;
  int                  decimFactor;
  int                  upSamplingFactor;
  int32_t              gainMant;
  uint8_t              gainExp;
  int32_t             *pBiquadMem;     // pBiquadMem size = ((2 * nbBiquadCells + 2) * nbChannels) samples
  void (*pBiquadProcess)(struct biquadIntContextStruct *const pCtx, void *const in, void *const out, int const nbSamplesIn);
} biquadIntContext_t;

typedef struct biquadFloatContextStruct
{
  memPool_t            memPool;
  biquadFloat_t const *pBiquad;
  int                  nbChannels;
  int                  decimFactor;
  int                  upSamplingFactor;
  float               *pBiquadMem;     // pBiquadMem size = ((2 * nbBiquadCells + 2) * nbChannels) samples
  void (*pBiquadProcess)(struct biquadFloatContextStruct *const pCtx, float *const in, float *const out, int const nbSamplesIn);
} biquadFloatContext_t;

/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

/*******************/
/* design routines */
/******************/
void    biquadDesign_computeRootsFromConfig(biquadConfig_t const *const pBiquadConfig, float const samplingRate, biquad_cell_float_t *const pBiquadCell);
int32_t biquadDesign_computeQuadRoots(quad_t const *const pQuad, root_t *const pRoot);
int32_t biquadDesign_sort(IIR_filter_float_t *const pIirFilterFloat, biquadSort_t const biquadSort);
float   biquadDesign_normalizeBiquadCells(IIR_filter_float_t *const pIirFilterFloat, float *const pModule2, biquadNormalize_t const normalize);
int32_t biquadDesign_quantifyBiquadFilter(IIR_filter_float_t *const pIirFilterFloat, float const gain0, biquadDesignCtx_t *const pBiquadDesignCtx, float *const pModule2);
void    biquadDesign_freeBiquadFilter(biquadDesignCtx_t *const pBiquadDesignCtx);

/**********************/
/* filtering routines */
/**********************/
int32_t biquadInt16Init(biquadIntContext_t           *const pCtx, biquadInt32_t const *const pBiquad, int const nbChannels, int const decimFactor, int const upSamplingFactor, memPool_t const memPool);
void    biquadInt16DeInit(biquadIntContext_t         *const pCtx);
void    biquadInt16Process(biquadIntContext_t        *const pCtx, int16_t *const in, int16_t *const out, int const nbSamplesIn);
void    biquadInt16ClearHistory(biquadIntContext_t   *const pCtx);

int32_t biquadInt32Init(biquadIntContext_t           *const pCtx, biquadInt32_t const *const pBiquad, int const nbChannels, int const decimFactor, int const upSamplingFactor, memPool_t const memPool);
void    biquadInt32DeInit(biquadIntContext_t         *const pCtx);
void    biquadInt32Process(biquadIntContext_t        *const pCtx, int32_t *const in, int32_t *const out, int const nbSamplesIn);
void    biquadInt32ClearHistory(biquadIntContext_t   *const pCtx);

int32_t biquadFloatInit(biquadFloatContext_t         *const pCtx, biquadFloat_t const *const pBiquad, int const nbChannels, int const decimFactor, int const upSamplingFactor, memPool_t const memPool);
void    biquadFloatDeInit(biquadFloatContext_t       *const pCtx);
void    biquadFloatProcess(biquadFloatContext_t      *const pCtx, float *const in, float *const out, int const nbSamplesIn);
void    biquadFloatClearHistory(biquadFloatContext_t *const pCtx);

#ifdef __cplusplus
}
#endif

#endif // __BIQUAD_H
