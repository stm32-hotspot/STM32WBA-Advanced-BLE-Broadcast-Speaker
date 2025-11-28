---
pagetitle: Release Notes for stm32_audio Component
lang: en
header-includes: <link rel="icon" type="image/x-icon" href="./scripts/_htmresc/favicon.png" />
---

::: {.row}
::: {.col-sm-12 .col-lg-4}

<center>
# Release Notes for stm32_audio Component

Copyright &copy;2024 STMicroelectronics

All rights reserved
    
[![ST logo](./scripts/_htmresc/st_logo_2020.png)](https://www.st.com){.logo}
</center>

# License

The component license is described in : [SLA0044_LICENSE.md](SLA0044_LICENSE.md) or [SLA0044_LICENSE.txt](SLA0044_LICENSE.txt)

\
You may not use this component except in compliance with these Licenses.


# Purpose

Offers a unique API to register any Audio BSP or other code in order to start audio capture and audio player in a unique way on any STM32. 
It requires implementation of one file that will connect the end API to stm32_audio API.


:::
::: {.col-sm-12 .col-lg-8}
# Update history

::: {.collapse}
<input type="checkbox" id="collapse-section33" checked aria-hidden="true">
<label for="collapse-section33"  aria-hidden="true">2.7.2 April 02, 2025</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   Update SAIPDM driver to allow usage of HAL_SAI_Rx* routines for both PCM & PDM capture.
 
   Bug fixes
   ------------------------
   None


## Known limitations

None

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section32"  aria-hidden="true">
<label for="collapse-section32"  aria-hidden="true">2.7.1 March 06, 2025</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   update DMA audio management
   USBX audio rework
 
   Bug fixes
   ------------------------
   None


## Known limitations

None

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section31"  aria-hidden="true">
<label for="collapse-section31"  aria-hidden="true">2.7.0 March 03, 2025</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   Add MDF driver for microphone capture; aka stm32_audio_mdf.c
   Add SAI_PDM driver for microphone capture; aka stm32_audio_sai_pdm.c
   Add dma driver for audio H5xx, H7xx, H7RSxx; aka stm32_audio_dma_xxx.c
   DFSDM driver uses new dma driver
   Add usbx wrapper 

   Bug fixes
   ------------------------
   None


## Known limitations

None

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section30"  aria-hidden="true">
<label for="collapse-section30"  aria-hidden="true">2.6.3 February 10, 2025</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   Remove useless dependancy and include
   Add ThreadX support

   Bug fixes
   ------------------------
   None


## Known limitations

None

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section29"  aria-hidden="true">
<label for="collapse-section29"  aria-hidden="true">2.6.2 January 16, 2025</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------

   Bug fixes
   ------------------------
   Fix a bug in DFSDM driver when using the High Pass Filter.


## Known limitations

None

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section28"  aria-hidden="true">
<label for="collapse-section28"  aria-hidden="true">2.6.1 December 3, 2024</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------

   Bug fixes
   ------------------------
   disable/enable irq with counter of nested commands to avoid re-enabling irq too early


## Known limitations

None

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section27"  aria-hidden="true">
<label for="collapse-section27"  aria-hidden="true">2.6.0 November 26, 2024</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   Add new API UTIL_AUDIO_init that implements IT masking to avoid init failures when higher prio services are started before Audio
   Add a check that configTICK_RATE_HZ value is OK in case of low latency mode (when UTIL_AUDIO_N_MS_DIV != 1)

   Bug fixes
   ------------------------
   MISRAC


## Known limitations

None

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section26"  aria-hidden="true">
<label for="collapse-section26"  aria-hidden="true">2.5.1 November 22, 2024</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   check that configTICK_RATE_HZ value is OK

   Bug fixes
   ------------------------
   None


## Known limitations

None

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section25"  aria-hidden="true">
<label for="collapse-section25"  aria-hidden="true">2.5.0 November 13, 2024</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   Add support of 32-bit
   Add support of DMA IT < 1ms for low latency
   Add possibility to remove PDM2PCM lib if not needed

   Bug fixes
   ------------------------
   None


## Known limitations

None

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section24"  aria-hidden="true">
<label for="collapse-section24"  aria-hidden="true">2.4.0 September 26, 2024</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   Add support of 96kHz 
   
   Bug fixes
   ------------------------
   None


## Known limitations

None

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section23"  aria-hidden="true">
<label for="collapse-section23"  aria-hidden="true">2.3.2 August 22, 2024</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   minor: MISRAC
   minor: remove misleading comments 

   Bug fixes
   ------------------------
   None


## Known limitations

None

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section22"  aria-hidden="true">
<label for="collapse-section22"  aria-hidden="true">2.3.1 June 6, 2024</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   Support of LineIn added
   Add new api to set the rendering device id

   Bug fixes
   ------------------------
   None


## Known limitations

None

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section21"  aria-hidden="true">
<label for="collapse-section21"  aria-hidden="true">2.3.0 May 28, 2024</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   For audio capture, introduced separated configuration of the DMA buffer (can be 32bit non interleaved with DFSDM or other format with other IPs, also can be mono stereo) and the final buffer (can be stereo even if DMA is mono, can be PCM interleaved even if DMA is not) 
   Remove parameter argument UTIL_AUDIO_params_t from UTIL_AUDIO_RENDER_init(); UTIL_AUDIO_CAPTURE_init();
   Removed mono2Stereo parameter, now deduced from UTIL_AUDIO_IN_HW_CH_NB & UTIL_AUDIO_IN_CH_NB
   Support of MDF added

   Bug fixes
   ------------------------
   None


## Known limitations

None

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section20"  aria-hidden="true">
<label for="collapse-section20"  aria-hidden="true">2.2.4 March 12, 2024</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   Update to align to new API of PDM2PCM library & tiny USB

   Bug fixes
   ------------------------
   None


## Known limitations

None

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section19"  aria-hidden="true">
<label for="collapse-section19"  aria-hidden="true">2.2.3 February 22, 2024</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   Warnings removal
   
   Bug fixes
   ------------------------
   None


## Known limitations

None

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section18"  aria-hidden="true">
<label for="collapse-section18"  aria-hidden="true">2.2.2 January 31, 2024</label>
<div>


## Main changes

removed legacy Utilities/cmsisos replaced by Utilities/STOs

## Known limitations


   Limitations per feature
   ----------------------------------------------------------
   Reconfiguration API: Tested only for reconfiguration at reset. Reconfiguration @execution is not supported
                  

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section17"  aria-hidden="true">
<label for="collapse-section17"  aria-hidden="true">2.2.1 January 30, 2024</label>
<div>


## Main changes

remove USBX partial support, USB support through TinyUsb

## Known limitations


   Limitations per feature
   ----------------------------------------------------------
   Reconfiguration API: Tested only for reconfiguration at reset. Reconfiguration @execution is not supported
            

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section16"  aria-hidden="true">
<label for="collapse-section16"  aria-hidden="true">2.2.0 January 29, 2024</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   Added a dfsdm driver to ensure that multi microphones are always started in sync whatever the board.
         

## Known limitations


   Limitations per feature
   ----------------------------------------------------------
   USBX support: integration is partial, no synchro available yet
   Reconfiguration API: Tested only for reconfiguration at reset. Reconfiguration @execution is not supported
   

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section15"  aria-hidden="true">
<label for="collapse-section15"  aria-hidden="true">2.1.0 January 18, 2024</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   Tiny Usb wrapper
   New API to start & stop microphone & rendering at the same time 
   API is now doxygen formatted
         

## Known limitations


   Limitations per feature
   ----------------------------------------------------------
   USBX support: integration is partial, no synchro available yet
   Reconfiguration API: Tested only for reconfiguration at reset. Reconfiguration @execution is not supported
            

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section14"  aria-hidden="true">
<label for="collapse-section14"  aria-hidden="true">2.0.0 October 30, 2023</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   Add dump of PDM as debug feature
   New API to chose if microphone data should be sent as PDM or PCM when available
   __weak issues solving
   usbx first integration (limited because no synchro is available)
   add audio interrupts cycles measure
   New file stm32_audio_usb_legacy.c replaces  stm32_audio_usb_in.c &  stm32_audio_usb_out.c 
   Add isMono2Stereo for case when single microphone is started but stereo audio stream is needed 
   API change to allow reconfig of mic selection, nb channels, sampling frequency from user params. Reconfig is done at reset/reboot, not dynamically t execution. Add UTIL_AUDIO_USB_DISABLE_RESAMPLING to remove the resampling if not needed. If resampling is needed, it is called only when stream is used (for instance USB is not calling SRC236 if not stream to/from PC/host)
   modified usb rec routine to allow more flexibility to performa latency checks
   add clear buffer API for all buffer not only RENDER
   task creation done using new utility called stm32_cmsisos.ch (threadx support tested )
   more accurate error management (OK, NOK, warning, error, noError)


## Known limitations


   Limitations per feature
   ----------------------------------------------------------
   USBX support: integration is partial, no synchro available yet
   Reconfiguration API: Tested only for reconfiguration at reset. Reconfiguration @execution is not supported
   

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section13"  aria-hidden="true">
<label for="collapse-section13"  aria-hidden="true">1.9.1 January 25, 2023</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   Update release note
   


## Known limitations

NA

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section12"  aria-hidden="true">
<label for="collapse-section12"  aria-hidden="true">1.9.0 January 24, 2023</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   OS task creation are handled thrue a new wrapper called wrapper_cmsisos
   


## Known limitations

NA

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section11"  aria-hidden="true">
<label for="collapse-section11"  aria-hidden="true">1.8.0 January 11, 2023</label>
<div>


## Main changes


  Additional features
  ----------------------------------------------------------
  Add a test and log traces if Message queue for AudioIn is full
  Cleaned API to remove old legacy features no longer used and added function to check if UTIL_AUDIO_CAPTURE & UTIL_AUDIO_RENDER are being used as well as a new function to clear the rendering buffer.
  added support of CMSISOS2
  API modification to allow support for standard H5 audio BSP
  Add MDF support for U5
        

## Known limitations

NA

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section10"  aria-hidden="true">
<label for="collapse-section10"  aria-hidden="true">1.7.0 September 23, 2022</label>
<div>


## Main changes


  Additional features
  ----------------------------------------------------------
  remove all BSP pseudo deps
  1 sec is enough for dump and allows compile for all conf, 48kHz wouldn't compile in HomeAuto with 3 sec dump
        

## Known limitations

NA

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section9"  aria-hidden="true">
<label for="collapse-section9"  aria-hidden="true">1.6.0 July 29, 2022</label>
<div>


## Main changes


  Additional features
  ----------------------------------------------------------
  C-code formatting ==> aligning with official templates & MISRA check
  Added support of 8kHz from SAI_PDM + PDM2PCM library
  Add a missing memory check after malloc
  Minor API renaming for USB routine to make function clearer (In & Out is always fuzzy)

  Bug fixes
  ------------------------
  Fixed a bug of cache maintenance of DMA microphone buffer (bug present only when cache was used which is not the default behavior of the demos)
        

## Known limitations

NA

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section8"  aria-hidden="true">
<label for="collapse-section8"  aria-hidden="true">1.5.5 June 21, 2022</label>
<div>


## Main changes

No functional modification, remaining c code formatting & MISRA check\n

## Known limitations

NA

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section7"  aria-hidden="true">
<label for="collapse-section7"  aria-hidden="true">1.5.4 March 08, 2022</label>
<div>


## Main changes

No functional modification, only c code formatting ==> aligning with official templates & MISRA check\n

## Known limitations

NA

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section6"  aria-hidden="true">
<label for="collapse-section6"  aria-hidden="true">1.5.2 November 23, 2021</label>
<div>


## Main changes


  Additional features
  ----------------------------------------------------------
  minor update to align with traces API update
  

  Bug fixes                
  ------------------------ 
  correction for AudioBuffer pData allocated
  



## Known limitations

NA

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section5"  aria-hidden="true">
<label for="collapse-section5"  aria-hidden="true">__v1.4.0__ February 22, 2021</label>
<div>


## Main changes

HPF used if define UTIL_AUDIO_USE_HPF (when AFE is off)\n added SAIPDM usecase for H735\n

## Known limitations

NA

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section4"  aria-hidden="true">
<label for="collapse-section4"  aria-hidden="true">__v1.3.0__ February 22, 2021</label>
<div>


## Main changes

update header + remove CUSTOM_BOARDS defines

## Known limitations

NA

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section3"  aria-hidden="true">
<label for="collapse-section3"  aria-hidden="true">__v1.2.0__ February 10, 2021</label>
<div>


## Main changes

audioChain API change alignement + add error checks\n minor change to support H747 custom BSP for Audio connector

## Known limitations

NA

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section2"  aria-hidden="true">
<label for="collapse-section2"  aria-hidden="true">__v1.1.0__ December 16, 2020</label>
<div>


## Main changes

minor change to support H747 custom BSP for Audio connector

## Known limitations

NA

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section1"  aria-hidden="true">
<label for="collapse-section1"  aria-hidden="true">__v1.0.0__ April 4, 2020</label>
<div>


## Main changes

first version. Tested with 3 different BSP, H747DK, H735DK, H743STVS

## Known limitations

NA

</div>
:::


:::
:::

<footer class="sticky">
::: {.columns}
::: {.column width="95%"}
For complete documentation on STM32 Microcontrollers , visit: http://www.st.com/STM32
:::
::: {.column width="5%"}
<abbr title="Based on template cx566953 version 2.0">Info</abbr>
:::
:::
</footer>        
