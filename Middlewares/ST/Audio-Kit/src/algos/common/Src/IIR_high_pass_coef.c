#include <stdlib.h> // for NULL definition
#include "common/IIR_high_pass_coef.h"

#ifdef AC_SUPPORT_FS_8000
// 8000 Hz high-pass IIR
//     stop frequency :     50 Hz
//     pass frequency :    200 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_butterworth_DC_remove_fs8000_BiquadCellFloat[] =
{
  {+0.94995506, -0.94995506, +0.00000000, -0.89991012, +0.00000000},
  {+0.91099249, -1.82198499, +0.91099249, -1.81692838, +0.82704159},
  {+0.93591953, -1.87183906, +0.93591953, -1.86664409, +0.87703402},
  {+0.97444922, -1.94889843, +0.97444922, -1.94348960, +0.95430726}
};

const biquadFloat_t IIR_butterworth_DC_remove_fs8000_biquadFloat =
{
  sizeof(IIR_butterworth_DC_remove_fs8000_BiquadCellFloat) / sizeof(IIR_butterworth_DC_remove_fs8000_BiquadCellFloat[0]), IIR_butterworth_DC_remove_fs8000_BiquadCellFloat
};

static const biquadCellInt32_t IIR_butterworth_DC_remove_fs8000_BiquadCellInt32[] =
{
  {(int32_t)0x799820A0UL, (int32_t)0x8667DF60UL, (int32_t)0x00000000UL, (int32_t)0xC667DF60UL, (int32_t)0x00000000UL},
  {(int32_t)0x3A4DB376UL, (int32_t)0x8B649914UL, (int32_t)0x3A4DB376UL, (int32_t)0x8BB77204UL, (int32_t)0x69DC7FB8UL},
  {(int32_t)0x3BE61B06UL, (int32_t)0x8833C9F3UL, (int32_t)0x3BE61B06UL, (int32_t)0x8888E737UL, (int32_t)0x7042A6A3UL},
  {(int32_t)0x3E5D603EUL, (int32_t)0x83453F84UL, (int32_t)0x3E5D603EUL, (int32_t)0x839DDDCCUL, (int32_t)0x7A26BD89UL}
};

const biquadInt32_t IIR_butterworth_DC_remove_fs8000_biquadInt32 =
{
  (int32_t)0x40000000UL, 4, sizeof(IIR_butterworth_DC_remove_fs8000_BiquadCellInt32) / sizeof(IIR_butterworth_DC_remove_fs8000_BiquadCellInt32[0]), IIR_butterworth_DC_remove_fs8000_BiquadCellInt32
};

// 8000 Hz high-pass IIR
//     stop frequency :     50 Hz
//     pass frequency :    200 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_chebyshev1_DC_remove_fs8000_BiquadCellFloat[] =
{
  {+0.87257179, -0.87257179, +0.00000000, -0.74514357, +0.00000000},
  {+0.89472895, -1.78945790, +0.89472895, -1.77202830, +0.80688749},
  {+0.97359589, -1.94719178, +0.97359589, -1.93709851, +0.95728505}
};

const biquadFloat_t IIR_chebyshev1_DC_remove_fs8000_biquadFloat =
{
  sizeof(IIR_chebyshev1_DC_remove_fs8000_BiquadCellFloat) / sizeof(IIR_chebyshev1_DC_remove_fs8000_BiquadCellFloat[0]), IIR_chebyshev1_DC_remove_fs8000_BiquadCellFloat
};

static const biquadCellInt32_t IIR_chebyshev1_DC_remove_fs8000_BiquadCellInt32[] =
{
  {(int32_t)0x6FB06EADUL, (int32_t)0x904F9153UL, (int32_t)0x00000000UL, (int32_t)0xD04F9153UL, (int32_t)0x00000000UL},
  {(int32_t)0x39433D35UL, (int32_t)0x8D798597UL, (int32_t)0x39433D35UL, (int32_t)0x8E971698UL, (int32_t)0x674816D4UL},
  {(int32_t)0x3E4F6522UL, (int32_t)0x836135BDUL, (int32_t)0x3E4F6522UL, (int32_t)0x840693FDUL, (int32_t)0x7A885107UL}
};

const biquadInt32_t IIR_chebyshev1_DC_remove_fs8000_biquadInt32 =
{
  (int32_t)0x7FFFFFFFUL, 2, sizeof(IIR_chebyshev1_DC_remove_fs8000_BiquadCellInt32) / sizeof(IIR_chebyshev1_DC_remove_fs8000_BiquadCellInt32[0]), IIR_chebyshev1_DC_remove_fs8000_BiquadCellInt32
};

// 8000 Hz high-pass IIR
//     stop frequency :     50 Hz
//     pass frequency :    200 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_chebyshev2_DC_remove_fs8000_BiquadCellFloat[] =
{
  {+0.95899850, -0.95899850, +0.00000000, -0.91799700, +0.00000000},
  {+0.93370967, -1.86692182, +0.93370967, -1.86350873, +0.87083244},
  {+0.97253386, -1.94371130, +0.97253386, -1.94015706, +0.94862197}
};

const biquadFloat_t IIR_chebyshev2_DC_remove_fs8000_biquadFloat =
{
  sizeof(IIR_chebyshev2_DC_remove_fs8000_BiquadCellFloat) / sizeof(IIR_chebyshev2_DC_remove_fs8000_BiquadCellFloat[0]), IIR_chebyshev2_DC_remove_fs8000_BiquadCellFloat
};

static const biquadCellInt32_t IIR_chebyshev2_DC_remove_fs8000_BiquadCellInt32[] =
{
  {(int32_t)0x7AC0767CUL, (int32_t)0x853F8984UL, (int32_t)0x00000000UL, (int32_t)0xC53F8984UL, (int32_t)0x00000000UL},
  {(int32_t)0x3BC1E639UL, (int32_t)0x88845A5DUL, (int32_t)0x3BC1E639UL, (int32_t)0x88BC45E6UL, (int32_t)0x6F776FF6UL},
  {(int32_t)0x3E3DFEADUL, (int32_t)0x839A3BEEUL, (int32_t)0x3E3DFEADUL, (int32_t)0x83D4777FUL, (int32_t)0x796C71D7UL}
};

const biquadInt32_t IIR_chebyshev2_DC_remove_fs8000_biquadInt32 =
{
  (int32_t)0x40000000UL, 3, sizeof(IIR_chebyshev2_DC_remove_fs8000_BiquadCellInt32) / sizeof(IIR_chebyshev2_DC_remove_fs8000_BiquadCellInt32[0]), IIR_chebyshev2_DC_remove_fs8000_BiquadCellInt32
};

// 8000 Hz high-pass IIR
//     stop frequency :     50 Hz
//     pass frequency :    200 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_elliptic_DC_remove_fs8000_BiquadCellFloat[] =
{
  {+0.85905039, -1.71779494, +0.85905039, -1.70183450, +0.73406123},
  {+0.95634496, -1.91077400, +0.95634496, -1.92477746, +0.94296010}
};

const biquadFloat_t IIR_elliptic_DC_remove_fs8000_biquadFloat =
{
  sizeof(IIR_elliptic_DC_remove_fs8000_BiquadCellFloat) / sizeof(IIR_elliptic_DC_remove_fs8000_BiquadCellFloat[0]), IIR_elliptic_DC_remove_fs8000_BiquadCellFloat
};

