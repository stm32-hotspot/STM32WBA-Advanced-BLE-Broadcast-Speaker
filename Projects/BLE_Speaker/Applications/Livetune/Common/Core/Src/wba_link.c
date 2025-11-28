/**
******************************************************************************
* @file    wba_link.c
* @author  MCD Application Team
* @brief
******************************************************************************
* @attention
*
* Copyright (c) 2018(-2022) STMicroelectronics.
* All rights reserved.
*
* This software is licensed under terms that can be found in the LICENSE file
* in the root directory of this software component.
* If no LICENSE file comes with this software, it is provided AS-IS.
*
******************************************************************************
*/
#include "main.h"
#include "wba_link.h"
#include "BoardSetup.h"
#include "BLE_Speaker_H5_bsp_audio.h"
#include "BLE_Speaker_H5_bsp_com.h"
#include "st_os.h"
#include "voice_announce.h"

/* for algo tuning */
#include "acSdk.h"
#include "audio_chain_instance.h"

/* Private defines -----------------------------------------------------------*/

#define CMD_PLAY_MASK             0x10

/************* COMMANDS *************/
#define H5APP_SHUTDOWN            0x01
#define H5APP_RUN_AUDIKIT         0x02
#define H5APP_RESET               0x03
#define H5APP_START_LOCAL_CLK     0x04
#define H5APP_STOP_LOCAL_CLK      0x05
#define H5APP_SET_I2S_AUDIOCLK    0x06
#define H5APP_SET_I2S_PLLQ        0x07

#define H5APP_PLAY_ADV            0x10
#define H5APP_PLAY_CON            0x11
#define H5APP_PLAY_STB            0x12
#define H5APP_PLAY_LOWBAT         0x13
#define H5APP_PLAY_DISCO          0x14
#define H5APP_PLAY_SYNCH          0x15
#define H5APP_PLAY_SCAN           0x16
#define H5APP_PLAY_EXE            0x17
#define H5APP_PLAY_BIP            0x18

#define H5APP_STOP_PLAY           0x20

#define H5APP_BOOT_MEDIA          0x30
#define H5APP_BOOT_AURACAST       0x31
#define H5APP_BOOT_TELEPHONY      0x32

/*************** STATES ***************/
#define H5APP_STATE_IDLE             0x00
#define H5APP_STATE_CLK_ON           0x01
#define H5APP_STATE_LOCAL_PLAY       0x02
#define H5APP_STATE_WBA_PLAY         0x04
#define H5APP_STATE_BUSY             0x08 /* command pending to be executed */

#define H5APP_STATE_RUN_AUDIOCLK     0x10
#define H5APP_STATE_GRAPH_MEDIA_48k  0x20
#define H5APP_STATE_GRAPH_MEDIA_24k  0x40
#define H5APP_STATE_GRAPH_TELELPHONY 0x80


#define BOARD_SET_STATE(state) ({BoardState |= state; \
                                aI2CTxBuffer[0] = BoardState;})

#define BOARD_RMV_STATE(state) ({BoardState &= ~state; \
                                aI2CTxBuffer[0] = BoardState;})

#define TICKS_BEFORE_STBY       5000

#define SAMPLE_PER_FRAME_16k (AC_SYSIN_WAVFILE_FS * AC_N_MS_PER_RUN)/1000 /* Freq (/ms) * period (ms) for annoucement */
#define ANNOUCEMENT_BUF_SIZE (SAMPLE_PER_FRAME_16k * 1 * 1)               /* SAMPLE_PER_FRAME * stereo(I2S) * double buff */

/* Function ------------------------------------------------------------------*/
void APP_AUDIO_OUT_Tranfert_Callback(uint8_t halfbuff);

extern HAL_StatusTypeDef MX_I2S1_ClockConfig(uint32_t SampleRate, uint32_t mode);
extern uint8_t *UTIL_AUDIO_WAVFILE_getSampleAddr(void);

/* Private Function ----------------------------------------------------------*/
static void Enter_Standby_Mode(void);
static void Local_audio_play(uint8_t fileid);
static void Local_audio_stop(void);
static void Execute_cmd(uint8_t cmd);
static void app_task(const void *pCookie);

static void ConfigureMix1Gain(int8_t input, int8_t gain1, int8_t gain2);

