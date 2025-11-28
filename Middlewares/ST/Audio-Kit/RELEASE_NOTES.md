---
pagetitle: Release Notes for Audio-Kit Component
lang: en
header-includes: <link rel="icon" type="image/x-icon" href="./scripts/_htmresc/favicon.png" />
---

::: {.row}
::: {.col-sm-12 .col-lg-4}

<center>
# Release Notes for Audio-Kit Component

Copyright &copy;2024 STMicroelectronics

All rights reserved
    
[![ST logo](./scripts/_htmresc/st_logo_2020.png)](https://www.st.com){.logo}
</center>

# License

The component license is described in : [SLA0044_LICENSE.md](SLA0044_LICENSE.md) or [SLA0044_LICENSE.txt](SLA0044_LICENSE.txt) or [SLA0044_LICENSE.md](SLA0044_LICENSE.md) or [SLA0044_LICENSE.txt](SLA0044_LICENSE.txt)

\
You may not use this component except in compliance with these Licenses.


# Purpose

Audio-kit offers capability to chain algorithms that will process & deliver audio data one to the other.
It is based on a tool called LiveTune that offers a canvas for designing audio data flows. Livetune communicates with the STM32 firmware called AudioChain.

AudioChain is a multipath audio processing framework. It offers a generic way of building simple to complex audio
flows made of different processing blocks.\

AudioChain receives its input data from audio buffers and provides its results to output buffers. As a result, it is
independent from hardware and BSP source code. It can work on many different types of audio data such as:\
• Temporal or spectral domain\
• Fixed or floating-point\
• Interleaved or not\
• Mono, stereo, or a wider range of channels\

AudioChain supports a wide variety of audio-handling components such as routing blocks, gains, audio analysis
blocks, audio processing, and voice audio denoising.
These elements are provided as plugins called “algorithms” that the user can add and remove to create a
complete data flow. It is also possible to encapsulate external or third-party processing blocks to insert them in an
AudioChain data flow.\
Connecting algorithms together is done through multiframe buffers called “chunks”. The chunks used to interface
with external audio hardware management (such as microphones, USB, audio codec, or others) have some
specificities. These specific chunks are called system chunks. More details are given in a dedicated section of the
reference manual, called AudioChain connection to hardware.\

Note: The data flow can be implemented by using LiveTune and also by writing C code manually.


:::
::: {.col-sm-12 .col-lg-8}
# Update history

::: {.collapse}
<input type="checkbox" id="collapse-section12" checked aria-hidden="true">
<label for="collapse-section12"  aria-hidden="true">1.6.3 March 11, 2025</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   
   
   Bugs fixed  
   ----------------------------------------------------------
   update STM32H735 STM32Cube linker file (heap size)


## Known limitations



</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section11"  aria-hidden="true">
<label for="collapse-section11"  aria-hidden="true">1.6.2 March 11, 2025</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   Add memory pool for chunks and algos' handler & config
   Save chunk memory pool & algo memory pool into st registry (flash storage)
   Documentation update
   Add support of STM32H7S78-DK
   
   
   Bugs fixed  
   ----------------------------------------------------------
   AcousticBf memory leaks bug correction
   st_message_param construction bug correction for gcc build


## Known limitations



</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section10"  aria-hidden="true">
<label for="collapse-section10"  aria-hidden="true">1.6.1 February 10, 2025</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   Add ThreadX support 
   Updates to allow ease of integration inside CubeMx 
   Update N6 Project.ld to be compatible with CubeMx generated code
   Remove the need of commonAudioConf.h & generatedCodeConf.h 
   Rename ST_DESIGNER -> USE_LIVETUNE_DESIGNER
   Remove livetuneconf.h used as a pre include; now a standard include, all conf of diverse components distributed to the comps's conf file; for instance UART stuff are in stm32_uart_conf.h
   
   Bugs fixed  
   ----------------------------------------------------------
   Spectrum bug correction: fftLength was wrong following added ramType parameter
   Bug correction for Release_generated_code_ThreadX (TX_EXECUTION_PROFILE_ENABLE mustn't be defined at all)
   Audio conf bug correction when coming from generated_code (removed weak pragma in acSDK.hheader for acGetPersistConfigIndexFromString function)



## Known limitations



</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section9"  aria-hidden="true">
<label for="collapse-section9"  aria-hidden="true">1.6.0 January 16, 2025</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   Solve error trace issue in case of algo not linked into binary
   Minor algos renaming to follow the same naming rules in source code for all algos
   Add _ac_algo_list.c files that replace ALGO_FORCE_DECLARE bceause too compilcate to use it in clearVui
   Add AUDIO_MEM_NOCACHED_ENABLED conf to avoid useless memory consumption if AUDIO_MEM_NOCACHED is not used
   Add ramType static parameter for most algos: memory pool used for allocation
   New API in audioAlgo.h AudioAlgo_setCycleMgntCb
   Update default ramType for nlms and sam
   Remove deps of acSDK on audio_persist => clearVui is not using audio persist
   Add SAM for clearVUI livetune mode
   Rename audio memory pools
   Introduce audio_chain_conf.h to tune audio chain supported feature to allow memory reduction; first example applied on FIR as function of fs (see defines AC_SUPPORT_FS_* & AC_SUPPORT_RATIO_*)
   Do not display a trace error in AudioAlgo_getFactory if pointer is NULL
   Do not display a trace error in AudioAlgo_getCtrlData if pointer is NULL

   Bugs fixed  
   ----------------------------------------------------------
   bug correction in case of static parameter update while graph is running for an algo which has both dataInOut & process call-backs
   AcousticBf: fix Max distance & add consistency check, plus possible crash fixed
   SpeexAec: fixed remaining noise due to arm_cos lack of precision.



## Known limitations



</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section8"  aria-hidden="true">
<label for="collapse-section8"  aria-hidden="true">1.5.0 November 29, 2024</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   Introduce low-latency ac audio configuration as well as support of audio DMA IT < 1ms
   Introduce support of 32-bit audio data from all system inputs & outputs 
   First version of CIC filtering for alternative PDM2PCM conversion (very close to DFSDM processing)
   First version of graphical equalizer
   Support of frames size < 1ms thanks to the define AC_N_MS_DIV;  1/n ms (n = 2, 4, 8)
   Update of the Mix & linear mix algorithms: possibility to modify number of inputs up to 10 with a simple #define
   Update of FIR-equalizer to support smooth dynamic config change (use previous config for filtering instead of outputting silence during FIR coefficients re-computation)
   Improved Livetune connect duration using FAST memory instead of SLOW for json connection string (small impact on FAST memory consumption because most of these json strings are freed after connection)
   Improved memory pool choice strategy for audio mallocs
   Removed __disable_irq & __enable_irq from libraries (now a callback in user space)
   Direct call of __disable_irq replaced by a wrapped version of __disable_irq to count and avoid nested calls.


   Bugs fixed  
   ----------------------------------------------------------
   Fixed bug that would occur when the user would replace the algorithm description with spaces or with the same name as another algorithm



## Known limitations



</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section7"  aria-hidden="true">
<label for="collapse-section7"  aria-hidden="true">1.4.0 October 11, 2024</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   Add support of low-latency
   constantSource algorithm: add complex data support (for spectral samples)
   remove NR algorithm control callback
   use new st_os_mem_realloc API
   added sound-font flasher for N6

   Bugs fixed
   ----------------------------------------------------------
   constantSource algorithm: bug correction in case of non-interleaved samples

        

## Known limitations



</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section6"  aria-hidden="true">
<label for="collapse-section6"  aria-hidden="true">1.3.2 September 26, 2024</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   Add support of 96kHz
   Memory usage rework: terminal uses preferably slow memory for reallocation in case input pointer is NULL
   NR algorithm now has a control callback to get internal measure (Noise mean value, Speech & Noise attenuation information, etc... )
   Add mdrc capability to support variable number of compressor sub-band (between 1 and 5): MDRC5B_SUBBAND_MAX defined in mdrc_config.h
   ClearVui ASR wrapper update
   ClearVui Freertos config fixed when traceAlyzer is used
   Add the algorithm called SAM = farfield front end in front of ASR
   Add a memory footprint monitoring brief and detailed commands in CLI; commands are mem & mem2

   Bugs fixed
   ----------------------------------------------------------
   Collection: Fix a issue when the list is empty

        

## Known limitations



</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section5"  aria-hidden="true">
<label for="collapse-section5"  aria-hidden="true">1.3.1 July 15, 2024</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   More compact 'generated code'
   Audio HW config included in 'generated code' to insure consistent audio HW initialization with this 'generated code'
   FIR-equalizer algo reinit (config change while audio graph is running): split reinit CPU load in successive frames to avoid underrun/overrun issues (because FIR-equalizer algo's reinit requires a lot of processing)

   Bugs fixed
   ----------------------------------------------------------
   Configuration from curve (MDRC, FIR-equalizer algos): clamp out of range config to mix/max
   Faust algos dynamic config update bug correction (there was an issue for 2nd config change)
   Devel_generated_code, Release_generated_code targets: algo config JSON stringification correction
   Devel_generated_code, Release_generated_code targets: realloc issue correction
   bug correction in algo's config stringification for Devel_generated_code mode



## Known limitations



</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section4"  aria-hidden="true">
<label for="collapse-section4"  aria-hidden="true">1.3.0 June 27, 2024</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   Audio-kit Welcome page that provides many useful links
   Algorithms are documented
   Ease customer dedicated system IOs creation (FW & reference manual updates)
   More use cases given as examples: bass booster, Graphical equalizer, High directivity recording, Musical instruments such as guitar pedal and midi keyboard.
   Support PDM format in System IOs and all routing algorithms (router, splitter, interleaver, and so on)
   Support of Midi (SoundFont algorithm)
   Support of 1ms frame size with ac audio command
   Ease customer algo integration with AcIntergrate.py and dedicated documentation
   WOLA enhancements (more ratio and sampling frequencies supported)
   AcousticBf library integrated in LiveTune & AudioChain
   Update deinterleave algo to show separate outputs on separate pins (previously they were all on the same pin like split, which could be confusing)
   Added up to 8 channels capability for acChunk (for instance to comply to 7.1 audio systems format)
   On graph start, display all chunk capabilities consistency errors instead of only first one
   Color distinction as function of traces levels
   Fixed CPU info > 100% issue
   Added log scale option for graphs with frequency axis (spectrum graph output or FIR-eq config for instance)

   Bugs fixed
   ----------------------------------------------------------
   AudioChain: bug correction (HW handler crash) in some cases of parameter update
   AudioChain: wrong log message correction in case of input/output chunks duration consistency issue
   AudioChainJson_get_default_index correction in case drop list is too long
   AudioChain: HW handler crash bug correction in case of malloc fail (insufficient memory)
   Tools: fix ac_flash_firmware
   Replace faust_echo (bugged) with echo algo
   Fixed livetune error management in case of inconsistent graph (previously error was ignored)
   FNLMS: bug correction in matrix copy and rework/optimization



## Known limitations



</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section3"  aria-hidden="true">
<label for="collapse-section3"  aria-hidden="true">1.2.0 February 22, 2024</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   Removed audio_buffer.c audio_error_mgnt.ch audio_mem_mgnt.ch from here and place in Middlewares/ST/AudioBuffer because it is needed by several Utilities components

   Bug fixes
   ------------------------
   None

        

## Known limitations



</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section2"  aria-hidden="true">
<label for="collapse-section2"  aria-hidden="true">1.1.0 January 30, 2024</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   MISRAC corrections
   Remove terminal warning messages in case of wrong algo param settings; replaced by UI message.
   AudioChain update: chunk descriptor with better parameter selectable values
   AudioChain update: better error message for chunks consistency check
   AudioChain update: new API for sysIos feature oriented rather than index oriented
   AudioChain update: tasks creation done using Utilities/STOs that replaces the old Utilities/Cmsisos
   Mapped AudioMalloc to the memory pools manager to use all the available memory (not only internal RAM).
   Add AudioMalloc debug message (display error with TRACE_LEVEL_ERROR level)
   Rework cmd line presentation
   Updated documentation
   Adding usecases
   Added tinyusb support with debug capabilities: Add second console via VCP CDC tusb_cdc_debug_printf(char *pFormat, ...);
   Add guitar-phaser &  guitar-reverb

   Bug fixes
   ------------------------
   Fix unexpected stop pipe
   Restricted the noise reductor to frequencies <= 16000 (only for voice usecase)



## Known limitations



</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section1"  aria-hidden="true">
<label for="collapse-section1"  aria-hidden="true">1.0.0 December 6, 2023</label>
<div>


## Main changes


   First release


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
