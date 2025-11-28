#ifndef __FIR_NOISE_BANDPASS_COEF_H
#define __FIR_NOISE_BANDPASS_COEF_H

#include "fir_config.h"



#ifdef AC_SUPPORT_FS_8000
  extern const firCoeffFloat_t FIR_ParksMcClellan_noise_fs8000_firFloat;
  extern const firCoeffInt32_t FIR_ParksMcClellan_noise_fs8000_firInt32;
  extern const firCoeffInt16_t FIR_ParksMcClellan_noise_fs8000_firInt16;
  #define FIR_PARKSMCCLELLAN_NOISE_FS8000_FIR_FLOAT &FIR_ParksMcClellan_noise_fs8000_firFloat
  #define FIR_PARKSMCCLELLAN_NOISE_FS8000_FIR_INT32 &FIR_ParksMcClellan_noise_fs8000_firInt32
  #define FIR_PARKSMCCLELLAN_NOISE_FS8000_FIR_INT16 &FIR_ParksMcClellan_noise_fs8000_firInt16

  extern const firCoeffFloat_t FIR_KaiserWindow_noise_fs8000_firFloat;
  extern const firCoeffInt32_t FIR_KaiserWindow_noise_fs8000_firInt32;
  extern const firCoeffInt16_t FIR_KaiserWindow_noise_fs8000_firInt16;
  #define FIR_KAISERWINDOW_NOISE_FS8000_FIR_FLOAT &FIR_KaiserWindow_noise_fs8000_firFloat
  #define FIR_KAISERWINDOW_NOISE_FS8000_FIR_INT32 &FIR_KaiserWindow_noise_fs8000_firInt32
  #define FIR_KAISERWINDOW_NOISE_FS8000_FIR_INT16 &FIR_KaiserWindow_noise_fs8000_firInt16

#else /* AC_SUPPORT_FS_8000 */

  #define FIR_PARKSMCCLELLAN_NOISE_FS8000_FIR_FLOAT NULL
  #define FIR_PARKSMCCLELLAN_NOISE_FS8000_FIR_INT32 NULL
  #define FIR_PARKSMCCLELLAN_NOISE_FS8000_FIR_INT16 NULL

  #define FIR_KAISERWINDOW_NOISE_FS8000_FIR_FLOAT NULL
  #define FIR_KAISERWINDOW_NOISE_FS8000_FIR_INT32 NULL
  #define FIR_KAISERWINDOW_NOISE_FS8000_FIR_INT16 NULL

#endif /* AC_SUPPORT_FS_8000 */

#ifdef AC_SUPPORT_FS_16000
  extern const firCoeffFloat_t FIR_ParksMcClellan_noise_fs16000_firFloat;
  extern const firCoeffInt32_t FIR_ParksMcClellan_noise_fs16000_firInt32;
  extern const firCoeffInt16_t FIR_ParksMcClellan_noise_fs16000_firInt16;
  #define FIR_PARKSMCCLELLAN_NOISE_FS16000_FIR_FLOAT &FIR_ParksMcClellan_noise_fs16000_firFloat
  #define FIR_PARKSMCCLELLAN_NOISE_FS16000_FIR_INT32 &FIR_ParksMcClellan_noise_fs16000_firInt32
  #define FIR_PARKSMCCLELLAN_NOISE_FS16000_FIR_INT16 &FIR_ParksMcClellan_noise_fs16000_firInt16

  extern const firCoeffFloat_t FIR_KaiserWindow_noise_fs16000_firFloat;
  extern const firCoeffInt32_t FIR_KaiserWindow_noise_fs16000_firInt32;
  extern const firCoeffInt16_t FIR_KaiserWindow_noise_fs16000_firInt16;
  #define FIR_KAISERWINDOW_NOISE_FS16000_FIR_FLOAT &FIR_KaiserWindow_noise_fs16000_firFloat
  #define FIR_KAISERWINDOW_NOISE_FS16000_FIR_INT32 &FIR_KaiserWindow_noise_fs16000_firInt32
  #define FIR_KAISERWINDOW_NOISE_FS16000_FIR_INT16 &FIR_KaiserWindow_noise_fs16000_firInt16

#else /* AC_SUPPORT_FS_16000 */

  #define FIR_PARKSMCCLELLAN_NOISE_FS16000_FIR_FLOAT NULL
  #define FIR_PARKSMCCLELLAN_NOISE_FS16000_FIR_INT32 NULL
  #define FIR_PARKSMCCLELLAN_NOISE_FS16000_FIR_INT16 NULL

  #define FIR_KAISERWINDOW_NOISE_FS16000_FIR_FLOAT NULL
  #define FIR_KAISERWINDOW_NOISE_FS16000_FIR_INT32 NULL
  #define FIR_KAISERWINDOW_NOISE_FS16000_FIR_INT16 NULL

