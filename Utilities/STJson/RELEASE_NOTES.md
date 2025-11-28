---
pagetitle: Release Notes for json Component
lang: en
header-includes: <link rel="icon" type="image/x-icon" href="./scripts/_htmresc/favicon.png" />
---

::: {.row}
::: {.col-sm-12 .col-lg-4}

<center>
# Release Notes for json Component

Copyright &copy;2024 STMicroelectronics

All rights reserved
    
[![ST logo](./scripts/_htmresc/st_logo_2020.png)](https://www.st.com){.logo}
</center>

# License

The component license is described in : [SLA0044_LICENSE.md](SLA0044_LICENSE.md) or [SLA0044_LICENSE.txt](SLA0044_LICENSE.txt)

\
You may not use this component except in compliance with these Licenses.


# Purpose


Simple API to manage json files exchange from/to host to/from STM32. 


:::
::: {.col-sm-12 .col-lg-8}
# Update history

::: {.collapse}
<input type="checkbox" id="collapse-section10" checked aria-hidden="true">
<label for="collapse-section10"  aria-hidden="true">1.3.5 July 30, 2024</label>
<div>


## Main changes

expose json realloc\n

## Known limitations

None

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section9"  aria-hidden="true">
<label for="collapse-section9"  aria-hidden="true">1.3.4 July 17, 2024</label>
<div>


## Main changes

remove useless trailing zeroes after decimal point in float stringification\n

## Known limitations

None

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section8"  aria-hidden="true">
<label for="collapse-section8"  aria-hidden="true">1.3.3 May 28, 2024</label>
<div>


## Main changes

code spell check\n

## Known limitations

None

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section7"  aria-hidden="true">
<label for="collapse-section7"  aria-hidden="true">1.3.2 October 30, 2023</label>
<div>


## Main changes

Add json_stringify to master memory pointer\n

## Known limitations

None

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section6"  aria-hidden="true">
<label for="collapse-section6"  aria-hidden="true">1.3.1 October 04, 2022</label>
<div>


## Main changes

Fix bug when no memory at address 0x0\n

## Known limitations

None

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section5"  aria-hidden="true">
<label for="collapse-section5"  aria-hidden="true">1.3.0 September 23, 2022</label>
<div>


## Main changes

Add min & max values descriptors for better parameter management \n

## Known limitations

None

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section4"  aria-hidden="true">
<label for="collapse-section4"  aria-hidden="true">1.2.0 September 08, 2022</label>
<div>


## Main changes

Add macros to allow user to check returned error and keep its source code easy to read. That way, the JSON structure is visible in the user source code\n

## Known limitations

None

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section3"  aria-hidden="true">
<label for="collapse-section3"  aria-hidden="true">1.1.0 August 30, 2022</label>
<div>


## Main changes

Minor API modification to ease usage of legacy API\n

## Known limitations

None

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section2"  aria-hidden="true">
<label for="collapse-section2"  aria-hidden="true">1.0.0 July 18, 2022</label>
<div>


## Main changes


  Additional features
  ----------------------------------------------------------
  c code formatting ==> aligning with official templates & MISRA check
  New API that doesn't mix error returned with values and index
  Legacy API is kept and still usable setting the define called JSON_LEGACY_API_USED

  Bug fixes                
  ------------------------ 
  With new API only, now handles the cases where a value = -1 (previous was a code of error)


## Known limitations

None

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section1"  aria-hidden="true">
<label for="collapse-section1"  aria-hidden="true">alpha June 24, 2022</label>
<div>


## Main changes


Creation of the component from legacy STVS project


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
