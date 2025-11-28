/**
  ******************************************************************************
  * @file    app_menu_cfg.h
  * @author  MCD Application Team
  * @brief   Header for Menu Configuration file.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2019-2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef APP_MENU_CFG_H
#define APP_MENU_CFG_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "tmap_app.h"
#include "stm_list.h"


/* Defines ------------------------------------------------------------------ */

/* Exported Types ------------------------------------------------------------ */

/* Shared variables --------------------------------------------------------- */

/* Exported Prototypes -------------------------------------------------------*/
/**
 * @brief Initialize and setup the menu
 */
void Menu_Config(void);

/**
 * @brief Set the synchronization state
 */
void Menu_SetSyncState(APP_ScanState_t ScanState, APP_PASyncState_t PASyncState, APP_BIGSyncState_t BIGSyncState);

/**
 * @brief Set the broadcast source name
 */
void Menu_SetBroadcastSourceName(const char* pBroadcastSourceName);

/**
 * @brief Set the connection state
 */
void Menu_SetConnectionState(uint8_t ConnectionState);

/**
 * @brief Set language
 */
void Menu_SetLanguage(char* pLanguage);

/**
 * @brief Set Current volume value
 */
void Menu_SetVolume(uint8_t Volume);

/**
 * @brief Set Call State
 */
void Menu_SetCallState(char *pCallState);

/**
 * @brief Set Media State
 */
void Menu_SetMediaState(char *pMediaState);

/**
 * @brief Set Track Title
 */
void Menu_SetTrackTitle(uint8_t *pTrackTitle, uint8_t TrackNameLen);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* APP_MENU_CFG_H */
