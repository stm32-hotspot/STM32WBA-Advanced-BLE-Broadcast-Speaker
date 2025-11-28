/**
******************************************************************************
* @file    app_menu_cfg.c
* @author  MCD Application Team
* @brief   Configuration interface of menu for application
******************************************************************************
* @attention
*
* Copyright (c) 2020-2021 STMicroelectronics.
* All rights reserved.
*
* This software is licensed under terms that can be found in the LICENSE file
* in the root directory of this software component.
* If no LICENSE file comes with this software, it is provided AS-IS.
*
******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "app_menu_cfg.h"
#include "app_menu.h"
#include "app_conf.h"
#include "ble_gap_aci.h"
#include "log_module.h"
#include "stm32_timer.h"
#include "stm32_seq.h"

/* External variables ------------------------------------------------------- */
extern uint8_t volume_mute_byteicon[];
extern uint8_t volume_up_byteicon[];
extern uint8_t volume_down_byteicon[];
extern uint8_t arrow_right_byteicon[];
extern uint8_t initiate_call_byteicon[];
extern uint8_t answer_call_byteicon[];
extern uint8_t terminate_call_byteicon[];
extern uint8_t play_pause_byteicon[];
extern uint8_t next_track_byteicon[];
extern uint8_t previous_track_byteicon[];

/* Private defines ---------------------------------------------------------- */
#define ADVERTISING_TIMEOUT             (60000u)
#define STARTUP_TIMEOUT                 (1500u)
#define MAX_NUM_BROADCAST_SOURCE        (6u)

typedef struct
{
  uint8_t AdvSID;
  uint8_t Address[6u];
  uint8_t AddressType;
} Broadcast_Source_t;

/* Private variables -------------------------------------------------------- */
Menu_Icon_t access_menu_icon = {MENU_ICON_TYPE_IMAGE, (uint8_t *) &arrow_right_byteicon, 16, 16, 0};
Menu_Icon_t volume_up_icon = {MENU_ICON_TYPE_IMAGE, (uint8_t *) &volume_up_byteicon, 16, 16, 0};
Menu_Icon_t volume_down_icon = {MENU_ICON_TYPE_IMAGE, (uint8_t *) &volume_down_byteicon, 16, 16, 0};
Menu_Icon_t volume_mute_icon = {MENU_ICON_TYPE_IMAGE, (uint8_t *) &volume_mute_byteicon, 16, 16, 0};
Menu_Icon_t initiate_call_icon = {MENU_ICON_TYPE_IMAGE, (uint8_t *) &initiate_call_byteicon, 16, 16, 0};
Menu_Icon_t answer_call_icon = {MENU_ICON_TYPE_IMAGE, (uint8_t *) &answer_call_byteicon, 16, 16, 0};
Menu_Icon_t terminate_call_icon = {MENU_ICON_TYPE_IMAGE, (uint8_t *) &terminate_call_byteicon, 16, 16, 0};
Menu_Icon_t next_track_icon = {MENU_ICON_TYPE_IMAGE, (uint8_t *) &next_track_byteicon, 16, 16, 0};
Menu_Icon_t play_pause_icon = {MENU_ICON_TYPE_IMAGE, (uint8_t *) &play_pause_byteicon, 16, 16, 0};
Menu_Icon_t previous_track_icon = {MENU_ICON_TYPE_IMAGE, (uint8_t *) &previous_track_byteicon, 16, 16, 0};

Menu_Content_Text_t broadcast_synced_text = {2, {"Scanning", "", "Advertising"}};
Menu_Content_Text_t volume_text = {1, {"Volume"}};
Menu_Content_Text_t media_text = {2, {"Media PAUSED", "Track1"}};
Menu_Content_Text_t call_text = {2, {"Call State", "IDLE"}};

Menu_Page_t *p_broadcast_synced_menu;
Menu_Page_t *p_volume_control_menu;
Menu_Page_t *p_call_control_menu;
Menu_Page_t *p_media_control_menu;
Menu_Page_t *p_broadcast_menu;

uint8_t language_on = 0;
uint8_t connect_status = 0;

