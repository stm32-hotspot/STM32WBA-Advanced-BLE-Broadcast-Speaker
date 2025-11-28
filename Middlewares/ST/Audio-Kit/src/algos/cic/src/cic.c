/**
******************************************************************************
* @file    cic.c
* @author  MCD Application Team
* @brief   wrapper of cic algo to match usage inside audio_chain.c
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
#include "cic/audio_chain_cic.h"
#include "cic/src/cic.h"
//#include "cic/src/scripts/cic_design.h"
//#include "cic/src/scripts/cic_design.c"
/*cstat -MISRAC2012-* CMSIS not misra compliant */
#include <arm_math.h>
/*cstat +MISRAC2012-* */
#include <stdint.h>
#include <stddef.h>

/* Private defines -----------------------------------------------------------*/
#define SIZEOF_ALIGN AUDIO_MEM_SIZEOF_ALIGN
#define CIC_FRAME_SIZE 8U                   //fixed at 8 to limit memory consumption. 
#define CIC_ORDER_MAX  5U
//#define CIC_LOOP_UNROLL

/* Private typedef -----------------------------------------------------------*/

typedef struct
{
  // Following commented field were used for generic dfsdm model that consumes to high CPU to run on board
  //  uint16_t        inCnt;               /* Input sample counter                */
  //  uint16_t        nextDecimCnt;        /* Next decimation point               */
  //  uint16_t        integratorCnt;       /* Integrator sample counter           */
  //  int32_t         curIntegrator;       /* Current integrator pSum             */
  //  int32_t         fastSyncState[2];    /* State of FastSync pipeline          */
  int32_t        *pComb;               /* State of comb pipeline              */
  int32_t        *pDiff;               /* Output of comb subtractors          */
  int32_t        *pSum;                /* State of integrator pipeline        */
} cic_ch_context_t;


typedef struct
{
  audio_algo_cb_t     process_cb;
  uint8_t             rbs;              /* Right Bit Shift for attenuation    */
  uint8_t             nbChannels;       /* number of channels                 */
  uint32_t            decRatio;         /* decimation ratio                   */
  uint32_t            decRatioPacket;   /* number of byte packet to accumulate for decimation*/
  uint32_t            nbPdmSamples;     /* number of 1-bit PDM samples        */
  uint32_t            nbPdmBytesPacket; /* number of 8-bit PDM packet         */

  /* filter state parameters (modified by the function)                       */
  audio_chunk_t      *pChunkIn ;       /* avoid calling getter during dataIos */
  audio_chunk_t      *pChunkOut;       /* avoid calling getter during dataIos */
  cic_ch_context_t *pCh;
  //  uint16_t            inCntMax;

  //  int32_t **ppLookUp; // Pointer to an array of pointers for lookup tables

  /* Lookup under ifdef to minimize memory usage */
  int32_t pLookUp0[256]; // Pointer to an array of pointers for lookup tables
  #if CIC_ORDER_MAX > 1U
  int32_t pLookUp1[256]; // Pointer to an array of pointers for lookup tables
  #else
  int32_t *pLookUp1; // Pointer to an array of pointers for lookup tables
  #endif
  #if CIC_ORDER_MAX > 2U
  int32_t pLookUp2[256]; // Pointer to an array of pointers for lookup tables
  #else
  int32_t *pLookUp2; // Pointer to an array of pointers for lookup tables
  #endif
  #if CIC_ORDER_MAX > 3U
  int32_t pLookUp3[256]; // Pointer to an array of pointers for lookup tables
  #else
  int32_t *pLookUp3; // Pointer to an array of pointers for lookup tables
  #endif
  #if CIC_ORDER_MAX > 4U
  int32_t pLookUp4[256]; // Pointer to an array of pointers for lookup tables
  #else
  int32_t *pLookUp4; // Pointer to an array of pointers for lookup tables
  #endif

  memPool_t memPool;
} cic_context_t;


/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
//static int32_t s_compute_lookUpSize(uint16_t frame_size, uint8_t cic_order);
static int32_t s_compute_lookUp(cic_context_t *const pContext, uint8_t frame_size, uint8_t cic_order, bool msb_first);
static int32_t s_CIC3_LUT8b_pcm16bit(audio_algo_t *const pAlgo);
static int32_t s_CIC4_LUT8b_pcm16bit(audio_algo_t *const pAlgo);
static int32_t s_CIC5_LUT8b_pcm16bit(audio_algo_t *const pAlgo);
static int32_t s_CIC3_LUT8b_pcm32bit(audio_algo_t *const pAlgo);
static int32_t s_CIC4_LUT8b_pcm32bit(audio_algo_t *const pAlgo);
static int32_t s_CIC5_LUT8b_pcm32bit(audio_algo_t *const pAlgo);

