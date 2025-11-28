/**
******************************************************************************
* @file    usb_descriptors.c
* @author  MCD Application Team
* @brief   implement usb descriptor
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

/*cstat -MISRAC2012* this code section uses macros coming from a third party not misra compliant */


#include "tinyusb.h"
#include "tusb.h"
#include "usb_descriptors.h"


/* private defines  ------------------------------------------------------------------*/

#define PID_MAP(itf, n)  ( ((uint32_t)(itf)) << ((uint32_t)(n)))
#define MAX_EXTRA_INTERFACE   5


/* private variables   ------------------------------------------------------------------*/

static uint8_t  desc_configuration[CFG_MAX_DESCRIPTOR_SIZE];
static void (*tExtraInterface[MAX_EXTRA_INTERFACE])(int32_t *numInterface);
static void (*tExtraDescriptor[MAX_EXTRA_INTERFACE])(uint8_t **pDescriptor);
static uint8_t nbExtraClass;



//--------------------------------------------------------------------+
// String Descriptors
//--------------------------------------------------------------------+
// String Descriptor Index

#define USBD_VID                                      0x0483
#define USBD_MANUFACTURER_STRING_DEFAULT             "STMicroelectronics"
#define USBD_PRODUCT_STRING_DEFAULT                   "STM32 UAC 2.0"
#define USBD_SPK_STRING_DEFAULT                       "STM32 Speaker"
#define USBD_MIC_STRING_DEFAULT                       "STM32 Microphone"
#define USBD_HID_STRING_DEFAULT                       "STM32 debug HID"
#define USBD_CDC_VPC0_STRING_DEFAULT                  "STM32 VPC0"
#define USBD_CDC_VPC1_STRING_DEFAULT                  "STM32 VPC1"
#define USBD_MIDI_STRING_DEFAULT                      "STM32 Midi"
#define USBD_VIDEO_CTRL_STRING_DEFAULT                "STM32 Video Control"
#define USBD_VIDEO_STREAM_STRING_DEFAULT              "STM32 Video Stream"
#define USBD_CDC_TRACEALYZER_STRING_DEFAULT           "STM32 TraceAlyzer"





enum
{
  STRID_LANGID = 0,
  STRID_MANUFACTURER,
  STRID_PRODUCT,
  STRID_SERIAL,
};

// array of pointer to string descriptors
static char const *string_desc_arr[] =
{
  (char[]) { 0x09, 0x04 },  // 0: is supported language is English (0x0409)
  USBD_MANUFACTURER_STRING_DEFAULT,       // 1: Manufacturer
  USBD_PRODUCT_STRING_DEFAULT,            // 2: Product
  NULL,                                   // 3: Serials will use unique ID if possible
  USBD_SPK_STRING_DEFAULT,                // 4: Audio Interface
  USBD_MIC_STRING_DEFAULT,                // 5: Audio Interface
  USBD_HID_STRING_DEFAULT,                // 6: debug Interface
  USBD_CDC_VPC0_STRING_DEFAULT,           // 7: debug Interface
  USBD_MIDI_STRING_DEFAULT,               // 8: debug Interface
  USBD_VIDEO_CTRL_STRING_DEFAULT,         // 9: debug Interface
  USBD_VIDEO_STREAM_STRING_DEFAULT,       // 10: debug Interface
  USBD_CDC_VPC1_STRING_DEFAULT,           // 11: debug Interface
  USBD_CDC_TRACEALYZER_STRING_DEFAULT,    // 12: debug Interface


};

static uint16_t _desc_str[32 + 1];



int8_t  tusb_set_descriptor_string(int32_t index, const char *pString)
{
  int8_t  result = 0;
  if (index < sizeof(string_desc_arr) / sizeof(string_desc_arr[0]))
  {
    string_desc_arr[index] = pString;
    result = 1;
  }
  return result ;
}

static int8_t  endpointAllocator = 1;

/**
* @brief endpoint allocator
*
* @return the endpoint or 0 if error
*/

int8_t  tusb_allocate_ep(void)
{
  int8_t ep = endpointAllocator;
  if (ep > 9)
  {
    ep = 0;
    TU_ASSERT(0);
  }
  else
  {
    endpointAllocator++;
  }
  return ep;

}




