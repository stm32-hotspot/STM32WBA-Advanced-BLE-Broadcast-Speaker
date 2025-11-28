/**
  ******************************************************************************
  * @file    board_mngr.c
  * @author  MCD Application Team
  * @brief   file for whole system and PCB board management
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

#include "tmap_app.h"
#include "ble_gap_aci.h"
#include "board_mngr.h"
#include "log_module.h"

#include "ltc3556.h"
#include "max9867.h"
#include "H5com.h"

#if (BAT_MNGMT == 1)
#include "stc3115_Driver.h"
#endif /* BAT_MNGMT == 1 */

/* Private defines -----------------------------------------------------------*/
/* high level application requests */
#define APP_REQ_BAT_READ      0x00000001
#define APP_REQ_BP1           0x00000002
#define APP_REQ_BP2           0x00000004
#define APP_REQ_BP3           0x00000008
#define APP_REQ_BP_PWR        0x00000010
#define APP_REQ_LONG_BP1      0x00000020
#define APP_REQ_LONG_BP2      0x00000040
#define APP_REQ_LONG_BP3      0x00000080
#define APP_REQ_LONG_BP_PWR   0x00000100

#define APP_REQ_RUN_MIXMOD    0x00000200
#define APP_REQ_STOP_MIXMOD   0x00000400

#define APP_REQ_SHUTDOWN      0x00010000
#define APP_REQ_STOP_ADV      0x00100000
#define APP_REQ_START_ADV     0x00200000
#define APP_REQ_SCAN          0x00400000

/* system states */
/* ble states */
#define BLE_STATE_MASK          0x00ff

#define APP_STATE_IDLE          0x0000
#define APP_STATE_ADV           0x0001
#define APP_STATE_CON           0x0002
#define APP_STATE_SCAN          0x0004
#define APP_STATE_BIG_SYNC      0x0008

#define APP_STATE_STREAM_TEL    0x0010  /* connected telephony */
#define APP_STATE_STREAM_MED    0x0020  /* connected media */
#define APP_STATE_STREAM_SNK    0x0040  /* synchronized */

#define APP_STATE_ANNOUNCE      0x0100

/* board states */
#define APP_STATE_OVERHEAT      0x0800
#define APP_STATE_LOW_BAT       0x1000
#define APP_STATE_VERY_LOW_BAT  0x2000
#define APP_STATE_MEDIUM_ERR    0x4000
#define APP_STATE_CRITICAL_ERR  0x8000


#define IS(var, VAL) ((var & VAL) == (VAL))

#define ENABLE_1V8()      HAL_GPIO_WritePin(SPEAKER_1V8_EN_PORT, SPEAKER_1V8_EN_PIN, GPIO_PIN_SET);
#define DISABLE_1V8()     HAL_GPIO_WritePin(SPEAKER_1V8_EN_PORT, SPEAKER_1V8_EN_PIN, GPIO_PIN_RESET);

#define STC_I2C_TIMEOUT        10
/* Private Typedef ----------------------------------------------------------*/
typedef enum
{
  LED_OFF   = 0x00,
  LED_RED   = 0x01,
  LED_GREEN = 0x02,
  LED_ORANGE= 0x03,
  LED_BLUE  = 0x04,
  LED_PINK  = 0x05,
  LED_CIAN  = 0x06,
  LED_WHITH = 0x07,
} LED_Color_t;

typedef enum
{
  NO_ERR,
  APP_ERR,
  SYSTEM_OVERHEAT_ERR,
  COM_LTC_ERR,
  COM_MAX9867_ERR,
  COM_STC3115_ERR,
  COM_H5_ERR,
  COM_MAX17055_ERR,
} Board_Error_t;

typedef struct
{
  uint16_t voltage;
  int16_t current;
  int16_t temperature;
  int16_t level;
  bool is_charging;
} bat_t;

typedef struct
{
  uint8_t state;
  uint32_t duration_on;
  uint32_t duration_off;
  LED_Color_t color;
} leddriver_t;

typedef struct
{
  Button_t id;
  uint8_t started;
  uint8_t longpress;
} button_push_t;

/* Private Function ----------------------------------------------------------*/
static void board_gpio_init(void);
static void board_process(void);
static void board_error(Board_Error_t error);

static void speaker_audio_announce(uint8_t announcement_cmd);
static void speaker_audio_end_announce(void);

static void boardApp_timerCallback(void* arg);
static void led_timerCallback(void* arg);
static void button_timerCallback(void* arg);

static void speaker_update_IHM(void);
static void speaker_set_led(LED_Color_t color, uint32_t duration_on, uint32_t duration_off);

/* Global variables ----------------------------------------------------------*/
extern I2C_HandleTypeDef hi2c3;
extern TIM_HandleTypeDef htim1;

/* Private variables ---------------------------------------------------------*/
static UTIL_TIMER_Object_t check_timer;
static UTIL_TIMER_Object_t led_timer;
static UTIL_TIMER_Object_t button_timer;

static bat_t Battery = {0};
static leddriver_t Led = {0};
static button_push_t ActiveButton;

static uint32_t Counter_BatReading = 0;
static uint32_t Counter_ADV = 0;
static uint32_t Counter_Announcement = 0;
static uint32_t Counter_Activity = 0;

static uint8_t Announcement_is_timeout;

static uint32_t AppState = APP_STATE_IDLE;
static uint32_t AppRequest = 0;

#if (BAT_MNGMT == 1)
static STC3115_ConfigData_TypeDef STC3115_ConfigData;
static STC3115_BatteryData_TypeDef STC3115_BatteryData;
#endif /* BAT_MNGMT == 1 */