/* Private functions prototypes-----------------------------------------------*/
static void Menu_SetMenuList(void);
static void Menu_Volume_Up(void);
static void Menu_Volume_Down(void);
static void Menu_Volume_Mute(void);
static void Menu_Call_Answer(void);
static void Menu_Call_Terminate(void);
static void Menu_Media_NextTrack(void);
static void Menu_Media_PlayPause(void);
static void Menu_Media_PreviousTrack(void);
static void Menu_NextBroadcastSource(void);
static void Menu_SwitchLanguage(void);
static void Menu_StartAdvertising(void);
static void Menu_StopAdvertising(void);
static void Menu_Disconnect(void);
static void Menu_ClearSecDB(void);
/* Exported Functions Definition -------------------------------------------- */
/**
 * @brief Initialize and setup the menu
 */
void Menu_Config(void)
{
  Menu_Action_t broadcast_menu_action = {MENU_ACTION_MENU_PAGE, &access_menu_icon, 0, 0};

  Menu_Action_t volume_control_up_action = {MENU_ACTION_CALLBACK, &volume_up_icon, &Menu_Volume_Up, 0};
  Menu_Action_t volume_control_down_action = {MENU_ACTION_CALLBACK, &volume_down_icon, &Menu_Volume_Down, 0};
  Menu_Action_t volume_control_right_action = {MENU_ACTION_CALLBACK, &volume_mute_icon, &Menu_Volume_Mute, 0};

  Menu_Action_t call_control_up_action = {MENU_ACTION_CALLBACK, &answer_call_icon, &Menu_Call_Answer, 0};
  Menu_Action_t call_control_down_action = {MENU_ACTION_CALLBACK, &terminate_call_icon, &Menu_Call_Terminate, 0};

  Menu_Action_t media_control_up_action = {MENU_ACTION_CALLBACK, &next_track_icon, &Menu_Media_NextTrack, 0};
  Menu_Action_t media_control_down_action = {MENU_ACTION_CALLBACK, &previous_track_icon, &Menu_Media_PreviousTrack, 0};
  Menu_Action_t media_control_right_action = {MENU_ACTION_CALLBACK, &play_pause_icon, &Menu_Media_PlayPause, 0};

  Menu_Init();

  p_broadcast_synced_menu = Menu_CreatePage(MENU_TYPE_CONTROL);
  p_broadcast_menu = Menu_CreatePage(MENU_TYPE_LIST);
  p_volume_control_menu = Menu_CreatePage(MENU_TYPE_CONTROL);
  p_call_control_menu = Menu_CreatePage(MENU_TYPE_CONTROL);
  p_media_control_menu = Menu_CreatePage(MENU_TYPE_CONTROL);

  broadcast_menu_action.pPage = (struct Menu_Page_t*) p_broadcast_menu;

  Menu_SetControlContent(p_volume_control_menu, &volume_text, 0);
  Menu_SetControlAction(p_volume_control_menu, MENU_DIRECTION_UP, volume_control_up_action);
  Menu_SetControlAction(p_volume_control_menu, MENU_DIRECTION_DOWN, volume_control_down_action);
  Menu_SetControlAction(p_volume_control_menu, MENU_DIRECTION_RIGHT, volume_control_right_action);

  Menu_SetControlContent(p_call_control_menu,&call_text, 0);
  Menu_SetControlAction(p_call_control_menu, MENU_DIRECTION_UP, call_control_up_action);
  Menu_SetControlAction(p_call_control_menu, MENU_DIRECTION_DOWN, call_control_down_action);

  Menu_SetControlContent(p_media_control_menu, &media_text, 0);
  Menu_SetControlAction(p_media_control_menu, MENU_DIRECTION_UP, media_control_up_action);
  Menu_SetControlAction(p_media_control_menu, MENU_DIRECTION_DOWN, media_control_down_action);
  Menu_SetControlAction(p_media_control_menu, MENU_DIRECTION_RIGHT, media_control_right_action);

  Menu_SetControlContent(p_broadcast_synced_menu, &broadcast_synced_text, 0);
  Menu_SetControlAction(p_broadcast_synced_menu, MENU_DIRECTION_RIGHT, broadcast_menu_action);

  Menu_SetMenuList();

  TMAPAPP_Init(0);
  Menu_SetActivePage(p_broadcast_synced_menu);
}

/**
 * @brief Set the synchronization state
 */