/* Global variables ----------------------------------------------------------*/
/* Functions Definition ------------------------------------------------------*/
int32_t cic_init(audio_algo_t *const pAlgo)
{
  int32_t                            error         = AUDIO_ERR_MGNT_NONE;
  cic_static_config_t   const *const pStaticConfig = AudioAlgo_getStaticConfig(pAlgo);
  audio_chunk_t               *const pChunkIn      = AudioAlgo_getChunkPtrIn(pAlgo, 0U);
  audio_chunk_t               *const pChunkOut     = AudioAlgo_getChunkPtrOut(pAlgo, 0U);
  audio_buffer_t        const *const pBuffIn       = AudioChunk_getBuffInfo(pChunkIn);
  audio_buffer_t        const *const pBuffOut      = AudioChunk_getBuffInfo(pChunkOut);
  uint8_t                      const nbChannels    = AudioBuffer_getNbChannels(pBuffIn);
  uint32_t                     const fsIn          = AudioBuffer_getFs(pBuffIn);
  uint32_t                     const fsOut         = AudioBuffer_getFs(pBuffOut);
  uint8_t                      const spleSizeOut   = AudioBuffer_getSampleSize(pBuffOut);
  uint32_t                     const nbSamples     = AudioBuffer_getNbSamples(pBuffIn);
  size_t                             allocSize     = SIZEOF_ALIGN(cic_context_t);
  //  uint32_t                          lutSize        = 1UL << CIC_FRAME_SIZE; // 2^frame_size
  //  size_t                            allocLut       = s_compute_lookUpSize(CIC_FRAME_SIZE, pStaticConfig->order);
  uint32_t                           byteOffset    = allocSize;
  cic_context_t                     *pContext      = NULL;
  memPool_t                          memPool       = AUDIO_MEM_UNKNOWN;

  if (pStaticConfig == NULL)
  {
    AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "missing static conf !");
    error = AUDIO_ERR_MGNT_INIT;
  }

  if (AudioError_isOk(error))
  {
    allocSize += SIZEOF_ALIGN(cic_ch_context_t) * (uint32_t)nbChannels;                             /* cic_ch_context_t */
    allocSize += 3UL * SIZEOF_ALIGN(int32_t) * (uint32_t)pStaticConfig->order * (uint32_t)nbChannels; /* pComb; pDiff ; pSum*/
    //    allocSize += allocLut;
    memPool    = (memPool_t)pStaticConfig->ramType;
    pContext   = (cic_context_t *)AudioAlgo_malloc(allocSize, memPool);
    if (pContext == NULL)
    {
      AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "Alloc failed !");
      error = AUDIO_ERR_MGNT_ALLOCATION;
    }
  }

  if (AudioError_isOk(error))
  {
    uint8_t *pContext_u8 = (uint8_t *)pContext;

    memset(pContext, 0, allocSize);
    pContext->memPool = memPool;

    switch (pStaticConfig->order)
    {
      case 3U:
        pContext->process_cb = (spleSizeOut == 2U) ? s_CIC3_LUT8b_pcm16bit : s_CIC3_LUT8b_pcm32bit;
        break;
      case 4U:
        pContext->process_cb = (spleSizeOut == 2U) ? s_CIC4_LUT8b_pcm16bit : s_CIC4_LUT8b_pcm32bit;
        break;
      case 5U:
        pContext->process_cb = (spleSizeOut == 2U) ? s_CIC5_LUT8b_pcm16bit : s_CIC5_LUT8b_pcm32bit;
        break;
      default:
        AudioAlgo_trace(pAlgo, TRACE_LVL_ERROR, NULL, 0, "unsupported input sample format !");
        error = AUDIO_ERR_MGNT_INIT;
        break;
    }
    //    pContext->process_cb   = s_process_generic;

    pContext->pChunkIn         = pChunkIn;
    pContext->pChunkOut        = AudioAlgo_getChunkPtrOut(pAlgo, 0U);
    pContext->nbChannels       = nbChannels;
    pContext->nbPdmSamples     = nbSamples;
    pContext->nbPdmBytesPacket = nbSamples / 8UL;
    pContext->decRatio         = (fsOut == 0UL) ? 0xFFFFFFFFUL : (fsIn / fsOut);
    pContext->decRatioPacket   = pContext->decRatio / 8UL;


    //    pContext->inCntMax         = (((uint16_t)pStaticConfig->order + (2U * (uint16_t)pStaticConfig->fastsync)) * pStaticConfig->dec) + (uint16_t)pStaticConfig->order;

    /* Now process pointer addr within allocated memory */
    pContext->pCh          = (cic_ch_context_t *)(pContext_u8  + byteOffset);
    byteOffset += SIZEOF_ALIGN(cic_ch_context_t) * pContext->nbChannels; /* one cic_ch_context_t instance  per channel */

    for (uint8_t ch = 0U; ch < pContext->nbChannels; ch++)
    {
      pContext->pCh[ch].pComb = (int32_t *)(pContext_u8  + byteOffset);
      byteOffset += SIZEOF_ALIGN(int32_t) * (uint32_t)pStaticConfig->order; /* pComb */

      pContext->pCh[ch].pDiff = (int32_t *)(pContext_u8  + byteOffset);
      byteOffset += SIZEOF_ALIGN(int32_t) * (uint32_t)pStaticConfig->order; /* pDiff */

      pContext->pCh[ch].pSum = (int32_t *)(pContext_u8  + byteOffset);
      byteOffset += SIZEOF_ALIGN(int32_t) * (uint32_t)pStaticConfig->order; /* pSum */

      //      if (pStaticConfig->dec > 0U)
      //      {
      //        /* Todo : divergence with model */
      //        /* model code was pContext->pCh[ch].nextDecimCnt = pStaticConfig->order + 1; */
      //        pContext->pCh[ch].nextDecimCnt = pStaticConfig->dec;
      //      }
      //      else
      //      {
      //        pContext->pCh[ch].nextDecimCnt = 1U; /* filter bypassed so no decimation */
      //      }
    }

    ////    pContext->ppLookUp = (int32_t **)(pContext_u8  + byteOffset);
    //    byteOffset += pStaticConfig->order * sizeof(int32_t *);
    //
    //    for (uint8_t k = 0; k < pStaticConfig->order; k++)
    //    {
    //      pContext->ppLookUp[k] = (int32_t *)(pContext_u8  + byteOffset);
    //      byteOffset += lutSize * sizeof(int32_t);
    //    }

    if (byteOffset != allocSize)
    {
      error = AUDIO_ERR_MGNT_ALLOCATION;
    }
  }

  if (AudioError_isOk(error))
  {
    bool const nMsbFirst = (AudioBuffer_getType(AudioChunk_getBuffInfo(pContext->pChunkIn)) == ABUFF_FORMAT_PDM_MSB_FIRST);
    error = s_compute_lookUp(pContext, CIC_FRAME_SIZE, pStaticConfig->order, nMsbFirst);
  }

  if (AudioError_isOk(error))
  {
    AudioAlgo_setWrapperContext(pAlgo, pContext);
    error = cic_configure(pAlgo);
  }

  if (AudioError_isError(error))
  {
    cic_deinit(pAlgo);
  }

  return error;
}


