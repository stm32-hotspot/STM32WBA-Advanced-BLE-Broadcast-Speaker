/**
  ******************************************************************************
  * @file    stm32_audio.h
  * @author  MCD Application Team
  * @brief   Header for stm32_audio.c module
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32_AUDIO_H
#define __STM32_AUDIO_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include "stm32_audio_conf.h"
#include "audio_buffer.h"


/** @addtogroup MIDDLEWARES
* @{
*/

/** @defgroup STM32_AUDIO STM32_AUDIO
* @{
*/
/* Exported types ------------------------------------------------------------*/

/** @defgroup STM32_AUDIO_Exported_Types STM32_AUDIO Exported Types
* @{
*/

/**
 * @brief  audio hw config.
 */
typedef struct
{
  uint32_t Device;        /*!< identifier of the device IN or Out, DFSDM vs MDF or SAI etc..*/
  uint32_t SampleRate;    /*!< 16kHz or 48kHz for now */
  uint32_t BitsPerSample; /*!< 16-bit used for now */
  uint32_t ChannelsNbr;   /*!< 1 for mono and 2 for stereo */
  uint32_t Volume;        /*!< In percentage from 0 to 100 */
} UTIL_AUDIO_params_t;


/**
 * @brief  audio hw callback registration (allows to manage audio hw from BSP or CubeMX or any other API).
 */
typedef struct
{
  void (* error)(void);                                                                    /*!< callback for error */
  int32_t (* initIn)(uint32_t Instance, UTIL_AUDIO_params_t *AudioInit);                   /*!< callback for registration of capture init */
  int32_t (* preprocIn)(uint32_t Instance, uint8_t *dataIn, uint16_t *dataOut);            /*!< callback for registration of signal conditionning if any (pdm2pcm or dcrem etc...) */
  int32_t (* recordIn)(uint32_t Instance, uint8_t *pBuf, uint32_t NbrOfBytes);             /*!< callback for registration of start capturing all microphones */
  int32_t (* recordInChannels)(uint32_t Instance, uint8_t **pBuf, uint32_t NbrOfBytes);    /*!< callback for registration of start capturing microphones specified (legacy API from BSP) */
  int32_t (* stopIn)(uint32_t Instance);                                                   /*!< callback for registration of stop microphones capture */
  int32_t (* getStateIn)(uint32_t Instance, uint32_t *State);                              /*!< callback for registration of state getter for capture object*/
  int32_t (* initOut)(uint32_t Instance, UTIL_AUDIO_params_t *AudioInit);                  /*!< callback for registration of rendering init */
  int32_t (* playOut)(uint32_t Instance, uint8_t *pData, uint32_t NbrOfBytes);             /*!< callback for registration of rendering start */
  int32_t (* stopOut)(uint32_t Instance);                                                  /*!< callback for registration of rendering stop  */
  int32_t (* setVolumeOut)(uint32_t Instance, uint32_t Volume);                            /*!< callback for registration of volume setup */
  uint8_t (* allocScratchOut)(uint32_t *pScratch, uint32_t sizeInByte);                    /*!< callback for registration of scratch buffer alloc (legacy BSP API) */

} UTIL_AUDIO_cbs_t;


typedef uint32_t Path_id_t;

#define MAIN_PATH       0
#define ALTERNATE_PATH  1

/**
  * @}
  */

/* Exported constants --------------------------------------------------------*/


/** @defgroup STM32_AUDIO_Exported_Constants STM32_AUDIO Exported Constants
* @{
*/

/** @defgroup STM32_AUDIO_default_conf
* @brief    Default configuration , can be overwritten inside stm32_audio_conf.h
* @{
*/
#define UTIL_AUDIO_ERROR_NONE                  0
#define UTIL_AUDIO_ERROR                       1

#define UTIL_AUDIO_RESOLUTION_8B               8U
#define UTIL_AUDIO_RESOLUTION_16B              16U
#define UTIL_AUDIO_RESOLUTION_24B              24U
#define UTIL_AUDIO_RESOLUTION_32B              32U


