#include <stdlib.h> // for NULL definition
#include "common/IIR_noise_bandpass_coef.h"

#ifdef AC_SUPPORT_FS_8000
// 8000 Hz band-pass IIR
//     first  stop frequency :   1500 Hz
//     first  pass frequency :   2000 Hz
//     second pass frequency :   3500 Hz
//     second stop frequency :   3999 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_butterworth_noise_bandpass_fs8000_BiquadCellFloat[] =
{
  {+0.09439430, -0.18878860, +0.09439430, +0.76031576, +0.13789294},
  {+0.17493300, -0.34986600, +0.17493300, +0.37954689, +0.07927888},
  {+0.21921885, -0.43843770, +0.21921885, +0.24218076, +0.11905617},
  {+0.26107651, -0.52215301, +0.26107651, +0.13948032, +0.18378634},
  {+1.72744645, +0.00000000, -1.72744645, +0.05608550, +0.26919087},
  {+0.92937693, +1.85875385, +0.92937693, +1.14263656, +0.34809408},
  {+0.31619742, -0.63239485, +0.31619742, -0.01396948, +0.37603268},
  {+0.77522714, +1.55045428, +0.77522714, +1.28476334, +0.45291528},
  {+0.33467707, -0.66935413, +0.33467707, -0.07322309, +0.50792869},
  {+0.74404923, +1.48809846, +0.74404923, +1.39576283, +0.54310362},
  {+0.74527566, +1.49055132, +0.74527566, +1.49159889, +0.62637210},
  {+0.35036508, -0.70073015, +0.35036508, -0.12223026, +0.67137459},
  {+0.76604457, +1.53208915, +0.76604457, +1.57921180, +0.70660575},
  {+0.77872395, +1.55744791, +0.77872395, +1.66265612, +0.78648554},
  {+0.78153571, +1.56307142, +0.78153571, +1.74484459, +0.86831880},
  {+0.36759434, -0.73518869, +0.36759434, -0.15975673, +0.87676358},
  {+0.91114010, +1.82228021, +0.91114010, +1.82828079, +0.95446770}
};

const biquadFloat_t IIR_butterworth_noise_bandpass_fs8000_biquadFloat =
{
  sizeof(IIR_butterworth_noise_bandpass_fs8000_BiquadCellFloat) / sizeof(IIR_butterworth_noise_bandpass_fs8000_BiquadCellFloat[0]), IIR_butterworth_noise_bandpass_fs8000_BiquadCellFloat
};

static const biquadCellInt32_t IIR_butterworth_noise_bandpass_fs8000_BiquadCellInt32[] =
{
  {(int32_t)0x0C151CCBUL, (int32_t)0xE7D5C66BUL, (int32_t)0x0C151CCBUL, (int32_t)0x30A9036CUL, (int32_t)0x11A679DAUL},
  {(int32_t)0x1664345DUL, (int32_t)0xD3379746UL, (int32_t)0x1664345DUL, (int32_t)0x184A7F0BUL, (int32_t)0x0A25CF77UL},
  {(int32_t)0x1C0F5D03UL, (int32_t)0xC7E145FBUL, (int32_t)0x1C0F5D03UL, (int32_t)0x0F7FE3C0UL, (int32_t)0x0F3D3B82UL},
  {(int32_t)0x216AF478UL, (int32_t)0xBD2A1711UL, (int32_t)0x216AF478UL, (int32_t)0x08ED3ED8UL, (int32_t)0x17864F90UL},
  {(int32_t)0x6E8E7B8FUL, (int32_t)0x00000000UL, (int32_t)0x91718471UL, (int32_t)0x0396E7A8UL, (int32_t)0x2274D8AFUL},
  {(int32_t)0x3B7AE95DUL, (int32_t)0x76F5D2BAUL, (int32_t)0x3B7AE95DUL, (int32_t)0x4920F51CUL, (int32_t)0x2C8E58CAUL},
  {(int32_t)0x2879283FUL, (int32_t)0xAF0DAF82UL, (int32_t)0x2879283FUL, (int32_t)0xFF1B1FBDUL, (int32_t)0x3021D6BBUL},
  {(int32_t)0x319D524CUL, (int32_t)0x633AA498UL, (int32_t)0x319D524CUL, (int32_t)0x52399009UL, (int32_t)0x39F920C0UL},
  {(int32_t)0x2AD6B2B5UL, (int32_t)0xAA529A97UL, (int32_t)0x2AD6B2B5UL, (int32_t)0xFB505016UL, (int32_t)0x4103CEA6UL},
  {(int32_t)0x2F9E80A8UL, (int32_t)0x5F3D0150UL, (int32_t)0x2F9E80A8UL, (int32_t)0x59542D9EUL, (int32_t)0x45846B68UL},
  {(int32_t)0x2FB298AFUL, (int32_t)0x5F65315DUL, (int32_t)0x2FB298AFUL, (int32_t)0x5F765B31UL, (int32_t)0x502CF605UL},
  {(int32_t)0x2CD8C34AUL, (int32_t)0xA64E796BUL, (int32_t)0x2CD8C34AUL, (int32_t)0xF82D611EUL, (int32_t)0x55EF9A3FUL},
  {(int32_t)0x3106DFD3UL, (int32_t)0x620DBFA5UL, (int32_t)0x3106DFD3UL, (int32_t)0x6511CE5EUL, (int32_t)0x5A720EA2UL},
  {(int32_t)0x31D69CFEUL, (int32_t)0x63AD39FCUL, (int32_t)0x31D69CFEUL, (int32_t)0x6A68F533UL, (int32_t)0x64AB8EDAUL},
  {(int32_t)0x3204AE59UL, (int32_t)0x64095CB3UL, (int32_t)0x3204AE59UL, (int32_t)0x6FAB88A1UL, (int32_t)0x6F251212UL},
  {(int32_t)0x2F0D54DAUL, (int32_t)0xA1E5564BUL, (int32_t)0x2F0D54DAUL, (int32_t)0xF5C68BB8UL, (int32_t)0x7039CA04UL},
  {(int32_t)0x3A501E96UL, (int32_t)0x74A03D2BUL, (int32_t)0x3A501E96UL, (int32_t)0x75028D6CUL, (int32_t)0x7A2BFF67UL}
};

const biquadInt32_t IIR_butterworth_noise_bandpass_fs8000_biquadInt32 =
{
  (int32_t)0x40000000UL, 10, sizeof(IIR_butterworth_noise_bandpass_fs8000_BiquadCellInt32) / sizeof(IIR_butterworth_noise_bandpass_fs8000_BiquadCellInt32[0]), IIR_butterworth_noise_bandpass_fs8000_BiquadCellInt32
};