int32_t cic_deinit(audio_algo_t *const pAlgo)
{
  cic_context_t *const pContext = (cic_context_t *)AudioAlgo_getWrapperContext(pAlgo);

  if (pContext != NULL)
  {
    /* disconnect context from Algo first: insure algo won't be executed during deinit */
    memPool_t const memPool = pContext->memPool;

    AudioAlgo_setWrapperContext(pAlgo, NULL);
    AudioAlgo_free(pContext, memPool);
  }

  return AUDIO_ERR_MGNT_NONE;
}


int32_t cic_configure(audio_algo_t *const pAlgo)
{
  (void)pAlgo;
  cic_context_t *const pContext = (cic_context_t *)AudioAlgo_getWrapperContext(pAlgo);

  if (pContext != NULL)
  {
    cic_dynamic_config_t const *const pDynamicConfig = AudioAlgo_getDynamicConfig(pAlgo);
    pContext->rbs = pDynamicConfig->rbs;

  }
  return AUDIO_ERR_MGNT_NONE;
}


int32_t cic_process(audio_algo_t *const pAlgo)
{
  int32_t                            error         = AUDIO_ERR_MGNT_NONE;
  cic_context_t             *const pContext      = (cic_context_t *)AudioAlgo_getWrapperContext(pAlgo);

  if (pContext->process_cb != NULL)
  {
    pContext->process_cb(pAlgo);
  }

  return error;
}

//static int32_t s_compute_lookUpSize(uint16_t frame_size, uint8_t cic_order)
//{
//  int32_t ret = 0UL;
//  if (cic_order == 0 || frame_size == 0)
//  {
//    return AUDIO_ERR_MGNT_INIT; // Error: Invalid parameters
//  }
//
//  uint32_t lut_size = 1 << frame_size; // 2^frame_size
//  ret = cic_order * SIZEOF_ALIGN(int32_t *);
//
//  for (uint8_t i = 0; i < cic_order; i++)
//  {
//    ret += lut_size * SIZEOF_ALIGN(int32_t);
//  }
//  return ret;
//}

static int32_t s_compute_lookUp(cic_context_t *const pContext, uint8_t frame_size, uint8_t cic_order, bool msb_first)
{
  int32_t  error    = AUDIO_ERR_MGNT_NONE;
  uint32_t lut_size = 1UL << frame_size; // 2^frame_size

  if ((cic_order > CIC_ORDER_MAX) || (cic_order == 0U) || (frame_size == 0U))
  {
    error = AUDIO_ERR_MGNT_INIT; // Error: Invalid parameters
  }

  if (AudioError_isOk(error))
  {
    for (uint32_t i = 0UL; i < lut_size; i++)
    {
      int32_t  acc[CIC_ORDER_MAX];
      uint32_t num = i;

      memset(acc, 0, cic_order * sizeof(int32_t));

      if (msb_first)
      {
        uint8_t bit_pos = frame_size - 1U;

        for (uint8_t j = 0U; j < frame_size; j++)
        {
          uint32_t bit    = ((num >> bit_pos) & 1UL);
          int32_t  sample = (2 * (int32_t)bit) - 1;

          acc[0] += sample;
          for (uint8_t k = 1U; k < cic_order; k++)
          {
            acc[k] += acc[k - 1U];
          }
          bit_pos--;
        }
      }
      else
      {
        for (uint8_t j = 0U; j < frame_size; j++)
        {
          uint32_t bit    = (num & 1UL);
          int32_t  sample = (2 * (int32_t)bit) - 1;

          acc[0] += sample;
          for (uint8_t k = 1U; k < cic_order; k++)
          {
            acc[k] += acc[k - 1U];
          }
          num >>= 1;
        }
      }

      /* Having one table per order index is much better for MHz; it reduces mem access time */
      //        pContext->ppLookUp[k][i] = acc[k];
      switch (cic_order)
      {
        case 5U:                            /*cstat !MISRAC2012-Rule-16.3 missing break is intentional*/
          pContext->pLookUp4[i] = acc[4];
        case 4U:                            /*cstat !MISRAC2012-Rule-16.3 missing break is intentional*/
          pContext->pLookUp3[i] = acc[3];
        case 3U:                            /*cstat !MISRAC2012-Rule-16.3 missing break is intentional*/
          pContext->pLookUp2[i] = acc[2];
        case 2U:                            /*cstat !MISRAC2012-Rule-16.3 missing break is intentional*/
          pContext->pLookUp1[i] = acc[1];
        case 1U:
          pContext->pLookUp0[i] = acc[0];
          break;
        default:
          break;
      }
    }
  }
  return error;
}


