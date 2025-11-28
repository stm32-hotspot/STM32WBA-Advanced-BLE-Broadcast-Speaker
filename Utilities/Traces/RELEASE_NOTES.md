---
pagetitle: Release Notes for traces Component
lang: en
header-includes: <link rel="icon" type="image/x-icon" href="./scripts/_htmresc/favicon.png" />
---

::: {.row}
::: {.col-sm-12 .col-lg-4}

<center>
# Release Notes for traces Component

Copyright &copy;2024 STMicroelectronics

All rights reserved
    
[![ST logo](./scripts/_htmresc/st_logo_2020.png)](https://www.st.com){.logo}
</center>

# License

The component license is described in : [SLA0044_LICENSE.md](SLA0044_LICENSE.md) or [SLA0044_LICENSE.txt](SLA0044_LICENSE.txt)

\
You may not use this component except in compliance with these Licenses.


# Purpose


traces allows sending trace over:
\
- UART virtual com port
\
- Display

It supports two modes:
\
- synchronous
\
- asynchronous

The current mode can be changed dynamically.
\


:::
::: {.col-sm-12 .col-lg-8}
# Update history

::: {.collapse}
<input type="checkbox" id="collapse-section16" checked aria-hidden="true">
<label for="collapse-section16"  aria-hidden="true">2.5.2 December 3, 2024</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   Direct call of __disable_irq replaced by a wrapped version of __disable_irq to count and avoid nested calls.

   Bug fixes
   ------------------------
   Minor


## Known limitations

In case of asynchronous mode, the depth of the traces FIFO must be tuned properly. It is a static table sized by the define NB_TRACE_LOG_BUFFERS

                        Traces over display has a dependency on Utilities/lcd source code

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section15"  aria-hidden="true">
<label for="collapse-section15"  aria-hidden="true">2.5.1 November 22, 2024</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   None

   Bug fixes
   ------------------------
   Minor


## Known limitations

In case of asynchronous mode, the depth of the traces FIFO must be tuned properly. It is a static table sized by the define NB_TRACE_LOG_BUFFERS

                Traces over display has a dependency on Utilities/lcd source code

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section14"  aria-hidden="true">
<label for="collapse-section14"  aria-hidden="true">2.5.0 May 28, 2024</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   API: rename trace_setDebugLevel into trace_setLevel and trace_getDebugLevel into trace_getLevel
   codespell corrections
   add UART trace color depending on trace level

   Bug fixes
   ------------------------
   None


## Known limitations

In case of asynchronous mode, the depth of the traces FIFO must be tuned properly. It is a static table sized by the define NB_TRACE_LOG_BUFFERS

                Traces over display has a dependency on Utilities/lcd source code

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section13"  aria-hidden="true">
<label for="collapse-section13"  aria-hidden="true">2.4.0 October 30, 2023</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   API: Allow to hook cleanly trace_print
   API: add trace_getDebugLevel for bit wise operations
   API: add TRACE_LVL_VERBOSE trace level
   API: filter trace_print depending on trace level
   API: Add trace_print_args_hook (used by designer to stop the pipe upon error)
    
   Bug fixes
   ------------------------ 
   remove __weak from header
   remove malloc in trace2disply and better init concurrency management
   traces2display: better protection of firstTime
   no use stdlib (malloc, printf, etc...) when called from interrupt
   rework traces: use malloc instead of static buffer in case of print without format (args) so that big buffers are not truncated; more checks inside code


## Known limitations

In case of asynchronous mode, the depth of the traces FIFO must be tuned properly. It is a static table sized by the define NB_TRACE_LOG_BUFFERS

        Traces over display has a dependency on Utilities/lcd source code

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section12"  aria-hidden="true">
<label for="collapse-section12"  aria-hidden="true">2.3.0 January 11, 2023</label>
<div>


## Main changes


  Additional features
  ----------------------------------------------------------
  add trace buffer consistency check (CHECK_TRACE_CONSISTENCY must be enabed for check)
  add UART characters color/style modifiers constants
  flush trace if level >= error so that error traces are output even in asynchronous mode when FW is blocked (for instance in an assert); only for trace over UART, not on display
  introduce ERROR_FATAL level that will stop application
  rename TRACE_SendPrintf in TRACE_fputs
  increase nb of trace buffers

  Bug fixes
  ------------------------ 
  more robust test for allocation at init; also memset memory otherwise some hardFault occurs
  warning correction in case PUTCHAR_PROTOTYPE is not defined

        

## Known limitations

In case of asynchronous mode, the depth of the traces FIFO must be tuned properly. It is a static table sized by the define NB_TRACE_LOG_BUFFERS

Traces over display has a dependency on Utilities/lcd source code

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section11"  aria-hidden="true">
<label for="collapse-section11"  aria-hidden="true">2.2.8 September 23, 2022</label>
<div>


## Main changes


  Additional features
  ----------------------------------------------------------
  remove hard coded dependency on Utilities/terminal
  MISRA warnings removal

  Bug fixes
  ------------------------ 
  None



## Known limitations

In case of asynchronous mode, the depth of the traces FIFO must be tuned properly. It is a static table sized by the define NB_TRACE_LOG_BUFFERS
            
Traces over display has a dependency on Utilities/lcd source code

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section10"  aria-hidden="true">
<label for="collapse-section10"  aria-hidden="true">2.2.7 June 21, 2022</label>
<div>


## Main changes


  Additional features
  ----------------------------------------------------------
  No functional modification, only c code formatting ==> aligning with official templates & MISRA check

  Bug fixes                
  ------------------------ 
  None



## Known limitations

In case of asynchronous mode, the depth of the traces FIFO must be tuned properly. It is a static table sized by the define NB_TRACE_LOG_BUFFERS

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section9"  aria-hidden="true">
<label for="collapse-section9"  aria-hidden="true">2.2.6 November 22, 2021</label>
<div>


## Main changes


  Additional features
  ----------------------------------------------------------
  API break for easier management of synchronous vs asynchronous behaviour

  Bug fixes                
  ------------------------ 
  None



## Known limitations

In case of asynchronous mode, the depth of the traces FIFO must be tuned properly. It is a static table sized by the define NB_TRACE_LOG_BUFFERS

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section8"  aria-hidden="true">
<label for="collapse-section8"  aria-hidden="true">__v2.2.4__ July 9, 2021</label>
<div>


## Main changes

synchronous/asynchronous traces management now allows dynamic swith from one to the other

## Known limitations

Network stability issues after long runs

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section7"  aria-hidden="true">
<label for="collapse-section7"  aria-hidden="true">__v2.2.2__ March 15, 2021</label>
<div>


## Main changes

solve display H735 fill rectangle

## Known limitations

NA

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section6"  aria-hidden="true">
<label for="collapse-section6"  aria-hidden="true">__v2.2.1__ February 10, 2021</label>
<div>


## Main changes

Enhance tracing system for long traces over display (trace remain the same on uart)

## Known limitations

NA

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section5"  aria-hidden="true">
<label for="collapse-section5"  aria-hidden="true">__v2.2.0__ February 10, 2021</label>
<div>


## Main changes

introduces #include trace_conf.h to allow to build AudioChain library

## Known limitations

NA

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section4"  aria-hidden="true">
<label for="collapse-section4"  aria-hidden="true">__v2.1.0__ November 18, 2020</label>
<div>


## Main changes

Api break to be aligned with STVS project tracing system\n removed useless enum

## Known limitations

NA

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section3"  aria-hidden="true">
<label for="collapse-section3"  aria-hidden="true">__v2.0.0__ November 18, 2020</label>
<div>


## Main changes

Api break to be aligned with STVS project tracing system\n removed useless enum

## Known limitations

NA

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section2"  aria-hidden="true">
<label for="collapse-section2"  aria-hidden="true">__v1.2.0__ July 31, 2019</label>
<div>


## Main changes

Minor modif, integrated in the washing Machine local reco project

## Known limitations

NA

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section1"  aria-hidden="true">
<label for="collapse-section1"  aria-hidden="true">__v1.1.0__ July 5, 2019</label>
<div>


## Main changes

First release

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
