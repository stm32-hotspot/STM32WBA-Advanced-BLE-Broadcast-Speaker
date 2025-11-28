---
pagetitle: Release Notes for STPmem Component
lang: en
header-includes: <link rel="icon" type="image/x-icon" href="./scripts/_htmresc/favicon.png" />
---

::: {.row}
::: {.col-sm-12 .col-lg-4}

<center>
# Release Notes for STPmem Component

Copyright &copy;2024 STMicroelectronics

All rights reserved
    
[![ST logo](./scripts/_htmresc/st_logo_2020.png)](https://www.st.com){.logo}
</center>

# License

The component license is described in : [SLA0044_LICENSE.md](SLA0044_LICENSE.md) or [SLA0044_LICENSE.txt](SLA0044_LICENSE.txt)

\
You may not use this component except in compliance with these Licenses.


# Purpose


This API implements a custom memory pool management using standard allocation functions (malloc,free,realloc,calloc). 
The API implement also some tools to detect memory corruptions and can mark blocks with name or a tag.\


Blocks can support bytes alignements 4 8 16 32.


:::
::: {.col-sm-12 .col-lg-8}
# Update history

::: {.collapse}
<input type="checkbox" id="collapse-section14" checked aria-hidden="true">
<label for="collapse-section14"  aria-hidden="true">1.3.0 March 11, 2025</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   API: Expose pool name assert
   Add a check when a pool is created where RW_CONTENT are stored => traces can be sent to warn the developper.

   Bug fixes
   ------------------------
   Fix a realloc issue when the first block is reallocated
   Fix a bug in pmem_info


## Known limitations



</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section13"  aria-hidden="true">
<label for="collapse-section13"  aria-hidden="true">1.2.9 December 3, 2024</label>
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
<input type="checkbox" id="collapse-section12"  aria-hidden="true">
<label for="collapse-section12"  aria-hidden="true">1.2.8 November 22, 2024</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   None

   Bug fixes                
   ------------------------
   Fix: pmem_leak_detector bug correction (wrong address displayed)


## Known limitations

None

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section11"  aria-hidden="true">
<label for="collapse-section11"  aria-hidden="true">1.2.7 October 11, 2024</label>
<div>


## Main changes

minor bugs corrections depending on platform

## Known limitations

None

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section10"  aria-hidden="true">
<label for="collapse-section10"  aria-hidden="true">1.2.6 September 26, 2024</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   improve memory perf measure (remove loop non-RW cycles)
   Add : pmem_compute_performance_index() computes a performance index for 1ms

   Bug fixes                
   ------------------------
   Fix: Performance counter not working if not initialized


## Known limitations

None

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section9"  aria-hidden="true">
<label for="collapse-section9"  aria-hidden="true">1.2.5 August 22, 2024</label>
<div>


## Main changes

Add : pmem_compute_performance_index() computes a performance index for 1ms\n

## Known limitations

None

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section8"  aria-hidden="true">
<label for="collapse-section8"  aria-hidden="true">1.2.4 July 15, 2024</label>
<div>


## Main changes

use bool type for boolean values\n

## Known limitations

None

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section7"  aria-hidden="true">
<label for="collapse-section7"  aria-hidden="true">1.2.3 June 17, 2024</label>
<div>


## Main changes

warnings correction\n

## Known limitations

None

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section6"  aria-hidden="true">
<label for="collapse-section6"  aria-hidden="true">1.2.2 June 14, 2024</label>
<div>


## Main changes

code spell check\n

## Known limitations

None

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section5"  aria-hidden="true">
<label for="collapse-section5"  aria-hidden="true">1.2.1 May 28, 2024</label>
<div>


## Main changes

code spell check\n

## Known limitations

None

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section4"  aria-hidden="true">
<label for="collapse-section4"  aria-hidden="true">1.2.0 April 24, 2024</label>
<div>


## Main changes

Leak information added\n

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
  Remove char_t
  Misra 2012 
  CStat Update
  Add pmem_check_pool_ptr
  Cleaned the Leak detector
  Disable default corruption check for each allocation
  Remove pmem_collapse_free_blk for each alloc & default PMEM_MALLOC_NAMED for optimization
  Rework re-alloc and cleanup
  Add hook functions

  Bug fixes                
  ------------------------ 
  Fix gcc build
  Fix gcc warning
  Fix link issue in none instrumentation mode


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
