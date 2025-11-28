/**
******************************************************************************
* @file    tusb_midi_in.c
* @author  MCD Application Team
* @brief   Manage a midi interface in for keyboard or sequencer
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
#include "tusb_config.h"


/*
  Add the following flags to the project
  CFG_TUD_MIDI_IN=1

*/




#if CFG_TUD_MIDI_IN

#include "tinyusb.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
/*cstat -MISRAC2012-* tinyusb not misra compliant */
#include "tusb.h"
#include "usb_descriptors.h"





/* private defines -----------------------------------------------------------*/

#define EP_MIDI_IN                       EP_IN(EP_NUM_MIDI)
#define EP_MIDI_OUT                      EP_OUT(EP_NUM_MIDI)


/* Private typedef -----------------------------------------------------------*/
typedef struct  tusb_midi_listener_client
{
  tusb_midi_listener_cb cb;
  void *                pCookie;
}tusb_midi_listener_client;


/* Private variables ---------------------------------------------------------*/

static uint32_t interfacemidi;
static tusb_midi_listener_client tListeners[4];


/* Private function prototypes -----------------------------------------------*/


/* Functions Definition ------------------------------------------------------*/



/**
 * @brief Send  midi events to a port
 *
 * @param port  midi port 
 * @param pEvents  midi events pointer buffer
 * @param szEvents  midi events pointer buffer
 * @return result
 */
uint8_t  usb_midi_send_events(uint8_t port,uint8_t const  *pEvents,uint32_t szEvents)
{
  return tud_midi_stream_write(port,pEvents,szEvents);
}



/**
 * @brief unregister a midi listener 
 *
 * @param cb midi listener  callback 
 * @return true or false
 */

uint8_t usb_midi_listener_register(tusb_midi_listener_cb cb,void *pCookie)
{
  uint8_t result = 0;
  uint32_t count = sizeof(tListeners)/sizeof(tListeners[0]);
  for(int32_t index=0 ; index < count  ; index++)
  {
    if(tListeners[index].cb == NULL)
    {
      tListeners[index].cb = cb;
      tListeners[index].pCookie = pCookie;
      result = 1;
      break;
    }
  }
  return  result;
}


/**
 * @brief register a midi listener 
 *
 * @param cb midi listener  callback 
 * @return true or false
 */
uint8_t usb_midi_listener_unregister(tusb_midi_listener_cb cb)
{
  uint8_t result = 0;
  uint32_t count = sizeof(tListeners)/sizeof(tListeners[0]);
  for(int32_t index=0 ; index < count  ; index++)
  {
    if(tListeners[index].cb == cb)
    {
      tListeners[index].cb = NULL;
      result = 1;
      break;
    }
  }
  return  result;
}


/**
 * @brief called from the descriptor construction allow to build an interface index
 *
 * @param numInterface  the ref num interface
 */
static void tusb_midi_in_add_interface(int32_t *numInterface)
{
  interfacemidi = *numInterface;
  (*numInterface)++;
  /* MIDI has 2 interfaces (MIDI and STREAMING) , TUD_MIDI_DESCRIPTOR assumes that STREAMING = MIDI+1, IE 2 interfaces */
  (*numInterface)++;
}


/**
 * @brief called from the descriptor construction allow add descriptors to the main instance
 *
 * @param pDescriptor  descriptor instance under construction pointer
 */


static void tusb_midi_in_descriptor(uint8_t **pDescriptor)
{
  int8_t ep  = tusb_allocate_ep();
  // Interface number, string index, EP Out & EP In address, EP size
  TUD_MIDI_DESCRIPTOR_IN_D(*pDescriptor,interfacemidi, TUD_MIDI_STRING_INDEX /* string id */ , EP_OUT(ep), EP_IN(ep), TUD_OPT_HIGH_SPEED==0? 64:512);
  
}


void tusb_midi_in_init(void)
{
  if(tusb_handle()->hConfig.mount & CFG_MIDI_IN_ENABLED)
  {
    tud_extra_class_add(tusb_midi_in_add_interface, tusb_midi_in_descriptor);
  }
}



void tud_midi_rx_cb(uint8_t itf)
{
  uint8_t tBuffer[64];
  int nb = tud_midi_available();
  uint32_t nbRead = tud_midi_stream_read(tBuffer,nb );
  for(uint32_t index=0; index < sizeof(tListeners)/sizeof(tListeners[0]); index++)
  {
    if(tListeners[index].cb)
    {
      /* use port 0 only for now */
      tListeners[index].cb(0,tBuffer,nbRead,tListeners[index].pCookie);
    }
  }
}


#endif