void Menu_SetSyncState(APP_ScanState_t ScanState, APP_PASyncState_t PASyncState, APP_BIGSyncState_t BIGSyncState)
{
  if (BIGSyncState == APP_BIG_SYNC_STATE_SYNCHRONIZED)
  {
    snprintf(broadcast_synced_text.Lines[0], MENU_CONTROL_MAX_LINE_LEN, "BIS Synchronized");
  }
  else if(BIGSyncState == APP_BIG_SYNC_STATE_SYNCHRONIZING)
  {
    snprintf(broadcast_synced_text.Lines[0], MENU_CONTROL_MAX_LINE_LEN, "BIS Synchronizing");
  }
  else if(PASyncState == APP_PA_SYNC_STATE_SYNCHRONIZING)
  {
    snprintf(broadcast_synced_text.Lines[0], MENU_CONTROL_MAX_LINE_LEN, "PA Synchronizing");
  }
  else if(ScanState == APP_SCAN_STATE_SCANNING)
  {
    snprintf(broadcast_synced_text.Lines[0], MENU_CONTROL_MAX_LINE_LEN, "Scanning");
  }
  else
  {
    snprintf(broadcast_synced_text.Lines[0], MENU_CONTROL_MAX_LINE_LEN, "Idle");
  }
  Menu_Print();
}

/**
 * @brief Set the broadcast source name
 */
void Menu_SetBroadcastSourceName(const char* pBroadcastSourceName)
{
  snprintf(broadcast_synced_text.Lines[1], MENU_CONTROL_MAX_LINE_LEN, "%s", pBroadcastSourceName);
  Menu_Print();
}

/**
 * @brief Set the connection state
 */
void Menu_SetConnectionState(uint8_t ConnectionState)
{
  switch (ConnectionState)
  {
    case 0:
      /* Device is disconnected */
      if (connect_status > 0u)
      {
        broadcast_synced_text.NumLines --;
        connect_status = 0u;
        Menu_SetMenuList();
      }
      break;

    case 1:
      /* Device is connected */
      snprintf(broadcast_synced_text.Lines[broadcast_synced_text.NumLines - 1], MENU_CONTROL_MAX_LINE_LEN, "Connected");
      connect_status = 2u;
      Menu_SetMenuList();
      break;

    case 2:
      /* Device is streaming in unicast mode */
      snprintf(broadcast_synced_text.Lines[broadcast_synced_text.NumLines - 1], MENU_CONTROL_MAX_LINE_LEN, "Unicast Streaming");
      connect_status = 3u;
      Menu_SetMenuList();
      break;

  }
  Menu_Print();
}

/**
 * @brief Set language
 */
void Menu_SetLanguage(char* pLanguage)
{
  if (pLanguage != 0)
  {
    if (language_on == 0)
    {
      if (connect_status > 0u)
      {
        snprintf(broadcast_synced_text.Lines[3], MENU_CONTROL_MAX_LINE_LEN, "%s", broadcast_synced_text.Lines[2]);
      }
      broadcast_synced_text.NumLines++;
      language_on = 1u;
      Menu_SetMenuList();
    }

    memcpy(broadcast_synced_text.Lines[2], pLanguage, 3);
    broadcast_synced_text.Lines[2][3] = '\0';
  }
  else
  {
    if (language_on == 1u)
    {
      if (connect_status > 0u)
      {
        snprintf(broadcast_synced_text.Lines[2], MENU_CONTROL_MAX_LINE_LEN, "%s", broadcast_synced_text.Lines[3]);
      }
      broadcast_synced_text.NumLines--;
      language_on = 0u;
      Menu_SetMenuList();
    }
  }

  Menu_Print();
}

/**
 * @brief Set Current volume value
 */
void Menu_SetVolume(uint8_t Volume)
{
  snprintf(volume_text.Lines[0], MENU_CONTROL_MAX_LINE_LEN, "Volume: %d%%", (uint8_t) (((uint16_t) Volume)*100/255));
  Menu_Print();
}

/**
 * @brief Set Call State
 */
void Menu_SetCallState(char *pCallState)
{
  snprintf(call_text.Lines[1], MENU_CONTROL_MAX_LINE_LEN, "%s", pCallState);
  Menu_Print();
}

