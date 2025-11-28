/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Ha Thach (tinyusb.org)
 * Copyright (c) 2020 Jerzy Kasenberg
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "tusb_hw_config.h"

//--------------------------------------------------------------------+
// Board Specific Configuration
//--------------------------------------------------------------------+

#ifndef TRUE
#define TRUE (1)
#endif

#ifndef FALSE
#define FALSE (0)
#endif

// RHPort number used for device can be defined by board.mk, default to port 0
#ifndef BOARD_TUD_RHPORT
#define BOARD_TUD_RHPORT      0
#endif


// RHPort max operational speed can defined by board.mk
#ifndef CFG_TUD_MAX_SPEED
#define CFG_TUD_MAX_SPEED   OPT_MODE_FULL_SPEED
#endif

//--------------------------------------------------------------------
// Common Configuration
//--------------------------------------------------------------------

// defined by compiler flags for flexibility
#ifndef CFG_TUSB_MCU
#error  "Select the MCU in CFG_TUSB_MCU"
#endif


//#define CFG_TUSB_DEBUG        3

#if CFG_TUSB_DEBUG != 0
/* by default CFG_TUSB_DEBUG_PRINTF is defined as printf() and crashes if called from interrupt, so replace it by a better implementation */
#define CFG_TUSB_DEBUG_PRINTF tud_log
#endif

//--------------------------------------------------------------------
// Common Configuration
//--------------------------------------------------------------------

// defined by compiler flags for flexibility

#ifndef CFG_TUSB_OS
#define CFG_TUSB_OS           OPT_OS_NONE
#endif

#define CFG_SHOW_DESCRIPTOR                     0

// Enable Device stack
#define CFG_TUD_ENABLED       1
#define USB_PID_BASE                                   0x4143U       // BASE PID is "AC"
#define CFG_MAX_DESCRIPTOR_SIZE                       (400)
#define USBD_VID                                      0x0483
#define CFG_USB_DCDDEVICE                             (0x100)



#define  TUD_MANUFACTURER_STRING_INDEX       1  // 1: Manufacturer
#define  TUD_PRODUCT_STRING_INDEX            2  // 2: Product
#define  TUD_PLAY_STRING_INDEX               4  // 4: Audio Interface
#define  TUD_REC_STRING_INDEX                5  // 5: Audio Interface
#define  TUD_HID_STRING_INDEX                6  // 6: debug Interface
#define  TUD_CDC_VPC0_STRING_INDEX           7  // 7: debug Interface
#define  TUD_MIDI_STRING_INDEX               8  // 8: debug Interface
#define  TUD_VIDEO_CTRL_STRING_INDEX         9  // 9: debug Interface
#define  TUD_VIDEO_STREAM_STRING_INDEX       10 // 10: debug Interface   
#define  TUD_CDC_VPC1_STRING_INDEX           11 // 11: debug Interface   
#define  TUD_CDC_TRACEALYZER_STRING_INDEX    12 // 12: debug Interface   





/* USB DMA on some MCUs can only access a specific SRAM region with restriction on alignment.
 * Tinyusb use follows macros to declare transferring memory so that they can be put
 * into those specific section.
 * e.g
 * - CFG_TUSB_MEM SECTION : __attribute__ (( section(".usb_ram") ))
 * - CFG_TUSB_MEM_ALIGN   : __attribute__ ((aligned(4)))
 */
#ifndef CFG_TUSB_MEM_SECTION
#define CFG_TUSB_MEM_SECTION
#endif

#ifndef CFG_TUSB_MEM_ALIGN
#define CFG_TUSB_MEM_ALIGN        __attribute__ ((aligned(4)))
#endif




//--------------------------------------------------------------------
// DEVICE CONFIGURATION
//--------------------------------------------------------------------

#ifndef CFG_TUD_ENDPOINT0_SIZE
#define CFG_TUD_ENDPOINT0_SIZE    64
#endif



#ifndef TUD_USE_UNIQUE_SERIAL_ID
#define TUD_USE_UNIQUE_SERIAL_ID      0
#endif


// MIDI FIFO size of TX and RX
#define CFG_TUD_MIDI_RX_BUFSIZE   (TUD_OPT_HIGH_SPEED ? 512 : 64)
#define CFG_TUD_MIDI_TX_BUFSIZE   (TUD_OPT_HIGH_SPEED ? 512 : 64)



//------------- CLASS -------------//
#define MOUNT_BIT(a) (1UL << (a))
#define CFG_TUD_HID_BUFSIZE      (TUD_OPT_HIGH_SPEED ? 512 : 64)

#if defined(TUD_AUDIO_IN_USED) || defined(TUD_AUDIO_OUT_USED) // for legacy compatibility
#define CFG_TUD_UAC2_AUDIO      1
#endif