static int32_t s_CIC3_LUT8b_pcm16bit(audio_algo_t *const pAlgo)
{
  int32_t              error          = AUDIO_ERR_MGNT_NONE;
  cic_context_t *const pContext       = (cic_context_t *)AudioAlgo_getWrapperContext(pAlgo);
  uint8_t       *const pInBase        = (uint8_t *)AudioChunk_getReadPtr(pContext->pChunkIn, 0U, 0UL);
  int16_t       *const pOutBase       = (int16_t *)AudioChunk_getWritePtr(pContext->pChunkOut, 0U, 0UL);
  uint32_t       const decRatioPacket = pContext->decRatioPacket;

  for (uint8_t ch = 0U; ch < pContext->nbChannels; ch++)
  {
    cic_ch_context_t chCtxt = pContext->pCh[ch];
    uint8_t         *pInCh  = pInBase + ch;
    uint32_t         spl_in = 0UL;

    for (uint32_t spl_out = 0UL; spl_in < pContext->nbPdmBytesPacket; spl_out++)
    {
      int32_t sum0 = chCtxt.pSum[0];
      int32_t sum1 = chCtxt.pSum[1];
      int32_t sum2 = chCtxt.pSum[2];

      for (uint32_t i = 0UL; i < decRatioPacket; i++)
      {
        uint8_t inCh = *pInCh;
        int32_t acc0 = pContext->pLookUp0[inCh];
        int32_t acc1 = pContext->pLookUp1[inCh];
        int32_t acc2 = pContext->pLookUp2[inCh];

        pInCh += pContext->nbChannels;
        sum2  += acc2 + (8 * sum1) + (36 * sum0);
        sum1  += acc1 + (8 * sum0);
        sum0  += acc0;
      }
      spl_in += decRatioPacket;

      chCtxt.pSum[0] = sum0;
      chCtxt.pSum[1] = sum1;
      chCtxt.pSum[2] = sum2;

      /* Differentiator */
      int32_t diff0 = sum2 - chCtxt.pComb[0];
      chCtxt.pComb[0] = sum2;

      int32_t diff1 = diff0 - chCtxt.pComb[1];
      chCtxt.pComb[1] = diff0;

      int32_t diff2 = diff1 - chCtxt.pComb[2];
      chCtxt.pComb[2] = diff1;

      int16_t *pOut = &pOutBase[(pContext->nbChannels * spl_out) + ch];
      *pOut = (int16_t)(diff2 >> pContext->rbs);  /*cstat !MISRAC2012-Rule-10.1_R6 !MISRAC2012-Rule-1.3_n right shift of a signed value needed for sample attenuation*/
    }
  }
  return error;
}


static int32_t s_CIC4_LUT8b_pcm16bit(audio_algo_t *const pAlgo)
{
  int32_t              error          = AUDIO_ERR_MGNT_NONE;
  cic_context_t *const pContext       = (cic_context_t *)AudioAlgo_getWrapperContext(pAlgo);
  uint8_t       *const pInBase        = (uint8_t *)AudioChunk_getReadPtr(pContext->pChunkIn, 0U, 0UL);
  int16_t       *const pOutBase       = (int16_t *)AudioChunk_getWritePtr(pContext->pChunkOut, 0U, 0UL);
  uint32_t       const decRatioPacket = pContext->decRatioPacket;

  for (uint8_t ch = 0U; ch < pContext->nbChannels; ch++)
  {
    cic_ch_context_t chCtxt = pContext->pCh[ch];
    uint8_t         *pInCh  = pInBase + ch;
    uint32_t         spl_in = 0UL;

    for (uint32_t spl_out = 0UL; spl_in < pContext->nbPdmBytesPacket; spl_out++)
    {
      int32_t sum0 = chCtxt.pSum[0];
      int32_t sum1 = chCtxt.pSum[1];
      int32_t sum2 = chCtxt.pSum[2];
      int32_t sum3 = chCtxt.pSum[3];

      for (uint32_t i = 0UL; i < decRatioPacket; i++)
      {
        uint8_t inCh = *pInCh;
        int32_t acc0 = pContext->pLookUp0[inCh];//lookUpAcc1_FrameSize8Msb[inCh]; //pContext->ppLookUp[0][inCh];
        int32_t acc1 = pContext->pLookUp1[inCh];//lookUpAcc2_FrameSize8Msb[inCh]; //pContext->ppLookUp[1][inCh];
        int32_t acc2 = pContext->pLookUp2[inCh];//lookUpAcc3_FrameSize8Msb[inCh]; //pContext->ppLookUp[2][inCh];
        int32_t acc3 = pContext->pLookUp3[inCh];//lookUpAcc4_FrameSize8Msb[inCh]; //pContext->ppLookUp[3][inCh];

        pInCh += pContext->nbChannels;
        sum3  += acc3 + (8 * sum2) + (36 * sum1) + (120 * sum0);
        sum2  += acc2 + (8 * sum1) + (36 * sum0);
        sum1  += acc1 + (8 * sum0);
        sum0  += acc0;
      }
      spl_in += decRatioPacket;

      chCtxt.pSum[0] = sum0;
      chCtxt.pSum[1] = sum1;
      chCtxt.pSum[2] = sum2;
      chCtxt.pSum[3] = sum3;

      /* Differentiator */
      int32_t diff0 = sum3 - chCtxt.pComb[0];
      chCtxt.pComb[0] = sum3;

      int32_t diff1 = diff0 - chCtxt.pComb[1];
      chCtxt.pComb[1] = diff0;

      int32_t diff2 = diff1 - chCtxt.pComb[2];
      chCtxt.pComb[2] = diff1;

      int32_t diff3 = diff2 - chCtxt.pComb[3];
      chCtxt.pComb[3] = diff2;

      int16_t *pOut = &pOutBase[(pContext->nbChannels * spl_out) + ch];
      *pOut = (int16_t)(diff3 >> pContext->rbs);  /*cstat !MISRAC2012-Rule-10.1_R6 !MISRAC2012-Rule-1.3_n right shift of a signed value needed for sample attenuation*/
    }
  }
  return error;
}

