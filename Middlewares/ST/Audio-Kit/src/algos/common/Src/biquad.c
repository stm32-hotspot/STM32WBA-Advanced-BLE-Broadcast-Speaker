/**
  ******************************************************************************
  * @file    biquad.c (able to manage decimation and upsampling with zeros insertion)
  * @author  MCD Application Team
  * @brief   biquad filtering algo
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
#include <assert.h>
#include <string.h>
/*cstat -MISRAC2012-* CMSIS not misra compliant */
#include <arm_math.h>
/*cstat +MISRAC2012-* */
#include "audio_chain.h"
#include "common/biquad.h"

/* Global variables ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
//#define BIQUAD_USE_INTRINSICS
//#define BIQUAD_CODE_SIZE_OPTIM

/* Private macros ------------------------------------------------------------*/
#ifndef min
  #define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef max
  #define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/*******************/
/* design routines */
/*******************/
static int32_t s_computeQuadFromRoots(root_t const *const pRoot1, root_t const *const pRoot2, quad_t *const pQuad);
static float   s_squareDist(root_t *const pRoot1, root_t *const pRoot2);
static float   s_squareDistComplexComplex(complex_t const *const pCplx1, complex_t const *const pCplx2);
static float   s_squareDistComplexReal(complex_t const *const pCplx, float const real);
static float   s_squareDistRealReal(float const real1, float const real2);
static float   s_module2MaxBiquad(float const b0, float const b1, float const b2, float const a1, float const a2, float *const pModule2);
static int32_t s_check_biquad_stability(biquadDesignCtx_t *const pBiquadDesignCtx, float *const pModule2);
static int32_t s_round32(float const x);


/**********************/
/* filtering routines */
/**********************/
static int32_t s_biquadIntCommonInit(biquadIntContext_t                           *const pCtx, biquadInt32_t const *const pBiquad, int const nbChannels, int const decimFactor, int const upSamplingFactor, memPool_t const memPool);

#ifndef BIQUAD_CODE_SIZE_OPTIM
  static void  s_biquadInt16ProcessNoResamplingMono(biquadIntContext_t            *const pCtx, void  *const in, void  *const out, int const nbSamplesIn);
  static void  s_biquadInt16ProcessNoResamplingStereo(biquadIntContext_t          *const pCtx, void  *const in, void  *const out, int const nbSamplesIn);
  static void  s_biquadInt16ProcessNoResamplingMultiChannels(biquadIntContext_t   *const pCtx, void  *const in, void  *const out, int const nbSamplesIn);
  static void  s_biquadInt16ProcessResamplingMono(biquadIntContext_t              *const pCtx, void  *const in, void  *const out, int const nbSamplesIn);
  static void  s_biquadInt16ProcessResamplingStereo(biquadIntContext_t            *const pCtx, void  *const in, void  *const out, int const nbSamplesIn);

  static void  s_biquadInt32ProcessNoResamplingMono(biquadIntContext_t            *const pCtx, void  *const in, void  *const out, int const nbSamplesIn);
  static void  s_biquadInt32ProcessNoResamplingStereo(biquadIntContext_t          *const pCtx, void  *const in, void  *const out, int const nbSamplesIn);
  static void  s_biquadInt32ProcessNoResamplingMultiChannels(biquadIntContext_t   *const pCtx, void  *const in, void  *const out, int const nbSamplesIn);
  static void  s_biquadInt32ProcessResamplingMono(biquadIntContext_t              *const pCtx, void  *const in, void  *const out, int const nbSamplesIn);
  static void  s_biquadInt32ProcessResamplingStereo(biquadIntContext_t            *const pCtx, void  *const in, void  *const out, int const nbSamplesIn);

  static void  s_biquadFloatProcessNoResamplingMono(biquadFloatContext_t          *const pCtx, float *const in, float *const out, int const nbSamplesIn);
  static void  s_biquadFloatProcessNoResamplingStereo(biquadFloatContext_t        *const pCtx, float *const in, float *const out, int const nbSamplesIn);
  static void  s_biquadFloatProcessNoResamplingMultiChannels(biquadFloatContext_t *const pCtx, float *const in, float *const out, int const nbSamplesIn);
  static void  s_biquadFloatProcessResamplingMono(biquadFloatContext_t            *const pCtx, float *const in, float *const out, int const nbSamplesIn);
  static void  s_biquadFloatProcessResamplingStereo(biquadFloatContext_t          *const pCtx, float *const in, float *const out, int const nbSamplesIn);
#endif /* BIQUAD_CODE_SIZE_OPTIM */

static void    s_biquadInt16ProcessResamplingMultiChannels(biquadIntContext_t     *const pCtx, void  *const in, void  *const out, int const nbSamplesIn);
static void    s_biquadInt32ProcessResamplingMultiChannels(biquadIntContext_t     *const pCtx, void  *const in, void  *const out, int const nbSamplesIn);
static void    s_biquadFloatProcessResamplingMultiChannels(biquadFloatContext_t   *const pCtx, float *const in, float *const out, int const nbSamplesIn);

#ifdef BIQUAD_USE_INTRINSICS
#if defined(__ICCARM__)

#pragma inline = forced
static inline int32_t __SMMULR(int32_t const x, int32_t const y)
{
  int32_t result;
  __asm volatile("smmulr %0, %1, %2" : "=r"(result) : "r"(x), "r"(y));
  return result;
}

#pragma inline = forced
static inline int32_t __SMMLAR(int32_t const x, int32_t const y, int32_t const acc)
{
  int32_t result;
  __asm volatile("smmlar %0, %1, %2, %3" : "=r"(result) : "r"(x), "r"(y), "r"(acc));
  return result;
}

#pragma inline = forced
static inline int32_t __SMMLSR(int32_t const x, int32_t const y, int32_t const acc)
{
  int32_t result;
  __asm volatile("smmlsr %0, %1, %2, %3" : "=r"(result) : "r"(x), "r"(y), "r"(acc));
  return result;
}

#elif defined( __CC_ARM )

static inline int32_t __SMMULR(int32_t const x, int32_t const y)
{
  int32_t ret;
  __asm
  {
    smmulr ret, x, y;
  }
  return ret;
}

static inline int32_t __SMMLAR(int32_t const x, int32_t const y, int32_t const acc)
{
  int32_t ret;
  __asm
  {
    smmlar ret, x, y, a;
  }
  return ret;
}

static inline int32_t __SMMLSR(int32_t const x, int32_t const y, int32_t const acc)
{
  int32_t ret;
  __asm
  {
    smmlsr ret, x, y, a;
  }
  return ret;
}

#elif defined(__GNUC__)

static inline int32_t __SSAT(uint32_t x, uint32_t sat)
{
  int32_t val = (int32_t) x;

  assert((sat >= 1) && (sat <= 32));

  if (sat < 32)
  {
    int32_t valMax = (int32_t)((1LL << (sat - 1)) - 1LL);
    int32_t valMin = (int32_t)(-1LL << (sat - 1));

    return (val < valMin) ? valMin : (val > valMax) ? valMax : val;
  }
  else
  {
    return val;
  }
}


static inline int32_t __QADD(int32_t x, int32_t y)
{
  int64_t sum = ((int64_t) x) + ((int64_t) y);

  sum = (sum > (1LL << 31) - 1) ? (1LL << 31) - 1 : sum;
  sum = (sum < (-1LL << 31))    ? (-1LL << 31)    : sum;

  return (int32_t)sum;
}


static inline int32_t __SMMULR(int32_t x, int32_t y)
{
  return (int32_t)((((int64_t) x) * ((int64_t) y) + (1LL << 31)) >> 32);
}


static inline int32_t __SMMLAR(int32_t x, int32_t y, int32_t acc)
{
  return acc + ((int32_t)((((int64_t) x) * ((int64_t) y) + (1LL << 31)) >> 32));
}


static inline int32_t __SMMLSR(int32_t x, int32_t y, int32_t acc)
{
  return acc - ((int32_t)((((int64_t) x) * ((int64_t) y) + (1LL << 31)) >> 32));
}

#else
#error "unsupported compiler"
#endif
#endif

/* Functions Definition ------------------------------------------------------*/

void biquadDesign_computeRootsFromConfig(biquadConfig_t const *const pBiquadConfig, float const samplingRate, biquad_cell_float_t *const pBiquadCell)
{
  /*******************************************************************************

      On https://webaudio.github.io/Audio-EQ-Cookbook/audio-eq-cookbook.html
      (previously on http://www.musicdsp.org/files/Audio-EQ-Cookbook.txt) :
      ======================================================================


              Cookbook formulae for audio EQ biquad filter coefficients
      ----------------------------------------------------------------------------
              by Robert Bristow-Johnson  <rbj@audioimagination.com>


      All filter transfer functions were derived from analog prototypes (that
      are shown below for each EQ filter type) and had been digitized using the
      Bilinear Transform.  BLT frequency warping has been taken into account for
      both significant frequency relocation (this is the normal "prewarping" that
      is necessary when using the BLT) and for bandwidth readjustment (since the
      bandwidth is compressed when mapped from analog to digital using the BLT).

      First, given a biquad transfer function defined as:

                  b0 + b1*z^-1 + b2*z^-2
          H(z) = ------------------------                                  (Eq 1)
                  a0 + a1*z^-1 + a2*z^-2

      This shows 6 coefficients instead of 5 so, depending on your architecture,
      you will likely normalize a0 to be 1 and perhaps also b0 to 1 (and collect
      that into an overall gain coefficient).  Then your transfer function would
      look like:

                  (b0/a0) + (b1/a0)*z^-1 + (b2/a0)*z^-2
          H(z) = ---------------------------------------                   (Eq 2)
                  1 + (a1/a0)*z^-1 + (a2/a0)*z^-2

      or

                            1 + (b1/b0)*z^-1 + (b2/b0)*z^-2
          H(z) = (b0/a0) * ---------------------------------               (Eq 3)
                            1 + (a1/a0)*z^-1 + (a2/a0)*z^-2


      The most straight forward implementation would be the "Direct Form 1"
      (Eq 2):

          y[n] = (b0/a0)*x[n] + (b1/a0)*x[n-1] + (b2/a0)*x[n-2]
                              - (a1/a0)*y[n-1] - (a2/a0)*y[n-2]            (Eq 4)

      This is probably both the best and the easiest method to implement in the
      56K and other fixed-point or floating-point architectures with a double
      wide accumulator.


      Begin with these user defined parameters:

          Fs (the sampling frequency)

          f0 ("wherever it's happenin', man."  Center Frequency or
              Corner Frequency, or shelf midpoint frequency, depending
              on which filter type.  The "significant frequency".)

          dBgain (used only for peaking and shelving filters)

          Q (the EE kind of definition, except for peakingEQ in which A*Q is
              the classic EE Q.  That adjustment in definition was made so that
              a boost of N dB followed by a cut of N dB for identical Q and
              f0/Fs results in a precisely flat unity gain filter or "wire".)

          _or_ BW, the bandwidth in octaves (between -3 dB frequencies for BPF
              and notch or between midpoint (dBgain/2) gain frequencies for
              peaking EQ)

          _or_ S, a "shelf slope" parameter (for shelving EQ only).  When S = 1,
              the shelf slope is as steep as it can be and remain monotonically
              increasing or decreasing gain with frequency.  The shelf slope, in
              dB/octave, remains proportional to S for all other values for a
              fixed f0/Fs and dBgain.


      Then compute a few intermediate variables:

          A  = sqrt( 10^(dBgain/20) )
             =       10^(dBgain/40)     (for peaking and shelving EQ filters only)

          w0 = 2*pi*f0/Fs

          cos(w0)
          sin(w0)

          alpha = sin(w0)/(2*Q)                                       (case: Q)
                = sin(w0)*sinh( ln(2)/2 * BW * w0/sin(w0) )           (case: BW)
                = sin(w0)/2 * sqrt( (A + 1/A)*(1/S - 1) + 2 )         (case: S)

              FYI: The relationship between bandwidth and Q is
                   1/Q = 2*sinh(ln(2)/2*BW*w0/sin(w0))     (digital filter w BLT)
              or   1/Q = 2*sinh(ln(2)/2*BW)             (analog filter prototype)

              The relationship between shelf slope and Q is
                   1/Q = sqrt((A + 1/A)*(1/S - 1) + 2)

          2*sqrt(A)*alpha  =  sin(w0) * sqrt( (A^2 + 1)*(1/S - 1) + 2*A )
              is a handy intermediate variable for shelving EQ filters.


      Finally, compute the coefficients for whichever filter type you want:
      (The analog prototypes, H(s), are shown for each filter
              type for normalized frequency.)


      LPF:        H(s) = 1 / (s^2 + s/Q + 1)

                  b0 =  (1 - cos(w0))/2
                  b1 =   1 - cos(w0)
                  b2 =  (1 - cos(w0))/2
                  a0 =   1 + alpha
                  a1 =  -2*cos(w0)
                  a2 =   1 - alpha


      HPF:        H(s) = s^2 / (s^2 + s/Q + 1)

                  b0 =  (1 + cos(w0))/2
                  b1 = -(1 + cos(w0))
                  b2 =  (1 + cos(w0))/2
                  a0 =   1 + alpha
                  a1 =  -2*cos(w0)
                  a2 =   1 - alpha


      BPF:        H(s) = s / (s^2 + s/Q + 1)  (constant skirt gain, peak gain = Q)

                  b0 =   sin(w0)/2  =   Q*alpha
                  b1 =   0
                  b2 =  -sin(w0)/2  =  -Q*alpha
                  a0 =   1 + alpha
                  a1 =  -2*cos(w0)
                  a2 =   1 - alpha


      BPF:        H(s) = (s/Q) / (s^2 + s/Q + 1)      (constant 0 dB peak gain)

                  b0 =   alpha
                  b1 =   0
                  b2 =  -alpha
                  a0 =   1 + alpha
                  a1 =  -2*cos(w0)
                  a2 =   1 - alpha


      notch:      H(s) = (s^2 + 1) / (s^2 + s/Q + 1)

                  b0 =   1
                  b1 =  -2*cos(w0)
                  b2 =   1
                  a0 =   1 + alpha
                  a1 =  -2*cos(w0)
                  a2 =   1 - alpha


      APF:        H(s) = (s^2 - s/Q + 1) / (s^2 + s/Q + 1)

                  b0 =   1 - alpha
                  b1 =  -2*cos(w0)
                  b2 =   1 + alpha
                  a0 =   1 + alpha
                  a1 =  -2*cos(w0)
                  a2 =   1 - alpha


      peakingEQ:  H(s) = (s^2 + s*(A/Q) + 1) / (s^2 + s/(A*Q) + 1)

                  b0 =   1 + alpha*A
                  b1 =  -2*cos(w0)
                  b2 =   1 - alpha*A
                  a0 =   1 + alpha/A
                  a1 =  -2*cos(w0)
                  a2 =   1 - alpha/A


      lowShelf: H(s) = A * (s^2 + (sqrt(A)/Q)*s + A)/(A*s^2 + (sqrt(A)/Q)*s + 1)

                  b0 =    A*( (A+1) - (A-1)*cos(w0) + 2*sqrt(A)*alpha )
                  b1 =  2*A*( (A-1) - (A+1)*cos(w0)                   )
                  b2 =    A*( (A+1) - (A-1)*cos(w0) - 2*sqrt(A)*alpha )
                  a0 =        (A+1) + (A-1)*cos(w0) + 2*sqrt(A)*alpha
                  a1 =   -2*( (A-1) + (A+1)*cos(w0)                   )
                  a2 =        (A+1) + (A-1)*cos(w0) - 2*sqrt(A)*alpha


      highShelf: H(s) = A * (A*s^2 + (sqrt(A)/Q)*s + 1)/(s^2 + (sqrt(A)/Q)*s + A)

                  b0 =    A*( (A+1) + (A-1)*cos(w0) + 2*sqrt(A)*alpha )
                  b1 = -2*A*( (A-1) + (A+1)*cos(w0)                   )
                  b2 =    A*( (A+1) + (A-1)*cos(w0) - 2*sqrt(A)*alpha )
                  a0 =        (A+1) - (A-1)*cos(w0) + 2*sqrt(A)*alpha
                  a1 =    2*( (A-1) - (A+1)*cos(w0)                   )
                  a2 =        (A+1) - (A-1)*cos(w0) - 2*sqrt(A)*alpha


      FYI:   The bilinear transform (with compensation for frequency warping)
      substitutes:

                                      1         1 - z^-1
          (normalized)   s  <--  ----------- * ----------
                                  tan(w0/2)     1 + z^-1

      and makes use of these trig identities:

                          sin(w0)                               1 - cos(w0)
          tan(w0/2) = -------------           (tan(w0/2))^2 = -------------
                       1 + cos(w0)                             1 + cos(w0)


      resulting in these substitutions:


                      1 + cos(w0)     1 + 2*z^-1 + z^-2
          1    <--  ------------- * -------------------
                      1 + cos(w0)     1 + 2*z^-1 + z^-2


                      1 + cos(w0)     1 - z^-1
          s    <--  ------------- * ----------
                        sin(w0)       1 + z^-1

                                          1 + cos(w0)     1         -  z^-2
                                      =  ------------- * -------------------
                                            sin(w0)       1 + 2*z^-1 + z^-2


                      1 + cos(w0)     1 - 2*z^-1 + z^-2
          s^2  <--  ------------- * -------------------
                      1 - cos(w0)     1 + 2*z^-1 + z^-2


      The factor:

                          1 + cos(w0)
                      -------------------
                       1 + 2*z^-1 + z^-2

      is common to all terms in both numerator and denominator, can be factored
      out, and thus be left out in the substitutions above resulting in:


                      1 + 2*z^-1 + z^-2
          1    <--  -------------------
                         1 + cos(w0)


                      1         -  z^-2
          s    <--  -------------------
                         sin(w0)


                      1 - 2*z^-1 + z^-2
          s^2  <--  -------------------
                        1 - cos(w0)


      In addition, all terms, numerator and denominator, can be multiplied by a
      common (sin(w0))^2 factor, finally resulting in these substitutions:


          1         <--   (1 + 2*z^-1 + z^-2) * (1 - cos(w0))

          s         <--   (1         -  z^-2) * sin(w0)

          s^2       <--   (1 - 2*z^-1 + z^-2) * (1 + cos(w0))

          1 + s^2   <--   2 * (1 - 2*cos(w0)*z^-1 + z^-2)


      The biquad coefficient formulae above come out after a little
      simplification.

  *******************************************************************************/

  float const omega = 2.0f * PI * pBiquadConfig->centerCutoffFrequency / samplingRate;
  float const sn    = arm_sin_f32(omega);
  float const cs    = arm_cos_f32(omega);
  float       b0    = 1.0f;   // flat filter
  float       b1    = 0.0f;   //
  float       b2    = 0.0f;   //
  float       a0    = 1.0f;   //
  float       a1    = 0.0f;   //
  float       a2    = 0.0f;   //
  float       Q, A, alpha, t0, t1, t2, t3;


  if (pBiquadConfig->steepnessByQorBW == (uint8_t)STEEPNESS_BW)
  {
    /*
       BiQuad requires bw to be expressed in octave rather than in Hz.
       The following reasoning provides the conversion:

          rbandwidth = (w0p - w0m)/w0;
      w0p = a * w0;  w0m = 1/a * w0;  with a>1 so that w0m < w0p
      bw = w0p/w0m = a^2; (actually converted in octave)

    Given those relationships, lets find "a" given "bandwidth":

      rbandwidth = (a - 1/a)  => a^2 - rbandwidth * a - 1 = 0
      delta^2 = rbandwidth^2 + 4
      a = 0.5*(rbandwidth + sqrt(rbandwidth^2 + 4))

        The other solution is negative and therefore not suitable;
    once b is found, bw can be evaluated as follow:

      bw_octave = log2(a^2) = 2 * log2(a);
    */
    float const rbandwidth = pBiquadConfig->steepness / pBiquadConfig->centerCutoffFrequency;
    //float const bw_octave  = 2.0f / LOG2 * logf(0.5f * (rbandwidth + sqrtf(rbandwidth * rbandwidth + 4.0f)));

    //Q  = 0.5f / sinh(LOG2 / 2.0f * bw_octave * omega / sn);
    //Q  = 0.5f / sinhf(logf(0.5f * (rbandwidth + sqrtf((rbandwidth * rbandwidth) + 4.0f))) * omega / sn);
    Q = (rbandwidth * rbandwidth) + 4.0f;
    Q = (Q < 0.0f) ? 0.0f : (0.5f * (rbandwidth + sqrtf(Q)));       /*cstat !MISRAC2012-Rule-22.8 no issue with sqrtf of a positive value => errno check is useless */                // MISRAC: check sign of Q before sqrtf even if it is useless (it can't be < 0)
    Q = (Q <= 0.0f) ? 0.0f : (0.5f / sinhf(logf(Q) * omega / sn));  /*cstat !MISRAC2012-Rule-22.8 no issue with logf of a positive value nor with sinhf => errno check is useless */  // MISRAC: check strict positiveness of Q before sqrtf even if it is useless (it can't be <= 0)
  }
  else
  {
    Q = pBiquadConfig->steepness;
  }
  alpha = sn / (2.0f * Q);

  switch (pBiquadConfig->iirType)
  {
    case LOWPASS:
    case HIGHPASS:
    case BANDPASS:
    case NOTCH:
      a0 =  1.0f + alpha;
      a1 = -2.0f * cs;
      a2 =  1.0f - alpha;
      switch (pBiquadConfig->iirType)
      {
        case LOWPASS:
          /*
          b0 = (1 - cs) / 2;
          b1 = 1 - cs;
          b2 = (1 - cs) / 2;
          a0 = 1 + alpha;
          a1 = -2 * cs;
          a2 = 1 - alpha;
          */
          b1 = 1.0f - cs;
          b0 = 0.5f * b1;
          b2 = b0;
          break;

        case HIGHPASS:
          /*
          b0 = (1 + cs) / 2;
          b1 = -(1 + cs);
          b2 = (1 + cs) / 2;
          a0 = 1 + alpha;
          a1 = -2 * cs;
          a2 = 1 - alpha;
          */
          b1 = -1.0f - cs;
          b0 = -0.5f * b1;
          b2 =  b0;
          break;

        case BANDPASS:
          /*
          b0 = alpha;
          b1 = 0;
          b2 = -alpha;
          a0 = 1 + alpha;
          a1 = -2 * cs;
          a2 = 1 - alpha;
          */
          b0 =  alpha;
          b1 =  0.0f;
          b2 = -alpha;
          break;

        case NOTCH:
          /*
          b0 = 1;
          b1 = -2 * cs;
          b2 = 1;
          a0 = 1 + alpha;
          a1 = -2 * cs;
          a2 = 1 - alpha;
          */
          b0 = 1.0f;
          b1 = a1;
          b2 = 1.0f;
          break;

        default:
          break;
      }
      break;

    case PEAKING:
      A = powf(10.0f, pBiquadConfig->gain_dB / 40.0f);  /*cstat !MISRAC2012-Rule-22.8 no issue with powf(10, ...) => errno check is useless*/

      /*
      b0 = 1 + (alpha * A);
      b1 = -2 * cs;
      b2 = 1 - (alpha * A);
      a0 = 1 + (alpha / A);
      a1 = -2 * cs;
      a2 = 1 - (alpha / A);
      */
      t0 =  alpha * A;
      b0 =  1.0f  + t0;
      b1 = -2.0f  * cs;
      b2 =  1.0f  - t0;
      t0 =  alpha / A;
      a0 =  1.0f  + t0;
      a1 =  b1;
      a2 =  1.0f  - t0;
      break;

    case LOWSHELF:
    case HIGHSHELF:
      A  = powf(10.0f, pBiquadConfig->gain_dB / 40.0f); /*cstat !MISRAC2012-Rule-22.8 no issue with powf(10, ...) => errno check is useless*/
      t0 = A + 1.0f;
      t1 = A - 1.0f;
      t2 = 2.0f * ((A < 0.0f) ? 0.0f : sqrtf(A)) * alpha; /*cstat !MISRAC2012-Rule-22.8 no issue with sqrtf of a positive value => errno check is useless */ // MISRAC: check sign of A before sqrtf even if it is useless (it can't be < 0)
      switch (pBiquadConfig->iirType)
      {
        case LOWSHELF:
          /*
          b0 =      A * ((A + 1) - (A - 1) * cs + 2 * sqrt(A) * alpha)
          b1 =  2 * A * ((A - 1) - (A + 1) * cs)
          b2 =      A * ((A + 1) - (A - 1) * cs - 2 * sqrt(A) * alpha)
          a0 =           (A + 1) + (A - 1) * cs + 2 * sqrt(A) * alpha
          a1 = -2     * ((A - 1) + (A + 1) * cs)
          a2 =           (A + 1) + (A - 1) * cs - 2 * sqrt(A) * alpha
          */
          t3 = t0 - (t1 * cs);
          b0 = A * (t3 + t2);
          b1 = 2.0f * A * (t1 - (t0 * cs));
          b2 = A * (t3 - t2);
          t3 = t0 + (t1 * cs);
          a0 = t3 + t2;
          a1 = -2.0f * (t1 + (t0 * cs));
          a2 = t3 - t2;
          break;

        case HIGHSHELF:
          /*
          b0 =      A * ((A + 1) + (A - 1) * cs + 2 * sqrt(A) * alpha)
          b1 = -2 * A * ((A - 1) + (A + 1) * cs)
          b2 =      A * ((A + 1) + (A - 1) * cs - 2 * sqrt(A) * alpha)
          a0 =           (A + 1) - (A - 1) * cs + 2 * sqrt(A) * alpha
          a1 =  2     * ((A - 1) - (A + 1) * cs)
          a2 =           (A + 1) - (A - 1) * cs - 2 * sqrt(A) * alpha
          */
          t3 = t0 + (t1 * cs);
          b0 = A * (t3 + t2);
          b1 = -2.0f * A * (t1 + (t0 * cs));
          b2 = A * (t3 - t2);
          t3 = t0 - (t1 * cs);
          a0 = t3 + t2;
          a1 = 2.0f * (t1 - (t0 * cs));
          a2 = t3 - t2;
          break;

        default:
          break;
      }
      break;

    default:
      break;
  }

  pBiquadCell->num.q0 = b0;
  pBiquadCell->num.q1 = b1;
  pBiquadCell->num.q2 = b2;
  pBiquadCell->den.q0 = a0;
  pBiquadCell->den.q1 = a1;
  pBiquadCell->den.q2 = a2;
}