#define CFG_UAC2_AUDIO_ENABLED  MOUNT_BIT(0)
#ifndef CFG_TUD_UAC2_AUDIO
#define CFG_TUD_UAC2_AUDIO       0
#endif

#define CFG_CDC_VP0_ENABLED  MOUNT_BIT(1)
#ifndef CFG_TUD_CDC_VPC0
#define CFG_TUD_CDC_VPC0 1
#endif

#define CFG_CDC_VP1_ENABLED  MOUNT_BIT(2)
#ifndef CFG_TUD_CDC_VPC1
#define CFG_TUD_CDC_VPC1  0
#endif

#define CFG_HID_MONITOR_ENABLED  MOUNT_BIT(3)
#ifndef CFG_TUD_HID_MONITOR
#define CFG_TUD_HID_MONITOR       0
#endif


#define CFG_AWE4_HID_ENABLED  MOUNT_BIT(4)
#ifndef CFG_TUD_AWE4_HID
#define CFG_TUD_AWE4_HID         0
#endif

#define CFG_MIDI_IN_ENABLED  MOUNT_BIT(5)
#ifndef  CFG_TUD_MIDI_IN
#define  CFG_TUD_MIDI_IN         0
#endif

#define CFG_VIDEO_UVC_ENABLED  MOUNT_BIT(6)
#ifndef CFG_TUD_VIDEO_UVC
#define CFG_TUD_VIDEO_UVC  0
#endif



#ifndef CFG_TUD_TRACEALYZER_CDC
#define CFG_TUD_TRACEALYZER_CDC  0
#endif

/* by default , only one USB  bulk size   is allowed for a fifo, it is short, we need to tune this size according to the throughput expected for the use case */
#if   CFG_TUD_TRACEALYZER_CDC
#define CFG_TUD_CDC_TX_BUFSIZE  (156*64) /* We need an hight bw for the TX */
#endif

#if  CFG_TUD_CDC_VPC0 || CFG_TUD_CDC_VPC1
#ifndef CFG_TUD_CDC_TX_BUFSIZE
#define CFG_TUD_CDC_TX_BUFSIZE  (30*1024) /* We need an hight bw for the TX */
#endif
#endif



// CDC FIFO size of TX and RX*
#ifndef CFG_TUD_CDC_RX_BUFSIZE
#define CFG_TUD_CDC_RX_BUFSIZE   (TUD_OPT_HIGH_SPEED ? 512 : 64)
#endif

#ifndef CFG_TUD_CDC_TX_BUFSIZE
#define CFG_TUD_CDC_TX_BUFSIZE   (TUD_OPT_HIGH_SPEED ? 512 : 64)
#endif





/* TRUE if we add the class implementation */

#define CFG_TUD_CDC               (CFG_TUD_CDC_VPC0+CFG_TUD_TRACEALYZER_CDC + CFG_TUD_CDC_VPC1)
#define CFG_TUD_MSC               0
#define CFG_TUD_MIDI              ((CFG_TUD_MIDI_IN)?1:0)
#define CFG_TUD_AUDIO             ((CFG_TUD_UAC2_AUDIO)?1:0)
#define CFG_TUD_VENDOR            0
#define CFG_TUD_HID               ((CFG_TUD_HID_MONITOR+CFG_TUD_AWE4_HID)?1:0)
#define CFG_TUD_VIDEO             ((CFG_TUD_VIDEO_UVC)?1:0)



// video support
#ifdef CFG_TUD_VIDEO_UVC
// The number of video streaming interfaces
#define CFG_TUD_VIDEO_STREAMING  1

// video streaming endpoint buffer size
#define CFG_TUD_VIDEO_STREAMING_EP_BUFSIZE  1024

// use bulk endpoint for streaming interface
#define CFG_TUD_VIDEO_STREAMING_BULK 1


#endif




//--------------------------------------------------------------------
// AUDIO CLASS DRIVER CONFIGURATION
//--------------------------------------------------------------------
#define CFG_TUD_AUDIO_FUNC_1_DESC_LEN                                tud_get_dynamic_descriptor_len()

// How many formats are used, need to adjust USB descriptor if changed
#define CFG_TUD_AUDIO_FUNC_1_N_FORMATS                               1

// Audio format type I specifications
#if (UTIL_AUDIO_MAX_IN_FREQUENCY == 96000U) || (UTIL_AUDIO_MAX_OUT_FREQUENCY == 96000U)
#define CFG_TUD_AUDIO_MAX_SAMPLE_RATE                                96000U
#else
#define CFG_TUD_AUDIO_MAX_SAMPLE_RATE                                48000U
#endif
  
  
#define CFG_CONFIG_BMATTRIBUTES                                      0 
#define CFG_USB_CONFIG_MAXPOWER                                      500U