// 8000 Hz band-pass IIR
//     first  stop frequency :   1500 Hz
//     first  pass frequency :   2000 Hz
//     second pass frequency :   3500 Hz
//     second stop frequency :   3999 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_chebyshev1_noise_bandpass_fs8000_BiquadCellFloat[] =
{
  {+0.10317964, -0.20635928, +0.10317964, +0.74438053, +0.65563712},
  {+0.09330700, -0.18661401, +0.09330700, +1.11915225, +0.67492971},
  {+0.22706355, -0.45412710, +0.22706355, +0.38763237, +0.71500837},
  {+0.19793078, +0.00000000, -0.19793078, +1.42807656, +0.76033131},
  {+1.52990884, +3.05981767, +1.52990884, +0.11576940, +0.81662243},
  {+0.71258745, +1.42517489, +0.71258745, +1.63284340, +0.85120639},
  {+0.74020437, +1.48040874, +0.74020437, +1.75585445, +0.92114401},
  {+0.33471037, -0.66942073, +0.33471037, -0.03352851, +0.93553504},
  {+0.91425754, +1.82851508, +0.91425754, +1.82979820, +0.97522107}
};

const biquadFloat_t IIR_chebyshev1_noise_bandpass_fs8000_biquadFloat =
{
  sizeof(IIR_chebyshev1_noise_bandpass_fs8000_BiquadCellFloat) / sizeof(IIR_chebyshev1_noise_bandpass_fs8000_BiquadCellFloat[0]), IIR_chebyshev1_noise_bandpass_fs8000_BiquadCellFloat
};

static const biquadCellInt32_t IIR_chebyshev1_noise_bandpass_fs8000_BiquadCellInt32[] =
{
  {(int32_t)0x0D34FD8FUL, (int32_t)0xE59604E2UL, (int32_t)0x0D34FD8FUL, (int32_t)0x2FA3EE3FUL, (int32_t)0x53EBEACCUL},
  {(int32_t)0x0BF17BE3UL, (int32_t)0xE81D083AUL, (int32_t)0x0BF17BE3UL, (int32_t)0x47A030C7UL, (int32_t)0x566418CCUL},
  {(int32_t)0x1D106B1CUL, (int32_t)0xC5DF29C9UL, (int32_t)0x1D106B1CUL, (int32_t)0x18CEF7FFUL, (int32_t)0x5B8564F0UL},
  {(int32_t)0x1955CBBCUL, (int32_t)0x00000000UL, (int32_t)0xE6AA3444UL, (int32_t)0x5B659B37UL, (int32_t)0x6152894AUL},
  {(int32_t)0x30F50361UL, (int32_t)0x61EA06C1UL, (int32_t)0x30F50361UL, (int32_t)0x0768C413UL, (int32_t)0x68871570UL},
  {(int32_t)0x2D9B085FUL, (int32_t)0x5B3610BFUL, (int32_t)0x2D9B085FUL, (int32_t)0x68808197UL, (int32_t)0x6CF454BBUL},
  {(int32_t)0x2F5F8227UL, (int32_t)0x5EBF044DUL, (int32_t)0x2F5F8227UL, (int32_t)0x705FEB5BUL, (int32_t)0x75E80C0CUL},
  {(int32_t)0x2AD7CA0CUL, (int32_t)0xAA506BE8UL, (int32_t)0x2AD7CA0CUL, (int32_t)0xFDDAAB3AUL, (int32_t)0x77BF9CBEUL},
  {(int32_t)0x3A833211UL, (int32_t)0x75066422UL, (int32_t)0x3A833211UL, (int32_t)0x751B69E8UL, (int32_t)0x7CD40B3DUL}
};

const biquadInt32_t IIR_chebyshev1_noise_bandpass_fs8000_biquadInt32 =
{
  (int32_t)0x7FFFFFFFUL, 5, sizeof(IIR_chebyshev1_noise_bandpass_fs8000_BiquadCellInt32) / sizeof(IIR_chebyshev1_noise_bandpass_fs8000_BiquadCellInt32[0]), IIR_chebyshev1_noise_bandpass_fs8000_BiquadCellInt32
};

// 8000 Hz band-pass IIR
//     first  stop frequency :   1500 Hz
//     first  pass frequency :   2000 Hz
//     second pass frequency :   3500 Hz
//     second stop frequency :   3999 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_chebyshev2_noise_bandpass_fs8000_BiquadCellFloat[] =
{
  {+0.61185972, +0.00000000, -0.61185972, +0.77536864, -0.22371943},
  {+0.39774926, -0.71649615, +0.39774926, -0.41246587, +0.09931767},
  {+0.46320694, -0.63778158, +0.46320694, -0.32344635, +0.24071029},
  {+0.56324540, -0.56127374, +0.56324540, -0.22237954, +0.46551506},
  {+0.69735980, -0.55184390, +0.69735980, -0.16125256, +0.78594058},
  {+0.99930047, +1.99860087, +0.99930047, +1.99859977, +0.99860040},
  {+0.99942968, +1.99885911, +0.99942968, +1.99885866, +0.99885947},
  {+0.99962753, +1.99925460, +0.99962753, +1.99925484, +0.99925586},
  {+0.99987011, +1.99973962, +0.99987011, +1.99974045, +0.99974160}
};

const biquadFloat_t IIR_chebyshev2_noise_bandpass_fs8000_biquadFloat =
{
  sizeof(IIR_chebyshev2_noise_bandpass_fs8000_BiquadCellFloat) / sizeof(IIR_chebyshev2_noise_bandpass_fs8000_BiquadCellFloat[0]), IIR_chebyshev2_noise_bandpass_fs8000_BiquadCellFloat
};

static const biquadCellInt32_t IIR_chebyshev2_noise_bandpass_fs8000_BiquadCellInt32[] =
{
  {(int32_t)0x4E516B4DUL, (int32_t)0x00000000UL, (int32_t)0xB1AE94B3UL, (int32_t)0x319FA3CBUL, (int32_t)0xE35D2966UL},
  {(int32_t)0x32E97299UL, (int32_t)0xA449DAA5UL, (int32_t)0x32E97299UL, (int32_t)0xE59A28BDUL, (int32_t)0x0CB670F6UL},
  {(int32_t)0x3B4A5D76UL, (int32_t)0xAE5D2C4EUL, (int32_t)0x3B4A5D76UL, (int32_t)0xEB4CA7B0UL, (int32_t)0x1ECF983CUL},
  {(int32_t)0x48186CDAUL, (int32_t)0xB8282EA6UL, (int32_t)0x48186CDAUL, (int32_t)0xF1C48896UL, (int32_t)0x3B95FF55UL},
  {(int32_t)0x59431607UL, (int32_t)0xB95D2DE0UL, (int32_t)0x59431607UL, (int32_t)0xF5AE09B9UL, (int32_t)0x6499B369UL},
  {(int32_t)0x3FF489F4UL, (int32_t)0x7FE9139BUL, (int32_t)0x3FF489F4UL, (int32_t)0x7FE90F01UL, (int32_t)0x7FD22345UL},
  {(int32_t)0x3FF6A7EDUL, (int32_t)0x7FED4EC7UL, (int32_t)0x3FF6A7EDUL, (int32_t)0x7FED4CE0UL, (int32_t)0x7FDAA08AUL},
  {(int32_t)0x3FF9E5C0UL, (int32_t)0x7FF3C990UL, (int32_t)0x3FF9E5C0UL, (int32_t)0x7FF3CA94UL, (int32_t)0x7FE79DB0UL},
  {(int32_t)0x3FFDDF34UL, (int32_t)0x7FFBBBE7UL, (int32_t)0x3FFDDF34UL, (int32_t)0x7FFBBF59UL, (int32_t)0x7FF7885DUL}
};

