/**
  ******************************************************************************
  * @file    traces2display.c
  * @author  MCD Application Team
  * @brief   basic traces mechanism over display
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2019(-2022) STMicroelectronics.
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
#include "traces_internals.h"

#ifdef TRACE_USE_DISPLAY

/* Global variables ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
typedef struct
{
  uint8_t *pLevelIdx;               // TRACE_LINE_MAX_WRAPPING_SIZE     elements
  uint8_t *pBuffer;                 // TRACE_LINE_MAX_WRAPPING_SIZE + 1 elements (+1 for end of string character)
} lineInfo_t;

/* Private defines -----------------------------------------------------------*/
#ifndef LCD_LOG_INTERLINE
  #define LCD_LOG_INTERLINE 50UL
#endif

#ifndef SCROLL_AREA_WIDTH
  #define SCROLL_AREA_WIDTH  600UL
#endif

#ifndef SCROLL_AREA_HEIGHT
  #define SCROLL_AREA_HEIGHT (600UL - Y_POS_HEADER)
#endif

#define TRACE_LINE_MAX_PIXEL_SIZE          ((uint32_t)(LCD_TXT_FONT_DEFAULT.Width) * DISPLAY_LINE_WRAPPING_SIZE)
#define TRACE_LINE_MAX_WRAPPING_SIZE       (TRACE_LINE_MAX_PIXEL_SIZE / (uint32_t)(Font8.Width))    // Font8 is the smallest font => it will give the biggest line size
#define TRACE_LINE_MAX_WRAPPING_SIZE_ALLOC ((17UL * DISPLAY_LINE_WRAPPING_SIZE) / 5UL)              // TRACE_LINE_MAX_WRAPPING_SIZE_ALLOC must be constant for static allocation, today LCD_TXT_FONT_DEFAULT.Width=17 and Font8.Width=5
#define LINE_INFO_SIZE                     (sizeof(lineInfo_t) + TRACE_LINE_MAX_WRAPPING_SIZE + TRACE_LINE_MAX_WRAPPING_SIZE + 1UL)
#define LINE_INFO_SIZE_ALLOC               (sizeof(lineInfo_t) + TRACE_LINE_MAX_WRAPPING_SIZE_ALLOC + TRACE_LINE_MAX_WRAPPING_SIZE_ALLOC + 1UL)

/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static uint32_t          s_scrollBuffer[((TRACE_MAX_SCROLL_LINE * LINE_INFO_SIZE_ALLOC) + 3UL) >> 2]; // uint32_t to insure 32 bits alignment
static lineInfo_t *const pScrollBuffer = (lineInfo_t *)s_scrollBuffer;                                // TRACE_MAX_SCROLL_LINE elements

static const struct
{
  sFONT   *font;
  uint32_t color;
}
s_lcdTxtFontColor[] =
{
  [TRACE_LVL_VERBOSE_BIT]     = {&LCD_TXT_FONT_INFO,    LCD_TXT_COLOR_INFO},
  [TRACE_LVL_DEBUG_BIT]       = {&LCD_TXT_FONT_INFO,    LCD_TXT_COLOR_INFO},
  [TRACE_LVL_LOG_BIT]         = {&LCD_TXT_FONT_DEFAULT, LCD_TXT_COLOR_DEFAULT},
  [TRACE_LVL_INFO_BIT]        = {&LCD_TXT_FONT_RESULT,  LCD_TXT_COLOR_RESULT},
  [TRACE_LVL_INFO2_BIT]       = {&LCD_TXT_FONT_RESULT,  LCD_TXT_COLOR_RESULT},
  [TRACE_LVL_WARNING_BIT]     = {&LCD_TXT_FONT_WARNING, LCD_TXT_COLOR_WARNING},
  [TRACE_LVL_ERROR_BIT]       = {&LCD_TXT_FONT_ERROR,   LCD_TXT_COLOR_ERROR},
  [TRACE_LVL_ERROR_FATAL_BIT] = {&LCD_TXT_FONT_ERROR,   LCD_TXT_COLOR_ERROR},
  [NB_TRACE_LVL]              = {&LCD_TXT_FONT_ERROR,   LCD_TXT_COLOR_ERROR}    // default value
};

/* Private function prototypes -----------------------------------------------*/
static void s_clearLine(uint32_t const line);
static void s_traceClear(void);
static void s_traceLine(lineInfo_t const *const pLineEntry, uint32_t const line, uint32_t const column);
static void s_traceScrollingDisplay(traceLogInfo_t const *const pTraceLogInfo);

/* Functions Definition ------------------------------------------------------*/

/**
  * @brief  Send trace to display.
  * @param  log info table
  * @retval None
  */
