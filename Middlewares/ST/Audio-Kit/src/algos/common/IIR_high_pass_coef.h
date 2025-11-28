#ifndef __IIR_HIGH_PASS_COEF_H
  #define __IIR_HIGH_PASS_COEF_H

  #include "biquad_typedef.h"
  #include "audio_chain_conf.h"

  #ifdef AC_SUPPORT_FS_8000
    extern const biquadFloat_t IIR_butterworth_DC_remove_fs8000_biquadFloat;
    extern const biquadInt32_t IIR_butterworth_DC_remove_fs8000_biquadInt32;

    #define IIR_BUTTERWORTH_DC_REMOVE_FS8000_BIQUAD_FLOAT &IIR_butterworth_DC_remove_fs8000_biquadFloat
    #define IIR_BUTTERWORTH_DC_REMOVE_FS8000_BIQUAD_INT32 &IIR_butterworth_DC_remove_fs8000_biquadInt32

    extern const biquadFloat_t IIR_chebyshev1_DC_remove_fs8000_biquadFloat;
    extern const biquadInt32_t IIR_chebyshev1_DC_remove_fs8000_biquadInt32;

    #define IIR_CHEBYSHEV1_DC_REMOVE_FS8000_BIQUAD_FLOAT &IIR_chebyshev1_DC_remove_fs8000_biquadFloat
    #define IIR_CHEBYSHEV1_DC_REMOVE_FS8000_BIQUAD_INT32 &IIR_chebyshev1_DC_remove_fs8000_biquadInt32

    extern const biquadFloat_t IIR_chebyshev2_DC_remove_fs8000_biquadFloat;
    extern const biquadInt32_t IIR_chebyshev2_DC_remove_fs8000_biquadInt32;

    #define IIR_CHEBYSHEV2_DC_REMOVE_FS8000_BIQUAD_FLOAT &IIR_chebyshev2_DC_remove_fs8000_biquadFloat
    #define IIR_CHEBYSHEV2_DC_REMOVE_FS8000_BIQUAD_INT32 &IIR_chebyshev2_DC_remove_fs8000_biquadInt32

    extern const biquadFloat_t IIR_elliptic_DC_remove_fs8000_biquadFloat;
    extern const biquadInt32_t IIR_elliptic_DC_remove_fs8000_biquadInt32;

    #define IIR_ELLIPTIC_DC_REMOVE_FS8000_BIQUAD_FLOAT &IIR_elliptic_DC_remove_fs8000_biquadFloat
    #define IIR_ELLIPTIC_DC_REMOVE_FS8000_BIQUAD_INT32 &IIR_elliptic_DC_remove_fs8000_biquadInt32

  #else /* AC_SUPPORT_FS_8000 */

    #define IIR_BUTTERWORTH_DC_REMOVE_FS8000_BIQUAD_FLOAT NULL
    #define IIR_BUTTERWORTH_DC_REMOVE_FS8000_BIQUAD_INT32 NULL
    #define IIR_CHEBYSHEV1_DC_REMOVE_FS8000_BIQUAD_FLOAT NULL
    #define IIR_CHEBYSHEV1_DC_REMOVE_FS8000_BIQUAD_INT32 NULL
    #define IIR_CHEBYSHEV2_DC_REMOVE_FS8000_BIQUAD_FLOAT NULL
    #define IIR_CHEBYSHEV2_DC_REMOVE_FS8000_BIQUAD_INT32 NULL
    #define IIR_ELLIPTIC_DC_REMOVE_FS8000_BIQUAD_FLOAT NULL
    #define IIR_ELLIPTIC_DC_REMOVE_FS8000_BIQUAD_INT32 NULL
  #endif /* AC_SUPPORT_FS_8000 */

  #ifdef AC_SUPPORT_FS_12000
    extern const biquadFloat_t IIR_butterworth_DC_remove_fs12000_biquadFloat;
    extern const biquadInt32_t IIR_butterworth_DC_remove_fs12000_biquadInt32;

    #define IIR_BUTTERWORTH_DC_REMOVE_FS12000_BIQUAD_FLOAT &IIR_butterworth_DC_remove_fs12000_biquadFloat
    #define IIR_BUTTERWORTH_DC_REMOVE_FS12000_BIQUAD_INT32 &IIR_butterworth_DC_remove_fs12000_biquadInt32

    extern const biquadFloat_t IIR_chebyshev1_DC_remove_fs12000_biquadFloat;
    extern const biquadInt32_t IIR_chebyshev1_DC_remove_fs12000_biquadInt32;

    #define IIR_CHEBYSHEV1_DC_REMOVE_FS12000_BIQUAD_FLOAT &IIR_chebyshev1_DC_remove_fs12000_biquadFloat
    #define IIR_CHEBYSHEV1_DC_REMOVE_FS12000_BIQUAD_INT32 &IIR_chebyshev1_DC_remove_fs12000_biquadInt32

    extern const biquadFloat_t IIR_chebyshev2_DC_remove_fs12000_biquadFloat;
    extern const biquadInt32_t IIR_chebyshev2_DC_remove_fs12000_biquadInt32;

    #define IIR_CHEBYSHEV2_DC_REMOVE_FS12000_BIQUAD_FLOAT &IIR_chebyshev2_DC_remove_fs12000_biquadFloat
    #define IIR_CHEBYSHEV2_DC_REMOVE_FS12000_BIQUAD_INT32 &IIR_chebyshev2_DC_remove_fs12000_biquadInt32

    extern const biquadFloat_t IIR_elliptic_DC_remove_fs12000_biquadFloat;
    extern const biquadInt32_t IIR_elliptic_DC_remove_fs12000_biquadInt32;

    #define IIR_ELLIPTIC_DC_REMOVE_FS12000_BIQUAD_FLOAT &IIR_elliptic_DC_remove_fs12000_biquadFloat
    #define IIR_ELLIPTIC_DC_REMOVE_FS12000_BIQUAD_INT32 &IIR_elliptic_DC_remove_fs12000_biquadInt32

  #else /* AC_SUPPORT_FS_12000 */

    #define IIR_BUTTERWORTH_DC_REMOVE_FS12000_BIQUAD_FLOAT NULL
    #define IIR_BUTTERWORTH_DC_REMOVE_FS12000_BIQUAD_INT32 NULL
    #define IIR_CHEBYSHEV1_DC_REMOVE_FS12000_BIQUAD_FLOAT NULL
    #define IIR_CHEBYSHEV1_DC_REMOVE_FS12000_BIQUAD_INT32 NULL
    #define IIR_CHEBYSHEV2_DC_REMOVE_FS12000_BIQUAD_FLOAT NULL
    #define IIR_CHEBYSHEV2_DC_REMOVE_FS12000_BIQUAD_INT32 NULL
    #define IIR_ELLIPTIC_DC_REMOVE_FS12000_BIQUAD_FLOAT NULL
    #define IIR_ELLIPTIC_DC_REMOVE_FS12000_BIQUAD_INT32 NULL
  #endif /* AC_SUPPORT_FS_12000 */

  #ifdef AC_SUPPORT_FS_16000
    extern const biquadFloat_t IIR_butterworth_DC_remove_fs16000_biquadFloat;
    extern const biquadInt32_t IIR_butterworth_DC_remove_fs16000_biquadInt32;

    #define IIR_BUTTERWORTH_DC_REMOVE_FS16000_BIQUAD_FLOAT &IIR_butterworth_DC_remove_fs16000_biquadFloat
    #define IIR_BUTTERWORTH_DC_REMOVE_FS16000_BIQUAD_INT32 &IIR_butterworth_DC_remove_fs16000_biquadInt32

    extern const biquadFloat_t IIR_chebyshev1_DC_remove_fs16000_biquadFloat;
    extern const biquadInt32_t IIR_chebyshev1_DC_remove_fs16000_biquadInt32;

    #define IIR_CHEBYSHEV1_DC_REMOVE_FS16000_BIQUAD_FLOAT &IIR_chebyshev1_DC_remove_fs16000_biquadFloat
    #define IIR_CHEBYSHEV1_DC_REMOVE_FS16000_BIQUAD_INT32 &IIR_chebyshev1_DC_remove_fs16000_biquadInt32

    extern const biquadFloat_t IIR_chebyshev2_DC_remove_fs16000_biquadFloat;
    extern const biquadInt32_t IIR_chebyshev2_DC_remove_fs16000_biquadInt32;

    #define IIR_CHEBYSHEV2_DC_REMOVE_FS16000_BIQUAD_FLOAT &IIR_chebyshev2_DC_remove_fs16000_biquadFloat
    #define IIR_CHEBYSHEV2_DC_REMOVE_FS16000_BIQUAD_INT32 &IIR_chebyshev2_DC_remove_fs16000_biquadInt32

    extern const biquadFloat_t IIR_elliptic_DC_remove_fs16000_biquadFloat;
    extern const biquadInt32_t IIR_elliptic_DC_remove_fs16000_biquadInt32;

    #define IIR_ELLIPTIC_DC_REMOVE_FS16000_BIQUAD_FLOAT &IIR_elliptic_DC_remove_fs16000_biquadFloat
    #define IIR_ELLIPTIC_DC_REMOVE_FS16000_BIQUAD_INT32 &IIR_elliptic_DC_remove_fs16000_biquadInt32

  #else /* AC_SUPPORT_FS_16000 */

    #define IIR_BUTTERWORTH_DC_REMOVE_FS16000_BIQUAD_FLOAT NULL
    #define IIR_BUTTERWORTH_DC_REMOVE_FS16000_BIQUAD_INT32 NULL
    #define IIR_CHEBYSHEV1_DC_REMOVE_FS16000_BIQUAD_FLOAT NULL
    #define IIR_CHEBYSHEV1_DC_REMOVE_FS16000_BIQUAD_INT32 NULL
    #define IIR_CHEBYSHEV2_DC_REMOVE_FS16000_BIQUAD_FLOAT NULL
    #define IIR_CHEBYSHEV2_DC_REMOVE_FS16000_BIQUAD_INT32 NULL
    #define IIR_ELLIPTIC_DC_REMOVE_FS16000_BIQUAD_FLOAT NULL
    #define IIR_ELLIPTIC_DC_REMOVE_FS16000_BIQUAD_INT32 NULL
  #endif /* AC_SUPPORT_FS_16000 */

  #ifdef AC_SUPPORT_FS_24000
    extern const biquadFloat_t IIR_butterworth_DC_remove_fs24000_biquadFloat;
    extern const biquadInt32_t IIR_butterworth_DC_remove_fs24000_biquadInt32;

    #define IIR_BUTTERWORTH_DC_REMOVE_FS24000_BIQUAD_FLOAT &IIR_butterworth_DC_remove_fs24000_biquadFloat
    #define IIR_BUTTERWORTH_DC_REMOVE_FS24000_BIQUAD_INT32 &IIR_butterworth_DC_remove_fs24000_biquadInt32

    extern const biquadFloat_t IIR_chebyshev1_DC_remove_fs24000_biquadFloat;
    extern const biquadInt32_t IIR_chebyshev1_DC_remove_fs24000_biquadInt32;

    #define IIR_CHEBYSHEV1_DC_REMOVE_FS24000_BIQUAD_FLOAT &IIR_chebyshev1_DC_remove_fs24000_biquadFloat
    #define IIR_CHEBYSHEV1_DC_REMOVE_FS24000_BIQUAD_INT32 &IIR_chebyshev1_DC_remove_fs24000_biquadInt32

    extern const biquadFloat_t IIR_chebyshev2_DC_remove_fs24000_biquadFloat;
    extern const biquadInt32_t IIR_chebyshev2_DC_remove_fs24000_biquadInt32;

    #define IIR_CHEBYSHEV2_DC_REMOVE_FS24000_BIQUAD_FLOAT &IIR_chebyshev2_DC_remove_fs24000_biquadFloat
    #define IIR_CHEBYSHEV2_DC_REMOVE_FS24000_BIQUAD_INT32 &IIR_chebyshev2_DC_remove_fs24000_biquadInt32

    extern const biquadFloat_t IIR_elliptic_DC_remove_fs24000_biquadFloat;
    extern const biquadInt32_t IIR_elliptic_DC_remove_fs24000_biquadInt32;

    #define IIR_ELLIPTIC_DC_REMOVE_FS24000_BIQUAD_FLOAT &IIR_elliptic_DC_remove_fs24000_biquadFloat
    #define IIR_ELLIPTIC_DC_REMOVE_FS24000_BIQUAD_INT32 &IIR_elliptic_DC_remove_fs24000_biquadInt32

  #else /* AC_SUPPORT_FS_24000 */

    #define IIR_BUTTERWORTH_DC_REMOVE_FS24000_BIQUAD_FLOAT NULL
    #define IIR_BUTTERWORTH_DC_REMOVE_FS24000_BIQUAD_INT32 NULL
    #define IIR_CHEBYSHEV1_DC_REMOVE_FS24000_BIQUAD_FLOAT NULL
    #define IIR_CHEBYSHEV1_DC_REMOVE_FS24000_BIQUAD_INT32 NULL
    #define IIR_CHEBYSHEV2_DC_REMOVE_FS24000_BIQUAD_FLOAT NULL
    #define IIR_CHEBYSHEV2_DC_REMOVE_FS24000_BIQUAD_INT32 NULL
    #define IIR_ELLIPTIC_DC_REMOVE_FS24000_BIQUAD_FLOAT NULL
    #define IIR_ELLIPTIC_DC_REMOVE_FS24000_BIQUAD_INT32 NULL
  #endif /* AC_SUPPORT_FS_24000 */

  #ifdef AC_SUPPORT_FS_32000
    extern const biquadFloat_t IIR_butterworth_DC_remove_fs32000_biquadFloat;
    extern const biquadInt32_t IIR_butterworth_DC_remove_fs32000_biquadInt32;

    #define IIR_BUTTERWORTH_DC_REMOVE_FS32000_BIQUAD_FLOAT &IIR_butterworth_DC_remove_fs32000_biquadFloat
    #define IIR_BUTTERWORTH_DC_REMOVE_FS32000_BIQUAD_INT32 &IIR_butterworth_DC_remove_fs32000_biquadInt32

    extern const biquadFloat_t IIR_chebyshev1_DC_remove_fs32000_biquadFloat;
    extern const biquadInt32_t IIR_chebyshev1_DC_remove_fs32000_biquadInt32;

    #define IIR_CHEBYSHEV1_DC_REMOVE_FS32000_BIQUAD_FLOAT &IIR_chebyshev1_DC_remove_fs32000_biquadFloat
    #define IIR_CHEBYSHEV1_DC_REMOVE_FS32000_BIQUAD_INT32 &IIR_chebyshev1_DC_remove_fs32000_biquadInt32

    extern const biquadFloat_t IIR_chebyshev2_DC_remove_fs32000_biquadFloat;
    extern const biquadInt32_t IIR_chebyshev2_DC_remove_fs32000_biquadInt32;

    #define IIR_CHEBYSHEV2_DC_REMOVE_FS32000_BIQUAD_FLOAT &IIR_chebyshev2_DC_remove_fs32000_biquadFloat
    #define IIR_CHEBYSHEV2_DC_REMOVE_FS32000_BIQUAD_INT32 &IIR_chebyshev2_DC_remove_fs32000_biquadInt32

    extern const biquadFloat_t IIR_elliptic_DC_remove_fs32000_biquadFloat;
    extern const biquadInt32_t IIR_elliptic_DC_remove_fs32000_biquadInt32;

    #define IIR_ELLIPTIC_DC_REMOVE_FS32000_BIQUAD_FLOAT &IIR_elliptic_DC_remove_fs32000_biquadFloat
    #define IIR_ELLIPTIC_DC_REMOVE_FS32000_BIQUAD_INT32 &IIR_elliptic_DC_remove_fs32000_biquadInt32

  #else /* AC_SUPPORT_FS_32000 */

    #define IIR_BUTTERWORTH_DC_REMOVE_FS32000_BIQUAD_FLOAT NULL
    #define IIR_BUTTERWORTH_DC_REMOVE_FS32000_BIQUAD_INT32 NULL
    #define IIR_CHEBYSHEV1_DC_REMOVE_FS32000_BIQUAD_FLOAT NULL
    #define IIR_CHEBYSHEV1_DC_REMOVE_FS32000_BIQUAD_INT32 NULL
    #define IIR_CHEBYSHEV2_DC_REMOVE_FS32000_BIQUAD_FLOAT NULL
    #define IIR_CHEBYSHEV2_DC_REMOVE_FS32000_BIQUAD_INT32 NULL
    #define IIR_ELLIPTIC_DC_REMOVE_FS32000_BIQUAD_FLOAT NULL
    #define IIR_ELLIPTIC_DC_REMOVE_FS32000_BIQUAD_INT32 NULL
  #endif /* AC_SUPPORT_FS_32000 */

  #ifdef AC_SUPPORT_FS_48000
    extern const biquadFloat_t IIR_butterworth_DC_remove_fs48000_biquadFloat;
    extern const biquadInt32_t IIR_butterworth_DC_remove_fs48000_biquadInt32;

    #define IIR_BUTTERWORTH_DC_REMOVE_FS48000_BIQUAD_FLOAT &IIR_butterworth_DC_remove_fs48000_biquadFloat
    #define IIR_BUTTERWORTH_DC_REMOVE_FS48000_BIQUAD_INT32 &IIR_butterworth_DC_remove_fs48000_biquadInt32

    extern const biquadFloat_t IIR_chebyshev1_DC_remove_fs48000_biquadFloat;
    extern const biquadInt32_t IIR_chebyshev1_DC_remove_fs48000_biquadInt32;

    #define IIR_CHEBYSHEV1_DC_REMOVE_FS48000_BIQUAD_FLOAT &IIR_chebyshev1_DC_remove_fs48000_biquadFloat
    #define IIR_CHEBYSHEV1_DC_REMOVE_FS48000_BIQUAD_INT32 &IIR_chebyshev1_DC_remove_fs48000_biquadInt32

    extern const biquadFloat_t IIR_chebyshev2_DC_remove_fs48000_biquadFloat;
    extern const biquadInt32_t IIR_chebyshev2_DC_remove_fs48000_biquadInt32;

    #define IIR_CHEBYSHEV2_DC_REMOVE_FS48000_BIQUAD_FLOAT &IIR_chebyshev2_DC_remove_fs48000_biquadFloat
    #define IIR_CHEBYSHEV2_DC_REMOVE_FS48000_BIQUAD_INT32 &IIR_chebyshev2_DC_remove_fs48000_biquadInt32

    extern const biquadFloat_t IIR_elliptic_DC_remove_fs48000_biquadFloat;
    extern const biquadInt32_t IIR_elliptic_DC_remove_fs48000_biquadInt32;

    #define IIR_ELLIPTIC_DC_REMOVE_FS48000_BIQUAD_FLOAT &IIR_elliptic_DC_remove_fs48000_biquadFloat
    #define IIR_ELLIPTIC_DC_REMOVE_FS48000_BIQUAD_INT32 &IIR_elliptic_DC_remove_fs48000_biquadInt32

  #else /* AC_SUPPORT_FS_48000 */

    #define IIR_BUTTERWORTH_DC_REMOVE_FS48000_BIQUAD_FLOAT NULL
    #define IIR_BUTTERWORTH_DC_REMOVE_FS48000_BIQUAD_INT32 NULL
    #define IIR_CHEBYSHEV1_DC_REMOVE_FS48000_BIQUAD_FLOAT NULL
    #define IIR_CHEBYSHEV1_DC_REMOVE_FS48000_BIQUAD_INT32 NULL
    #define IIR_CHEBYSHEV2_DC_REMOVE_FS48000_BIQUAD_FLOAT NULL
    #define IIR_CHEBYSHEV2_DC_REMOVE_FS48000_BIQUAD_INT32 NULL
    #define IIR_ELLIPTIC_DC_REMOVE_FS48000_BIQUAD_FLOAT NULL
    #define IIR_ELLIPTIC_DC_REMOVE_FS48000_BIQUAD_INT32 NULL
  #endif /* AC_SUPPORT_FS_48000 */

  #ifdef AC_SUPPORT_FS_96000
    extern const biquadFloat_t IIR_butterworth_DC_remove_fs96000_biquadFloat;
    extern const biquadInt32_t IIR_butterworth_DC_remove_fs96000_biquadInt32;

    #define IIR_BUTTERWORTH_DC_REMOVE_FS96000_BIQUAD_FLOAT &IIR_butterworth_DC_remove_fs96000_biquadFloat
    #define IIR_BUTTERWORTH_DC_REMOVE_FS96000_BIQUAD_INT32 &IIR_butterworth_DC_remove_fs96000_biquadInt32

    extern const biquadFloat_t IIR_chebyshev1_DC_remove_fs96000_biquadFloat;
    extern const biquadInt32_t IIR_chebyshev1_DC_remove_fs96000_biquadInt32;

    #define IIR_CHEBYSHEV1_DC_REMOVE_FS96000_BIQUAD_FLOAT &IIR_chebyshev1_DC_remove_fs96000_biquadFloat
    #define IIR_CHEBYSHEV1_DC_REMOVE_FS96000_BIQUAD_INT32 &IIR_chebyshev1_DC_remove_fs96000_biquadInt32

    extern const biquadFloat_t IIR_chebyshev2_DC_remove_fs96000_biquadFloat;
    extern const biquadInt32_t IIR_chebyshev2_DC_remove_fs96000_biquadInt32;

    #define IIR_CHEBYSHEV2_DC_REMOVE_FS96000_BIQUAD_FLOAT &IIR_chebyshev2_DC_remove_fs96000_biquadFloat
    #define IIR_CHEBYSHEV2_DC_REMOVE_FS96000_BIQUAD_INT32 &IIR_chebyshev2_DC_remove_fs96000_biquadInt32

    extern const biquadFloat_t IIR_elliptic_DC_remove_fs96000_biquadFloat;
    extern const biquadInt32_t IIR_elliptic_DC_remove_fs96000_biquadInt32;

    #define IIR_ELLIPTIC_DC_REMOVE_FS96000_BIQUAD_FLOAT &IIR_elliptic_DC_remove_fs96000_biquadFloat
    #define IIR_ELLIPTIC_DC_REMOVE_FS96000_BIQUAD_INT32 &IIR_elliptic_DC_remove_fs96000_biquadInt32

  #else /* AC_SUPPORT_FS_96000 */

    #define IIR_BUTTERWORTH_DC_REMOVE_FS96000_BIQUAD_FLOAT NULL
    #define IIR_BUTTERWORTH_DC_REMOVE_FS96000_BIQUAD_INT32 NULL
    #define IIR_CHEBYSHEV1_DC_REMOVE_FS96000_BIQUAD_FLOAT NULL
    #define IIR_CHEBYSHEV1_DC_REMOVE_FS96000_BIQUAD_INT32 NULL
    #define IIR_CHEBYSHEV2_DC_REMOVE_FS96000_BIQUAD_FLOAT NULL
    #define IIR_CHEBYSHEV2_DC_REMOVE_FS96000_BIQUAD_INT32 NULL
    #define IIR_ELLIPTIC_DC_REMOVE_FS96000_BIQUAD_FLOAT NULL
    #define IIR_ELLIPTIC_DC_REMOVE_FS96000_BIQUAD_INT32 NULL
  #endif /* AC_SUPPORT_FS_96000 */


#endif
extern const biquadFloat_t IIR_flat_biquadFloat; extern const biquadInt32_t IIR_flat_biquadInt32;