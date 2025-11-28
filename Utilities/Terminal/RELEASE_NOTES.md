---
pagetitle: Release Notes for terminal Component
lang: en
header-includes: <link rel="icon" type="image/x-icon" href="./scripts/_htmresc/favicon.png" />
---

::: {.row}
::: {.col-sm-12 .col-lg-4}

<center>
# Release Notes for terminal Component

Copyright &copy;2024 STMicroelectronics

All rights reserved
    
[![ST logo](./scripts/_htmresc/st_logo_2020.png)](https://www.st.com){.logo}
</center>

# License

The component license is described in : [SLA0044_LICENSE.md](SLA0044_LICENSE.md) or [SLA0044_LICENSE.txt](SLA0044_LICENSE.txt)

\
You may not use this component except in compliance with these Licenses.


# Purpose


It contains 2 main components:
\
- stm32_usart that manages UART for virtual com port communication with a host
\
- stm32_term that offers an API to build a console with some user defined commands
 
It supports two modes:
\
- bare metal implementation
\
- os based; example given with FreeRTOS

For bare metal, it uses the utility buff component:
\
- stm32_buff manages ring buffer of bytes (very simple mechanism used to exchange data between threads or SW tasks)


:::
::: {.col-sm-12 .col-lg-8}
# Update history

::: {.collapse}
<input type="checkbox" id="collapse-section17" checked aria-hidden="true">
<label for="collapse-section17"  aria-hidden="true">1.6.6 February 10, 2025</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   Add threadX support

   Bug fixes
   ------------------------
   None


## Known limitations



</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section16"  aria-hidden="true">
<label for="collapse-section16"  aria-hidden="true">1.6.5 December 3, 2024</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   Direct call of __disable_irq replaced by a wrapped version of __disable_irq to count and avoid nested calls.
   API renaming: align define naming; TIMx_DMA_TIMEOUT_IRQn replaced by UTIL_UART_TIMx_DMA_TIMEOUT_IRQn

   Bug fixes
   ------------------------
   Minor


## Known limitations



</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section15"  aria-hidden="true">
<label for="collapse-section15"  aria-hidden="true">1.6.4 November 13, 2024</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   In case of FreeRTOS, use mutex instead of __disable_irq/__enable_irq as locking mechanism

   Bug fixes                
   ------------------------ 
   None


## Known limitations



</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section14"  aria-hidden="true">
<label for="collapse-section14"  aria-hidden="true">1.6.3 October 10, 2024</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   In case of FreeRTOS, use mutex instead of __disable_irq/__enable_irq
   Code unification for all STM32 families

   Bug fixes                
   ------------------------ 
   Bug correction for stop dma RX/TX


## Known limitations



</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section13"  aria-hidden="true">
<label for="collapse-section13"  aria-hidden="true">1.6.2 August 22, 2024</label>
<div>


## Main changes

Use linked list DMA

## Known limitations



</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section12"  aria-hidden="true">
<label for="collapse-section12"  aria-hidden="true">1.6.1 July 30, 2024</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   Solve potential lock/unlock issue in UART buffer management
   Better management of errors
   API unification for error types & boolean types



## Known limitations



</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section11"  aria-hidden="true">
<label for="collapse-section11"  aria-hidden="true">1.6.0 June 14, 2024</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   LiveTune graph load & algos preset update speed-up (data transfer through terminal improvement)



## Known limitations



</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section10"  aria-hidden="true">
<label for="collapse-section10"  aria-hidden="true">1.5.0 May 28, 2024</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   Remove 16 bits limitation
   Migration FreeRtos: 10.5.x


## Known limitations



</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section9"  aria-hidden="true">
<label for="collapse-section9"  aria-hidden="true">1.4.0 March 28, 2024</label>
<div>


## Main changes

codespell checks

## Known limitations



</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section8"  aria-hidden="true">
<label for="collapse-section8"  aria-hidden="true">1.3.3 March 12, 2024</label>
<div>


## Main changes

priority change, default value were too high

## Known limitations



</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section7"  aria-hidden="true">
<label for="collapse-section7"  aria-hidden="true">1.3.2 January 31, 2024</label>
<div>


## Main changes

removed legacy Utilities/cmsisos replaced by Utilities/STOs

## Known limitations



</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section6"  aria-hidden="true">
<label for="collapse-section6"  aria-hidden="true">1.3.1 January 30, 2024</label>
<div>


## Main changes

ran MISRA over source code

## Known limitations



</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section5"  aria-hidden="true">
<label for="collapse-section5"  aria-hidden="true">1.3.0 October 30, 2023</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   introduce UTIL_TERM_receive_file that use realloc to save memory
   finalize  VT100 CSI support
   Support of H5: Fix Rx and TX DMA transmission
   Remove while loop in case of error != HAL_OK) in s_startDmaTx
   HAL UART block sizes are uint16_t thus make terminal compliant to these sizes
   UTIL_TERM_printf doesn't add carriage-return; add new UTIL_TERM_printf_cr routine for this purpose
   add UTIL_TERM_inject_rx for terminal playback after recording
   Introduce the terminal logger
   Add enable log async function
   set baud rate to 921600

   Bug fixes                
   ------------------------ 
   __weak removed from header 
   don't activate back-ground task from terminal task
   increase tFormat size for new cpu load display
   increase tFormat string size for new displays in cpu command
   Fix uart issue when high speed
   Manage UART DMA busy state
   move malloc and free outside of a disable interupt section 
   manage circular buffer differently (there was an issue when 32 bits index was wrapping : modulus wasn't ok after wrapping) 
   better interrputs protection of dma buffer structures 
   every lock must have its related unlock (there was an unlock without related lock in s_UTIL_UART_FillBlock)
   Fix extra CR
   Create independ stream buffer to speed up the transmision
   Add char stream TX
   Send block uart async usin DMA ok
   rework terminal using generic os wrapper



## Known limitations



</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section4"  aria-hidden="true">
<label for="collapse-section4"  aria-hidden="true">1.2.0 January 11, 2022</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   Introduced wrapper for cmsis OS v2
   Default settings changed (high baud rate)

   Bug fixes                
   ------------------------ 
   Fix xStreamBufferSendFromISR when size = 0


## Known limitations

In case of sending big file from host to STM32, the UART Rx DMA might be two small. Error is sent, it is up to the user to compile with deeper buffer.

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section3"  aria-hidden="true">
<label for="collapse-section3"  aria-hidden="true">1.1.0 September 23, 2022</label>
<div>


## Main changes


  Additional features
  ----------------------------------------------------------
  stm32_usart is now HAL independant, thru callback registering. It allows to handle several families not only H7.
  Some templates of callbacks registration are provided for H5 F7 & F4.
  The bare metal implementation of the terminal was simplified

  Bug fixes                
  ------------------------ 
  The bare metal implementation was failing upon big data amount transfers



## Known limitations

In case of sending big file from host to STM32, the UART Rx DMA might be two small. Error is sent, it is up to the user to compile with deeper buffer.

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section2"  aria-hidden="true">
<label for="collapse-section2"  aria-hidden="true">1.0.0 July 19, 2022</label>
<div>


## Main changes


  Additional features
  ----------------------------------------------------------
  c code formatting ==> aligning with official templates & MISRA check
  adding bare metal support with stm32_buff.c

  Bug fixes                
  ------------------------ 
  None





## Known limitations

In case of sending big file from host to STM32, the UART Rx DMA might be two small. Error is sent, it is up to the user to compile with deeper buffer.

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section1"  aria-hidden="true">
<label for="collapse-section1"  aria-hidden="true">alpha May 22, 2022</label>
<div>


## Main changes


First release


## Known limitations

In case of asynchronous mode,  the depth of the traces FIFO must be tuned properly. It is a static table sized by the define NB_TRACE_LOG_BUFFERS

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