int32_t biquadDesign_computeQuadRoots(quad_t const *const pQuad, root_t *const pRoot)
{
  // biquad(z) = (num.q0 + num.q1*z^-1 + num.q2*z^-2) / (den.q0 + den.q1*z^-1 + den.q2*z^-2)
  //           = (num.q0*z^2 + num.q1*z + num.q2) / (den.q0*z^2 + den.q1*z + den.q2)
  //           = [gz * (z - z1) * (z - z2)] / [gp * (z - p1) * (z - p2)]
  //           = (gz*z^2 - gz*(z1+z2)*z + gz*z1*z2) / (qp*z^2 - gp*(p1+p2)*z + gp*p1*p2)
  // => num.q0=gz, num.q1=-gz*(z1+z2), num.q2=gz*z1*z2, den.q0=gp, den.q1=-qp*(p1+p2), den.q2=qp*p1*p2
  // with zeros z1 & z2 and poles p1 & p2
  //
  // pQuad is either pointer to num or den and roots are z1 & z2 or p1 & p2 respectively
  //
  // roots are solutions of equation solving:
  //   pQuad->q0*z^2 + pQuad->q1*z + pQuad->q2 = 0
  //   or a*z^2 + b*z + c = 0 with a = pQuad->q0, b = pQuad->q1, c = pQuad->q2
  //
  //   delta = b^2 - 4*a*c
  //   if delta < 0: roots are complex conjugate roots, else roots are real

  int32_t     error = AUDIO_ERR_MGNT_NONE;
  float const a     = pQuad->q0;
  float const b     = pQuad->q1;
  float const c     = pQuad->q2;
  float const delta = (b * b) - (4.0f * a * c);

  if (delta < 0.0f)
  {
    float const _2a = a + a;
    float       distanceToUnitCircle, sqrtDelta;

    arm_sqrt_f32(-delta, &sqrtDelta);
    // re = -b / _2a;
    // im = sqrtf(-delta)/ _2a;
    // distanceToUnitCircle = fabsf(sqrtf(re * re + im * im) - 1.0f);
    // distanceToUnitCircle = fabsf(sqrtf((b * b - delta) / (4.0f * a * a)) - 1.0f);
    // distanceToUnitCircle = fabsf(sqrtf((4.0f * a * c) / (4.0f * a * a)) - 1.0f);
    // distanceToUnitCircle = fabsf(sqrtf(c / a) - 1.0f);
    arm_sqrt_f32(c / a, &distanceToUnitCircle);
    distanceToUnitCircle                           = fabsf(distanceToUnitCircle - 1.0f);  /*cstat !MISRAC2012-Rule-22.8 no issue with fabsf => errno check is useless */
    pRoot[0].rootType                              = COMPLEX_ROOT;
    pRoot[1].rootType                              = COMPLEX_ROOT;
    pRoot[0].root.complexRoot.root.re              = -b / _2a;
    pRoot[0].root.complexRoot.root.im              = sqrtDelta / _2a;
    pRoot[1].root.complexRoot.root.re              =  pRoot[0].root.complexRoot.root.re;
    pRoot[1].root.complexRoot.root.im              = -pRoot[0].root.complexRoot.root.im;
    pRoot[0].root.complexRoot.distanceToUnitCircle = distanceToUnitCircle;
    pRoot[1].root.complexRoot.distanceToUnitCircle = distanceToUnitCircle;
    // a.x^2+b.x+c = gain1.gain2.(x-root1).(x-root2) => gain1 = a, gain2 = 1 is a possible solution
    pRoot[0].root.complexRoot.gain                 = a;
    pRoot[1].root.complexRoot.gain                 = 1.0f;
  }
  else if (a != 0.0f)
  {
    float const _2a = a + a;
    float sqrtDelta;

    arm_sqrt_f32(delta, &sqrtDelta);
    pRoot[0].rootType                           = REAL_ROOT;
    pRoot[1].rootType                           = REAL_ROOT;
    pRoot[0].root.realRoot.root                 = (-b + sqrtDelta) / _2a;
    pRoot[1].root.realRoot.root                 = (-b - sqrtDelta) / _2a;
    pRoot[0].root.realRoot.distanceToUnitCircle = fabsf(fabsf(pRoot[0].root.realRoot.root) - 1.0f); /*cstat !MISRAC2012-Rule-22.8 no issue with fabsf => errno check is useless */
    pRoot[1].root.realRoot.distanceToUnitCircle = fabsf(fabsf(pRoot[1].root.realRoot.root) - 1.0f); /*cstat !MISRAC2012-Rule-22.8 no issue with fabsf => errno check is useless */
    // a.x^2+b.x+c = gain1.gain2.(x-root1).(x-root2) => gain1 = a, gain2 = 1 is a possible solution
    pRoot[0].root.realRoot.gain                 = a;
    pRoot[1].root.realRoot.gain                 = 1.0f;
  }
  else if (b != 0.0f)
  {
    pRoot[0].rootType                           = REAL_ROOT;
    pRoot[1].rootType                           = NO_ROOT;
    pRoot[0].root.realRoot.root                 = -c / b;
    pRoot[0].root.realRoot.distanceToUnitCircle = fabsf(fabsf(pRoot[0].root.realRoot.root) - 1.0f); /*cstat !MISRAC2012-Rule-22.8 no issue with fabsf => errno check is useless */
    // b.x+c = gain.(x-root) => gain = b
    pRoot[0].root.realRoot.gain                 = b;
  }
  else
  {
    error = AUDIO_ERR_MGNT_ERROR;
  }

  return error;
}


// zp2sos matlab function does the same poles/zeros sort as biquadDesign_sort routine above
// but when it reassembles poles and zeros into biquad cells, it insert each new biquad cell
// at the beginning of the list, so it reverts biquad list order.
// so biquadDesign_sort with 'up' sort type is equivalent to zp2sos with 'down' sort type
// and biquadDesign_sort with 'down' sort type is equivalent to zp2sos with 'up' sort type
int32_t biquadDesign_sort(IIR_filter_float_t *const pIirFilterFloat, biquadSort_t const biquadSort)
{
  int32_t error = AUDIO_ERR_MGNT_NONE;

  pIirFilterFloat->nbPoles        = 0;
  pIirFilterFloat->nbComplexPoles = 0;
  pIirFilterFloat->nbRealPoles    = 0;
  pIirFilterFloat->nbZeros        = 0;
  pIirFilterFloat->nbComplexZeros = 0;
  pIirFilterFloat->nbRealZeros    = 0;

  if (biquadSort == BIQUAD_NO_SORT)
  {
    for (int i = 0; i < pIirFilterFloat->nbCells; i++)
    {
      switch (pIirFilterFloat->biquadCells[i].poles[0].rootType)
      {
        case COMPLEX_ROOT:
          pIirFilterFloat->pPoles[pIirFilterFloat->nbPoles++] = &pIirFilterFloat->biquadCells[i].poles[0];
          pIirFilterFloat->nbComplexPoles++;
          break;
        case REAL_ROOT:
          pIirFilterFloat->pPoles[pIirFilterFloat->nbPoles++] = &pIirFilterFloat->biquadCells[i].poles[0];
          pIirFilterFloat->nbRealPoles++;
          break;
        default:
          break;
      }
      switch (pIirFilterFloat->biquadCells[i].poles[1].rootType)
      {
        case COMPLEX_ROOT:
          pIirFilterFloat->pPoles[pIirFilterFloat->nbPoles++] = &pIirFilterFloat->biquadCells[i].poles[1];
          pIirFilterFloat->nbComplexPoles++;
          break;
        case REAL_ROOT:
          pIirFilterFloat->pPoles[pIirFilterFloat->nbPoles++] = &pIirFilterFloat->biquadCells[i].poles[1];
          pIirFilterFloat->nbRealPoles++;
          break;
        default:
          break;
      }
      switch (pIirFilterFloat->biquadCells[i].zeros[0].rootType)
      {
        case COMPLEX_ROOT:
          pIirFilterFloat->pZeros[pIirFilterFloat->nbZeros++] = &pIirFilterFloat->biquadCells[i].zeros[0];
          pIirFilterFloat->nbComplexZeros++;
          break;
        case REAL_ROOT:
          pIirFilterFloat->pZeros[pIirFilterFloat->nbZeros++] = &pIirFilterFloat->biquadCells[i].zeros[0];
          pIirFilterFloat->nbRealZeros++;
          break;
        default:
          break;
      }
      switch (pIirFilterFloat->biquadCells[i].zeros[1].rootType)
      {
        case COMPLEX_ROOT:
          pIirFilterFloat->pZeros[pIirFilterFloat->nbZeros++] = &pIirFilterFloat->biquadCells[i].zeros[1];
          pIirFilterFloat->nbComplexZeros++;
          break;
        case REAL_ROOT:
          pIirFilterFloat->pZeros[pIirFilterFloat->nbZeros++] = &pIirFilterFloat->biquadCells[i].zeros[1];
          pIirFilterFloat->nbRealZeros++;
          break;
        default:
          break;
      }
    }
  }
  else
  {
    float   dist1, dist2;
    root_t *pRootSwap;

    // complex poles first
    for (int i = 0; i < pIirFilterFloat->nbCells; i++)
    {
      if (pIirFilterFloat->biquadCells[i].poles[0].rootType == COMPLEX_ROOT)
      {
        pIirFilterFloat->pPoles[pIirFilterFloat->nbPoles++] = &pIirFilterFloat->biquadCells[i].poles[0];
        pIirFilterFloat->pPoles[pIirFilterFloat->nbPoles++] = &pIirFilterFloat->biquadCells[i].poles[1]; // when poles[0] is COMPLEX_ROOT, poles[1] is COMPLEX_ROOT too
        pIirFilterFloat->nbComplexPoles += 2;
      }
    }
    // real poles second
    for (int i = 0; i < pIirFilterFloat->nbCells; i++)
    {
      if (pIirFilterFloat->biquadCells[i].poles[0].rootType == REAL_ROOT)
      {
        pIirFilterFloat->pPoles[pIirFilterFloat->nbPoles++] = &pIirFilterFloat->biquadCells[i].poles[0];
        pIirFilterFloat->pPoles[pIirFilterFloat->nbPoles++] = &pIirFilterFloat->biquadCells[i].poles[1]; // when poles[0] is REAL_ROOT, poles[1] is REAL_ROOT too
        pIirFilterFloat->nbRealPoles += 2;
      }
    }
    // complex zeros first
    for (int i = 0; i < pIirFilterFloat->nbCells; i++)
    {
      if (pIirFilterFloat->biquadCells[i].zeros[0].rootType == COMPLEX_ROOT)
      {
        pIirFilterFloat->pZeros[pIirFilterFloat->nbZeros++] = &pIirFilterFloat->biquadCells[i].zeros[0];
        pIirFilterFloat->pZeros[pIirFilterFloat->nbZeros++] = &pIirFilterFloat->biquadCells[i].zeros[1]; // when zeros[0] is COMPLEX_ROOT, zeros[1] is COMPLEX_ROOT too
        pIirFilterFloat->nbComplexZeros += 2;
      }
    }
    // real zeros second
    for (int i = 0; i < pIirFilterFloat->nbCells; i++)
    {
      if (pIirFilterFloat->biquadCells[i].zeros[0].rootType == REAL_ROOT)
      {
        pIirFilterFloat->pZeros[pIirFilterFloat->nbZeros++] = &pIirFilterFloat->biquadCells[i].zeros[0];
        pIirFilterFloat->pZeros[pIirFilterFloat->nbZeros++] = &pIirFilterFloat->biquadCells[i].zeros[1]; // when zeros[0] is REAL_ROOT, zeros[1] is REAL_ROOT too
        pIirFilterFloat->nbRealZeros += 2;
      }
    }

    // sort list of complex poles (2 by 2 because in case of complex roots, 2 successive roots must be kept together: complex & complex conjugate)
    for (int i = 0; i < (pIirFilterFloat->nbComplexPoles - 2); i += 2)
    {
      dist1 = pIirFilterFloat->pPoles[i]->root.complexRoot.distanceToUnitCircle;
      for (int j = i + 2; j < (pIirFilterFloat->nbComplexPoles); j += 2)
      {
        dist2 = pIirFilterFloat->pPoles[j]->root.complexRoot.distanceToUnitCircle;
        if (((biquadSort == BIQUAD_DOWN_SORT) && (dist2 < dist1)) ||
            ((biquadSort == BIQUAD_UP_SORT)   && (dist2 > dist1)))
        {
          pRootSwap                      = pIirFilterFloat->pPoles[i + 0];
          pIirFilterFloat->pPoles[i + 0] = pIirFilterFloat->pPoles[j + 0];
          pIirFilterFloat->pPoles[j + 0] = pRootSwap;
          pRootSwap                      = pIirFilterFloat->pPoles[i + 1];
          pIirFilterFloat->pPoles[i + 1] = pIirFilterFloat->pPoles[j + 1];
          pIirFilterFloat->pPoles[j + 1] = pRootSwap;
          dist1                          = dist2;
        }
      }
    }

    // sort list of real poles
    for (int i = pIirFilterFloat->nbComplexPoles; i < (pIirFilterFloat->nbPoles - 1); i++)
    {
      dist1 = pIirFilterFloat->pPoles[i]->root.realRoot.distanceToUnitCircle;
      for (int j = i + 1; j < (pIirFilterFloat->nbComplexPoles + pIirFilterFloat->nbRealPoles); j++)
      {
        dist2 = pIirFilterFloat->pPoles[j]->root.realRoot.distanceToUnitCircle;
        if (((biquadSort == BIQUAD_DOWN_SORT) && (dist2 < dist1)) ||
            ((biquadSort == BIQUAD_UP_SORT)   && (dist2 > dist1)))
        {
          pRootSwap                  = pIirFilterFloat->pPoles[i];
          pIirFilterFloat->pPoles[i] = pIirFilterFloat->pPoles[j];
          pIirFilterFloat->pPoles[j] = pRootSwap;
          dist1                      = dist2;
        }
      }
    }

    // select zeros closest to poles (beginning with complex poles, then real poles)
    for (int i = 0; i < pIirFilterFloat->nbComplexPoles; i += 2)
    {
      int bestj = -1;
      dist1 = 0.0f; // MISRAC: may be uninitialized
      for (int j = i; j < pIirFilterFloat->nbZeros; j += ((pIirFilterFloat->pZeros[j]->rootType == COMPLEX_ROOT) ? 2 : 1))
      {
        dist2 = s_squareDist(pIirFilterFloat->pPoles[i], pIirFilterFloat->pZeros[j]);
        if ((bestj == -1) || (dist2 < dist1))
        {
          dist1 = dist2;
          bestj = j;
        }
      }
      if (bestj != i)
      {
        if (pIirFilterFloat->pZeros[bestj]->rootType == COMPLEX_ROOT)
        {
          // swap 2 complex & complex conjugate roots with 2 complex & complex conjugate roots
          pRootSwap                          = pIirFilterFloat->pZeros[i     + 0];
          pIirFilterFloat->pZeros[i     + 0] = pIirFilterFloat->pZeros[bestj + 0];
          pIirFilterFloat->pZeros[bestj + 0] = pRootSwap;
          pRootSwap                          = pIirFilterFloat->pZeros[i     + 1];
          pIirFilterFloat->pZeros[i     + 1] = pIirFilterFloat->pZeros[bestj + 1];
          pIirFilterFloat->pZeros[bestj + 1] = pRootSwap;
        }
        else
        {
          // swap 2 complex & complex conjugate roots with 2 real roots => search for the second real root
          pRootSwap                      = pIirFilterFloat->pZeros[i    ];
          pIirFilterFloat->pZeros[i]     = pIirFilterFloat->pZeros[bestj];
          pIirFilterFloat->pZeros[bestj] = pRootSwap;
          if (pIirFilterFloat->pZeros[i + 1]->rootType == COMPLEX_ROOT)
          {
            // if next root is complex, swap it too with root associated to bestj (bestj^1 instead of bestj+1 in case bestj is odd)
            unsigned int const swapIdx = (unsigned int)bestj ^ 1U;

            pRootSwap                        = pIirFilterFloat->pZeros[i   + 1];
            pIirFilterFloat->pZeros[i   + 1] = pIirFilterFloat->pZeros[swapIdx];
            pIirFilterFloat->pZeros[swapIdx] = pRootSwap;
          }
          bestj = -1;
          dist1 = 0.0f; // MISRAC: may be uninitialized
          for (int j = i + 1; j < pIirFilterFloat->nbZeros; j++)
          {
            if (pIirFilterFloat->pZeros[j]->rootType == REAL_ROOT)
            {
              dist2 = s_squareDist(pIirFilterFloat->pPoles[i], pIirFilterFloat->pZeros[j]);
              if ((bestj == -1) || (dist2 < dist1))
              {
                dist1 = dist2;
                bestj = j;
              }
            }
          }
          if (bestj != (i + 1))
          {
            pRootSwap                      = pIirFilterFloat->pZeros[i + 1];
            pIirFilterFloat->pZeros[i + 1] = pIirFilterFloat->pZeros[bestj];
            pIirFilterFloat->pZeros[bestj] = pRootSwap;
          }
        }
      }
    }
    // real poles
    for (int i = pIirFilterFloat->nbComplexPoles; i < pIirFilterFloat->nbPoles; i += ((pIirFilterFloat->pZeros[i]->rootType == COMPLEX_ROOT) ? 2 : 1))
    {
      int bestj = -1;
      dist1 = 0.0f; // MISRAC: may be uninitialized
      for (int j = i; j < pIirFilterFloat->nbZeros; j += ((pIirFilterFloat->pZeros[j]->rootType == COMPLEX_ROOT) ? 2 : 1))
      {
        dist2 = s_squareDist(pIirFilterFloat->pPoles[i], pIirFilterFloat->pZeros[j]);
        if ((bestj == -1) || (dist2 < dist1))
        {
          if ((((unsigned int)i & 1U) == 0U) || (pIirFilterFloat->pZeros[j]->rootType == REAL_ROOT))
          {
            // do not choose complex zero root if i is odd, else complex zero root and its conjugate would be split in 2 different cells
            dist1 = dist2;
            bestj = j;
          }
        }
      }
      if (bestj != i)
      {
        if (pIirFilterFloat->pZeros[bestj]->rootType == COMPLEX_ROOT)
        {
          // swap 2 real roots with 2 complex & complex conjugate roots
          unsigned int const swapIdx = (unsigned int)bestj ^ 1U;  // bestj^1 instead of bestj+1 in case bestj is odd

          pRootSwap                          = pIirFilterFloat->pZeros[i     + 0];
          pIirFilterFloat->pZeros[i     + 0] = pIirFilterFloat->pZeros[bestj + 0];
          pIirFilterFloat->pZeros[bestj + 0] = pRootSwap;
          pRootSwap                          = pIirFilterFloat->pZeros[i     + 1];
          pIirFilterFloat->pZeros[i     + 1] = pIirFilterFloat->pZeros[swapIdx];
          pIirFilterFloat->pZeros[swapIdx]   = pRootSwap;
        }
        else
        {
          // swap 1 real conjugate root with 1 real root
          pRootSwap                      = pIirFilterFloat->pZeros[i];
          pIirFilterFloat->pZeros[i]     = pIirFilterFloat->pZeros[bestj];
          pIirFilterFloat->pZeros[bestj] = pRootSwap;
        }
      }
    }
  }

  // recomputes biquad coefficients from roots
  for (int i = 0; AudioError_isOk(error) && (i < pIirFilterFloat->nbCells); i++)
  {
    biquad_cell_float_t *const pBiquadCell = &pIirFilterFloat->biquadCells[i];

    error = s_computeQuadFromRoots(pIirFilterFloat->pZeros[2 * i], pIirFilterFloat->pZeros[(2 * i) + 1], &pBiquadCell->num);
    if (AudioError_isOk(error))
    {
      error = s_computeQuadFromRoots(pIirFilterFloat->pPoles[2 * i], pIirFilterFloat->pPoles[(2 * i) + 1], &pBiquadCell->den);
    }
  }

  return error;
}