#ifndef UTIL_AUDIO_USB_BIT_RESOLUTION
#define UTIL_AUDIO_USB_BIT_RESOLUTION          UTIL_AUDIO_RESOLUTION_16B
#endif


#ifndef UTIL_AUDIO_LOG_TASK_QUEUE_LEVELS
#define UTIL_AUDIO_LOG_TASK_QUEUE_LEVELS       (false)
#endif

#ifndef UTIL_AUDIO_IN_INSTANCE
#define UTIL_AUDIO_IN_INSTANCE                 2U /* DFSDM */
#endif

#ifndef UTIL_AUDIO_USE_MIC_PDM
#define UTIL_AUDIO_USE_MIC_PDM                 0U
#endif


#ifndef UTIL_AUDIO_IN_HW_CH_NB
#define UTIL_AUDIO_IN_HW_CH_NB                 1U
#endif

#ifndef UTIL_AUDIO_IN_CH_NB
#define UTIL_AUDIO_IN_CH_NB                    2U
#endif

#ifndef UTIL_AUDIO_OUT_CH_NB
#define UTIL_AUDIO_OUT_CH_NB                   2U
#endif

#ifndef UTIL_AUDIO_MAX_IN_FREQUENCY
#define UTIL_AUDIO_MAX_IN_FREQUENCY            UTIL_AUDIO_IN_FREQUENCY
#endif

#ifndef UTIL_AUDIO_MAX_OUT_FREQUENCY
#define UTIL_AUDIO_MAX_OUT_FREQUENCY           UTIL_AUDIO_OUT_FREQUENCY
#endif

#ifndef UTIL_AUDIO_MAX_IN_CH_NB
#define UTIL_AUDIO_MAX_IN_CH_NB                UTIL_AUDIO_IN_CH_NB
#endif

#ifndef UTIL_AUDIO_MAX_OUT_CH_NB
#define UTIL_AUDIO_MAX_OUT_CH_NB               UTIL_AUDIO_OUT_CH_NB
#endif

#ifndef UTIL_AUDIO_IN_FREQUENCY
#define UTIL_AUDIO_IN_FREQUENCY                16000UL
#endif

#ifndef UTIL_AUDIO_CPU_CACHE_MAINTENANCE
#define UTIL_AUDIO_CPU_CACHE_MAINTENANCE       0
#endif

#ifndef UTIL_AUDIO_OUT_FREQUENCY
#define UTIL_AUDIO_OUT_FREQUENCY               UTIL_AUDIO_IN_FREQUENCY /*!< The UTIL_AUDIO_OUT_FREQUENCY default value is equal to capture frequency*/
#endif

#ifndef UTIL_AUDIO_MEMORY_SECTION_HW_BUFFERS
#define UTIL_AUDIO_MEMORY_SECTION_HW_BUFFERS   DEFAULT_SECTION             /*!< By default there is no specific section */
#endif

#ifndef UTIL_AUDIO_N_MS_PER_INTERRUPT
#define UTIL_AUDIO_N_MS_PER_INTERRUPT          1UL  /*!< The N_MS_PER_INTERRUPT value defines the number of millisecond to be processed at each AudioProcess call. The default value of the N_MS_PER_INTERRUPT directive in the driver is set to 1 */
#endif

#ifndef UTIL_AUDIO_N_MS_DIV
#define UTIL_AUDIO_N_MS_DIV                    1UL  /*!< The N_MS_DIV allows to set interruption length < 1ms*/
#endif


#ifndef UTIL_AUDIO_MEMPOOL
#define UTIL_AUDIO_MEMPOOL                     AUDIO_MEM_RAMINT
#endif
/**
* @}
*/


/** @defgroup STM32_AUDIO_framesize
* @brief    Processed frame size for in & out
* @{
*/
/* Following Macros are used to calculate audio buffers sizes*/
#define UTIL_AUDIO_IN_SAMPLES_NB               (UTIL_AUDIO_N_MS_PER_INTERRUPT * (UTIL_AUDIO_IN_FREQUENCY / 1000UL) /UTIL_AUDIO_N_MS_DIV)    /*!< Number of samples for pcm microphone buffer ; address getter is UTIL_AUDIO_CAPTURE_getAudioBuffer*/
#define UTIL_AUDIO_OUT_SAMPLES_NB              (UTIL_AUDIO_N_MS_PER_INTERRUPT * (UTIL_AUDIO_OUT_FREQUENCY / 1000UL)/UTIL_AUDIO_N_MS_DIV)   /*!< Number of samples for rendering buffer ; address getter is UTIL_AUDIO_RENDER_getAudioBuffer*/