const biquadInt32_t IIR_chebyshev2_noise_bandpass_fs8000_biquadInt32 =
{
  (int32_t)0x40000000UL, 5, sizeof(IIR_chebyshev2_noise_bandpass_fs8000_BiquadCellInt32) / sizeof(IIR_chebyshev2_noise_bandpass_fs8000_BiquadCellInt32[0]), IIR_chebyshev2_noise_bandpass_fs8000_BiquadCellInt32
};

// 8000 Hz band-pass IIR
//     first  stop frequency :   1500 Hz
//     first  pass frequency :   2000 Hz
//     second pass frequency :   3500 Hz
//     second stop frequency :   3999 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_elliptic_noise_bandpass_fs8000_BiquadCellFloat[] =
{
  {+0.16319831, -0.29280593, +0.16319831, +0.64624767, +0.46152085},
  {+0.57594001, +1.14694582, +0.57594001, +1.31527304, +0.61501969},
  {+0.39759878, -0.40725170, +0.39759878, +0.16230915, +0.66857960},
  {+0.78248472, +1.52551871, +0.78248472, +1.67644540, +0.84584309},
  {+0.60416205, -0.40302953, +0.60416205, -0.05938350, +0.89723005},
  {+0.94633655, +1.81927425, +0.94633655, +1.82000186, +0.96050223}
};

const biquadFloat_t IIR_elliptic_noise_bandpass_fs8000_biquadFloat =
{
  sizeof(IIR_elliptic_noise_bandpass_fs8000_BiquadCellFloat) / sizeof(IIR_elliptic_noise_bandpass_fs8000_BiquadCellFloat[0]), IIR_elliptic_noise_bandpass_fs8000_BiquadCellFloat
};

static const biquadCellInt32_t IIR_elliptic_noise_bandpass_fs8000_BiquadCellInt32[] =
{
  {(int32_t)0x14E3AEA9UL, (int32_t)0xDA8555D0UL, (int32_t)0x14E3AEA9UL, (int32_t)0x295C1F2BUL, (int32_t)0x3B131D87UL},
  {(int32_t)0x24DC3379UL, (int32_t)0x49678F75UL, (int32_t)0x24DC3379UL, (int32_t)0x542D6EF9UL, (int32_t)0x4EB8F710UL},
  {(int32_t)0x32E48446UL, (int32_t)0xCBDF2D18UL, (int32_t)0x32E48446UL, (int32_t)0x0A6345ECUL, (int32_t)0x55940428UL},
  {(int32_t)0x32143AC7UL, (int32_t)0x61A21937UL, (int32_t)0x32143AC7UL, (int32_t)0x6B4AE1A1UL, (int32_t)0x6C44961DUL},
  {(int32_t)0x4D552E92UL, (int32_t)0xCC69873BUL, (int32_t)0x4D552E92UL, (int32_t)0xFC330F8EUL, (int32_t)0x72D86F37UL},
  {(int32_t)0x3C90C732UL, (int32_t)0x746EFD48UL, (int32_t)0x3C90C732UL, (int32_t)0x747AE913UL, (int32_t)0x7AF1BCB3UL}
};

const biquadInt32_t IIR_elliptic_noise_bandpass_fs8000_biquadInt32 =
{
  (int32_t)0x7FFFFFFFUL, 3, sizeof(IIR_elliptic_noise_bandpass_fs8000_BiquadCellInt32) / sizeof(IIR_elliptic_noise_bandpass_fs8000_BiquadCellInt32[0]), IIR_elliptic_noise_bandpass_fs8000_BiquadCellInt32
};

#endif /* AC_SUPPORT_FS_8000 */

#ifdef AC_SUPPORT_FS_16000
// 16000 Hz band-pass IIR
//     first  stop frequency :   1500 Hz
//     first  pass frequency :   2000 Hz
//     second pass frequency :   3500 Hz
//     second stop frequency :   3999 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_butterworth_noise_bandpass_fs16000_BiquadCellFloat[] =
{
  {+0.14883513, +0.29767025, +0.14883513, -0.64946094, +0.48383295},
  {+0.13743057, +0.27486113, +0.13743057, -0.73403945, +0.49010781},
  {+0.16120505, +0.32241009, +0.16120505, -0.57019494, +0.49201121},
  {+0.12646639, +0.25293277, +0.12646639, -0.82113285, +0.50981442},
  {+0.17508519, +0.35017039, +0.17508519, -0.49858118, +0.51484371},
  {+0.11539814, +0.23079629, +0.11539814, -0.90803203, +0.54118771},
  {+0.19105268, +0.38210536, +0.19105268, -0.43628790, +0.55199146},
  {+0.10365728, +0.20731456, +0.10365728, -0.99266772, +0.58215053},
  {+0.20964374, +0.41928749, +0.20964374, -0.38445100, +0.60296525},
  {+0.19036242, +0.00000000, -0.19036242, -1.07383890, +0.63081302},
  {+0.86500965, -1.73001930, +0.86500965, -0.34397519, +0.66741398},
  {+0.37018414, -0.74036828, +0.37018414, -1.15109864, +0.68577846},
  {+0.76809519, -1.53619038, +0.76809519, -0.31583368, +0.74525444},
  {+0.38640012, -0.77280025, +0.38640012, -1.22448812, +0.74621987},
  {+0.34871475, -0.69742949, +0.34871475, -1.29428094, +0.81182565},
  {+0.82993979, -1.65987958, +0.82993979, -0.30130683, +0.83667018},
  {+0.38042382, -0.76084764, +0.38042382, -1.36079228, +0.88270868},
  {+0.52694177, -1.05388355, +0.52694177, -0.30215944, +0.94200754},
  {+0.64444956, -1.28889913, +0.64444956, -1.42424092, +0.95932384}
};

const biquadFloat_t IIR_butterworth_noise_bandpass_fs16000_biquadFloat =
{
  sizeof(IIR_butterworth_noise_bandpass_fs16000_BiquadCellFloat) / sizeof(IIR_butterworth_noise_bandpass_fs16000_BiquadCellFloat[0]), IIR_butterworth_noise_bandpass_fs16000_BiquadCellFloat
};