float biquadDesign_normalizeBiquadCells(IIR_filter_float_t *const pIirFilterFloat, float *const pModule2, biquadNormalize_t const normalize)
{
  float globalGain = 1.0f;

  for (int i = 0; i < (IIR_RESPONSE_ACCURACY / 2); i++)
  {
    pModule2[i] = 1.0f;
  }

  for (int i = 0; i < pIirFilterFloat->nbCells; i++)
  {
    biquad_cell_float_t *const pBiquadCell = &pIirFilterFloat->biquadCells[i];
    float                const norm        = 1.0f / pBiquadCell->den.q0;
    float                const a1          = pBiquadCell->den.q1 * norm;
    float                const a2          = pBiquadCell->den.q2 * norm;
    float                const b0          = pBiquadCell->num.q0 * norm;
    float                const b1          = pBiquadCell->num.q1 * norm;
    float                const b2          = pBiquadCell->num.q2 * norm;
    float                const module2Max  = s_module2MaxBiquad(b0, b1, b2, a1, a2, pModule2);
    float                      normCoef    = 1.0f;
    float                      gain        = 1.0f;

    if ((normalize == BIQUAD_COMPUTE_MODULE_AND_NORMALIZE) && (module2Max > 1.0f))
    {
      arm_scale_f32(pModule2, 1.0f / module2Max, pModule2, IIR_RESPONSE_ACCURACY / 2);
      arm_sqrt_f32(module2Max, &gain);
      globalGain *= gain;
      normCoef    = 1.0f / gain;
    }

    pBiquadCell->den.q0 = 1.0f;
    pBiquadCell->den.q1 = a1;
    pBiquadCell->den.q2 = a2;
    pBiquadCell->num.q0 = b0 * normCoef;
    pBiquadCell->num.q1 = b1 * normCoef;
    pBiquadCell->num.q2 = b2 * normCoef;
  }

  return globalGain;
}


int32_t biquadDesign_quantifyBiquadFilter(IIR_filter_float_t *const pIirFilterFloat, float const gain0, biquadDesignCtx_t *const pBiquadDesignCtx, float *const pModule2)
{
  int32_t error = AUDIO_ERR_MGNT_NONE;

  switch (pBiquadDesignCtx->sampleType)
  {
    case ABUFF_FORMAT_FIXED16:
    case ABUFF_FORMAT_FIXED32:
      if (pIirFilterFloat->nbCells == 0)
      {
        pBiquadDesignCtx->biquadCoef.pBiquadIntCoef->pBiquadCell = NULL;
        pBiquadDesignCtx->biquadCoef.pBiquadIntCoef->gainMant    = 0x7FFFFFFF;
        pBiquadDesignCtx->biquadCoef.pBiquadIntCoef->gainExp     = 0;
        pBiquadDesignCtx->biquadCoef.pBiquadIntCoef->nbCells     = 0;
      }
      else
      {
        biquadCellInt32_t *pBiquadCell = (biquadCellInt32_t *)AudioAlgo_malloc((size_t)pIirFilterFloat->nbCells * sizeof(biquadCellInt32_t), pBiquadDesignCtx->memPool);

        if (pBiquadCell == NULL)
        {
          error = AUDIO_ERR_MGNT_ALLOCATION;
        }
        else
        {
          float gain = gain0;
          int   shift;

          for (int i = 0; i < pIirFilterFloat->nbCells; i++)
          {
            float const norm = 1.0f / pIirFilterFloat->biquadCells[i].den.q0;
            float const b0   = pIirFilterFloat->biquadCells[i].num.q0 * norm;
            float const b1   = pIirFilterFloat->biquadCells[i].num.q1 * norm;
            float const b2   = pIirFilterFloat->biquadCells[i].num.q2 * norm;
            float const a1   = pIirFilterFloat->biquadCells[i].den.q1 * norm;
            float const a2   = pIirFilterFloat->biquadCells[i].den.q2 * norm;
            float       bMin = 0.0f;
            float       bMax = 0.0f;
            float       coef = 1.0f;

            bMin  = min(bMin, b0);
            bMin  = min(bMin, b1);
            bMin  = min(bMin, b2);
            bMax  = max(bMax, b0);
            bMax  = max(bMax, b1);
            bMax  = max(bMax, b2);
            bMin *= 2147483648.0f;
            bMax *= 2147483648.0f;
            if (bMin < -2147483648.0f)
            {
              coef = -2147483648.0f / bMin;
            }
            if (bMax > 2147483647.0f)
            {
              coef = min(coef, 2147483647.0f / bMax);
            }
            gain /= coef;
            coef *= 2147483648.0f;
            pBiquadCell[i].b0 = s_round32(b0 * coef);
            pBiquadCell[i].b1 = s_round32(b1 * coef);
            pBiquadCell[i].b2 = s_round32(b2 * coef);
            pBiquadCell[i].a1 = s_round32(a1 * 1073741824.0f);  // a1 / 2
            pBiquadCell[i].a2 = s_round32(a2 * 2147483648.0f);
          }
          gain *= 2147483648.0f;
          for (shift = 0; gain > 2147483647.0f; shift++)
          {
            gain *= 0.5f;
          }
          pBiquadDesignCtx->biquadCoef.pBiquadIntCoef->pBiquadCell = (biquadCellInt32_t const *)pBiquadCell;  // add const qualifier
          pBiquadDesignCtx->biquadCoef.pBiquadIntCoef->gainMant    = s_round32(gain);
          pBiquadDesignCtx->biquadCoef.pBiquadIntCoef->gainExp     = (uint8_t)shift;
          pBiquadDesignCtx->biquadCoef.pBiquadIntCoef->nbCells     = (uint8_t)pIirFilterFloat->nbCells;
          error                                                    = s_check_biquad_stability(pBiquadDesignCtx, pModule2);
        }
      }
      break;

    case ABUFF_FORMAT_FLOAT:
      if (pIirFilterFloat->nbCells == 0)
      {
        pBiquadDesignCtx->biquadCoef.pBiquadFloatCoef->pBiquadCell = NULL;
        pBiquadDesignCtx->biquadCoef.pBiquadFloatCoef->nbCells     = 0;
      }
      else
      {
        biquadCellFloat_t *pBiquadCell = (biquadCellFloat_t *)AudioAlgo_malloc((size_t)pIirFilterFloat->nbCells * sizeof(biquadCellFloat_t), pBiquadDesignCtx->memPool);

        if (pBiquadCell == NULL)
        {
          error = AUDIO_ERR_MGNT_ALLOCATION;
        }
        else
        {
          for (int i = 0; i < pIirFilterFloat->nbCells; i++)
          {
            float const norm = 1.0f / pIirFilterFloat->biquadCells[i].den.q0;

            pBiquadCell[i].b0 = pIirFilterFloat->biquadCells[i].num.q0 * norm;
            pBiquadCell[i].b1 = pIirFilterFloat->biquadCells[i].num.q1 * norm;
            pBiquadCell[i].b2 = pIirFilterFloat->biquadCells[i].num.q2 * norm;
            pBiquadCell[i].a1 = pIirFilterFloat->biquadCells[i].den.q1 * norm;
            pBiquadCell[i].a2 = pIirFilterFloat->biquadCells[i].den.q2 * norm;
          }
          if (pIirFilterFloat->nbCells > 0)
          {
            // apply gain0 to last biquad cell
            pBiquadCell[pIirFilterFloat->nbCells - 1].b0 *= gain0;
            pBiquadCell[pIirFilterFloat->nbCells - 1].b1 *= gain0;
            pBiquadCell[pIirFilterFloat->nbCells - 1].b2 *= gain0;
          }
          pBiquadDesignCtx->biquadCoef.pBiquadFloatCoef->pBiquadCell = (biquadCellFloat_t const *)pBiquadCell;  // add const qualifier
          pBiquadDesignCtx->biquadCoef.pBiquadFloatCoef->nbCells     = (uint8_t)pIirFilterFloat->nbCells;
          error                                                      = s_check_biquad_stability(pBiquadDesignCtx, pModule2);
        }
      }
      break;

    default:
      error = AUDIO_ERR_MGNT_CONFIG;
      break;
  }

  return error;
}


void biquadDesign_freeBiquadFilter(biquadDesignCtx_t *const pBiquadDesignCtx)
{
  switch (pBiquadDesignCtx->sampleType)
  {
    case ABUFF_FORMAT_FIXED16:
    case ABUFF_FORMAT_FIXED32:
      if (pBiquadDesignCtx->biquadCoef.pBiquadIntCoef->pBiquadCell != NULL)
      {
        AudioAlgo_free((void *)pBiquadDesignCtx->biquadCoef.pBiquadIntCoef->pBiquadCell, pBiquadDesignCtx->memPool); // remove const qualifier
      }
      break;

    case ABUFF_FORMAT_FLOAT:
      if (pBiquadDesignCtx->biquadCoef.pBiquadFloatCoef->pBiquadCell != NULL)
      {
        AudioAlgo_free((void *)pBiquadDesignCtx->biquadCoef.pBiquadFloatCoef->pBiquadCell, pBiquadDesignCtx->memPool); // remove const qualifier
      }
      break;

    default:
      break;
  }
}


/**
  * @brief  initialize biquad (16 bits fixed-point version)
  * @param  pCtx              biquad context pointer
  * @param  pBiquad           biquad filter coefficients pointer
  * @param  nbChannels        number of channels
  * @param  decimFactor       decimation factor
  * @param  upSamplingFactor  upsampling factor
  * @param  memPool           memory pool used for allocation
  * @retval Error; AUDIO_ERR_MGNT_NONE if no issue
  */
int32_t biquadInt16Init(biquadIntContext_t *const pCtx, biquadInt32_t const *const pBiquad, int const nbChannels, int const decimFactor, int const upSamplingFactor, memPool_t const memPool)
{
  int32_t error = AUDIO_ERR_MGNT_NONE;

  if (pBiquad == NULL)
  {
    error = AUDIO_ERR_MGNT_INIT;
  }
  if (AudioError_isOk(error))
  {
    error = s_biquadIntCommonInit(pCtx, pBiquad, nbChannels, decimFactor, upSamplingFactor, memPool);
  }

  if (AudioError_isOk(error))
  {
    #ifdef BIQUAD_CODE_SIZE_OPTIM
    pCtx->pBiquadProcess = s_biquadInt16ProcessResamplingMultiChannels;
    #else /* BIQUAD_CODE_SIZE_OPTIM */
    if ((decimFactor == 1) && (upSamplingFactor == 1))
    {
      switch (nbChannels)
      {
        case 1:
          pCtx->pBiquadProcess = s_biquadInt16ProcessNoResamplingMono;
          break;
        case 2:
          pCtx->pBiquadProcess = s_biquadInt16ProcessNoResamplingStereo;
          break;
        default:
          pCtx->pBiquadProcess = s_biquadInt16ProcessNoResamplingMultiChannels;
          break;
      }
    }
    else
    {
      switch (nbChannels)
      {
        case 1:
          pCtx->pBiquadProcess = s_biquadInt16ProcessResamplingMono;
          break;
        case 2:
          pCtx->pBiquadProcess = s_biquadInt16ProcessResamplingStereo;
          break;
        default:
          pCtx->pBiquadProcess = s_biquadInt16ProcessResamplingMultiChannels;
          break;
      }
    }
    #endif /* BIQUAD_CODE_SIZE_OPTIM */
  }

  return error;
}


/**
  * @brief  initialize biquad (32 bits fixed-point version)
  * @param  pCtx              biquad context pointer
  * @param  pBiquad           biquad filter coefficients pointer
  * @param  nbChannels        number of channels
  * @param  decimFactor       decimation factor
  * @param  upSamplingFactor  upsampling factor
  * @param  memPool           memory pool used for allocation
  * @retval Error; AUDIO_ERR_MGNT_NONE if no issue
  */
int32_t biquadInt32Init(biquadIntContext_t *const pCtx, biquadInt32_t const *const pBiquad, int const nbChannels, int const decimFactor, int const upSamplingFactor, memPool_t const memPool)
{
  int32_t error = s_biquadIntCommonInit(pCtx, pBiquad, nbChannels, decimFactor, upSamplingFactor, memPool);

  if (AudioError_isOk(error))
  {
    #ifdef BIQUAD_CODE_SIZE_OPTIM
    pCtx->pBiquadProcess = s_biquadInt32ProcessResamplingMultiChannels;
    #else /* BIQUAD_CODE_SIZE_OPTIM */
    if ((decimFactor == 1) && (upSamplingFactor == 1))
    {
      switch (nbChannels)
      {
        case 1:
          pCtx->pBiquadProcess = s_biquadInt32ProcessNoResamplingMono;
          break;
        case 2:
          pCtx->pBiquadProcess = s_biquadInt32ProcessNoResamplingStereo;
          break;
        default:
          pCtx->pBiquadProcess = s_biquadInt32ProcessNoResamplingMultiChannels;
          break;
      }
    }
    else
    {
      switch (nbChannels)
      {
        case 1:
          pCtx->pBiquadProcess = s_biquadInt32ProcessResamplingMono;
          break;
        case 2:
          pCtx->pBiquadProcess = s_biquadInt32ProcessResamplingStereo;
          break;
        default:
          pCtx->pBiquadProcess = s_biquadInt32ProcessResamplingMultiChannels;
          break;
      }
    }
    #endif /* BIQUAD_CODE_SIZE_OPTIM */
  }

  return error;
}


/**
  * @brief  initialize biquad (floating-point version)
  * @param  pCtx              biquad context pointer
  * @param  pBiquad           biquad filter coefficients pointer
  * @param  nbChannels        number of channels
  * @param  decimFactor       decimation factor
  * @param  upSamplingFactor  upsampling factor
  * @param  memPool           memory pool used for allocation
  * @retval Error; AUDIO_ERR_MGNT_NONE if no issue
  */
int32_t biquadFloatInit(biquadFloatContext_t *const pCtx, biquadFloat_t const *const pBiquad, int const nbChannels, int const decimFactor, int const upSamplingFactor, memPool_t const memPool)
{
  int32_t   error = AUDIO_ERR_MGNT_NONE;
  int const size  = ((2 * (int)pBiquad->nbCells) + 2) * nbChannels;
  float    *pBiquadMem;

  pBiquadMem = (float *)AudioAlgo_malloc((size_t)size * sizeof(float), memPool);
  if (pBiquadMem == NULL)
  {
    error = AUDIO_ERR_MGNT_ALLOCATION;
  }
  else
  {
    memset(pBiquadMem, 0, (size_t)size * sizeof(float));

    pCtx->memPool          = memPool;
    pCtx->pBiquad          = pBiquad;
    pCtx->nbChannels       = nbChannels;
    pCtx->decimFactor      = decimFactor;
    pCtx->upSamplingFactor = upSamplingFactor;
    pCtx->pBiquadMem       = pBiquadMem;
    #ifdef BIQUAD_CODE_SIZE_OPTIM
    pCtx->pBiquadProcess = s_biquadFloatProcessResamplingMultiChannels;
    #else /* BIQUAD_CODE_SIZE_OPTIM */
    if ((decimFactor == 1) && (upSamplingFactor == 1))
    {
      switch (nbChannels)
      {
        case 1:
          pCtx->pBiquadProcess = s_biquadFloatProcessNoResamplingMono;
          break;
        case 2:
          pCtx->pBiquadProcess = s_biquadFloatProcessNoResamplingStereo;
          break;
        default:
          pCtx->pBiquadProcess = s_biquadFloatProcessNoResamplingMultiChannels;
          break;
      }
    }
    else
    {
      switch (nbChannels)
      {
        case 1:
          pCtx->pBiquadProcess = s_biquadFloatProcessResamplingMono;
          break;
        case 2:
          pCtx->pBiquadProcess = s_biquadFloatProcessResamplingStereo;
          break;
        default:
          pCtx->pBiquadProcess = s_biquadFloatProcessResamplingMultiChannels;
          break;
      }
    }
    #endif /* BIQUAD_CODE_SIZE_OPTIM */
  }

  return error;
}


/**
  * @brief  de-initialize biquad (16 bits fixed-point version)
  * @param  pCtx              biquad context pointer
  * @retval NONE
  */
void biquadInt16DeInit(biquadIntContext_t *const pCtx)
{
  biquadInt32DeInit(pCtx);
}


/**
  * @brief  de-initialize biquad (32 bits fixed-point version)
  * @param  pCtx              biquad context pointer
  * @retval NONE
  */
void biquadInt32DeInit(biquadIntContext_t *const pCtx)
{
  if (pCtx != NULL)
  {
    if (pCtx->pBiquadMem != NULL)
    {
      memPool_t const memPool = pCtx->memPool;

      AudioAlgo_free(pCtx->pBiquadMem, memPool);
    }
    memset(pCtx, 0, sizeof(biquadIntContext_t));
  }
}


/**
  * @brief  de-initialize biquad (floating-point version)
  * @param  pCtx              biquad context pointer
  * @retval NONE
  */
void biquadFloatDeInit(biquadFloatContext_t *const pCtx)
{
  if (pCtx != NULL)
  {
    if (pCtx->pBiquadMem != NULL)
    {
      memPool_t const memPool = pCtx->memPool;

      AudioAlgo_free(pCtx->pBiquadMem, memPool);
    }
    memset(pCtx, 0, sizeof(biquadFloatContext_t));
  }
}


/**
  * @brief  process biquad filtering (16 bits fixed-point version)
  * @param  pCtx              biquad context pointer
  * @param  in                input buffer pointer
  * @param  out               output buffer pointer
  * @param  nbSamplesIn       number of input samples
  * @retval NONE
  */
void biquadInt16Process(biquadIntContext_t *const pCtx, int16_t *const in, int16_t *const out, int const nbSamplesIn)
{
  (*pCtx->pBiquadProcess)(pCtx, in, out, nbSamplesIn);
}


/**
  * @brief  process biquad filtering (32 bits fixed-point version)
  * @param  pCtx              biquad context pointer
  * @param  in                input buffer pointer
  * @param  out               output buffer pointer
  * @param  nbSamplesIn       number of input samples
  * @retval NONE
  */
void biquadInt32Process(biquadIntContext_t *const pCtx, int32_t *const in, int32_t *const out, int const nbSamplesIn)
{
  (*pCtx->pBiquadProcess)(pCtx, in, out, nbSamplesIn);
}


/**
  * @brief  process biquad filtering (floating-point version)
  * @param  pCtx           biquad pCtx pointer
  * @param  in                input buffer pointer
  * @param  out               output buffer pointer
  * @param  nbSamplesIn       number of input samples
  * @retval NONE
  */
void biquadFloatProcess(biquadFloatContext_t *const pCtx, float *const in, float *const out, int const nbSamplesIn)
{
  (*pCtx->pBiquadProcess)(pCtx, in, out, nbSamplesIn);
}


/**
  * @brief  clear biquad history (16 bits fixed-point version)
  * @param  pCtx           biquad pCtx pointer
  * @retval NONE
  */
void biquadInt16ClearHistory(biquadIntContext_t *const pCtx)
{
  biquadInt32ClearHistory(pCtx);
}


/**
  * @brief  clear biquad history (32 bits fixed-point version)
  * @param  pCtx           biquad pCtx pointer
  * @retval NONE
  */
void biquadInt32ClearHistory(biquadIntContext_t *const pCtx)
{
  if (pCtx->pBiquadMem != NULL)
  {
    memset(pCtx->pBiquadMem, 0, ((2UL * (size_t)pCtx->pBiquad->nbCells) + 2UL) * (size_t)pCtx->nbChannels * sizeof(*pCtx->pBiquadMem));
  }
}


/**
  * @brief  clear biquad history (floating-point version)
  * @param  pCtx           biquad pCtx pointer
  * @retval NONE
  */
void biquadFloatClearHistory(biquadFloatContext_t *const pCtx)
{
  if (pCtx->pBiquadMem != NULL)
  {
    memset(pCtx->pBiquadMem, 0, ((2UL * (size_t)pCtx->pBiquad->nbCells) + 2UL) * (size_t)pCtx->nbChannels * sizeof(*pCtx->pBiquadMem));
  }
}


/* Private Functions Definition ------------------------------------------------------*/