void trace2display(traceLogInfo_t const *const pTraceLogInfo)
{
  static bool firstTime = true;

  TRACE_DISABLE_IRQ();
  if (firstTime)
  {
    uint8_t *pByte = (uint8_t *)s_scrollBuffer;

    TRACE_ASSERT(LINE_INFO_SIZE == LINE_INFO_SIZE_ALLOC);
    memset(s_scrollBuffer, 0, sizeof(s_scrollBuffer));
    pByte += TRACE_MAX_SCROLL_LINE * sizeof(lineInfo_t);
    for (uint16_t i = 0U; i < TRACE_MAX_SCROLL_LINE; i++)
    {
      pScrollBuffer[i].pLevelIdx = pByte;
      pByte                     += TRACE_LINE_MAX_WRAPPING_SIZE;
      pScrollBuffer[i].pBuffer   = pByte;
      pByte                     += TRACE_LINE_MAX_WRAPPING_SIZE + 1U;  // +1 for end of string
    }
    firstTime = false;
  }
  TRACE_ENABLE_IRQ();

  if (LCDIsInitDone())
  {
    s_traceScrollingDisplay(pTraceLogInfo);
  }
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////// static routines ///////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
  * @brief  Clear display lines.
  * @param  line
  * @retval None
  */
static void s_clearLine(uint32_t const line)
{
  static uint8_t clearString[DISPLAY_LINE_WRAPPING_SIZE + 1U];
  static bool    firstTime = true;

  // clear line (display spaces)
  TRACE_DISABLE_IRQ();
  if (firstTime)
  {
    firstTime = false;
    memset(clearString, (int)' ', DISPLAY_LINE_WRAPPING_SIZE);
    clearString[DISPLAY_LINE_WRAPPING_SIZE] = 0U;   // end of string
  }
  TRACE_ENABLE_IRQ();

  UTIL_LCD_SetTextColor(LCD_TXT_COLOR_DEFAULT);
  UTIL_LCD_SetFont(&LCD_TXT_FONT_DEFAULT);
  UTIL_LCD_DisplayStringAt(0UL, Y_POS_HEADER + (LCD_LOG_INTERLINE * (line + 1UL)), clearString, LEFT_MODE);
}


/**
  * @brief  Clear all trace in display.
  * @param  None
  * @retval None
  */
static void s_traceClear(void)
{
  // Clear whole text area: 2 choices to clear scroll area (#if 0 or #if 1)
  #if 1
  UTIL_LCD_SetTextColor(UTIL_LCD_GetBackColor());
  UTIL_LCD_FillRect(0U, Y_POS_HEADER + LCD_LOG_INTERLINE, SCROLL_AREA_WIDTH, SCROLL_AREA_HEIGHT, UTIL_LCD_COLOR_WHITE);
  #else
  for (uint32_t line = 0UL; line < TRACE_MAX_SCROLL_LINE; line++)
  {
    s_clearLine(line);
  }
  #endif
}


/**
  * @brief  print line on display.
  * @param  line info
  * @param  line
  * @param  column
  * @retval None
  */
static void s_traceLine(lineInfo_t const *const pLineEntry, uint32_t const line, uint32_t const column)
{
  uint8_t  levelIdx;
  uint32_t xPos, i, j, n;

  if (column == 0UL)
  {
    s_clearLine(line);
  }

  // compute xPos depending on column
  xPos = 0UL;
  for (i = 0UL; i < column; i++)
  {
    xPos += s_lcdTxtFontColor[pLineEntry->pLevelIdx[i]].font->Width;
  }

  // display string
  levelIdx = pLineEntry->pLevelIdx[column];
  for (i = column; pLineEntry->pBuffer[i] != 0U; i = j)
  {
    // search for characters with the same levelIdx (same font & color)
    n = 0UL;
    for (j = i; pLineEntry->pBuffer[j] != 0U; j++)
    {
      if (pLineEntry->pLevelIdx[j] != levelIdx)
      {
        break;
      }
      n++;
    }

    if (n > 0UL)
    {
      // display characters with the same levelIdx (same font & color)
      uint8_t *const pString = &pLineEntry->pBuffer[i];
      uint8_t  const charSav = pString[n];

      pString[n] = 0U;        // end of string
      UTIL_LCD_SetFont(s_lcdTxtFontColor[levelIdx].font);
      UTIL_LCD_SetTextColor(s_lcdTxtFontColor[levelIdx].color);
      UTIL_LCD_DisplayStringAt(xPos, Y_POS_HEADER + (LCD_LOG_INTERLINE * (line + 1UL)), pString, LEFT_MODE);
      pString[n] = charSav;   // restore cleared character for end of string

      if (charSav != 0U)
      {
        // it is not the end of line buffer => update xPos and get next levelIdx (font & color)
        xPos    += n * s_lcdTxtFontColor[levelIdx].font->Width;
        levelIdx = pLineEntry->pLevelIdx[j];
      }
    }
  }
}


/**
  * @brief  manages scrolling of traces on display.
  * @param  trace log table
  * @retval None
  */
static void s_traceScrollingDisplay(traceLogInfo_t const *const pTraceLogInfo)
{
  int const levelIdx = trace_getLevelIdx(pTraceLogInfo->level); /* -1 if level==0, i.e. no trace */

  if (levelIdx >= 0)
  {
    static uint32_t line       = 0UL;
    static uint32_t lineOffset = 0UL; // line offset in pScrollBuffer circular buffer
    static uint32_t column     = 0UL;
    static uint32_t xPos       = 0UL;
    uint32_t const  fontWidth  = s_lcdTxtFontColor[levelIdx].font->Width;
    char     const *pSrc;
    lineInfo_t     *pLineEntry;
    char            c;
    uint32_t        newColumn, endOfWordPos;
    uint8_t         endOfLine, endOfString;

    // Add new buffer in the scroll buffer
    pSrc         = pTraceLogInfo->buffer;
    endOfString  = 0U;
    endOfWordPos = column;
    while (endOfString == 0U)
    {
      pLineEntry = &pScrollBuffer[(line + lineOffset) % TRACE_MAX_SCROLL_LINE];
      newColumn  = column;
      endOfLine  = 0U;
      while (endOfLine == 0U)
      {
        c = *pSrc++;
        switch (c)
        {
          case '\0':
            endOfString = 1U;
            break;

          case '\n':
            endOfLine = 1U;
            if (*pSrc == '\r')
            {
              pSrc++;
            }
            if (*pSrc == '\0')
            {
              endOfString = 1U;
            }
            break;

          case '\r':
            endOfLine = 1U;
            if (*pSrc == '\n')
            {
              pSrc++;
            }
            if (*pSrc == '\0')
            {
              endOfString = 1U;
            }
            break;

          default:
            // end of word ?
            switch (c)
            {
              case ' ':
              case '<':
              case '(':
              case '[':
              case '{':
                endOfWordPos = newColumn;
                break;

              case ',':
              case ';':
              case '.':
              case ':':
              case '!':
              case '+':
              case '-':
              case '*':
              case '/':
              case '=':
              case '>':
              case ')':
              case ']':
              case '}':
              case '&':
              case '#':
              case '%':
                endOfWordPos = newColumn + 1UL;
                break;

              default:
                break;
            }

            // add c character in pLineEntry
            pLineEntry->pBuffer  [newColumn] = (uint8_t)c;
            pLineEntry->pLevelIdx[newColumn] = (uint8_t)levelIdx;
            xPos                            += fontWidth;
            newColumn++;
            if ((xPos + fontWidth) > TRACE_LINE_MAX_PIXEL_SIZE)
            {
              // line too big with next character => end of line
              if (endOfWordPos > 0UL)
              {
                // rewind to last end of word
                pSrc     -= (newColumn - endOfWordPos);
                newColumn = endOfWordPos;
                while (*pSrc == ' ')
                {
                  // while next character is a space, skip it
                  pSrc++;
                }
              }
              endOfLine = 1U;
            }
            break;
        }
      }

      if (column == 0UL)
      {
        if (line == 0UL)
        {
          // Clear whole text area
          s_traceClear();
        }
        else if (line == (TRACE_MAX_SCROLL_LINE - 1UL))
        {
          // last line => scroll up => redraw all scrooled lines
          for (uint32_t pos = 0U; pos < (TRACE_MAX_SCROLL_LINE - 1UL); pos++)
          {
            s_traceLine(&pScrollBuffer[(pos + lineOffset) % TRACE_MAX_SCROLL_LINE], pos, 0UL);
          }
        }
        else
        {
          // do nothing (MISRA)
        }
      }

      // Draw new data
      pLineEntry->pBuffer[newColumn] = 0U;  // end of string
      s_traceLine(pLineEntry, line, column);
      column = newColumn;

      if (endOfLine)
      {
        if (line < (TRACE_MAX_SCROLL_LINE - 1UL))
        {
          line++;
        }
        else
        {
          // screen if full: increment lineOffset (circularly) instead of incrementing line
          // which will remain equal to (TRACE_MAX_SCROLL_LINE - 1UL), i.e. last line of screen
          lineOffset++;
          if (lineOffset == TRACE_MAX_SCROLL_LINE)
          {
            lineOffset = 0UL;
          }
        }
        column       = 0UL;
        endOfWordPos = 0UL;
        xPos         = 0UL;
      }
    }

    UTIL_LCD_SetTextColor(LCD_TXT_COLOR_DEFAULT);
    UTIL_LCD_SetFont(&LCD_TXT_FONT_DEFAULT);
  }
}

#endif //TRACE_USE_DISPLAY
