/**
  ******************************************************************************
  * @file    g711_st.h
  * @author  MCD Application Team
  * @brief   g711 ST version (tables based)
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
#ifndef __G711_ST_H
#define __G711_ST_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported define -----------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
int16_t g711_alaw_decode(uint8_t         const alaw_sample);
uint8_t g711_alaw_encode(int16_t         const pcm_sample);
int16_t g711_ulaw_decode(uint8_t         const ulaw_sample);
uint8_t g711_ulaw_encode(int16_t         const pcm_sample);
uint8_t g711_alaw2ulaw_transcode(uint8_t const alaw_sample);
uint8_t g711_ulaw2alaw_transcode(uint8_t const ulaw_sample);
void    g711_alaw_buff_decode(uint8_t         const *const input, int16_t *const output, int const nb_samples, int const offset);
void    g711_alaw_buff_encode(int16_t         const *const input, uint8_t *const output, int const nb_samples, int const offset);
void    g711_ulaw_buff_decode(uint8_t         const *const input, int16_t *const output, int const nb_samples, int const offset);
void    g711_ulaw_buff_encode(int16_t         const *const input, uint8_t *const output, int const nb_samples, int const offset);
void    g711_alaw2ulaw_buff_transcode(uint8_t const *const input, uint8_t *const output, int const nb_samples, int const offset);
void    g711_ulaw2alaw_buff_transcode(uint8_t const *const input, uint8_t *const output, int const nb_samples, int const offset);

#ifdef __cplusplus
}
#endif

#endif  /* __G711_ST_H */
