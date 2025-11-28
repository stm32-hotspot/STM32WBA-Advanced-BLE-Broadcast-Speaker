/**
  ******************************************************************************
  * @file    common_math.h
  * @author  MCD Application Team
  * @brief   math algos
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __COMMON_MATH_H
#define __COMMON_MATH_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
/*cstat -MISRAC2012-* CMSIS not misra compliant */
#include <arm_math.h>
/*cstat +MISRAC2012-* */

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
#ifndef min
#define min(a ,b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

/* Exported functions ------------------------------------------------------- */
void smooth_single(float       *const pNew, float *const pOld, float const         lambda, uint16_t const len);
void smooth(float              *const pNew, float *const pOld, float const *const pLambda, uint16_t const len);
void smooth_single_cmplx(float *const pNew, float *const pOld, float const         lambda, uint16_t const len);
void smooth_cmplx(float        *const pNew, float *const pOld, float const *const pLambda, uint16_t const len);

void convol(float *const xInOut, int const xSz, float *const filter, int const fltSz, float *const pXConvol);

void ac_convolve(float Signal[/* SignalLen */], unsigned short const SignalLen,
                 float Kernel[/* KernelLen */], unsigned short const KernelLen,
                 float Result[/* SignalLen + KernelLen - 1 */]);

float32_t sqrt_f32(float32_t in);
float32_t tan_f32(float32_t in);
float32_t sinh_f32(float32_t in);
float32_t cosh_f32(float32_t in);
float32_t tanh_f32(float32_t in);
float32_t asinh_f32(float32_t in);
float32_t acosh_f32(float32_t in);
float32_t atanh_f32(float32_t in);


/*DBG*/
void printf_complex(float const *const pBuf, int const startIdx, int const endIdx);
void printf_long(float const *const pBuf, int const startIdx, int const endIdx);
void fprintf_complex(float const *const pBuf, int const startIdx, int const endIdx, char const *const fileName);
void fprintf_long(float const *const pBuf, int const startIdx, int const endIdx, char const *const fileName);
void division_cmplx_f32(float32_t *pA, float32_t *pB, float32_t *pC);
void vect_division_cmplx_f32(float32_t *pSrcA, float32_t *pSrcB, float32_t *pDest, uint16_t nbElements);
void exp_cmplx_f32(float32_t *pSrc, float32_t *pDest);

void multiplication_cmplx_f32(float32_t *pA, float32_t *pB, float32_t *pC);
void absolute_square_cmplx_f32(float32_t *pSrc, float32_t *pDest);


#ifdef __cplusplus
}
#endif

#endif  /* __COMMON_MATH_H */