static int8_t Local_volume = VOLUME_INIT;

static void board_gpio_init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /*** PUSH BUTTONS ***/
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Pin = SPEAKER_BP1_PIN|SPEAKER_BP2_PIN;
  HAL_GPIO_Init(SPEAKER_BP1_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = SPEAKER_BP3_PIN;
  HAL_GPIO_Init(SPEAKER_BP3_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = SPEAKER_BP_POWER_PIN;
  HAL_GPIO_Init(SPEAKER_BP_POWER_PORT, &GPIO_InitStruct);

  /*** INTERRUPTS ***/
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pin = SPEAKER_FUELG_IT_PIN;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(SPEAKER_FUELG_IT_PORT, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = SPEAKER_CHG_IT_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(SPEAKER_CHG_IT_PORT, &GPIO_InitStruct);

  /*** LEDS ***/
  /* init by the LED drivers only if needed */
  /*HAL_GPIO_WritePin(SPEAKER_LED_PORT, SPEAKER_LED_RED_PIN|SPEAKER_LED_GREEN_PIN|SPEAKER_LED_BLUE_PIN, GPIO_PIN_SET);

  GPIO_InitStruct.Pin = SPEAKER_LED_RED_PIN|SPEAKER_LED_GREEN_PIN|SPEAKER_LED_BLUE_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SPEAKER_LED_PORT, &GPIO_InitStruct);*/


  /*HAL_GPIO_WritePin(SPEAKER_COMP_WPUP_PORT, SPEAKER_COMP_WKUP_PIN, GPIO_PIN_RESET);
  GPIO_InitStruct.Pin = SPEAKER_COMP_WKUP_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SPEAKER_COMP_WPUP_PORT, &GPIO_InitStruct);*/


  /*** CONFIG ***/
  DISABLE_1V8();

  GPIO_InitStruct.Pin = SPEAKER_1V8_EN_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  HAL_GPIO_Init(SPEAKER_1V8_EN_PORT, &GPIO_InitStruct);

  /* debug */
  /*GPIO_InitStruct.Pin = SPEAKER_COMP_COM_PIN_RX;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SPEAKER_COMP_COM_PORT, &GPIO_InitStruct);*/


  HAL_NVIC_SetPriority(SPEAKER_BP3_IT, BP_INTERRUPT_PRIO, 0);
  HAL_NVIC_EnableIRQ(SPEAKER_BP3_IT);

  HAL_NVIC_SetPriority(SPEAKER_BP_POWER_IT, BP_INTERRUPT_PRIO, 0);
  HAL_NVIC_EnableIRQ(SPEAKER_BP_POWER_IT);

  HAL_NVIC_SetPriority(SPEAKER_BP1_IT, BP_INTERRUPT_PRIO, 0);
  HAL_NVIC_EnableIRQ(SPEAKER_BP1_IT);

  HAL_NVIC_SetPriority(SPEAKER_BP2_IT, BP_INTERRUPT_PRIO, 0);
  HAL_NVIC_EnableIRQ(SPEAKER_BP2_IT);

  HAL_NVIC_SetPriority(EXTI10_IRQn, COMP_INTERRUPT_PRIO, 0);
  HAL_NVIC_EnableIRQ(EXTI10_IRQn);

  HAL_NVIC_SetPriority(EXTI13_IRQn, COMP_INTERRUPT_PRIO, 0);
  HAL_NVIC_EnableIRQ(EXTI13_IRQn);

  /* visual boot */
  speaker_set_led(LED_GREEN, 0, 0);
  HAL_Delay(200);
  speaker_set_led(LED_OFF, 0, 0);

}

static void speaker_set_led(LED_Color_t color, uint32_t duration_on, uint32_t duration_off)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  uint32_t pin_on = 0;
  uint32_t pin_off = 0;

  UTIL_TIMER_Stop(&led_timer);

  /* RED */
  if (color & LED_RED)
  {
    pin_on |= SPEAKER_LED_RED_PIN;
  }
  else
  {
    pin_off |= SPEAKER_LED_RED_PIN;
  }

  /* GREEN */
  if (color & LED_GREEN)
  {
    pin_on |= SPEAKER_LED_GREEN_PIN;
  }
  else
  {
    pin_off |= SPEAKER_LED_GREEN_PIN;
  }

  /* BLUE */
  if (color & LED_BLUE)
  {
    pin_on |= SPEAKER_LED_BLUE_PIN;
  }
  else
  {
    pin_off |= SPEAKER_LED_BLUE_PIN;
  }

  /* configure LED that are on */
  if (pin_on)
  {
    HAL_GPIO_WritePin(SPEAKER_LED_PORT, pin_on, GPIO_PIN_RESET);

    GPIO_InitStruct.Pin = pin_on;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(SPEAKER_LED_PORT, &GPIO_InitStruct);
  }

  /* release LED that are Off */
  if( pin_off)
  {
    GPIO_InitStruct.Pin = pin_off;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(SPEAKER_LED_PORT, &GPIO_InitStruct);
  }

  if (color != LED_OFF)
  {
    Led.state = 1;
    Led.duration_on = duration_on;
    Led.duration_off = duration_off;
    Led.color = color;
  }

  if ((duration_on != 0))
  {
    UTIL_TIMER_StartWithPeriod(&led_timer, duration_on);
  }
}

static void led_timerCallback(void* arg)
{
  if(Led.state == 1)
  {
    /* Led was on, shutdown it for the duration off */
    Led.state = 0;
    speaker_set_led(LED_OFF, 0, 0);
    UTIL_TIMER_StartWithPeriod(&led_timer, Led.duration_off);
  }
  else
  {
    /* Led was off, restart cycle */
    speaker_set_led(Led.color, Led.duration_on, Led.duration_off);
  }
}


uint8_t BoardInit (void)
{
  SysVoltage_t vdd_voltage;

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* reset database if this button is pushed at init */
  __HAL_RCC_GPIOB_CLK_ENABLE();
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Pin = SPEAKER_BP1_PIN;
  HAL_GPIO_Init(SPEAKER_BP1_PORT, &GPIO_InitStruct);
  if (HAL_GPIO_ReadPin(SPEAKER_BP1_PORT, SPEAKER_BP1_PIN) == GPIO_PIN_SET)
  {
    LOG_INFO_APP("Clear security database\n");
    aci_gap_clear_security_db();
    TMAPAPP_ClearDatabase();
  }

  board_gpio_init();

#if (USB_LIVETUNE == 1)
  vdd_voltage = V_3V3; /* requiered for USB com */
#else
  vdd_voltage = V_2V2;
#endif /* USB_LIVETUNE == 1 */

  if (LTC3556_init(&hi2c3, vdd_voltage) !=0)
  {
    board_error(COM_LTC_ERR);
  }

#if (BAT_MNGMT == 1)
  /* disable charging before gaz gauge initialization */
  LTC3556_disable_chrg(1);

  /* battery voltage stabilisation */
  HAL_Delay(1000);

  if (GasGauge_Initialization(&STC3115_ConfigData, &STC3115_BatteryData) != 0)
  {
    board_error(COM_STC3115_ERR);
  }

  STC3115_SetPowerSavingMode();

  /* enable charging after OCV reading */
  LTC3556_disable_chrg(0);
#endif  /* (BAT_MNGMT == 1) */

  /* test communication with the codec, no configuration here */
  ENABLE_1V8();
  HAL_Delay(50); //boot
  if (MAX9867_Init(&hi2c3) !=0)
  {
    board_error(COM_MAX9867_ERR);
  }
  DISABLE_1V8();
  /* H5 automatically set in standby */

#if (EW25DEMO == 1) /* avoid pop for the demo */
  if (LTC3556_set_5V_on() != 0)
  {
    board_error(COM_LTC_ERR);
  }
#endif

  /* create timers */
  if (UTIL_TIMER_Create(&check_timer, CHECKING_PERIOD_MS, UTIL_TIMER_PERIODIC, &boardApp_timerCallback,0) != UTIL_TIMER_OK)
  {
    board_error(APP_ERR);
  }

  if (UTIL_TIMER_Start(&check_timer)!= UTIL_TIMER_OK)
  {
    board_error(APP_ERR);
  }

  if (UTIL_TIMER_Create(&led_timer, 100, UTIL_TIMER_ONESHOT, &led_timerCallback,0) != UTIL_TIMER_OK)
  {
    board_error(APP_ERR);
  }

  if (UTIL_TIMER_Create(&button_timer, 100, UTIL_TIMER_ONESHOT, &button_timerCallback,0) != UTIL_TIMER_OK)
  {
    board_error(APP_ERR);
  }

  TMAPAPP_Init(CSIP_CONF_ID);

  UTIL_SEQ_RegTask(1U << CFG_TASK_BOARD_MNGR_ID, UTIL_SEQ_RFU, board_process);

  AppState = APP_STATE_IDLE;
#if (EW25DEMO == 1)
  AppRequest |= APP_REQ_SCAN;
#else
  AppRequest |= APP_REQ_START_ADV;
#endif
  AppRequest |= APP_REQ_BAT_READ;

  /* variable initialization in case com fail */
  Battery.level = 50;
  Battery.temperature = 20;

  Local_volume = VOLUME_INIT;

  /* execute a first time */
  UTIL_SEQ_SetTask(1U<<CFG_TASK_BOARD_MNGR_ID, CFG_SEQ_PRIO_0);

  return 0;
}

static void boardApp_timerCallback(void* arg)
{
  /* Periodic Battery reading */
  if (Counter_BatReading >= (BAT_READING_PERIOD_SEC * 1000 / CHECKING_PERIOD_MS))
  {
    AppRequest |= APP_REQ_BAT_READ;
    Counter_BatReading = 0;
  }
  Counter_BatReading++;

  /* Inactivity counter */
  if ((AppState & BLE_STATE_MASK) == APP_STATE_IDLE)
  {
    if (Counter_Activity >= (NO_ACTIVITY_SHTDWN_SEC * 1000 / CHECKING_PERIOD_MS))
    {
#if (RELEASE == 1)
      AppRequest |= APP_REQ_SHUTDOWN;
#endif /* RELEASE == 1 */
      Counter_Activity = 0;
    }
    Counter_Activity++;
  }

  /* Adv timeout */
  if (IS(AppState, APP_STATE_ADV))
  {
    Counter_ADV++;
    if (Counter_ADV >= (ADV_TIMEOUT_SEC * 1000 / CHECKING_PERIOD_MS))
    {
      Counter_ADV = 0;
      AppRequest |= APP_REQ_STOP_ADV;
    }
  }

  /* announcement timeout */
  if (IS(AppState, APP_STATE_ANNOUNCE))
  {
    Counter_Announcement++;
    if (Counter_Announcement >= (ANNOUNCEMENT_TIMEOUT_SEC * 1000 / CHECKING_PERIOD_MS))
    {
      Counter_Announcement = 0;
      Announcement_is_timeout = 1;
    }
  }

  /* request the sequencer to execute */
  UTIL_SEQ_SetTask(1U<<CFG_TASK_BOARD_MNGR_ID, CFG_SEQ_PRIO_0);
}


/**
  * @brief Main task
  * @param none
  * @retval none
  */
static void board_process(void)
{
   __HAL_RCC_I2C3_CLK_ENABLE();
   /*
   ***************** BUTTON REQUEST *****************
   */
  if ((AppRequest & (APP_REQ_BP1 | APP_REQ_BP2 | APP_REQ_BP3)))
  {
    if (((AppState & BLE_STATE_MASK) == APP_STATE_IDLE) && (Battery.level > BAT_LVL_LOW))
    {
      AppRequest |= APP_REQ_START_ADV;
    }
  }

  if ((AppRequest & APP_REQ_BP1) ==  APP_REQ_BP1)
  {
    AppRequest &= ~APP_REQ_BP1;
    if (IS(AppState, APP_STATE_STREAM_MED) || IS(AppState, APP_STATE_STREAM_TEL) || IS(AppState, APP_STATE_STREAM_SNK))
    {
     TMAPAPP_VolumeUp();
    }
  }


  if ((AppRequest & APP_REQ_LONG_BP1) ==  APP_REQ_LONG_BP1)
  {
    AppRequest &= ~APP_REQ_LONG_BP1;

    if (IS(AppState, APP_STATE_STREAM_MED))
    {
      TMAPAPP_PreviousTrack();
    }

    if (IS(AppState, APP_STATE_STREAM_TEL))
    {
      TMAPAPP_TerminateCall();
    }

    if (IS(AppState, APP_STATE_STREAM_SNK) || IS(AppState, APP_STATE_SCAN))
    {
      //speaker_audio_announce(H5APP_PLAY_BIP);
      speaker_set_led(LED_GREEN, 0, 0);
      HAL_Delay(300);
      TMAPAPP_StopSink();
      TMAPAPP_NextSource();
      TMAPAPP_StartSink();
    }
  }


  if ((AppRequest & APP_REQ_BP2) ==  APP_REQ_BP2)
  {
    AppRequest &= ~APP_REQ_BP2;

    if (IS(AppState, APP_STATE_STREAM_MED) || IS(AppState, APP_STATE_STREAM_TEL) || IS(AppState, APP_STATE_STREAM_SNK))
    {
      TMAPAPP_ToggleMute();
    }
  }


  if ((AppRequest & APP_REQ_LONG_BP2) ==  APP_REQ_LONG_BP2)
  {
    AppRequest &= ~APP_REQ_LONG_BP2;

#if (EW25DEMO == 1)
    speaker_set_led(LED_GREEN, 0, 0);
    HAL_Delay(300);
    AppRequest |= APP_REQ_START_ADV; /* scan delegator */
#endif
  }


  if ((AppRequest & APP_REQ_BP3) ==  APP_REQ_BP3)
  {
    AppRequest &= ~APP_REQ_BP3;

    if (IS(AppState, APP_STATE_STREAM_MED) || IS(AppState, APP_STATE_STREAM_TEL) || IS(AppState, APP_STATE_STREAM_SNK))
    {
     TMAPAPP_VolumeDown();
    }
  }


  if ((AppRequest & APP_REQ_LONG_BP3) ==  APP_REQ_LONG_BP3)
  {
    AppRequest &= ~APP_REQ_LONG_BP3;
    if (IS(AppState, APP_STATE_STREAM_MED))
    {
      TMAPAPP_NextTrack();
    }

    if (IS(AppState, APP_STATE_STREAM_TEL))
    {
      TMAPAPP_AnswerCall();
    }

    if (IS(AppState, APP_STATE_STREAM_SNK) || IS(AppState, APP_STATE_SCAN))
    {
      /*speaker_audio_announce(H5APP_PLAY_BIP);*/
      speaker_set_led(LED_GREEN, 0, 0);
      HAL_Delay(300);
      TMAPAPP_StopSink();
      TMAPAPP_SwitchLanguage();
      TMAPAPP_StartSink();
    }
  }


  if ((AppRequest & APP_REQ_BP_PWR) ==  APP_REQ_BP_PWR)
  {
    AppRequest &= ~APP_REQ_BP_PWR;
    speaker_audio_announce(H5APP_PLAY_EXE); /* for demonstration only */
  }


  if ((AppRequest & APP_REQ_LONG_BP_PWR) ==  APP_REQ_LONG_BP_PWR)
  {
    AppRequest &= ~APP_REQ_LONG_BP_PWR;
    AppRequest |= APP_REQ_SHUTDOWN;
  }


  /*
   ***************** POWER MGMT REQUEST *****************
   */
  if ((AppRequest & APP_REQ_SHUTDOWN) ==  APP_REQ_SHUTDOWN)
  {
    AppRequest &= ~APP_REQ_SHUTDOWN;
    speaker_set_led(LED_ORANGE, 0, 0);

    uint8_t status = 1;
    uint8_t retry = 3;

    while((status != 0) && (retry != 0))
    {
#if (BAT_MNGMT == 1)
      STC3115_SetPowerSavingMode();
#endif /* BAT_MNGMT */
      status = LTC3556_set_all_off();
      /* board beeing shutdown, exept if command not send */
      HAL_Delay(50);
      if (status != 0)
      {
        /* try recovery of the I2C bus */
        H5_wakeup();
      }
    }
    board_error(COM_LTC_ERR);
    /* no issue ... */
#if (RELEASE == 1)
    HAL_Delay(10000);
    HAL_NVIC_SystemReset();
#endif /* RELEASE == 1 */
  }


  if (AppRequest & APP_REQ_BAT_READ)
  {
    AppRequest &= ~APP_REQ_BAT_READ;
#if (BAT_MNGMT == 1)
    if (GasGauge_Task(&STC3115_ConfigData, &STC3115_BatteryData) == -1)
    {
      board_error(COM_STC3115_ERR);
    }
    else
    {
      Battery.level = STC3115_BatteryData.SOC / 10;
      Battery.temperature = STC3115_BatteryData.Temperature / 10;

      Battery.voltage = STC3115_BatteryData.Voltage;
      Battery.current = STC3115_BatteryData.Current;


      if (Battery.current <= HIGH_CURRENT_MODE)
      {
        LTC3556_set_5V_high_load(1);
      }
      else
      {
        LTC3556_set_5V_high_load(0);
      }

      /* charging state based on current with hysteresis */
      /*if (Battery.current > BAT_CURRENT_THRHOLD)
      {
        Battery.is_charging = 1;
      }
      else if (Battery.current <= 0)
      {
        Battery.is_charging = 0;
      }*/

      /* temperature state */
      if (Battery.temperature > BAT_MAX_TEMPERATURE)
      {
        board_error(SYSTEM_OVERHEAT_ERR);
        AppState |= APP_STATE_OVERHEAT;
        LOG_INFO_APP("SPEAKER Warning, overheat !\n");
      }else{
        AppState &= ~APP_STATE_OVERHEAT;
      }
      /* battery level */
      if (Battery.level < BAT_LVL_CRITICAL)
      {
        LOG_INFO_APP("SPEAKER Warning, battery critical ... shutting down !\n");
        AppState &= ~ (APP_STATE_LOW_BAT);
        AppState |= APP_STATE_VERY_LOW_BAT;
        AppRequest |= APP_REQ_SHUTDOWN;
      }
      else if (Battery.level < BAT_LVL_VERY_LOW)
      {
        if (IS(AppState,  APP_STATE_VERY_LOW_BAT))
        {
          /* no change in state */
        }
        else
        {
          /* comming from low bat state */
          LOG_INFO_APP("SPEAKER Warning, very low battery !\n");
          AppState &= ~ (APP_STATE_LOW_BAT);
          AppState |= APP_STATE_VERY_LOW_BAT;
          speaker_audio_announce(H5APP_PLAY_LOWBAT);
          /* remove BLE */
          if (IS(AppState,  APP_STATE_ADV))
          {
            TMAPAPP_Disconnect();
          }
          else if (IS(AppState,  APP_STATE_ADV))
          {
            TMAPAPP_StopAdvertising();
          }
        }
      }
      else if (Battery.level < BAT_LVL_LOW)
      {
        if (IS(AppState,  APP_STATE_LOW_BAT))
        {
          /* no change in state */
        }
        else if (IS(AppState,  APP_STATE_VERY_LOW_BAT))
        {
          /* from very low bat state */
          AppState &= ~ (APP_STATE_VERY_LOW_BAT);
          AppState |= APP_STATE_LOW_BAT;
        }
        else
        {
          LOG_INFO_APP("SPEAKER Warning, low battery\n");
          /* comming from normal state */
          AppState |= APP_STATE_LOW_BAT;
          speaker_audio_announce(H5APP_PLAY_LOWBAT);
        }
      }else{
        AppState &= ~ (APP_STATE_VERY_LOW_BAT | APP_STATE_LOW_BAT);
      }
    }
#endif /* (BAT_MNGMT == 1) */
  }


  if (AppRequest & APP_REQ_RUN_MIXMOD)
  {
    AppRequest &= ~APP_REQ_RUN_MIXMOD;
#if (BAT_MNGMT == 1)
    STC3115_StopPowerSavingMode();
#endif /* (BAT_MNGMT == 1)  */
  }


  if (AppRequest & APP_REQ_STOP_MIXMOD)
  {
    AppRequest &= ~APP_REQ_STOP_MIXMOD;
#if (BAT_MNGMT == 1)
    STC3115_SetPowerSavingMode();
#endif
  }

  /*
   ***************** BLUETOOTH REQUEST *****************
   */
  if ((AppRequest & APP_REQ_START_ADV) ==  APP_REQ_START_ADV)
  {
    AppRequest &= ~APP_REQ_START_ADV;

    if (!IS(AppState,  APP_STATE_ADV))
    {
      TMAPAPP_StartAdvertising(CAP_GENERAL_ANNOUNCEMENT,1,GAP_APPEARANCE_STANDALONE_SPEAKER);
    }
    Counter_ADV = 0;
    speaker_update_IHM();
  }


  if ((AppRequest & APP_REQ_STOP_ADV) ==  APP_REQ_STOP_ADV)
  {
    AppRequest &= ~APP_REQ_STOP_ADV;

    TMAPAPP_StopAdvertising();
  }


  if ((AppRequest & APP_REQ_SCAN) ==  APP_REQ_SCAN)
  {
    AppRequest &= ~APP_REQ_SCAN;

    TMAPAPP_StartSink();
  }


  /*
   ***************** STATE BASED REQUEST *****************
   */
  if (IS(AppState, APP_STATE_ANNOUNCE))
  {
    uint8_t H5status;
    H5_read_status(&H5status);

    if (Announcement_is_timeout == 1)
    {
      LOG_INFO_APP("WARNING : announcement timeout\n");
      Announcement_is_timeout = 0;
    }

    if (((H5status & H5APP_STATE_BUSY) == 0) && (H5status & H5APP_STATE_LOCAL_PLAY) == 0)
    {
      speaker_audio_end_announce();
    }

    if (H5status == H5APP_STATE_IDLE)
    {
      /* error, should not be reached */
      AppState &= ~APP_STATE_ANNOUNCE;
    }
  }

  __HAL_RCC_I2C3_CLK_DISABLE();
  //speaker_update_IHM();
}


void App_Notify_Evt(Tmapapp_Event_t event)
{
  Counter_Activity = 0;

  switch (event)
  {
  case START_ADVERTISING:
    AppState |= APP_STATE_ADV;
    speaker_audio_announce(H5APP_PLAY_ADV);
    speaker_update_IHM();
    break;

  case STOP_ADVERTISING:
    AppState &= ~APP_STATE_ADV;
    if ((AppState & BLE_STATE_MASK) == APP_STATE_IDLE)
    {
      speaker_audio_announce(H5APP_PLAY_STB);
    }
    speaker_update_IHM();
    break;

  case CONNECTED :
    AppState &= ~APP_STATE_ADV;
    if (!IS(AppState, APP_STATE_CON))
    {
      AppState |= APP_STATE_CON;
      speaker_audio_announce(H5APP_PLAY_CON);
      speaker_update_IHM();
    }
    break;

  case DISCONNECTED:
     AppState &= ~APP_STATE_CON;
     speaker_audio_announce(H5APP_PLAY_DISCO);
     speaker_update_IHM();

#if (EW25DEMO == 1)
     if (IS(AppState, APP_STATE_IDLE))
     {
       AppRequest |= APP_REQ_SCAN;
     }
#endif /* EW25DEMO == 1 */
     break;

  case START_SCAN:
     AppState |= APP_STATE_SCAN;
     speaker_update_IHM();
     break;

   case STOP_SCAN:
     AppState &= ~APP_STATE_SCAN;
     speaker_update_IHM();
     break;

   case BIG_SYNC:
     AppState |= APP_STATE_BIG_SYNC;
     break;

   case BIG_SYNC_LOST:
     AppState &= ~APP_STATE_BIG_SYNC;
     break;

  default:
     break;
  }

  /* handle the event */
  UTIL_SEQ_SetTask(1U<<CFG_TASK_BOARD_MNGR_ID, CFG_SEQ_PRIO_0);
}

void Sys_Notify_Evt(SysEvnt_t evnt)
{
#if (BAT_MNGMT == 1)
  if (evnt == START_CHRG)
  {
    Battery.is_charging = 1;
    AppRequest |= APP_REQ_RUN_MIXMOD;
  }
  else if (evnt == STOP_CHRG)
  {
    Battery.is_charging = 0;
    AppRequest &= ~APP_REQ_RUN_MIXMOD; /* remove the request if not handled yet */
    AppRequest |= APP_REQ_STOP_MIXMOD;
  }
  else if (evnt == FUEL_GAUGE_IT)
  {
  }
  /* handle the event */
  UTIL_SEQ_SetTask(1U<<CFG_TASK_BOARD_MNGR_ID, CFG_SEQ_PRIO_0);
#endif /* BAT_MNGMT == 1 */
}

static void button_timerCallback(void* arg)
{
  ActiveButton.longpress = 1;
  if (ActiveButton.id == BUTTON_1)
  {
    AppRequest |= APP_REQ_LONG_BP1;
  }
  else if (ActiveButton.id == BUTTON_2)
  {
    AppRequest |= APP_REQ_LONG_BP2;
  }
  else if (ActiveButton.id == BUTTON_3)
  {
    AppRequest |= APP_REQ_LONG_BP3;
  }
  else
  {
    AppRequest |= APP_REQ_LONG_BP_PWR;
  }

  ActiveButton.started = 0;

   /* handle the event */
  UTIL_SEQ_SetTask(1U<<CFG_TASK_BOARD_MNGR_ID, CFG_SEQ_PRIO_0);
}

void Button_rising_evnt(Button_t button)
{
  Counter_Activity = 0;
  /* we don't support simultaneous push */
  if (ActiveButton.started == 0)
  {
    ActiveButton.id = button;
    ActiveButton.started = 1;
    ActiveButton.longpress = 0;
    uint32_t timer_duration = BUTTON_LONG_PRESS_MS;
    if (ActiveButton.id == BUTTON_PWR)
    {
      timer_duration = BUTTON_PWR_LONG_PRESS_MS;
    }
    UTIL_TIMER_StartWithPeriod(&button_timer, timer_duration);
  }
}

void Button_falling_evnt(Button_t button)
{
  Counter_Activity = 0;
  if (ActiveButton.started == 1 && (button == ActiveButton.id))
  {
    if (ActiveButton.longpress == 0)
    {
      /* short press button */
      UTIL_TIMER_Stop(&button_timer);
      if (button == BUTTON_1)
      {
        AppRequest |= APP_REQ_BP1;
      }
      else if (button == BUTTON_2)
      {
        AppRequest |= APP_REQ_BP2;
      }
      else if (button == BUTTON_3)
      {
        AppRequest |= APP_REQ_BP3;
      }
      else
      {
        AppRequest |= APP_REQ_BP_PWR;
      }

      ActiveButton.started = 0;

      /* handle the event */
      UTIL_SEQ_SetTask(1U<<CFG_TASK_BOARD_MNGR_ID, CFG_SEQ_PRIO_0);
    }
    else
    {
      /* already notified the action */
    }
  }
}

static void speaker_update_IHM(void)
{
  if (IS(AppState, APP_STATE_CRITICAL_ERR))
  {
    speaker_set_led(LED_RED, 0, 0);
  }
  else if (IS(AppState, APP_STATE_VERY_LOW_BAT))
  {
    speaker_set_led(LED_RED, LED_ON_DEFAULT_MS,LED_OFF_DEFAULT_MS);
  }
  else if (IS(AppState, APP_STATE_LOW_BAT))
  {
    speaker_set_led(LED_ORANGE, LED_ON_DEFAULT_MS,LED_OFF_DEFAULT_MS);
  }
  else
  {
    if (IS(AppState, APP_STATE_STREAM_MED) || IS(AppState, APP_STATE_STREAM_TEL) || IS(AppState, APP_STATE_STREAM_SNK))
    {
      speaker_set_led(LED_PINK, LED_ON_DEFAULT_MS, 450);
    }
    else if (IS(AppState, APP_STATE_SCAN) && (IS(AppState, APP_STATE_CON)))
    {
      speaker_set_led(LED_CIAN, 400, 100);
    }
    else if (IS(AppState, APP_STATE_SCAN))
    {
      speaker_set_led(LED_BLUE, 400, 100);
    }
    else if (IS(AppState, APP_STATE_CON))
    {
      speaker_set_led(LED_CIAN, LED_ON_DEFAULT_MS,LED_OFF_DEFAULT_MS);
    }
    else if (IS(AppState, APP_STATE_ADV))
    {
      speaker_set_led(LED_BLUE, LED_ON_DEFAULT_MS, 1950);
    }
    else if (IS(AppState, APP_STATE_IDLE))
    {
      speaker_set_led(LED_OFF, 0, 0);
    }
    else
    {
      speaker_set_led(LED_OFF, 0, 0); //not reached
    }
  }
}

static void speaker_audio_announce(uint8_t announcement_req)
{
  uint8_t is_clock_en = __HAL_RCC_I2C3_IS_CLK_ENABLED();

  LOG_INFO_APP("SPEAKER : start announcement %d \n", announcement_req);
  __HAL_RCC_I2C3_CLK_ENABLE();

  if (IS(AppState, APP_STATE_STREAM_MED) || IS(AppState, APP_STATE_STREAM_TEL) || IS(AppState, APP_STATE_STREAM_SNK) || IS(AppState, APP_STATE_ANNOUNCE))
  {
    /* audio path is already done */
    LOG_INFO_APP("SPEAKER : audio chain was ready\n");
  }
  else
  {
    LOG_INFO_APP("SPEAKER : configure audio chain\n");
    /* wakeup H5  */
    H5_wakeup();

    H5_send_cmd(H5APP_BOOT_AURACAST); // default mode

    HAL_Delay(200); /* H5 boot second part : Os Ready */

    uint8_t status = H5_send_cmd(H5APP_START_LOCAL_CLK);

    if (status != HAL_OK)
    {
      /* issue with com */
      board_error(COM_H5_ERR);
      if (is_clock_en == 0)
        __HAL_RCC_I2C3_CLK_DISABLE();
      return;
    }

    /* switch gas gauge to mix mode */
    AppRequest |= APP_REQ_RUN_MIXMOD;

    /* enable codec power supply */
    ENABLE_1V8();
    HAL_Delay(500);

    /* setup codec */
    if (MAX9867_StartMedia() != 0)
    {
      board_error(COM_MAX9867_ERR);
    }

    Component_Mute(1);

#if (EW25DEMO == 0)
    if (LTC3556_set_5V_on() != 0)
    {
      board_error(COM_LTC_ERR);
    }
#endif /* EW25DEMO == 0 */

    Component_SetVolume(Local_volume);

    HAL_Delay(100); /* wait the audio chain to be ready */
  }

  AppState |= APP_STATE_ANNOUNCE;
  Counter_Announcement = 0;
  Announcement_is_timeout = 0;

  H5_send_cmd(announcement_req);

  if (is_clock_en == 0)
    __HAL_RCC_I2C3_CLK_DISABLE();
}

static void speaker_audio_end_announce(void)
{
  LOG_INFO_APP("SPEAKER : end of announcement detected\n");

  if (IS(AppState, APP_STATE_ANNOUNCE))
  {
    AppState &= ~APP_STATE_ANNOUNCE;

    if (IS(AppState, APP_STATE_STREAM_MED) || IS(AppState, APP_STATE_STREAM_TEL) || IS(AppState, APP_STATE_STREAM_SNK))
    {
      /* nothing to do */
      return;
    }

    LOG_INFO_APP("SPEAKER : release audio chain\n");

    Component_Mute(1);

#if (EW25DEMO == 0) /* avoid pop for the demo */
    if (LTC3556_set_5V_off() != 0)
    {
      board_error(COM_LTC_ERR);
    }
#endif /* EW25DEMO == 0 */

    if (MAX9867_Shutdown() != 0)
    {
      board_error(COM_MAX9867_ERR);
    }

    HAL_Delay(50);

    H5_send_cmd(H5APP_STOP_LOCAL_CLK);
    H5_send_cmd(H5APP_SHUTDOWN);

    DISABLE_1V8();

    /* switch gas gauge to low power mode */
    AppRequest |= APP_REQ_STOP_MIXMOD;
  }
}

void Speaker_audio_start(SpeakerAudioConfig_t mode, uint32_t frequency, uint8_t vol)
{
  LOG_INFO_APP("SPEAKER : configure audio chain for mode %d and frequency %d\n", mode, frequency);

  if (mode == CONFIG_SIMPLEX)
  {
    if (IS(AppState, APP_STATE_BIG_SYNC))
    {
      AppState |= APP_STATE_STREAM_SNK;
    }
    else
    {
      AppState |= APP_STATE_STREAM_MED;
    }
  }
  else /* (mode == CONFIG_DUPLEX) */
  {
    AppState |= APP_STATE_STREAM_TEL;
  }

  /* switch gas gauge to mix mode */
  AppRequest |= APP_REQ_RUN_MIXMOD;

  /* wakeup H5 and ensure local clock is off */
  uint8_t H5status;

  H5_wakeup();

  if ((mode == CONFIG_SIMPLEX) && (frequency == 48000))
  {
    H5_send_cmd(H5APP_BOOT_MEDIA);
  }
  else if ((mode == CONFIG_SIMPLEX) && (frequency == 24000))
  {
    H5_send_cmd(H5APP_BOOT_AURACAST);
  }
  else if (mode == CONFIG_DUPLEX)
  {
    H5_send_cmd(H5APP_BOOT_TELEPHONY);
  }
  else
  {
    board_error(APP_ERR);
  }

  HAL_Delay(200); /* H5 boot second part : Os Ready */

  uint8_t retry = 5;
  H5_read_status(&H5status);
  while ((H5status & H5APP_STATE_CLK_ON) == 1 && (retry != 0))
  {
    /* H5 is already running local clock, maybe due to an announcement */
    H5_send_cmd(H5APP_STOP_LOCAL_CLK);
    HAL_Delay(50);
    retry--;
    LOG_INFO_APP("SPEAKER Warning, H5 has clock on, trying to stop it...\n");
  }

  if (retry == 0)
  {
    board_error(APP_ERR);
  }

  /* enable codec power supply */
  ENABLE_1V8();

  /* generate audio clock */
  //activation of the PWM timer which is intended to be the MCLK : 12.288 MHz
  TIM1->ARR=7;
  TIM1->CCR3=3;
  HAL_TIM_MspPostInit(&htim1);
  if (HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3) != HAL_OK)
  {
    board_error(APP_ERR);
  }

  HAL_Delay(50);

  /* Set H5 on comon audio clock */
  H5_send_cmd(H5APP_SET_I2S_AUDIOCLK);

  /* setup codec */
  if(MAX9867_StartMedia() != 0)
  {
    board_error(COM_MAX9867_ERR);
  }

  Component_Mute(1); /* mute for avoiding pops */

#if (EW25DEMO == 0) /* avoid pop for the demo */
  /* Enable power output */
  HAL_Delay(10);
  if (LTC3556_set_5V_on() != 0)
  {
    board_error(COM_LTC_ERR);
  }
  HAL_Delay(10);
#endif /* EW25DEMO == 0 */

  Component_SetVolume(vol);
  speaker_update_IHM();
}

