#ifndef __FIR_LOWPASS_6000_COEF_H
#define __FIR_LOWPASS_6000_COEF_H

#include "fir_config.h"



#ifdef AC_SUPPORT_FS_16000
  extern const firCoeffFloat_t FIR_ParksMcClellan_fs16000_firFloat;
  extern const firCoeffInt32_t FIR_ParksMcClellan_fs16000_firInt32;
  extern const firCoeffInt16_t FIR_ParksMcClellan_fs16000_firInt16;
  #define FIR_PARKSMCCLELLAN_FS16000_FIR_FLOAT &FIR_ParksMcClellan_fs16000_firFloat
  #define FIR_PARKSMCCLELLAN_FS16000_FIR_INT32 &FIR_ParksMcClellan_fs16000_firInt32
  #define FIR_PARKSMCCLELLAN_FS16000_FIR_INT16 &FIR_ParksMcClellan_fs16000_firInt16

  extern const firCoeffFloat_t FIR_KaiserWindow_fs16000_firFloat;
  extern const firCoeffInt32_t FIR_KaiserWindow_fs16000_firInt32;
  extern const firCoeffInt16_t FIR_KaiserWindow_fs16000_firInt16;
  #define FIR_KAISERWINDOW_FS16000_FIR_FLOAT &FIR_KaiserWindow_fs16000_firFloat
  #define FIR_KAISERWINDOW_FS16000_FIR_INT32 &FIR_KaiserWindow_fs16000_firInt32
  #define FIR_KAISERWINDOW_FS16000_FIR_INT16 &FIR_KaiserWindow_fs16000_firInt16

#else /* AC_SUPPORT_FS_16000 */

  #define FIR_PARKSMCCLELLAN_FS16000_FIR_FLOAT NULL
  #define FIR_PARKSMCCLELLAN_FS16000_FIR_INT32 NULL
  #define FIR_PARKSMCCLELLAN_FS16000_FIR_INT16 NULL

  #define FIR_KAISERWINDOW_FS16000_FIR_FLOAT NULL
  #define FIR_KAISERWINDOW_FS16000_FIR_INT32 NULL
  #define FIR_KAISERWINDOW_FS16000_FIR_INT16 NULL

#endif /* AC_SUPPORT_FS_16000 */


#endif