static const biquadCellInt32_t IIR_butterworth_noise_bandpass_fs16000_BiquadCellInt32[] =
{
  {(int32_t)0x130D0788UL, (int32_t)0x261A0F10UL, (int32_t)0x130D0788UL, (int32_t)0xD66F3B63UL, (int32_t)0x3DEE3CECUL},
  {(int32_t)0x11975325UL, (int32_t)0x232EA64BUL, (int32_t)0x11975325UL, (int32_t)0xD1057F64UL, (int32_t)0x3EBBDA4CUL},
  {(int32_t)0x14A25DF2UL, (int32_t)0x2944BBE4UL, (int32_t)0x14A25DF2UL, (int32_t)0xDB81ED18UL, (int32_t)0x3EFA3924UL},
  {(int32_t)0x10300CF1UL, (int32_t)0x206019E1UL, (int32_t)0x10300CF1UL, (int32_t)0xCB728F33UL, (int32_t)0x41419957UL},
  {(int32_t)0x1669310FUL, (int32_t)0x2CD2621EUL, (int32_t)0x1669310FUL, (int32_t)0xE0173EF7UL, (int32_t)0x41E66616UL},
  {(int32_t)0x0EC55DC9UL, (int32_t)0x1D8ABB92UL, (int32_t)0x0EC55DC9UL, (int32_t)0xC5E2CD9FUL, (int32_t)0x4545A394UL},
  {(int32_t)0x18746A0BUL, (int32_t)0x30E8D415UL, (int32_t)0x18746A0BUL, (int32_t)0xE413DBEBUL, (int32_t)0x46A7A7F3UL},
  {(int32_t)0x0D44A446UL, (int32_t)0x1A89488BUL, (int32_t)0x0D44A446UL, (int32_t)0xC07821D0UL, (int32_t)0x4A83E89DUL},
  {(int32_t)0x1AD59B2EUL, (int32_t)0x35AB365BUL, (int32_t)0x1AD59B2EUL, (int32_t)0xE76527A2UL, (int32_t)0x4D2DF719UL},
  {(int32_t)0x185DCBB7UL, (int32_t)0x00000000UL, (int32_t)0xE7A23449UL, (int32_t)0xBB463938UL, (int32_t)0x50BE7B1BUL},
  {(int32_t)0x375C5171UL, (int32_t)0x91475D1EUL, (int32_t)0x375C5171UL, (int32_t)0xE9FC4F7AUL, (int32_t)0x556DD243UL},
  {(int32_t)0x2F62319FUL, (int32_t)0xA13B9CC2UL, (int32_t)0x2F62319FUL, (int32_t)0xB6546663UL, (int32_t)0x57C796A8UL},
  {(int32_t)0x312878BCUL, (int32_t)0x9DAF0E88UL, (int32_t)0x312878BCUL, (int32_t)0xEBC9618CUL, (int32_t)0x5F647F51UL},
  {(int32_t)0x31758F2DUL, (int32_t)0x9D14E1A6UL, (int32_t)0x31758F2DUL, (int32_t)0xB1A1FC91UL, (int32_t)0x5F8421F5UL},
  {(int32_t)0x2CA2AF50UL, (int32_t)0xA6BAA160UL, (int32_t)0x2CA2AF50UL, (int32_t)0xAD2A804BUL, (int32_t)0x67E9E723UL},
  {(int32_t)0x351DBBC7UL, (int32_t)0x95C48871UL, (int32_t)0x351DBBC7UL, (int32_t)0xECB7638FUL, (int32_t)0x6B180224UL},
  {(int32_t)0x30B1BA51UL, (int32_t)0x9E9C8B5DUL, (int32_t)0x30B1BA51UL, (int32_t)0xA8E8C77EUL, (int32_t)0x70FC9916UL},
  {(int32_t)0x21B969FEUL, (int32_t)0xBC8D2C03UL, (int32_t)0x21B969FEUL, (int32_t)0xECA96B72UL, (int32_t)0x7893B3FBUL},
  {(int32_t)0x293EA963UL, (int32_t)0xAD82AD3BUL, (int32_t)0x293EA963UL, (int32_t)0xA4D93C9EUL, (int32_t)0x7ACB1FA6UL}
};

const biquadInt32_t IIR_butterworth_noise_bandpass_fs16000_biquadInt32 =
{
  (int32_t)0x40000000UL, 6, sizeof(IIR_butterworth_noise_bandpass_fs16000_BiquadCellInt32) / sizeof(IIR_butterworth_noise_bandpass_fs16000_BiquadCellInt32[0]), IIR_butterworth_noise_bandpass_fs16000_BiquadCellInt32
};

// 16000 Hz band-pass IIR
//     first  stop frequency :   1500 Hz
//     first  pass frequency :   2000 Hz
//     second pass frequency :   3500 Hz
//     second stop frequency :   3999 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_chebyshev1_noise_bandpass_fs16000_BiquadCellFloat[] =
{
  {+0.04696947, +0.09393895, +0.04696947, -0.90543803, +0.83804873},
  {+0.07317398, +0.14634796, +0.07317398, -0.72285653, +0.84022891},
  {+0.06858569, +0.13717138, +0.06858569, -1.07896809, +0.85727821},
  {+0.13596220, +0.27192439, +0.13596220, -0.55638963, +0.86552728},
  {+0.14209933, +0.00000000, -0.14209933, -1.22500620, +0.89118942},
  {+0.63284414, -1.26568829, +0.63284414, -0.43130777, +0.91014396},
  {+0.33836504, -0.67673008, +0.33836504, -1.33473367, +0.93251505},
  {+0.45182570, -0.90365140, +0.45182570, -0.36795007, +0.96799404},
  {+0.63219636, -1.26439272, +0.63219636, -1.40623079, +0.97699472}
};

const biquadFloat_t IIR_chebyshev1_noise_bandpass_fs16000_biquadFloat =
{
  sizeof(IIR_chebyshev1_noise_bandpass_fs16000_BiquadCellFloat) / sizeof(IIR_chebyshev1_noise_bandpass_fs16000_BiquadCellFloat[0]), IIR_chebyshev1_noise_bandpass_fs16000_BiquadCellFloat
};

static const biquadCellInt32_t IIR_chebyshev1_noise_bandpass_fs16000_BiquadCellInt32[] =
{
  {(int32_t)0x06031882UL, (int32_t)0x0C063104UL, (int32_t)0x06031882UL, (int32_t)0xC60D4DA7UL, (int32_t)0x6B452E3EUL},
  {(int32_t)0x095DC3D0UL, (int32_t)0x12BB87A1UL, (int32_t)0x095DC3D0UL, (int32_t)0xD1BCB7FCUL, (int32_t)0x6B8C9EF2UL},
  {(int32_t)0x08C76A78UL, (int32_t)0x118ED4EFUL, (int32_t)0x08C76A78UL, (int32_t)0xBAF22FD6UL, (int32_t)0x6DBB4AE0UL},
  {(int32_t)0x11673590UL, (int32_t)0x22CE6B20UL, (int32_t)0x11673590UL, (int32_t)0xDC641CBCUL, (int32_t)0x6EC9990CUL},
  {(int32_t)0x12304F9AUL, (int32_t)0x00000000UL, (int32_t)0xEDCFB066UL, (int32_t)0xB1997F9AUL, (int32_t)0x72127EB4UL},
  {(int32_t)0x288084B9UL, (int32_t)0xAEFEF68EUL, (int32_t)0x288084B9UL, (int32_t)0xE4657418UL, (int32_t)0x747F98EBUL},
  {(int32_t)0x2B4F8BAAUL, (int32_t)0xA960E8ACUL, (int32_t)0x2B4F8BAAUL, (int32_t)0xAA93B93BUL, (int32_t)0x775CA739UL},
  {(int32_t)0x39D56CABUL, (int32_t)0x8C5526A9UL, (int32_t)0x39D56CABUL, (int32_t)0xE873818CUL, (int32_t)0x7BE73A89UL},
  {(int32_t)0x2875E7C0UL, (int32_t)0xAF14307FUL, (int32_t)0x2875E7C0UL, (int32_t)0xA6005097UL, (int32_t)0x7D0E29C3UL}
};