uint8_t *tud_descriptor_get_configuration(void)
{
  return desc_configuration;
}


//--------------------------------------------------------------------+
// Device Descriptors
//--------------------------------------------------------------------+
tusb_desc_device_t desc_device =
{

  .bLength            = sizeof(tusb_desc_device_t),
  .bDescriptorType    = TUSB_DESC_DEVICE,
  .bcdUSB             = 0x0200,

  // Use Interface Association Descriptor (IAD) for Audio
  // As required by USB Specs IAD's subclass must be common class (2) and protocol must be IAD (1)
  .bDeviceClass       = TUSB_CLASS_CDC,
  .bDeviceSubClass    = CDC_COMM_SUBCLASS_ABSTRACT_CONTROL_MODEL,
  .bDeviceProtocol    = 0,
  .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

  .idVendor           = USBD_VID,
  .idProduct          = USB_PID_BASE,
  .bcdDevice          = CFG_USB_DCDDEVICE,

  .iManufacturer      = 0x01,
  .iProduct           = 0x02,
  .iSerialNumber      = 0x03,
  .bNumConfigurations = 0x01
};





tusb_desc_device_t *tud_descriptor_get_descriptor_device(void)
{
  return &desc_device;
}

/* Functions Definition ------------------------------------------------------*/



/**
* @brief Invoked when received GET DEVICE DESCRIPTOR
*
* @return uint8_t const* Application return pointer to descriptor
*/
uint8_t const *tud_descriptor_device_cb(void)
{
  return (uint8_t const *)&desc_device;
}


uint8_t tud_extra_class_add(void (*interface)(int32_t *numInterface), void (*descriptor)(uint8_t **pDescriptor))
{
  tExtraInterface[nbExtraClass] = interface;
  tExtraDescriptor[nbExtraClass] = descriptor;
  nbExtraClass++;
  TU_ASSERT(nbExtraClass <  MAX_EXTRA_INTERFACE);
  return 1;
}


/**
* @brief print the a memory block as an C array
*
* @param pVariable variable name
* @param pData memory block pointer
* @param size  memory block size
* @param maxLine  max num by line
*/
void tud_dump_array(const char *pVariable, void *pData, uint32_t size, int maxLine)
{
  static int cpt = 0;

  uint8_t *pData8 = (uint8_t *)pData;
  printf("%02d:%-50s: ",cpt++, pVariable);
  for (int a = 0; a < size; a++)
  {
    if (a != 0 && (a % maxLine) == 0)
    {
      printf("\n");
    }
    if (a != 0)
    {
      printf(",");
    }
    printf("0x%02x", pData8[a]);
  }
  printf("\n");
}



static inline uint8_t tud_get_code_sample_size(uint8_t szSple)
{
  uint8_t code = 0; // default is 16-bit
  switch (szSple)
  {
    case 4U:
      code = 1U;
      break;
    case 2U:
      code = 0U;
      break;
    default:
      break;
  }
  return code;
}


static inline uint8_t tud_get_code_freq(uint32_t freq)
{
  uint8_t code = 7; // unknown
  switch (freq)
  {
    case 8000:
      code = 0;
      break;
    case 16000:
      code = 1;
      break;
    case 32000:
      code = 2;
      break;
    case 48000:
      code = 3;
      break;
    case 96000:
      code = 4;
      break;
    case 44100:
      code = 5;
      break;
    default:
      break;
  }
  return code;
}

