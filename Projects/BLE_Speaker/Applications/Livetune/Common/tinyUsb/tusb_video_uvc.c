/**
******************************************************************************
* @file    tusb_video_uvc.c
* @author  MCD Application Team
* @brief   Instanciate a video class UVC
*
*
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

/* Includes ------------------------------------------------------------------*/
/*cstat +MISRAC2012-* */
#include "tusb.h"
#include "tinyusb.h"

/*
Add the following flags to the project
CFG_TUD_VIDEO_UVC=1


*/
#if CFG_TUD_VIDEO_UVC  
#include "tinyusb.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
/*cstat -MISRAC2012-* tinyusb not misra compliant */
#include "tusb.h"
#include "usb_descriptors.h"

/* private defines -----------------------------------------------------------*/
#define   ITF_NUM_VIDEO_STREAMING               (interfacevideo+1)
#define   ITF_NUM_VIDEO_CONTROL                 (interfacevideo)
#define   ITF_NUM_TOTAL                         (2)
#define   FRAME_INTERVAL(n)                     (10000000U/(n))
#define   UVC_CLOCK_FREQUENCY                   (48*1000*1000)
#define   UVC_ENTITY_CAP_INPUT_TERMINAL         (0x01)
#define   UVC_ENTITY_CAP_OUTPUT_TERMINAL        (0x02)
#define   USE_ISO_STREAMING                     (1)


/* Private typedef -----------------------------------------------------------*/
/* Windows support 
 * https://docs.microsoft.com/en-us/windows-hardware/drivers/stream/usb-video-class-driver-overview */

/* control block */
typedef struct TU_ATTR_PACKED {
  tusb_desc_interface_t itf;
  tusb_desc_video_control_header_1itf_t header;
  tusb_desc_video_control_camera_terminal_t camera_terminal;
  tusb_desc_video_control_output_terminal_t output_terminal;
} uvc_control_desc_t;


/* Streaming block */

typedef struct TU_ATTR_PACKED {
  tusb_desc_interface_t itf;
  tusb_desc_video_streaming_input_header_1byte_t header;
  struct TU_ATTR_PACKED
  {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubType;
    uint8_t bFormatIndex;
    uint8_t bNumFrameDescriptors;
    uint8_t guidFormat[16];
    uint8_t bBitsPerPixel;
    uint8_t bDefaultFrameIndex;
    uint8_t bAspectRatioX;
    uint8_t bAspectRatioY;
    uint8_t bmInterlaceFlag;
    uint8_t bCopyProtect;
    uint8_t bVariableSize;
  } format;

  /* Frame Descriptor */

  struct TU_ATTR_PACKED
  {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubType;
    uint8_t bFrameIndex;
    uint8_t bmCapabilities;
    uint16_t wWidth;
    uint16_t wHeight;
    uint32_t dwMinBitRate;
    uint32_t dwMaxBitRate;
    uint32_t dwDefaultFrameInterval;
    uint8_t bFrameIntervalType;
    uint32_t dwBytesPerLine;
    uint32_t dwFrameInterval;
  }  frame;  
} uvc_streaming_desc_t;

/* Terminator  block */

typedef struct TU_ATTR_PACKED {

  tusb_desc_video_streaming_color_matching_t color;
#if USE_ISO_STREAMING
  // For ISO streaming, USB spec requires to alternate interface
  tusb_desc_interface_t itf_alt;
#endif
  tusb_desc_endpoint_t ep;
} uvc_end_desc_t;


/* Complet UVC descriptor */

typedef struct TU_ATTR_PACKED {
  tusb_desc_interface_assoc_t iad;
  uvc_control_desc_t video_control;
  uvc_streaming_desc_t video_streaming;
  uvc_end_desc_t       video_end;
} uvc_cfg_desc_t;

/* Private functions ---------------------------------------------------------*/

static void              s_signal_frame_complete_bare_metal(uint_fast8_t ctl_idx, uint_fast8_t stm_idx);
static int32_t           s_wait_frame_complete_bare_metal(void);
static void              s_video_desc_frame_based_h264(uint8_t **pDescriptor);