static const biquadCellInt32_t IIR_elliptic_DC_remove_fs8000_BiquadCellInt32[] =
{
  {(int32_t)0x36FAAE7EUL, (int32_t)0x920FA5CCUL, (int32_t)0x36FAAE7EUL, (int32_t)0x931524C3UL, (int32_t)0x5DF5B7E6UL},
  {(int32_t)0x3D34C183UL, (int32_t)0x85B5E0FAUL, (int32_t)0x3D34C183UL, (int32_t)0x84D07235UL, (int32_t)0x78B2EA99UL}
};

const biquadInt32_t IIR_elliptic_DC_remove_fs8000_biquadInt32 =
{
  (int32_t)0x7FFFFFFFUL, 2, sizeof(IIR_elliptic_DC_remove_fs8000_BiquadCellInt32) / sizeof(IIR_elliptic_DC_remove_fs8000_BiquadCellInt32[0]), IIR_elliptic_DC_remove_fs8000_BiquadCellInt32
};

#endif /* AC_SUPPORT_FS_8000 */

#ifdef AC_SUPPORT_FS_12000
// 12000 Hz high-pass IIR
//     stop frequency :     50 Hz
//     pass frequency :    200 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_butterworth_DC_remove_fs12000_BiquadCellFloat[] =
{
  {+0.96607305, -0.96607305, +0.00000000, -0.93214610, +0.00000000},
  {+0.93939539, -1.87879077, +0.93939539, -1.87647366, +0.88110789},
  {+0.95691468, -1.91382937, +0.95691468, -1.91146904, +0.91618970},
  {+0.98341716, -1.96683432, +0.98341716, -1.96440862, +0.96926003}
};

const biquadFloat_t IIR_butterworth_DC_remove_fs12000_biquadFloat =
{
  sizeof(IIR_butterworth_DC_remove_fs12000_BiquadCellFloat) / sizeof(IIR_butterworth_DC_remove_fs12000_BiquadCellFloat[0]), IIR_butterworth_DC_remove_fs12000_BiquadCellFloat
};

static const biquadCellInt32_t IIR_butterworth_DC_remove_fs12000_BiquadCellInt32[] =
{
  {(int32_t)0x7BA84820UL, (int32_t)0x8457B7E0UL, (int32_t)0x00000000UL, (int32_t)0xC457B7E0UL, (int32_t)0x00000000UL},
  {(int32_t)0x3C1F0DD4UL, (int32_t)0x87C1E459UL, (int32_t)0x3C1F0DD4UL, (int32_t)0x87E7DB0BUL, (int32_t)0x70C824B3UL},
  {(int32_t)0x3D3E1716UL, (int32_t)0x8583D1D3UL, (int32_t)0x3D3E1716UL, (int32_t)0x85AA7DC5UL, (int32_t)0x7545B43EUL},
  {(int32_t)0x3EF04E8AUL, (int32_t)0x821F62EDUL, (int32_t)0x3EF04E8AUL, (int32_t)0x8247210EUL, (int32_t)0x7C10B66AUL}
};

const biquadInt32_t IIR_butterworth_DC_remove_fs12000_biquadInt32 =
{
  (int32_t)0x40000000UL, 4, sizeof(IIR_butterworth_DC_remove_fs12000_BiquadCellInt32) / sizeof(IIR_butterworth_DC_remove_fs12000_BiquadCellInt32[0]), IIR_butterworth_DC_remove_fs12000_BiquadCellInt32
};

// 12000 Hz high-pass IIR
//     stop frequency :     50 Hz
//     pass frequency :    200 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_chebyshev1_DC_remove_fs12000_BiquadCellFloat[] =
{
  {+0.91137186, -0.91137186, +0.00000000, -0.82274372, +0.00000000},
  {+0.92921150, -1.85842299, +0.92921150, -1.85039636, +0.86644963},
  {+0.98337483, -1.96674965, +0.98337483, -1.96222906, +0.97127025}
};

const biquadFloat_t IIR_chebyshev1_DC_remove_fs12000_biquadFloat =
{
  sizeof(IIR_chebyshev1_DC_remove_fs12000_BiquadCellFloat) / sizeof(IIR_chebyshev1_DC_remove_fs12000_BiquadCellFloat[0]), IIR_chebyshev1_DC_remove_fs12000_BiquadCellFloat
};

static const biquadCellInt32_t IIR_chebyshev1_DC_remove_fs12000_BiquadCellInt32[] =
{
  {(int32_t)0x74A7D543UL, (int32_t)0x8B582ABDUL, (int32_t)0x00000000UL, (int32_t)0xCB582ABEUL, (int32_t)0x00000000UL},
  {(int32_t)0x3B78337EUL, (int32_t)0x890F9903UL, (int32_t)0x3B78337EUL, (int32_t)0x89931B28UL, (int32_t)0x6EE7D245UL},
  {(int32_t)0x3EEF9CF9UL, (int32_t)0x8220C60FUL, (int32_t)0x3EEF9CF9UL, (int32_t)0x826AD6CEUL, (int32_t)0x7C529561UL}
};

const biquadInt32_t IIR_chebyshev1_DC_remove_fs12000_biquadInt32 =
{
  (int32_t)0x7FFFFFFFUL, 2, sizeof(IIR_chebyshev1_DC_remove_fs12000_BiquadCellInt32) / sizeof(IIR_chebyshev1_DC_remove_fs12000_BiquadCellInt32[0]), IIR_chebyshev1_DC_remove_fs12000_BiquadCellInt32
};

// 12000 Hz high-pass IIR
//     stop frequency :     50 Hz
//     pass frequency :    200 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_chebyshev2_DC_remove_fs12000_BiquadCellFloat[] =
{
  {+0.97228883, -0.97228883, +0.00000000, -0.94457766, +0.00000000},
  {+0.95517821, -1.91013022, +0.95517821, -1.90857852, +0.91190812},
  {+0.98190929, -1.96320988, +0.98190929, -1.96161491, +0.96541355}
};

const biquadFloat_t IIR_chebyshev2_DC_remove_fs12000_biquadFloat =
{
  sizeof(IIR_chebyshev2_DC_remove_fs12000_BiquadCellFloat) / sizeof(IIR_chebyshev2_DC_remove_fs12000_BiquadCellFloat[0]), IIR_chebyshev2_DC_remove_fs12000_BiquadCellFloat
};

static const biquadCellInt32_t IIR_chebyshev2_DC_remove_fs12000_BiquadCellInt32[] =
{
  {(int32_t)0x7C73F5DDUL, (int32_t)0x838C0A23UL, (int32_t)0x00000000UL, (int32_t)0xC38C0A23UL, (int32_t)0x00000000UL},
  {(int32_t)0x3D21A3C9UL, (int32_t)0x85C06D2AUL, (int32_t)0x3D21A3C9UL, (int32_t)0x85D9D977UL, (int32_t)0x74B967BDUL},
  {(int32_t)0x3ED79A10UL, (int32_t)0x825AC4F4UL, (int32_t)0x3ED79A10UL, (int32_t)0x8274E6C0UL, (int32_t)0x7B92ABD8UL}
};