const biquadInt32_t IIR_chebyshev1_noise_bandpass_fs16000_biquadInt32 =
{
  (int32_t)0x7FFFFFE8UL, 2, sizeof(IIR_chebyshev1_noise_bandpass_fs16000_BiquadCellInt32) / sizeof(IIR_chebyshev1_noise_bandpass_fs16000_BiquadCellInt32[0]), IIR_chebyshev1_noise_bandpass_fs16000_BiquadCellInt32
};

// 16000 Hz band-pass IIR
//     first  stop frequency :   1500 Hz
//     first  pass frequency :   2000 Hz
//     second pass frequency :   3500 Hz
//     second stop frequency :   3999 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_chebyshev2_noise_bandpass_fs16000_BiquadCellFloat[] =
{
  {+0.36029393, -0.00000000, -0.36029393, -0.68403945, +0.27941213},
  {+0.28519106, +0.37133818, +0.28519106, -0.33616846, +0.32716641},
  {+0.62582089, -1.20392015, +0.62582089, -1.07998735, +0.47416408},
  {+0.43560790, +0.23214267, +0.43560790, -0.24199309, +0.52092464},
  {+0.67530439, -1.21402574, +0.67530439, -1.28997423, +0.66442166},
  {+0.56615755, +0.09088747, +0.56615755, -0.25276963, +0.71955922},
  {+0.74891665, -1.28034855, +0.74891665, -1.41395890, +0.81292108},
  {+0.61271843, +0.00959824, +0.61271843, -0.28705909, +0.90470504},
  {+0.87885968, -1.46592237, +0.87885968, -1.50894720, +0.93841958}
};

const biquadFloat_t IIR_chebyshev2_noise_bandpass_fs16000_biquadFloat =
{
  sizeof(IIR_chebyshev2_noise_bandpass_fs16000_BiquadCellFloat) / sizeof(IIR_chebyshev2_noise_bandpass_fs16000_BiquadCellFloat[0]), IIR_chebyshev2_noise_bandpass_fs16000_BiquadCellFloat
};

static const biquadCellInt32_t IIR_chebyshev2_noise_bandpass_fs16000_BiquadCellInt32[] =
{
  {(int32_t)0x2E1E1C93UL, (int32_t)0x00000000UL, (int32_t)0xD1E1E36DUL, (int32_t)0xD438B295UL, (int32_t)0x23C3C6DCUL},
  {(int32_t)0x2481240BUL, (int32_t)0x2F880270UL, (int32_t)0x2481240BUL, (int32_t)0xEA7C374AUL, (int32_t)0x29E096CAUL},
  {(int32_t)0x280D7315UL, (int32_t)0xB2F2F8E4UL, (int32_t)0x280D7315UL, (int32_t)0xBAE17CC0UL, (int32_t)0x3CB16896UL},
  {(int32_t)0x37C1FFE3UL, (int32_t)0x1DB6D9DFUL, (int32_t)0x37C1FFE3UL, (int32_t)0xF0832F69UL, (int32_t)0x42ADA89CUL},
  {(int32_t)0x2B382FEDUL, (int32_t)0xB24D66FFUL, (int32_t)0x2B382FEDUL, (int32_t)0xAD710FE8UL, (int32_t)0x550BC4D1UL},
  {(int32_t)0x4877D9C5UL, (int32_t)0x0BA23352UL, (int32_t)0x4877D9C5UL, (int32_t)0xEFD29F4FUL, (int32_t)0x5C1A8442UL},
  {(int32_t)0x2FEE4017UL, (int32_t)0xAE0EC4F0UL, (int32_t)0x2FEE4017UL, (int32_t)0xA581B28CUL, (int32_t)0x680DCC42UL},
  {(int32_t)0x4E6D8EBCUL, (int32_t)0x013A83DBUL, (int32_t)0x4E6D8EBCUL, (int32_t)0xEDA0D2E5UL, (int32_t)0x73CD5FEEUL},
  {(int32_t)0x383F3CB0UL, (int32_t)0xA22E53EEUL, (int32_t)0x383F3CB0UL, (int32_t)0x9F6D68BEUL, (int32_t)0x781E21FBUL}
};

const biquadInt32_t IIR_chebyshev2_noise_bandpass_fs16000_biquadInt32 =
{
  (int32_t)0x40000000UL, 5, sizeof(IIR_chebyshev2_noise_bandpass_fs16000_BiquadCellInt32) / sizeof(IIR_chebyshev2_noise_bandpass_fs16000_BiquadCellInt32[0]), IIR_chebyshev2_noise_bandpass_fs16000_BiquadCellInt32
};

// 16000 Hz band-pass IIR
//     first  stop frequency :   1500 Hz
//     first  pass frequency :   2000 Hz
//     second pass frequency :   3500 Hz
//     second stop frequency :   3999 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_elliptic_noise_bandpass_fs16000_BiquadCellFloat[] =
{
  {+0.11116676, +0.15177289, +0.11116676, -0.68565801, +0.73388473},
  {+0.30434424, -0.58272137, +0.30434424, -1.01890264, +0.76016568},
  {+0.35289058, +0.08912487, +0.35289058, -0.44738590, +0.83320270},
  {+0.58899783, -0.98416995, +0.58899783, -1.27332453, +0.87133673},
  {+0.53714459, -0.01099669, +0.53714459, -0.35153194, +0.94801790},
  {+0.85740906, -1.35298082, +0.85740906, -1.40260205, +0.96275081}
};

const biquadFloat_t IIR_elliptic_noise_bandpass_fs16000_biquadFloat =
{
  sizeof(IIR_elliptic_noise_bandpass_fs16000_BiquadCellFloat) / sizeof(IIR_elliptic_noise_bandpass_fs16000_BiquadCellFloat[0]), IIR_elliptic_noise_bandpass_fs16000_BiquadCellFloat
};

static const biquadCellInt32_t IIR_elliptic_noise_bandpass_fs16000_BiquadCellInt32[] =
{
  {(int32_t)0x0E3AB656UL, (int32_t)0x136D4B3DUL, (int32_t)0x0E3AB656UL, (int32_t)0xD41E2DDEUL, (int32_t)0x5DEFEF4EUL},
  {(int32_t)0x26F4C07FUL, (int32_t)0xB56962DEUL, (int32_t)0x26F4C07FUL, (int32_t)0xBECA4C90UL, (int32_t)0x614D1BF2UL},
  {(int32_t)0x2D2B84BFUL, (int32_t)0x0B687197UL, (int32_t)0x2D2B84BFUL, (int32_t)0xE35E078AUL, (int32_t)0x6AA662DAUL},
  {(int32_t)0x4B6447E8UL, (int32_t)0x8206B814UL, (int32_t)0x4B6447E8UL, (int32_t)0xAE81D9D6UL, (int32_t)0x6F87F645UL},
  {(int32_t)0x44C12769UL, (int32_t)0xFE97A911UL, (int32_t)0x44C12769UL, (int32_t)0xE980802FUL, (int32_t)0x7958A682UL},
  {(int32_t)0x36DFCA45UL, (int32_t)0xA968C324UL, (int32_t)0x36DFCA45UL, (int32_t)0xA63BC4A0UL, (int32_t)0x7B3B6B1DUL}
};