static int32_t s_CIC5_LUT8b_pcm16bit(audio_algo_t *const pAlgo)
{
  int32_t              error          = AUDIO_ERR_MGNT_NONE;
  cic_context_t *const pContext       = (cic_context_t *)AudioAlgo_getWrapperContext(pAlgo);
  uint8_t       *const pInBase        = (uint8_t *)AudioChunk_getReadPtr(pContext->pChunkIn, 0U, 0UL);
  int16_t       *const pOutBase       = (int16_t *)AudioChunk_getWritePtr(pContext->pChunkOut, 0U, 0UL);
  uint32_t       const decRatioPacket = pContext->decRatioPacket;

  for (uint8_t ch = 0U; ch < pContext->nbChannels; ch++)
  {
    cic_ch_context_t chCtxt = pContext->pCh[ch];
    uint8_t         *pInCh  = pInBase + ch;
    uint32_t         spl_in = 0UL;

    for (uint32_t spl_out = 0UL; spl_in < pContext->nbPdmBytesPacket; spl_out++)
    {
      int32_t sum0 = chCtxt.pSum[0];
      int32_t sum1 = chCtxt.pSum[1];
      int32_t sum2 = chCtxt.pSum[2];
      int32_t sum3 = chCtxt.pSum[3];
      int32_t sum4 = chCtxt.pSum[4];

      for (uint32_t i = 0UL; i < decRatioPacket; i++)
      {
        uint8_t inCh = *pInCh;
        int32_t acc0 = pContext->pLookUp0[inCh];
        int32_t acc1 = pContext->pLookUp1[inCh];
        int32_t acc2 = pContext->pLookUp2[inCh];
        int32_t acc3 = pContext->pLookUp3[inCh];
        int32_t acc4 = pContext->pLookUp4[inCh];

        pInCh += pContext->nbChannels;
        sum4  += acc4 + (8 * sum3) + (36 * sum2) + (120 * sum1) + (330 * sum0);
        sum3  += acc3 + (8 * sum2) + (36 * sum1) + (120 * sum0);
        sum2  += acc2 + (8 * sum1) + (36 * sum0);
        sum1  += acc1 + (8 * sum0);
        sum0  += acc0;
      }
      spl_in += decRatioPacket;

      chCtxt.pSum[0] = sum0;
      chCtxt.pSum[1] = sum1;
      chCtxt.pSum[2] = sum2;
      chCtxt.pSum[3] = sum3;
      chCtxt.pSum[4] = sum4;

      /* Differentiator */
      int32_t diff0 = sum4 - chCtxt.pComb[0];
      chCtxt.pComb[0] = sum4;

      int32_t diff1 = diff0 - chCtxt.pComb[1];
      chCtxt.pComb[1] = diff0;

      int32_t diff2 = diff1 - chCtxt.pComb[2];
      chCtxt.pComb[2] = diff1;

      int32_t diff3 = diff2 - chCtxt.pComb[3];
      chCtxt.pComb[3] = diff2;

      int32_t diff4 = diff3 - chCtxt.pComb[4];
      chCtxt.pComb[4] = diff3;

      int16_t *pOut = &pOutBase[(pContext->nbChannels * spl_out) + ch];
      *pOut = (int16_t)(diff4 >> pContext->rbs);  /*cstat !MISRAC2012-Rule-10.1_R6 !MISRAC2012-Rule-1.3_n right shift of a signed value needed for sample attenuation*/
    }
  }
  return error;
}


static int32_t s_CIC3_LUT8b_pcm32bit(audio_algo_t *const pAlgo)
{
  int32_t              error          = AUDIO_ERR_MGNT_NONE;
  cic_context_t *const pContext       = (cic_context_t *)AudioAlgo_getWrapperContext(pAlgo);
  uint8_t       *const pInBase        = (uint8_t *)AudioChunk_getReadPtr(pContext->pChunkIn, 0U, 0UL);
  int32_t       *const pOutBase       = (int32_t *)AudioChunk_getWritePtr(pContext->pChunkOut, 0U, 0UL);
  uint32_t       const decRatioPacket = pContext->decRatioPacket;

  for (uint8_t ch = 0U; ch < pContext->nbChannels; ch++)
  {
    cic_ch_context_t chCtxt = pContext->pCh[ch];
    uint8_t         *pInCh  = pInBase + ch;
    uint32_t         spl_in = 0UL;

    for (uint32_t spl_out = 0UL; spl_in < pContext->nbPdmBytesPacket; spl_out++)
    {
      int32_t sum0 = chCtxt.pSum[0];
      int32_t sum1 = chCtxt.pSum[1];
      int32_t sum2 = chCtxt.pSum[2];

      for (uint32_t i = 0UL; i < decRatioPacket; i++)
      {
        uint8_t inCh = *pInCh;
        int32_t acc0 = pContext->pLookUp0[inCh];
        int32_t acc1 = pContext->pLookUp1[inCh];
        int32_t acc2 = pContext->pLookUp2[inCh];

        pInCh += pContext->nbChannels;
        sum2  += acc2 + (8 * sum1) + (36 * sum0);
        sum1  += acc1 + (8 * sum0);
        sum0  += acc0;
      }
      spl_in += decRatioPacket;

      chCtxt.pSum[0] = sum0;
      chCtxt.pSum[1] = sum1;
      chCtxt.pSum[2] = sum2;

      /* Differentiator */
      int32_t diff0 = sum2 - chCtxt.pComb[0];
      chCtxt.pComb[0] = sum2;

      int32_t diff1 = diff0 - chCtxt.pComb[1];
      chCtxt.pComb[1] = diff0;

      int32_t diff2 = diff1 - chCtxt.pComb[2];
      chCtxt.pComb[2] = diff1;

      int32_t *pOut = &pOutBase[(pContext->nbChannels * spl_out) + ch];
      *pOut = (int32_t)(diff2 >> pContext->rbs);  /*cstat !MISRAC2012-Rule-10.1_R6 !MISRAC2012-Rule-1.3_n right shift of a signed value needed for sample attenuation*/
    }
  }
  return error;
}