const biquadInt32_t IIR_chebyshev2_DC_remove_fs12000_biquadInt32 =
{
  (int32_t)0x40000000UL, 3, sizeof(IIR_chebyshev2_DC_remove_fs12000_BiquadCellInt32) / sizeof(IIR_chebyshev2_DC_remove_fs12000_BiquadCellInt32[0]), IIR_chebyshev2_DC_remove_fs12000_BiquadCellInt32
};

// 12000 Hz high-pass IIR
//     stop frequency :     50 Hz
//     pass frequency :    200 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_elliptic_DC_remove_fs12000_BiquadCellFloat[] =
{
  {+0.90318123, -1.80621986, +0.90318123, -1.79877861, +0.81380370},
  {+0.96774786, -1.93463577, +0.96774786, -1.95339219, +0.96155330}
};

const biquadFloat_t IIR_elliptic_DC_remove_fs12000_biquadFloat =
{
  sizeof(IIR_elliptic_DC_remove_fs12000_BiquadCellFloat) / sizeof(IIR_elliptic_DC_remove_fs12000_BiquadCellFloat[0]), IIR_elliptic_DC_remove_fs12000_BiquadCellFloat
};

static const biquadCellInt32_t IIR_elliptic_DC_remove_fs12000_BiquadCellInt32[] =
{
  {(int32_t)0x39CDB8A0UL, (int32_t)0x8C66E4D2UL, (int32_t)0x39CDB8A0UL, (int32_t)0x8CE0CFB2UL, (int32_t)0x682AB841UL},
  {(int32_t)0x3DEF94B8UL, (int32_t)0x842EED71UL, (int32_t)0x3DEF94B8UL, (int32_t)0x82FB9F4FUL, (int32_t)0x7B142DABUL}
};

const biquadInt32_t IIR_elliptic_DC_remove_fs12000_biquadInt32 =
{
  (int32_t)0x7FFFFFF8UL, 2, sizeof(IIR_elliptic_DC_remove_fs12000_BiquadCellInt32) / sizeof(IIR_elliptic_DC_remove_fs12000_BiquadCellInt32[0]), IIR_elliptic_DC_remove_fs12000_BiquadCellInt32
};

#endif /* AC_SUPPORT_FS_12000 */

#ifdef AC_SUPPORT_FS_16000
// 16000 Hz high-pass IIR
//     stop frequency :     50 Hz
//     pass frequency :    200 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_butterworth_DC_remove_fs16000_BiquadCellFloat[] =
{
  {+0.97433775, -0.97433775, +0.00000000, -0.94867550, +0.00000000},
  {+0.95405882, -1.90811764, +0.95405882, -1.90679399, +0.90944130},
  {+0.96755139, -1.93510278, +0.96755139, -1.93376041, +0.93644516},
  {+0.98773697, -1.97547394, +0.98773697, -1.97410356, +0.97684433}
};

const biquadFloat_t IIR_butterworth_DC_remove_fs16000_biquadFloat =
{
  sizeof(IIR_butterworth_DC_remove_fs16000_BiquadCellFloat) / sizeof(IIR_butterworth_DC_remove_fs16000_BiquadCellFloat[0]), IIR_butterworth_DC_remove_fs16000_BiquadCellFloat
};

static const biquadCellInt32_t IIR_butterworth_DC_remove_fs16000_BiquadCellInt32[] =
{
  {(int32_t)0x7CB7196CUL, (int32_t)0x8348E694UL, (int32_t)0x00000000UL, (int32_t)0xC348E694UL, (int32_t)0x00000000UL},
  {(int32_t)0x3D0F4CBCUL, (int32_t)0x85E16689UL, (int32_t)0x3D0F4CBCUL, (int32_t)0x85F7165BUL, (int32_t)0x74689294UL},
  {(int32_t)0x3DEC5CACUL, (int32_t)0x842746A9UL, (int32_t)0x3DEC5CACUL, (int32_t)0x843D44FFUL, (int32_t)0x77DD6F5BUL},
  {(int32_t)0x3F371522UL, (int32_t)0x8191D5BCUL, (int32_t)0x3F371522UL, (int32_t)0x81A84989UL, (int32_t)0x7D093C21UL}
};

const biquadInt32_t IIR_butterworth_DC_remove_fs16000_biquadInt32 =
{
  (int32_t)0x40000000UL, 4, sizeof(IIR_butterworth_DC_remove_fs16000_BiquadCellInt32) / sizeof(IIR_butterworth_DC_remove_fs16000_BiquadCellInt32[0]), IIR_butterworth_DC_remove_fs16000_BiquadCellInt32
};

// 16000 Hz high-pass IIR
//     stop frequency :     50 Hz
//     pass frequency :    200 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_chebyshev1_DC_remove_fs16000_BiquadCellFloat[] =
{
  {+0.93204807, -0.93204807, +0.00000000, -0.86409614, +0.00000000},
  {+0.94669841, -1.89339681, +0.94669841, -1.88880054, +0.89799308},
  {+0.98790479, -1.97580959, +0.98790479, -1.97325708, +0.97836209}
};

const biquadFloat_t IIR_chebyshev1_DC_remove_fs16000_biquadFloat =
{
  sizeof(IIR_chebyshev1_DC_remove_fs16000_BiquadCellFloat) / sizeof(IIR_chebyshev1_DC_remove_fs16000_BiquadCellFloat[0]), IIR_chebyshev1_DC_remove_fs16000_BiquadCellFloat
};

static const biquadCellInt32_t IIR_chebyshev1_DC_remove_fs16000_BiquadCellInt32[] =
{
  {(int32_t)0x774D59E6UL, (int32_t)0x88B2A61AUL, (int32_t)0x00000000UL, (int32_t)0xC8B2A61BUL, (int32_t)0x00000000UL},
  {(int32_t)0x3C96B4E9UL, (int32_t)0x86D2962FUL, (int32_t)0x3C96B4E9UL, (int32_t)0x871DE455UL, (int32_t)0x72F16FF0UL},
  {(int32_t)0x3F39D506UL, (int32_t)0x818C55F4UL, (int32_t)0x3F39D506UL, (int32_t)0x81B627EFUL, (int32_t)0x7D3AF80DUL}
};

const biquadInt32_t IIR_chebyshev1_DC_remove_fs16000_biquadInt32 =
{
  (int32_t)0x7FFFFFFFUL, 2, sizeof(IIR_chebyshev1_DC_remove_fs16000_BiquadCellInt32) / sizeof(IIR_chebyshev1_DC_remove_fs16000_BiquadCellInt32[0]), IIR_chebyshev1_DC_remove_fs16000_BiquadCellInt32
};

// 16000 Hz high-pass IIR
//     stop frequency :     50 Hz
//     pass frequency :    200 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_chebyshev2_DC_remove_fs16000_BiquadCellFloat[] =
{
  {+0.97907215, -0.97907215, +0.00000000, -0.95814430, +0.00000000},
  {+0.96614482, -1.93216095, +0.96614482, -1.93127812, +0.93317248},
  {+0.98651795, -1.97269189, +0.98651795, -1.97179049, +0.97393730}
};

const biquadFloat_t IIR_chebyshev2_DC_remove_fs16000_biquadFloat =
{
  sizeof(IIR_chebyshev2_DC_remove_fs16000_BiquadCellFloat) / sizeof(IIR_chebyshev2_DC_remove_fs16000_BiquadCellFloat[0]), IIR_chebyshev2_DC_remove_fs16000_BiquadCellFloat
};

