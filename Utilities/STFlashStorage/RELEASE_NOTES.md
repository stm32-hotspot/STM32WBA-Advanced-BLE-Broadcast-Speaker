---
pagetitle: Release Notes for STFlashStorage Component
lang: en
header-includes: <link rel="icon" type="image/x-icon" href="./scripts/_htmresc/favicon.png" />
---

::: {.row}
::: {.col-sm-12 .col-lg-4}

<center>
# Release Notes for STFlashStorage Component

Copyright &copy;2024 STMicroelectronics

All rights reserved
    
[![ST logo](./scripts/_htmresc/st_logo_2020.png)](https://www.st.com){.logo}
</center>

# License

The component license is described in : [SLA0044_LICENSE.md](SLA0044_LICENSE.md) or [SLA0044_LICENSE.txt](SLA0044_LICENSE.txt)

\
You may not use this component except in compliance with these Licenses.


# Purpose


Management of storing data in flash. 


:::
::: {.col-sm-12 .col-lg-8}
# Update history

::: {.collapse}
<input type="checkbox" id="collapse-section5" checked aria-hidden="true">
<label for="collapse-section5"  aria-hidden="true">1.1.3 March 20, 2025</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   H7RS support

   Bug fixes                
   ------------------------


## Known limitations



</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section4"  aria-hidden="true">
<label for="collapse-section4"  aria-hidden="true">1.1.2 February 10, 2025</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   Remove dependancy on core_init.h

   Bug fixes                
   ------------------------


## Known limitations



</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section3"  aria-hidden="true">
<label for="collapse-section3"  aria-hidden="true">1.1.1 January 16, 2025</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   None

   Bug fixes                
   ------------------------
   Solve DCache issue during flash write
   Remove st_os_enter_critical_section & st_os_exit_critical_section calls around BSP/HAL routines calls (makes timeout measurement with HAL_delay blocked)
   fix Flashing on STM32H747-DK and STM32N650-DK


## Known limitations



</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section2"  aria-hidden="true">
<label for="collapse-section2"  aria-hidden="true">1.1.0 November 22, 2024</label>
<div>


## Main changes


   Additional features
   ----------------------------------------------------------
   None

   Bug fixes                
   ------------------------
   fix flash KO after H5 HAL/BSP update


## Known limitations



</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section1"  aria-hidden="true">
<label for="collapse-section1"  aria-hidden="true">1.0.0 May 28, 2024</label>
<div>


## Main changes

First Release

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