static int32_t s_CIC4_LUT8b_pcm32bit(audio_algo_t *const pAlgo)
{
  int32_t              error          = AUDIO_ERR_MGNT_NONE;
  cic_context_t *const pContext       = (cic_context_t *)AudioAlgo_getWrapperContext(pAlgo);
  uint8_t       *const pInBase        = (uint8_t *)AudioChunk_getReadPtr(pContext->pChunkIn, 0U, 0UL);
  int32_t       *const pOutBase       = (int32_t *)AudioChunk_getWritePtr(pContext->pChunkOut, 0U, 0UL);
  uint32_t       const decRatioPacket = pContext->decRatioPacket;

  for (uint8_t ch = 0U; ch < pContext->nbChannels; ch++)
  {
    cic_ch_context_t chCtxt = pContext->pCh[ch];
    uint8_t         *pInCh  = pInBase + ch;
    uint32_t         spl_in = 0UL;

    for (uint32_t spl_out = 0UL; spl_in < pContext->nbPdmBytesPacket; spl_out++)
    {
      int32_t sum0 = chCtxt.pSum[0];
      int32_t sum1 = chCtxt.pSum[1];
      int32_t sum2 = chCtxt.pSum[2];
      int32_t sum3 = chCtxt.pSum[3];

      for (uint32_t i = 0UL; i < decRatioPacket; i++)
      {
        uint8_t inCh = *pInCh;
        int32_t acc0 = pContext->pLookUp0[inCh];//lookUpAcc1_FrameSize8Msb[inCh]; //pContext->ppLookUp[0][inCh];
        int32_t acc1 = pContext->pLookUp1[inCh];//lookUpAcc2_FrameSize8Msb[inCh]; //pContext->ppLookUp[1][inCh];
        int32_t acc2 = pContext->pLookUp2[inCh];//lookUpAcc3_FrameSize8Msb[inCh]; //pContext->ppLookUp[2][inCh];
        int32_t acc3 = pContext->pLookUp3[inCh];//lookUpAcc4_FrameSize8Msb[inCh]; //pContext->ppLookUp[3][inCh];

        pInCh += pContext->nbChannels;
        sum3  += acc3 + (8 * sum2) + (36 * sum1) + (120 * sum0);
        sum2  += acc2 + (8 * sum1) + (36 * sum0);
        sum1  += acc1 + (8 * sum0);
        sum0  += acc0;
      }
      spl_in += decRatioPacket;

      chCtxt.pSum[0] = sum0;
      chCtxt.pSum[1] = sum1;
      chCtxt.pSum[2] = sum2;
      chCtxt.pSum[3] = sum3;

      /* Differentiator */
      int32_t diff0 = sum3 - chCtxt.pComb[0];
      chCtxt.pComb[0] = sum3;

      int32_t diff1 = diff0 - chCtxt.pComb[1];
      chCtxt.pComb[1] = diff0;

      int32_t diff2 = diff1 - chCtxt.pComb[2];
      chCtxt.pComb[2] = diff1;

      int32_t diff3 = diff2 - chCtxt.pComb[3];
      chCtxt.pComb[3] = diff2;

      int32_t *pOut = &pOutBase[(pContext->nbChannels * spl_out) + ch];
      *pOut = (int32_t)(diff3 >> pContext->rbs);  /*cstat !MISRAC2012-Rule-10.1_R6 !MISRAC2012-Rule-1.3_n right shift of a signed value needed for sample attenuation*/
    }
  }
  return error;
}


static int32_t s_CIC5_LUT8b_pcm32bit(audio_algo_t *const pAlgo)
{
  int32_t              error          = AUDIO_ERR_MGNT_NONE;
  cic_context_t *const pContext       = (cic_context_t *)AudioAlgo_getWrapperContext(pAlgo);
  uint8_t       *const pInBase        = (uint8_t *)AudioChunk_getReadPtr(pContext->pChunkIn, 0U, 0UL);
  int32_t       *const pOutBase       = (int32_t *)AudioChunk_getWritePtr(pContext->pChunkOut, 0U, 0UL);
  uint32_t       const decRatioPacket = pContext->decRatioPacket;

  for (uint8_t ch = 0U; ch < pContext->nbChannels; ch++)
  {
    cic_ch_context_t chCtxt = pContext->pCh[ch];
    uint8_t         *pInCh  = pInBase + ch;
    uint32_t         spl_in = 0UL;

    for (uint32_t spl_out = 0UL; spl_in < pContext->nbPdmBytesPacket; spl_out++)
    {
      int32_t sum0 = chCtxt.pSum[0];
      int32_t sum1 = chCtxt.pSum[1];
      int32_t sum2 = chCtxt.pSum[2];
      int32_t sum3 = chCtxt.pSum[3];
      int32_t sum4 = chCtxt.pSum[4];

      for (uint32_t i = 0UL; i < decRatioPacket; i++)
      {
        uint8_t inCh = *pInCh;
        int32_t acc0 = pContext->pLookUp0[inCh];
        int32_t acc1 = pContext->pLookUp1[inCh];
        int32_t acc2 = pContext->pLookUp2[inCh];
        int32_t acc3 = pContext->pLookUp3[inCh];
        int32_t acc4 = pContext->pLookUp4[inCh];

        pInCh += pContext->nbChannels;
        sum4  += acc4 + (8 * sum3) + (36 * sum2) + (120 * sum1) + (330 * sum0);
        sum3  += acc3 + (8 * sum2) + (36 * sum1) + (120 * sum0);
        sum2  += acc2 + (8 * sum1) + (36 * sum0);
        sum1  += acc1 + (8 * sum0);
        sum0  += acc0;
      }
      spl_in += decRatioPacket;

      chCtxt.pSum[0] = sum0;
      chCtxt.pSum[1] = sum1;
      chCtxt.pSum[2] = sum2;
      chCtxt.pSum[3] = sum3;
      chCtxt.pSum[4] = sum4;

      /* Differentiator */
      int32_t diff0 = sum4 - chCtxt.pComb[0];
      chCtxt.pComb[0] = sum4;

      int32_t diff1 = diff0 - chCtxt.pComb[1];
      chCtxt.pComb[1] = diff0;

      int32_t diff2 = diff1 - chCtxt.pComb[2];
      chCtxt.pComb[2] = diff1;

      int32_t diff3 = diff2 - chCtxt.pComb[3];
      chCtxt.pComb[3] = diff2;

      int32_t diff4 = diff3 - chCtxt.pComb[4];
      chCtxt.pComb[4] = diff3;

      int32_t *pOut = &pOutBase[(pContext->nbChannels * spl_out) + ch];
      *pOut = (int32_t)(diff4 >> pContext->rbs);  /*cstat !MISRAC2012-Rule-10.1_R6 !MISRAC2012-Rule-1.3_n right shift of a signed value needed for sample attenuation*/
    }
  }
  return error;
}