static const biquadCellInt32_t IIR_chebyshev2_DC_remove_fs16000_BiquadCellInt32[] =
{
  {(int32_t)0x7D523C73UL, (int32_t)0x82ADC38DUL, (int32_t)0x00000000UL, (int32_t)0xC2ADC38DUL, (int32_t)0x00000000UL},
  {(int32_t)0x3DD55118UL, (int32_t)0x84577997UL, (int32_t)0x3DD55118UL, (int32_t)0x8465F073UL, (int32_t)0x77723219UL},
  {(int32_t)0x3F231C2EUL, (int32_t)0x81BF6A83UL, (int32_t)0x3F231C2EUL, (int32_t)0x81CE2F40UL, (int32_t)0x7CA9FA34UL}
};

const biquadInt32_t IIR_chebyshev2_DC_remove_fs16000_biquadInt32 =
{
  (int32_t)0x40000000UL, 3, sizeof(IIR_chebyshev2_DC_remove_fs16000_BiquadCellInt32) / sizeof(IIR_chebyshev2_DC_remove_fs16000_BiquadCellInt32[0]), IIR_chebyshev2_DC_remove_fs16000_BiquadCellInt32
};

// 16000 Hz high-pass IIR
//     stop frequency :     50 Hz
//     pass frequency :    200 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_elliptic_DC_remove_fs16000_BiquadCellFloat[] =
{
  {+0.92627386, -1.85246553, +0.92627386, -1.84817617, +0.85683709},
  {+0.97320598, -1.94592586, +0.97320598, -1.96639786, +0.97101112}
};

const biquadFloat_t IIR_elliptic_DC_remove_fs16000_biquadFloat =
{
  sizeof(IIR_elliptic_DC_remove_fs16000_BiquadCellFloat) / sizeof(IIR_elliptic_DC_remove_fs16000_BiquadCellFloat[0]), IIR_elliptic_DC_remove_fs16000_BiquadCellFloat
};

static const biquadCellInt32_t IIR_elliptic_DC_remove_fs16000_BiquadCellInt32[] =
{
  {(int32_t)0x3B48122CUL, (int32_t)0x89713467UL, (int32_t)0x3B48122CUL, (int32_t)0x89B77B4DUL, (int32_t)0x6DACD67EUL},
  {(int32_t)0x3E4901C3UL, (int32_t)0x8375F35CUL, (int32_t)0x3E4901C3UL, (int32_t)0x8226899AUL, (int32_t)0x7C4A17AEUL}
};

const biquadInt32_t IIR_elliptic_DC_remove_fs16000_biquadInt32 =
{
  (int32_t)0x7FFFFFFDUL, 2, sizeof(IIR_elliptic_DC_remove_fs16000_BiquadCellInt32) / sizeof(IIR_elliptic_DC_remove_fs16000_BiquadCellInt32[0]), IIR_elliptic_DC_remove_fs16000_BiquadCellInt32
};

#endif /* AC_SUPPORT_FS_16000 */

#ifdef AC_SUPPORT_FS_24000
// 24000 Hz high-pass IIR
//     stop frequency :     50 Hz
//     pass frequency :    200 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_butterworth_DC_remove_fs24000_BiquadCellFloat[] =
{
  {+0.98274453, -0.98274453, +0.00000000, -0.96548905, +0.00000000},
  {+0.96904151, -1.93808302, +0.96904151, -1.93748551, +0.93868053},
  {+0.97827895, -1.95655790, +0.97827895, -1.95595469, +0.95716110},
  {+0.99194290, -1.98388580, +0.99194290, -1.98327417, +0.98449743}
};

const biquadFloat_t IIR_butterworth_DC_remove_fs24000_biquadFloat =
{
  sizeof(IIR_butterworth_DC_remove_fs24000_BiquadCellFloat) / sizeof(IIR_butterworth_DC_remove_fs24000_BiquadCellFloat[0]), IIR_butterworth_DC_remove_fs24000_BiquadCellFloat
};

static const biquadCellInt32_t IIR_butterworth_DC_remove_fs24000_BiquadCellInt32[] =
{
  {(int32_t)0x7DCA929AUL, (int32_t)0x82356D66UL, (int32_t)0x00000000UL, (int32_t)0xC2356D66UL, (int32_t)0x00000000UL},
  {(int32_t)0x3E04C6AFUL, (int32_t)0x83F672A2UL, (int32_t)0x3E04C6AFUL, (int32_t)0x84003CC5UL, (int32_t)0x7826AF01UL},
  {(int32_t)0x3E9C1F4EUL, (int32_t)0x82C7C163UL, (int32_t)0x3E9C1F4EUL, (int32_t)0x82D1A36AUL, (int32_t)0x7A844148UL},
  {(int32_t)0x3F7BFE11UL, (int32_t)0x810803DEUL, (int32_t)0x3F7BFE11UL, (int32_t)0x8112093BUL, (int32_t)0x7E0402FFUL}
};

const biquadInt32_t IIR_butterworth_DC_remove_fs24000_biquadInt32 =
{
  (int32_t)0x40000000UL, 4, sizeof(IIR_butterworth_DC_remove_fs24000_BiquadCellInt32) / sizeof(IIR_butterworth_DC_remove_fs24000_BiquadCellInt32[0]), IIR_butterworth_DC_remove_fs24000_BiquadCellInt32
};

// 24000 Hz high-pass IIR
//     stop frequency :     50 Hz
//     pass frequency :    200 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_chebyshev1_DC_remove_fs24000_BiquadCellFloat[] =
{
  {+0.95366146, -0.95366146, +0.00000000, -0.90732292, +0.00000000},
  {+0.96433434, -1.92866867, +0.96433434, -1.92658902, +0.93074833},
  {+0.99218880, -1.98437760, +0.99218880, -1.98323888, +0.98551631}
};

const biquadFloat_t IIR_chebyshev1_DC_remove_fs24000_biquadFloat =
{
  sizeof(IIR_chebyshev1_DC_remove_fs24000_BiquadCellFloat) / sizeof(IIR_chebyshev1_DC_remove_fs24000_BiquadCellFloat[0]), IIR_chebyshev1_DC_remove_fs24000_BiquadCellFloat
};

static const biquadCellInt32_t IIR_chebyshev1_DC_remove_fs24000_BiquadCellInt32[] =
{
  {(int32_t)0x7A119422UL, (int32_t)0x85EE6BDEUL, (int32_t)0x00000000UL, (int32_t)0xC5EE6BDEUL, (int32_t)0x00000000UL},
  {(int32_t)0x3DB7A75EUL, (int32_t)0x8490B143UL, (int32_t)0x3DB7A75EUL, (int32_t)0x84B2C3F5UL, (int32_t)0x7722C2DDUL},
  {(int32_t)0x3F800572UL, (int32_t)0x80FFF51DUL, (int32_t)0x3F800572UL, (int32_t)0x81129D3BUL, (int32_t)0x7E256602UL}
};