/**
 * @brief Set Media State
 */
void Menu_SetMediaState(char *pMediaState)
{
  snprintf(media_text.Lines[0], MENU_CONTROL_MAX_LINE_LEN, "Media %s", pMediaState);
  Menu_Print();
}

/**
 * @brief Set Track Title
 */
void Menu_SetTrackTitle(uint8_t *pTrackTitle, uint8_t TrackNameLen)
{
  if (TrackNameLen <= 14)
  {
    snprintf(media_text.Lines[1], TrackNameLen +1, "%s", pTrackTitle);
  }
  else
  {
    char display_string[15];
    snprintf(display_string, 12, "%s", pTrackTitle);
    display_string[11] = '.';
    display_string[12] = '.';
    display_string[13] = '.';
    display_string[14] = '\0';
    snprintf(media_text.Lines[1], 15, "%s", display_string);
  }
  Menu_Print();
}

/* Private Functions Definition --------------------------------------------- */

/**
 * @brief Volume Up Callback
 */
static void Menu_Volume_Up(void)
{
  LOG_INFO_APP("[APP_MENU_CONF] Volume+\n");
  TMAPAPP_VolumeUp();
}

/**
 * @brief Volume Down Callback
 */
static void Menu_Volume_Down(void)
{
  LOG_INFO_APP("[APP_MENU_CONF] Volume-\n");
  TMAPAPP_VolumeDown();
}

/**
 * @brief Volume Mute Callback
 */
static void Menu_Volume_Mute(void)
{
  LOG_INFO_APP("[APP_MENU_CONF] Volume Mute\n");
  TMAPAPP_ToggleMute();
}
/**
 * @brief Call Answer Callback
 */
static void Menu_Call_Answer(void)
{
  LOG_INFO_APP("[APP_MENU_CONF] Answer Call\n");
  TMAPAPP_AnswerCall();
}

/**
 * @brief Call terminate Callback
 */
static void Menu_Call_Terminate(void)
{
  LOG_INFO_APP("[APP_MENU_CONF] Terminate Call\n");
  TMAPAPP_TerminateCall();
}

/**
 * @brief Next Track Callback
 */
static void Menu_Media_NextTrack(void)
{
  LOG_INFO_APP("[APP_MENU_CONF] Next Track\n");
  TMAPAPP_NextTrack();
}

/**
 * @brief Play Pause Callback
 */
static void Menu_Media_PlayPause(void)
{
  LOG_INFO_APP("[APP_MENU_CONF] Play/Pause\n");
  TMAPAPP_PlayPause();
}

/**
 * @brief Previous track Callback
 */
static void Menu_Media_PreviousTrack(void)
{
  LOG_INFO_APP("[APP_MENU_CONF] Previous Track\n");
  TMAPAPP_PreviousTrack();
}

/**
 * @brief Clear Security Database Callback
 */
static void Menu_ClearSecDB(void)
{
  LOG_INFO_APP("[APP_MENU_CONF] Clear Security DB\n");
  aci_gap_clear_security_db();
  TMAPAPP_ClearDatabase();
  Menu_SetActivePage(p_broadcast_synced_menu);
}

static void Menu_NextBroadcastSource(void)
{
  LOG_INFO_APP("[APP_MENU_CONF] Next Broadcast Source\n");

  TMAPAPP_StopSink();
  TMAPAPP_NextSource();
  TMAPAPP_StartSink();

  Menu_SetActivePage(p_broadcast_synced_menu);
}

static void Menu_SwitchLanguage(void)
{
  LOG_INFO_APP("[APP_MENU_CONF] Switch Language\n");

  TMAPAPP_StopSink();
  TMAPAPP_SwitchLanguage();
  TMAPAPP_StartSink();

  Menu_SetActivePage(p_broadcast_synced_menu);
}