void Speaker_audio_stop(void)
{
  if( IS(AppState, APP_STATE_STREAM_TEL))
  {
    AppState &= ~APP_STATE_STREAM_TEL;
  }

  if( IS(AppState, APP_STATE_STREAM_MED))
  {
    AppState &= ~APP_STATE_STREAM_MED;
  }

  if( IS(AppState, APP_STATE_STREAM_SNK))
  {
    AppState &= ~APP_STATE_STREAM_SNK;
  }

  LOG_INFO_APP("SPEAKER : release audio chain\n");

  /* switch volume to zero for avoiding pops*/
  Component_Mute(1);

  /* prepare H5 to switch to its own clock*/
  H5_send_cmd(H5APP_SET_I2S_PLLQ);

  HAL_Delay(10);

#if (EW25DEMO == 0) /* avoid pop for the demo */
  if (LTC3556_set_5V_off() != 0)
  {
    board_error(COM_LTC_ERR);
  }
#endif /* EW25DEMO == 0 */

  if (MAX9867_Shutdown() != 0)
  {
    board_error(COM_MAX9867_ERR);
  }

  /* remove codec power supply */
  DISABLE_1V8();

  /* remove clock */
  HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_3);
  HAL_TIM_MspPostDeinit(&htim1);

  speaker_update_IHM();

  /* switch gas gauge to low power mode */
  AppRequest |= APP_REQ_STOP_MIXMOD;
}