const biquadInt32_t IIR_chebyshev1_DC_remove_fs24000_biquadInt32 =
{
  (int32_t)0x7FFFFFFFUL, 2, sizeof(IIR_chebyshev1_DC_remove_fs24000_BiquadCellInt32) / sizeof(IIR_chebyshev1_DC_remove_fs24000_BiquadCellInt32[0]), IIR_chebyshev1_DC_remove_fs24000_BiquadCellInt32
};

// 24000 Hz high-pass IIR
//     stop frequency :     50 Hz
//     pass frequency :    200 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_chebyshev2_DC_remove_fs24000_BiquadCellFloat[] =
{
  {+0.98595033, -0.98595033, +0.00000000, -0.97190067, +0.00000000},
  {+0.97726922, -1.95448058, +0.97726922, -1.95408370, +0.95493531},
  {+0.99107083, -1.98198807, +0.99107083, -1.98158559, +0.98254414}
};

const biquadFloat_t IIR_chebyshev2_DC_remove_fs24000_biquadFloat =
{
  sizeof(IIR_chebyshev2_DC_remove_fs24000_BiquadCellFloat) / sizeof(IIR_chebyshev2_DC_remove_fs24000_BiquadCellFloat[0]), IIR_chebyshev2_DC_remove_fs24000_BiquadCellFloat
};

static const biquadCellInt32_t IIR_chebyshev2_DC_remove_fs24000_BiquadCellInt32[] =
{
  {(int32_t)0x7E339EDCUL, (int32_t)0x81CC6124UL, (int32_t)0x00000000UL, (int32_t)0xC1CC6124UL, (int32_t)0x00000000UL},
  {(int32_t)0x3E8B942EUL, (int32_t)0x82E9CA4DUL, (int32_t)0x3E8B942EUL, (int32_t)0x82F04AF0UL, (int32_t)0x7A3B51FDUL},
  {(int32_t)0x3F6DB45DUL, (int32_t)0x81271B85UL, (int32_t)0x3F6DB45DUL, (int32_t)0x812DB3A0UL, (int32_t)0x7DC401A8UL}
};

const biquadInt32_t IIR_chebyshev2_DC_remove_fs24000_biquadInt32 =
{
  (int32_t)0x40000000UL, 3, sizeof(IIR_chebyshev2_DC_remove_fs24000_BiquadCellInt32) / sizeof(IIR_chebyshev2_DC_remove_fs24000_BiquadCellInt32[0]), IIR_chebyshev2_DC_remove_fs24000_BiquadCellInt32
};

// 24000 Hz high-pass IIR
//     stop frequency :     50 Hz
//     pass frequency :    200 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_elliptic_DC_remove_fs24000_BiquadCellFloat[] =
{
  {+0.95008835, -1.90013926, +0.95008835, -1.89818495, +0.90213101},
  {+0.97849596, -1.95677481, +0.97849596, -1.97851279, +0.98057324}
};

const biquadFloat_t IIR_elliptic_DC_remove_fs24000_biquadFloat =
{
  sizeof(IIR_elliptic_DC_remove_fs24000_BiquadCellFloat) / sizeof(IIR_elliptic_DC_remove_fs24000_BiquadCellFloat[0]), IIR_elliptic_DC_remove_fs24000_BiquadCellFloat
};

static const biquadCellInt32_t IIR_elliptic_DC_remove_fs24000_BiquadCellInt32[] =
{
  {(int32_t)0x3CCE3F62UL, (int32_t)0x86641E4FUL, (int32_t)0x3CCE3F62UL, (int32_t)0x86842341UL, (int32_t)0x7379076CUL},
  {(int32_t)0x3E9FAD89UL, (int32_t)0x82C43391UL, (int32_t)0x3E9FAD89UL, (int32_t)0x81600BE8UL, (int32_t)0x7D836C80UL}
};

const biquadInt32_t IIR_elliptic_DC_remove_fs24000_biquadInt32 =
{
  (int32_t)0x7FFFFFFFUL, 2, sizeof(IIR_elliptic_DC_remove_fs24000_BiquadCellInt32) / sizeof(IIR_elliptic_DC_remove_fs24000_BiquadCellInt32[0]), IIR_elliptic_DC_remove_fs24000_BiquadCellInt32
};

#endif /* AC_SUPPORT_FS_24000 */

#ifdef AC_SUPPORT_FS_32000
// 32000 Hz high-pass IIR
//     stop frequency :     50 Hz
//     pass frequency :    200 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_butterworth_DC_remove_fs32000_BiquadCellFloat[] =
{
  {+0.98700241, -0.98700241, +0.00000000, -0.97400481, +0.00000000},
  {+0.97665530, -1.95331061, +0.97665530, -1.95297187, +0.95364935},
  {+0.98367630, -1.96735260, +0.98367630, -1.96701142, +0.96769377},
  {+0.99400213, -1.98800425, +0.99400213, -1.98765950, +0.98834901}
};

const biquadFloat_t IIR_butterworth_DC_remove_fs32000_biquadFloat =
{
  sizeof(IIR_butterworth_DC_remove_fs32000_BiquadCellFloat) / sizeof(IIR_butterworth_DC_remove_fs32000_BiquadCellFloat[0]), IIR_butterworth_DC_remove_fs32000_BiquadCellFloat
};

static const biquadCellInt32_t IIR_butterworth_DC_remove_fs32000_BiquadCellInt32[] =
{
  {(int32_t)0x7E561846UL, (int32_t)0x81A9E7BAUL, (int32_t)0x00000000UL, (int32_t)0xC1A9E7BAUL, (int32_t)0x00000000UL},
  {(int32_t)0x3E818540UL, (int32_t)0x82FCF57FUL, (int32_t)0x3E818540UL, (int32_t)0x83028242UL, (int32_t)0x7A112E87UL},
  {(int32_t)0x3EF48D6EUL, (int32_t)0x8216E524UL, (int32_t)0x3EF48D6EUL, (int32_t)0x821C7C1DUL, (int32_t)0x7BDD63AAUL},
  {(int32_t)0x3F9DBB19UL, (int32_t)0x80C489CEUL, (int32_t)0x3F9DBB19UL, (int32_t)0x80CA2FCDUL, (int32_t)0x7E823862UL}
};

const biquadInt32_t IIR_butterworth_DC_remove_fs32000_biquadInt32 =
{
  (int32_t)0x40000000UL, 4, sizeof(IIR_butterworth_DC_remove_fs32000_BiquadCellInt32) / sizeof(IIR_butterworth_DC_remove_fs32000_BiquadCellInt32[0]), IIR_butterworth_DC_remove_fs32000_BiquadCellInt32
};

// 32000 Hz high-pass IIR
//     stop frequency :     50 Hz
//     pass frequency :    200 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_chebyshev1_DC_remove_fs32000_BiquadCellFloat[] =
{
  {+0.96484215, -0.96484215, +0.00000000, -0.92968431, +0.00000000},
  {+0.97320421, -1.94640843, +0.97320421, -1.94522810, +0.94758876},
  {+0.99423709, -1.98847417, +0.99423709, -1.98783245, +0.98911590}
};

const biquadFloat_t IIR_chebyshev1_DC_remove_fs32000_biquadFloat =
{
  sizeof(IIR_chebyshev1_DC_remove_fs32000_BiquadCellFloat) / sizeof(IIR_chebyshev1_DC_remove_fs32000_BiquadCellFloat[0]), IIR_chebyshev1_DC_remove_fs32000_BiquadCellFloat
};

