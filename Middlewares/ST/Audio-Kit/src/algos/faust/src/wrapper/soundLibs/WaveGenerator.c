/**
******************************************************************************
* @file    WaveGenerator.c
* @author  MCD Application Team
* @brief   produce waves buffer
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

/*cstat -MISRAC2012-* CMSIS not misra compliant */
#include <arm_math.h>
/*cstat +MISRAC2012-* */
#include "WaveGenerator.h"
#define TWO_PI  (3.14159265359f * 2.0f)

/* ARM CMSIS DSP optimization */
#define sinf  arm_sin_f32
#define cosf  arm_cos_f32
#define tanf  tan_f32
#define sqrtf sqrt_f32

/*cstat -MISRAC2012-Rule-22.8 no issue with powf*/
void generateWave(float_t *outWaveForm, int32_t inLength, int16_t inWaveType, int16_t inMaxHarmo, float_t inVolume)
{
  float_t hCount = (float_t)inMaxHarmo;
  float_t harmo;
  int i;
  float_t ifrac = 0.0f;
  float_t comp = inVolume;
  const float_t kTwoPiOverLen = TWO_PI / (float_t)inLength;

  switch (inWaveType)
  {

    case kAdditiveSquare:
    {
      comp *= 1.0f;
      if (hCount == 0.0f)
      {
        hCount = 64.0f;
      }

      for (i = 0; i < inLength; ++ i)
      {
        outWaveForm[i] = 0.0f;
      }

      float_t period;
      float_t fact;
      for (harmo = 1.0f; harmo <= hCount; harmo += 2.0f)
      {
        period = harmo * kTwoPiOverLen;
        fact = comp / harmo;

        for (i = 0, ifrac = 0.0f; i < inLength; ++ i)
        {
          float alpha = ifrac * period;
          outWaveForm[i] += (fact * sinf(alpha));
          ++ ifrac;
        }
      }
      break;
    }

    case kAdditiveTriangle:
    {
      comp *= 0.80f;
      if (hCount == 0.0f)
      {
        hCount = 16.0f;
      }

      for (i = 0; i < inLength; ++i)
      {
        outWaveForm[i] = 0.0f;
      }

      float_t period;
      float_t fact;
      for (harmo = 1.0f; harmo <= hCount; harmo += 2.0f)
      {
        period = harmo * kTwoPiOverLen;
        fact = (comp / (harmo * harmo));
        for (i = 0, ifrac = 0.0f; i < inLength; ++i)
        {
          outWaveForm[i] += (fact * cosf(ifrac * period));
          ++ ifrac;
        }
      }
      break;
    }

    case kAdditiveSaw:
    {
      comp *= 0.53f;
      if (hCount == 0.0f)
      {
        hCount = 16.0f;
      }

      for (i = 0; i < inLength; ++ i)
      {
        outWaveForm[i] = 0.0f;
      }

      float_t period;
      float_t fact;

      for (harmo = 1.0f; harmo <= hCount; ++ harmo)
      {
        period = harmo * kTwoPiOverLen;
        fact = (comp * powf(-1.0f, ((harmo + 1.0f)) / harmo));
        for (i = 0, ifrac = 0.0f; i < inLength; ++ i)
        {
          outWaveForm[i] += (fact * sinf(ifrac * period));
          ++ ifrac;
        }
      }
      break;
    }

    // - Orgues

    case kOrgan:
    {
      comp *= 1.2f;
      if (hCount == 0.0f)
      {
        hCount = 32.0f;
      }

      float_t period;
      float_t fact;

      for (i = 0; i < inLength; i ++)
      {
        outWaveForm[i] = 0.0f;
      }

      for (harmo = 1.0f; powf(2.0f, harmo - 1.0f) <= hCount; ++harmo)
      {
        period = (powf(2.0f, harmo - 1.0f) * kTwoPiOverLen);
        fact = comp * 0.5f / harmo;
        for (i = 0, ifrac = 0.0f; i < inLength; ++i)
        {
          outWaveForm[i] += (fact * sinf(ifrac * period));
          ++ ifrac;
        }
      }
      break;
    }

    case kOctaveOrgan:
    {
      comp *= 0.63f;
      if (hCount > 22.0f)
      {
        hCount = 22.0f;
      }

      float_t period;
      float_t fact;

      for (i = 0; i < inLength; i ++)
      {
        outWaveForm[i] = 0.0f;
      }

      for (harmo = 1.0f; (powf(2.0f, harmo) * 0.5f) <= hCount; ++ harmo)
      {
        period = (powf(2.0f, harmo) * 0.5f * kTwoPiOverLen);
        fact = comp * 0.5f; //(0.5f / harmo);
        for (i = 0, ifrac = 0.0f; i < inLength; ++i)
        {
          outWaveForm[i] += (fact * sinf(ifrac * period));
          ++ ifrac;
        }
      }

      break;
    }

    case kBassOrgan:
    {
      comp *= 1.2f;
      if (hCount == 0.0f)
      {
        hCount = 32.0f;
      }

      float_t period = kTwoPiOverLen;
      float_t fact = comp * 0.5f;

      for (i = 0; i < inLength; i ++)
      {
        outWaveForm[i]  = (fact * sinf((float)i * period));
        outWaveForm[i] += (fact * cosf((float)i * period));
      }

      fact = comp / (2.0f * hCount);
      for (harmo = 2.0f; powf(2.0f, harmo) <= hCount; ++ harmo)
      {
        period = (powf(2.0f, harmo) * kTwoPiOverLen);
        for (i = 0, ifrac = 0.0f; i < inLength; ++ i)
        {
          outWaveForm[i] += (fact * sinf(ifrac * period));
          outWaveForm[i] += (fact * cosf(ifrac * period));
          ++ ifrac;
        }
      }
      break;
    }

    // - Misc
    case kSmoothSquare:
    {
      if (hCount == 0.0f)
      {
        hCount = 16.0f;
      }

      float_t h;
      // test :
      h = 1.0f;
      float_t fact;
      float_t period;
      for (i = 0; i < inLength; i ++)
      {
        outWaveForm[i] = 0.0f;
      }

      for (harmo = 1.0f; harmo <= hCount; harmo += 2.0f)
      {
        fact = (1.0f / (h * powf(h, 0.5f)));
        period = (((2.0f * h) - 1.0f) * kTwoPiOverLen);
        for (i = 0, ifrac = 0.0f; i < inLength; ++i)
        {
          outWaveForm[i] += (fact * sinf(period * ifrac));

          ++ ifrac;
        }
        ++h;
      }
      break;
    }


    case kSinus:
    default:
      // comp *= 1;
      for (i = 0; i < inLength; ++i)
      {
        outWaveForm[i] = (comp * sinf((float)i * kTwoPiOverLen));
      }
      break;
  }
}
/*cstat +MISRAC2012-Rule-22.8*/