/**
******************************************************************************
* @file          st_base.h
* @author        MCD Application Team
* @brief         ST_Result SDK type definition
*******************************************************************************
* @attention
*
* Copyright (c) 2019(-2022) STMicroelectronics.
* All rights reserved.
*
* This software is licensed under terms that can be found in the LICENSE file
* in the root directory of this software component.
* If no LICENSE file comes with this software, it is provided AS-IS.
*
********************************************************************************
*/




#ifndef ST_BASE_H
#define ST_BASE_H

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdarg.h>
#include "st_base_conf.h"
/* Exported types ------------------------------------------------------------*/



/*!

Defines some groups for the auto-documentation Doxygen

@defgroup api livetune_db_find_instance_index_from_name api
@name api Describes ST_Result API functions
@defgroup type livetune_db_find_instance_index_from_name types
@name type Describes ST_Result types
@defgroup enum livetune_db_find_instance_index_from_name enums
@name enum Describes ST_Result enumerations
@defgroup macro livetune_db_find_instance_index_from_name macros
@name macro Describes ST_Result macros
*/



typedef void *ST_Handle; /*!< generic instance handle @ingroup type*/

#ifndef HAVE_CHAR_T
typedef char char_t; /*!< Char overload @ingroup type*/
#define HAVE_CHAR_T
#endif


#ifndef HAVE_LONG_T
typedef long long_t; /*!< Long overload @ingroup type*/
#define HAVE_LONG_T
#endif


#ifndef HAVE_ULONG_T
typedef unsigned long ulong_t; /*!< ULong overload @ingroup type*/
#define HAVE_ULONG_T
#endif



/*!

@brief  ST_Result errors
@ingroup enum
*/
typedef enum st_base_result
{
  /* Generic errors */
  ST_ERROR = 0 /*!< Error */
  ,
  ST_OK = 1 /*!< Success */
  ,
  ST_BUSY = 3 /*!< Busy*/
  ,
  ST_NOT_IMPL = 4 /*!< The function or a function used by the caller is not implemented */
  ,
  ST_HANDLED = 5 /*!< The function or a function used by the caller is not implemented */
  ,
  ST_END_RESULT
} ST_Result;

/*!

@brief Debug message level
@ingroup enum

*/

#define ST_TRACE_LVL_ALL (0x7FFFFFFFUL) /*!< All traces messages */

#define ST_TRACE_LVL_ADD_CR         (1UL << 31UL)
#define ST_TRACE_LVL_MUTE           (0UL) /*!< No Trace messages */
#define ST_TRACE_LVL_ERROR          (1UL << 0UL) /*!< Trace error   messages */
#define ST_TRACE_LVL_WARNING        (1UL << 1UL) /*!< Trace warning messages */
#define ST_TRACE_LVL_INFO           (1UL << 2UL) /*!< Trace info    messages */
#define ST_TRACE_LVL_DEBUG          (1UL << 3UL) /*!< Trace debug   messages */
#define ST_TRACE_LVL_VERBOSE        (1UL << 4UL) /*!< Trace in loops that could produce many debug messages */
#define ST_TRACE_LVL_JSON           (1UL << 5UL) /*!< Trace json    messages */
#define ST_TRACE_LVL_USER1          (1UL << 6UL) /*!< Trace lvl for extra  user customization */
#define ST_TRACE_LVL_USER2          (1UL << 7UL) /*!< Trace lvl for extra  user customization */
#define ST_TRACE_LVL_USER3          (1UL << 8UL) /*!< Trace lvl for extra  user customization */
#define ST_TRACE_LVL_USER4          (1UL << 9UL) /*!< Trace lvl for extra user customization  */
#define ST_TRACE_LVL_USER5          (1UL << 10UL) /*!< Trace lvl for extra user customization  */
#define ST_TRACE_LVL_USER6          (1UL << 11UL) /*!< Trace lvl for extra user customization */
#define ST_TRACE_LVL_USER7          (1UL << 12UL) /*!< Trace lvl for extra user customization */
#define ST_TRACE_LVL_COLORIZE       (1UL << 29UL) /*!< Enable colorization  */
#define ST_TRACE_LVL_JSON_FORMATTED (1UL << 30UL) /*!< format  json messages */



#ifndef ST_TRACE_LVL_DEFAULT
#define ST_TRACE_LVL_DEFAULT (ST_TRACE_LVL_COLORIZE | ST_TRACE_LVL_ERROR | ST_TRACE_LVL_WARNING | ST_TRACE_LVL_INFO) /*!< Default traces */
#endif