//
//static int32_t s_CIC_LUT8b_pcm16bit_Dec128(audio_algo_t *const pAlgo)
//{
//  int32_t                          error         = AUDIO_ERR_MGNT_NONE;
//  cic_static_config_t const *const pStaticConfig = AudioAlgo_getStaticConfig(pAlgo);
//  cic_context_t             *const pContext      = (cic_context_t *)AudioAlgo_getWrapperContext(pAlgo);
//  uint8_t                  *const pIn            = (uint8_t *)AudioChunk_getReadPtr(pContext->pChunkIn, 0U, 0UL);
//  int16_t                  *const pOutBase       = (int16_t *)AudioChunk_getWritePtr(pContext->pChunkOut, 0U, 0UL);
//  uint8_t                         cic_order      = pStaticConfig->order;
//  uint32_t k[4] =     {1, 8, 36, 120};
//
//  for (uint8_t ch = 0U; ch < pContext->nbChannels; ch++)
//  {
//    cic_ch_context_t chCtxt = pContext->pCh[ch];
//    uint32_t spl_in = 0UL;
//    for (uint32_t spl_out = 0UL; spl_in < pContext->nbPdmBytesPacket; spl_out++)
//    {
//      int32_t sums[CIC_ORDER_MAX];
//      memcpy(sums, chCtxt.pSum, cic_order * sizeof(int32_t));
//
//      for (int i = 0; i < 16; i++, spl_in++)
//      {
//        uint8_t inCh = pIn[(pContext->nbChannels * spl_in) + ch];
//        int32_t acc[CIC_ORDER_MAX];
//        for (uint8_t k = 0; k < cic_order; k++)
//        {
//          acc[k] = pContext->ppLookUp[k][inCh];
//        }
//
//        for (uint8_t k = cic_order - 1; k > 0; k--)
//        {
//          sums[k] += acc[k] + (8 * sums[k - 1]);
//        }
//        sums[0] += acc[0];
//      }
//
//      for (uint8_t k = 0; k < cic_order; k++)
//      {
//        chCtxt.pSum[k] = sums[k];
//      }
//
//      /* Differentiator */
//      int32_t diffs[CIC_ORDER_MAX];
//      diffs[0] = sums[cic_order - 1] - chCtxt.pComb[0];
//      chCtxt.pComb[0] = sums[cic_order - 1];
//
//      for (uint8_t k = 1; k < cic_order; k++)
//      {
//        diffs[k] = diffs[k - 1] - chCtxt.pComb[k];
//        chCtxt.pComb[k] = diffs[k - 1];
//      }
//
//      int16_t *pOut = &pOutBase[(pContext->nbChannels * spl_out) + ch];
//      *pOut = (int16_t)(diffs[cic_order - 1] >> pContext->rbs);  /*cstat !MISRAC2012-Rule-10.1_R6 !MISRAC2012-Rule-1.3_n right shift of a signed value needed for sample attenuation*/
//    }
//  }
//  return error;
//}



