/**
  ******************************************************************************
  * @file    platform_dev.h
  * @author  MCD Application Team
  * @brief   Header with common defines & macros
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _PLATEFORM_DEF_H_
#define _PLATEFORM_DEF_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/
/* generic macro for memory placement */
#ifdef __ICCARM__
#define DEFAULT_SECTION ""
#else
#define DEFAULT_SECTION ".bss"
#endif

#if defined ( __ICCARM__ )
#define VAR_DECLARE_ALIGN4_AT_SECTION(type, var, section)   \
_Pragma("data_alignment=4") type var @ section
#elif defined(__GNUC__)
#define VAR_DECLARE_ALIGN4_AT_SECTION(type, var, section)   \
__attribute__((__section__(section)))                       \
type var __attribute__ ((aligned (4)))
#elif defined(__CC_ARM)
#define VAR_DECLARE_ALIGN4_AT_SECTION(type, var, section)   \
__attribute__((__section__(section)))                       \
__align(4) type var
#else
#error "unsupported compiler"
#endif

#define __QSPI_DATA __attribute__((section(".qspi_data"),aligned(4)))
/*#define __QSPI_DATA  If empty, logos are in internal flash */


#ifndef HAVE_STVS_FLOAT_T
typedef  float    stvs_float_t; /*!< Float overload @ingroup type*/
#define HAVE_STVS_FLOAT_T
#endif

#ifndef HAVE_STVS_DOUBLE_T
typedef  double    stvs_double_t; /*!< Double overload @ingroup type*/
#define HAVE_STVS_DOUBLE_T
#endif


#ifndef HAVE_CHAR_T
typedef   char char_t; /*!< Char overload @ingroup type*/
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

#ifndef TRUE
#define TRUE    (1) /*!< True overload @ingroup type*/
#define HAVE_TRUE
#endif

#ifndef FALSE
#define FALSE   (0) /*!< False overload @ingroup type*/
#define HAVE_FALSE
#endif


/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
#if  defined ( __GNUC__ ) && !defined   (__CC_ARM)
#ifndef __weak
#define __weak   __attribute__((weak))
#endif /* __weak */
#ifndef __packed
#define __packed __attribute__((__packed__))
#endif /* __packed */
#endif /* __GNUC__ */
/* Exported functions ------------------------------------------------------- */

#ifdef __cplusplus
}
#endif

#endif /*_PLATEFORM_DEF_H_*/