/* Private variables ---------------------------------------------------------*/

static int32_t                          interfacevideo;
static int8_t                           EP_NUM_VIDEO_IN ;
static uint8_t                          bFrameSignaled;
static bool                             bFrameComplet;
static vs_frame_complete_cb             vs_signal_frame_complete=s_signal_frame_complete_bare_metal;
static vs_commit_cb                     vs_commit=NULL;
static vs_wait_frame_complete_cb        vs_wait_frame_complet=s_wait_frame_complete_bare_metal;


/* 

  We can support more codecs using this list by adding a callback associated with the ID set in the configuration struct. 
  Many codecs can be inherited from the H264 descriptor; only the GUID and frame/format descriptor must be updated.
*/ 
struct {
  void     (*descritorBuilder)(uint8_t **pDescriptor);
  uint8_t  descID;
}tDescriptorList[]=
{
  {
    .descritorBuilder = s_video_desc_frame_based_h264,
    .descID           = TUSB_VIDEO_DESC_ID_H264_FRAME_BASED
  },
  {0,0}
};



/**
* @brief default bare metal signal frame complet
* @param ctl_idx  index of bound video control interface
* @param stm_idx  index from the video control interface 
*/

static void  s_signal_frame_complete_bare_metal(uint_fast8_t ctl_idx, uint_fast8_t stm_idx)
{
  static int cpt=0;
  bFrameSignaled = TRUE;
}


/**
* @brief Wait a complet transmission bare metal implementation 
*/

static int32_t s_wait_frame_complete_bare_metal(void)
{
  bFrameSignaled = FALSE;
  while(!bFrameSignaled);
  return TRUE;
}


/* Functions Definition ------------------------------------------------------*/

/**
* @brief called from the descriptor construction allow to build an interface index
*
* @param numInterface  the ref num interface
*/
static void tusb_video_h264_add_interface(int32_t *numInterface)
{
  interfacevideo = *numInterface;
  (*numInterface)++;
  (*numInterface)++;
}

/**
* @brief Overload callback for RTOS implementation 
* @param signal_frame_complete   signal frame complete
* @param vs_wait_frame_complete_cb  wait for a frame completly transmited 
* @param vs_commit_cb          commit and probe callback 
*/

int32_t tusb_video_set_cb( vs_frame_complete_cb signal_frame_complete,vs_wait_frame_complete_cb wait_frame_complete,vs_commit_cb vs_commit)
{
  if(wait_frame_complete)
  {
    vs_wait_frame_complet = wait_frame_complete;
  }
  else
  {
    vs_wait_frame_complet = s_wait_frame_complete_bare_metal;
  }
  
  if(signal_frame_complete)
  {
    vs_signal_frame_complete=signal_frame_complete;
  }
  else
  {
    vs_signal_frame_complete=s_signal_frame_complete_bare_metal;
  }
  vs_commit=vs_commit;
  return TRUE;
}



/**
* @brief Push a video frame. 

   The packetization will be done by TinyUSB. 
   Frames will be split into packets of CFG_TUD_VIDEO_STREAMING_EP_BUFSIZE (1024). 
   The packetized data will form packets with a header of 12 bytes + payload and will emit packets until the szBuffer is empty.

* @param pBuffer   payload buffer 
* @param szBuffer  payload size 
* @return count sent 
*/

int32_t tusb_video_push(uint8_t *pBuffer, uint32_t szBuffer)
{
  uint32_t count = 0;
  if (tud_ready())
  {
    if(!tud_video_n_frame_xfer(0,0, pBuffer,szBuffer))
    {
      return FALSE;
    }
    if(!vs_wait_frame_complet())
    {
      return FALSE;
    }
  }
  return TRUE;
}
/**
* @brief Callback frame complet
*@param ctl_idx  index of bound video control interface
*@param stm_idx  index from the video control interface 
*/