/* Global variables ----------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;
TIM_HandleTypeDef htim3;

uint8_t aI2CTxBuffer[I2C_TX_LEN] = {0};
uint8_t aI2CRxBuffer[I2C_RX_LEN] = {0};

extern audio_chain_t AudioChainInstance; /* algo tuning */

/* Private variables ---------------------------------------------------------*/
static volatile uint32_t  BoardState = H5APP_STATE_IDLE;
static uint32_t I2CActivityTimer = 0;
static st_task  hAppliTask;
static st_queue hAppliCmdQueue;

static int16_t aAudiobuff[ANNOUCEMENT_BUF_SIZE];
static int16_t *pFile;
static int32_t Filelen;

static volatile audio_mode_t Audioconf = 0xff;

static void app_task(const void *pCookie)
{
  uint8_t msg;
  while(1)
  {
    if (st_os_queue_get(&hAppliCmdQueue, &msg, 10) == osOK)
    {
      Execute_cmd(msg);
    }
    else
    {
      /* no comamnd pending, so allow to go in idle */
      BOARD_RMV_STATE(H5APP_STATE_BUSY);

      st_os_task_delay(10);
    }

    if (((HAL_GetTick() - I2CActivityTimer) > TICKS_BEFORE_STBY) &&
        ((BoardState & H5APP_STATE_LOCAL_PLAY) == 0) &&             /* currently no annoucement */
        ((BoardState & H5APP_STATE_RUN_AUDIOCLK) == 0))             /* not runinng audio from wba */
    {
      I2CActivityTimer = HAL_GetTick(); /* reset timer (when low power is not enabled) */

      /* no activity on I2C or no audio stream, could enter in sleep mode */
      Execute_cmd(H5APP_SHUTDOWN);
    }

    if (((HAL_GetTick() - I2CActivityTimer) > 500) &&
        ((hi2c1.State == HAL_I2C_STATE_BUSY_TX) || (hi2c1.State == HAL_I2C_STATE_BUSY_RX)))
    {
      __HAL_I2C_GENERATE_NACK(&hi2c1);
    }
  }
}

audio_mode_t WBA_link_init(void)
{
  /* state initialization */
  BoardState = H5APP_STATE_IDLE;
  aI2CTxBuffer[0] = BoardState;

  I2CActivityTimer = HAL_GetTick();

  /* PC13 wakeup Pin & Reset */
  __HAL_RCC_GPIOC_CLK_ENABLE();

  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  HAL_NVIC_SetPriority(EXTI13_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI13_IRQn);

#if 0
  /* debug */
  GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
#endif

  /* I2C COM */
  MX_I2C1_Init();

  if (HAL_I2C_EnableListen_IT(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /* Task */
  st_os_task_create(&hAppliTask,"appli_task()",&app_task,NULL,1000,ST_Priority_Above_Normal);
  st_os_queue_create_named(&hAppliCmdQueue, 10, 1, NULL);

  // wait a boot command to be received
  uint32_t tickstart = HAL_GetTick();
  uint8_t timeout = 0;
  while ((timeout == 0) && (Audioconf == 0xff)){
    if(HAL_GetTick() - tickstart > 1000)
    {
      timeout = 1;
    }
  }

  if (Audioconf == MODE_MEDIA_48k)
  {
    BOARD_SET_STATE(H5APP_STATE_GRAPH_MEDIA_48k);
    Execute_cmd(H5APP_SET_I2S_AUDIOCLK);
  }
  else if (Audioconf == MODE_MEDIA_24k)
  {
    BOARD_SET_STATE(H5APP_STATE_GRAPH_MEDIA_24k);
    Execute_cmd(H5APP_SET_I2S_AUDIOCLK);
  }
  else if (Audioconf == MODE_TELEPHONY)
  {
    BOARD_SET_STATE(H5APP_STATE_GRAPH_TELELPHONY);
    Execute_cmd(H5APP_SET_I2S_AUDIOCLK);
  }
  else
  {
    Audioconf = MODE_MEDIA_24k; /* default mode since not command received */
    BOARD_SET_STATE(H5APP_STATE_GRAPH_MEDIA_24k);
    Execute_cmd(H5APP_SET_I2S_PLLQ);
  }
  return Audioconf;
}


static void ConfigureMix1Gain(int8_t input, int8_t gain1, int8_t gain2)
{
  char s_val1[3] = "";
  char s_val2[3] = "";

  sprintf(s_val1, "%d", gain1);
  sprintf(s_val2, "%d", gain2);

  acAlgo hAlgo = acAlgoGetInstance(&AudioChainInstance, "mix-3");

  if (hAlgo != NULL)
  {
    acAlgoSetConfig(hAlgo, "gain0", s_val1);
    acAlgoSetConfig(hAlgo, "gain1", s_val2);
    acAlgoRequestUpdate(hAlgo);
  }
}

static void Execute_cmd(uint8_t cmd)
{
  if (cmd & CMD_PLAY_MASK)
  {
    Local_audio_play(cmd);
  }
  else
  {
    switch(cmd)
    {
      case H5APP_SHUTDOWN:
        Enter_Standby_Mode();
        break;

      case H5APP_RESET:
        BoardState = H5APP_STATE_IDLE;
        BOARD_SET_STATE(H5APP_STATE_IDLE);
        HAL_NVIC_SystemReset();
        break;

      case H5APP_STOP_PLAY:
        Local_audio_stop();
        break;

      case H5APP_START_LOCAL_CLK:
        BOARD_SET_STATE(H5APP_STATE_CLK_ON);
        MX_TIM3_Init();
        HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);
        break;

      case H5APP_STOP_LOCAL_CLK:
        if (htim3.Instance != 0)
        {
          HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_4);
          MX_TIM3_Deinit();
        }

        BOARD_RMV_STATE(H5APP_STATE_CLK_ON);
        break;

      case H5APP_SET_I2S_AUDIOCLK:
        MX_I2S1_ClockConfig(0,1);
        BOARD_SET_STATE(H5APP_STATE_RUN_AUDIOCLK);
        break;

      case H5APP_SET_I2S_PLLQ:
        MX_I2S1_ClockConfig(0,0);
        BOARD_RMV_STATE(H5APP_STATE_RUN_AUDIOCLK);
        break;

      default:
        break;
    }
  }
}

