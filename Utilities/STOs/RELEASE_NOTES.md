---
pagetitle: Release Notes for STOs Component
lang: en
header-includes: <link rel="icon" type="image/x-icon" href="./scripts/_htmresc/favicon.png" />
---

::: {.row}
::: {.col-sm-12 .col-lg-4}

<center>
# Release Notes for STOs Component

Copyright &copy;2024 STMicroelectronics

All rights reserved
    
[![ST logo](./scripts/_htmresc/st_logo_2020.png)](https://www.st.com){.logo}
</center>

# License

The component license is described in : [SLA0044_LICENSE.md](SLA0044_LICENSE.md) or [SLA0044_LICENSE.txt](SLA0044_LICENSE.txt)

\
You may not use this component except in compliance with these Licenses.


# Purpose



This OS Wrapper offers a common API for different OS. It is OS agnostic but in our project it lays on top of 
the cmsisos and offers an easy way to support either V1 & V2. The API is in the files st_os.[ch].

It comes with a compiler support file to manage Dlib_threads and also ensure that no malloc is done under ISRs.
The file st_os_mem.[ch] given as example is implementing allocation through another Utility component called STPmem.
STPmem is a memory pool creator and manager. It is optional. 
It helps using the all the different memory banks on a device through a single allocation API. 
Using st_os_mem.[ch] is not mandatory.

The files st_os_hl.[ch] offers higher level services such as:\
  + API to create tasks that are triggered by message queue (single API st_os_hl_task_create),\
  + encapsulates traces for real time tasks monitoring (CPU & memory footprints, stack levels, messages queues fifo level, etc... ), \
  + API to create a background task that can be triggered by others (helps monitoring the exact idle CPU load).  \
  + Please refer to the header [st_os_hl.h](st_os_hl.h) for more details.\


:::
::: {.col-sm-12 .col-lg-8}
# Update history

::: {.collapse}
<input type="checkbox" id="collapse-section21" checked aria-hidden="true">
<label for="collapse-section21"  aria-hidden="true">1.6.1 March 05, 2025</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   Add DTCM, ITCM & RAMINT memory pools for H7RS
   update mem pools names to simplify AudioMalloc pools management

   Bug fixes
   ------------------------


## Known limitations



</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section20"  aria-hidden="true">
<label for="collapse-section20"  aria-hidden="true">1.6.0 February 10, 2025</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   Add ThreadX support

   Bug fixes
   ------------------------


## Known limitations



</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section19"  aria-hidden="true">
<label for="collapse-section19"  aria-hidden="true">1.5.0 January 16, 2025</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   API change mem pool size in bytes instead of kbytes
   Increase ITCM pool size
   API: Allow pool enable per project thanks to new defines
   API H7xx: rename pool ; no longer as function of size which varies from on H7 to another but adresses which are the same

   Bug fixes
   ------------------------
   Fix rsize compute


## Known limitations



</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section18"  aria-hidden="true">
<label for="collapse-section18"  aria-hidden="true">1.4.10 December 3, 2024</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   Direct call of __disable_irq replaced by a wrapped version of __disable_irq to count and avoid nested calls.

   Bug fixes
   ------------------------
   Minor


## Known limitations



</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section17"  aria-hidden="true">
<label for="collapse-section17"  aria-hidden="true">1.4.9 November 27, 2024</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   None

   Bug fixes
   ------------------------
   Fix bug in st_os_mem_check_ptr; in case ST_os_mem is used but no pool is created it would return false even if pointer in HEAP was valid.


## Known limitations

None

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section16"  aria-hidden="true">
<label for="collapse-section16"  aria-hidden="true">1.4.8 November 22, 2024</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   None
   
   Bug fixes
   ------------------------
   Fix do not hide HEAP in mem command


## Known limitations

None

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section15"  aria-hidden="true">
<label for="collapse-section15"  aria-hidden="true">1.4.7 November 13, 2024</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   Add lock __iar_file_xxx using a mutex rather vTaskSuspendAll ( condition ST_OS_SUPPORT_IAR_USE_MUTEX) 
   
   Bug fixes
   ------------------------
   Fix kernel state active
   Fix a bug when a task is created and finished before to quit the create call
   Fix tasks' chained list


## Known limitations

None

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section14"  aria-hidden="true">
<label for="collapse-section14"  aria-hidden="true">1.4.6 September 26, 2024</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   st_os_mem_realloc API rework
   use mutex instead of disable irq for memory allocation protection

   Bug fixes
   ------------------------
   small bugs correction for stmcube build


## Known limitations

None

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section13"  aria-hidden="true">
<label for="collapse-section13"  aria-hidden="true">1.4.5 September 26, 2024</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   rename st_os_mem_best_max into st_os_mem_any_max (used for search of best memory pool in case of memory type ST_Mem_Type_ANY_FAST or ST_Mem_Type_ANY_SLOW)
   reorder memory pool: DTCM before ITCM (on H735, DTCM is more efficient than ITCM)
   add st_os_mem_generic_realloc_slow which allocates preferably in slow memory if input pointer is NULL
   realloc strings preferably in slow memory if input string is NULL (avoids usage and fragmentation of TCM with small strings)
   Implement st_os_task_join not available in freertos
   Add real time UTC support
   Add pool number
   st_os_mem_type_from_name returns ST_Mem_Type_UNMAPPED if it doesn't find, st_os_mem_alloc_named & st_os_mem_calloc_named don't allocate if type is wrong, s_scan_for_slow bug correction
   Refactor the way to implemant pools in the template and cleanup & new it is possible to have pools not used by the Fast/slow mechanism
   Add a performance memory index to pmem pools

   Bug fixes
   ------------------------
   Fix: Check pointer for error
   Fix a bug for scan slow and fast allocation
   ST_Mem_is_any bug correction : it was returning True when pool was ST_Mem_Type_UNMAPPED
   Fixed bug in case of ST_USE_PMEM undefined


## Known limitations

None

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section12"  aria-hidden="true">
<label for="collapse-section12"  aria-hidden="true">1.4.4 August 22, 2024</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   Refactor the way to implement pools in the template and cleanup
   Now possible to create pools not used by the Fast/slow mechanisms

   Bug fixes                
   ------------------------
   st_os_mem_type_from_name returns ST_Mem_Type_UNMAPPED if it doesn't find any
   st_os_mem_alloc_named & st_os_mem_calloc_named don't allocate if type is wrong
   s_scan_for_slow bug correction


## Known limitations

None

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section11"  aria-hidden="true">
<label for="collapse-section11"  aria-hidden="true">1.4.3 July 15, 2024</label>
<div>


## Main changes


   Bug fixes                
   ------------------------
   circular buffer management issue correction
   random crash with host command correction


## Known limitations

None

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section10"  aria-hidden="true">
<label for="collapse-section10"  aria-hidden="true">1.4.2 June 27, 2024</label>
<div>


## Main changes


   Bug fixes                
   ------------------------
   removes dependency on st_base.h
   fixes wrong includes of st_os_mem_conf.h


## Known limitations

None

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section9"  aria-hidden="true">
<label for="collapse-section9"  aria-hidden="true">1.4.1 June 17, 2024</label>
<div>


## Main changes


   Bug fixes                
   ------------------------
   remove gcc warnings



## Known limitations

None

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section8"  aria-hidden="true">
<label for="collapse-section8"  aria-hidden="true">1.4.0 June 14, 2024</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   Rework memory allocation strategy



## Known limitations

None

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section7"  aria-hidden="true">
<label for="collapse-section7"  aria-hidden="true">1.3.0 May 28, 2024</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   Migration FreeRtos: 10.5.x
   API: Add st_os_mem_type_from_name



## Known limitations

None

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section6"  aria-hidden="true">
<label for="collapse-section6"  aria-hidden="true">1.2.2 March 12, 2024</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   MISRAC

   Bug fixes                
   ------------------------
None


## Known limitations

None

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section5"  aria-hidden="true">
<label for="collapse-section5"  aria-hidden="true">1.2.1 February 22, 2024</label>
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
<input type="checkbox" id="collapse-section4"  aria-hidden="true">
<label for="collapse-section4"  aria-hidden="true">1.2.0 January 31, 2024</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   add st_os_hl.[ch] which offer high level services to create triggered tasks (thread that run upon message queue recpetion). It also offers debug means and traces for all threads (CPU, prio level, stack size usage etc..) 
   add iar tls initialization hook

   Bug fixes                
   ------------------------
   tls management : use ticks instead of cycles to remove dependency with cycles.h



## Known limitations

None

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section3"  aria-hidden="true">
<label for="collapse-section3"  aria-hidden="true">1.1.0 November 23, 2023</label>
<div>


## Main changes


  Additional features
  ----------------------------------------------------------
  Manage mutex if kernel not started
  Introduce st_os_task_udelay
  Add  st_os_monitor_cpu
  Add threadx
  Add Stream buffer support
  Fix FreeRtos dependency
  Remove freertos code in st_os_cmsis_v2.c
  H5 support
  Misra 2012 


  Bug fixes                
  ------------------------ 
  Fix a bug when a bank doesn't exist
  Fix build issue with TRUE/FALSE
  Fix gcc build
  Fix gcc warning


## Known limitations

None

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section2"  aria-hidden="true">
<label for="collapse-section2"  aria-hidden="true">1.0.0 August 18, 2022</label>
<div>


## Main changes

First Release \n

## Known limitations

None

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section1"  aria-hidden="true">
<label for="collapse-section1"  aria-hidden="true">alpha June 24, 2022</label>
<div>


## Main changes


Creation of the dedicated component from legacy source code.


## Known limitations

none

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