#define UTIL_AUDIO_BSP_OUT_SPLES_NB            (UTIL_AUDIO_OUT_SAMPLES_NB * UTIL_AUDIO_OUT_CH_NB * 2UL)                /*!< Number of samples for the bsp rendering buffer ; x2 for ping pong*/
/**
* @}
*/

/** @defgroup STM32_AUDIO_defs
* @brief    options constants
* @{
*/
#define UTIL_AUDIO_OUT_STOP_OPTION             0UL   /*!< STOP option define*/
#ifndef UTIL_AUDIO_IN_ALL_MIKES_PER_CB
#define UTIL_AUDIO_IN_ALL_MIKES_PER_CB         0U    /*!< helps to specify if the capture BSP callback was configured to return a single microphone or all of them at once */
#endif
/**
* @}
*/
/**
* @}
*/

/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
/** @defgroup STM32_AUDIO_Exported_Functions STM32_AUDIO Exported Functions
* @{
*/

/** @defgroup STM32_AUDIO_common_services
* @brief    Common services for CAPTURE & RENDERING
* @{
*/

/**
  * @brief  This utility code doesn't return error but call registered call back (while TRUE if no cb registered)
  * @param  None
  * @retval None
  */
void UTIL_AUDIO_error(void);

/**
  * @brief  Reset audio callbacks
  * @param  Handler of the callbacks
  * @retval None
  */
void UTIL_AUDIO_resetCallbacks(UTIL_AUDIO_cbs_t *const pCbs);

/**
  * @brief  Register BSP audio callbacks or other functions to control audio
  * @param  Handler of the callbacks
  * @retval None
  */
void UTIL_AUDIO_registerCallbacks(UTIL_AUDIO_cbs_t const *const pCbs, uint8_t ID);

/**
  * @brief  Init audio capture & rendering
  * @param  enable_alt : enable alternate path
  * @retval None
  */
void UTIL_AUDIO_init(uint8_t enable_alt);

/**
  * @brief  deinit audio capture & rendering
  * @param  None
  * @retval None
  */
void UTIL_AUDIO_deinit(void);

/**
  * @brief  Start audio capture & rendering
  * @param  None
  * @retval None
  */
void UTIL_AUDIO_start(void);

/**
  * @brief  Start audio capture & rendering
  * @param  None
  * @retval None
  */
void UTIL_AUDIO_stop(void);

/**
* @}
*/

/** @defgroup STM32_AUDIO_capture_services
* @brief    services for CAPTURE
* @{
*/

void UTIL_AUDIO_WAVFILE_init(void);

audio_buffer_t *UTIL_AUDIO_WAVFILE_CAPTURE_getAudioBuffer(void);

uint8_t *UTIL_AUDIO_WAVFILE_getSampleAddr(void);

/**
  * @brief  Init BSP_AUDIO_IN or other registered Audio in function. Also allocate microphone buffer
  * @param  log CMSIS OS stack/queue level
  * @retval None
  */
void UTIL_AUDIO_CAPTURE_init(bool log_enable, Path_id_t path_id);
/**
  * @brief  free output buffer
  * @param  None
  * @retval None
  */
void UTIL_AUDIO_CAPTURE_deinit(Path_id_t path_id);

/**
  * @brief  Starts BSP_AUDIO_IN or other registered Audio in function. Handles different cases (dfsdm & SAI)
  * @param  None
  * @retval None
  */
void UTIL_AUDIO_CAPTURE_start(Path_id_t path_id);

/**
  * @brief  Starts BSP_AUDIO_IN or other registered Audio in function. Handles different cases (dfsdm & SAI)
  * @param  None
  * @retval None
  */