void Enter_Standby_Mode(void)
{
#ifdef LOWPWR
  HAL_GPIO_DeInit(GPIOA, GPIO_PIN_ALL);
  HAL_GPIO_DeInit(GPIOB, GPIO_PIN_ALL);
  HAL_GPIO_DeInit(GPIOC, GPIO_PIN_ALL);
  HAL_GPIO_DeInit(GPIOE, GPIO_PIN_ALL);

  __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SBF);
  __HAL_PWR_CLEAR_FLAG(PWR_WAKEUP_FLAG4);
  HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN4_HIGH);// | (0x03 << 22)); /* PC13 + pull down*/
  HAL_PWR_EnterSTANDBYMode();
#endif
}


static void Local_audio_play(uint8_t fileid)
{
  memset((void*)aAudiobuff, 0, sizeof(aAudiobuff));

  switch (fileid)
  {
    case H5APP_PLAY_ADV:
      pFile = (int16_t*)Advertising_16kHz;
      Filelen = ADVERTISING_LEN;
    break;

    case H5APP_PLAY_CON:
      pFile = (int16_t*)Connected_16kHz;
      Filelen = CONNECTED_LEN;
    break;

    case H5APP_PLAY_STB:
      pFile = (int16_t*)Standby_16kHz;
      Filelen = STANDBY_LEN;
    break;

    case H5APP_PLAY_LOWBAT:
      pFile = (int16_t*)LowBattery_16kHz;
      Filelen = LOW_BATTERY_LEN;
    break;

    case H5APP_PLAY_DISCO:
      pFile = (int16_t*)Disconnected_16kHz;
      Filelen = DISCONNECTED_LEN;
    break;

    case H5APP_PLAY_SYNCH:
      pFile = (int16_t*)Synchronized_16kHz;
      Filelen = SYNCHRONIZED_LEN;
    break;

    case H5APP_PLAY_SCAN:
      pFile = (int16_t*)Scanning_16kHz;
      Filelen = SCANNING_LEN;
    break;

    case H5APP_PLAY_EXE:
      pFile = (int16_t*)Example_16kHz;
      Filelen = EXAMPLE_LEN;
    break;

    case H5APP_PLAY_BIP:
      pFile = (int16_t*)Bip_16kHz;
      Filelen = BIP_LEN;
    break;

    default:
      Error_Handler();
    break;
  }

  /* reduce gain on main branch */
  ConfigureMix1Gain(0, -16, -4);
  osDelay(200);

  BOARD_SET_STATE(H5APP_STATE_LOCAL_PLAY);
}


