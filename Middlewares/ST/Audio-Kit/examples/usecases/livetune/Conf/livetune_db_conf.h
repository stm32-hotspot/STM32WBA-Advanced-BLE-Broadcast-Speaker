/**
******************************************************************************
* @file          livetune_db_conf.h
* @author        MCD Application Team
* @brief         Designer db customization
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


#ifndef livetune_db_conf_h
#define livetune_db_conf_h


#ifdef __cplusplus
extern "C"
{
#endif
/* Includes ------------------------------------------------------------------*/
/* Exported defines --------------------------------------------------------*/

#define LIVETUNE_VAR_NAME_MAX 128
/* pool management */
#define ST_Mem_Type_Designer    ST_Mem_Type_ANY_SLOW /* preference to use ram ext */


/* Exported types --------------------------------------------------------*/
/*
  This struct is equivalent to audio_chunk_conf but never copied BY CAST, each field is accessed separately,
  This duplication allows to separate APIs and unlink audio chain dependencies & designer.
  structs  old chunk/instance configurations in the livetune_db nodes
  The designer uses only acSDK and AC Factory API
  We have to pay attention to warnings if a type has changed in AC
*/

typedef struct livetune_chunk_conf_t
{
  uint8_t     chunkType;
  uint8_t     timeFreq;
  uint8_t     bufferType;
  uint8_t     interleaved;
  uint8_t     nbFrames;
  uint8_t     nbChannels;
  uint32_t    nbElements;
  uint32_t    fs;
  const char *pName;
} livetune_chunk_conf_t;

typedef struct st_audio_chain_instance
{
  uint8_t bValid;
  uint8_t bSysIO;
  void   *hAlgo;
  char_t  tVarName[LIVETUNE_VAR_NAME_MAX];
} st_audio_chain_instance;


typedef struct st_audio_chain_cnx
{
  uint8_t               bValid;
  livetune_chunk_conf_t hConf;
  void                 *hChunk;
  char_t               *pSysIoName;
  char_t                tVarName[LIVETUNE_VAR_NAME_MAX];
} st_audio_chain_cnx;




#define LIVETUNE_INSTANCE_EXTRA_STRUCT st_audio_chain_instance hAc;
#define LIVETUNE_CNX_EXTRA_STRUCT      st_audio_chain_cnx hAc;


/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */




#ifdef __cplusplus
}
#endif

#endif