void UTIL_AUDIO_CAPTURE_stop(Path_id_t path_id);

/**
  * @brief  Clear buffer that will be fed by microphones.
  * @param  None
  * @retval None
  */
void UTIL_AUDIO_CAPTURE_clearBuffer(Path_id_t path_id);

/**
  * @brief  Get capture buffer pointer
  * @param  pointer on an audio_buffer_t
  * @retval None
  */
audio_buffer_t *UTIL_AUDIO_CAPTURE_getAudioBuffer(Path_id_t path_id);


/**
 * @brief UTIL_AUDIO_CAPTURE_TxComplete_cb; called by every microphone DMA IT
 * @param  offsetSpleBytes (to manage half, full or more)
 * @param  instance (to manage type of data sent, one mic only or multiple; or other info if needed)
 * @retval None
*/
void UTIL_AUDIO_CAPTURE_TxComplete_cb(uint32_t const offsetSpleBytes, Path_id_t path_id);

/**
  * @brief  Get capture state active = 1 disabled = 0
  * @param  None
  * @retval True if capture is running
  */
bool UTIL_AUDIO_CAPTURE_used(void);

/**
  * @brief  Get ratio of conversion used in pdm2pcm library
  * @param  None
  * @retval None
  */
uint32_t UTIL_AUDIO_CAPTURE_getPdm2PcmLibRatio(void);

/**
  * @brief  Get microphone clk
  * @param  None
  * @retval None
  */
uint32_t UTIL_AUDIO_CAPTURE_getMicClk(void);

/**
  * @brief  Get ratio of conversion used in pdm2pcm library
  * @param  None
  * @retval None
  */
uint32_t UTIL_AUDIO_CAPTURE_getHalfBuffOffsetBytes(Path_id_t path_id);

/**
  * @brief  Set the device
  * @param  path_id
  * @retval None
  */
void UTIL_AUDIO_CAPTURE_setDevice(Path_id_t path_id);
/**
* @}
*/

/** @defgroup STM32_AUDIO_render_services
* @brief    services for RENDERING
* @{
*/
/**
  * @brief  Init BSP_AUDIO_OUT or other registered Audio out function. Also allocate output buffer
  * @param  None
  * @retval None
  */
void UTIL_AUDIO_RENDER_init(Path_id_t path_id);

/**
  * @brief  free output buffer
  * @param  None
  * @retval None
  */
void UTIL_AUDIO_RENDER_deinit(Path_id_t path_id);

/**
  * @brief  Starts BSP_AUDIO_OUT or other registered Audio out function.
  * @param  None
  * @retval None
  */
void UTIL_AUDIO_RENDER_start(Path_id_t path_id);

/**
  * @brief  Stops BSP_AUDIO_OUT or other registered Audio out function.
  * @param  None
  * @retval None
  */
void UTIL_AUDIO_RENDER_stop(Path_id_t path_id);

/**
  * @brief  Set the device
  * @param  device
  * @retval None
  */
void UTIL_AUDIO_RENDER_setDevice(uint32_t device);

/**
  * @brief  Get rendering audio_buffer_t pointer (with all meta data)
  * @param  pointer on an audio_buffer_t
  * @retval None
  */
audio_buffer_t *UTIL_AUDIO_RENDER_getAudioBuffer(Path_id_t path_id);

/**
  * @brief  Get rendering sample buffer half address
  * @param  None
  * @retval None
  */
void *UTIL_AUDIO_RENDER_getHalfAddr(Path_id_t path_id);

/**
  * @brief  Get rendering sample buffer base address
  * @param  None
  * @retval None
  */
void *UTIL_AUDIO_RENDER_getBaseAddr(Path_id_t path_id);

/**
  * @brief  Clears rendering buffer (upon mute or stop it can be useful to avoid pops).
  * @param  None
  * @retval None
  */
void UTIL_AUDIO_RENDER_clearBuffer(void);

/**
 * @brief
 * @param  Volume linear
 * @retval None
 */
void UTIL_AUDIO_RENDER_setVolume(uint16_t const Volume);

/**
 * @brief  Request a volume change, will be applied asynchronously
 * @param  Volume linear
 * @retval None
 */