static void Menu_StartAdvertising(void)
{
  LOG_INFO_APP("[APP_MENU_CONF] Start Advertising\n");

  if (connect_status == 0)
  {
    uint8_t status;
    UNUSED(status);
    status = TMAPAPP_StartAdvertising(CAP_GENERAL_ANNOUNCEMENT, 1, GAP_APPEARANCE_STANDALONE_SPEAKER);
    LOG_INFO_APP("TMAPAPP_StartAdvertising() returns status 0x%02X\n",status);

    snprintf(broadcast_synced_text.Lines[broadcast_synced_text.NumLines], MENU_CONTROL_MAX_LINE_LEN, "Advertising");
    broadcast_synced_text.NumLines++;
    connect_status = 1u;
    Menu_SetMenuList();
  }

  Menu_SetActivePage(p_broadcast_synced_menu);
}

static void Menu_Disconnect(void)
{
  LOG_INFO_APP("[APP_MENU_CONF] Disconnect\n");

  if (connect_status == 2)
  {
    uint8_t status;
    UNUSED(status);
    status = TMAPAPP_Disconnect();
    LOG_INFO_APP("TMAPAPP_Disconnect() returns status 0x%02X\n",status);

    broadcast_synced_text.NumLines --;
    connect_status = 0u;
    Menu_SetMenuList();
  }

  Menu_SetActivePage(p_broadcast_synced_menu);
}

static void Menu_StopAdvertising(void)
{
  LOG_INFO_APP("[APP_MENU_CONF] Stop Advertising\n");

  if (connect_status == 1)
  {
    uint8_t status;
    UNUSED(status);
    status = TMAPAPP_StopAdvertising();
    LOG_INFO_APP("TMAPAPP_StopAdvertising() returns status 0x%02X\n",status);

    broadcast_synced_text.NumLines --;
    connect_status = 0u;
    Menu_SetMenuList();
  }

  Menu_SetActivePage(p_broadcast_synced_menu);
}

static void Menu_SetMenuList(void)
{
  Menu_Action_t next_broadcast_source_action = {MENU_ACTION_CALLBACK, &access_menu_icon, &Menu_NextBroadcastSource, 0};
  Menu_Action_t switch_language_action = {MENU_ACTION_CALLBACK, &access_menu_icon, &Menu_SwitchLanguage, 0};
  Menu_Action_t start_advertising_action = {MENU_ACTION_CALLBACK, &access_menu_icon, &Menu_StartAdvertising, 0};
  Menu_Action_t stop_advertising_action = {MENU_ACTION_CALLBACK, &access_menu_icon, &Menu_StopAdvertising, 0};
  Menu_Action_t disconnect_action = {MENU_ACTION_CALLBACK, &access_menu_icon, &Menu_Disconnect, 0};
  Menu_Action_t entry_menu_volume_action = {MENU_ACTION_MENU_PAGE, &access_menu_icon, 0, (struct Menu_Page_t*) p_volume_control_menu};
  Menu_Action_t entry_menu_call_action = {MENU_ACTION_MENU_PAGE, &access_menu_icon, 0, (struct Menu_Page_t*) p_call_control_menu};
  Menu_Action_t entry_menu_media_action = {MENU_ACTION_MENU_PAGE, &access_menu_icon, 0, (struct Menu_Page_t*) p_media_control_menu};
  Menu_Action_t clear_sec_db_action = {MENU_ACTION_CALLBACK, &access_menu_icon, &Menu_ClearSecDB, 0};

  Menu_ClearList(p_broadcast_menu);
  Menu_AddListEntry(p_broadcast_menu, "Next Source", next_broadcast_source_action);
  if (language_on == 1)
  {
    Menu_AddListEntry(p_broadcast_menu, "Switch Language", switch_language_action);
  }
  switch (connect_status)
  {
    case 0:
      Menu_AddListEntry(p_broadcast_menu, "Start Adv.", start_advertising_action);
      break;
    case 1:
      Menu_AddListEntry(p_broadcast_menu, "Stop Adv.", stop_advertising_action);
      break;
    case 2:
    case 3:
      Menu_AddListEntry(p_broadcast_menu, "Disconnect", disconnect_action);
      Menu_AddListEntry(p_broadcast_menu, "Call...", entry_menu_call_action);
      Menu_AddListEntry(p_broadcast_menu, "Media...", entry_menu_media_action);
      break;
  }
  Menu_AddListEntry(p_broadcast_menu, "Volume...", entry_menu_volume_action);
  Menu_AddListEntry(p_broadcast_menu, "Clear Sec Db", clear_sec_db_action);

  Menu_Print();
}