const biquadInt32_t IIR_elliptic_noise_bandpass_fs16000_biquadInt32 =
{
  (int32_t)0x7FFFFFFBUL, 1, sizeof(IIR_elliptic_noise_bandpass_fs16000_BiquadCellInt32) / sizeof(IIR_elliptic_noise_bandpass_fs16000_BiquadCellInt32[0]), IIR_elliptic_noise_bandpass_fs16000_BiquadCellInt32
};

#endif /* AC_SUPPORT_FS_16000 */

#ifdef AC_SUPPORT_FS_48000
// 48000 Hz band-pass IIR
//     first  stop frequency :   1500 Hz
//     first  pass frequency :   2000 Hz
//     second pass frequency :   3500 Hz
//     second stop frequency :   3999 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_butterworth_noise_bandpass_fs48000_BiquadCellFloat[] =
{
  {+0.01818562, +0.03637125, +0.01818562, -1.67371417, +0.79660048},
  {+0.01916295, +0.03832590, +0.01916295, -1.66321135, +0.79736836},
  {+0.01724721, +0.03449443, +0.01724721, -1.68811999, +0.80048666},
  {+0.02019372, +0.04038743, +0.02019372, -1.65727086, +0.80318342},
  {+0.01633322, +0.03266643, +0.01633322, -1.70565752, +0.80846504},
  {+0.02129790, +0.04259581, +0.02129790, -1.65639623, +0.81424708},
  {+0.01542312, +0.03084624, +0.01542312, -1.72551631, +0.81986185},
  {+0.02250627, +0.04501254, +0.02250627, -1.66092162, +0.83057381},
  {+0.01448382, +0.02896765, +0.01448382, -1.74693081, +0.83397132},
  {+0.01269880, +0.02539761, +0.01269880, -1.76924977, +0.85013490},
  {+0.02530141, +0.05060283, +0.02530141, -1.67101731, +0.85200878},
  {+0.45219333, -0.90438666, +0.45219333, -1.79197295, +0.86779760},
  {+0.90862325, -1.81724649, +0.90862325, -1.68670015, +0.87824460},
  {+0.45160481, -0.90320963, +0.45160481, -1.81475362, +0.88653275},
  {+0.42083320, -0.84166639, +0.42083320, -1.83737755, +0.90604003},
  {+0.89908551, -1.79817101, +0.89908551, -1.70783887, +0.90882932},
  {+0.43902472, -0.87804944, +0.43902472, -1.85973208, +0.92612798},
  {+0.77165226, -1.54330453, +0.77165226, -1.73414978, +0.94316429},
  {+0.49022356, -0.98044712, +0.49022356, -1.88177494, +0.94669091},
  {+0.41234457, -0.82468914, +0.41234457, -1.90350735, +0.96768631},
  {+0.60780225, -1.21560449, +0.60780225, -1.76518248, +0.98049411},
  {+0.67942312, -1.35884625, +0.67942312, -1.92495214, +0.98911513}
};

const biquadFloat_t IIR_butterworth_noise_bandpass_fs48000_biquadFloat =
{
  sizeof(IIR_butterworth_noise_bandpass_fs48000_BiquadCellFloat) / sizeof(IIR_butterworth_noise_bandpass_fs48000_BiquadCellFloat[0]), IIR_butterworth_noise_bandpass_fs48000_BiquadCellFloat
};

static const biquadCellInt32_t IIR_butterworth_noise_bandpass_fs48000_BiquadCellInt32[] =
{
  {(int32_t)0x0253E814UL, (int32_t)0x04A7D027UL, (int32_t)0x0253E814UL, (int32_t)0x94E1DDF9UL, (int32_t)0x65F7012EUL},
  {(int32_t)0x0273EE79UL, (int32_t)0x04E7DCF2UL, (int32_t)0x0273EE79UL, (int32_t)0x958DF1FAUL, (int32_t)0x66102A90UL},
  {(int32_t)0x02352820UL, (int32_t)0x046A5040UL, (int32_t)0x02352820UL, (int32_t)0x93F5D791UL, (int32_t)0x667658D4UL},
  {(int32_t)0x0295B52DUL, (int32_t)0x052B6A5AUL, (int32_t)0x0295B52DUL, (int32_t)0x95EF462FUL, (int32_t)0x66CEB6D8UL},
  {(int32_t)0x021734F2UL, (int32_t)0x042E69E4UL, (int32_t)0x021734F2UL, (int32_t)0x92D681D8UL, (int32_t)0x677BC849UL},
  {(int32_t)0x02B9E3C2UL, (int32_t)0x0573C784UL, (int32_t)0x02B9E3C2UL, (int32_t)0x95FD9AA8UL, (int32_t)0x68393F8DUL},
  {(int32_t)0x01F9627EUL, (int32_t)0x03F2C4FBUL, (int32_t)0x01F9627EUL, (int32_t)0x9191240CUL, (int32_t)0x68F13BA9UL},
  {(int32_t)0x02E17C47UL, (int32_t)0x05C2F88EUL, (int32_t)0x02E17C47UL, (int32_t)0x95B375CBUL, (int32_t)0x6A503E12UL},
  {(int32_t)0x01DA9B1FUL, (int32_t)0x03B5363DUL, (int32_t)0x01DA9B1FUL, (int32_t)0x9032491EUL, (int32_t)0x6ABF9280UL},
  {(int32_t)0x01A01D48UL, (int32_t)0x03403A90UL, (int32_t)0x01A01D48UL, (int32_t)0x8EC49C9EUL, (int32_t)0x6CD13868UL},
  {(int32_t)0x033D13A2UL, (int32_t)0x067A2745UL, (int32_t)0x033D13A2UL, (int32_t)0x950E0D65UL, (int32_t)0x6D0E9FB2UL},
  {(int32_t)0x39E1789BUL, (int32_t)0x8C3D0ECAUL, (int32_t)0x39E1789BUL, (int32_t)0x8D5050B0UL, (int32_t)0x6F13FDE2UL},
  {(int32_t)0x3A26E21EUL, (int32_t)0x8BB23BC4UL, (int32_t)0x3A26E21EUL, (int32_t)0x940D1AD2UL, (int32_t)0x706A51AEUL},
  {(int32_t)0x39CE2FC0UL, (int32_t)0x8C63A080UL, (int32_t)0x39CE2FC0UL, (int32_t)0x8BDB13A4UL, (int32_t)0x7179E7B6UL},
  {(int32_t)0x35DDDCBAUL, (int32_t)0x9444468DUL, (int32_t)0x35DDDCBAUL, (int32_t)0x8A6867FFUL, (int32_t)0x73F91EACUL},
  {(int32_t)0x398A9DEFUL, (int32_t)0x8CEAC422UL, (int32_t)0x398A9DEFUL, (int32_t)0x92B2C494UL, (int32_t)0x745484ECUL},
  {(int32_t)0x3831F643UL, (int32_t)0x8F9C137AUL, (int32_t)0x3831F643UL, (int32_t)0x88FA2649UL, (int32_t)0x768B5C9DUL},
  {(int32_t)0x3162C02CUL, (int32_t)0x9D3A7FA8UL, (int32_t)0x3162C02CUL, (int32_t)0x9103B0A7UL, (int32_t)0x78B99B78UL},
  {(int32_t)0x3EBFA546UL, (int32_t)0x8280B574UL, (int32_t)0x3EBFA546UL, (int32_t)0x8790FFD3UL, (int32_t)0x792D2AEDUL},
  {(int32_t)0x34C7B4F3UL, (int32_t)0x9670961AUL, (int32_t)0x34C7B4F3UL, (int32_t)0x862CEF80UL, (int32_t)0x7BDD2522UL},
  {(int32_t)0x26E63B65UL, (int32_t)0xB2338936UL, (int32_t)0x26E63B65UL, (int32_t)0x8F07400FUL, (int32_t)0x7D80D4BEUL},
  {(int32_t)0x2B7BAB21UL, (int32_t)0xA908A9BEUL, (int32_t)0x2B7BAB21UL, (int32_t)0x84CD9585UL, (int32_t)0x7E9B5316UL}
};