/*!

@brief Leds definitions
2 bits LSB = leds
bit 0  : Read
bit 1  : green
value >>2 = lighting delay
@ingroup enum
*/
typedef enum st_base_leds
{
  SYS_LED_RED = 0, /*!< Small led red */
  SYS_LED_GREEN,   /*!< Small led green */

  USER_LED1 = 0x20, /*!< Small led user 1 */
  USER_LED2,        /*!< Small led user 2 */
  USER_LED3,        /*!< Small led user 3 */
  USER_LED4,        /*!< Small led user 4 */
  USER_LED_MAX
} st_base_leds;




/*!

@brief Generic exception notification
2 bits LSB = leds
bit 0  : red
bit 1  : green
value >>2 = lighting delay
@ingroup enum
*/
typedef enum st_base_exception
{
  ST_SIGNAL_EXCEPTION_HARD_FAULT     = (0 << 2) + 3, /*!< red+green fast */
  ST_SIGNAL_EXCEPTION_ASSERT         = (3 << 2) + 1, /*!< red fast */
  ST_SIGNAL_EXCEPTION_MEM_CORRUPTION = (3 << 2) + 2, /*!< green slow */
  ST_SIGNAL_EXCEPTION_GENERAL_ERROR  = (5 << 2) + 1, /*!< red very slow */
} st_base_exception;

typedef void (*st_base_debug_level_cb)(uint32_t *level); /*!< Trace error callback overload */



/* Exported constants --------------------------------------------------------*/

#ifndef TRUE
#define TRUE (1) /*!< True overload @ingroup type*/
#define HAVE_TRUE
#endif

#ifndef FALSE
#define FALSE (0) /*!< False overload @ingroup type*/
#define HAVE_FALSE
#endif

/*!< NULL */
#define ST_NULL 0 /*!< null overload @ingroup macro*/
#ifndef __CSTAT__
#define ST_UNUSED(x) ((void)((x))) /*!< prevent unused warning @ingroup macro*/
#else
#define ST_UNUSED(x) /*!< prevent unused warning @ingroup macro*/
#endif


#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef MAX_INT
#define MAX_INT +2147483647
#endif

/* Exported macros ------------------------------------------------------------*/

#define ST_MAX(a, b) (((a) > (b)) ? (a) : (b))
#define ST_MIN(a, b) (((a) < (b)) ? (a) : (b))

#ifndef ST_MAKE_INT64
#define ST_MAKE_INT64(high, low) ((((uint64_t)(uint32_t)(high)) << 32) | ((uint64_t)(uint32_t)(low)))
#endif
#ifndef ST_LOW_INT64
#define ST_LOW_INT64(l64) ((uint32_t)(l64))
#endif
#ifndef ST_HIGH_INT64
#define ST_HIGH_INT64(l64) ((uint32_t)((l64) >> 32))
#endif
#ifndef ST_MAKE_INT32
#define ST_MAKE_INT32(high, low) ((((uint32_t)(uint16_t)(high)) << 16) | (uint32_t)(uint16_t)(low)))
#endif
#ifndef ST_LOW_INT32
#define ST_LOW_INT32(l32) ((uint16_t)(l32))
#endif
#ifndef ST_HIGH_INT32
#define ST_HIGH_INT32(l32) ((uint16_t)((l32) >> 16))
#endif


#define ST_IS_VOID_NULL_POINTER(p)  ((p) == 0U) /*!< simplify writing @ingroup macro*/
#define ST_IS_VOID_VALID_POINTER(p) ((p) != 0U) /*!< simplify writing @ingroup macro*/
#define ST_IS_NULL_POINTER(p)       ((p) == 0U) /*!< simplify writing @ingroup macro*/
#define ST_IS_VALID_POINTER(p)      ((p) != 0U) /*!< simplify writing @ingroup macro*/



#if defined(__IAR_SYSTEMS_ICC__) || defined(__GNUC__)
#define ST_ALIGN_START     _Pragma("pack(push,4)")
#define ST_ALIGN_START_2() _Pragma("pack(push,2)")
#define ST_ALIGN_STOP      _Pragma("pack(pop)")
#else
#define ST_ALIGN_START(nb)
#define ST_ALIGN_STOP
#warning "data strcut are not aligned"
#endif

#if defined(__GNU__)
#define PRI_UINT64_Val(value) ((unsigned long)(value >> 32)), ((unsigned long)value)
#define PRI_UINT64            "%ld%ld"
#else
#define PRI_UINT64_Val(value) (value)
#define PRI_UINT64            "%lld"
#endif