static int32_t s_computeQuadFromRoots(root_t const *const pRoot1, root_t const *const pRoot2, quad_t *const pQuad)
{
  int32_t error  = AUDIO_ERR_MGNT_NONE;
  int     nbRoot = 0;
  float   re1    = 0.0f;
  float   re2    = 0.0f;
  float   im1    = 0.0f;
  float   im2    = 0.0f;
  float   gain   = 0.0f;

  switch (pRoot1->rootType)
  {
    case NO_ROOT:
      switch (pRoot2->rootType)
      {
        case NO_ROOT:
          gain   = 1.0f;
          nbRoot = 0;
          break;
        case REAL_ROOT:
          gain   = pRoot2->root.realRoot.gain;
          re1    = pRoot2->root.realRoot.root;
          im1    = 0.0f;
          nbRoot = 1;
          break;
        case COMPLEX_ROOT:
          error = AUDIO_ERR_MGNT_ERROR;
          break;
        default:
          error = AUDIO_ERR_MGNT_ERROR;
          break;
      }
      break;

    case REAL_ROOT:
      switch (pRoot2->rootType)
      {
        case NO_ROOT:
          gain   = pRoot1->root.realRoot.gain;
          re1    = pRoot1->root.realRoot.root;
          im1    = 0.0f;
          nbRoot = 1;
          break;
        case REAL_ROOT:
          gain   = pRoot1->root.realRoot.gain * pRoot2->root.realRoot.gain;
          re1    = pRoot1->root.realRoot.root;
          im1    = 0.0f;
          re2    = pRoot2->root.realRoot.root;
          im2    = 0.0f;
          nbRoot = 2;
          break;
        case COMPLEX_ROOT:
          error = AUDIO_ERR_MGNT_ERROR;
          break;
        default:
          error = AUDIO_ERR_MGNT_ERROR;
          break;
      }
      break;

    case COMPLEX_ROOT:
      switch (pRoot2->rootType)
      {
        case NO_ROOT:
          error = AUDIO_ERR_MGNT_ERROR;
          break;
        case REAL_ROOT:
          error = AUDIO_ERR_MGNT_ERROR;
          break;
        case COMPLEX_ROOT:
          gain   = pRoot1->root.complexRoot.gain * pRoot2->root.complexRoot.gain;
          re1    = pRoot1->root.complexRoot.root.re;
          im1    = pRoot1->root.complexRoot.root.im;
          re2    = pRoot2->root.complexRoot.root.re;
          im2    = pRoot2->root.complexRoot.root.im;
          nbRoot = 2;
          if ((fabsf(re1 - re2) + fabsf(im1 + im2)) > 0.0000001f) /*cstat !MISRAC2012-Rule-22.8 no issue with fabsf => errno check is useless */
          {
            // if we have 2 complex poles/zeros, they must be conjugates
            error = AUDIO_ERR_MGNT_ERROR;
          }
          break;
        default:
          error = AUDIO_ERR_MGNT_ERROR;
          break;
      }
      break;

    default:
      error = AUDIO_ERR_MGNT_ERROR;
      break;
  }

  if (AudioError_isOk(error))
  {
    switch (nbRoot)
    {
      case 0:
        // biquad(z) = num.q0 / den.q0
        //           = gz / qp
        // => num.q0=gz, num.q1=-0, num.q2=0, den.q0=gp, den.q1=0, den.q2=0
        // with zeros z1 & z2 and poles p1 & p2
        //
        // pQuad is either &num or &den and roots are z1 & z2 or p1 & p2 respectively
        pQuad->q0 = gain;
        pQuad->q1 = 0.0f;
        pQuad->q2 = 0.0f;
        break;

      case 1:
        // biquad(z) = (num.q0 + num.q1*z^-1) / (den.q0 + den.q1*z^-1)
        //           = (num.q0*z + num.q1) / (den.q0*z + den.q1)
        //           = [gz * (z - z1)] / [gp * (z - p1)]
        //           = (gz*z - gz*z1) / (qp*z - gp*p1)
        // => num.q0=gz, num.q1=-gz*z1, num.q2=0, den.q0=gp, den.q1=-qp*p1, den.q2=0
        // with zeros z1 & z2 and poles p1 & p2
        //
        // pQuad is either &num or &den and roots are z1 & z2 or p1 & p2 respectively
        pQuad->q0 =  gain;
        pQuad->q1 = -gain * re1;
        pQuad->q2 =  0.0f;
        break;

      case 2:
        // biquad(z) = (num.q0 + num.q1*z^-1 + num.q2*z^-2) / (den.q0 + den.q1*z^-1 + den.q2*z^-2)
        //           = (num.q0*z^2 + num.q1*z + num.q2) / (den.q0*z^2 + den.q1*z + den.q2)
        //           = [gz * (z - z1) * (z - z2)] / [gp * (z - p1) * (z - p2)]
        //           = (gz*z^2 - gz*(z1+z2)*z + gz*z1*z2) / (qp*z^2 - gp*(p1+p2)*z + gp*p1*p2)
        // => num.q0=gz, num.q1=-gz*(z1+z2), num.q2=gz*z1*z2, den.q0=gp, den.q1=-qp*(p1+p2), den.q2=qp*p1*p2
        // with zeros z1 & z2 and poles p1 & p2
        //
        // pQuad is either &num or &den and roots are z1 & z2 or p1 & p2 respectively
        pQuad->q0 =  gain;
        pQuad->q1 = -gain * (re1 + re2);                  // no imaginary part (when roots are complex, they are conjugates)
        pQuad->q2 =  gain * ((re1 * re2) - (im1 * im2));  // no imaginary part (when roots are complex, they are conjugates)
        break;

      default:
        break;
    }
  }

  return error;
}