const biquadInt32_t IIR_butterworth_noise_bandpass_fs48000_biquadInt32 =
{
  (int32_t)0x40000000UL, 6, sizeof(IIR_butterworth_noise_bandpass_fs48000_BiquadCellInt32) / sizeof(IIR_butterworth_noise_bandpass_fs48000_BiquadCellInt32[0]), IIR_butterworth_noise_bandpass_fs48000_BiquadCellInt32
};

// 48000 Hz band-pass IIR
//     first  stop frequency :   1500 Hz
//     first  pass frequency :   2000 Hz
//     second pass frequency :   3500 Hz
//     second stop frequency :   3999 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_chebyshev1_noise_bandpass_fs48000_BiquadCellFloat[] =
{
  {+0.00472328, +0.00944656, +0.00472328, -1.82130127, +0.94841237},
  {+0.00703603, +0.01407205, +0.00703603, -1.79884454, +0.94982914},
  {+0.00750997, +0.01501994, +0.00750997, -1.84649766, +0.95266885},
  {+0.01274617, +0.02549234, +0.01274617, -1.78282666, +0.95773583},
  {+0.00988774, +0.01977547, +0.00988774, -1.87125386, +0.96091053},
  {+0.29211698, -0.58423396, +0.29211698, -1.89344999, +0.97133304},
  {+0.75658696, -1.51317392, +0.75658696, -1.77664465, +0.97167127},
  {+0.37504527, -0.75009055, +0.37504527, -1.91209730, +0.98263369},
  {+0.52187587, -1.04375174, +0.52187587, -1.78249394, +0.98997492},
  {+0.66956938, -1.33913875, +0.66956938, -1.92700119, +0.99418401}
};

const biquadFloat_t IIR_chebyshev1_noise_bandpass_fs48000_biquadFloat =
{
  sizeof(IIR_chebyshev1_noise_bandpass_fs48000_BiquadCellFloat) / sizeof(IIR_chebyshev1_noise_bandpass_fs48000_BiquadCellFloat[0]), IIR_chebyshev1_noise_bandpass_fs48000_BiquadCellFloat
};

static const biquadCellInt32_t IIR_chebyshev1_noise_bandpass_fs48000_BiquadCellInt32[] =
{
  {(int32_t)0x009AC5C3UL, (int32_t)0x01358B87UL, (int32_t)0x009AC5C3UL, (int32_t)0x8B6FCCD0UL, (int32_t)0x796593A3UL},
  {(int32_t)0x00E68E75UL, (int32_t)0x01CD1CEAUL, (int32_t)0x00E68E75UL, (int32_t)0x8CDFBB2CUL, (int32_t)0x7994004BUL},
  {(int32_t)0x00F61632UL, (int32_t)0x01EC2C65UL, (int32_t)0x00F61632UL, (int32_t)0x89D2FB7BUL, (int32_t)0x79F10D7FUL},
  {(int32_t)0x01A1AAA0UL, (int32_t)0x03435541UL, (int32_t)0x01A1AAA0UL, (int32_t)0x8DE62B00UL, (int32_t)0x7A97166EUL},
  {(int32_t)0x01440059UL, (int32_t)0x028800B2UL, (int32_t)0x01440059UL, (int32_t)0x883D6072UL, (int32_t)0x7AFF1DC0UL},
  {(int32_t)0x256416DBUL, (int32_t)0xB537D24AUL, (int32_t)0x256416DBUL, (int32_t)0x86D1B722UL, (int32_t)0x7C54A41AUL},
  {(int32_t)0x306BEBB5UL, (int32_t)0x9F282896UL, (int32_t)0x306BEBB5UL, (int32_t)0x8E4B743CUL, (int32_t)0x7C5FB961UL},
  {(int32_t)0x30017BCBUL, (int32_t)0x9FFD086BUL, (int32_t)0x30017BCBUL, (int32_t)0x85A032A4UL, (int32_t)0x7DC6F0DFUL},
  {(int32_t)0x21666A0FUL, (int32_t)0xBD332BE2UL, (int32_t)0x21666A0FUL, (int32_t)0x8DEB9E89UL, (int32_t)0x7EB77F8DUL},
  {(int32_t)0x2ADA3988UL, (int32_t)0xAA4B8CF1UL, (int32_t)0x2ADA3988UL, (int32_t)0x84AC032FUL, (int32_t)0x7F416BF3UL}
};

const biquadInt32_t IIR_chebyshev1_noise_bandpass_fs48000_biquadInt32 =
{
  (int32_t)0x7FFFFFF2UL, 3, sizeof(IIR_chebyshev1_noise_bandpass_fs48000_BiquadCellInt32) / sizeof(IIR_chebyshev1_noise_bandpass_fs48000_BiquadCellInt32[0]), IIR_chebyshev1_noise_bandpass_fs48000_BiquadCellInt32
};

// 48000 Hz band-pass IIR
//     first  stop frequency :   1500 Hz
//     first  pass frequency :   2000 Hz
//     second pass frequency :   3500 Hz
//     second stop frequency :   3999 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_chebyshev2_noise_bandpass_fs48000_BiquadCellFloat[] =
{
  {+0.05207734, +0.01051821, +0.05207734, -1.52008511, +0.63971812},
  {+0.25476623, -0.34806400, +0.25476623, -1.52115001, +0.69719236},
  {+0.93832520, -1.87451822, +0.93832520, -1.66757065, +0.73003849},
  {+0.45561667, -0.74041771, +0.45561667, -1.60266563, +0.80345465},
  {+0.80643309, -1.60098402, +0.80643309, -1.78936569, +0.83549538},
  {+0.58152175, -0.99027696, +0.58152175, -1.68441295, +0.89379382},
  {+0.82991627, -1.63763802, +0.82991627, -1.85844463, +0.90285993},
  {+0.84245310, -1.65592094, +0.84245310, -1.90264807, +0.94846104},
  {+0.61926160, -1.07088345, +0.61926160, -1.75173171, +0.96625026},
  {+0.92858193, -1.82188351, +0.92858193, -1.93643224, +0.98372780}
};