static void board_error(Board_Error_t error_code)
{
  switch (error_code)
  {
    case APP_ERR:
      LOG_INFO_APP("SPEAKER ERR: critical appli error - reset... \n");
      AppState |= APP_STATE_CRITICAL_ERR;
      speaker_set_led(LED_RED, 0, 0);
      HAL_Delay(10000);
      HAL_NVIC_SystemReset();
    break;

    default:
      /* could be COM error due to missing battery, H5 not flashed, etc .. */
      LOG_INFO_APP("SPEAKER ERR: com error with code %d \n", error_code);
    break;
  }
}

void Component_SetVolume(uint8_t vol)
{
  Local_volume = vol;
  MAX9867_SetVolume(vol);
}

void Component_Mute(uint8_t en)
{
  if (en)
    MAX9867_Mute(1);
  else
    MAX9867_Mute(0);
}

#if (BAT_MNGMT == 1)
int32_t I2C_Write(int32_t byte_nb, int32_t addr, uint8_t *tx_buffer)
{
  return HAL_I2C_Mem_Write(&hi2c3, STC3115_SLAVE_ADDRESS, addr, 1, tx_buffer, byte_nb, STC_I2C_TIMEOUT);
}

int32_t I2C_Read(int32_t byte_nb, int32_t addr, uint8_t *rx_buffer)
{
  return HAL_I2C_Mem_Read(&hi2c3, STC3115_SLAVE_ADDRESS, addr, 1, rx_buffer, byte_nb, STC_I2C_TIMEOUT);
}
#endif /* (BAT_MNGMT == 1) */