void tud_video_frame_xfer_complete_cb(uint_fast8_t ctl_idx, uint_fast8_t stm_idx)
{
  /* Delegation in  callbacks */
    vs_signal_frame_complete(ctl_idx,stm_idx);
}

/**
* @brief Callback VS Commit 
* @param ctl_idx  index of bound video control interface
* @param stm_idx  index from the video control interface 
* @param  parameters  Probe and Commit control 
* @return error  code 
*/

int tud_video_commit_cb(uint_fast8_t ctl_idx,uint_fast8_t stm_idx,video_probe_and_commit_control_t const *parameters)
{
  int32_t ret = VIDEO_ERROR_NONE;
  if(vs_commit) 
  {
    /* Delegation in callbacks */
    ret = vs_commit(ctl_idx,stm_idx,(struct video_probe_and_commit_control_t const *)parameters);
  }
  return ret;
}






/**
* @brief Create a descriptor composite for UVC Frame_based H264

  A descripor UVC is composed like this  2 interfaces , one for the control and one for the streaming 
  The strcuture UVC for Frame based 
    IAD 
    //Control definition 
    ITF  control
        header
        terminal input 
        terminal output 
    ITF streaming
        header
       format stream definition ( frame based h264)
       frame  stream definition 

  alt 1 streaming defintion 
  color format description ( frame based)
  endpoint description

  Many fields are set to 0. In order to save code generation, the descriptor is filled with zeros before initialization, 
  and all fields set to zero are under comment to keep in mind its value must be 0.

  @param uint8_t **pDescriptor Pointer descriptor under construction.
*/