static const biquadCellInt32_t IIR_chebyshev1_DC_remove_fs32000_BiquadCellInt32[] =
{
  {(int32_t)0x7B7FF29EUL, (int32_t)0x84800D62UL, (int32_t)0x00000000UL, (int32_t)0xC4800D62UL, (int32_t)0x00000000UL},
  {(int32_t)0x3E48FA54UL, (int32_t)0x836E0B58UL, (int32_t)0x3E48FA54UL, (int32_t)0x83816200UL, (int32_t)0x794A96A0UL},
  {(int32_t)0x3FA19498UL, (int32_t)0x80BCD6D1UL, (int32_t)0x3FA19498UL, (int32_t)0x80C75A63UL, (int32_t)0x7E9B5984UL}
};

const biquadInt32_t IIR_chebyshev1_DC_remove_fs32000_biquadInt32 =
{
  (int32_t)0x7FFFFFFFUL, 2, sizeof(IIR_chebyshev1_DC_remove_fs32000_BiquadCellInt32) / sizeof(IIR_chebyshev1_DC_remove_fs32000_BiquadCellInt32[0]), IIR_chebyshev1_DC_remove_fs32000_BiquadCellInt32
};

// 32000 Hz high-pass IIR
//     stop frequency :     50 Hz
//     pass frequency :    200 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_chebyshev2_DC_remove_fs32000_BiquadCellFloat[] =
{
  {+0.98942567, -0.98942567, +0.00000000, -0.97885135, +0.00000000},
  {+0.98289126, -1.96574978, +0.98289126, -1.96552525, +0.96600704},
  {+0.99332564, -1.98656468, +0.99332564, -1.98633777, +0.98687818}
};

const biquadFloat_t IIR_chebyshev2_DC_remove_fs32000_biquadFloat =
{
  sizeof(IIR_chebyshev2_DC_remove_fs32000_BiquadCellFloat) / sizeof(IIR_chebyshev2_DC_remove_fs32000_BiquadCellFloat[0]), IIR_chebyshev2_DC_remove_fs32000_BiquadCellFloat
};

static const biquadCellInt32_t IIR_chebyshev2_DC_remove_fs32000_BiquadCellInt32[] =
{
  {(int32_t)0x7EA58021UL, (int32_t)0x815A7FDFUL, (int32_t)0x00000000UL, (int32_t)0xC15A7FDFUL, (int32_t)0x00000000UL},
  {(int32_t)0x3EE7B0B9UL, (int32_t)0x823127D5UL, (int32_t)0x3EE7B0B9UL, (int32_t)0x8234D592UL, (int32_t)0x7BA61E60UL},
  {(int32_t)0x3F92A5B0UL, (int32_t)0x80DC1FD6UL, (int32_t)0x3F92A5B0UL, (int32_t)0x80DFD78FUL, (int32_t)0x7E520635UL}
};

const biquadInt32_t IIR_chebyshev2_DC_remove_fs32000_biquadInt32 =
{
  (int32_t)0x40000000UL, 3, sizeof(IIR_chebyshev2_DC_remove_fs32000_BiquadCellInt32) / sizeof(IIR_chebyshev2_DC_remove_fs32000_BiquadCellInt32[0]), IIR_chebyshev2_DC_remove_fs32000_BiquadCellInt32
};

// 32000 Hz high-pass IIR
//     stop frequency :     50 Hz
//     pass frequency :    200 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_elliptic_DC_remove_fs32000_BiquadCellFloat[] =
{
  {+0.96227533, -1.92452932, +0.96227533, -1.92341614, +0.92566383},
  {+0.98107647, -1.96203052, +0.98107647, -1.98423075, +0.98539261}
};

const biquadFloat_t IIR_elliptic_DC_remove_fs32000_biquadFloat =
{
  sizeof(IIR_elliptic_DC_remove_fs32000_BiquadCellFloat) / sizeof(IIR_elliptic_DC_remove_fs32000_BiquadCellFloat[0]), IIR_elliptic_DC_remove_fs32000_BiquadCellFloat
};

static const biquadCellInt32_t IIR_elliptic_DC_remove_fs32000_BiquadCellInt32[] =
{
  {(int32_t)0x3D95EB40UL, (int32_t)0x84D482F8UL, (int32_t)0x3D95EB40UL, (int32_t)0x84E6BFFBUL, (int32_t)0x767C2706UL},
  {(int32_t)0x3EC9F502UL, (int32_t)0x826E177AUL, (int32_t)0x3EC9F502UL, (int32_t)0x81025D04UL, (int32_t)0x7E215853UL}
};

const biquadInt32_t IIR_elliptic_DC_remove_fs32000_biquadInt32 =
{
  (int32_t)0x7FFFFFEBUL, 2, sizeof(IIR_elliptic_DC_remove_fs32000_BiquadCellInt32) / sizeof(IIR_elliptic_DC_remove_fs32000_BiquadCellInt32[0]), IIR_elliptic_DC_remove_fs32000_BiquadCellInt32
};

#endif /* AC_SUPPORT_FS_32000 */

#ifdef AC_SUPPORT_FS_48000
// 48000 Hz high-pass IIR
//     stop frequency :     50 Hz
//     pass frequency :    200 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_butterworth_DC_remove_fs48000_BiquadCellFloat[] =
{
  {+0.99129727, -0.99129727, +0.00000000, -0.98259454, +0.00000000},
  {+0.98435222, -1.96870445, +0.98435222, -1.96855271, +0.96885618},
  {+0.98909574, -1.97819148, +0.98909574, -1.97803902, +0.97834395},
  {+0.99603166, -1.99206331, +0.99603166, -1.99190978, +0.99221685}
};

const biquadFloat_t IIR_butterworth_DC_remove_fs48000_biquadFloat =
{
  sizeof(IIR_butterworth_DC_remove_fs48000_BiquadCellFloat) / sizeof(IIR_butterworth_DC_remove_fs48000_BiquadCellFloat[0]), IIR_butterworth_DC_remove_fs48000_BiquadCellFloat
};

static const biquadCellInt32_t IIR_butterworth_DC_remove_fs48000_BiquadCellInt32[] =
{
  {(int32_t)0x7EE2D436UL, (int32_t)0x811D2BCAUL, (int32_t)0x00000000UL, (int32_t)0xC11D2BCAUL, (int32_t)0x00000000UL},
  {(int32_t)0x3EFFA077UL, (int32_t)0x8200BF12UL, (int32_t)0x3EFFA077UL, (int32_t)0x82033B7EUL, (int32_t)0x7C037AB2UL},
  {(int32_t)0x3F4D583AUL, (int32_t)0x81654F8CUL, (int32_t)0x3F4D583AUL, (int32_t)0x8167CF09UL, (int32_t)0x7D3A5FE1UL},
  {(int32_t)0x3FBEFB8FUL, (int32_t)0x808208E2UL, (int32_t)0x3FBEFB8FUL, (int32_t)0x80848CDAUL, (int32_t)0x7F00F62DUL}
};

const biquadInt32_t IIR_butterworth_DC_remove_fs48000_biquadInt32 =
{
  (int32_t)0x40000000UL, 4, sizeof(IIR_butterworth_DC_remove_fs48000_BiquadCellInt32) / sizeof(IIR_butterworth_DC_remove_fs48000_BiquadCellInt32[0]), IIR_butterworth_DC_remove_fs48000_BiquadCellInt32
};

