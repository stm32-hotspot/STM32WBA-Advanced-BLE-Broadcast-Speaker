---
pagetitle: Release Notes for CyclesCnt Component
lang: en
header-includes: <link rel="icon" type="image/x-icon" href="./scripts/_htmresc/favicon.png" />
---

::: {.row}
::: {.col-sm-12 .col-lg-4}

<center>
# Release Notes for CyclesCnt Component

Copyright &copy;2024 STMicroelectronics

All rights reserved
    
[![ST logo](./scripts/_htmresc/st_logo_2020.png)](https://www.st.com){.logo}
</center>

# License

The component license is described in : [SLA0044_LICENSE.md](SLA0044_LICENSE.md) or [SLA0044_LICENSE.txt](SLA0044_LICENSE.txt)

\
You may not use this component except in compliance with these Licenses.


# Purpose

Count cycles consumed using CortexM dedicated register


:::
::: {.col-sm-12 .col-lg-8}
# Update history

::: {.collapse}
<input type="checkbox" id="collapse-section14" checked aria-hidden="true">
<label for="collapse-section14"  aria-hidden="true">3.1.1 December 3, 2024</label>
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
<input type="checkbox" id="collapse-section13"  aria-hidden="true">
<label for="collapse-section13"  aria-hidden="true">3.1.0 May 28, 2024</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   Improve cycles measure accuracy



## Known limitations

None

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section12"  aria-hidden="true">
<label for="collapse-section12"  aria-hidden="true">3.0.1 January 30, 2024</label>
<div>


## Main changes

ran MISRA over source code

## Known limitations



</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section11"  aria-hidden="true">
<label for="collapse-section11"  aria-hidden="true">3.0.0 October 30, 2023</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   API: split cycles timeout in 2 different durations: 1 for measure duration, 1 for call-back (for instance for log display) interval
   API: add new types of task for cycles measure
   API: add cycleMeasure_isTimeoutExpired to manage tasks such as control whose occurence is very irregular and may be very long
   API: add task type in cyclesStats structure (DATAINOUT_TASK, PROCESS_TASK, PROCESSLOWLEVEL_TASK or CONTROL_TASK)
   API: add pUserCookie in CycleStatsTypeDef
   API: add cpuLoadPcent 
   cycles measure bug correction (interrupts wasn't disabled and re-enabled at the right time)
   misrac
   removed __weak from header => issues solving
   if last measure is not available (timeout not reached), give current measure instead (less accurate)
   measure cycles on last complete measure (CYCLES_LAST_MEASURE) or current on-going measure (CYCLES_CURRENT_MEASURE); add cycleMeasure_getCyclesMgntStats routine to store cycles stat in a readable structure from a raw cyclesCnt structure
   use CoreDebug and DWT structures fields instead of hard coded pointers
   cycles measure can now manage time slice between task of the same priority

   Bug fixes                
   ------------------------ 
   None


## Known limitations

NA

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section10"  aria-hidden="true">
<label for="collapse-section10"  aria-hidden="true">2.2.6 January 23, 2022</label>
<div>


## Main changes

Introduce ERROR_FATAL for traces..
            \
            Add cycleMeasure_getSystemCoreClock routine

## Known limitations

NA

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section9"  aria-hidden="true">
<label for="collapse-section9"  aria-hidden="true">2.2.5 September 23, 2022</label>
<div>


## Main changes

Removed dependency on Utilities/traces.
             \
             Added possibilty to configure through cycles_cnt_conf.h
             \
             Now possible to remove traces or chose to use traces.c or with the simple printf

## Known limitations

NA

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section8"  aria-hidden="true">
<label for="collapse-section8"  aria-hidden="true">2.2.4 March 08, 2022</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   No functional modification, only c code formatting ==> aligning with official templates & MISRA check

   Bug fixes                
   ------------------------ 
   None


## Known limitations

NA

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section7"  aria-hidden="true">
<label for="collapse-section7"  aria-hidden="true">2.2.3 September 6, 2021</label>
<div>


## Main changes

minor API change to use traces utility ==> renaming of trace routines

## Known limitations

NA

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section6"  aria-hidden="true">
<label for="collapse-section6"  aria-hidden="true">__v2.2.2__ June 16, 2021</label>
<div>


## Main changes

new field to store name of component being analyzed

## Known limitations

NA

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section5"  aria-hidden="true">
<label for="collapse-section5"  aria-hidden="true">__v2.2.1__ March 15, 2021</label>
<div>


## Main changes

better system on Display & uart for long traces

## Known limitations

NA

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section4"  aria-hidden="true">
<label for="collapse-section4"  aria-hidden="true">__v2.2.0__ November 18, 2020</label>
<div>


## Main changes

minor api change for min max\n remove while(1) 

## Known limitations

NA

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section3"  aria-hidden="true">
<label for="collapse-section3"  aria-hidden="true">__v2.1.0__ December 8, 2020</label>
<div>


## Main changes

added headers files\n remove dep on HAL

## Known limitations

NA

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section2"  aria-hidden="true">
<label for="collapse-section2"  aria-hidden="true">__v2.0.0__ November 24, 2020</label>
<div>


## Main changes

Api break

## Known limitations

NA

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section1"  aria-hidden="true">
<label for="collapse-section1"  aria-hidden="true">__v1.0.0__ April 5, 2017</label>
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