static void Local_audio_stop(void)
{
  osDelay(200);
  /* restore gain */
  ConfigureMix1Gain(0, 0, -4);
}


void APP_AUDIO_OUT_Tranfert_Callback(uint8_t halfbuff)
{

  if ((BoardState & H5APP_STATE_LOCAL_PLAY) == 0)
  {
    return;
  }

  uint32_t i;
  int16_t *paudiobuff = aAudiobuff;

  if (Filelen - (int32_t)SAMPLE_PER_FRAME_16k >= 0)
  {
    Filelen -= SAMPLE_PER_FRAME_16k;
    for (i=0 ; i<SAMPLE_PER_FRAME_16k ; i++)
    {
      *paudiobuff++ = (*pFile++); /* copy to RAM */
    }
  }
  else
  {
    /* file ended */
    memset((void*)paudiobuff, 0, sizeof(aAudiobuff));

    if ((BoardState & H5APP_STATE_LOCAL_PLAY) != 0)
    {
      BOARD_RMV_STATE(H5APP_STATE_LOCAL_PLAY);

      uint8_t cmd = H5APP_STOP_PLAY;
      if (st_os_queue_put(&hAppliCmdQueue, (void *)&cmd, 0) != osOK)
      {
        Error_Handler();
      }
    }
  }

  /* inject samples inside livetune */
  uint8_t *pOutSample_u8 = UTIL_AUDIO_WAVFILE_getSampleAddr();
  memcpy(pOutSample_u8, aAudiobuff, sizeof(aAudiobuff));
}

void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *I2cHandle)
{
  BOARD_SET_STATE(H5APP_STATE_BUSY);

  uint8_t msg = aI2CRxBuffer[0];
  if (msg == H5APP_BOOT_MEDIA)
  {
    Audioconf = MODE_MEDIA_48k;
  }
  else if (msg == H5APP_BOOT_AURACAST)
  {
    Audioconf = MODE_MEDIA_24k;
  }
  else if (msg == H5APP_BOOT_TELEPHONY)
  {
    Audioconf = MODE_TELEPHONY;
  }
  else
  {
    if (osKernelGetState() != osKernelInactive)
    {
      HAL_NVIC_SetPriority(I2C1_EV_IRQn, I2C_IT_PRIO_STD, 0);
      if (st_os_queue_put(&hAppliCmdQueue, (void *)&msg, 0) != osOK)
      {
        Error_Handler();
      }
      HAL_NVIC_SetPriority(I2C1_EV_IRQn, I2C_IT_PRIO_LISTEN, 0);
    }
  }
  I2CActivityTimer = HAL_GetTick();
}

void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *I2cHandle)
{

}

void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *I2cHandle)
{
  if (HAL_I2C_EnableListen_IT(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  I2CActivityTimer = HAL_GetTick();
}

void EXTI13_IRQHandler(void)
{
  /* PC13 is used as wakeup pin, if a interrupt is detected (awake), we generate a reset */
  __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_13);

  Execute_cmd(H5APP_RESET);
}

void app_sleep(void)
{
#ifdef LOWPWR
  //__HAL_RCC_CRC_CLK_DISABLE(); used by PDM converter

  __disable_irq();
  if ((BoardState & H5APP_STATE_RUN_AUDIOCLK) != 0)
  {
    MODIFY_REG(RCC->CFGR2, RCC_CFGR2_HPRE, RCC_SYSCLK_DIV4);
    MODIFY_REG(RCC->CFGR1, RCC_CFGR1_SW, RCC_SYSCLKSOURCE_HSI);
  }
  /* sleep */
  HAL_PWR_EnterSLEEPMode(0, PWR_SLEEPENTRY_WFI);

  if ((BoardState & H5APP_STATE_RUN_AUDIOCLK) != 0)
  {
    MODIFY_REG(RCC->CFGR1, RCC_CFGR1_SW, RCC_SYSCLKSOURCE_PLLCLK);
    MODIFY_REG(RCC->CFGR2, RCC_CFGR2_HPRE, RCC_SYSCLK_DIV1);
  }

  __enable_irq();
#endif
}

void Error_Handler(void)
{
  while(1);
}