void UTIL_AUDIO_RENDER_requestVolumeChange(uint16_t const Volume);

/**
 * @brief  apply request volume change,
 * @param  None
 * @retval None
 */
void UTIL_AUDIO_RENDER_applyVolumeChange(void);

/**
  * @brief Unique function for both half and complete Tx
  * @param pointer on data
  * @retval None
  */
void UTIL_AUDIO_RENDER_TxComplete_cb(void *const pData, uint8_t path_ID);

/**
  * @brief This function returns nBbytes remaining in audio out DMA buffer.
           Should be redefined inside stm32_audio_boardxxname.c
  * @param None
  * @retval nb bytes
  */
uint32_t UTIL_AUDIO_RENDER_getNbRemainingBytesDma(void);

/**
  * @brief  Get capture state active = 1 disabled = 0
  * @param  None
  * @retval True if capture is running
  */
bool UTIL_AUDIO_RENDER_used(void);
/**
  * @brief  Check if rendering is started
  * @param  None
  * @retval True if is started
  */
bool UTIL_AUDIO_RENDER_isStarted(Path_id_t path_id);

/**
* @}
*/

/** @defgroup STM32_AUDIO_weak_services
* @brief    weak function that implement a default behavior that can be overwritten
* @{
*/

/**
* @brief  User function that is called to set all callbacks needed for audio; calling UTIL_AUDIO_registerCallbacks
* @param  none
* @retval None
*/
void UTIL_AUDIO_setCallbacks(Path_id_t path_id);

/**
* @brief  User function with a predefined behavior given as examples. (see implementation of function which starts, cycle counting, synchronous rendering, asynchronous volume change etc...)
* @param  none
* @retval None
*/
void UTIL_AUDIO_process(void);

/**
 * @brief UTIL_AUDIO_feed; weak function, called at the pace of audio interrupt. Allows to distribute audio to different components SW or HW sucha USB streaming for instance
 * @param  None
 * @retval None
 */
void UTIL_AUDIO_feed(void);

/**
* @}
*/

/** @defgroup STM32_AUDIO_usb_services
* @brief    USB Services (weak function inside stm32_audio.c will be overloaded by stm32_audio_usb*.c files if linked
* @{
*/

/**
* @brief  USB init.
* @param  None
* @retval None
*/
void UTIL_AUDIO_USB_init(void);

/**
* @brief  USB deinit.
* @param  None
* @retval None
*/
void UTIL_AUDIO_USB_deinit(void);

/**
* @brief  USB idle.
* @param  None
* @retval None
*/
void UTIL_AUDIO_USB_idle(void);

/**
  * @brief  Get usb record audio_buffer_t pointer (with all meta data)
  * @param  pointer on an audio_buffer_t
  * @retval None
  */
audio_buffer_t *UTIL_AUDIO_USB_REC_getAudioBuffer(void);

/**
  * @brief  Clears rendering buffer (upon mute or stop it can be useful to avoid pops).
  * @param  None
  * @retval None
  */
void UTIL_AUDIO_USB_REC_clearBuffer(void);

/**
  * @brief  Sends data to usb for recording
  * @param  pointer on an audio_buffer_t
  * @retval None
  */
void UTIL_AUDIO_USB_REC_send(audio_buffer_t const *const pBuff);

/**
  * @brief  Sends data to usb for recording and merge to stereo if both pointers are set
  * @param  pointer on an audio_buffer_t
  * @param  pointer on an audio_buffer_t
  * @retval None
  */
void UTIL_AUDIO_USB_REC_mergeAndSend(audio_buffer_t const *const pIn1Buffer, audio_buffer_t const *const pIn2Buffer);

/**
  * @brief  Specifies which channels to used from both pointer of UTIL_AUDIO_USB_REC_mergeAndSend
  * @param  channels of first pointer on an audio_buffer_t pIn1Buffer
  * @param  channels of second pointer on an audio_buffer_t pIn2Buffer
  * @retval None
  */
void UTIL_AUDIO_USB_REC_setChannelsId(uint8_t ch1, uint8_t ch2);