#if defined(__GNUC__)
#ifndef __weak
#define __weak __attribute__((weak))
#endif /* __weak */
#ifndef __packed
#define __packed __attribute__((packed, aligned(1)))
#endif /* __packed */
#endif /* __GNUC__ */



#if defined(__ICCARM__)
#define TOOLCHAIN "IAR"
#elif defined(__GNUC__)
#define TOOLCHAIN "GCC"
#elif defined(__CC_ARM)
#define TOOLCHAIN "Keil"
#else
#define TOOLCHAIN "Unknown"
#endif
/* Force Keil compiler to inline critical functions. */
#ifndef __ALWAYS_INLINE
#if defined(__GNUC__) || defined(__ICCARM__) /* GCC + IAR */
#define __ALWAYS_INLINE inline
#elif defined(__CC_ARM) /* Keil */
#define __ALWAYS_INLINE __attribute__((always_inline))
#else
#warning "__ALWAYS_INLINE undefined "
#define __ALWAYS_INLINE
#endif
#endif




#if defined(ST_USE_DEBUG)
#if !defined(ST_NO_ASSERT)
#define ST_ASSERT(a)                               \
    if ((int32_t)(a) == 0)                         \
    {                                              \
        st_base_assert(0, #a, __LINE__, __FILE__); \
    } /*!< Return an Assert if the condition is false, Remove the code in release mode @ingroup macro*/
#define ST_VERIFY(a)                               \
    if ((int32_t)(a) == 0)                         \
    {                                              \
        st_base_assert(0, #a, __LINE__, __FILE__); \
    } /*!< Return an Assert if the condition is false, Remove the code in release mode @ingroup macro*/
#else
#define ST_ASSERT(a)
#define ST_VERIFY(a) ((void)(a))
#endif
#define ST_TRACE_INFO(...)           st_base_trace((uint32_t)(ST_TRACE_LVL_INFO | ST_TRACE_LVL_ADD_CR), (NULL), 0, __VA_ARGS__) /*!< Print an info message @ingroup macro*/
#define ST_TRACE_ERROR(...)          st_base_trace((uint32_t)ST_TRACE_LVL_ERROR | ST_TRACE_LVL_ADD_CR, (__FILE__), (__LINE__), __VA_ARGS__) /*!< Print a error message @ingroup macro*/
#define ST_TRACE_WARNING(...)        st_base_trace((uint32_t)ST_TRACE_LVL_WARNING | ST_TRACE_LVL_ADD_CR, (__FILE__), (__LINE__), __VA_ARGS__) /*!< Print a warning  message @ingroup macro*/
#define ST_TRACE_TEST(...)           st_base_trace((uint32_t)ST_TRACE_LVL_TEST | ST_TRACE_LVL_ADD_CR, (NULL), 0, __VA_ARGS__) /*!< Print a test message @ingroup macro*/
#define ST_TRACE_JSON(...)           st_base_trace((uint32_t)ST_TRACE_LVL_JSON | ST_TRACE_LVL_ADD_CR, (NULL), 0, __VA_ARGS__) /*!< Print a json  message, the trace is removed in release mode  @ingroup macro*/
#define ST_PRINTF(lvl, ...)          st_base_printf((uint32_t)lvl, __VA_ARGS__) /*!< Just a printf ( without CR) at the end , the trace is removed in release mode @ingroup macro*/
#define ST_PRINT_STRING(lvl, string) st_base_string((lvl), (string)) /*!< Just a print string ( without string size limitation), the trace is removed in release mode  @ingroup macro*/

#ifndef ST_TRACE_MINIMUM
#define ST_TRACE_DEBUG(...)                        st_base_trace((uint32_t)ST_TRACE_LVL_DEBUG | ST_TRACE_LVL_ADD_CR, (__FILE__), (__LINE__), __VA_ARGS__) /*!< Print a debug message, the trace is removed in release mode @ingroup macro*/
#define ST_TRACE_VERBOSE(...)                      st_base_trace((uint32_t)ST_TRACE_LVL_VERBOSE | ST_TRACE_LVL_ADD_CR, (__FILE__), (__LINE__), __VA_ARGS__) /*!< Print a debug message, the trace is removed in release mode @ingroup macro*/
#define ST_TRACE_LEVEL(lvl, ...)                   st_base_trace((uint32_t)lvl, (__FILE__), (__LINE__), __VA_ARGS__) /*!< Print a debug message, the trace is removed in release mode @ingroup macro*/
#define ST_TRACE_USER(lvl, ...)                    st_base_trace((uint32_t)lvl | ST_TRACE_LVL_ADD_CR, NULL, 0, __VA_ARGS__) /*!< Print a user  message, the trace is removed in release mode  @ingroup macro*/
#define ST_TRACE_DUMP(lvl, title, ptr, size)       st_base_dump(lvl | ST_TRACE_LVL_ADD_CR, (title), (ptr), (size)) /*!< Dump a buffer on the console, the trace is removed in release mode  @ingroup macro*/
#define ST_TRACE_DUMP_ARRAY(lvl, title, ptr, size) st_base_array(lvl | ST_TRACE_LVL_ADD_CR, (title), (ptr), (size)) /*!< Dump a buffer on the console, the trace is removed in release mode  @ingroup macro*/
#endif
#else
#define ST_ASSERT(...) ((void)0)
#define ST_VERIFY(a)   ((void)(a))
#endif
#ifndef ST_TRACE_INFO
#define ST_TRACE_INFO(...) ((void)0)
#endif
#ifndef ST_TRACE_ERROR
#define ST_TRACE_ERROR(...)                        st_base_trace((uint32_t)ST_TRACE_LVL_ERROR | ST_TRACE_LVL_ADD_CR, (__FILE__), (__LINE__), __VA_ARGS__) /*!< Print a error message @ingroup macro*/
#endif
#ifndef ST_TRACE_WARNING
#define ST_TRACE_WARNING(...) ((void)0)
#endif
#ifndef ST_TRACE_TEST
#define ST_TRACE_TEST(...) ((void)0)
#endif
#ifndef ST_TRACE_DEBUG
#define ST_TRACE_DEBUG(...) ((void)0)
#endif
#ifndef ST_TRACE_VERBOSE
#define ST_TRACE_VERBOSE(...) ((void)0)
#endif


#ifndef ST_TRACE_JSON
#define ST_TRACE_JSON(...) ((void)0)
#endif


#ifndef ST_TRACE_LEVEL
#define ST_TRACE_LEVEL(...) ((void)0)
#endif
#ifndef ST_TRACE_USER
#define ST_TRACE_USER(lvl, ...) ((void)0)
#endif
#ifndef ST_TRACE_AFE
#define ST_TRACE_AFE(...) ((void)0)
#endif
#ifndef ST_PRINTF
#define ST_PRINTF(lvl, ...) ((void)0)
#endif
#ifndef ST_PRINT_STRING
#define ST_PRINT_STRING(lvl, string) ((void)0)
#endif
#ifndef ST_TRACE_DUMP
#define ST_TRACE_DUMP(lvl, title, ptr, size) ((void)0)
#endif
#ifndef ST_TRACE_DUMP_ARRAY
#define ST_TRACE_DUMP_ARRAY(lvl, title, ptr, size) ((void)0)
#endif


#ifndef ST_Signal_Exception
#define ST_Signal_Exception(level) st_base_signal_exception(level)
#endif


/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
ST_Result st_base_init(void);                                                                    /*!< @ingroup api */
ST_Result st_base_term(void);                                                                    /*!< @ingroup api */
void      st_base_trace(uint32_t level, const char_t *pFile, uint32_t line, ...);                /*!< @ingroup api */
void      st_base_dump(uint32_t level, const char_t *pTitle, void *pData, uint32_t size);        /*!< @ingroup api */
void      st_base_dump_array(uint32_t level, const char_t *pTitle, void *pData, uint32_t size);  /*!< @ingroup api */
void      st_base_printf(uint32_t level, const char_t *pFormat, ...);                            /*!< @ingroup api */
void      st_base_printf_args(uint32_t level, const char *lpszFormat, va_list args);             /*!< @ingroup api */
void      st_base_assert(uint32_t eval, const char_t *peval, uint32_t line, const char_t *file); /*!< @ingroup api */
uint32_t  st_base_set_debug_level(uint32_t level);                                               /*!< @ingroup api */
uint32_t  st_base_get_debug_level(void);                                                         /*!< @ingroup api */
ST_Result st_base_set_debug_level_cb(st_base_debug_level_cb set, st_base_debug_level_cb get);    /*!< @ingroup api */
void      st_base_signal_exception(st_base_exception level);                                     /*!< @ingroup api */
uint32_t  st_base_set_led_state(st_base_leds numLed, uint32_t state);                            /*!< @ingroup api */
void      st_base_generate_long_delay(uint64_t delay);                                           /*!< @ingroup api */
void      st_base_puts(uint32_t level, const char_t *pString);                                   /*!< @ingroup api */
void      st_base_string(uint32_t level, const char_t *pString);                                 /*!< @ingroup api */
void      st_base_trace_puts(uint32_t level, const char *pString);                               /*!< @ingroup api */
#ifdef __cplusplus
};
#endif

#endif

