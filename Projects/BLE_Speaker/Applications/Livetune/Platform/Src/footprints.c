/**
******************************************************************************
* @file    footprints.c
* @author  MCD Application Team
* @brief   utility to monitor footprints
******************************************************************************
* @attention
*
* Copyright (c) 2022(-2022) STMicroelectronics.
* All rights reserved.
*
* This software is licensed under terms that can be found in the LICENSE file
* in the root directory of this software component.
* If no LICENSE file comes with this software, it is provided AS-IS.
*
******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include "iar_dlmalloc.h"
#include "traces.h"

/* Global variables ----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
#define PRINT_SECTION(s, v)    if ((v) > 0UL) {trace_print(TRACE_OUTPUT_UART, TRACE_LVL_INFO2, "%s\t%8.3f KB\n", (s), (double)(v) / 1024.0);}
#define PRINT_SEP()            trace_print(TRACE_OUTPUT_UART, TRACE_LVL_INFO2, "===================================\n");

/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
#pragma section = "HEAP"
#pragma section = ".bss"
#pragma section = ".text"
#pragma section = ".data"
#pragma section = "CSTACK"
#pragma section = ".rodata"
#pragma section = ".qspi_data"
#pragma section = ".RAM2_Section"
#pragma section = ".SLOWRAM_Section"
#pragma section = ".DTCMRAM_Section"
#pragma section = ".noncacheable"
#pragma section = ".FB_Section"
#pragma section = ".ASR_Section"
#pragma section = "IntFlashSection"
#pragma section = "FontFlashSection"
#pragma section = "TextFlashSection"
#pragma section = "ExtFlashSection"

/* RAM*/
static uint32_t heap_size          = __section_size("HEAP");
static uint32_t cstack_size        = __section_size("CSTACK");
static uint32_t ram2_section_size  = __section_size(".RAM2_Section");
static uint32_t slowram_size       = __section_size(".SLOWRAM_Section");
static uint32_t dtcmram_size       = __section_size(".DTCMRAM_Section");
static uint32_t nocache_size       = __section_size(".noncacheable");
//static uint32_t bss_size           = __section_size(".bss");
//static uint32_t data_size          = __section_size(".data");

/*FLASH*/
//static uint32_t rodata_size        = __section_size(".rodata");
//static uint32_t text_size          = __section_size(".text");
static uint32_t int_flash_size     = __section_size("IntFlashSection");
static uint32_t font_flash_size    = __section_size("FontFlashSection");
static uint32_t text_flash_size    = __section_size("TextFlashSection");

/* QSPI*/
static uint32_t qspi_size          = __section_size(".qspi_data");
static uint32_t ext_flash_size     = __section_size("ExtFlashSection");

/* SDRAM*/
static uint32_t fb_size            = __section_size(".FB_Section");
static uint32_t asr_size           = __section_size(".ASR_Section");

static uint32_t total_ram_size_without_heap = 0UL;

/*"ROM_CONTENT needs to be defined in .icf file"*/
#pragma section = "ROM_CONTENT"
const size_t ROM_CONTENT_size @ "ROM_length_used" = __section_size("ROM_CONTENT");

/*"W_CONTENT needs to be defined in .icf file"*/
#pragma section = "RW_CONTENT"
const size_t RW_CONTENT_size @ "RW_length_used" = __section_size("RW_CONTENT");

/* Private function prototypes -----------------------------------------------*/
void footprint_static(void);
void main_hooks_footprintDump(void);

/* Functions Definition ------------------------------------------------------*/
void footprint_static(void)
{
  /* FLASH + QSPI */
  uint32_t total_flash_size         = ROM_CONTENT_size + int_flash_size + font_flash_size + text_flash_size;
  uint32_t total_qspi_size          = qspi_size + ext_flash_size;
  uint32_t total_ram2_size          = RW_CONTENT_size + ram2_section_size + slowram_size;
  uint32_t total_ram_size_with_heap = 0UL;

  PRINT_SEP();
  PRINT_SECTION("ROM               ", (total_flash_size + total_qspi_size));
  PRINT_SEP();
  PRINT_SECTION("-QSPI             ", total_qspi_size);
  PRINT_SECTION("-FLASH            ", total_flash_size);
  //PRINT_SEP();
  //PRINT_SECTION("text", text_size);
  //PRINT_SECTION("rodata", rodata_size);

  /* RAM = bss + data + CSTACK + HEAP */
  total_ram_size_without_heap = cstack_size + nocache_size + dtcmram_size + total_ram2_size;
  total_ram_size_with_heap    = heap_size + total_ram_size_without_heap;
  PRINT_SEP() ;
  PRINT_SECTION("RAM without heap  ", total_ram_size_without_heap);
  PRINT_SECTION("RAM with heap     ", total_ram_size_with_heap);
  PRINT_SEP();
  PRINT_SECTION("-HEAP             ", heap_size);
  PRINT_SECTION("-CSTACK           ", cstack_size);
  PRINT_SECTION("-NOCACHE_Section  ", nocache_size);
  PRINT_SECTION("-DTCMRAM_Section  ", dtcmram_size);
  PRINT_SECTION("-RAM2             ", total_ram2_size);
  PRINT_SECTION("--rw              ", RW_CONTENT_size);
  PRINT_SECTION("--RAM2_Section    ", ram2_section_size);
  PRINT_SECTION("--SLOWRAM_Section ", slowram_size);
  PRINT_SEP();

  /* SDRAM */
  PRINT_SECTION("SDRAM             ", (fb_size + asr_size));
  PRINT_SEP();
  PRINT_SECTION("-FB_Section       ", fb_size);
  PRINT_SECTION("-ASR_Section      ", asr_size);
}


void main_hooks_footprintDump(void)
{
  static bool     pr_footprint_static = false;
  static uint32_t max_heap_used = 0UL;
  struct mallinfo mi = __iar_dlmallinfo();

  /* Prints static info only once */
  if (!pr_footprint_static)
  {
    // for static info display (only once at the beginning),
    // set synchronous traces (because there are lots of displays)
    // then restore trace state
    bool traceAsynchronous = trace_setAsynchronous(false);
    pr_footprint_static = true;
    footprint_static();
    trace_setAsynchronous(traceAsynchronous);
  }

  /* Prints dynamic info only when used head increases*/
  if (mi.uordblks > max_heap_used)
  {
    PRINT_SEP();
    max_heap_used = mi.uordblks;
    PRINT_SECTION("Max Heap Used     ", max_heap_used);
    PRINT_SECTION("RAM with Heap Used", (total_ram_size_without_heap + max_heap_used));
    PRINT_SEP();
    PRINT_SECTION("Heap Free         ", (heap_size - max_heap_used));
    PRINT_SEP();
  }
}