#define CFG_TUD_AUDIO_MAX_CHANNELS                                   2U
#define CFG_TUD_AUDIO_MAX_CHANNELS_RX                                CFG_TUD_AUDIO_MAX_CHANNELS
#define CFG_TUD_AUDIO_MAX_CHANNELS_TX                                CFG_TUD_AUDIO_MAX_CHANNELS
#define CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX                           CFG_TUD_AUDIO_MAX_CHANNELS_TX
#define CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX                           CFG_TUD_AUDIO_MAX_CHANNELS_RX

#define CFG_TUD_AUDIO_MAX_SPLE_SIZE                                  4U // 2U  /* 2 if 16-bit samples ; 4 if 32-bit samples */

#define CFG_TUD_AUDIO_FUNC_1_FORMAT_1_MAX_N_BYTES_PER_SAMPLE_RX      CFG_TUD_AUDIO_MAX_SPLE_SIZE
#define CFG_TUD_AUDIO_FUNC_1_FORMAT_1_MAX_N_BYTES_PER_SAMPLE_TX      CFG_TUD_AUDIO_MAX_SPLE_SIZE

// the host can modify the frequency from a set of frequencies */
#define CFG_DYNAMIC_MULTI_FREQUENCY                                  0  /* use static frequency at bootup */

#if (UTIL_AUDIO_MAX_IN_FREQUENCY == 96000U) || (UTIL_AUDIO_MAX_OUT_FREQUENCY == 96000U)
#define CFG_DYNAMIC_MULTI_FREQUENCY_SET                              {8000, 16000, 48000, 96000}
#else
#define CFG_DYNAMIC_MULTI_FREQUENCY_SET                              {8000, 16000, 48000}
#endif

// Enable/disable feedback EP (required for asynchronous play applications)
#define CFG_TUD_AUDIO_ENABLE_FEEDBACK_EP                    1                           // Feedback - 0 or 1

// EP and buffer size - for isochronous EP´s, the buffer and EP size are equal (different sizes would not make sense)
#define CFG_TUD_AUDIO_ENABLE_EP_IN                1

// EP and buffer size - for isochronous EP´s, the buffer and EP size are equal (different sizes would not make sense)
#define CFG_TUD_AUDIO_ENABLE_EP_OUT               1

#define CFG_TUD_AUDIO_FUNC_1_FORMAT_1_EP_SZ_IN    TUD_AUDIO_EP_SIZE(CFG_TUD_AUDIO_MAX_SAMPLE_RATE, CFG_TUD_AUDIO_FUNC_1_FORMAT_1_MAX_N_BYTES_PER_SAMPLE_TX, CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX)

/* these macros are used for the fifo buffer size */
#define CFG_TUD_AUDIO_FUNC_1_EP_IN_SW_BUF_SZ      (CFG_TUD_AUDIO_FUNC_1_FORMAT_1_EP_SZ_IN*2)
#define CFG_TUD_AUDIO_FUNC_1_EP_IN_SZ_MAX         (CFG_TUD_AUDIO_FUNC_1_FORMAT_1_EP_SZ_IN) // Maximum EP IN size for all AS alternate settings used

#define CFG_TUD_AUDIO_UNC_1_FORMAT_1_EP_SZ_OUT    TUD_AUDIO_EP_SIZE(CFG_TUD_AUDIO_MAX_SAMPLE_RATE, CFG_TUD_AUDIO_FUNC_1_FORMAT_1_MAX_N_BYTES_PER_SAMPLE_RX, CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX)
#define CFG_TUD_AUDIO_FUNC_1_EP_OUT_SW_BUF_SZ     (CFG_TUD_AUDIO_UNC_1_FORMAT_1_EP_SZ_OUT * 2)
#define CFG_TUD_AUDIO_FUNC_1_EP_OUT_SZ_MAX        (CFG_TUD_AUDIO_UNC_1_FORMAT_1_EP_SZ_OUT)


// Number of Standard AS Interface Descriptors (4.9.1) defined per audio function - this is required to be able to remember the current alternate settings of these interfaces - We restrict us here to have a constant number for all audio functions (which means this has to be the maximum number of AS interfaces an audio function has and a second audio function with less AS interfaces just wastes a few bytes)
#define CFG_TUD_AUDIO_FUNC_1_N_AS_INT             2

// Size of control request buffer
#define CFG_TUD_AUDIO_FUNC_1_CTRL_BUF_SZ  64

uint32_t tud_get_dynamic_descriptor_len(void);
int      tud_log(const char *pFormat, ...);




#ifdef __cplusplus
}
#endif

#endif /* _TUSB_CONFIG_H_ */