/**
  * @brief  Get channels index of the pIn1Buffer used by UTIL_AUDIO_USB_REC_mergeAndSend
  * @param  None
  * @retval channel index
  */
uint8_t UTIL_AUDIO_USB_REC_getChannelsId1(void);

/**
  * @brief  Get channels index of the pIn2Buffer used by UTIL_AUDIO_USB_REC_mergeAndSend
  * @param  None
  * @retval channel index
  */
uint8_t UTIL_AUDIO_USB_REC_getChannelsId2(void);

/**
  * @brief  Get usb play audio_buffer_t pointer (with all meta data)
  * @param  pointer on an audio_buffer_t
  * @retval None
  */
audio_buffer_t *UTIL_AUDIO_USB_PLAY_getAudioBuffer(void);

/**
  * @brief  Clears rendering buffer (upon mute or stop it can be useful to avoid pops).
  * @param  None
  * @retval None
  */
void UTIL_AUDIO_USB_PLAY_clearBuffer(void);

/**
* @brief  Weak function to implement in order to copy data from USB play into the pointer on audio_buffer_t passed as argument.
* @param  pointer on an audio_buffer_t
* @retval ERROR
*/
int32_t UTIL_AUDIO_USB_PLAY_get(audio_buffer_t const *const pBuff);

/**
* @brief  Weak function to implement based on the USB stack used. Some USB audio class need to update internal infos at pace of tick.
* @param  None
* @retval None
*/
void UTIL_AUDIO_USB_PLAY_incTick(void);

/**
* @brief  Called upon audio usb streaming start.
* @param  None
* @retval None
*/
void UTIL_AUDIO_USB_PLAY_start_cb(void);

/**
* @brief  Called upon audio usb Mute.
* @param  None
* @retval None
*/
void UTIL_AUDIO_USB_PLAY_mute_cb(void);

/**
* @brief  Called upon audio usb stops
* @param  none
* @retval None
*/
void UTIL_AUDIO_USB_PLAY_stop_cb(void);

/**
* @brief  Called upon audio usb pause
* @param  none
* @retval None
*/
void UTIL_AUDIO_USB_PLAY_pause_cb(void);

/**
* @brief  Called upon audio usb resume
* @param  none
* @retval None
*/
void UTIL_AUDIO_USB_PLAY_resume_cb(void);
/**
* @}
*/

/** @defgroup STM32_AUDIO_cycles_services
* @brief    cycles count functions
* @{
*/

/**
* @brief  Initialize cycles count of audio capture interrupt
* @param  none
* @retval None
*/
void UTIL_AUDIO_INOUT_IRQ_cycleMeasure_Init(void);

/**
* @brief  Reset cycles count of audio capture interrupt
* @param  none
* @retval None
*/
void UTIL_AUDIO_INOUT_IRQ_cycleMeasure_Reset(void);

/**
* @brief  Starts cycles count of audio capture interrupt
* @param  none
* @retval None
*/
void UTIL_AUDIO_INOUT_IRQ_cycleMeasure_Start(void);

/**
* @brief  Stops cycles count of audio capture interrupt
* @param  none
* @retval None
*/
void UTIL_AUDIO_INOUT_IRQ_cycleMeasure_Stop(void);

/**
* @brief  Initialize cycles count of audio capture processing (UTIL_AUDIO_process)
* @param  none
* @retval None
*/
void UTIL_AUDIO_cycleMeasure_Init(void);

/**
* @brief  Reset cycles count of audio capture processing (UTIL_AUDIO_process)
* @param  none
* @retval None
*/
void UTIL_AUDIO_cycleMeasure_Reset(void);

/**
* @brief  Starts cycles count of audio capture processing (UTIL_AUDIO_process)
* @param  none
* @retval None
*/
void UTIL_AUDIO_cycleMeasure_Start(void);

/**
* @brief  Stops cycles count of audio capture processing (UTIL_AUDIO_process)
* @param  none
* @retval None
*/
void UTIL_AUDIO_cycleMeasure_Stop(void);

/**
  * @}
  */

/**
* @}
*/

/**
* @}
*/

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __STM32_AUDIO_H */
