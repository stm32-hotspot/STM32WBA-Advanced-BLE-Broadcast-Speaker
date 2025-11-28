/**
  ******************************************************************************
  * @file    fBeamforming_coef.h
  * @author  MCD Application Team
  * @brief   beamforming weighting coefficient
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
#ifndef __FBEAMFORMING_COEF_H
#define __FBEAMFORMING_COEF_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>


/* Exported types ------------------------------------------------------------*/
typedef struct
{
  float const *const pCoeff;
  uint32_t           nbElements;
}
BEAMFORMER_Coeff_t;

typedef struct
{
  BEAMFORMER_Coeff_t const *pAntenna0;
  BEAMFORMER_Coeff_t const *pAntenna1;
}
FBF_Coeffs_t;

/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

#ifdef __cplusplus
}
#endif

#endif  /* __FBEAMFORMING_COEF_H */
