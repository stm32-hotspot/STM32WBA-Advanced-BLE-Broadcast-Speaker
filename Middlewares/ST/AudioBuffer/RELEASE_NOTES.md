---
pagetitle: Release Notes for AudioBuffer Component
lang: en
header-includes: <link rel="icon" type="image/x-icon" href="./scripts/_htmresc/favicon.png" />
---

::: {.row}
::: {.col-sm-12 .col-lg-4}

<center>
# Release Notes for AudioBuffer Component

Copyright &copy;2024 STMicroelectronics

All rights reserved
    
[![ST logo](./scripts/_htmresc/st_logo_2020.png)](https://www.st.com){.logo}
</center>

# License

The component license is described in : [SLA0044_LICENSE.md](SLA0044_LICENSE.md) or [SLA0044_LICENSE.txt](SLA0044_LICENSE.txt)

\
You may not use this component except in compliance with these Licenses.


# Purpose

AudioBuffer offers some services to manage basic buffering of audio.

:::
::: {.col-sm-12 .col-lg-8}
# Update history

::: {.collapse}
<input type="checkbox" id="collapse-section14" checked aria-hidden="true">
<label for="collapse-section14"  aria-hidden="true">1.5.2 March 05, 2025</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   update mem pools names to simplify AudioMalloc pools management
   
   Bug fixes
   ------------------------
   None



## Known limitations



</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section13"  aria-hidden="true">
<label for="collapse-section13"  aria-hidden="true">1.5.1 February 10, 2025</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   Remove need to have livetuneconf.h used as a pre include
   Remove usage of define STM32xx to identify the family for a better integration in Cubemx
   
   Bug fixes
   ------------------------
   None



## Known limitations



</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section12"  aria-hidden="true">
<label for="collapse-section12"  aria-hidden="true">1.5.0 January 16, 2025</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   API: rename audio memory pools
   API: add AUDIO_MEM_NOCACHED_ENABLED conf to avoid useless memory consumption if AUDIO_MEM_NOCACHED is not used
   reorder memPool_t enum to consume less memory in algos' contexts
   
   Bug fixes
   ------------------------
   None



## Known limitations



</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section11"  aria-hidden="true">
<label for="collapse-section11"  aria-hidden="true">1.4.3 December 3, 2024</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------

   Bug fixes
   ------------------------
   disable/enable irq with counter of nested commands to avoid re-enabling irq too early



## Known limitations



</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section10"  aria-hidden="true">
<label for="collapse-section10"  aria-hidden="true">1.4.2 November 28, 2024</label>
<div>


## Main changes

   
   Additional features
   ---------------------------------------------------------
   improve memory pool allocation strategy
   add capability to request 32 bits data from captured mic



## Known limitations



</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section9"  aria-hidden="true">
<label for="collapse-section9"  aria-hidden="true">1.4.1 October 11, 2024</label>
<div>


## Main changes

   
   Additional features
   ---------------------------------------------------------
   use new st_os_mem_realloc API



## Known limitations



</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section8"  aria-hidden="true">
<label for="collapse-section8"  aria-hidden="true">1.4.0 October 2, 2024</label>
<div>


## Main changes

   
   Additional features
   ---------------------------------------------------------
   rework audio mallocs according to available st_os_meme memory pools (depending of platform)
   96 kHz



## Known limitations



</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section7"  aria-hidden="true">
<label for="collapse-section7"  aria-hidden="true">1.3.3 August 21, 2024</label>
<div>


## Main changes

   
   Additional features
   ---------------------------------------------------------
   Harmonized configuration of audio_mem_mgnt.ch (introduced audio_mem_mgnt_conf.h and renamed some define)
   If AUDIO_MEM_CONF_STOS_USED is not defined, all AudioMalloc are allocated in AUDIO_MEM_HEAP_FAST, i.e. standard heap



## Known limitations



</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section6"  aria-hidden="true">
<label for="collapse-section6"  aria-hidden="true">1.3.2 June 19, 2024</label>
<div>


## Main changes

implementation of AudioMallocCheckPtr routine

## Known limitations



</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section5"  aria-hidden="true">
<label for="collapse-section5"  aria-hidden="true">1.3.1 June 19, 2024</label>
<div>


## Main changes

Malloc statistics improvements (no more weak routine)

## Known limitations



</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section4"  aria-hidden="true">
<label for="collapse-section4"  aria-hidden="true">1.3.0 June 14, 2024</label>
<div>


## Main changes

Added detailled memory usage statistics capability

## Known limitations



</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section3"  aria-hidden="true">
<label for="collapse-section3"  aria-hidden="true">1.2.0 May 28, 2024</label>
<div>


## Main changes

Introduced FS_CUSTOM to support other sampling frequencies than the ones in the audio_fs_t structure.

## Known limitations



</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section2"  aria-hidden="true">
<label for="collapse-section2"  aria-hidden="true">1.1.0 March 12, 2024</label>
<div>


## Main changes

Added support of PDM formats (MSB, LSB, interleaved or not) and support of G711 fomats (ulaw, alaw)

## Known limitations



</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section1"  aria-hidden="true">
<label for="collapse-section1"  aria-hidden="true">1.0.0 January 20, 2024</label>
<div>


## Main changes

This is a first version. This code was extracted from Audio-kit v1.1.0 because it is needed by some application that won't benefit from the Audio-kit complexity

## Known limitations



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