static void s_video_desc_frame_based_h264(uint8_t **pDescriptor)
{
  uvc_cfg_desc_t *pBase =(uvc_cfg_desc_t*)(*pDescriptor);
  memset(pBase,0,sizeof(*pBase));
  
  pBase->iad.bLength = sizeof(pBase->iad);
  pBase->iad.bDescriptorType = TUSB_DESC_INTERFACE_ASSOCIATION ; /* IAD descriptor */
  pBase->iad.bFirstInterface = ITF_NUM_VIDEO_CONTROL;
  pBase->iad.bInterfaceCount = ITF_NUM_TOTAL;    /* 2 interfaces */
  pBase->iad.bFunctionClass = TUSB_CLASS_VIDEO; 
  pBase->iad.bFunctionSubClass = VIDEO_SUBCLASS_INTERFACE_COLLECTION;
  pBase->iad.bFunctionProtocol = VIDEO_ITF_PROTOCOL_UNDEFINED;
  pBase->iad.iFunction = TUD_VIDEO_STREAM_STRING_INDEX; /* String Index */
    
  /* defines the interface video control */
  
  pBase->video_control.itf.bLength = sizeof(pBase->video_control.itf);
  pBase->video_control.itf.bDescriptorType = TUSB_DESC_INTERFACE;
  pBase->video_control.itf.bInterfaceNumber = ITF_NUM_VIDEO_CONTROL;
//  pBase->video_control.itf.bAlternateSetting = 0U;
//  pBase->video_control.itf.bNumEndpoints = 0U;  // bulk 1, iso 0
  pBase->video_control.itf.bInterfaceClass = TUSB_CLASS_VIDEO;
  pBase->video_control.itf.bInterfaceSubClass = VIDEO_SUBCLASS_CONTROL;
  pBase->video_control.itf.bInterfaceProtocol = VIDEO_ITF_PROTOCOL_UNDEFINED;
  pBase->video_control.itf.iInterface = TUD_VIDEO_CTRL_STRING_INDEX; 
  
  /* Header tree control, wTotalLength will be updated later  */
  
  pBase->video_control.header.bLength = sizeof(pBase->video_control.header);
  pBase->video_control.header.bDescriptorType    = TUSB_DESC_CS_INTERFACE;
  pBase->video_control.header.bDescriptorSubType = VIDEO_CS_ITF_VC_HEADER; // 0x01U;
  pBase->video_control.header.bcdUVC             = 0x0110U; /* version 1.10 */
//  pBase->video_control.header.wTotalLength       = -1; // HEADER_SIZE;
  pBase->video_control.header.dwClockFrequency   = UVC_CLOCK_FREQUENCY;
  pBase->video_control.header.bInCollection      = 0x01U; // The number of VideoStreaming interfaces
  pBase->video_control.header.baInterfaceNr[0]       = ITF_NUM_VIDEO_STREAMING; // Interface number of the first VideoStreaming interface in the Collection
  
  /* Terminal camera def */
  
  pBase->video_control.camera_terminal.bLength = sizeof(pBase->video_control.camera_terminal);
  pBase->video_control.camera_terminal.bDescriptorType = TUSB_DESC_CS_INTERFACE;
  pBase->video_control.camera_terminal.bDescriptorSubType = VIDEO_CS_ITF_VC_INPUT_TERMINAL; // 0x02U;
  pBase->video_control.camera_terminal.bTerminalID = UVC_ENTITY_CAP_INPUT_TERMINAL;
  pBase->video_control.camera_terminal.wTerminalType = VIDEO_ITT_CAMERA; /* CAMERA */
//  pBase->video_control.camera_terminal.bAssocTerminal = 0x00U;
//  pBase->video_control.camera_terminal.iTerminal =  0x00U;
//  pBase->video_control.camera_terminal.wObjectiveFocalLengthMin = 0x0000; /* wObjectiveFocalLengthMin */
//  pBase->video_control.camera_terminal.wObjectiveFocalLengthMax = 0x0000; /* wObjectiveFocalLengthMax */
//  pBase->video_control.camera_terminal.wOcularFocalLength = 0x0000;       /* wOcularFocalLength       */
  pBase->video_control.camera_terminal.bControlSize = 0x03;               /* bControlSize             */
//  pBase->video_control.camera_terminal.bmControls[0] = 0x00U;             /* bmControls               */
//  pBase->video_control.camera_terminal.bmControls[1] = 0x00U;
//  pBase->video_control.camera_terminal.bmControls[2] = 0x00U;

  /* Terminal def */

  pBase->video_control.output_terminal.bLength = sizeof(pBase->video_control.output_terminal);
  pBase->video_control.output_terminal.bDescriptorType = TUSB_DESC_CS_INTERFACE;
  pBase->video_control.output_terminal.bDescriptorSubType = VIDEO_CS_ITF_VC_OUTPUT_TERMINAL; //  0x03U;
  pBase->video_control.output_terminal.bTerminalID = UVC_ENTITY_CAP_OUTPUT_TERMINAL;
  pBase->video_control.output_terminal.wTerminalType = VIDEO_TT_STREAMING;
//  pBase->video_control.output_terminal.bAssocTerminal = 0x00U;
  pBase->video_control.output_terminal.bSourceID = UVC_ENTITY_CAP_INPUT_TERMINAL;
//  pBase->video_control.output_terminal.iTerminal = 0x00U;
  
  /* video streaming interface */
  
  pBase->video_streaming.itf.bLength = sizeof(pBase->video_streaming.itf);
  pBase->video_streaming.itf.bDescriptorType  = TUSB_DESC_INTERFACE;
  pBase->video_streaming.itf.bInterfaceNumber = ITF_NUM_VIDEO_STREAMING;
//  pBase->video_streaming.itf.bAlternateSetting = 0U;
//  pBase->video_streaming.itf.bNumEndpoints = 0U; // CFG_TUD_VIDEO_STREAMING_BULK, // bulk 1, iso 0
  pBase->video_streaming.itf.bInterfaceClass = TUSB_CLASS_VIDEO;
  pBase->video_streaming.itf.bInterfaceSubClass = VIDEO_SUBCLASS_STREAMING;
  pBase->video_streaming.itf.bInterfaceProtocol = VIDEO_ITF_PROTOCOL_UNDEFINED;
//  pBase->video_streaming.itf.iInterface = 0; 
  
  
  /* Append Class-specific VS Header Descriptor (Input) to Configuration descriptor  */
  /* wTotalLength will be updated later */
  
  pBase->video_streaming.header.bLength = sizeof(pBase->video_streaming.header);
  pBase->video_streaming.header.bDescriptorType = TUSB_DESC_CS_INTERFACE;
  pBase->video_streaming.header.bDescriptorSubType = VIDEO_CS_ITF_VS_INPUT_HEADER;
  pBase->video_streaming.header.bNumFormats = 0x01U;    // only 1 format H264
  pBase->video_streaming.header.wTotalLength = -1; //  HEADER_SIZE; will be computed later 
  pBase->video_streaming.header.bEndpointAddress = EP_IN(EP_NUM_VIDEO_IN);
//  pBase->video_streaming.header.bmInfo          = 0x00U; //Indicates the capabilities of this VideoStreaming interface:D0: Dynamic Format Change supportedD7..1: Reserved, set to zero
  pBase->video_streaming.header.bTerminalLink = UVC_ENTITY_CAP_OUTPUT_TERMINAL;//The terminal ID of the Output Terminal to which the video endpoint of this interface is connected.
//  pBase->video_streaming.header.bStillCaptureMethod = 0x00U;
//  pBase->video_streaming.header.bTriggerSupport = 0x00U;
//  pBase->video_streaming.header.bTriggerUsage = 0x00U;
  pBase->video_streaming.header.bControlSize = 0x01U;
//  pBase->video_streaming.header.bmaControls[0] = 0x00U;
  
  
  
  /* Format  definition */

  pBase->video_streaming.format.bLength = sizeof(pBase->video_streaming.format);
  pBase->video_streaming.format.bDescriptorType = TUSB_DESC_CS_INTERFACE;
  pBase->video_streaming.format.bDescriptorSubType = VIDEO_CS_ITF_VS_FORMAT_FRAME_BASED;
  pBase->video_streaming.format.bFormatIndex = 0x01U;
  pBase->video_streaming.format.bNumFrameDescriptors = 0x01U;
  pBase->video_streaming.format.bBitsPerPixel = 16;
  pBase->video_streaming.format.bDefaultFrameIndex = 0x01U;
//  pBase->video_streaming.format.bAspectRatioX = 0x00U;
//  pBase->video_streaming.format.bAspectRatioY = 0x00U;
//  pBase->video_streaming.format.bmInterlaceFlag = 0x00U;
//  pBase->video_streaming.format.bCopyProtect = 0x00U;
  pBase->video_streaming.format.bVariableSize = TRUE;
  {
    uint8_t guid[] =  {TUD_VIDEO_GUID_H264};
    memcpy(&pBase->video_streaming.format.guidFormat,&guid, sizeof(pBase->video_streaming.format.guidFormat));
  }
  
  /* frame definition */
  
  pBase->video_streaming.frame.bLength = sizeof(pBase->video_streaming.frame);
  pBase->video_streaming.frame.bDescriptorType = TUSB_DESC_CS_INTERFACE;
  pBase->video_streaming.frame.bDescriptorSubType = VIDEO_CS_ITF_VS_FRAME_FRAME_BASED;
  pBase->video_streaming.frame.bFrameIndex = 0x01U;
//  pBase->video_streaming.frame.bmCapabilities = 0x00U;
  pBase->video_streaming.frame.wWidth = tusb_handle()->hConfig.video.width;
  pBase->video_streaming.frame.wHeight = tusb_handle()->hConfig.video.height;
//  pBase->video_streaming.frame.dwBytesPerLine = 0x0U;
  pBase->video_streaming.frame.bFrameIntervalType = 0x01U;
  pBase->video_streaming.frame.dwMinBitRate = tusb_handle()->hConfig.video.bitrateMin;
  pBase->video_streaming.frame.dwMaxBitRate = tusb_handle()->hConfig.video.bitrateMax;
  pBase->video_streaming.frame.dwDefaultFrameInterval = FRAME_INTERVAL(tusb_handle()->hConfig.video.fps);
  pBase->video_streaming.frame.dwFrameInterval = FRAME_INTERVAL(tusb_handle()->hConfig.video.fps);
  
 
  
#if USE_ISO_STREAMING
  
  pBase->video_end.itf_alt.bLength = sizeof(pBase->video_end.itf_alt);  
  pBase->video_end.itf_alt.bDescriptorType = TUSB_DESC_INTERFACE;  
  pBase->video_end.itf_alt.bInterfaceNumber = ITF_NUM_VIDEO_STREAMING;
  pBase->video_end.itf_alt.bAlternateSetting = 1;
  pBase->video_end.itf_alt.bNumEndpoints = 1; 
  pBase->video_end.itf_alt.bInterfaceClass = TUSB_CLASS_VIDEO;
  pBase->video_end.itf_alt.bInterfaceSubClass = VIDEO_SUBCLASS_STREAMING;
  pBase->video_end.itf_alt.bInterfaceProtocol = VIDEO_ITF_PROTOCOL_UNDEFINED;  
//  pBase->video_end.itf_alt.iInterface = 0;  
#endif
   
    /* Color mapping */  

  pBase->video_end.color.bLength = sizeof(pBase->video_end.color);
  pBase->video_end.color.bDescriptorType = TUSB_DESC_CS_INTERFACE;
  pBase->video_end.color.bDescriptorSubType = VIDEO_CS_ITF_VS_COLORFORMAT;
  pBase->video_end.color.bColorPrimaries = VIDEO_COLOR_PRIMARIES_BT709;
  pBase->video_end.color.bTransferCharacteristics = VIDEO_COLOR_XFER_CH_BT709;
  pBase->video_end.color.bMatrixCoefficients = VIDEO_COLOR_COEF_SMPTE170M;

  /* Endpoint def */  
  
  pBase->video_end.ep.bLength             = sizeof(pBase->video_end.ep); 
  pBase->video_end.ep.bDescriptorType    = TUSB_DESC_ENDPOINT; 
  pBase->video_end.ep.bEndpointAddress   = EP_IN(EP_NUM_VIDEO_IN); 
  pBase->video_end.ep.bmAttributes.xfer= 1;
  pBase->video_end.ep.bmAttributes.sync= 1;//TU_BIT(0) | TU_BIT(2); // transfert Type -> Isochronous(0x1) ;  Synchronisation-> Isochrone (0x4)
//  pBase->video_end.ep.bmAttributes.usage = 0;
  pBase->video_end.ep.wMaxPacketSize     = CFG_TUD_VIDEO_STREAMING_EP_BUFSIZE; 
  pBase->video_end.ep.bInterval          = 1; // HS_BINTERVAL
  
  /* Point to the end of the descriptor */
  *pDescriptor += sizeof(uvc_cfg_desc_t);
  
  /* Finalize block Video control  totalsize */
  pBase->video_control.header.wTotalLength = DESC_GET_LEN_16(&pBase->video_control.header, &pBase->video_streaming,0);
  /* Finalize block Video stream   totalsize */
  pBase->video_streaming.header.wTotalLength = DESC_GET_LEN_16(&pBase->video_streaming.header, &pBase->video_end,0);
  
  
}

/**
 * @brief Look for the descriptor associated with the codec and build it.
 *
 * @param pDescriptor  Descriptor instance under construction pointer
 */


static void tusb_video_uvc_descriptor(uint8_t **pDescriptor)
{
  for(int32_t index = 0; tDescriptorList[index].descritorBuilder ; index++)
  {
    if(tusb_handle()->hConfig.video.descID == tDescriptorList[index].descID)
    {
      EP_NUM_VIDEO_IN = tusb_allocate_ep();
      tDescriptorList[index].descritorBuilder(pDescriptor);
    }
  }
}



/**
 * @brief init the UVC usb instance
 *
 */

void tusb_video_uvc_init(void)
{
  if (tusb_handle()->hConfig.mount & CFG_VIDEO_UVC_ENABLED)
  {
    
    tud_extra_class_add(tusb_video_h264_add_interface, tusb_video_uvc_descriptor);
    
  }
}

#endif