static float s_squareDist(root_t *const pRoot1, root_t *const pRoot2)
{
  float squareDist = 0.0f;

  switch (pRoot1->rootType)
  {
    case REAL_ROOT:
      switch (pRoot2->rootType)
      {
        case REAL_ROOT:
          squareDist = s_squareDistRealReal(pRoot1->root.realRoot.root, pRoot2->root.realRoot.root);
          break;
        case COMPLEX_ROOT:
          squareDist = s_squareDistComplexReal(&pRoot2->root.complexRoot.root, pRoot1->root.realRoot.root);
          break;
        default:
          break;
      }
      break;
    case COMPLEX_ROOT:
      switch (pRoot2->rootType)
      {
        case REAL_ROOT:
          squareDist = s_squareDistComplexReal(&pRoot1->root.complexRoot.root, pRoot2->root.realRoot.root);
          break;
        case COMPLEX_ROOT:
          squareDist = s_squareDistComplexComplex(&pRoot1->root.complexRoot.root, &pRoot2->root.complexRoot.root);
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }

  return squareDist;
}


static float s_squareDistComplexComplex(complex_t const *const pCplx1, complex_t const *const pCplx2)
{
  float re = pCplx1->re - pCplx2->re;
  float im = pCplx1->im - pCplx2->im;

  return (re * re) + (im * im);
}


static float s_squareDistComplexReal(complex_t const *const pCplx, float const real)
{
  float re = pCplx->re - real;
  float im = pCplx->im;

  return (re * re) + (im * im);
}


static float s_squareDistRealReal(float const real1, float const real2)
{
  float re = real1 - real2;

  return re * re;
}


static float s_module2MaxBiquad(float const b0, float const b1, float const b2, float const a1, float const a2, float *const pModule2)
{
  /*********************************************************************************************************************************

                   || b0 + b1.exp(-j.w) + b2.exp(-2.j.w) ||
  ||H(exp(j.w)|| = ||------------------------------------||
                   || 1  + a1.exp(-j.w) + a2.exp(-2.j.w) ||

                   || b0 + b1.cos(w) + b2.cos(2.w) -j.(b1.sin(w) + b2.sin(2.w))  ||
                 = ||----------------------------------------------------------- ||
                   || 1  + a1.cos(w) + a2.cos(2.w) -j.(a1.sin(w) + a2.sin(2.w))  ||

                   [ (b0 + b1.cos(w) + b2.cos(2.w))^2 + (b1.sin(w) + b2.sin(2.w))^2  ]^1/2
                 = [---------------------------------------------------------------- ]
                   [ (1  + a1.cos(w) + a2.cos(2.w))^2 + (a1.sin(w) + a2.sin(2.w))^2  ]

                   [ b0^2 + b1^2 + b2^2 + 2.b0.b1.cos(w) + 2.b0.b2.cos(2.w) + 2.b1.b2.(cos(w).cos(2.w) + sin(w).sin(2.w))  ]^1/2
                 = [------------------------------------------------------------------------------------------------------ ]
                   [  1   + a1^2 + a2^2 +    2.a1.cos(w) +    2.a2.cos(2.w) + 2.a1.a2.(cos(w).cos(2.w) + sin(w).sin(2.w))  ]

                   [ b0^2 + b1^2 + b2^2 + 2.b0.b1.cos(w) + 2.b0.b2.cos(2.w) + 2.b1.b2.cos(w)  ]^1/2
                 = [------------------------------------------------------------------------- ]
                   [  1   + a1^2 + a2^2 +    2.a1.cos(w) +    2.a2.cos(2.w) + 2.a1.a2.cos(w)  ]

                   [ b0^2 + b1^2 + b2^2 + 2.b1.(b0 + b2).cos(w) + 2.b0.b2.cos(2.w)  ]^1/2
                 = [--------------------------------------------------------------- ]
                   [  1   + a1^2 + a2^2 + 2.a1.( 1 + a2).cos(w) +    2.a2.cos(2.w)  ]

  *********************************************************************************************************************************/
  // this optimization creates accuracy issues and then NAN (Not A Number) results !
  //float const t0         = b0 * b0 + b1 * b1 + b2 * b2;
  //float const t1         = 2.0f * b1 * (b0 + b2);
  //float const t2         = 2.0f * b0 * b2;
  //float const t3         = 1.0f + a1 * a1 + a2 * a2;
  //float const t4         = 2.0f * a1 * (1.0f + a2);
  //float const t5         = 2.0f * a2;
  float const dTheta     = 2.0f * PI / (float)IIR_RESPONSE_ACCURACY;
  float       module2Max = 0.0f;

  for (int i = 0; i < (IIR_RESPONSE_ACCURACY / 2); i++)
  {
    float const theta = dTheta * (float)i;
    float const cos1  = arm_cos_f32(theta);
    float const cos2  = (2.0f * cos1 * cos1) - 1.0f;
    float const sin1  = arm_sin_f32(theta);
    float const sin2  = 2.0f * sin1 * cos1;
    float const re1   = b0   + (b1 * cos1) + (b2 * cos2);
    float const im1   = (b1 * sin1) + (b2 * sin2);
    float const re2   = 1.0f + (a1 * cos1) + (a2 * cos2);
    float const im2   = (a1 * sin1) + (a2 * sin2);
    float const num   = (re1 * re1) + (im1 * im1);
    float const den   = (re2 * re2) + (im2 * im2);
    //float const num   = t0 + (t1 * cos1) + (t2 * cos2);
    //float const den   = t3 + (t4 * cos1) + (t5 * cos2);

    if ((den < -0.0000000001f) || (den > 0.0000000001f))
    {
      pModule2[i] *= num / den;
    }

    module2Max = max(module2Max, pModule2[i]);
  }

  return module2Max;
}


static int32_t s_check_biquad_stability(biquadDesignCtx_t *const pBiquadDesignCtx, float *const pModule2)
{
  int32_t error   = AUDIO_ERR_MGNT_NONE;
  uint8_t nbCells = (pBiquadDesignCtx->sampleType == ABUFF_FORMAT_FLOAT) ? pBiquadDesignCtx->biquadCoef.pBiquadFloatCoef->nbCells : pBiquadDesignCtx->biquadCoef.pBiquadIntCoef->nbCells;
  float   gain    = 1.0f;
  float   coef, invCoef;

  for (int i = 0; i < (IIR_RESPONSE_ACCURACY / 2); i++)
  {
    pModule2[i] = 1.0f;
  }

  for (uint8_t i = 0U; AudioError_isOk(error) && (i < nbCells); i++)
  {
    biquadCellFloat_t *pBiquadCellFloat = NULL;
    biquadCellInt32_t *pBiquadCellInt32 = NULL;
    float              b0, b1, b2, a1, a2, delta, sqrtDelta, poleModule, module2Max;
    bool               stability;

    // get b0, b1, b2, a1, a2 as float biquad coefficients
    if (pBiquadDesignCtx->sampleType == ABUFF_FORMAT_FLOAT)
    {
      pBiquadCellFloat = (biquadCellFloat_t *)&pBiquadDesignCtx->biquadCoef.pBiquadFloatCoef->pBiquadCell[i]; // remove const qualifier
      b0               = pBiquadCellFloat->b0;
      b1               = pBiquadCellFloat->b1;
      b2               = pBiquadCellFloat->b2;
      a1               = pBiquadCellFloat->a1;
      a2               = pBiquadCellFloat->a2;
    }
    else
    {
      pBiquadCellInt32 = (biquadCellInt32_t *)&pBiquadDesignCtx->biquadCoef.pBiquadIntCoef->pBiquadCell[i];   // remove const qualifier
      b0               = (float)pBiquadCellInt32->b0 / 2147483648.0f;
      b1               = (float)pBiquadCellInt32->b1 / 2147483648.0f;
      b2               = (float)pBiquadCellInt32->b2 / 2147483648.0f;
      a1               = (float)pBiquadCellInt32->a1 / 1073741824.0f; // a1 / 2
      a2               = (float)pBiquadCellInt32->a2 / 2147483648.0f;
    }

    // compute max(module(p1), module(p2))
    delta = (a1 * a1) - (4.0f * a2);
    if (delta < 0.0f)
    {
      //sqrtDelta  = sqrtf(-delta);
      //re         = -a1       * 0.5f;
      //im         = sqrtDelta * 0.5f;
      //poleModule = sqrtf((re * re) + (im * im));
      arm_sqrt_f32(a2, &poleModule);  // poleModule = sqrt(re^2 + im^2) = sqrt(a1^2/4 - (a1^2 - 4*a2)/4) = sqrt(a2)
    }
    else
    {
      //sqrtDelta  = sqrtf(delta);
      //p1         = (-a1 - sqrtDelta) * 0.5f;
      //p2         = (-a1 + sqrtDelta) * 0.5f;
      //poleModule = (a1 > 0.0f) ? -p1 : p2;
      arm_sqrt_f32(delta, &sqrtDelta);
      poleModule = (a1 > 0.0f) ? ((a1 + sqrtDelta) * 0.5f) : ((-a1 + sqrtDelta) * 0.5f);  // the biggest of module(p1) and module(p2)
    }

    if (poleModule > 0.999f)
    {
      // at least one pole is outside of unit circle: recompute poles to be inside unit circle applying a coef of (0.999f / poleModule) to poles p1 and p2
      //
      // before poles recomputation:
      //   biquad(z) = (b0 + b1*z^-1 + b2*z^-2) / (1 + a1*z^-1 + a2*z^-2)
      //             = (b0*z^2 + b1*z + b2) / (z^2 + a1*z + a2)
      //             = g * [(z - z1) * (z - z2)] / [(z - p1) * (z - p2)]
      //             = (g*z^2 - g*(z1+z2)*z + g*z1*z2) / (z^2 - (p1+p2)*z + p1*p2)
      //   => b0=g, b1=-g*(z1+z2), b2=g*z1*z2, a1=-(p1+p2), a2=p1*p2
      //   with zeros z1 & z2 and poles p1 & p2
      //
      // after poles recomputation:
      //   biquad(z) = g * [(z - z1) * (z - z2)] / [(z - coef*p1) * (z - coef*p2)]
      //             = (g*z^2 - g*(z1+z2)*z + g*z1*z2) / (z^2 - coef*(p1+p2)*z + coef^2*p1*p2)
      //   => a'1 = a1*coef
      //      a'2 = a2*coef^2
      coef = 0.999f / poleModule;
      a1  *= coef;
      a2  *= coef * coef;
    }

    if (pBiquadDesignCtx->sampleType == ABUFF_FORMAT_FLOAT)
    {
      pBiquadCellFloat->a1 = a1;
      pBiquadCellFloat->a2 = a2;

      // module2Max value is useless for float format but module still must be computed
      s_module2MaxBiquad(b0, b1, b2, a1, a2, pModule2);
    }
    else
    {
      float bMin = 0.0f;
      float bMax = 0.0f;

      module2Max = s_module2MaxBiquad(b0, b1, b2, a1, a2, pModule2);
      if (module2Max > 1.0f)
      {
        arm_scale_f32(pModule2, 1.0f / module2Max, pModule2, IIR_RESPONSE_ACCURACY / 2);
        arm_sqrt_f32(module2Max, &coef);
        invCoef = 1.0f / coef;
        b0     *= invCoef;
        b1     *= invCoef;
        b2     *= invCoef;
        gain   *= coef;
      }

      bMin  = min(bMin, b0);
      bMin  = min(bMin, b1);
      bMin  = min(bMin, b2);
      bMax  = max(bMax, b0);
      bMax  = max(bMax, b1);
      bMax  = max(bMax, b2);
      bMin *= 2147483648.0f;
      bMax *= 2147483648.0f;
      coef  = 1.0f;
      if (bMin < -2147483648.0f)
      {
        coef = -2147483648.0f / bMin;
      }
      if (bMax > 2147483647.0f)
      {
        coef = min(coef, 2147483647.0f / bMax);
      }
      if (coef > 1.0f)
      {
        invCoef = 1.0f / coef;
        b0     *= invCoef;
        b1     *= invCoef;
        b2     *= invCoef;
        gain   *= coef;
      }

      pBiquadCellInt32->a1 = s_round32(a1 * 1073741824.0f);               // a1 / 2
      pBiquadCellInt32->a2 = s_round32(a2 * 2147483648.0f);
      pBiquadCellInt32->b0 = s_round32(b0 * 2147483648.0f);
      pBiquadCellInt32->b1 = s_round32(b1 * 2147483648.0f);
      pBiquadCellInt32->b2 = s_round32(b2 * 2147483648.0f);
      a1                   = (float)pBiquadCellInt32->a1 / 1073741824.0f; // a1 / 2
      a2                   = (float)pBiquadCellInt32->a2 / 2147483648.0f;
      b0                   = (float)pBiquadCellInt32->b0 / 2147483648.0f;
      b1                   = (float)pBiquadCellInt32->b1 / 2147483648.0f;
      b2                   = (float)pBiquadCellInt32->b2 / 2147483648.0f;
    }

    // stability check: poles must be inside unit circle and stability triangle
    delta = (a1 * a1) - (4.0f * a2);
    if (delta < 0.0f)
    {
      //sqrtDelta  = sqrtf(-delta);
      //re         = -a1       * 0.5f;
      //im         = sqrtDelta * 0.5f;
      //poleModule = sqrtf((re * re) + (im * im));
      arm_sqrt_f32((a1 * a1) - delta, &poleModule);
      poleModule *= 0.5f;
    }
    else
    {
      //sqrtDelta  = sqrtf(delta);
      //p1         = (-a1 - sqrtDelta) * 0.5f;
      //p2         = (-a1 + sqrtDelta) * 0.5f;
      //poleModule = (a1 > 0.0f) ? -p1 : p2;
      arm_sqrt_f32(delta, &sqrtDelta);
      poleModule = (a1 > 0.0f) ? ((a1 + sqrtDelta) * 0.5f) : ((-a1 + sqrtDelta) * 0.5f);  // the biggest of module(p1) and module(p2)
    }
    stability = (poleModule < 1.0f);                                                      // poles inside unit circle
    stability = stability && (a2 <= 1.0f) && (a2 >= (a1 - 1.0f)) && (a2 >= (-a1 - 1.0f)); // stability triangle : see https://dsp.stackexchange.com/questions/24293/ensure-stability-against-coefficient-quantization
    if (!stability)
    {
      error = AUDIO_ERR_MGNT_CONFIG;
    }
  }

  if (AudioError_isOk(error) && (pBiquadDesignCtx->sampleType != ABUFF_FORMAT_FLOAT))
  {
    /* update pModule2 with squared global biquad gain and update gainMant and gainExp */
    uint8_t  shift = pBiquadDesignCtx->biquadCoef.pBiquadIntCoef->gainExp;
    uint32_t div   = (shift > 31U) ? 1UL : (1UL << (31U - shift));  // MISRAC: issue in case shift>31 but in fact can't happen

    gain *= 2147483648.0f * (float)pBiquadDesignCtx->biquadCoef.pBiquadIntCoef->gainMant / (float)div;
    for (shift = 0U; gain > 2147483647.0f; shift++)
    {
      gain *= 0.5f;
    }
    pBiquadDesignCtx->biquadCoef.pBiquadIntCoef->gainMant = s_round32(gain);
    pBiquadDesignCtx->biquadCoef.pBiquadIntCoef->gainExp  = shift;
    div                                                   = (shift > 31U) ? 1UL : (1UL << (31U - shift)); // MISRAC: issue in case shift>31 but in fact can't happen
    gain                                                  = (float)pBiquadDesignCtx->biquadCoef.pBiquadIntCoef->gainMant / (float)div;
    arm_scale_f32(pModule2, gain * gain, pModule2, IIR_RESPONSE_ACCURACY / 2);
  }

  return error;
}


static int32_t s_round32(float const x)
{
  int32_t y;

  if (x >= 0.0f)
  {
    y = (x > 2147483647.0f) ? 2147483647              : (int32_t)(x + 0.5f);    /*cstat !MISRAC2012-Rule-10.8 could use floorf to solve MISRAC issue but at the expense of increased cpu load*/
  }
  else
  {
    y = (x < -2147483648.0f) ? (int32_t)(-2147483648) : -(int32_t)(-x + 0.5f);  /*cstat !MISRAC2012-Rule-10.8 could use floorf to solve MISRAC issue but at the expense of increased cpu load*/
  }

  return y;
}


static int32_t s_biquadIntCommonInit(biquadIntContext_t *const pCtx, biquadInt32_t const *const pBiquad, int const nbChannels, int const decimFactor, int const upSamplingFactor, memPool_t const memPool)
{
  int32_t   error = AUDIO_ERR_MGNT_NONE;
  int const size  = ((2 * (int)pBiquad->nbCells) + 2) * nbChannels;
  int32_t  *pBiquadMem;

  pBiquadMem = (int32_t *)AudioAlgo_malloc((size_t)size * sizeof(int32_t), memPool);
  if (pBiquadMem == NULL)
  {
    error = AUDIO_ERR_MGNT_ALLOCATION;
  }
  else
  {
    memset(pBiquadMem, 0, (size_t)size * sizeof(int32_t));

    pCtx->memPool          = memPool;
    pCtx->pBiquad          = pBiquad;
    pCtx->nbChannels       = nbChannels;
    pCtx->decimFactor      = decimFactor;
    pCtx->upSamplingFactor = upSamplingFactor;
    pCtx->pBiquadMem       = pBiquadMem;
    if (upSamplingFactor == 1)
    {
      pCtx->gainMant       = pBiquad->gainMant;
      pCtx->gainExp        = pBiquad->gainExp;
    }
    else
    {
      // apply a gain=upSamplingFactor inside gainMant*2^gainExp to compensate attenuation introduced by inserted zeros
      int64_t gainMant = (int64_t)pBiquad->gainMant * (int64_t)upSamplingFactor;

      for (pCtx->gainExp = pBiquad->gainExp; (gainMant > 0x7FFFFFFFLL); pCtx->gainExp++)
      {
        gainMant /= 2;
      }
      pCtx->gainMant = (int32_t)gainMant;
    }
  }

  return error;
}


#ifndef BIQUAD_CODE_SIZE_OPTIM
static void s_biquadInt16ProcessNoResamplingMono(biquadIntContext_t *const pCtx, void *const in, void *const out, int const nbSamplesIn)
{
  biquadInt32_t     const *const  pBiquad       = pCtx->pBiquad;
  uint8_t                  const  nbBiquadCells = pBiquad->nbCells;
  int16_t                  const *pIn           = (int16_t const *)in;
  int16_t                        *pOut          = (int16_t *)out;
  biquadCellInt32_t const        *pBiquadCell;
  int32_t                        *pBiquadMem;
  int                             i;
  uint8_t                         n;

  #ifdef BIQUAD_USE_INTRINSICS
  int32_t x, y;

  for (i = 0; i < nbSamplesIn; i++)
  {
    x = (((int32_t)(*pIn)) << 16U); /*cstat !MISRAC2012-Rule-10.1_R6 shift on signed integer for cpu load efficiency*/
    pIn++;
    if (nbBiquadCells > 0U)
    {
      pBiquadCell = pBiquad->pBiquadCell;
      pBiquadMem  = pCtx->pBiquadMem;
      for (n = 0U; n < nbBiquadCells; n++)
      {
        y             = __SMMULR(pBiquadCell->b0, x);
        y             = __SMMLAR(pBiquadCell->b1, pBiquadMem[0], y);
        y             = __SMMLAR(pBiquadCell->b2, pBiquadMem[1], y);
        y             = __SMMLSR(pBiquadCell->a1, pBiquadMem[2], y);  // a1 is Q30 whereas other coefs are Q31
        y             = __SMMLSR(pBiquadCell->a1, pBiquadMem[2], y);  // a1 is Q30 whereas other coefs are Q31
        y             = __SMMLSR(pBiquadCell->a2, pBiquadMem[3], y);
        y             = __QADD(y, y);
        pBiquadMem[1] = pBiquadMem[0];
        pBiquadMem[0] = x;
        x             = y;
        pBiquadMem   += 2;
        pBiquadCell++;
      }
      pBiquadMem[1] = pBiquadMem[0];
      pBiquadMem[0] = x;
    }
    x     = __SMMULR(x, pCtx->gainMant) >> (15U - pCtx->gainExp); /*cstat !MISRAC2012-Rule-10.1_R6 !MISRAC2012-Rule-1.3_n shift on signed integer for cpu load efficiency*/
    x     = __SSAT(x, 16);
    *pOut = (int16_t)x;
    pOut++;
  }
  #else
  int64_t x, y;

  for (i = 0; i < nbSamplesIn; i++)
  {
    x = (((int64_t)(*pIn)) << 16U);   /*cstat !MISRAC2012-Rule-10.1_R6 shift on signed integer for cpu load efficiency*/
    pIn++;
    if (nbBiquadCells > 0U)
    {
      pBiquadCell = pBiquad->pBiquadCell;
      pBiquadMem  = pCtx->pBiquadMem;
      for (n = 0U; n < nbBiquadCells; n++)
      {
        y             = (int64_t)pBiquadCell->b0 * x;
        y            += (int64_t)pBiquadCell->b1 * (int64_t)pBiquadMem[0];
        y            += (int64_t)pBiquadCell->b2 * (int64_t)pBiquadMem[1];
        y            -= (int64_t)pBiquadCell->a1 * (int64_t)pBiquadMem[2] * 2LL;  // a1 is Q30 whereas other coefs are Q31
        y            -= (int64_t)pBiquadCell->a2 * (int64_t)pBiquadMem[3];
        y           >>= 31U;        /*cstat !MISRAC2012-Rule-10.1_R6 !MISRAC2012-Rule-1.3_n shift on signed integer for cpu load efficiency*/
        y             = (y < -2147483648LL) ? -2147483648LL : ((y > 2147483647LL) ? 2147483647LL : y);
        pBiquadMem[1] = pBiquadMem[0];
        pBiquadMem[0] = (int32_t)x;
        x             = y;
        pBiquadMem   += 2;
        pBiquadCell++;
      }
      pBiquadMem[1] = pBiquadMem[0];
      pBiquadMem[0] = (int32_t)x;
    }
    x     = (x * (int64_t)pCtx->gainMant) >> (47U - pCtx->gainExp); /*cstat !MISRAC2012-Rule-10.1_R6 !MISRAC2012-Rule-1.3_n shift on signed integer for cpu load efficiency*/
    x     = (x < -32768LL) ? -32768LL : ((x > 32767LL) ? 32767LL : x);
    *pOut = (int16_t)x;
    pOut++;
  }
  #endif
}


static void s_biquadInt16ProcessNoResamplingStereo(biquadIntContext_t *const pCtx, void *const in, void *const out, int const nbSamplesIn)
{
  biquadInt32_t     const *const  pBiquad       = pCtx->pBiquad;
  uint8_t                  const  nbBiquadCells = pBiquad->nbCells;
  int16_t                  const *pIn           = (int16_t const *)in;
  int16_t                        *pOut          = (int16_t *)out;
  biquadCellInt32_t const        *pBiquadCell;
  int32_t                        *pBiquadMem;
  int                             i;
  uint8_t                         n;

  #ifdef BIQUAD_USE_INTRINSICS
  int32_t x1, x2, y1, y2;

  for (i = 0; i < nbSamplesIn; i++)
  {
    x1 = (((int32_t)(*pIn)) << 16U);  /*cstat !MISRAC2012-Rule-10.1_R6 shift on signed integer for cpu load efficiency*/
    pIn++;
    x2 = (((int32_t)(*pIn)) << 16U);  /*cstat !MISRAC2012-Rule-10.1_R6 shift on signed integer for cpu load efficiency*/
    pIn++;
    if (nbBiquadCells > 0U)
    {
      pBiquadCell = pBiquad->pBiquadCell;
      pBiquadMem  = pCtx->pBiquadMem;
      for (n = 0U; n < nbBiquadCells; n++)
      {
        y1            = __SMMULR(pBiquadCell->b0, x1);
        y2            = __SMMULR(pBiquadCell->b0, x2);
        y1            = __SMMLAR(pBiquadCell->b1, pBiquadMem[0], y1);
        y2            = __SMMLAR(pBiquadCell->b1, pBiquadMem[1], y2);
        y1            = __SMMLAR(pBiquadCell->b2, pBiquadMem[2], y1);
        y2            = __SMMLAR(pBiquadCell->b2, pBiquadMem[3], y2);
        y1            = __SMMLSR(pBiquadCell->a1, pBiquadMem[4], y1); // a1 is Q30 whereas other coefs are Q31
        y1            = __SMMLSR(pBiquadCell->a1, pBiquadMem[4], y1); // a1 is Q30 whereas other coefs are Q31
        y2            = __SMMLSR(pBiquadCell->a1, pBiquadMem[5], y2); // a1 is Q30 whereas other coefs are Q31
        y2            = __SMMLSR(pBiquadCell->a1, pBiquadMem[5], y2); // a1 is Q30 whereas other coefs are Q31
        y1            = __SMMLSR(pBiquadCell->a2, pBiquadMem[6], y1);
        y2            = __SMMLSR(pBiquadCell->a2, pBiquadMem[7], y2);
        y1            = __QADD(y1, y1);
        y2            = __QADD(y2, y2);
        pBiquadMem[2] = pBiquadMem[0];
        pBiquadMem[3] = pBiquadMem[1];
        pBiquadMem[0] = x1;
        pBiquadMem[1] = x2;
        x1            = y1;
        x2            = y2;
        pBiquadMem   += 4;
        pBiquadCell++;
      }
      pBiquadMem[2] = pBiquadMem[0];
      pBiquadMem[3] = pBiquadMem[1];
      pBiquadMem[0] = x1;
      pBiquadMem[1] = x2;
    }
    x1    = __SMMULR(x1, pCtx->gainMant) >> (15U - pCtx->gainExp);  /*cstat !MISRAC2012-Rule-10.1_R6 !MISRAC2012-Rule-1.3_n shift on signed integer for cpu load efficiency*/
    x2    = __SMMULR(x2, pCtx->gainMant) >> (15U - pCtx->gainExp);  /*cstat !MISRAC2012-Rule-10.1_R6 !MISRAC2012-Rule-1.3_n shift on signed integer for cpu load efficiency*/
    x1    = __SSAT(x1, 16);
    x2    = __SSAT(x2, 16);
    *pOut = (int16_t)x1;
    pOut++;
    *pOut = (int16_t)x2;
    pOut++;
  }
  #else
  int64_t x1, x2, y1, y2;

  for (i = 0; i < nbSamplesIn; i++)
  {
    x1 = (((int64_t)(*pIn)) << 16U);  /*cstat !MISRAC2012-Rule-10.1_R6 shift on signed integer for cpu load efficiency*/
    pIn++;
    x2 = (((int64_t)(*pIn)) << 16U);  /*cstat !MISRAC2012-Rule-10.1_R6 shift on signed integer for cpu load efficiency*/
    pIn++;
    if (nbBiquadCells > 0U)
    {
      pBiquadCell = pBiquad->pBiquadCell;
      pBiquadMem  = pCtx->pBiquadMem;
      for (n = 0U; n < nbBiquadCells; n++)
      {
        y1            = (int64_t)pBiquadCell->b0 * x1;
        y2            = (int64_t)pBiquadCell->b0 * x2;
        y1           += (int64_t)pBiquadCell->b1 * (int64_t)pBiquadMem[0];
        y2           += (int64_t)pBiquadCell->b1 * (int64_t)pBiquadMem[1];
        y1           += (int64_t)pBiquadCell->b2 * (int64_t)pBiquadMem[2];
        y2           += (int64_t)pBiquadCell->b2 * (int64_t)pBiquadMem[3];
        y1           -= (int64_t)pBiquadCell->a1 * (int64_t)pBiquadMem[4] * 2LL;  // a1 is Q30 whereas other coefs are Q31
        y2           -= (int64_t)pBiquadCell->a1 * (int64_t)pBiquadMem[5] * 2LL;  // a1 is Q30 whereas other coefs are Q31
        y1           -= (int64_t)pBiquadCell->a2 * (int64_t)pBiquadMem[6];
        y2           -= (int64_t)pBiquadCell->a2 * (int64_t)pBiquadMem[7];
        y1          >>= 31;   /*cstat !MISRAC2012-Rule-10.1_R6 !MISRAC2012-Rule-1.3_n shift on signed integer for cpu load efficiency*/
        y2          >>= 31;   /*cstat !MISRAC2012-Rule-10.1_R6 !MISRAC2012-Rule-1.3_n shift on signed integer for cpu load efficiency*/
        y1            = (y1 < -2147483648LL) ? -2147483648LL : ((y1 > 2147483647LL) ? 2147483647LL : y1);
        y2            = (y2 < -2147483648LL) ? -2147483648LL : ((y2 > 2147483647LL) ? 2147483647LL : y2);
        pBiquadMem[2] = pBiquadMem[0];
        pBiquadMem[3] = pBiquadMem[1];
        pBiquadMem[0] = (int32_t)x1;
        pBiquadMem[1] = (int32_t)x2;
        x1            = y1;
        x2            = y2;
        pBiquadMem   += 4;
        pBiquadCell++;
      }
      pBiquadMem[2] = pBiquadMem[0];
      pBiquadMem[3] = pBiquadMem[1];
      pBiquadMem[0] = (int32_t)x1;
      pBiquadMem[1] = (int32_t)x2;
    }
    x1    = (x1 * (int64_t)pCtx->gainMant) >> (47U - pCtx->gainExp);  /*cstat !MISRAC2012-Rule-10.1_R6 !MISRAC2012-Rule-1.3_n shift on signed integer for cpu load efficiency*/
    x2    = (x2 * (int64_t)pCtx->gainMant) >> (47U - pCtx->gainExp);  /*cstat !MISRAC2012-Rule-10.1_R6 !MISRAC2012-Rule-1.3_n shift on signed integer for cpu load efficiency*/
    x1    = (x1 < -32768LL) ? -32768LL : ((x1 > 32767LL) ? 32767LL : x1);
    x2    = (x2 < -32768LL) ? -32768LL : ((x2 > 32767LL) ? 32767LL : x2);
    *pOut = (int16_t)x1;
    pOut++;
    *pOut = (int16_t)x2;
    pOut++;
  }
  #endif
}


static void s_biquadInt16ProcessNoResamplingMultiChannels(biquadIntContext_t *const pCtx, void *const in, void *const out, int const nbSamplesIn)
{
  int                      const  nbChannels    = pCtx->nbChannels;
  biquadInt32_t     const *const  pBiquad       = pCtx->pBiquad;
  uint8_t                  const  nbBiquadCells = pBiquad->nbCells;
  int16_t                  const *pIn           = (int16_t const *)in;
  int16_t                        *pOut          = (int16_t *)out;
  biquadCellInt32_t const        *pBiquadCell;
  int32_t                        *pBiquadMem;
  int                             i, ch;
  uint8_t                         n;

  #ifdef BIQUAD_USE_INTRINSICS
  int32_t x, y;

  for (i = 0; i < nbSamplesIn; i++)
  {
    for (ch = 0; ch < nbChannels; ch++)
    {
      x = (((int32_t)(*pIn)) << 16U);   /*cstat !MISRAC2012-Rule-10.1_R6 shift on signed integer for cpu load efficiency*/
      pIn++;
      if (nbBiquadCells > 0U)
      {
        pBiquadCell = pBiquad->pBiquadCell;
        pBiquadMem  = &pCtx->pBiquadMem[((2 * (int)pBiquad->nbCells) + 2) * ch];
        for (n = 0U; n < nbBiquadCells; n++)
        {
          y             = __SMMULR(pBiquadCell->b0, x);
          y             = __SMMLAR(pBiquadCell->b1, pBiquadMem[0], y);
          y             = __SMMLAR(pBiquadCell->b2, pBiquadMem[1], y);
          y             = __SMMLSR(pBiquadCell->a1, pBiquadMem[2], y);  // a1 is Q30 whereas other coefs are Q31
          y             = __SMMLSR(pBiquadCell->a1, pBiquadMem[2], y);  // a1 is Q30 whereas other coefs are Q31
          y             = __SMMLSR(pBiquadCell->a2, pBiquadMem[3], y);
          y             = __QADD(y, y);
          pBiquadMem[1] = pBiquadMem[0];
          pBiquadMem[0] = x;
          x             = y;
          pBiquadMem   += 2;
          pBiquadCell++;
        }
        pBiquadMem[1] = pBiquadMem[0];
        pBiquadMem[0] = x;
      }
      x     = __SMMULR(x, pCtx->gainMant) >> (15U - pCtx->gainExp);   /*cstat !MISRAC2012-Rule-10.1_R6 !MISRAC2012-Rule-1.3_n shift on signed integer for cpu load efficiency*/
      x     = __SSAT(x, 16);
      *pOut = (int16_t)x;
      pOut++;
    }
  }
  #else
  int64_t x, y;

  for (i = 0; i < nbSamplesIn; i++)
  {
    for (ch = 0; ch < nbChannels; ch++)
    {
      x = (((int64_t)(*pIn)) << 16U);   /*cstat !MISRAC2012-Rule-10.1_R6 shift on signed integer for cpu load efficiency*/
      pIn++;
      if (nbBiquadCells > 0U)
      {
        pBiquadCell = pBiquad->pBiquadCell;
        pBiquadMem  = &pCtx->pBiquadMem[((2 * (int)pBiquad->nbCells) + 2) * ch];
        for (n = 0U; n < nbBiquadCells; n++)
        {
          y             = (int64_t)pBiquadCell->b0 * x;
          y            += (int64_t)pBiquadCell->b1 * (int64_t)pBiquadMem[0];
          y            += (int64_t)pBiquadCell->b2 * (int64_t)pBiquadMem[1];
          y            -= (int64_t)pBiquadCell->a1 * (int64_t)pBiquadMem[2] * 2LL;  // a1 is Q30 whereas other coefs are Q31
          y            -= (int64_t)pBiquadCell->a2 * (int64_t)pBiquadMem[3];
          y           >>= 31U;    /*cstat !MISRAC2012-Rule-10.1_R6 !MISRAC2012-Rule-1.3_n shift on signed integer for cpu load efficiency*/
          y             = (y < -2147483648LL) ? -2147483648LL : ((y > 2147483647LL) ? 2147483647LL : y);
          pBiquadMem[1] = pBiquadMem[0];
          pBiquadMem[0] = (int32_t)x;
          x            = y;
          pBiquadMem   += 2;
          pBiquadCell++;
        }
        pBiquadMem[1] = pBiquadMem[0];
        pBiquadMem[0] = (int32_t)x;
      }
      x             = (x * (int64_t)pCtx->gainMant) >> (47U - pCtx->gainExp);   /*cstat !MISRAC2012-Rule-10.1_R6 !MISRAC2012-Rule-1.3_n shift on signed integer for cpu load efficiency*/
      x             = (x < -32768LL) ? -32768LL : ((x > 32767LL) ? 32767LL : x);
      *pOut         = (int16_t)x;
      pOut++;
    }
  }
  #endif
}


static void s_biquadInt16ProcessResamplingMono(biquadIntContext_t *const pCtx, void *const in, void *const out, int const nbSamplesIn)
{
  int                             decimFactor      = pCtx->decimFactor;
  int                             upSamplingFactor = pCtx->upSamplingFactor;
  biquadInt32_t     const *const  pBiquad          = pCtx->pBiquad;
  uint8_t                  const  nbBiquadCells    = pBiquad->nbCells;
  int16_t                  const *pIn              = (int16_t const *)in;
  int16_t                        *pOut             = (int16_t *)out;
  biquadCellInt32_t const        *pBiquadCell;
  int32_t                        *pBiquadMem;
  int                             nbSamplesOut, i, j, k;
  uint8_t                         n;

  decimFactor      = (decimFactor      == 0) ? 1 : decimFactor;       // MISRAC: division by 0 (in fact can't happen)
  upSamplingFactor = (upSamplingFactor == 0) ? 1 : upSamplingFactor;  // MISRAC: division by 0 (in fact can't happen)
  nbSamplesOut     = (nbSamplesIn / decimFactor) * upSamplingFactor;

  assert(((nbSamplesOut / upSamplingFactor) * decimFactor) == nbSamplesIn);

  #ifdef BIQUAD_USE_INTRINSICS
  int32_t x, y;

  k = 0;
  x = 0;
  for (i = 0; i < nbSamplesOut; i++)
  {
    for (j = 0; j < decimFactor; j++)
    {
      if (k == 0)
      {
        x = (((int32_t)(*pIn)) << 16U);   /*cstat !MISRAC2012-Rule-10.1_R6 shift on signed integer for cpu load efficiency*/
        pIn++;
      }
      else
      {
        x = 0;                                                        // insert zeros for upsampling
      }
      k = ((k + 1) == upSamplingFactor) ? 0 : (k + 1);
      if (nbBiquadCells > 0U)
      {
        pBiquadCell = pBiquad->pBiquadCell;
        pBiquadMem  = pCtx->pBiquadMem;
        for (n = 0U; n < nbBiquadCells; n++)
        {
          y             = __SMMULR(pBiquadCell->b0, x);
          y             = __SMMLAR(pBiquadCell->b1, pBiquadMem[0], y);
          y             = __SMMLAR(pBiquadCell->b2, pBiquadMem[1], y);
          y             = __SMMLSR(pBiquadCell->a1, pBiquadMem[2], y);  // a1 is Q30 whereas other coefs are Q31
          y             = __SMMLSR(pBiquadCell->a1, pBiquadMem[2], y);  // a1 is Q30 whereas other coefs are Q31
          y             = __SMMLSR(pBiquadCell->a2, pBiquadMem[3], y);
          y             = __QADD(y, y);
          pBiquadMem[1] = pBiquadMem[0];
          pBiquadMem[0] = x;
          x             = y;
          pBiquadMem   += 2;
          pBiquadCell++;
        }
        pBiquadMem[1] = pBiquadMem[0];
        pBiquadMem[0] = x;
      }
    }
    x     = __SMMULR(x, pCtx->gainMant) >> (15U - pCtx->gainExp);   /*cstat !MISRAC2012-Rule-10.1_R6 !MISRAC2012-Rule-1.3_n shift on signed integer for cpu load efficiency*/
    x     = __SSAT(x, 16);
    *pOut = (int16_t)x;
    pOut++;
  }
  #else
  int64_t x, y;

  k = 0;
  x = 0LL;
  for (i = 0; i < nbSamplesOut; i++)
  {
    for (j = 0; j < decimFactor; j++)
    {
      if (k == 0)
      {
        x = (((int64_t)(*pIn)) << 16U);     /*cstat !MISRAC2012-Rule-10.1_R6 shift on signed integer for cpu load efficiency*/
        pIn++;
      }
      else
      {
        x = 0LL;                                                                  // insert zeros for upsampling
      }
      k = ((k + 1) == upSamplingFactor) ? 0 : (k + 1);
      if (nbBiquadCells > 0U)
      {
        pBiquadCell = pBiquad->pBiquadCell;
        pBiquadMem  = pCtx->pBiquadMem;
        for (n = 0U; n < nbBiquadCells; n++)
        {
          y             = (int64_t)pBiquadCell->b0 * x;
          y            += (int64_t)pBiquadCell->b1 * (int64_t)pBiquadMem[0];
          y            += (int64_t)pBiquadCell->b2 * (int64_t)pBiquadMem[1];
          y            -= (int64_t)pBiquadCell->a1 * (int64_t)pBiquadMem[2] * 2LL;  // a1 is Q30 whereas other coefs are Q31
          y            -= (int64_t)pBiquadCell->a2 * (int64_t)pBiquadMem[3];
          y           >>= 31U;  /*cstat !MISRAC2012-Rule-10.1_R6 !MISRAC2012-Rule-1.3_n shift on signed integer for cpu load efficiency*/
          y             = (y < -2147483648LL) ? -2147483648LL : ((y > 2147483647LL) ? 2147483647LL : y);
          pBiquadMem[1] = pBiquadMem[0];
          pBiquadMem[0] = (int32_t)x;
          x             = y;
          pBiquadMem   += 2;
          pBiquadCell++;
        }
        pBiquadMem[1] = pBiquadMem[0];
        pBiquadMem[0] = (int32_t)x;
      }
    }
    x     = (x * (int64_t)pCtx->gainMant) >> (47U - pCtx->gainExp); /*cstat !MISRAC2012-Rule-10.1_R6 !MISRAC2012-Rule-1.3_n shift on signed integer for cpu load efficiency*/
    x     = (x < -32768LL) ? -32768LL : ((x > 32767LL) ? 32767LL : x);
    *pOut = (int16_t)x;
    pOut++;
  }
  #endif
}


static void s_biquadInt16ProcessResamplingStereo(biquadIntContext_t *const pCtx, void *const in, void *const out, int const nbSamplesIn)
{
  int                             decimFactor      = pCtx->decimFactor;
  int                             upSamplingFactor = pCtx->upSamplingFactor;
  biquadInt32_t     const *const  pBiquad          = pCtx->pBiquad;
  uint8_t                  const  nbBiquadCells    = pBiquad->nbCells;
  int16_t                  const *pIn              = (int16_t const *)in;
  int16_t                        *pOut             = (int16_t *)out;
  biquadCellInt32_t const        *pBiquadCell;
  int32_t                        *pBiquadMem;
  int                             nbSamplesOut, i, j, k;
  uint8_t                         n;

  decimFactor      = (decimFactor      == 0) ? 1 : decimFactor;       // MISRAC: division by 0 (in fact can't happen)
  upSamplingFactor = (upSamplingFactor == 0) ? 1 : upSamplingFactor;  // MISRAC: division by 0 (in fact can't happen)
  nbSamplesOut     = (nbSamplesIn / decimFactor) * upSamplingFactor;

  assert(((nbSamplesOut / upSamplingFactor) * decimFactor) == nbSamplesIn);

  #ifdef BIQUAD_USE_INTRINSICS
  int32_t x1, x2, y1, y2;

  k  = 0;
  x1 = 0;
  x2 = 0;
  for (i = 0; i < nbSamplesOut; i++)
  {
    for (j = 0; j < decimFactor; j++)
    {
      if (k == 0)
      {
        x1 = (((int32_t)(*pIn)) << 16U);  /*cstat !MISRAC2012-Rule-10.1_R6 shift on signed integer for cpu load efficiency*/
        pIn++;
        x2 = (((int32_t)(*pIn)) << 16U);  /*cstat !MISRAC2012-Rule-10.1_R6 shift on signed integer for cpu load efficiency*/
        pIn++;
      }
      else
      {
        x1 = 0;                                                       // insert zeros for upsampling
        x2 = 0;                                                       // insert zeros for upsampling
      }
      k = ((k + 1) == upSamplingFactor) ? 0 : (k + 1);
      if (nbBiquadCells > 0U)
      {
        pBiquadCell = pBiquad->pBiquadCell;
        pBiquadMem  = pCtx->pBiquadMem;
        for (n = 0U; n < nbBiquadCells; n++)
        {
          y1            = __SMMULR(pBiquadCell->b0, x1);
          y2            = __SMMULR(pBiquadCell->b0, x2);
          y1            = __SMMLAR(pBiquadCell->b1, pBiquadMem[0], y1);
          y2            = __SMMLAR(pBiquadCell->b1, pBiquadMem[1], y2);
          y1            = __SMMLAR(pBiquadCell->b2, pBiquadMem[2], y1);
          y2            = __SMMLAR(pBiquadCell->b2, pBiquadMem[3], y2);
          y1            = __SMMLSR(pBiquadCell->a1, pBiquadMem[4], y1); // a1 is Q30 whereas other coefs are Q31
          y1            = __SMMLSR(pBiquadCell->a1, pBiquadMem[4], y1); // a1 is Q30 whereas other coefs are Q31
          y2            = __SMMLSR(pBiquadCell->a1, pBiquadMem[5], y2); // a1 is Q30 whereas other coefs are Q31
          y2            = __SMMLSR(pBiquadCell->a1, pBiquadMem[5], y2); // a1 is Q30 whereas other coefs are Q31
          y1            = __SMMLSR(pBiquadCell->a2, pBiquadMem[6], y1);
          y2            = __SMMLSR(pBiquadCell->a2, pBiquadMem[7], y2);
          y1            = __QADD(y1, y1);
          y2            = __QADD(y2, y2);
          pBiquadMem[2] = pBiquadMem[0];
          pBiquadMem[3] = pBiquadMem[1];
          pBiquadMem[0] = x1;
          pBiquadMem[1] = x2;
          x1            = y1;
          x2            = y2;
          pBiquadMem   += 4;
          pBiquadCell++;
        }
        pBiquadMem[2] = pBiquadMem[0];
        pBiquadMem[3] = pBiquadMem[1];
        pBiquadMem[0] = x1;
        pBiquadMem[1] = x2;
      }
    }
    x1    = __SMMULR(x1, pCtx->gainMant) >> (15U - pCtx->gainExp);  /*cstat !MISRAC2012-Rule-10.1_R6 !MISRAC2012-Rule-1.3_n shift on signed integer for cpu load efficiency*/
    x2    = __SMMULR(x2, pCtx->gainMant) >> (15U - pCtx->gainExp);  /*cstat !MISRAC2012-Rule-10.1_R6 !MISRAC2012-Rule-1.3_n shift on signed integer for cpu load efficiency*/
    x1    = __SSAT(x1, 16);
    x2    = __SSAT(x2, 16);
    *pOut = (int16_t)x1;
    pOut++;
    *pOut = (int16_t)x2;
    pOut++;
  }
  #else
  int64_t x1, x2, y1, y2;

  k  = 0;
  x1 = 0LL;
  x2 = 0LL;
  for (i = 0; i < nbSamplesOut; i++)
  {
    for (j = 0; j < decimFactor; j++)
    {
      if (k == 0)
      {
        x1 = (((int64_t)(*pIn)) << 16U);  /*cstat !MISRAC2012-Rule-10.1_R6 shift on signed integer for cpu load efficiency*/
        pIn++;
        x2 = (((int64_t)(*pIn)) << 16U);  /*cstat !MISRAC2012-Rule-10.1_R6 shift on signed integer for cpu load efficiency*/
        pIn++;
      }
      else
      {
        x1 = 0LL;  // insert zeros for upsampling
        x2 = 0LL;  // insert zeros for upsampling
      }
      k = ((k + 1) == upSamplingFactor) ? 0 : (k + 1);
      if (nbBiquadCells > 0U)
      {
        pBiquadCell = pBiquad->pBiquadCell;
        pBiquadMem  = pCtx->pBiquadMem;
        for (n = 0U; n < nbBiquadCells; n++)
        {
          y1            = (int64_t)pBiquadCell->b0 * x1;
          y2            = (int64_t)pBiquadCell->b0 * x2;
          y1           += (int64_t)pBiquadCell->b1 * (int64_t)pBiquadMem[0];
          y2           += (int64_t)pBiquadCell->b1 * (int64_t)pBiquadMem[1];
          y1           += (int64_t)pBiquadCell->b2 * (int64_t)pBiquadMem[2];
          y2           += (int64_t)pBiquadCell->b2 * (int64_t)pBiquadMem[3];
          y1           -= (int64_t)pBiquadCell->a1 * (int64_t)pBiquadMem[4] * 2LL;  // a1 is Q30 whereas other coefs are Q31
          y2           -= (int64_t)pBiquadCell->a1 * (int64_t)pBiquadMem[5] * 2LL;  // a1 is Q30 whereas other coefs are Q31
          y1           -= (int64_t)pBiquadCell->a2 * (int64_t)pBiquadMem[6];
          y2           -= (int64_t)pBiquadCell->a2 * (int64_t)pBiquadMem[7];
          y1          >>= 31U;  /*cstat !MISRAC2012-Rule-10.1_R6 !MISRAC2012-Rule-1.3_n shift on signed integer for cpu load efficiency*/
          y2          >>= 31U;  /*cstat !MISRAC2012-Rule-10.1_R6 !MISRAC2012-Rule-1.3_n shift on signed integer for cpu load efficiency*/
          y1            = (y1 < -2147483648LL) ? -2147483648LL : ((y1 > 2147483647LL) ? 2147483647LL : y1);
          y2            = (y2 < -2147483648LL) ? -2147483648LL : ((y2 > 2147483647LL) ? 2147483647LL : y2);
          pBiquadMem[2] = pBiquadMem[0];
          pBiquadMem[3] = pBiquadMem[1];
          pBiquadMem[0] = (int32_t)x1;
          pBiquadMem[1] = (int32_t)x2;
          x1            = y1;
          x2            = y2;
          pBiquadMem   += 4;
          pBiquadCell++;
        }
        pBiquadMem[2] = pBiquadMem[0];
        pBiquadMem[3] = pBiquadMem[1];
        pBiquadMem[0] = (int32_t)x1;
        pBiquadMem[1] = (int32_t)x2;
      }
    }
    x1    = (x1 * (int64_t)pCtx->gainMant) >> (47U - pCtx->gainExp);  /*cstat !MISRAC2012-Rule-10.1_R6 !MISRAC2012-Rule-1.3_n shift on signed integer for cpu load efficiency*/
    x2    = (x2 * (int64_t)pCtx->gainMant) >> (47U - pCtx->gainExp);  /*cstat !MISRAC2012-Rule-10.1_R6 !MISRAC2012-Rule-1.3_n shift on signed integer for cpu load efficiency*/
    x1    = (x1 < -32768LL) ? -32768LL : ((x1 > 32767LL) ? 32767LL : x1);
    x2    = (x2 < -32768LL) ? -32768LL : ((x2 > 32767LL) ? 32767LL : x2);
    *pOut = (int16_t)x1;
    pOut++;
    *pOut = (int16_t)x2;
    pOut++;
  }
  #endif
}
#endif /* BIQUAD_CODE_SIZE_OPTIM */


static void s_biquadInt16ProcessResamplingMultiChannels(biquadIntContext_t *const pCtx, void *const in, void *const out, int const nbSamplesIn)
{
  int                             decimFactor      = pCtx->decimFactor;
  int                             upSamplingFactor = pCtx->upSamplingFactor;
  int                      const  nbChannels       = pCtx->nbChannels;
  biquadInt32_t     const *const  pBiquad          = pCtx->pBiquad;
  uint8_t                  const  nbBiquadCells    = pBiquad->nbCells;
  int16_t                  const *pIn              = (int16_t const *)in;
  int16_t                        *pOut             = (int16_t *)out;
  int16_t                  const *pInSave          = pIn;
  biquadCellInt32_t const        *pBiquadCell;
  int32_t                        *pBiquadMem;
  int                             nbSamplesOut, i, j, ch, k, kSave;
  uint8_t                         n;

  decimFactor      = (decimFactor      == 0) ? 1 : decimFactor;       // MISRAC: division by 0 (in fact can't happen)
  upSamplingFactor = (upSamplingFactor == 0) ? 1 : upSamplingFactor;  // MISRAC: division by 0 (in fact can't happen)
  nbSamplesOut     = (nbSamplesIn / decimFactor) * upSamplingFactor;

  assert(((nbSamplesOut / upSamplingFactor) * decimFactor) == nbSamplesIn);

  #ifdef BIQUAD_USE_INTRINSICS
  int32_t x, y;

  k = 0;
  x = 0;
  for (i = 0; i < nbSamplesOut; i++)
  {
    kSave = k;
    for (ch = 0; ch < nbChannels; ch++)
    {
      pIn = &pInSave[ch];
      k   = kSave;
      for (j = 0; j < decimFactor; j++)
      {
        if (k == 0)
        {
          x    = (((int32_t)(*pIn)) << 16U);  /*cstat !MISRAC2012-Rule-10.1_R6 shift on signed integer for cpu load efficiency*/
          pIn += nbChannels;
        }
        else
        {
          x = 0;                                                        // insert zeros for upsampling
        }
        k = ((k + 1) == upSamplingFactor) ? 0 : (k + 1);
        if (nbBiquadCells > 0U)
        {
          pBiquadCell = pBiquad->pBiquadCell;
          pBiquadMem  = &pCtx->pBiquadMem[((2 * (int)pBiquad->nbCells) + 2) * ch];
          for (n = 0U; n < nbBiquadCells; n++)
          {
            y             = __SMMULR(pBiquadCell->b0, x);
            y             = __SMMLAR(pBiquadCell->b1, pBiquadMem[0], y);
            y             = __SMMLAR(pBiquadCell->b2, pBiquadMem[1], y);
            y             = __SMMLSR(pBiquadCell->a1, pBiquadMem[2], y);  // a1 is Q30 whereas other coefs are Q31
            y             = __SMMLSR(pBiquadCell->a1, pBiquadMem[2], y);  // a1 is Q30 whereas other coefs are Q31
            y             = __SMMLSR(pBiquadCell->a2, pBiquadMem[3], y);
            y             = __QADD(y, y);
            pBiquadMem[1] = pBiquadMem[0];
            pBiquadMem[0] = x;
            x             = y;
            pBiquadMem   += 2;
            pBiquadCell++;
          }
          pBiquadMem[1] = pBiquadMem[0];
          pBiquadMem[0] = x;
        }
      }
      x     = __SMMULR(x, pCtx->gainMant) >> (15U - pCtx->gainExp);   /*cstat !MISRAC2012-Rule-10.1_R6 !MISRAC2012-Rule-1.3_n shift on signed integer for cpu load efficiency*/
      x     = __SSAT(x, 16);
      *pOut = (int16_t)x;
      pOut++;
    }
    pInSave = &pIn[1 - nbChannels];   // next ch 0 input sample
  }
  #else
  int64_t x, y;

  k = 0;
  x = 0LL;
  for (i = 0; i < nbSamplesOut; i++)
  {
    kSave = k;
    for (ch = 0; ch < nbChannels; ch++)
    {
      pIn = &pInSave[ch];
      k   = kSave;
      for (j = 0; j < decimFactor; j++)
      {
        if (k == 0)
        {
          x    = (((int64_t)(*pIn)) << 16U);  /*cstat !MISRAC2012-Rule-10.1_R6 shift on signed integer for cpu load efficiency*/
          pIn += nbChannels;
        }
        else
        {
          x = 0LL;  // insert zeros for upsampling
        }
        k = ((k + 1) == upSamplingFactor) ? 0 : (k + 1);
        if (nbBiquadCells > 0U)
        {
          pBiquadCell = pBiquad->pBiquadCell;
          pBiquadMem  = &pCtx->pBiquadMem[((2 * (int)pBiquad->nbCells) + 2) * ch];
          for (n = 0U; n < nbBiquadCells; n++)
          {
            y             = (int64_t)pBiquadCell->b0 * x;
            y            += (int64_t)pBiquadCell->b1 * (int64_t)pBiquadMem[0];
            y            += (int64_t)pBiquadCell->b2 * (int64_t)pBiquadMem[1];
            y            -= (int64_t)pBiquadCell->a1 * (int64_t)pBiquadMem[2] * 2LL;  // a1 is Q30 whereas other coefs are Q31
            y            -= (int64_t)pBiquadCell->a2 * (int64_t)pBiquadMem[3];
            y           >>= 31U;  /*cstat !MISRAC2012-Rule-10.1_R6 !MISRAC2012-Rule-1.3_n shift on signed integer for cpu load efficiency*/
            y             = (y < -2147483648LL) ? -2147483648LL : ((y > 2147483647LL) ? 2147483647LL : y);
            pBiquadMem[1] = pBiquadMem[0];
            pBiquadMem[0] = (int32_t)x;
            x             = y;
            pBiquadMem    += 2;
            pBiquadCell++;
          }
          pBiquadMem[1] = pBiquadMem[0];
          pBiquadMem[0] = (int32_t)x;
        }
      }
      x     = (x * (int64_t)pCtx->gainMant) >> (47U - pCtx->gainExp);   /*cstat !MISRAC2012-Rule-10.1_R6 !MISRAC2012-Rule-1.3_n shift on signed integer for cpu load efficiency*/
      x     = (x < -32768LL) ? -32768LL : ((x > 32767LL) ? 32767LL : x);
      *pOut = (int16_t)x;
      pOut++;
    }
    pInSave = &pIn[1 - nbChannels];   // next ch 0 input sample
  }
  #endif
}


#ifndef BIQUAD_CODE_SIZE_OPTIM
static void s_biquadInt32ProcessNoResamplingMono(biquadIntContext_t *const pCtx, void *const in, void *const out, int const nbSamplesIn)
{
  biquadInt32_t     const *const  pBiquad       = pCtx->pBiquad;
  uint8_t                  const  nbBiquadCells = pBiquad->nbCells;
  int32_t                  const *pIn           = (int32_t const *)in;
  int32_t                        *pOut          = (int32_t *)out;
  biquadCellInt32_t const        *pBiquadCell;
  int32_t                        *pBiquadMem;
  int                             i;
  uint8_t                         n;

  #ifdef BIQUAD_USE_INTRINSICS
  int32_t x, y;

  for (i = 0; i < nbSamplesIn; i++)
  {
    x = *pIn;
    pIn++;
    if (nbBiquadCells > 0U)
    {
      pBiquadCell = pBiquad->pBiquadCell;
      pBiquadMem  = pCtx->pBiquadMem;
      for (n = 0U; n < nbBiquadCells; n++)
      {
        y             = __SMMULR(pBiquadCell->b0, x);
        y             = __SMMLAR(pBiquadCell->b1, pBiquadMem[0], y);
        y             = __SMMLAR(pBiquadCell->b2, pBiquadMem[1], y);
        y             = __SMMLSR(pBiquadCell->a1, pBiquadMem[2], y);  // a1 is Q30 whereas other coefs are Q31
        y             = __SMMLSR(pBiquadCell->a1, pBiquadMem[2], y);  // a1 is Q30 whereas other coefs are Q31
        y             = __SMMLSR(pBiquadCell->a2, pBiquadMem[3], y);
        y             = __QADD(y, y);
        pBiquadMem[1] = pBiquadMem[0];
        pBiquadMem[0] = x;
        x             = y;
        pBiquadMem   += 2;
        pBiquadCell++;
      }
      pBiquadMem[1] = pBiquadMem[0];
      pBiquadMem[0] = x;
    }
    x     = __SMMULR(x, pCtx->gainMant) << pCtx->gainExp; /*cstat !MISRAC2012-Rule-10.1_R6 shift on signed integer for cpu load efficiency*/
    x     = __QADD(x, x);
    *pOut = x;
    pOut++;
  }
  #else
  int64_t x, y;

  for (i = 0; i < nbSamplesIn; i++)
  {
    x = (int64_t)(*pIn);
    pIn++;
    if (nbBiquadCells > 0U)
    {
      pBiquadCell = pBiquad->pBiquadCell;
      pBiquadMem  = pCtx->pBiquadMem;
      for (n = 0U; n < nbBiquadCells; n++)
      {
        y             = (int64_t)pBiquadCell->b0 * x;
        y            += (int64_t)pBiquadCell->b1 * (int64_t)pBiquadMem[0];
        y            += (int64_t)pBiquadCell->b2 * (int64_t)pBiquadMem[1];
        y            -= (int64_t)pBiquadCell->a1 * (int64_t)pBiquadMem[2] * 2LL;
        y            -= (int64_t)pBiquadCell->a2 * (int64_t)pBiquadMem[3];
        y           >>= 31U;  /*cstat !MISRAC2012-Rule-10.1_R6 !MISRAC2012-Rule-1.3_n shift on signed integer for cpu load efficiency*/
        y             = (y < -2147483648LL) ? -2147483648LL : ((y > 2147483647LL) ? 2147483647LL : y);
        pBiquadMem[1] = pBiquadMem[0];
        pBiquadMem[0] = (int32_t)x;
        x             = y;
        pBiquadMem   += 2;
        pBiquadCell++;
      }
      pBiquadMem[1] = pBiquadMem[0];
      pBiquadMem[0] = (int32_t)x;
    }
    x     = (x * (int64_t)pCtx->gainMant) >> (31U - pCtx->gainExp); /*cstat !MISRAC2012-Rule-10.1_R6 !MISRAC2012-Rule-1.3_n shift on signed integer for cpu load efficiency*/
    x     = (x < -2147483648LL) ? -2147483648LL : ((x > 2147483647LL) ? 2147483647LL : x);
    *pOut = (int32_t)x;
    pOut++;
  }
  #endif
}


static void s_biquadInt32ProcessNoResamplingStereo(biquadIntContext_t *const pCtx, void *const in, void *const out, int const nbSamplesIn)
{
  biquadInt32_t     const *const  pBiquad       = pCtx->pBiquad;
  uint8_t                  const  nbBiquadCells = pBiquad->nbCells;
  int32_t                  const *pIn           = (int32_t const *)in;
  int32_t                        *pOut          = (int32_t *)out;
  biquadCellInt32_t const        *pBiquadCell;
  int32_t                        *pBiquadMem;
  int                             i;
  uint8_t                         n;

  #ifdef BIQUAD_USE_INTRINSICS
  int32_t x1, x2, y1, y2;

  for (i = 0; i < nbSamplesIn; i++)
  {
    x1 = *pIn;
    pIn++;
    x2 = *pIn;
    pIn++;
    if (nbBiquadCells > 0U)
    {
      pBiquadCell = pBiquad->pBiquadCell;
      pBiquadMem  = pCtx->pBiquadMem;
      for (n = 0U; n < nbBiquadCells; n++)
      {
        y1            = __SMMULR(pBiquadCell->b0, x1);
        y2            = __SMMULR(pBiquadCell->b0, x2);
        y1            = __SMMLAR(pBiquadCell->b1, pBiquadMem[0], y1);
        y2            = __SMMLAR(pBiquadCell->b1, pBiquadMem[1], y2);
        y1            = __SMMLAR(pBiquadCell->b2, pBiquadMem[2], y1);
        y2            = __SMMLAR(pBiquadCell->b2, pBiquadMem[3], y2);
        y1            = __SMMLSR(pBiquadCell->a1, pBiquadMem[4], y1); // a1 is Q30 whereas other coefs are Q31
        y1            = __SMMLSR(pBiquadCell->a1, pBiquadMem[4], y1); // a1 is Q30 whereas other coefs are Q31
        y2            = __SMMLSR(pBiquadCell->a1, pBiquadMem[5], y2); // a1 is Q30 whereas other coefs are Q31
        y2            = __SMMLSR(pBiquadCell->a1, pBiquadMem[5], y2); // a1 is Q30 whereas other coefs are Q31
        y1            = __SMMLSR(pBiquadCell->a2, pBiquadMem[6], y1);
        y2            = __SMMLSR(pBiquadCell->a2, pBiquadMem[7], y2);
        y1            = __QADD(y1, y1);
        y2            = __QADD(y2, y2);
        pBiquadMem[2] = pBiquadMem[0];
        pBiquadMem[3] = pBiquadMem[1];
        pBiquadMem[0] = x1;
        pBiquadMem[1] = x2;
        x1            = y1;
        x2            = y2;
        pBiquadMem   += 4;
        pBiquadCell++;
      }
      pBiquadMem[2] = pBiquadMem[0];
      pBiquadMem[3] = pBiquadMem[1];
      pBiquadMem[0] = x1;
      pBiquadMem[1] = x2;
    }
    x1    = __SMMULR(x1, pCtx->gainMant) << pCtx->gainExp;  /*cstat !MISRAC2012-Rule-10.1_R6 shift on signed integer for cpu load efficiency*/
    x2    = __SMMULR(x2, pCtx->gainMant) << pCtx->gainExp;  /*cstat !MISRAC2012-Rule-10.1_R6 shift on signed integer for cpu load efficiency*/
    x1    = __QADD(x1, x1);
    x2    = __QADD(x2, x2);
    *pOut = x1;
    pOut++;
    *pOut = x2;
    pOut++;
  }
  #else
  int64_t x1, x2, y1, y2;

  for (i = 0; i < nbSamplesIn; i++)
  {
    x1 = (int64_t)(*pIn);
    pIn++;
    x2 = (int64_t)(*pIn);
    pIn++;
    if (nbBiquadCells > 0U)
    {
      pBiquadCell = pBiquad->pBiquadCell;
      pBiquadMem  = pCtx->pBiquadMem;
      for (n = 0U; n < nbBiquadCells; n++)
      {
        y1            = (int64_t)pBiquadCell->b0 * x1;
        y2            = (int64_t)pBiquadCell->b0 * x2;
        y1           += (int64_t)pBiquadCell->b1 * (int64_t)pBiquadMem[0];
        y2           += (int64_t)pBiquadCell->b1 * (int64_t)pBiquadMem[1];
        y1           += (int64_t)pBiquadCell->b2 * (int64_t)pBiquadMem[2];
        y2           += (int64_t)pBiquadCell->b2 * (int64_t)pBiquadMem[3];
        y1           -= (int64_t)pBiquadCell->a1 * (int64_t)pBiquadMem[4] * 2LL;
        y2           -= (int64_t)pBiquadCell->a1 * (int64_t)pBiquadMem[5] * 2LL;
        y1           -= (int64_t)pBiquadCell->a2 * (int64_t)pBiquadMem[6];
        y2           -= (int64_t)pBiquadCell->a2 * (int64_t)pBiquadMem[7];
        y1          >>= 31U;  /*cstat !MISRAC2012-Rule-10.1_R6 !MISRAC2012-Rule-1.3_n shift on signed integer for cpu load efficiency*/
        y2          >>= 31U;  /*cstat !MISRAC2012-Rule-10.1_R6 !MISRAC2012-Rule-1.3_n shift on signed integer for cpu load efficiency*/
        y1            = (y1 < -2147483648LL) ? -2147483648LL : ((y1 > 2147483647LL) ? 2147483647LL : y1);
        y2            = (y2 < -2147483648LL) ? -2147483648LL : ((y2 > 2147483647LL) ? 2147483647LL : y2);
        pBiquadMem[2] = pBiquadMem[0];
        pBiquadMem[3] = pBiquadMem[1];
        pBiquadMem[0] = (int32_t)x1;
        pBiquadMem[1] = (int32_t)x2;
        x1            = y1;
        x2            = y2;
        pBiquadMem   += 4;
        pBiquadCell++;
      }
      pBiquadMem[2] = pBiquadMem[0];
      pBiquadMem[3] = pBiquadMem[1];
      pBiquadMem[0] = (int32_t)x1;
      pBiquadMem[1] = (int32_t)x2;
    }
    x1    = (x1 * (int64_t)pCtx->gainMant) >> (31U - pCtx->gainExp);  /*cstat !MISRAC2012-Rule-10.1_R6 !MISRAC2012-Rule-1.3_n shift on signed integer for cpu load efficiency*/
    x2    = (x2 * (int64_t)pCtx->gainMant) >> (31U - pCtx->gainExp);  /*cstat !MISRAC2012-Rule-10.1_R6 !MISRAC2012-Rule-1.3_n shift on signed integer for cpu load efficiency*/
    x1    = (x1 < -2147483648LL) ? -2147483648LL : ((x1 > 2147483647LL) ? 2147483647LL : x1);
    x2    = (x2 < -2147483648LL) ? -2147483648LL : ((x2 > 2147483647LL) ? 2147483647LL : x2);
    *pOut = (int32_t)x1;
    pOut++;
    *pOut = (int32_t)x2;
    pOut++;
  }
  #endif
}


static void s_biquadInt32ProcessNoResamplingMultiChannels(biquadIntContext_t *const pCtx, void *const in, void *const out, int const nbSamplesIn)
{
  int                      const  nbChannels    = pCtx->nbChannels;
  biquadInt32_t     const *const  pBiquad       = pCtx->pBiquad;
  uint8_t                  const  nbBiquadCells = pBiquad->nbCells;
  int32_t                  const *pIn           = (int32_t const *)in;
  int32_t                        *pOut          = (int32_t *)out;
  biquadCellInt32_t const        *pBiquadCell;
  int32_t                        *pBiquadMem;
  int                             i, ch;
  uint8_t                         n;

  #ifdef BIQUAD_USE_INTRINSICS
  int32_t x, y;

  for (i = 0; i < nbSamplesIn; i++)
  {
    for (ch = 0; ch < nbChannels; ch++)
    {
      x = *pIn;
      pIn++;
      if (nbBiquadCells > 0U)
      {
        pBiquadCell = pBiquad->pBiquadCell;
        pBiquadMem  = &pCtx->pBiquadMem[((2 * (int)pBiquad->nbCells) + 2) * ch];
        for (n = 0U; n < nbBiquadCells; n++)
        {
          y             = __SMMULR(pBiquadCell->b0, x);
          y             = __SMMLAR(pBiquadCell->b1, pBiquadMem[0], y);
          y             = __SMMLAR(pBiquadCell->b2, pBiquadMem[1], y);
          y             = __SMMLSR(pBiquadCell->a1, pBiquadMem[2], y);  // a1 is Q30 whereas other coefs are Q31
          y             = __SMMLSR(pBiquadCell->a1, pBiquadMem[2], y);  // a1 is Q30 whereas other coefs are Q31
          y             = __SMMLSR(pBiquadCell->a2, pBiquadMem[3], y);
          y             = __QADD(y, y);
          pBiquadMem[1] = pBiquadMem[0];
          pBiquadMem[0] = x;
          x             = y;
          pBiquadMem   += 2;
          pBiquadCell++;
        }
        pBiquadMem[1] = pBiquadMem[0];
        pBiquadMem[0] = x;
      }
      x     = __SMMULR(x, pCtx->gainMant) << pCtx->gainExp; /*cstat !MISRAC2012-Rule-10.1_R6 shift on signed integer for cpu load efficiency*/
      x     = __QADD(x, x);
      *pOut = x;
      pOut++;
    }
  }
  #else
  int64_t x, y;

  for (i = 0; i < nbSamplesIn; i++)
  {
    for (ch = 0; ch < nbChannels; ch++)
    {
      x = (int64_t)(*pIn);
      pIn++;
      if (nbBiquadCells > 0U)
      {
        pBiquadCell = pBiquad->pBiquadCell;
        pBiquadMem  = &pCtx->pBiquadMem[((2 * (int)pBiquad->nbCells) + 2) * ch];
        for (n = 0U; n < nbBiquadCells; n++)
        {
          y             = (int64_t)pBiquadCell->b0 * x;
          y            += (int64_t)pBiquadCell->b1 * (int64_t)pBiquadMem[0];
          y            += (int64_t)pBiquadCell->b2 * (int64_t)pBiquadMem[1];
          y            -= (int64_t)pBiquadCell->a1 * (int64_t)pBiquadMem[2] * 2LL;  // a1 is Q30 whereas other coefs are Q31
          y            -= (int64_t)pBiquadCell->a2 * (int64_t)pBiquadMem[3];
          y           >>= 31U;  /*cstat !MISRAC2012-Rule-10.1_R6 !MISRAC2012-Rule-1.3_n shift on signed integer for cpu load efficiency*/
          y             = (y < -2147483648LL) ? -2147483648LL : ((y > 2147483647LL) ? 2147483647LL : y);
          pBiquadMem[1] = pBiquadMem[0];
          pBiquadMem[0] = (int32_t)x;
          x             = y;
          pBiquadMem    += 2;
          pBiquadCell++;
        }
        pBiquadMem[1] = pBiquadMem[0];
        pBiquadMem[0] = (int32_t)x;
      }
      x     = (x * (int64_t)pCtx->gainMant) >> (31U - pCtx->gainExp);   /*cstat !MISRAC2012-Rule-10.1_R6 !MISRAC2012-Rule-1.3_n shift on signed integer for cpu load efficiency*/
      x     = (x < -2147483648LL) ? -2147483648LL : ((x > 2147483647LL) ? 2147483647LL : x);
      *pOut = (int32_t)x;
      pOut++;
    }
  }
  #endif
}


static void s_biquadInt32ProcessResamplingMono(biquadIntContext_t *const pCtx, void *const in, void *const out, int const nbSamplesIn)
{
  int                             decimFactor      = pCtx->decimFactor;
  int                             upSamplingFactor = pCtx->upSamplingFactor;
  biquadInt32_t     const *const  pBiquad          = pCtx->pBiquad;
  uint8_t                  const  nbBiquadCells    = pBiquad->nbCells;
  int32_t                  const *pIn              = (int32_t const *)in;
  int32_t                        *pOut             = (int32_t *)out;
  biquadCellInt32_t const        *pBiquadCell;
  int32_t                        *pBiquadMem;
  int                             nbSamplesOut, i, j, k;
  uint8_t                         n;

  decimFactor      = (decimFactor      == 0) ? 1 : decimFactor;       // MISRAC: division by 0 (in fact can't happen)
  upSamplingFactor = (upSamplingFactor == 0) ? 1 : upSamplingFactor;  // MISRAC: division by 0 (in fact can't happen)
  nbSamplesOut     = (nbSamplesIn / decimFactor) * upSamplingFactor;

  assert(((nbSamplesOut / upSamplingFactor) * decimFactor) == nbSamplesIn);

  #ifdef BIQUAD_USE_INTRINSICS
  int32_t x, y;

  k = 0;
  x = 0;
  for (i = 0; i < nbSamplesOut; i++)
  {
    for (j = 0; j < decimFactor; j++)
    {
      if (k == 0)
      {
        x = *pIn;
        pIn++;
      }
      else
      {
        x = 0;                                                        // insert zeros for upsampling
      }
      k = ((k + 1) == upSamplingFactor) ? 0 : (k + 1);
      if (nbBiquadCells > 0U)
      {
        pBiquadCell = pBiquad->pBiquadCell;
        pBiquadMem  = pCtx->pBiquadMem;
        for (n = 0U; n < nbBiquadCells; n++)
        {
          y             = __SMMULR(pBiquadCell->b0, x);
          y             = __SMMLAR(pBiquadCell->b1, pBiquadMem[0], y);
          y             = __SMMLAR(pBiquadCell->b2, pBiquadMem[1], y);
          y             = __SMMLSR(pBiquadCell->a1, pBiquadMem[2], y);  // a1 is Q30 whereas other coefs are Q31
          y             = __SMMLSR(pBiquadCell->a1, pBiquadMem[2], y);  // a1 is Q30 whereas other coefs are Q31
          y             = __SMMLSR(pBiquadCell->a2, pBiquadMem[3], y);
          y             = __QADD(y, y);
          pBiquadMem[1] = pBiquadMem[0];
          pBiquadMem[0] = x;
          x             = y;
          pBiquadMem   += 2;
          pBiquadCell++;
        }
        pBiquadMem[1] = pBiquadMem[0];
        pBiquadMem[0] = x;
      }
    }
    x     = __SMMULR(x, pCtx->gainMant) << pCtx->gainExp;   /*cstat !MISRAC2012-Rule-10.1_R6 shift on signed integer for cpu load efficiency*/
    x     = __QADD(x, x);
    *pOut = x;
  }
  #else
  int64_t x, y;

  k = 0;
  x = 0LL;
  for (i = 0; i < nbSamplesOut; i++)
  {
    for (j = 0; j < decimFactor; j++)
    {
      if (k == 0)
      {
        x = (int64_t)(*pIn);
        pIn++;
      }
      else
      {
        x = 0LL;  // insert zeros for upsampling
      }
      k = ((k + 1) == upSamplingFactor) ? 0 : (k + 1);
      if (nbBiquadCells > 0U)
      {
        pBiquadCell = pBiquad->pBiquadCell;
        pBiquadMem  = pCtx->pBiquadMem;
        for (n = 0U; n < nbBiquadCells; n++)
        {
          y             = (int64_t)pBiquadCell->b0 * x;
          y            += (int64_t)pBiquadCell->b1 * (int64_t)pBiquadMem[0];
          y            += (int64_t)pBiquadCell->b2 * (int64_t)pBiquadMem[1];
          y            -= (int64_t)pBiquadCell->a1 * (int64_t)pBiquadMem[2] * 2LL;
          y            -= (int64_t)pBiquadCell->a2 * (int64_t)pBiquadMem[3];
          y           >>= 31U;  /*cstat !MISRAC2012-Rule-10.1_R6 !MISRAC2012-Rule-1.3_n shift on signed integer for cpu load efficiency*/
          y             = (y < -2147483648LL) ? -2147483648LL : ((y > 2147483647LL) ? 2147483647LL : y);
          pBiquadMem[1] = pBiquadMem[0];
          pBiquadMem[0] = (int32_t)x;
          x             = y;
          pBiquadMem   += 2;
          pBiquadCell++;
        }
        pBiquadMem[1] = pBiquadMem[0];
        pBiquadMem[0] = (int32_t)x;
      }
    }
    x     = (x * (int64_t)pCtx->gainMant) >> (31U - pCtx->gainExp);   /*cstat !MISRAC2012-Rule-10.1_R6 !MISRAC2012-Rule-1.3_n shift on signed integer for cpu load efficiency*/
    x     = (x < -2147483648LL) ? -2147483648LL : ((x > 2147483647LL) ? 2147483647LL : x);
    *pOut = (int32_t)x;
    pOut++;
  }
  #endif
}


static void s_biquadInt32ProcessResamplingStereo(biquadIntContext_t *const pCtx, void *const in, void *const out, int const nbSamplesIn)
{
  int                             decimFactor      = pCtx->decimFactor;
  int                             upSamplingFactor = pCtx->upSamplingFactor;
  biquadInt32_t     const *const  pBiquad          = pCtx->pBiquad;
  uint8_t                  const  nbBiquadCells    = pBiquad->nbCells;
  int32_t                  const *pIn              = (int32_t const *)in;
  int32_t                        *pOut             = (int32_t *)out;
  biquadCellInt32_t const        *pBiquadCell;
  int32_t                        *pBiquadMem;
  int                             nbSamplesOut, i, j, k;
  uint8_t                         n;

  decimFactor      = (decimFactor      == 0) ? 1 : decimFactor;       // MISRAC: division by 0 (in fact can't happen)
  upSamplingFactor = (upSamplingFactor == 0) ? 1 : upSamplingFactor;  // MISRAC: division by 0 (in fact can't happen)
  nbSamplesOut     = (nbSamplesIn / decimFactor) * upSamplingFactor;

  assert(((nbSamplesOut / upSamplingFactor) * decimFactor) == nbSamplesIn);

  #ifdef BIQUAD_USE_INTRINSICS
  int32_t x1, x2, y1, y2;

  k  = 0;
  x1 = 0;
  x2 = 0;
  for (i = 0; i < nbSamplesOut; i++)
  {
    for (j = 0; j < decimFactor; j++)
    {
      if (k == 0)
      {
        x1 = *pIn;
        pIn++;
        x2 = *pIn;
        pIn++;
      }
      else
      {
        x1 = 0;                                                       // insert zeros for upsampling
        x2 = 0;                                                       // insert zeros for upsampling
      }
      k = ((k + 1) == upSamplingFactor) ? 0 : (k + 1);
      if (nbBiquadCells > 0U)
      {
        pBiquadCell = pBiquad->pBiquadCell;
        pBiquadMem  = pCtx->pBiquadMem;
        for (n = 0U; n < nbBiquadCells; n++)
        {
          y1            = __SMMULR(pBiquadCell->b0, x1);
          y2            = __SMMULR(pBiquadCell->b0, x2);
          y1            = __SMMLAR(pBiquadCell->b1, pBiquadMem[0], y1);
          y2            = __SMMLAR(pBiquadCell->b1, pBiquadMem[1], y2);
          y1            = __SMMLAR(pBiquadCell->b2, pBiquadMem[2], y1);
          y2            = __SMMLAR(pBiquadCell->b2, pBiquadMem[3], y2);
          y1            = __SMMLSR(pBiquadCell->a1, pBiquadMem[4], y1); // a1 is Q30 whereas other coefs are Q31
          y1            = __SMMLSR(pBiquadCell->a1, pBiquadMem[4], y1); // a1 is Q30 whereas other coefs are Q31
          y2            = __SMMLSR(pBiquadCell->a1, pBiquadMem[5], y2); // a1 is Q30 whereas other coefs are Q31
          y2            = __SMMLSR(pBiquadCell->a1, pBiquadMem[5], y2); // a1 is Q30 whereas other coefs are Q31
          y1            = __SMMLSR(pBiquadCell->a2, pBiquadMem[6], y1);
          y2            = __SMMLSR(pBiquadCell->a2, pBiquadMem[7], y2);
          y1            = __QADD(y1, y2);
          y2            = __QADD(y1, y2);
          pBiquadMem[2] = pBiquadMem[0];
          pBiquadMem[3] = pBiquadMem[1];
          pBiquadMem[0] = x1;
          pBiquadMem[1] = x2;
          x1            = y1;
          x2            = y2;
          pBiquadMem   += 4;
          pBiquadCell++;
        }
        pBiquadMem[2] = pBiquadMem[0];
        pBiquadMem[3] = pBiquadMem[1];
        pBiquadMem[0] = x1;
        pBiquadMem[1] = x2;
      }
    }
    x1    = __SMMULR(x1, pCtx->gainMant) << pCtx->gainExp;  /*cstat !MISRAC2012-Rule-10.1_R6 shift on signed integer for cpu load efficiency*/
    x2    = __SMMULR(x2, pCtx->gainMant) << pCtx->gainExp;  /*cstat !MISRAC2012-Rule-10.1_R6 shift on signed integer for cpu load efficiency*/
    x1    = __QADD(x1, x1);
    x2    = __QADD(x2, x2);
    *pOut = x1;
    pOut++;
    *pOut = x2;
    pOut++;
  }
  #else
  int64_t x1, x2, y1, y2;

  k  = 0;
  x1 = 0LL;
  x2 = 0LL;
  for (i = 0; i < nbSamplesOut; i++)
  {
    for (j = 0; j < decimFactor; j++)
    {
      if (k == 0)
      {
        x1 = (int64_t)(*pIn);
        pIn++;
        x2 = (int64_t)(*pIn);
        pIn++;
      }
      else
      {
        x1 = 0LL;   // insert zeros for upsampling
        x2 = 0LL;   // insert zeros for upsampling
      }
      k = ((k + 1) == upSamplingFactor) ? 0 : (k + 1);
      if (nbBiquadCells > 0U)
      {
        pBiquadCell = pBiquad->pBiquadCell;
        pBiquadMem  = pCtx->pBiquadMem;
        for (n = 0U; n < nbBiquadCells; n++)
        {
          y1            = (int64_t)pBiquadCell->b0 * x1;
          y2            = (int64_t)pBiquadCell->b0 * x2;
          y1           += (int64_t)pBiquadCell->b1 * (int64_t)pBiquadMem[0];
          y2           += (int64_t)pBiquadCell->b1 * (int64_t)pBiquadMem[1];
          y1           += (int64_t)pBiquadCell->b2 * (int64_t)pBiquadMem[2];
          y2           += (int64_t)pBiquadCell->b2 * (int64_t)pBiquadMem[3];
          y1           -= (int64_t)pBiquadCell->a1 * (int64_t)pBiquadMem[4] * 2LL;
          y2           -= (int64_t)pBiquadCell->a1 * (int64_t)pBiquadMem[5] * 2LL;
          y1           -= (int64_t)pBiquadCell->a2 * (int64_t)pBiquadMem[6];
          y2           -= (int64_t)pBiquadCell->a2 * (int64_t)pBiquadMem[7];
          y1          >>= 31U;  /*cstat !MISRAC2012-Rule-10.1_R6 !MISRAC2012-Rule-1.3_n shift on signed integer for cpu load efficiency*/
          y2          >>= 31U;  /*cstat !MISRAC2012-Rule-10.1_R6 !MISRAC2012-Rule-1.3_n shift on signed integer for cpu load efficiency*/
          y1            = (y1 < -2147483648LL) ? -2147483648LL : ((y1 > 2147483647LL) ? 2147483647LL : y1);
          y2            = (y2 < -2147483648LL) ? -2147483648LL : ((y2 > 2147483647LL) ? 2147483647LL : y2);
          pBiquadMem[2] = pBiquadMem[0];
          pBiquadMem[3] = pBiquadMem[1];
          pBiquadMem[0] = (int32_t)x1;
          pBiquadMem[1] = (int32_t)x2;
          x1            = y1;
          x2            = y2;
          pBiquadMem   += 4;
          pBiquadCell++;
        }
        pBiquadMem[2] = pBiquadMem[0];
        pBiquadMem[3] = pBiquadMem[1];
        pBiquadMem[0] = (int32_t)x1;
        pBiquadMem[1] = (int32_t)x2;
      }
    }
    x1    = (x1 * (int64_t)pCtx->gainMant) >> (31U - pCtx->gainExp);  /*cstat !MISRAC2012-Rule-10.1_R6 !MISRAC2012-Rule-1.3_n shift on signed integer for cpu load efficiency*/
    x2    = (x2 * (int64_t)pCtx->gainMant) >> (31U - pCtx->gainExp);  /*cstat !MISRAC2012-Rule-10.1_R6 !MISRAC2012-Rule-1.3_n shift on signed integer for cpu load efficiency*/
    x1    = (x1 < -2147483648LL) ? -2147483648LL : ((x1 > 2147483647LL) ? 2147483647LL : x1);
    x2    = (x2 < -2147483648LL) ? -2147483648LL : ((x2 > 2147483647LL) ? 2147483647LL : x2);
    *pOut = (int32_t)x1;
    pOut++;
    *pOut = (int32_t)x2;
    pOut++;
  }
  #endif
}
#endif /* BIQUAD_CODE_SIZE_OPTIM */


static void s_biquadInt32ProcessResamplingMultiChannels(biquadIntContext_t *const pCtx, void *const in, void *const out, int const nbSamplesIn)
{
  int                             decimFactor      = pCtx->decimFactor;
  int                             upSamplingFactor = pCtx->upSamplingFactor;
  int                      const  nbChannels       = pCtx->nbChannels;
  biquadInt32_t     const *const  pBiquad          = pCtx->pBiquad;
  uint8_t                  const  nbBiquadCells    = pBiquad->nbCells;
  int32_t                  const *pIn              = (int32_t const *)in;
  int32_t                        *pOut             = (int32_t *)out;
  int32_t                  const *pInSave         = pIn;
  biquadCellInt32_t const        *pBiquadCell;
  int32_t                        *pBiquadMem;
  int                             nbSamplesOut, i, j, ch, k, kSave;
  uint8_t                         n;

  decimFactor      = (decimFactor      == 0) ? 1 : decimFactor;       // MISRAC: division by 0 (in fact can't happen)
  upSamplingFactor = (upSamplingFactor == 0) ? 1 : upSamplingFactor;  // MISRAC: division by 0 (in fact can't happen)
  nbSamplesOut     = (nbSamplesIn / decimFactor) * upSamplingFactor;

  assert(((nbSamplesOut / upSamplingFactor) * decimFactor) == nbSamplesIn);

  #ifdef BIQUAD_USE_INTRINSICS
  int32_t x, y;

  k = 0;
  x = 0;
  for (i = 0; i < nbSamplesOut; i++)
  {
    kSave = k;
    for (ch = 0; ch < nbChannels; ch++)
    {
      pIn = &pInSave[ch];
      k   = kSave;
      for (j = 0; j < decimFactor; j++)
      {
        if (k == 0)
        {
          x    = *pIn;
          pIn += nbChannels;
        }
        else
        {
          x = 0;                                                        // insert zeros for upsampling
        }
        k = ((k + 1) == upSamplingFactor) ? 0 : (k + 1);
        if (nbBiquadCells > 0U)
        {
          pBiquadCell = pBiquad->pBiquadCell;
          pBiquadMem  = &pCtx->pBiquadMem[((2 * (int)pBiquad->nbCells) + 2) * ch];
          for (n = 0U; n < nbBiquadCells; n++)
          {
            y             = __SMMULR(pBiquadCell->b0, x);
            y             = __SMMLAR(pBiquadCell->b1, pBiquadMem[0], y);
            y             = __SMMLAR(pBiquadCell->b2, pBiquadMem[1], y);
            y             = __SMMLSR(pBiquadCell->a1, pBiquadMem[2], y);  // a1 is Q30 whereas other coefs are Q31
            y             = __SMMLSR(pBiquadCell->a1, pBiquadMem[2], y);  // a1 is Q30 whereas other coefs are Q31
            y             = __SMMLSR(pBiquadCell->a2, pBiquadMem[3], y);
            y             = __QADD(y, y);
            pBiquadMem[1] = pBiquadMem[0];
            pBiquadMem[0] = x;
            x             = y;
            pBiquadMem   += 2;
            pBiquadCell++;
          }
          pBiquadMem[1] = pBiquadMem[0];
          pBiquadMem[0] = (int32_t)x;
        }
      }
      x     = __SMMULR(x, pCtx->gainMant) << pCtx->gainExp;   /*cstat !MISRAC2012-Rule-10.1_R6 !MISRAC2012-Rule-1.3_n shift on signed integer for cpu load efficiency*/
      x     = __QADD(x, x);
      *pOut = x;
      pOut++;
    }
    pInSave = &pIn[1 - nbChannels];                                     // next ch 0 input sample
  }
  #else
  int64_t x, y;

  k = 0;
  x = 0LL;
  for (i = 0; i < nbSamplesOut; i++)
  {
    kSave = k;
    for (ch = 0; ch < nbChannels; ch++)
    {
      pIn = &pInSave[ch];
      k   = kSave;
      for (j = 0; j < decimFactor; j++)
      {
        if (k == 0)
        {
          x    = (int64_t)(*pIn);
          pIn += nbChannels;
        }
        else
        {
          x = 0LL;  // insert zeros for upsampling
        }
        k = ((k + 1) == upSamplingFactor) ? 0 : (k + 1);
        if (nbBiquadCells > 0U)
        {
          pBiquadCell = pBiquad->pBiquadCell;
          pBiquadMem  = &pCtx->pBiquadMem[((2 * (int)pBiquad->nbCells) + 2) * ch];
          for (n = 0U; n < nbBiquadCells; n++)
          {
            y             = (int64_t)pBiquadCell->b0 * x;
            y            += (int64_t)pBiquadCell->b1 * (int64_t)pBiquadMem[0];
            y            += (int64_t)pBiquadCell->b2 * (int64_t)pBiquadMem[1];
            y            -= (int64_t)pBiquadCell->a1 * (int64_t)pBiquadMem[2] * 2LL;  // a1 is Q30 whereas other coefs are Q31
            y            -= (int64_t)pBiquadCell->a2 * (int64_t)pBiquadMem[3];
            y           >>= 31U;  /*cstat !MISRAC2012-Rule-10.1_R6 !MISRAC2012-Rule-1.3_n shift on signed integer for cpu load efficiency*/
            y             = (y < -2147483648LL) ? -2147483648LL : ((y > 2147483647LL) ? 2147483647LL : y);
            pBiquadMem[1] = pBiquadMem[0];
            pBiquadMem[0] = (int32_t)x;
            x             = y;
            pBiquadMem    += 2;
            pBiquadCell++;
          }
          pBiquadMem[1] = pBiquadMem[0];
          pBiquadMem[0] = (int32_t)x;
        }
      }
      x     = (x * (int64_t)pCtx->gainMant) >> (31U - pCtx->gainExp);   /*cstat !MISRAC2012-Rule-10.1_R6 !MISRAC2012-Rule-1.3_n shift on signed integer for cpu load efficiency*/
      x     = (x < -2147483648LL) ? -2147483648LL : ((x > 2147483647LL) ? 2147483647LL : x);
      *pOut = (int32_t)x;
      pOut++;
    }
    pInSave = &pIn[1 - nbChannels];   // next ch 0 input sample
  }
  #endif
}


#ifndef BIQUAD_CODE_SIZE_OPTIM
static void s_biquadFloatProcessNoResamplingMono(biquadFloatContext_t *const pCtx, float *const in, float *const out, int const nbSamplesIn)
{
  biquadFloat_t     const *const  pBiquad       = pCtx->pBiquad;
  uint8_t                  const  nbBiquadCells = pBiquad->nbCells;
  float                    const *pIn           = in;
  float                          *pOut          = out;
  biquadCellFloat_t const        *pBiquadCell;
  float                          *pBiquadMem;
  float                           x, y;
  int                             i;
  uint8_t                         n;

  for (i = 0; i < nbSamplesIn; i++)
  {
    x = *pIn;
    pIn++;
    if (nbBiquadCells > 0U)
    {
      pBiquadCell = pBiquad->pBiquadCell;
      pBiquadMem  = pCtx->pBiquadMem;
      for (n = 0U; n < nbBiquadCells; n++)
      {
        y             = pBiquadCell->b0 * x;
        y            += pBiquadCell->b1 * pBiquadMem[0];
        y            += pBiquadCell->b2 * pBiquadMem[1];
        y            -= pBiquadCell->a1 * pBiquadMem[2];
        y            -= pBiquadCell->a2 * pBiquadMem[3];
        pBiquadMem[1] = pBiquadMem[0];
        pBiquadMem[0] = x;
        x             = y;
        pBiquadMem   += 2;
        pBiquadCell++;
      }
      pBiquadMem[1] = pBiquadMem[0];
      pBiquadMem[0] = x;
    }
    *pOut = x;
    pOut++;
  }
}


static void s_biquadFloatProcessNoResamplingStereo(biquadFloatContext_t *const pCtx, float *const in, float *const out, int const nbSamplesIn)
{
  biquadFloat_t     const *const  pBiquad       = pCtx->pBiquad;
  uint8_t                  const  nbBiquadCells = pBiquad->nbCells;
  float                    const *pIn           = in;
  float                          *pOut          = out;
  biquadCellFloat_t const        *pBiquadCell;
  float                          *pBiquadMem;
  float                           x1, x2, y1, y2;
  int                             i;
  uint8_t                         n;

  for (i = 0; i < nbSamplesIn; i++)
  {
    x1 = *pIn;
    pIn++;
    x2 = *pIn;
    pIn++;
    if (nbBiquadCells > 0U)
    {
      pBiquadCell = pBiquad->pBiquadCell;
      pBiquadMem  = pCtx->pBiquadMem;
      for (n = 0U; n < nbBiquadCells; n++)
      {
        y1            = pBiquadCell->b0 * x1;
        y2            = pBiquadCell->b0 * x2;
        y1           += pBiquadCell->b1 * pBiquadMem[0];
        y2           += pBiquadCell->b1 * pBiquadMem[1];
        y1           += pBiquadCell->b2 * pBiquadMem[2];
        y2           += pBiquadCell->b2 * pBiquadMem[3];
        y1           -= pBiquadCell->a1 * pBiquadMem[4];
        y2           -= pBiquadCell->a1 * pBiquadMem[5];
        y1           -= pBiquadCell->a2 * pBiquadMem[6];
        y2           -= pBiquadCell->a2 * pBiquadMem[7];
        pBiquadMem[2] = pBiquadMem[0];
        pBiquadMem[3] = pBiquadMem[1];
        pBiquadMem[0] = x1;
        pBiquadMem[1] = x2;
        x1            = y1;
        x2            = y2;
        pBiquadMem   += 4;
        pBiquadCell++;
      }
      pBiquadMem[2] = pBiquadMem[0];
      pBiquadMem[3] = pBiquadMem[1];
      pBiquadMem[0] = x1;
      pBiquadMem[1] = x2;
    }
    *pOut         = x1;
    pOut++;
    *pOut         = x2;
    pOut++;
  }
}


static void s_biquadFloatProcessNoResamplingMultiChannels(biquadFloatContext_t *const pCtx, float *const in, float *const out, int const nbSamplesIn)
{
  int                      const  nbChannels    = pCtx->nbChannels;
  biquadFloat_t     const *const  pBiquad       = pCtx->pBiquad;
  uint8_t                  const  nbBiquadCells = pBiquad->nbCells;
  float                    const *pIn           = in;
  float                          *pOut          = out;
  biquadCellFloat_t const        *pBiquadCell;
  float                          *pBiquadMem;
  float                            x, y;
  int                              i, ch;
  uint8_t                         n;

  for (i = 0; i < nbSamplesIn; i++)
  {
    for (ch = 0; ch < nbChannels; ch++)
    {
      x = *pIn;
      pIn++;
      if (nbBiquadCells > 0U)
      {
        pBiquadCell = pBiquad->pBiquadCell;
        pBiquadMem  = &pCtx->pBiquadMem[((2 * (int)pBiquad->nbCells) + 2) * ch];
        for (n = 0U; n < nbBiquadCells; n++)
        {
          y             = pBiquadCell->b0 * x;
          y            += pBiquadCell->b1 * pBiquadMem[0];
          y            += pBiquadCell->b2 * pBiquadMem[1];
          y            -= pBiquadCell->a1 * pBiquadMem[2];
          y            -= pBiquadCell->a2 * pBiquadMem[3];
          pBiquadMem[1] = pBiquadMem[0];
          pBiquadMem[0] = x;
          x             = y;
          pBiquadMem    += 2;
          pBiquadCell++;
        }
        pBiquadMem[1] = pBiquadMem[0];
        pBiquadMem[0] = x;
      }
      *pOut = x;
      pOut++;
    }
  }
}


static void s_biquadFloatProcessResamplingMono(biquadFloatContext_t *const pCtx, float *const in, float *const out, int const nbSamplesIn)
{
  int                             decimFactor      = pCtx->decimFactor;
  int                             upSamplingFactor = pCtx->upSamplingFactor;
  biquadFloat_t     const *const  pBiquad          = pCtx->pBiquad;
  uint8_t                  const  nbBiquadCells    = pBiquad->nbCells;
  float                    const *pIn              = in;
  float                          *pOut             = out;
  biquadCellFloat_t const        *pBiquadCell;
  float                          *pBiquadMem;
  float                           x, y;
  int                             nbSamplesOut, i, j, k;
  uint8_t                         n;

  decimFactor      = (decimFactor      == 0) ? 1 : decimFactor;       // MISRAC: division by 0 (in fact can't happen)
  upSamplingFactor = (upSamplingFactor == 0) ? 1 : upSamplingFactor;  // MISRAC: division by 0 (in fact can't happen)
  nbSamplesOut     = (nbSamplesIn / decimFactor) * upSamplingFactor;

  assert(((nbSamplesOut / upSamplingFactor) * decimFactor) == nbSamplesIn);

  k = 0;
  x = 0.0f;
  for (i = 0; i < nbSamplesOut; i++)
  {
    for (j = 0; j < decimFactor; j++)
    {
      if (k == 0)
      {
        x = *pIn;
        pIn++;
      }
      else
      {
        x = 0.0f;   // insert zeros for upsampling
      }
      k = ((k + 1) == upSamplingFactor) ? 0 : (k + 1);
      if (nbBiquadCells > 0U)
      {
        pBiquadCell = pBiquad->pBiquadCell;
        pBiquadMem  = pCtx->pBiquadMem;
        for (n = 0U; n < nbBiquadCells; n++)
        {
          y             = pBiquadCell->b0 * x;
          y            += pBiquadCell->b1 * pBiquadMem[0];
          y            += pBiquadCell->b2 * pBiquadMem[1];
          y            -= pBiquadCell->a1 * pBiquadMem[2];
          y            -= pBiquadCell->a2 * pBiquadMem[3];
          pBiquadMem[1] = pBiquadMem[0];
          pBiquadMem[0] = x;
          x             = y;
          pBiquadMem   += 2;
          pBiquadCell++;
        }
        pBiquadMem[1] = pBiquadMem[0];
        pBiquadMem[0] = x;
      }
    }
    *pOut = x * (float)upSamplingFactor;  // apply a gain=upSamplingFactor to compensate attenuation introduced by inserted zeros
    pOut++;
  }
}


static void s_biquadFloatProcessResamplingStereo(biquadFloatContext_t *const pCtx, float *const in, float *const out, int const nbSamplesIn)
{
  int                             decimFactor      = pCtx->decimFactor;
  int                             upSamplingFactor = pCtx->upSamplingFactor;
  biquadFloat_t     const *const  pBiquad          = pCtx->pBiquad;
  uint8_t                  const  nbBiquadCells    = pBiquad->nbCells;
  float                    const *pIn              = in;
  float                          *pOut             = out;
  biquadCellFloat_t const        *pBiquadCell;
  float                          *pBiquadMem;
  float                           x1, x2, y1, y2;
  int                             nbSamplesOut, i, j, k;
  uint8_t                         n;

  decimFactor      = (decimFactor      == 0) ? 1 : decimFactor;       // MISRAC: division by 0 (in fact can't happen)
  upSamplingFactor = (upSamplingFactor == 0) ? 1 : upSamplingFactor;  // MISRAC: division by 0 (in fact can't happen)
  nbSamplesOut     = (nbSamplesIn / decimFactor) * upSamplingFactor;

  assert(((nbSamplesOut / upSamplingFactor) * decimFactor) == nbSamplesIn);

  k  = 0;
  x1 = 0.0f;
  x2 = 0.0f;
  for (i = 0; i < nbSamplesOut; i++)
  {
    for (j = 0; j < decimFactor; j++)
    {
      if (k == 0)
      {
        x1 = *pIn;
        pIn++;
        x2 = *pIn;
        pIn++;
      }
      else
      {
        x1 = 0.0f;  // insert zeros for upsampling
        x2 = 0.0f;  // insert zeros for upsampling
      }
      k = ((k + 1) == upSamplingFactor) ? 0 : (k + 1);
      if (nbBiquadCells > 0U)
      {
        pBiquadCell = pBiquad->pBiquadCell;
        pBiquadMem  = pCtx->pBiquadMem;
        for (n = 0U; n < nbBiquadCells; n++)
        {
          y1            = pBiquadCell->b0 * x1;
          y2            = pBiquadCell->b0 * x2;
          y1           += pBiquadCell->b1 * pBiquadMem[0];
          y2           += pBiquadCell->b1 * pBiquadMem[1];
          y1           += pBiquadCell->b2 * pBiquadMem[2];
          y2           += pBiquadCell->b2 * pBiquadMem[3];
          y1           -= pBiquadCell->a1 * pBiquadMem[4];
          y2           -= pBiquadCell->a1 * pBiquadMem[5];
          y1           -= pBiquadCell->a2 * pBiquadMem[6];
          y2           -= pBiquadCell->a2 * pBiquadMem[7];
          pBiquadMem[2] = pBiquadMem[0];
          pBiquadMem[3] = pBiquadMem[1];
          pBiquadMem[0] = x1;
          pBiquadMem[1] = x2;
          x1            = y1;
          x2            = y2;
          pBiquadMem   += 4;
          pBiquadCell++;
        }
        pBiquadMem[2] = pBiquadMem[0];
        pBiquadMem[3] = pBiquadMem[1];
        pBiquadMem[0] = x1;
        pBiquadMem[1] = x2;
      }
    }
    *pOut = x1 * (float)upSamplingFactor;   // apply a gain=upSamplingFactor to compensate attenuation introduced by inserted zeros
    pOut++;
    *pOut = x2 * (float)upSamplingFactor;   // apply a gain=upSamplingFactor to compensate attenuation introduced by inserted zeros
    pOut++;
  }
}
#endif /* BIQUAD_CODE_SIZE_OPTIM */


static void s_biquadFloatProcessResamplingMultiChannels(biquadFloatContext_t *const pCtx, float *const in, float *const out, int const nbSamplesIn)
{
  int                             decimFactor      = pCtx->decimFactor;
  int                             upSamplingFactor = pCtx->upSamplingFactor;
  int                      const  nbChannels       = pCtx->nbChannels;
  biquadFloat_t     const *const  pBiquad          = pCtx->pBiquad;
  uint8_t                  const  nbBiquadCells    = pBiquad->nbCells;
  float                    const *pIn              = in;
  float                          *pOut             = out;
  float                    const *pInSave          = pIn;
  biquadCellFloat_t const        *pBiquadCell;
  float                          *pBiquadMem;
  float                           x, y;
  int                             nbSamplesOut, i, j, ch, k, kSave;
  uint8_t                         n;

  decimFactor      = (decimFactor      == 0) ? 1 : decimFactor;       // MISRAC: division by 0 (in fact can't happen)
  upSamplingFactor = (upSamplingFactor == 0) ? 1 : upSamplingFactor;  // MISRAC: division by 0 (in fact can't happen)
  nbSamplesOut     = (nbSamplesIn / decimFactor) * upSamplingFactor;

  assert(((nbSamplesOut / upSamplingFactor) * decimFactor) == nbSamplesIn);

  k = 0;
  x = 0.0f;
  for (i = 0; i < nbSamplesOut; i++)
  {
    kSave = k;
    for (ch = 0; ch < nbChannels; ch++)
    {
      pIn = &pInSave[ch];
      k   = kSave;
      for (j = 0; j < decimFactor; j++)
      {
        if (k == 0)
        {
          x    = *pIn;
          pIn += nbChannels;
        }
        else
        {
          x = 0.0f;   // insert zeros for upsampling
        }
        k = ((k + 1) == upSamplingFactor) ? 0 : (k + 1);
        if (nbBiquadCells > 0U)
        {
          pBiquadCell = pBiquad->pBiquadCell;
          pBiquadMem  = &pCtx->pBiquadMem[((2 * (int)pBiquad->nbCells) + 2) * ch];
          for (n = 0U; n < nbBiquadCells; n++)
          {
            y             = pBiquadCell->b0 * x;
            y            += pBiquadCell->b1 * pBiquadMem[0];
            y            += pBiquadCell->b2 * pBiquadMem[1];
            y            -= pBiquadCell->a1 * pBiquadMem[2];
            y            -= pBiquadCell->a2 * pBiquadMem[3];
            pBiquadMem[1] = pBiquadMem[0];
            pBiquadMem[0] = x;
            x             = y;
            pBiquadMem   += 2;
            pBiquadCell++;
          }
          pBiquadMem[1] = pBiquadMem[0];
          pBiquadMem[0] = x;
        }
      }
      *pOut = x * (float)upSamplingFactor;  // apply a gain=upSamplingFactor to compensate attenuation introduced by inserted zeros
      pOut++;
    }
    pInSave = &pIn[1 - nbChannels];   // next ch 0 input sample
  }
}

