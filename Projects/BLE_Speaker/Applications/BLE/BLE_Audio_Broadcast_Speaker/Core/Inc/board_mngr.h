/**
  ******************************************************************************
  * @file    board_mngr.h
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

#ifndef __BOARD_MNGR_H_
#define __BOARD_MNGR_H_

#include <stdint.h>
#include "main.h"
#include "stm32wbaxx_hal.h"
#include "app_conf.h"
#include "stm32_seq.h"
#include "stm32_timer.h"

#define BAT_LVL_CRITICAL          0    /* minimum battery level before stopping BLE activity */
#define BAT_LVL_VERY_LOW          5
#define BAT_LVL_LOW               10
#define BAT_MAX_TEMPERATURE       35
#define BAT_CURRENT_THRHOLD       (+5)

#define CHECKING_PERIOD_MS        500

#define LED_ON_DEFAULT_MS         15
#define LED_OFF_DEFAULT_MS        950
#define BUTTON_LONG_PRESS_MS      500
#define BUTTON_PWR_LONG_PRESS_MS  3000

#define BAT_READING_PERIOD_SEC    30
#define ADV_TIMEOUT_SEC           60
#define ANNOUNCEMENT_TIMEOUT_SEC  20
#define NO_ACTIVITY_SHTDWN_SEC    120


#define CSIP_CONF_ID              0

#define VOLUME_INIT               80

#define HIGH_CURRENT_MODE         (-50)    /* current value before switching DC/DC mode to high current mode */

/* WKUP COMPONENT */
#define SPEAKER_COMP_WKUP_PIN   GPIO_PIN_7
#define SPEAKER_COMP_WPUP_PORT  GPIOA

#define SPEAKER_COMP_COM_PIN_TX GPIO_PIN_12
#define SPEAKER_COMP_COM_PIN_RX GPIO_PIN_11
#define SPEAKER_COMP_COM_PORT   GPIOA

/* I2C */
#define SPEAKER_I2C_SDA_PIN     GPIO_PIN_1
#define SPEAKER_I2C_SCL_PIN     GPIO_PIN_2
#define SPEAKER_I2C_PORT        GPIOB

/* PUSH BUTTONS */
#define SPEAKER_BP_POWER_PIN    GPIO_PIN_3
#define SPEAKER_BP_POWER_PORT   GPIOB
#define SPEAKER_BP_POWER_IT     EXTI3_IRQn

#define SPEAKER_BP1_PIN         GPIO_PIN_8
#define SPEAKER_BP1_PORT        GPIOB
#define SPEAKER_BP1_IT          EXTI8_IRQn

#define SPEAKER_BP2_PIN         GPIO_PIN_9
#define SPEAKER_BP2_PORT        GPIOB
#define SPEAKER_BP2_IT          EXTI9_IRQn

#define SPEAKER_BP3_PIN         GPIO_PIN_0
#define SPEAKER_BP3_PORT        GPIOA
#define SPEAKER_BP3_IT          EXTI0_IRQn

#define BP_INTERRUPT_PRIO       15

/* INTERRUPT */
#define SPEAKER_CHG_IT_PIN      GPIO_PIN_10
#define SPEAKER_CHG_IT_PORT     GPIOA

#define SPEAKER_FUELG_IT_PIN    GPIO_PIN_13
#define SPEAKER_FUELG_IT_PORT   GPIOC

#define COMP_INTERRUPT_PRIO     15

/* CONFIG */
#define SPEAKER_1V8_EN_PIN      GPIO_PIN_0
#define SPEAKER_1V8_EN_PORT     GPIOB

/* RGB LED */
#define SPEAKER_LED_RED_PIN     GPIO_PIN_2
#define SPEAKER_LED_GREEN_PIN   GPIO_PIN_6
#define SPEAKER_LED_BLUE_PIN    GPIO_PIN_5
#define SPEAKER_LED_PORT        GPIOA

typedef enum
{
  BUTTON_1,
  BUTTON_2,
  BUTTON_3,
  BUTTON_PWR,
} Button_t;

typedef enum
{
  START_CHRG,
  STOP_CHRG,
  FUEL_GAUGE_IT,
} SysEvnt_t;

typedef enum
{
  START_ADVERTISING,
  STOP_ADVERTISING,
  CONNECTED,
  DISCONNECTED,
  START_SCAN,
  STOP_SCAN,
  BIG_SYNC,
  BIG_SYNC_LOST,
} Tmapapp_Event_t;

typedef enum
{
  CONFIG_SIMPLEX,
  CONFIG_DUPLEX,
} SpeakerAudioConfig_t;


/*Initializes the components by I2C command and the clock which will call the system event processing function.*/
uint8_t BoardInit (void);

void Speaker_audio_start(SpeakerAudioConfig_t mode, uint32_t frequency, uint8_t vol);
void Speaker_audio_stop(void);

void App_Notify_Evt(Tmapapp_Event_t event);
void Sys_Notify_Evt(SysEvnt_t event);

void Component_SetVolume(uint8_t Volume);
void Component_Mute(uint8_t en);

void Button_rising_evnt(Button_t button);
void Button_falling_evnt(Button_t button);

#endif /* __BOARD_MNGR_H_ */