#endif /* AC_SUPPORT_FS_16000 */

#ifdef AC_SUPPORT_FS_48000
  extern const firCoeffFloat_t FIR_ParksMcClellan_noise_fs48000_firFloat;
  extern const firCoeffInt32_t FIR_ParksMcClellan_noise_fs48000_firInt32;
  extern const firCoeffInt16_t FIR_ParksMcClellan_noise_fs48000_firInt16;
  #define FIR_PARKSMCCLELLAN_NOISE_FS48000_FIR_FLOAT &FIR_ParksMcClellan_noise_fs48000_firFloat
  #define FIR_PARKSMCCLELLAN_NOISE_FS48000_FIR_INT32 &FIR_ParksMcClellan_noise_fs48000_firInt32
  #define FIR_PARKSMCCLELLAN_NOISE_FS48000_FIR_INT16 &FIR_ParksMcClellan_noise_fs48000_firInt16

  extern const firCoeffFloat_t FIR_KaiserWindow_noise_fs48000_firFloat;
  extern const firCoeffInt32_t FIR_KaiserWindow_noise_fs48000_firInt32;
  extern const firCoeffInt16_t FIR_KaiserWindow_noise_fs48000_firInt16;
  #define FIR_KAISERWINDOW_NOISE_FS48000_FIR_FLOAT &FIR_KaiserWindow_noise_fs48000_firFloat
  #define FIR_KAISERWINDOW_NOISE_FS48000_FIR_INT32 &FIR_KaiserWindow_noise_fs48000_firInt32
  #define FIR_KAISERWINDOW_NOISE_FS48000_FIR_INT16 &FIR_KaiserWindow_noise_fs48000_firInt16

#else /* AC_SUPPORT_FS_48000 */

  #define FIR_PARKSMCCLELLAN_NOISE_FS48000_FIR_FLOAT NULL
  #define FIR_PARKSMCCLELLAN_NOISE_FS48000_FIR_INT32 NULL
  #define FIR_PARKSMCCLELLAN_NOISE_FS48000_FIR_INT16 NULL

  #define FIR_KAISERWINDOW_NOISE_FS48000_FIR_FLOAT NULL
  #define FIR_KAISERWINDOW_NOISE_FS48000_FIR_INT32 NULL
  #define FIR_KAISERWINDOW_NOISE_FS48000_FIR_INT16 NULL

#endif /* AC_SUPPORT_FS_48000 */

#ifdef AC_SUPPORT_FS_96000
  extern const firCoeffFloat_t FIR_ParksMcClellan_noise_fs96000_firFloat;
  extern const firCoeffInt32_t FIR_ParksMcClellan_noise_fs96000_firInt32;
  extern const firCoeffInt16_t FIR_ParksMcClellan_noise_fs96000_firInt16;
  #define FIR_PARKSMCCLELLAN_NOISE_FS96000_FIR_FLOAT &FIR_ParksMcClellan_noise_fs96000_firFloat
  #define FIR_PARKSMCCLELLAN_NOISE_FS96000_FIR_INT32 &FIR_ParksMcClellan_noise_fs96000_firInt32
  #define FIR_PARKSMCCLELLAN_NOISE_FS96000_FIR_INT16 &FIR_ParksMcClellan_noise_fs96000_firInt16

  extern const firCoeffFloat_t FIR_KaiserWindow_noise_fs96000_firFloat;
  extern const firCoeffInt32_t FIR_KaiserWindow_noise_fs96000_firInt32;
  extern const firCoeffInt16_t FIR_KaiserWindow_noise_fs96000_firInt16;
  #define FIR_KAISERWINDOW_NOISE_FS96000_FIR_FLOAT &FIR_KaiserWindow_noise_fs96000_firFloat
  #define FIR_KAISERWINDOW_NOISE_FS96000_FIR_INT32 &FIR_KaiserWindow_noise_fs96000_firInt32
  #define FIR_KAISERWINDOW_NOISE_FS96000_FIR_INT16 &FIR_KaiserWindow_noise_fs96000_firInt16

#else /* AC_SUPPORT_FS_96000 */

  #define FIR_PARKSMCCLELLAN_NOISE_FS96000_FIR_FLOAT NULL
  #define FIR_PARKSMCCLELLAN_NOISE_FS96000_FIR_INT32 NULL
  #define FIR_PARKSMCCLELLAN_NOISE_FS96000_FIR_INT16 NULL

  #define FIR_KAISERWINDOW_NOISE_FS96000_FIR_FLOAT NULL
  #define FIR_KAISERWINDOW_NOISE_FS96000_FIR_INT32 NULL
  #define FIR_KAISERWINDOW_NOISE_FS96000_FIR_INT16 NULL

#endif /* AC_SUPPORT_FS_96000 */


#endif