/*
This driver assumes that we can change audio features between several boots. Changing features involves changing the USB device descriptor.
Each time we change the device descriptor, we need to re-install the USB device. Ie: uninstall the USB device from the windows device manager and re-plug the USB device).
During this operation , the USB HOST keeps new information in its registry.  So, we cannot change the devices features without to re-install the device…. it is painful !
To work around this issue, we can change the application ID and create a PID different for each possible configuration.
From this tip, we can have a device with multiple static configuration that will dynamically change between different boots.
In the PID we encode some features like frequency, channels,class,etc...  exposed are changeable between each boot.

*/
static void tud_build_dynamic_pid(tusb_device_conf_t *pConf)
{
  desc_device.idProduct = pConf->pid;   /* default fixed PID is set if == 0 */
  if (desc_device.idProduct  == 0)
  {
    #if CFG_DYNAMIC_MULTI_FREQUENCY == 0
    uint32_t customPID = 0;
    if (pConf->uac2_audio.rec_enabled)
    {
      // from 15-11 (freq 3 bits + ch 2 bits )
      customPID |= PID_MAP(tud_get_code_sample_size(pConf->uac2_audio.rec_szSple), 17);
      customPID |= PID_MAP(tud_get_code_freq(pConf->uac2_audio.rec_freq), 14);
      customPID |= PID_MAP(pConf->uac2_audio.rec_ch, 12);
    }
    if (pConf->uac2_audio.play_enabled)
    {
      // from 12-8 (freq 3 bits + ch 2 bits )
      customPID |= PID_MAP(tud_get_code_sample_size(pConf->uac2_audio.play_szSple), 11);
      customPID |= PID_MAP(tud_get_code_freq(pConf->uac2_audio.play_freq), 8);
      customPID |= PID_MAP(pConf->uac2_audio.play_ch, 6);
    }
    customPID |= PID_MAP(CFG_TUD_MIDI, 5);

    #if defined(ST_USE_DEBUG)
    /* if we are in devel , dont change the driver for debug classes */
    //customPID |= PID_MAP(pConf->uac2_audio.feature_enabled, 4);
    //customPID |= PID_MAP(CFG_TUD_HID, 3);
    //customPID |= PID_MAP(CFG_TUD_CDC, 2);
    #else
    // for 0-5 MCU add a part of the id
    customPID |= (*((volatile uint32_t *) UID_BASE) & 0x1FU);
    #endif
    desc_device.idProduct = (uint16_t)customPID;
    #endif
  }
}





static void tusb_extra_add_interface(int32_t *numInterface)
{
  for (int32_t count  = 0; count  < nbExtraClass ; count  ++)
  {
    tExtraInterface[count ](numInterface);
  }
}


static void tusb_extra_descriptor(uint8_t **pDescriptor)
{

  for (int32_t count = 0; count < nbExtraClass  ; count ++)
  {
    tExtraDescriptor[count](pDescriptor);
  }

}





/* A descriptor has 4 layers  from the high level to the low level


    Device :       Describes properties relate to the entire device,
    this includes supported USB version, manufacture device class and
    most importantly number configuration. Only one descriptor


    Configuration:
    A configuration defines how it�s being powered and describes how
    Many interface it has, and there can be multiple configurations.
    For example, you can have 2 options ,
    First option has 2 configuration and the second one has single config
    (multiple Configuration)

    Interface:
    A interface will have a class that will define what part of the USB spec
    it is and will be correlated to an endpoint and specific configuration for the class
    For example, if a device supports the MIDI and the Music PCM,
    the descriptor will have 2 interfaces, one for the MIDI and one for the Music PCM
    (multiple interface)

    Endpoint:
    A endpoint actual describes the way the data will be sent over.
    So, it describes the direction, packet size and type of USB transfer
    For example, a MIDI interface has 2 endpoints  TX and RX with 2 different addresses
    and Types ( IN interrupt and out Bulk)
    packet size is 8 bytes
    (multiple Endpoint)

    Builds a dynamic descriptor in ram. if the description is invariable between boots, it is possible to generate the binary to place
    it in flash using the define CFG_SHOW_DESCRIPTOR_STATIC
    The descriptor will be generated in the serial terminal and will replace the variable desc_configuration as const.

    the construction is no standard for tinyusb, but this method is easier for because most of descriptor issue comes from a wrong length calculation,
    using this dynamic method, the description length is computed automatically using macrosTUD_AUDIO_DESC_START_LEN/TUD_AUDIO_DESC _STOP_LEN_16
    and we can insert new description freely without to pay attention to the length coherency.
    macro used generate the same payload but dynamically in ram thanks to a dynamic  macro derived from the standard tinyusb macros

*/