const biquadFloat_t IIR_chebyshev2_noise_bandpass_fs48000_biquadFloat =
{
  sizeof(IIR_chebyshev2_noise_bandpass_fs48000_BiquadCellFloat) / sizeof(IIR_chebyshev2_noise_bandpass_fs48000_BiquadCellFloat[0]), IIR_chebyshev2_noise_bandpass_fs48000_BiquadCellFloat
};

static const biquadCellInt32_t IIR_chebyshev2_noise_bandpass_fs48000_BiquadCellInt32[] =
{
  {(int32_t)0x06AA785EUL, (int32_t)0x0158A926UL, (int32_t)0x06AA785EUL, (int32_t)0x9EB6ECF3UL, (int32_t)0x51E24895UL},
  {(int32_t)0x209C2DFEUL, (int32_t)0xD372A396UL, (int32_t)0x209C2DFEUL, (int32_t)0x9EA57A6EUL, (int32_t)0x593D9971UL},
  {(int32_t)0x3C0D8524UL, (int32_t)0x8807E4C0UL, (int32_t)0x3C0D8524UL, (int32_t)0x954685C5UL, (int32_t)0x5D71E6BDUL},
  {(int32_t)0x3A51A5AAUL, (int32_t)0xA139FE18UL, (int32_t)0x3A51A5AAUL, (int32_t)0x996DED26UL, (int32_t)0x66D79A10UL},
  {(int32_t)0x339C9986UL, (int32_t)0x99897A4EUL, (int32_t)0x339C9986UL, (int32_t)0x8D7B0855UL, (int32_t)0x6AF18343UL},
  {(int32_t)0x4A6F4E03UL, (int32_t)0x813E9AC4UL, (int32_t)0x4A6F4E03UL, (int32_t)0x94329405UL, (int32_t)0x7267D604UL},
  {(int32_t)0x351D5921UL, (int32_t)0x9730F04EUL, (int32_t)0x351D5921UL, (int32_t)0x890F3E44UL, (int32_t)0x7390EA0FUL},
  {(int32_t)0x35EAC066UL, (int32_t)0x96056430UL, (int32_t)0x35EAC066UL, (int32_t)0x863B0392UL, (int32_t)0x79672BDBUL},
  {(int32_t)0x27A1FB65UL, (int32_t)0xBB76A547UL, (int32_t)0x27A1FB65UL, (int32_t)0x8FE3A0ABUL, (int32_t)0x7BAE16B3UL},
  {(int32_t)0x3B6DE2EAUL, (int32_t)0x8B6642B2UL, (int32_t)0x3B6DE2EAUL, (int32_t)0x84117E85UL, (int32_t)0x7DEACADCUL}
};

const biquadInt32_t IIR_chebyshev2_noise_bandpass_fs48000_biquadInt32 =
{
  (int32_t)0x40000000UL, 7, sizeof(IIR_chebyshev2_noise_bandpass_fs48000_BiquadCellInt32) / sizeof(IIR_chebyshev2_noise_bandpass_fs48000_BiquadCellInt32[0]), IIR_chebyshev2_noise_bandpass_fs48000_BiquadCellInt32
};

// 48000 Hz band-pass IIR
//     first  stop frequency :   1500 Hz
//     first  pass frequency :   2000 Hz
//     second pass frequency :   3500 Hz
//     second stop frequency :   3999 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_elliptic_noise_bandpass_fs48000_BiquadCellFloat[] =
{
  {+0.04510213, -0.00000000, -0.04510213, -1.79603757, +0.90979574},
  {+0.15174132, -0.23796339, +0.15174132, -1.76152754, +0.92005902},
  {+0.47261644, -0.93062525, +0.47261644, -1.85805054, +0.94119050},
  {+0.47796544, -0.82522756, +0.47796544, -1.76484129, +0.95727625},
  {+0.73078123, -1.42450161, +0.73078123, -1.90292608, +0.97358062},
  {+0.64394512, -1.13035446, +0.64394512, -1.77963877, +0.98773690},
  {+0.92875024, -1.80452969, +0.92875024, -1.92605546, +0.99291075}
};

const biquadFloat_t IIR_elliptic_noise_bandpass_fs48000_biquadFloat =
{
  sizeof(IIR_elliptic_noise_bandpass_fs48000_BiquadCellFloat) / sizeof(IIR_elliptic_noise_bandpass_fs48000_BiquadCellFloat[0]), IIR_elliptic_noise_bandpass_fs48000_BiquadCellFloat
};

static const biquadCellInt32_t IIR_elliptic_noise_bandpass_fs48000_BiquadCellInt32[] =
{
  {(int32_t)0x05C5E81CUL, (int32_t)0x00000000UL, (int32_t)0xFA3A17E4UL, (int32_t)0x8D0DB874UL, (int32_t)0x74742FD9UL},
  {(int32_t)0x136C427BUL, (int32_t)0xE18A6A6FUL, (int32_t)0x136C427BUL, (int32_t)0x8F4321FEUL, (int32_t)0x75C47E7DUL},
  {(int32_t)0x3C7EB205UL, (int32_t)0x88E1459EUL, (int32_t)0x3C7EB205UL, (int32_t)0x8915B335UL, (int32_t)0x7878EE22UL},
  {(int32_t)0x3D2DF8BFUL, (int32_t)0x965EF17BUL, (int32_t)0x3D2DF8BFUL, (int32_t)0x8F0CD723UL, (int32_t)0x7A880740UL},
  {(int32_t)0x2EC51EA2UL, (int32_t)0xA4D4F733UL, (int32_t)0x2EC51EA2UL, (int32_t)0x8636758DUL, (int32_t)0x7C9E4A30UL},
  {(int32_t)0x29366597UL, (int32_t)0xB7A845C5UL, (int32_t)0x29366597UL, (int32_t)0x8E1A65FFUL, (int32_t)0x7E6E29A0UL},
  {(int32_t)0x3B70A4D6UL, (int32_t)0x8C8295EBUL, (int32_t)0x3B70A4D6UL, (int32_t)0x84BB81E2UL, (int32_t)0x7F17B308UL}
};

const biquadInt32_t IIR_elliptic_noise_bandpass_fs48000_biquadInt32 =
{
  (int32_t)0x7FFFFFFFUL, 3, sizeof(IIR_elliptic_noise_bandpass_fs48000_BiquadCellInt32) / sizeof(IIR_elliptic_noise_bandpass_fs48000_BiquadCellInt32[0]), IIR_elliptic_noise_bandpass_fs48000_BiquadCellInt32
};

#endif /* AC_SUPPORT_FS_48000 */