// 48000 Hz high-pass IIR
//     stop frequency :     50 Hz
//     pass frequency :    200 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_chebyshev1_DC_remove_fs48000_BiquadCellFloat[] =
{
  {+0.97628515, -0.97628515, +0.00000000, -0.95257030, +0.00000000},
  {+0.98210638, -1.96421276, +0.98210638, -1.96368345, +0.96474208},
  {+0.99622211, -1.99244422, +0.99622211, -1.99215849, +0.99272996}
};

const biquadFloat_t IIR_chebyshev1_DC_remove_fs48000_biquadFloat =
{
  sizeof(IIR_chebyshev1_DC_remove_fs48000_BiquadCellFloat) / sizeof(IIR_chebyshev1_DC_remove_fs48000_BiquadCellFloat[0]), IIR_chebyshev1_DC_remove_fs48000_BiquadCellFloat
};

static const biquadCellInt32_t IIR_chebyshev1_DC_remove_fs48000_BiquadCellInt32[] =
{
  {(int32_t)0x7CF6E96AUL, (int32_t)0x83091696UL, (int32_t)0x00000000UL, (int32_t)0xC3091696UL, (int32_t)0x00000000UL},
  {(int32_t)0x3EDAD4B9UL, (int32_t)0x824A568EUL, (int32_t)0x3EDAD4B9UL, (int32_t)0x825302A8UL, (int32_t)0x7B7CAB17UL},
  {(int32_t)0x3FC21A64UL, (int32_t)0x807BCB38UL, (int32_t)0x3FC21A64UL, (int32_t)0x808079B1UL, (int32_t)0x7F11C682UL}
};

const biquadInt32_t IIR_chebyshev1_DC_remove_fs48000_biquadInt32 =
{
  (int32_t)0x7FFFFFFFUL, 2, sizeof(IIR_chebyshev1_DC_remove_fs48000_BiquadCellInt32) / sizeof(IIR_chebyshev1_DC_remove_fs48000_BiquadCellInt32[0]), IIR_chebyshev1_DC_remove_fs48000_BiquadCellInt32
};

// 48000 Hz high-pass IIR
//     stop frequency :     50 Hz
//     pass frequency :    200 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_chebyshev2_DC_remove_fs48000_BiquadCellFloat[] =
{
  {+0.99292554, -0.99292554, +0.00000000, -0.98585109, +0.00000000},
  {+0.98855355, -1.97709247, +0.98855355, -1.97699211, +0.97720747},
  {+0.99556563, -1.99109268, +0.99556563, -1.99099160, +0.99123233}
};

const biquadFloat_t IIR_chebyshev2_DC_remove_fs48000_biquadFloat =
{
  sizeof(IIR_chebyshev2_DC_remove_fs48000_BiquadCellFloat) / sizeof(IIR_chebyshev2_DC_remove_fs48000_BiquadCellFloat[0]), IIR_chebyshev2_DC_remove_fs48000_BiquadCellFloat
};

static const biquadCellInt32_t IIR_chebyshev2_DC_remove_fs48000_BiquadCellInt32[] =
{
  {(int32_t)0x7F182F2BUL, (int32_t)0x80E7D0D5UL, (int32_t)0x00000000UL, (int32_t)0xC0E7D0D5UL, (int32_t)0x00000000UL},
  {(int32_t)0x3F44761FUL, (int32_t)0x81775120UL, (int32_t)0x3F44761FUL, (int32_t)0x8178F616UL, (int32_t)0x7D152266UL},
  {(int32_t)0x3FB758E3UL, (int32_t)0x8091F006UL, (int32_t)0x3FB758E3UL, (int32_t)0x809397F7UL, (int32_t)0x7EE0B36DUL}
};

const biquadInt32_t IIR_chebyshev2_DC_remove_fs48000_biquadInt32 =
{
  (int32_t)0x40000000UL, 3, sizeof(IIR_chebyshev2_DC_remove_fs48000_BiquadCellInt32) / sizeof(IIR_chebyshev2_DC_remove_fs48000_BiquadCellInt32[0]), IIR_chebyshev2_DC_remove_fs48000_BiquadCellInt32
};

// 48000 Hz high-pass IIR
//     stop frequency :     50 Hz
//     pass frequency :    200 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_elliptic_DC_remove_fs48000_BiquadCellFloat[] =
{
  {+0.97465373, -1.94929785, +0.97465373, -1.94879681, +0.94980849},
  {+0.98361322, -1.96717190, +0.98361322, -1.98971944, +0.99023709}
};

const biquadFloat_t IIR_elliptic_DC_remove_fs48000_biquadFloat =
{
  sizeof(IIR_elliptic_DC_remove_fs48000_BiquadCellFloat) / sizeof(IIR_elliptic_DC_remove_fs48000_BiquadCellFloat[0]), IIR_elliptic_DC_remove_fs48000_BiquadCellFloat
};

static const biquadCellInt32_t IIR_elliptic_DC_remove_fs48000_BiquadCellInt32[] =
{
  {(int32_t)0x3E60BA06UL, (int32_t)0x833EB43AUL, (int32_t)0x3E60BA06UL, (int32_t)0x8346E9BEUL, (int32_t)0x7993531FUL},
  {(int32_t)0x3EF384F4UL, (int32_t)0x8219DADFUL, (int32_t)0x3EF384F4UL, (int32_t)0x80A86FCFUL, (int32_t)0x7EC016C1UL}
};

const biquadInt32_t IIR_elliptic_DC_remove_fs48000_biquadInt32 =
{
  (int32_t)0x7FFFFFD3UL, 2, sizeof(IIR_elliptic_DC_remove_fs48000_BiquadCellInt32) / sizeof(IIR_elliptic_DC_remove_fs48000_BiquadCellInt32[0]), IIR_elliptic_DC_remove_fs48000_BiquadCellInt32
};

#endif /* AC_SUPPORT_FS_48000 */

#ifdef AC_SUPPORT_FS_96000
// 96000 Hz high-pass IIR
//     stop frequency :     50 Hz
//     pass frequency :    200 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_butterworth_DC_remove_fs96000_BiquadCellFloat[] =
{
  {+0.99562963, -0.99562963, +0.00000000, -0.99125926, +0.00000000},
  {+0.99213340, -1.98426680, +0.99213340, -1.98422857, +0.98430504},
  {+0.99453705, -1.98907411, +0.99453705, -1.98903578, +0.98911244},
  {+0.99803108, -1.99606216, +0.99803108, -1.99602370, +0.99610062}
};

const biquadFloat_t IIR_butterworth_DC_remove_fs96000_biquadFloat =
{
  sizeof(IIR_butterworth_DC_remove_fs96000_BiquadCellFloat) / sizeof(IIR_butterworth_DC_remove_fs96000_BiquadCellFloat[0]), IIR_butterworth_DC_remove_fs96000_BiquadCellFloat
};