uint8_t *tud_build_dynamic_descriptor(void)
{
  DESC_LEN hDeviceLen;

  tud_build_dynamic_pid(&tusb_handle()->hConfig);

  /* for len computation */
  /*  Select the interface index according to the API config  */
  int32_t numInterface = 0;
  /* add extra interface */
  tusb_extra_add_interface(&numInterface);

  /* start de construction */
  uint8_t *pDescriptor = desc_configuration;
  /* Start Len Computation (_totallen)*/
  DESC_START_LEN(hDeviceLen, pDescriptor);
  TUD_CONFIG_DESCRIPTOR_D(pDescriptor, 1, /*ITF_NUM_TOTAL*/numInterface, 0, -1 /*_total_len */, CFG_CONFIG_BMATTRIBUTES, CFG_USB_CONFIG_MAXPOWER);
  /* add extra descriptors */
  tusb_extra_descriptor(&pDescriptor);
  /* adjust the device configuration len */
  DESC_STOP_LEN_16(hDeviceLen, pDescriptor, 2, 0);
  TU_ASSERT(sizeof(desc_configuration) >= (pDescriptor - desc_configuration));
  #ifdef CFG_SHOW_DESCRIPTOR_STATIC
  tud_dump_array("desc_configuration", desc_configuration, pDescriptor - desc_configuration, 16);
  #endif
  return desc_configuration;

}
/**
* @brief Invoked when received GET CONFIGURATION DESCRIPTOR
Application return pointer to descriptor
Descriptor contents must exist long enough for transfer to complete
*
* @param index
* @return uint8_t const*
*/

uint8_t const *tud_descriptor_configuration_cb(uint8_t index)
{
  (void)index; // for multiple configurations
  return desc_configuration;
}

/**
* @brief Get USB Serial number string from unique ID if available. Return number of character.
*
* @param uid CPU ID
* @param uid_len id  len
* @param desc_str1  string
* @return size_t len generated
*/

static inline int32_t generate_serial(uint8_t *uid, int32_t uid_len, uint16_t desc_str1[])
{
  uint16_t *pSerial = desc_str1;
  for (int a = 0; a < uid_len ; a++)
  {
    *pSerial++   = (uid[a] >> 8U)  <= 9 ? '0' + (uid[a] >> 8U) : 'A' + (uid[a] >> 8U) - 10;
    *pSerial++   = (uid[a] & 0xFU) <= 9 ? '0' + (uid[a]  & 0xFU) : 'A' + (uid[a]  & 0xFU) - 10;
  }
  return (2 * uid_len);

}
/**
* @brief  Invoked when received GET STRING DESCRIPTOR request
*  Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
*
* @param index index string
* @param langid  lang id
* @return uint16_t const*
*/

uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
  (void) langid;
  int32_t  chr_count;

  switch (index)
  {
    case STRID_LANGID:
      memcpy(&_desc_str[1], string_desc_arr[0], 2);
      chr_count = 1;
      break;

    case STRID_SERIAL:
    {
      /* uses the stm32 MCU serial id as serail USB */

      uint32_t id32[3];
      #if TUD_USE_UNIQUE_SERIAL_ID
      volatile uint32_t *stm32_uuid = (volatile uint32_t *) UID_BASE;
      id32[0] = stm32_uuid[0];
      id32[1] = stm32_uuid[1];
      id32[2] = stm32_uuid[2];
      #else
      for (uint32_t gen = 0 ; gen < 12 ; gen++)
      {
        ((uint8_t *)&id32)[gen] = gen % 16U;

      }
      #endif
      chr_count = generate_serial((uint8_t *)id32, 12, _desc_str + 1);
    }
    break;

    default:
      // Note: the 0xEE index string is a Microsoft OS 1.0 Descriptors.
      // https://docs.microsoft.com/en-us/windows-hardware/drivers/usbcon/microsoft-defined-usb-descriptors
      if (!((uint32_t)index < (sizeof(string_desc_arr) / sizeof(string_desc_arr[0]))))
      {
        return NULL;
      }

      const char *str = string_desc_arr[index];

      // Cap at max char
      chr_count = strlen(str);
      size_t const max_count = sizeof(_desc_str) / sizeof(_desc_str[0]) - 1; // -1 for string type
      if (chr_count > max_count) { chr_count = max_count; }

      // Convert ASCII string into UTF-16
      for (size_t i = 0; i < chr_count; i++)
      {
        _desc_str[1 + i] = str[i];
      }
      break;
  }

  // first byte is length (including header), second byte is string type
  _desc_str[0] = (uint16_t)((TUSB_DESC_STRING << 8) | (2 * chr_count + 2));

  return _desc_str;
}

/*cstat +MISRAC2012* */

