/**
  ******************************************************************************
  * @file    commonMath.c
  * @author  MCD Application Team
  * @brief   math functions
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
#include "common/commonMath.h"

/* Global variables ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Functions Definition ------------------------------------------------------*/

void smooth(float *const pNew, float *const pOld, float const *const pLambda, uint16_t const len)
{
  arm_sub_f32(pNew,  pOld,    pNew, (uint32_t)len);
  arm_mult_f32(pNew, pLambda, pNew, (uint32_t)len);
  arm_add_f32(pNew,  pOld,    pNew, (uint32_t)len);
}


void smooth_single(float *const pNew, float *const pOld, float const lambda, uint16_t const len)
{
  arm_sub_f32(pNew,   pOld,   pNew, (uint32_t)len);
  arm_scale_f32(pNew, lambda, pNew, (uint32_t)len);
  arm_add_f32(pNew,   pOld,   pNew, (uint32_t)len);
}


void smooth_single_cmplx(float *const pNew, float *const pOld, float const lambda, uint16_t const len)
{
  /*Sbagliata*/
  arm_sub_f32(pNew,   pOld,   pNew, (uint32_t)len * 2U);
  arm_scale_f32(pNew, lambda, pNew, (uint32_t)len * 2U);
  arm_add_f32(pNew,   pOld,   pNew, (uint32_t)len * 2U);
}


void smooth_cmplx(float *const pNew, float *const pOld, float const *const pLambda, uint16_t const len)
{
  arm_sub_f32(pNew,             pOld,    pNew, (uint32_t)len * 2U);
  arm_cmplx_mult_real_f32(pNew, pLambda, pNew, (uint32_t)len);
  arm_add_f32(pNew,             pOld,    pNew, (uint32_t)len * 2U);
}


void ac_convolve(float Signal[/* SignalLen */], unsigned short const SignalLen,
                 float Kernel[/* KernelLen */], unsigned short const KernelLen,
                 float Result[/* SignalLen + KernelLen - 1 */])
{
  for (uint16_t n = 0U; n < (SignalLen + KernelLen - 1U); n++)
  {
    unsigned short kmin, kmax, k;

    Result[n] = 0.0f;

    kmin = (n >= (KernelLen - 1U)) ? (n - (KernelLen - 1U)) : 0U;
    kmax = (n < (SignalLen - 1U)) ? n : (SignalLen - 1U);

    for (k = kmin; k <= kmax; k++)
    {
      Result[n] += Signal[k] * Kernel[n - k];
    }
  }
}


void convol(float *const xInOut, int const xSz, float *const filter, int const fltSz, float *const pXConvol)
{
  ac_convolve(xInOut, (unsigned short)xSz, filter, (unsigned short)fltSz, pXConvol);

  for (int i = 0; i < xSz; i++)
  {
    xInOut[i] = pXConvol[i + (fltSz / 2)];
  }
}


void absolute_square_cmplx_f32(float32_t *pSrc, float32_t *pDest)
{
  #ifdef USE_ARM_CMSIS
  arm_power_f32(pSrc, 2, pDest);
  #else
  pDest[0] = (pSrc[0] * pSrc[0]) + (pSrc[1] * pSrc[1]);
  #endif
}


void multiplication_cmplx_f32(float32_t *pSrcA, float32_t *pSrcB, float32_t *pDest)
{
  float32_t aReal, bReal, aIm, bIm;

  /* Reading real part of input complex matrix  */
  aReal = *pSrcA;

  /* Reading real part of output tiled error complex matrix B */
  bReal = *pSrcB;

  /* Reading imaginary part of input complex matrix  */
  aIm = *(pSrcA + 1U);

  /* Reading imaginary part of output tiled error  complex matrix  */
  bIm = *(pSrcB + 1U);

  /*process real and cmplx parts*/
  pDest[0] = (aReal * bReal) - (aIm * bIm);  /* real part*/
  pDest[1] = (aReal * bIm)   + (aIm * bReal);  /* imaginary part*/
}


void division_cmplx_f32(float32_t *pSrcA, float32_t *pSrcB, float32_t *pDest)
{
  float32_t aReal, bReal, aIm, bIm;
  float32_t den;
  /* Reading real part of input complex matrix  */
  aReal = *pSrcA;

  /* Reading real part of output tiled error complex matrix B */
  bReal = *pSrcB;

  /* Reading imaginary part of input complex matrix  */
  aIm = *(pSrcA + 1U);

  /* Reading imaginary part of output tiled error  complex matrix  */
  bIm = *(pSrcB + 1U);

  /* process denominator */
  absolute_square_cmplx_f32(pSrcB, &den);

  /*process real and cmplx parts*/
  pDest[0] = ((aReal * bReal) + (aIm   * bIm)) / den; /* real part */
  pDest[1] = ((aIm   * bReal) - (aReal * bIm)) / den; /* imaginary part */
}