//static int32_t s_process_generic(audio_algo_t *const pAlgo)
//{
//  int32_t                            error         = AUDIO_ERR_MGNT_NONE;
//  cic_static_config_t const *const pStaticConfig = AudioAlgo_getStaticConfig(pAlgo);
//  cic_context_t             *const pContext      = (cic_context_t *)AudioAlgo_getWrapperContext(pAlgo);
//  bool                         const pdmLsbFirst   = (AudioBuffer_getType(AudioChunk_getBuffInfo(pContext->pChunkIn)) == ABUFF_FORMAT_PDM_LSB_FIRST);
//  uint8_t                     *const pIn           = (uint8_t *)AudioChunk_getReadPtr(pContext->pChunkIn,   0U, 0UL);
//  int16_t                     *const pOutBase      = (int16_t *)AudioChunk_getWritePtr(pContext->pChunkOut, 0U, 0UL);
//  uint8_t                            cntOrder      = 0U;
//  int32_t                            output        = 0;
//  int32_t                            lastIntOut    = 0;  /* Last integrator output              */
//
//  for (uint8_t ch = 0U; ch < pContext->nbChannels; ch++)
//  {
//    uint32_t spl_out          = 0UL;
//    cic_ch_context_t chCtxt = pContext->pCh[ch];
//    for (uint32_t spl_in = 0UL; spl_in < pContext->nbPdmSamples; spl_in++)
//    {
//      int16_t *pOut = &pOutBase[(pContext->nbChannels * spl_out) + ch];
//      chCtxt.inCnt++; /* increment input sample counter up to decimation factor to detect when to output a PCM sample*/
//      if (pStaticConfig->dec > 0U)
//      {
//        /* ============  Update CIC filter  ============ */
//
//        /* ============     Integration     ============ */
//        #ifdef CIC_LOOP_UNROLL
//        lastIntOut = chCtxt.pSum[3]; /* update last integrator output */
//        chCtxt.pSum[3] += chCtxt.pSum[2];
//        chCtxt.pSum[2] += chCtxt.pSum[1];
//        chCtxt.pSum[1] += chCtxt.pSum[0];
//        #else
//        lastIntOut = chCtxt.pSum[pStaticConfig->order - 1U]; /* update last integrator output */
//        /* update integration stages */
//        for (cntOrder = pStaticConfig->order - 1U; cntOrder > 0U; cntOrder--)
//        {
//          chCtxt.pSum[cntOrder] += chCtxt.pSum[cntOrder - 1U];
//        }
//        #endif
//        /* accumulate one sample from input*/
//        uint8_t tmpU8 = pIn[(pContext->nbChannels * (spl_in >> 3)) + ch]; /* will work 8 time on the same input since inputs are PDM samples inside a buffer of bytes */
//        tmpU8 = pdmLsbFirst ? ((tmpU8 >> (spl_in % 8UL)) & 1U) : ((tmpU8 >> (7UL - (spl_in % 8UL))) & 1U);
//        chCtxt.pSum[0] += (tmpU8 == 0U) ? -1 : 1;
//
//        /* ============     Decimation      ============ */
//        if (chCtxt.inCnt == chCtxt.nextDecimCnt)
//        {
//          /* ============        Comb         ============ */
//          /* update first comb stage */
//          chCtxt.pDiff[0] = lastIntOut - chCtxt.pComb[0];
//          chCtxt.pComb[0] = lastIntOut;
//          /* update remaining comb stages */
//          #ifdef CIC_LOOP_UNROLL
//          chCtxt.pDiff[1] = chCtxt.pDiff[0] - chCtxt.pComb[1];
//          chCtxt.pComb[1] = chCtxt.pDiff[0];
//
//          chCtxt.pDiff[2] = chCtxt.pDiff[1] - chCtxt.pComb[2];
//          chCtxt.pComb[2] = chCtxt.pDiff[1];
//
//          chCtxt.pDiff[3] = chCtxt.pDiff[2] - chCtxt.pComb[3];
//          chCtxt.pComb[3] = chCtxt.pDiff[2];
//          #else
//          for (cntOrder = 1U; cntOrder < pStaticConfig->order; cntOrder++)
//          {
//            chCtxt.pDiff[cntOrder] = chCtxt.pDiff[cntOrder - 1U] - chCtxt.pComb[cntOrder];
//            chCtxt.pComb[cntOrder] = chCtxt.pDiff[cntOrder - 1U];
//          }
//          #endif
//          output = chCtxt.pDiff[pStaticConfig->order - 1U];
//          *pOut = (int16_t)(output >> pContext->rbs);  /*cstat !MISRAC2012-Rule-10.1_R6 !MISRAC2012-Rule-1.3_n right shift of a signed value needed for sample attenuation*/
//
//          /* ============       FastSync      ============ */
//          if (pStaticConfig->fastsync)
//          {
//            output = chCtxt.fastSyncState[1] + chCtxt.pDiff[pStaticConfig->order - 1U];
//            *pOut  = (int16_t)(output >> pContext->rbs); /*cstat !MISRAC2012-Rule-10.1_R6 !MISRAC2012-Rule-1.3_n right shift of a signed value needed for sample attenuation*/
//            chCtxt.fastSyncState[1] = chCtxt.fastSyncState[0];
//            chCtxt.fastSyncState[0] = chCtxt.pDiff[pStaticConfig->order - 1U];
//          }
//        }
//      }
//
//      /* ============      Integrator     ============ */
//      if (chCtxt.inCnt == chCtxt.nextDecimCnt)
//      {
//        if (pStaticConfig->int_len == 0U) /* Integrator bypassed */
//        {
//          spl_out++; /* increment output vector pContext->nbSamples */
//        }
//        else
//        {
//          if ((pStaticConfig->dec == 0U) || (chCtxt.inCnt >= pContext->inCntMax)) /* Allow for CIC settling */
//          {
//            chCtxt.curIntegrator += output; /* Accumulate next sample */
//            chCtxt.integratorCnt++;
//            /* When last sample integrated */
//            if (chCtxt.integratorCnt == pStaticConfig->int_len)
//            {
//              chCtxt.integratorCnt = 0U; /* reset counter */
//              output = chCtxt.curIntegrator; /* output result */
//              *pOut = (int16_t)(output >> pContext->rbs);  /*cstat !MISRAC2012-Rule-10.1_R6 !MISRAC2012-Rule-1.3_n right shift of a signed value needed for sample attenuation*/
//              spl_out++; /* increment output vector pContext->nbSamples */
//              chCtxt.curIntegrator = 0; /* reset accumulator */
//            }
//          }
//        }
//
//        if (pStaticConfig->dec > 0U)
//        {
//          chCtxt.inCnt = 0U; /* Reset current cnt to increase up to next decimation point */
//          chCtxt.nextDecimCnt = pStaticConfig->dec; /* Set next decimation point */
//        }
//        else
//        {
//          chCtxt.inCnt = 0U; /* Reset current cnt to increase up to next decimation point */
//          chCtxt.nextDecimCnt = 1U;
//        }
//      }
//    }
//  }
//
//
//  return error;
//}