static const biquadCellInt32_t IIR_butterworth_DC_remove_fs96000_BiquadCellInt32[] =
{
  {(int32_t)0x7F70CAADUL, (int32_t)0x808F3553UL, (int32_t)0x00000000UL, (int32_t)0xC08F3553UL, (int32_t)0x00000000UL},
  {(int32_t)0x3F7F1D19UL, (int32_t)0x8101C5CEUL, (int32_t)0x3F7F1D19UL, (int32_t)0x8102662BUL, (int32_t)0x7DFDB51DUL},
  {(int32_t)0x3FA67EBFUL, (int32_t)0x80B30282UL, (int32_t)0x3FA67EBFUL, (int32_t)0x80B3A342UL, (int32_t)0x7E9B3C7DUL},
  {(int32_t)0x3FDFBDC1UL, (int32_t)0x8040847EUL, (int32_t)0x3FDFBDC1UL, (int32_t)0x804125CFUL, (int32_t)0x7F8039A5UL}
};

const biquadInt32_t IIR_butterworth_DC_remove_fs96000_biquadInt32 =
{
  (int32_t)0x40000000UL, 4, sizeof(IIR_butterworth_DC_remove_fs96000_BiquadCellInt32) / sizeof(IIR_butterworth_DC_remove_fs96000_BiquadCellInt32[0]), IIR_butterworth_DC_remove_fs96000_BiquadCellInt32
};

// 96000 Hz high-pass IIR
//     stop frequency :     50 Hz
//     pass frequency :    200 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_chebyshev1_DC_remove_fs96000_BiquadCellFloat[] =
{
  {+0.98800080, -0.98800080, +0.00000000, -0.97600159, +0.00000000},
  {+0.99103897, -1.98207794, +0.99103897, -1.98194442, +0.98221146},
  {+0.99814328, -1.99628656, +0.99814328, -1.99621500, +0.99635813}
};

const biquadFloat_t IIR_chebyshev1_DC_remove_fs96000_biquadFloat =
{
  sizeof(IIR_chebyshev1_DC_remove_fs96000_BiquadCellFloat) / sizeof(IIR_chebyshev1_DC_remove_fs96000_BiquadCellFloat[0]), IIR_chebyshev1_DC_remove_fs96000_BiquadCellFloat
};

static const biquadCellInt32_t IIR_chebyshev1_DC_remove_fs96000_BiquadCellInt32[] =
{
  {(int32_t)0x7E76CF64UL, (int32_t)0x8189309CUL, (int32_t)0x00000000UL, (int32_t)0xC189309CUL, (int32_t)0x00000000UL},
  {(int32_t)0x3F6D2EB6UL, (int32_t)0x8125A294UL, (int32_t)0x3F6D2EB6UL, (int32_t)0x8127D29AUL, (int32_t)0x7DB91AE6UL},
  {(int32_t)0x3FE1945BUL, (int32_t)0x803CD74AUL, (int32_t)0x3FE1945BUL, (int32_t)0x803E0376UL, (int32_t)0x7F88A9C2UL}
};

const biquadInt32_t IIR_chebyshev1_DC_remove_fs96000_biquadInt32 =
{
  (int32_t)0x7FFFFFFFUL, 2, sizeof(IIR_chebyshev1_DC_remove_fs96000_BiquadCellInt32) / sizeof(IIR_chebyshev1_DC_remove_fs96000_BiquadCellInt32[0]), IIR_chebyshev1_DC_remove_fs96000_BiquadCellInt32
};

// 96000 Hz high-pass IIR
//     stop frequency :     50 Hz
//     pass frequency :    200 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_chebyshev2_DC_remove_fs96000_BiquadCellFloat[] =
{
  {+0.99645023, -0.99645023, +0.00000000, -0.99290045, +0.00000000},
  {+0.99425638, -1.98850908, +0.99425638, -1.98848384, +0.98853799},
  {+0.99779051, -1.99557136, +0.99779051, -1.99554603, +0.99560635}
};

const biquadFloat_t IIR_chebyshev2_DC_remove_fs96000_biquadFloat =
{
  sizeof(IIR_chebyshev2_DC_remove_fs96000_BiquadCellFloat) / sizeof(IIR_chebyshev2_DC_remove_fs96000_BiquadCellFloat[0]), IIR_chebyshev2_DC_remove_fs96000_BiquadCellFloat
};

static const biquadCellInt32_t IIR_chebyshev2_DC_remove_fs96000_BiquadCellInt32[] =
{
  {(int32_t)0x7F8BAE55UL, (int32_t)0x807451ABUL, (int32_t)0x00000000UL, (int32_t)0xC07451ABUL, (int32_t)0x00000000UL},
  {(int32_t)0x3FA1E580UL, (int32_t)0x80BC446EUL, (int32_t)0x3FA1E580UL, (int32_t)0x80BCAE47UL, (int32_t)0x7E8869B2UL},
  {(int32_t)0x3FDBCCBBUL, (int32_t)0x80488F15UL, (int32_t)0x3FDBCCBBUL, (int32_t)0x8048F94EUL, (int32_t)0x7F70075DUL}
};

const biquadInt32_t IIR_chebyshev2_DC_remove_fs96000_biquadInt32 =
{
  (int32_t)0x40000000UL, 3, sizeof(IIR_chebyshev2_DC_remove_fs96000_BiquadCellInt32) / sizeof(IIR_chebyshev2_DC_remove_fs96000_BiquadCellInt32[0]), IIR_chebyshev2_DC_remove_fs96000_BiquadCellInt32
};

// 96000 Hz high-pass IIR
//     stop frequency :     50 Hz
//     pass frequency :    200 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_elliptic_DC_remove_fs96000_BiquadCellFloat[] =
{
  {+0.98722734, -1.97445224, +0.98722734, -1.97432538, +0.97458154},
  {+0.98610563, -1.97219758, +0.98610563, -1.99497660, +0.99510633}
};

const biquadFloat_t IIR_elliptic_DC_remove_fs96000_biquadFloat =
{
  sizeof(IIR_elliptic_DC_remove_fs96000_BiquadCellFloat) / sizeof(IIR_elliptic_DC_remove_fs96000_BiquadCellFloat[0]), IIR_elliptic_DC_remove_fs96000_BiquadCellFloat
};

static const biquadCellInt32_t IIR_elliptic_DC_remove_fs96000_BiquadCellInt32[] =
{
  {(int32_t)0x3F2EBB91UL, (int32_t)0x81A29310UL, (int32_t)0x3F2EBB91UL, (int32_t)0x81A4A72DUL, (int32_t)0x7CBF167DUL},
  {(int32_t)0x3F1C5B37UL, (int32_t)0x81C782E8UL, (int32_t)0x3F1C5B37UL, (int32_t)0x80524DA8UL, (int32_t)0x7F5FA4F2UL}
};

const biquadInt32_t IIR_elliptic_DC_remove_fs96000_biquadInt32 =
{
  (int32_t)0x7FFFFF1AUL, 2, sizeof(IIR_elliptic_DC_remove_fs96000_BiquadCellInt32) / sizeof(IIR_elliptic_DC_remove_fs96000_BiquadCellInt32[0]), IIR_elliptic_DC_remove_fs96000_BiquadCellInt32
};

#endif /* AC_SUPPORT_FS_96000 */

const biquadFloat_t IIR_flat_biquadFloat = {  0, NULL}; const biquadInt32_t IIR_flat_biquadInt32 = {  0x40000000, 1, 0, NULL};