void vect_division_cmplx_f32(float32_t *pSrcA, float32_t *pSrcB, float32_t *pDest, uint16_t nbElements)
{
  for (uint16_t k = 0U; k < nbElements ; k++)
  {
    division_cmplx_f32(&pSrcA[2U * k], &pSrcB[2U * k], &pDest[2U * k]);
  }
}


void exp_cmplx_f32(float32_t *pSrc, float32_t *pDest)
{
  float32_t srcReal, srcIm, r;

  srcReal  = pSrc [0];
  srcIm    = pSrc [1];
  r        = expf(srcReal);   /*cstat !MISRAC2012-Rule-22.8 no issue with expf => errno check is useless*/
  pDest[0] = r * arm_cos_f32(srcIm);
  pDest[1] = r * arm_sin_f32(srcIm);
}


float32_t sqrt_f32(float32_t in)
{
  float32_t out;
  arm_sqrt_f32(in, &out);
  return out;
}


float32_t tan_f32(float32_t in)
{
  return arm_sin_f32(in) / arm_cos_f32(in);
}


/*cstat -MISRAC2012-Rule-22.8 no need to check errno */
float32_t sinh_f32(float32_t in)
{
  float const e = expf(in);
  return 0.5f * (e - (1.0f / e));
}


float32_t cosh_f32(float32_t in)
{
  float const e = expf(in);
  return 0.5f * (e + (1.0f / e));
}


float32_t tanh_f32(float32_t in)
{
  float const e = expf(2.0f * in);
  return (e - 1.0f) / (e + 1.0f);
}


float32_t asinh_f32(float32_t in)
{
  return logf(in + sqrtf((in * in) + 1.0f));  /*cstat !MISRAC2012-Dir-4.11_a !MISRAC2012-Dir-4.11_b no check for efficiency purpose */
}


float32_t acosh_f32(float32_t in)
{
  return logf(in + sqrtf((in * in) - 1.0f));  /*cstat !MISRAC2012-Dir-4.11_a !MISRAC2012-Dir-4.11_b no check for efficiency purpose */
}


float32_t atanh_f32(float32_t in)
{
  return 0.5f * logf((1.0f + in) / (1.0f - in));  /*cstat !MISRAC2012-Dir-4.11_a*/
}
/*cstat +MISRAC2012-Rule-22.8*/


/*DEBUG*/
#if 0
void fprintf_complex(float const *const pBuf, int const startIdx, int const endIdx, char const *const fileName)
{
  //    FILE * file = fopen(fileName, "a");

  //    for (int i = startIdx; i < endIdx;i++)
  //    {
  //        if(pBuf[i].im > 0.0f)
  //            fprintf(file, "%.4f +%.4fi, ", pBuf[i].re, pBuf[i].im);
  //        else
  //            fprintf(file, "%.4f %.4fi, ", pBuf[i].re, pBuf[i].im);
  //    }
  //    fprintf(file, "\n");
  //    fclose(file);

}


void fprintf_long(float const *const pBuf, int const startIdx, int const endIdx, char const *const fileName)
{
  //    FILE * file = fopen(fileName, "a");
  //    for (int i = startIdx; i < endIdx;i++)
  //    {
  //        fprintf(file, "%.4f, ", pBuf[i]);
  //    }
  //    fprintf(file, "\n");
  //    fclose(file);
}


void printf_complex(float const *const pBuf, int const startIdx, int const endIdx)
{
  //    for (int i = startIdx; i < endIdx;i++)
  //    {
  //        if(pBuf[i].im > 0.0f)
  //            printf("%.4f +%.4fi, ", pBuf[i].re, pBuf[i].im);
  //        else
  //            printf("%.4f %.4fi, ", pBuf[i].re, pBuf[i].im);
  //    }
  //    printf("\n\r");
}


void printf_long(float const *const pBuf, int const startIdx, int const endIdx)
{
  //#ifndef EMBEDDED
  //  for (int i = startIdx; i < endIdx;i++)
  //  {
  //    printf("%.4f, ", pBuf[i]);
  //  }
  //  printf("\n\r");
  //#endif
}
#endif
