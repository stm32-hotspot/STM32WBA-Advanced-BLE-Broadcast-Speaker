#include <stdlib.h> // for NULL definition
#include "common/IIR_low_pass_coef.h"

#ifdef AC_SUPPORT_RATIO_2
// 16000 Hz low-pass IIR
//     pass frequency :   3500 Hz
//     stop frequency :   4000 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_butterworth_resampling_ratio2_BiquadCellFloat[] =
{
  {+0.46169877, +0.46169877, +0.00000000, -0.07660247, +0.00000000},
  {+0.21342394, +0.42684789, +0.21342394, -0.15339161, +0.00708738},
  {+0.21420239, +0.42840479, +0.21420239, -0.15394768, +0.01075726},
  {+0.21550621, +0.43101241, +0.21550621, -0.15488911, +0.01691393},
  {+0.21735128, +0.43470257, +0.21735128, -0.15621132, +0.02561645},
  {+0.21975305, +0.43950610, +0.21975305, -0.15794059, +0.03695278},
  {+0.22273742, +0.44547485, +0.22273742, -0.16008462, +0.05103432},
  {+0.22633485, +0.45266970, +0.22633485, -0.16266827, +0.06800766},
  {+0.23058146, +0.46116292, +0.23058146, -0.16572270, +0.08804854},
  {+0.23552433, +0.47104865, +0.23552433, -0.16927432, +0.11137162},
  {+0.24121727, +0.48243453, +0.24121727, -0.17336577, +0.13823483},
  {+0.24772504, +0.49545007, +0.24772504, -0.17804317, +0.16894332},
  {+0.25512453, +0.51024907, +0.25512453, -0.18336125, +0.20385939},
  {+0.26350645, +0.52701290, +0.26350645, -0.18938544, +0.24341124},
  {+0.27297803, +0.54595606, +0.27297803, -0.19619278, +0.28810490},
  {+0.28366616, +0.56733231, +0.28366616, -0.20387447, +0.33853910},
  {+0.29572126, +0.59144251, +0.29572126, -0.21253863, +0.39542365},
  {+0.30932222, +0.61864443, +0.30932222, -0.22231381, +0.45960268},
  {+0.32468259, +0.64936518, +0.32468259, -0.23335351, +0.53208387},
  {+0.34205847, +0.68411694, +0.34205847, -0.24584177, +0.61407565},
  {+0.36175862, +0.72351723, +0.36175862, -0.26000052, +0.70703498},
  {+0.38415752, +0.76831504, +0.38415752, -0.27609890, +0.81272899},
  {+0.40971248, +0.81942496, +0.40971248, -0.29446557, +0.93331548}
};

const biquadFloat_t IIR_butterworth_resampling_ratio2_biquadFloat =
{
  sizeof(IIR_butterworth_resampling_ratio2_BiquadCellFloat) / sizeof(IIR_butterworth_resampling_ratio2_BiquadCellFloat[0]), IIR_butterworth_resampling_ratio2_BiquadCellFloat
};

static const biquadCellInt32_t IIR_butterworth_resampling_ratio2_BiquadCellInt32[] =
{
  {(int32_t)0x3B18F1F6UL, (int32_t)0x3B18F1F6UL, (int32_t)0x00000000UL, (int32_t)0xFB18F1F6UL, (int32_t)0x00000000UL},
  {(int32_t)0x1B5179CCUL, (int32_t)0x36A2F399UL, (int32_t)0x1B5179CCUL, (int32_t)0xF62ED4F4UL, (int32_t)0x00E83D49UL},
  {(int32_t)0x1B6AFBEDUL, (int32_t)0x36D5F7D9UL, (int32_t)0x1B6AFBEDUL, (int32_t)0xF625B8A1UL, (int32_t)0x01607E71UL},
  {(int32_t)0x1B95B513UL, (int32_t)0x372B6A27UL, (int32_t)0x1B95B513UL, (int32_t)0xF6164BFCUL, (int32_t)0x022A3C57UL},
  {(int32_t)0x1BD22AB7UL, (int32_t)0x37A4556DUL, (int32_t)0x1BD22AB7UL, (int32_t)0xF600A23FUL, (int32_t)0x0347665DUL},
  {(int32_t)0x1C20DE30UL, (int32_t)0x3841BC61UL, (int32_t)0x1C20DE30UL, (int32_t)0xF5E44D2BUL, (int32_t)0x04BADE6BUL},
  {(int32_t)0x1C82A8F2UL, (int32_t)0x390551E4UL, (int32_t)0x1C82A8F2UL, (int32_t)0xF5C12C6DUL, (int32_t)0x06884AEDUL},
  {(int32_t)0x1CF88A51UL, (int32_t)0x39F114A3UL, (int32_t)0x1CF88A51UL, (int32_t)0xF596D7D3UL, (int32_t)0x08B4799FUL},
  {(int32_t)0x1D83B17CUL, (int32_t)0x3B0762F7UL, (int32_t)0x1D83B17CUL, (int32_t)0xF564CC9FUL, (int32_t)0x0B452CB0UL},
  {(int32_t)0x1E25A940UL, (int32_t)0x3C4B5280UL, (int32_t)0x1E25A940UL, (int32_t)0xF52A9C10UL, (int32_t)0x0E416CE1UL},
  {(int32_t)0x1EE03515UL, (int32_t)0x3DC06A2AUL, (int32_t)0x1EE03515UL, (int32_t)0xF4E79345UL, (int32_t)0x11B1ADCAUL},
  {(int32_t)0x1FB5743BUL, (int32_t)0x3F6AE876UL, (int32_t)0x1FB5743BUL, (int32_t)0xF49AF0D6UL, (int32_t)0x159FEF41UL},
  {(int32_t)0x20A7EBB6UL, (int32_t)0x414FD76CUL, (int32_t)0x20A7EBB6UL, (int32_t)0xF443CF29UL, (int32_t)0x1A181087UL},
  {(int32_t)0x21BA9451UL, (int32_t)0x437528A2UL, (int32_t)0x21BA9451UL, (int32_t)0xF3E11BE3UL, (int32_t)0x1F28197DUL},
  {(int32_t)0x22F0F1B1UL, (int32_t)0x45E1E362UL, (int32_t)0x22F0F1B1UL, (int32_t)0xF37193D5UL, (int32_t)0x24E09F1BUL},
  {(int32_t)0x244F2C2FUL, (int32_t)0x489E585DUL, (int32_t)0x244F2C2FUL, (int32_t)0xF2F3B87AUL, (int32_t)0x2B553FC6UL},
  {(int32_t)0x25DA31B0UL, (int32_t)0x4BB4635FUL, (int32_t)0x25DA31B0UL, (int32_t)0xF265C462UL, (int32_t)0x329D3DFBUL},
  {(int32_t)0x2797DED4UL, (int32_t)0x4F2FBDA7UL, (int32_t)0x2797DED4UL, (int32_t)0xF1C59C49UL, (int32_t)0x3AD442BCUL},
  {(int32_t)0x298F32F9UL, (int32_t)0x531E65F1UL, (int32_t)0x298F32F9UL, (int32_t)0xF110BC73UL, (int32_t)0x441B52FDUL},
  {(int32_t)0x2BC8926AUL, (int32_t)0x579124D5UL, (int32_t)0x2BC8926AUL, (int32_t)0xF04420DEUL, (int32_t)0x4E9A07EFUL},
  {(int32_t)0x2E4E1B38UL, (int32_t)0x5C9C366FUL, (int32_t)0x2E4E1B38UL, (int32_t)0xEF5C26C8UL, (int32_t)0x5A801F4EUL},
  {(int32_t)0x312C12DDUL, (int32_t)0x625825BAUL, (int32_t)0x312C12DDUL, (int32_t)0xEE546549UL, (int32_t)0x680780E1UL},
  {(int32_t)0x3471755FUL, (int32_t)0x68E2EABFUL, (int32_t)0x3471755FUL, (int32_t)0xED2779E2UL, (int32_t)0x7776E1BBUL}
};

const biquadInt32_t IIR_butterworth_resampling_ratio2_biquadInt32 =
{
  (int32_t)0x40000000UL, 1, sizeof(IIR_butterworth_resampling_ratio2_BiquadCellInt32) / sizeof(IIR_butterworth_resampling_ratio2_BiquadCellInt32[0]), IIR_butterworth_resampling_ratio2_BiquadCellInt32
};

// 16000 Hz low-pass IIR
//     pass frequency :   3500 Hz
//     stop frequency :   4000 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_chebyshev1_resampling_ratio2_BiquadCellFloat[] =
{
  {+0.12417668, +0.12417668, +0.00000000, -0.75164664, +0.00000000},
  {+0.03710015, +0.07420030, +0.03710015, -1.43342960, +0.58183019},
  {+0.09458310, +0.18916620, +0.09458310, -1.24908780, +0.62742021},
  {+0.17055375, +0.34110749, +0.17055375, -1.00823353, +0.69044851},
  {+0.24785643, +0.49571287, +0.24785643, -0.76866765, +0.76009338},
  {+0.31507570, +0.63015139, +0.31507570, -0.56959396, +0.82989675},
  {+0.36652747, +0.73305495, +0.36652747, -0.43181641, +0.89792630},
  {+0.39994055, +0.79988110, +0.39994055, -0.36568951, +0.96545170}
};

const biquadFloat_t IIR_chebyshev1_resampling_ratio2_biquadFloat =
{
  sizeof(IIR_chebyshev1_resampling_ratio2_BiquadCellFloat) / sizeof(IIR_chebyshev1_resampling_ratio2_BiquadCellFloat[0]), IIR_chebyshev1_resampling_ratio2_BiquadCellFloat
};

static const biquadCellInt32_t IIR_chebyshev1_resampling_ratio2_BiquadCellInt32[] =
{
  {(int32_t)0x0FE5057EUL, (int32_t)0x0FE5057EUL, (int32_t)0x00000000UL, (int32_t)0xCFE5057EUL, (int32_t)0x00000000UL},
  {(int32_t)0x04BFB29AUL, (int32_t)0x097F6534UL, (int32_t)0x04BFB29AUL, (int32_t)0xA442B07FUL, (int32_t)0x4A79696BUL},
  {(int32_t)0x0C1B4C91UL, (int32_t)0x18369922UL, (int32_t)0x0C1B4C91UL, (int32_t)0xB00EF207UL, (int32_t)0x504F4E35UL},
  {(int32_t)0x15D4B484UL, (int32_t)0x2BA96907UL, (int32_t)0x15D4B484UL, (int32_t)0xBF791A18UL, (int32_t)0x58609DDFUL},
  {(int32_t)0x1FB9C276UL, (int32_t)0x3F7384EDUL, (int32_t)0x1FB9C276UL, (int32_t)0xCECE2635UL, (int32_t)0x614ABD70UL},
  {(int32_t)0x28546683UL, (int32_t)0x50A8CD06UL, (int32_t)0x28546683UL, (int32_t)0xDB8BC5C7UL, (int32_t)0x6A3A0E7FUL},
  {(int32_t)0x2EEA5F4BUL, (int32_t)0x5DD4BE97UL, (int32_t)0x2EEA5F4BUL, (int32_t)0xE45D1EBAUL, (int32_t)0x72EF3FBBUL},
  {(int32_t)0x3331407BUL, (int32_t)0x666280F6UL, (int32_t)0x3331407BUL, (int32_t)0xE8988B09UL, (int32_t)0x7B93EBDAUL}
};

const biquadInt32_t IIR_chebyshev1_resampling_ratio2_biquadInt32 =
{
  (int32_t)0x40000000UL, 1, sizeof(IIR_chebyshev1_resampling_ratio2_BiquadCellInt32) / sizeof(IIR_chebyshev1_resampling_ratio2_BiquadCellInt32[0]), IIR_chebyshev1_resampling_ratio2_BiquadCellInt32
};

// 16000 Hz low-pass IIR
//     pass frequency :   3500 Hz
//     stop frequency :   4000 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_chebyshev2_resampling_ratio2_BiquadCellFloat[] =
{
  {+0.65415380, +0.65415380, +0.00000000, +0.30830760, +0.00000000},
  {+0.44260404, +0.81184893, +0.44260404, +0.57467186, +0.12238515},
  {+0.48339839, +0.69232112, +0.48339839, +0.46044633, +0.19867157},
  {+0.54243855, +0.52773375, +0.54243855, +0.30235869, +0.31025215},
  {+0.61129404, +0.35264384, +0.61129404, +0.13249278, +0.44273915},
  {+0.68407254, +0.19544930, +0.68407254, -0.02307532, +0.58666970},
  {+0.75853437, +0.07606539, +0.75853437, -0.14658765, +0.73972177},
  {+0.83605570, +0.00918509, +0.83605570, -0.22578933, +0.90708582}
};

const biquadFloat_t IIR_chebyshev2_resampling_ratio2_biquadFloat =
{
  sizeof(IIR_chebyshev2_resampling_ratio2_BiquadCellFloat) / sizeof(IIR_chebyshev2_resampling_ratio2_BiquadCellFloat[0]), IIR_chebyshev2_resampling_ratio2_BiquadCellFloat
};

static const biquadCellInt32_t IIR_chebyshev2_resampling_ratio2_BiquadCellInt32[] =
{
  {(int32_t)0x53BB4FD2UL, (int32_t)0x53BB4FD2UL, (int32_t)0x00000000UL, (int32_t)0x13BB4FD2UL, (int32_t)0x00000000UL},
  {(int32_t)0x38A73FC8UL, (int32_t)0x67EAAA77UL, (int32_t)0x38A73FC8UL, (int32_t)0x24C76C7BUL, (int32_t)0x0FAA5112UL},
  {(int32_t)0x3DDFFFA3UL, (int32_t)0x589DFA7DUL, (int32_t)0x3DDFFFA3UL, (int32_t)0x1D77F3E7UL, (int32_t)0x196E11F5UL},
  {(int32_t)0x456EA054UL, (int32_t)0x438CC78DUL, (int32_t)0x456EA054UL, (int32_t)0x1359D845UL, (int32_t)0x27B657ABUL},
  {(int32_t)0x4E3EE21BUL, (int32_t)0x2D236EECUL, (int32_t)0x4E3EE21BUL, (int32_t)0x087AC2FEUL, (int32_t)0x38ABAD26UL},
  {(int32_t)0x578FB064UL, (int32_t)0x19047B8AUL, (int32_t)0x578FB064UL, (int32_t)0xFE85EF14UL, (int32_t)0x4B17FE2AUL},
  {(int32_t)0x6117A77CUL, (int32_t)0x09BC82B4UL, (int32_t)0x6117A77CUL, (int32_t)0xF69E4ED8UL, (int32_t)0x5EAF33FBUL},
  {(int32_t)0x6B03DF89UL, (int32_t)0x012CFA20UL, (int32_t)0x6B03DF89UL, (int32_t)0xF18CAAECUL, (int32_t)0x741B635AUL}
};

const biquadInt32_t IIR_chebyshev2_resampling_ratio2_biquadInt32 =
{
  (int32_t)0x7FFFFFFFUL, 0, sizeof(IIR_chebyshev2_resampling_ratio2_BiquadCellInt32) / sizeof(IIR_chebyshev2_resampling_ratio2_BiquadCellInt32[0]), IIR_chebyshev2_resampling_ratio2_BiquadCellInt32
};

// 16000 Hz low-pass IIR
//     pass frequency :   3500 Hz
//     stop frequency :   4000 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_elliptic_resampling_ratio2_BiquadCellFloat[] =
{
  {+0.10422502, +0.18083119, +0.10422502, -0.86786993, +0.25715116},
  {+0.33512863, +0.23394224, +0.33512863, -0.63298904, +0.53718853},
  {+0.63410168, +0.09126338, +0.63410168, -0.43075481, +0.79189915},
  {+0.80880005, -0.03632785, +0.80880005, -0.34543253, +0.94304357}
};

const biquadFloat_t IIR_elliptic_resampling_ratio2_biquadFloat =
{
  sizeof(IIR_elliptic_resampling_ratio2_BiquadCellFloat) / sizeof(IIR_elliptic_resampling_ratio2_BiquadCellFloat[0]), IIR_elliptic_resampling_ratio2_BiquadCellFloat
};

static const biquadCellInt32_t IIR_elliptic_resampling_ratio2_BiquadCellInt32[] =
{
  {(int32_t)0x0D573ED9UL, (int32_t)0x172579F0UL, (int32_t)0x0D573ED9UL, (int32_t)0xC874D1B2UL, (int32_t)0x20EA543EUL},
  {(int32_t)0x2AE57EADUL, (int32_t)0x1DF1D1B7UL, (int32_t)0x2AE57EADUL, (int32_t)0xD77D1B8CUL, (int32_t)0x44C297F8UL},
  {(int32_t)0x512A3E6BUL, (int32_t)0x0BAE84AEUL, (int32_t)0x512A3E6BUL, (int32_t)0xE46E8362UL, (int32_t)0x655CF390UL},
  {(int32_t)0x6786C29BUL, (int32_t)0xFB599BF2UL, (int32_t)0x6786C29BUL, (int32_t)0xE9E46EFAUL, (int32_t)0x78B5A6DCUL}
};

const biquadInt32_t IIR_elliptic_resampling_ratio2_biquadInt32 =
{
  (int32_t)0x7FFFFFFFUL, 0, sizeof(IIR_elliptic_resampling_ratio2_BiquadCellInt32) / sizeof(IIR_elliptic_resampling_ratio2_BiquadCellInt32[0]), IIR_elliptic_resampling_ratio2_BiquadCellInt32
};

#endif /* AC_SUPPORT_RATIO_2 */

#ifdef AC_SUPPORT_RATIO_3
// 24000 Hz low-pass IIR
//     pass frequency :   3500 Hz
//     stop frequency :   4000 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_butterworth_resampling_ratio3_BiquadCellFloat[] =
{
  {+0.11687420, +0.23374840, +0.11687420, -0.63304602, +0.10054282},
  {+0.11690147, +0.23380294, +0.11690147, -0.63767376, +0.10527964},
  {+0.11577391, +0.23154781, +0.11577391, -0.65200954, +0.11510516},
  {+0.10982452, +0.21964904, +0.10982452, -0.67639498, +0.11569306},
  {+0.11455965, +0.22911930, +0.11455965, -0.66428033, +0.12251893},
  {+0.11721519, +0.23443039, +0.11721519, -0.65900381, +0.12786458},
  {+0.11748451, +0.23496901, +0.11748451, -0.66919587, +0.13913389},
  {+0.11884171, +0.23768342, +0.11884171, -0.67440437, +0.14977122},
  {+0.12019079, +0.24038157, +0.12019079, -0.68247817, +0.16324131},
  {+0.12170428, +0.24340855, +0.12170428, -0.69140989, +0.17822700},
  {+0.12350078, +0.24700156, +0.12350078, -0.70120028, +0.19520341},
  {+0.12546279, +0.25092558, +0.12546279, -0.71246603, +0.21431720},
  {+0.12765393, +0.25530786, +0.12765393, -0.72494351, +0.23555923},
  {+0.13008735, +0.26017470, +0.13008735, -0.73874692, +0.25909632},
  {+0.13277317, +0.26554634, +0.13277317, -0.75398888, +0.28508155},
  {+0.13572658, +0.27145316, +0.13572658, -0.77076554, +0.31367185},
  {+0.13896801, +0.27793603, +0.13896801, -0.78917419, +0.34504624},
  {+0.14251877, +0.28503753, +0.14251877, -0.80933732, +0.37941239},
  {+0.14640232, +0.29280463, +0.14640232, -0.83139137, +0.41700063},
  {+0.15064547, +0.30129095, +0.15064547, -0.85548751, +0.45806941},
  {+0.15527828, +0.31055655, +0.15527828, -0.88179631, +0.50290942},
  {+0.16033442, +0.32066885, +0.16033442, -0.91050923, +0.55184693},
  {+0.16585179, +0.33170359, +0.16585179, -0.94184135, +0.60524853},
  {+0.17187296, +0.34374592, +0.17187296, -0.97603444, +0.66352629},
  {+0.17844581, +0.35689161, +0.17844581, -1.01336039, +0.72714362},
  {+0.18562421, +0.37124842, +0.18562421, -1.05412522, +0.79662207},
  {+0.19346886, +0.38693771, +0.19346886, -1.09867349, +0.87254891},
  {+0.20204810, +0.40409620, +0.20204810, -1.14739341, +0.95558581}
};

const biquadFloat_t IIR_butterworth_resampling_ratio3_biquadFloat =
{
  sizeof(IIR_butterworth_resampling_ratio3_BiquadCellFloat) / sizeof(IIR_butterworth_resampling_ratio3_BiquadCellFloat[0]), IIR_butterworth_resampling_ratio3_BiquadCellFloat
};

static const biquadCellInt32_t IIR_butterworth_resampling_ratio3_BiquadCellInt32[] =
{
  {(int32_t)0x0EF5BBDEUL, (int32_t)0x1DEB77BCUL, (int32_t)0x0EF5BBDEUL, (int32_t)0xD77C2C90UL, (int32_t)0x0CDE9659UL},
  {(int32_t)0x0EF6A09CUL, (int32_t)0x1DED4137UL, (int32_t)0x0EF6A09CUL, (int32_t)0xD7305A65UL, (int32_t)0x0D79CDA4UL},
  {(int32_t)0x0ED1ADE9UL, (int32_t)0x1DA35BD3UL, (int32_t)0x0ED1ADE9UL, (int32_t)0xD64579C8UL, (int32_t)0x0EBBC415UL},
  {(int32_t)0x0E0EBADAUL, (int32_t)0x1C1D75B5UL, (int32_t)0x0E0EBADAUL, (int32_t)0xD4B5F1D2UL, (int32_t)0x0ECF07C5UL},
  {(int32_t)0x0EA9E401UL, (int32_t)0x1D53C802UL, (int32_t)0x0EA9E401UL, (int32_t)0xD57C6E5BUL, (int32_t)0x0FAEB34FUL},
  {(int32_t)0x0F00E850UL, (int32_t)0x1E01D09FUL, (int32_t)0x0F00E850UL, (int32_t)0xD5D2E1B3UL, (int32_t)0x105DDDD9UL},
  {(int32_t)0x0F09BB78UL, (int32_t)0x1E1376F0UL, (int32_t)0x0F09BB78UL, (int32_t)0xD52BE51BUL, (int32_t)0x11CF23ABUL},
  {(int32_t)0x0F36348AUL, (int32_t)0x1E6C6914UL, (int32_t)0x0F36348AUL, (int32_t)0xD4D68F0DUL, (int32_t)0x132BB40EUL},
  {(int32_t)0x0F626962UL, (int32_t)0x1EC4D2C3UL, (int32_t)0x0F626962UL, (int32_t)0xD4524711UL, (int32_t)0x14E51763UL},
  {(int32_t)0x0F94017AUL, (int32_t)0x1F2802F3UL, (int32_t)0x0F94017AUL, (int32_t)0xD3BFF0BFUL, (int32_t)0x16D0246AUL},
  {(int32_t)0x0FCEDFA5UL, (int32_t)0x1F9DBF4AUL, (int32_t)0x0FCEDFA5UL, (int32_t)0xD31F88D7UL, (int32_t)0x18FC6CE5UL},
  {(int32_t)0x100F2A2FUL, (int32_t)0x201E545DUL, (int32_t)0x100F2A2FUL, (int32_t)0xD266F4E2UL, (int32_t)0x1B6EBEF6UL},
  {(int32_t)0x1056F6C6UL, (int32_t)0x20ADED8CUL, (int32_t)0x1056F6C6UL, (int32_t)0xD19A868CUL, (int32_t)0x1E26CE00UL},
  {(int32_t)0x10A6B3C9UL, (int32_t)0x214D6791UL, (int32_t)0x10A6B3C9UL, (int32_t)0xD0B85ED4UL, (int32_t)0x212A117AUL},
  {(int32_t)0x10FEB611UL, (int32_t)0x21FD6C21UL, (int32_t)0x10FEB611UL, (int32_t)0xCFBEA56EUL, (int32_t)0x247D8D66UL},
  {(int32_t)0x115F7D0FUL, (int32_t)0x22BEFA1EUL, (int32_t)0x115F7D0FUL, (int32_t)0xCEABC707UL, (int32_t)0x2826662FUL},
  {(int32_t)0x11C9B431UL, (int32_t)0x23936862UL, (int32_t)0x11C9B431UL, (int32_t)0xCD7E2B8BUL, (int32_t)0x2C2A79AEUL},
  {(int32_t)0x123E0E10UL, (int32_t)0x247C1C21UL, (int32_t)0x123E0E10UL, (int32_t)0xCC33D13CUL, (int32_t)0x309095C9UL},
  {(int32_t)0x12BD4FA2UL, (int32_t)0x257A9F43UL, (int32_t)0x12BD4FA2UL, (int32_t)0xCACA7BDDUL, (int32_t)0x356046CCUL},
  {(int32_t)0x134859D6UL, (int32_t)0x2690B3ACUL, (int32_t)0x134859D6UL, (int32_t)0xC93FB14DUL, (int32_t)0x3AA204BEUL},
  {(int32_t)0x13E02896UL, (int32_t)0x27C0512DUL, (int32_t)0x13E02896UL, (int32_t)0xC790A634UL, (int32_t)0x405F55F3UL},
  {(int32_t)0x1485D6A0UL, (int32_t)0x290BAD41UL, (int32_t)0x1485D6A0UL, (int32_t)0xC5BA377CUL, (int32_t)0x46A2EB89UL},
  {(int32_t)0x153AA1B1UL, (int32_t)0x2A754362UL, (int32_t)0x153AA1B1UL, (int32_t)0xC3B8DF0DUL, (int32_t)0x4D78C8ACUL},
  {(int32_t)0x15FFEEE8UL, (int32_t)0x2BFFDDCFUL, (int32_t)0x15FFEEE8UL, (int32_t)0xC188A6DBUL, (int32_t)0x54EE6DE7UL},
  {(int32_t)0x16D74FECUL, (int32_t)0x2DAE9FD7UL, (int32_t)0x16D74FECUL, (int32_t)0xBF251A74UL, (int32_t)0x5D130AC6UL},
  {(int32_t)0x17C288C1UL, (int32_t)0x2F851182UL, (int32_t)0x17C288C1UL, (int32_t)0xBC893662UL, (int32_t)0x65F7B640UL},
  {(int32_t)0x18C39666UL, (int32_t)0x31872CCDUL, (int32_t)0x18C39666UL, (int32_t)0xB9AF5566UL, (int32_t)0x6FAFAECEUL},
  {(int32_t)0x19DCB650UL, (int32_t)0x33B96CA1UL, (int32_t)0x19DCB650UL, (int32_t)0xB6911B3BUL, (int32_t)0x7A50A2CCUL}
};

const biquadInt32_t IIR_butterworth_resampling_ratio3_biquadInt32 =
{
  (int32_t)0x40000000UL, 1, sizeof(IIR_butterworth_resampling_ratio3_BiquadCellInt32) / sizeof(IIR_butterworth_resampling_ratio3_BiquadCellInt32[0]), IIR_butterworth_resampling_ratio3_BiquadCellInt32
};

// 24000 Hz low-pass IIR
//     pass frequency :   3500 Hz
//     stop frequency :   4000 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_chebyshev1_resampling_ratio3_BiquadCellFloat[] =
{
  {+0.06984721, +0.06984721, +0.00000000, -0.86030558, +0.00000000},
  {+0.01192443, +0.02384887, +0.01192443, -1.69809631, +0.74579405},
  {+0.03174267, +0.06348534, +0.03174267, -1.63514802, +0.76211869},
  {+0.06082575, +0.12165149, +0.06082575, -1.54400984, +0.78731282},
  {+0.09455197, +0.18910393, +0.09455197, -1.44088373, +0.81909159},
  {+0.12830468, +0.25660935, +0.12830468, -1.34208901, +0.85530771},
  {+0.15820125, +0.31640249, +0.15820125, -1.26156148, +0.89436646},
  {+0.18134382, +0.36268764, +0.18134382, -1.20998791, +0.93536318},
  {+0.19574591, +0.39149182, +0.19574591, -1.19503148, +0.97801512}
};

const biquadFloat_t IIR_chebyshev1_resampling_ratio3_biquadFloat =
{
  sizeof(IIR_chebyshev1_resampling_ratio3_BiquadCellFloat) / sizeof(IIR_chebyshev1_resampling_ratio3_BiquadCellFloat[0]), IIR_chebyshev1_resampling_ratio3_BiquadCellFloat
};

static const biquadCellInt32_t IIR_chebyshev1_resampling_ratio3_BiquadCellInt32[] =
{
  {(int32_t)0x08F0C0DBUL, (int32_t)0x08F0C0DBUL, (int32_t)0x00000000UL, (int32_t)0xC8F0C0DBUL, (int32_t)0x00000000UL},
  {(int32_t)0x0186BD67UL, (int32_t)0x030D7ACDUL, (int32_t)0x0186BD67UL, (int32_t)0x935263D7UL, (int32_t)0x5F762DECUL},
  {(int32_t)0x041024CFUL, (int32_t)0x0820499EUL, (int32_t)0x041024CFUL, (int32_t)0x9759BC21UL, (int32_t)0x618D1AF9UL},
  {(int32_t)0x07C92356UL, (int32_t)0x0F9246ACUL, (int32_t)0x07C92356UL, (int32_t)0x9D2EF15EUL, (int32_t)0x64C6AA9CUL},
  {(int32_t)0x0C1A4760UL, (int32_t)0x18348EC0UL, (int32_t)0x0C1A4760UL, (int32_t)0xA3C88F9DUL, (int32_t)0x68D7FE45UL},
  {(int32_t)0x106C49A0UL, (int32_t)0x20D89340UL, (int32_t)0x106C49A0UL, (int32_t)0xAA1B36AFUL, (int32_t)0x6D7AB921UL},
  {(int32_t)0x143FF03BUL, (int32_t)0x287FE076UL, (int32_t)0x143FF03BUL, (int32_t)0xAF4293A7UL, (int32_t)0x727A999DUL},
  {(int32_t)0x17364633UL, (int32_t)0x2E6C8C67UL, (int32_t)0x17364633UL, (int32_t)0xB28F8EDCUL, (int32_t)0x77B9FB14UL},
  {(int32_t)0x190E33BAUL, (int32_t)0x321C6774UL, (int32_t)0x190E33BAUL, (int32_t)0xB3849AB4UL, (int32_t)0x7D2F997FUL}
};

const biquadInt32_t IIR_chebyshev1_resampling_ratio3_biquadInt32 =
{
  (int32_t)0x7FFFFFFFUL, 0, sizeof(IIR_chebyshev1_resampling_ratio3_BiquadCellInt32) / sizeof(IIR_chebyshev1_resampling_ratio3_BiquadCellInt32[0]), IIR_chebyshev1_resampling_ratio3_BiquadCellInt32
};

// 24000 Hz low-pass IIR
//     pass frequency :   3500 Hz
//     stop frequency :   4000 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_chebyshev2_resampling_ratio3_BiquadCellFloat[] =
{
  {+0.55540685, +0.55540685, +0.00000000, +0.11081370, +0.00000000},
  {+0.33213779, +0.54208166, +0.33213779, +0.15558210, +0.05077514},
  {+0.39473540, +0.34524482, +0.39473540, -0.01830180, +0.15301742},
  {+0.47775488, +0.08796921, +0.47775488, -0.24634649, +0.28982547},
  {+0.56423368, -0.17278598, +0.56423368, -0.47897259, +0.43465398},
  {+0.64447619, -0.40322496, +0.64447619, -0.68699933, +0.57272674},
  {+0.71536629, -0.59010228, +0.71536629, -0.85942818, +0.70005848},
  {+0.77774070, -0.73145639, +0.77774070, -0.99546219, +0.81948720},
  {+0.83445349, -0.82909119, +0.83445349, -1.09815613, +0.93797192}
};

const biquadFloat_t IIR_chebyshev2_resampling_ratio3_biquadFloat =
{
  sizeof(IIR_chebyshev2_resampling_ratio3_BiquadCellFloat) / sizeof(IIR_chebyshev2_resampling_ratio3_BiquadCellFloat[0]), IIR_chebyshev2_resampling_ratio3_BiquadCellFloat
};

static const biquadCellInt32_t IIR_chebyshev2_resampling_ratio3_BiquadCellInt32[] =
{
  {(int32_t)0x4717925AUL, (int32_t)0x4717925AUL, (int32_t)0x00000000UL, (int32_t)0x0717925AUL, (int32_t)0x00000000UL},
  {(int32_t)0x2A837DBAUL, (int32_t)0x4562EE8AUL, (int32_t)0x2A837DBAUL, (int32_t)0x09F50E9BUL, (int32_t)0x067FCCC8UL},
  {(int32_t)0x3286B091UL, (int32_t)0x2C30FB71UL, (int32_t)0x3286B091UL, (int32_t)0xFED424B1UL, (int32_t)0x13961331UL},
  {(int32_t)0x3D271272UL, (int32_t)0x0B429335UL, (int32_t)0x3D271272UL, (int32_t)0xF03BDBEAUL, (int32_t)0x25190045UL},
  {(int32_t)0x4838CF30UL, (int32_t)0xE9E22623UL, (int32_t)0x4838CF30UL, (int32_t)0xE1588355UL, (int32_t)0x37A2BDD9UL},
  {(int32_t)0x527E321FUL, (int32_t)0xCC631FD8UL, (int32_t)0x527E321FUL, (int32_t)0xD40833F8UL, (int32_t)0x494F1C25UL},
  {(int32_t)0x5B911F69UL, (int32_t)0xB477874AUL, (int32_t)0x5B911F69UL, (int32_t)0xC8FF20F5UL, (int32_t)0x599B8431UL},
  {(int32_t)0x638D01E0UL, (int32_t)0xA25FA30FUL, (int32_t)0x638D01E0UL, (int32_t)0xC04A58F3UL, (int32_t)0x68E4F4E8UL},
  {(int32_t)0x6ACF5F38UL, (int32_t)0x95E056FEUL, (int32_t)0x6ACF5F38UL, (int32_t)0xB9B7CF5BUL, (int32_t)0x780F76B8UL}
};

const biquadInt32_t IIR_chebyshev2_resampling_ratio3_biquadInt32 =
{
  (int32_t)0x7FFFFFFFUL, 0, sizeof(IIR_chebyshev2_resampling_ratio3_BiquadCellInt32) / sizeof(IIR_chebyshev2_resampling_ratio3_BiquadCellInt32[0]), IIR_chebyshev2_resampling_ratio3_BiquadCellInt32
};

// 24000 Hz low-pass IIR
//     pass frequency :   3500 Hz
//     stop frequency :   4000 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_elliptic_resampling_ratio3_BiquadCellFloat[] =
{
  {+0.17907685, +0.17907685, +0.00000000, -0.64184631, +0.00000000},
  {+0.12036833, +0.03413689, +0.12036833, -1.25729463, +0.53216819},
  {+0.40896453, -0.28556376, +0.40896453, -1.21206388, +0.74442917},
  {+0.69704949, -0.68846036, +0.69704949, -1.18585854, +0.89149716},
  {+0.84667712, -0.90839387, +0.84667712, -1.18618323, +0.97114359}
};

const biquadFloat_t IIR_elliptic_resampling_ratio3_biquadFloat =
{
  sizeof(IIR_elliptic_resampling_ratio3_BiquadCellFloat) / sizeof(IIR_elliptic_resampling_ratio3_BiquadCellFloat[0]), IIR_elliptic_resampling_ratio3_BiquadCellFloat
};

static const biquadCellInt32_t IIR_elliptic_resampling_ratio3_BiquadCellInt32[] =
{
  {(int32_t)0x16EBFD75UL, (int32_t)0x16EBFD75UL, (int32_t)0x00000000UL, (int32_t)0xD6EBFD75UL, (int32_t)0x00000000UL},
  {(int32_t)0x0F683AC4UL, (int32_t)0x045E98F6UL, (int32_t)0x0F683AC4UL, (int32_t)0xAF887C17UL, (int32_t)0x441E164FUL},
  {(int32_t)0x3458F320UL, (int32_t)0xDB72A585UL, (int32_t)0x3458F320UL, (int32_t)0xB26D8B9FUL, (int32_t)0x5F497487UL},
  {(int32_t)0x5938EAE7UL, (int32_t)0xA7E087F4UL, (int32_t)0x5938EAE7UL, (int32_t)0xB41AE4CBUL, (int32_t)0x721C942CUL},
  {(int32_t)0x6C5FEA71UL, (int32_t)0x8BB9BFF4UL, (int32_t)0x6C5FEA71UL, (int32_t)0xB41592F4UL, (int32_t)0x7C4E6EEEUL}
};

const biquadInt32_t IIR_elliptic_resampling_ratio3_biquadInt32 =
{
  (int32_t)0x7FFFFFFFUL, 0, sizeof(IIR_elliptic_resampling_ratio3_BiquadCellInt32) / sizeof(IIR_elliptic_resampling_ratio3_BiquadCellInt32[0]), IIR_elliptic_resampling_ratio3_BiquadCellInt32
};

#endif /* AC_SUPPORT_RATIO_3 */

#ifdef AC_SUPPORT_RATIO_4
// 32000 Hz low-pass IIR
//     pass frequency :   3500 Hz
//     stop frequency :   4000 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_butterworth_resampling_ratio4_BiquadCellFloat[] =
{
  {+0.27000140, +0.27000140, +0.00000000, -0.45999720, +0.00000000},
  {+0.07425064, +0.14850128, +0.07425064, -0.91353192, +0.21053449},
  {+0.07282536, +0.14565071, +0.07282536, -0.92117339, +0.21247481},
  {+0.07546273, +0.15092546, +0.07546273, -0.91267392, +0.21452485},
  {+0.07587225, +0.15174451, +0.07587225, -0.92086488, +0.22435389},
  {+0.07085341, +0.14170682, +0.07085341, -0.94145118, +0.22486481},
  {+0.07660288, +0.15320577, +0.07660288, -0.93069945, +0.23711098},
  {+0.07056262, +0.14112523, +0.07056262, -0.96195305, +0.24420352},
  {+0.07580132, +0.15160263, +0.07580132, -0.95345531, +0.25666058},
  {+0.07460075, +0.14920149, +0.07460075, -0.96676647, +0.26516946},
  {+0.07749352, +0.15498703, +0.07749352, -0.96472776, +0.27470183},
  {+0.07764597, +0.15529194, +0.07764597, -0.98172900, +0.29231288},
  {+0.07849325, +0.15698649, +0.07849325, -0.99291458, +0.30688757},
  {+0.07974108, +0.15948217, +0.07974108, -1.00510908, +0.32407341},
  {+0.08082889, +0.16165779, +0.08082889, -1.02015353, +0.34346910},
  {+0.08208109, +0.16416217, +0.08208109, -1.03591656, +0.36424091},
  {+0.08345319, +0.16690637, +0.08345319, -1.05313616, +0.38694891},
  {+0.08493349, +0.16986698, +0.08493349, -1.07185726, +0.41159122},
  {+0.08653910, +0.17307820, +0.08653910, -1.09210396, +0.43826035},
  {+0.08827271, +0.17654541, +0.08827271, -1.11398490, +0.46707572},
  {+0.09014213, +0.18028425, +0.09014213, -1.13757847, +0.49814697},
  {+0.09215532, +0.18431064, +0.09215532, -1.16298313, +0.53160441},
  {+0.09432030, +0.18864061, +0.09432030, -1.19030524, +0.56758645},
  {+0.09664621, +0.19329243, +0.09664621, -1.21965782, +0.60624267},
  {+0.09914282, +0.19828563, +0.09914282, -1.25116443, +0.64773569},
  {+0.10182064, +0.20364127, +0.10182064, -1.28495807, +0.69224062},
  {+0.10469104, +0.20938207, +0.10469104, -1.32118199, +0.73994613},
  {+0.10776620, +0.21553241, +0.10776620, -1.35999005, +0.79105486},
  {+0.11105920, +0.22211839, +0.11105920, -1.40154702, +0.84578380},
  {+0.11458395, +0.22916789, +0.11458395, -1.44602874, +0.90436453},
  {+0.11835526, +0.23671053, +0.11835526, -1.49362210, +0.96704316}
};

const biquadFloat_t IIR_butterworth_resampling_ratio4_biquadFloat =
{
  sizeof(IIR_butterworth_resampling_ratio4_BiquadCellFloat) / sizeof(IIR_butterworth_resampling_ratio4_BiquadCellFloat[0]), IIR_butterworth_resampling_ratio4_BiquadCellFloat
};

static const biquadCellInt32_t IIR_butterworth_resampling_ratio4_BiquadCellInt32[] =
{
  {(int32_t)0x228F67E9UL, (int32_t)0x228F67E9UL, (int32_t)0x00000000UL, (int32_t)0xE28F67E9UL, (int32_t)0x00000000UL},
  {(int32_t)0x09810B88UL, (int32_t)0x13021710UL, (int32_t)0x09810B88UL, (int32_t)0xC588B168UL, (int32_t)0x1AF2CB4FUL},
  {(int32_t)0x0952575CUL, (int32_t)0x12A4AEB8UL, (int32_t)0x0952575CUL, (int32_t)0xC50B7EC6UL, (int32_t)0x1B325FE4UL},
  {(int32_t)0x09A8C347UL, (int32_t)0x1351868EUL, (int32_t)0x09A8C347UL, (int32_t)0xC596C01EUL, (int32_t)0x1B758CE2UL},
  {(int32_t)0x09B62E97UL, (int32_t)0x136C5D2EUL, (int32_t)0x09B62E97UL, (int32_t)0xC5108CBEUL, (int32_t)0x1CB7A0E0UL},
  {(int32_t)0x0911B977UL, (int32_t)0x122372EDUL, (int32_t)0x0911B977UL, (int32_t)0xC3BF4392UL, (int32_t)0x1CC85EB7UL},
  {(int32_t)0x09CE1F91UL, (int32_t)0x139C3F22UL, (int32_t)0x09CE1F91UL, (int32_t)0xC46F6B95UL, (int32_t)0x1E59A719UL},
  {(int32_t)0x09083222UL, (int32_t)0x12106444UL, (int32_t)0x09083222UL, (int32_t)0xC26F5C75UL, (int32_t)0x1F420F9FUL},
  {(int32_t)0x09B3DB85UL, (int32_t)0x1367B70BUL, (int32_t)0x09B3DB85UL, (int32_t)0xC2FA9690UL, (int32_t)0x20DA40F5UL},
  {(int32_t)0x098C846AUL, (int32_t)0x131908D5UL, (int32_t)0x098C846AUL, (int32_t)0xC2207F86UL, (int32_t)0x21F1129DUL},
  {(int32_t)0x09EB4EBCUL, (int32_t)0x13D69D78UL, (int32_t)0x09EB4EBCUL, (int32_t)0xC241E67AUL, (int32_t)0x23296DFBUL},
  {(int32_t)0x09F04D9DUL, (int32_t)0x13E09B3AUL, (int32_t)0x09F04D9DUL, (int32_t)0xC12B5A24UL, (int32_t)0x256A822DUL},
  {(int32_t)0x0A0C1114UL, (int32_t)0x14182228UL, (int32_t)0x0A0C1114UL, (int32_t)0xC0741669UL, (int32_t)0x2748177EUL},
  {(int32_t)0x0A34F4AFUL, (int32_t)0x1469E95EUL, (int32_t)0x0A34F4AFUL, (int32_t)0xBFAC4AF4UL, (int32_t)0x297B3CD5UL},
  {(int32_t)0x0A5899E7UL, (int32_t)0x14B133CEUL, (int32_t)0x0A5899E7UL, (int32_t)0xBEB5CDFDUL, (int32_t)0x2BF6CBA3UL},
  {(int32_t)0x0A81A20FUL, (int32_t)0x1503441EUL, (int32_t)0x0A81A20FUL, (int32_t)0xBDB38B06UL, (int32_t)0x2E9F722FUL},
  {(int32_t)0x0AAE9813UL, (int32_t)0x155D3027UL, (int32_t)0x0AAE9813UL, (int32_t)0xBC996ACBUL, (int32_t)0x31878AB8UL},
  {(int32_t)0x0ADF19C2UL, (int32_t)0x15BE3384UL, (int32_t)0x0ADF19C2UL, (int32_t)0xBB66B0D0UL, (int32_t)0x34AF0568UL},
  {(int32_t)0x0B13B694UL, (int32_t)0x16276D28UL, (int32_t)0x0B13B694UL, (int32_t)0xBA1AF801UL, (int32_t)0x3818EA4EUL},
  {(int32_t)0x0B4C8521UL, (int32_t)0x16990A41UL, (int32_t)0x0B4C8521UL, (int32_t)0xB8B478AFUL, (int32_t)0x3BC92324UL},
  {(int32_t)0x0B89C6F5UL, (int32_t)0x17138DEBUL, (int32_t)0x0B89C6F5UL, (int32_t)0xB731EA12UL, (int32_t)0x3FC347B2UL},
  {(int32_t)0x0BCBBEDCUL, (int32_t)0x17977DB8UL, (int32_t)0x0BCBBEDCUL, (int32_t)0xB591AF34UL, (int32_t)0x440B9D09UL},
  {(int32_t)0x0C12B00BUL, (int32_t)0x18256017UL, (int32_t)0x0C12B00BUL, (int32_t)0xB3D209F6UL, (int32_t)0x48A6AC42UL},
  {(int32_t)0x0C5EE733UL, (int32_t)0x18BDCE65UL, (int32_t)0x0C5EE733UL, (int32_t)0xB1F12055UL, (int32_t)0x4D995C22UL},
  {(int32_t)0x0CB0B637UL, (int32_t)0x19616C6FUL, (int32_t)0x0CB0B637UL, (int32_t)0xAFECEC0AUL, (int32_t)0x52E900C9UL},
  {(int32_t)0x0D087568UL, (int32_t)0x1A10EAD0UL, (int32_t)0x0D087568UL, (int32_t)0xADC33F35UL, (int32_t)0x589B5736UL},
  {(int32_t)0x0D66840FUL, (int32_t)0x1ACD081FUL, (int32_t)0x0D66840FUL, (int32_t)0xAB71C119UL, (int32_t)0x5EB68E0BUL},
  {(int32_t)0x0DCB4870UL, (int32_t)0x1B9690DFUL, (int32_t)0x0DCB4870UL, (int32_t)0xA8F5EC4BUL, (int32_t)0x6541492AUL},
  {(int32_t)0x0E37300FUL, (int32_t)0x1C6E601EUL, (int32_t)0x0E37300FUL, (int32_t)0xA64D0DB9UL, (int32_t)0x6C42A4C9UL},
  {(int32_t)0x0EAAAFD0UL, (int32_t)0x1D555F9FUL, (int32_t)0x0EAAAFD0UL, (int32_t)0xA37443E0UL, (int32_t)0x73C2377FUL},
  {(int32_t)0x0F2643ECUL, (int32_t)0x1E4C87D8UL, (int32_t)0x0F2643ECUL, (int32_t)0xA0687EDEUL, (int32_t)0x7BC811F3UL}
};

const biquadInt32_t IIR_butterworth_resampling_ratio4_biquadInt32 =
{
  (int32_t)0x40000000UL, 1, sizeof(IIR_butterworth_resampling_ratio4_BiquadCellInt32) / sizeof(IIR_butterworth_resampling_ratio4_BiquadCellInt32[0]), IIR_butterworth_resampling_ratio4_BiquadCellInt32
};

// 32000 Hz low-pass IIR
//     pass frequency :   3500 Hz
//     stop frequency :   4000 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_chebyshev1_resampling_ratio4_BiquadCellFloat[] =
{
  {+0.00327106, +0.00654212, +0.00327106, -1.80161616, +0.81470041},
  {+0.01010426, +0.02020852, +0.01010426, -1.78063995, +0.82105699},
  {+0.02280383, +0.04560767, +0.02280383, -1.74211040, +0.83332574},
  {+0.03964861, +0.07929722, +0.03964861, -1.69213225, +0.85072668},
  {+0.05850285, +0.11700570, +0.05850285, -1.63830564, +0.87231704},
  {+0.07715369, +0.15430738, +0.07715369, -1.58850331, +0.89716495},
  {+0.09347063, +0.18694126, +0.09347063, -1.54991954, +0.92447492},
  {+0.10589992, +0.21179984, +0.10589992, -1.52849405, +0.95364399},
  {+0.11323255, +0.22646511, +0.11323255, -1.52863713, +0.98425204}
};

const biquadFloat_t IIR_chebyshev1_resampling_ratio4_biquadFloat =
{
  sizeof(IIR_chebyshev1_resampling_ratio4_BiquadCellFloat) / sizeof(IIR_chebyshev1_resampling_ratio4_BiquadCellFloat[0]), IIR_chebyshev1_resampling_ratio4_BiquadCellFloat
};

static const biquadCellInt32_t IIR_chebyshev1_resampling_ratio4_BiquadCellInt32[] =
{
  {(int32_t)0x006B2FA7UL, (int32_t)0x00D65F4EUL, (int32_t)0x006B2FA7UL, (int32_t)0x8CB25220UL, (int32_t)0x68481A5BUL},
  {(int32_t)0x014B18ACUL, (int32_t)0x02963157UL, (int32_t)0x014B18ACUL, (int32_t)0x8E09FEBBUL, (int32_t)0x69186539UL},
  {(int32_t)0x02EB3C6DUL, (int32_t)0x05D678DAUL, (int32_t)0x02EB3C6DUL, (int32_t)0x9081435EUL, (int32_t)0x6AAA6AF9UL},
  {(int32_t)0x051334A1UL, (int32_t)0x0A266943UL, (int32_t)0x051334A1UL, (int32_t)0x93B41AECUL, (int32_t)0x6CE49CADUL},
  {(int32_t)0x077D0577UL, (int32_t)0x0EFA0AEEUL, (int32_t)0x077D0577UL, (int32_t)0x97260018UL, (int32_t)0x6FA815ACUL},
  {(int32_t)0x09E02C13UL, (int32_t)0x13C05825UL, (int32_t)0x09E02C13UL, (int32_t)0x9A55F638UL, (int32_t)0x72D64D1AUL},
  {(int32_t)0x0BF6D87AUL, (int32_t)0x17EDB0F4UL, (int32_t)0x0BF6D87AUL, (int32_t)0x9CCE1E4BUL, (int32_t)0x765531BBUL},
  {(int32_t)0x0D8E20E7UL, (int32_t)0x1B1C41CDUL, (int32_t)0x0D8E20E7UL, (int32_t)0x9E2D2747UL, (int32_t)0x7A1101A3UL},
  {(int32_t)0x0E7E6782UL, (int32_t)0x1CFCCF04UL, (int32_t)0x0E7E6782UL, (int32_t)0x9E2ACF31UL, (int32_t)0x7DFBF88DUL}
};

const biquadInt32_t IIR_chebyshev1_resampling_ratio4_biquadInt32 =
{
  (int32_t)0x7FFFFFF0UL, 0, sizeof(IIR_chebyshev1_resampling_ratio4_BiquadCellInt32) / sizeof(IIR_chebyshev1_resampling_ratio4_BiquadCellInt32[0]), IIR_chebyshev1_resampling_ratio4_BiquadCellInt32
};

// 32000 Hz low-pass IIR
//     pass frequency :   3500 Hz
//     stop frequency :   4000 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_chebyshev2_resampling_ratio4_BiquadCellFloat[] =
{
  {+0.24635593, +0.45093343, +0.24635593, -0.06927490, +0.01292020},
  {+0.30751724, +0.26963318, +0.30751724, -0.21806225, +0.10272991},
  {+0.40441062, -0.01624511, +0.40441062, -0.45317275, +0.24574888},
  {+0.50824734, -0.31966738, +0.50824734, -0.70381105, +0.40063836},
  {+0.60202717, -0.58883275, +0.60202717, -0.92798921, +0.54321080},
  {+0.68037106, -0.80657368, +0.68037106, -1.11207305, +0.66624149},
  {+0.74412492, -0.97394596, +0.74412492, -1.25746898, +0.77177285},
  {+0.79637966, -1.09795964, +0.79637966, -1.37073047, +0.86553016},
  {+0.84069096, -1.18570272, +0.84069096, -1.45895841, +0.95463761}
};

const biquadFloat_t IIR_chebyshev2_resampling_ratio4_biquadFloat =
{
  sizeof(IIR_chebyshev2_resampling_ratio4_BiquadCellFloat) / sizeof(IIR_chebyshev2_resampling_ratio4_BiquadCellFloat[0]), IIR_chebyshev2_resampling_ratio4_BiquadCellFloat
};

static const biquadCellInt32_t IIR_chebyshev2_resampling_ratio4_BiquadCellInt32[] =
{
  {(int32_t)0x1F889759UL, (int32_t)0x39B82FC9UL, (int32_t)0x1F889759UL, (int32_t)0xFB910001UL, (int32_t)0x01A75E79UL},
  {(int32_t)0x275CB99EUL, (int32_t)0x22835702UL, (int32_t)0x275CB99EUL, (int32_t)0xF20B44A7UL, (int32_t)0x0D2640F0UL},
  {(int32_t)0x33C3BA31UL, (int32_t)0xFDEBAE1DUL, (int32_t)0x33C3BA31UL, (int32_t)0xE2FF37BAUL, (int32_t)0x1F74B30BUL},
  {(int32_t)0x410E3FBBUL, (int32_t)0xD71523B1UL, (int32_t)0x410E3FBBUL, (int32_t)0xD2F4C27EUL, (int32_t)0x33481E2BUL},
  {(int32_t)0x4D0F39F1UL, (int32_t)0xB4A120DAUL, (int32_t)0x4D0F39F1UL, (int32_t)0xC49BD324UL, (int32_t)0x4587EE74UL},
  {(int32_t)0x57166621UL, (int32_t)0x98C2318DUL, (int32_t)0x57166621UL, (int32_t)0xB8D3CB90UL, (int32_t)0x554766AEUL},
  {(int32_t)0x5F3F7C3BUL, (int32_t)0x8355BD18UL, (int32_t)0x5F3F7C3BUL, (int32_t)0xAF85A0D6UL, (int32_t)0x62C973E2UL},
  {(int32_t)0x32F7E264UL, (int32_t)0xB9BB0780UL, (int32_t)0x32F7E264UL, (int32_t)0xA845F3B1UL, (int32_t)0x6EC9B130UL},
  {(int32_t)0x35CDE173UL, (int32_t)0xB41D7256UL, (int32_t)0x35CDE173UL, (int32_t)0xA2A06CE6UL, (int32_t)0x7A3190AAUL}
};

const biquadInt32_t IIR_chebyshev2_resampling_ratio4_biquadInt32 =
{
  (int32_t)0x40000000UL, 3, sizeof(IIR_chebyshev2_resampling_ratio4_BiquadCellInt32) / sizeof(IIR_chebyshev2_resampling_ratio4_BiquadCellInt32[0]), IIR_chebyshev2_resampling_ratio4_BiquadCellInt32
};

// 32000 Hz low-pass IIR
//     pass frequency :   3500 Hz
//     stop frequency :   4000 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_elliptic_resampling_ratio4_BiquadCellFloat[] =
{
  {+0.13664632, +0.13664632, +0.00000000, -0.72670736, +0.00000000},
  {+0.09723179, -0.03426296, +0.09723179, -1.46400633, +0.62420696},
  {+0.38356357, -0.45638905, +0.38356357, -1.48366161, +0.79439969},
  {+0.68048874, -0.94894866, +0.68048874, -1.50065110, +0.91267992},
  {+0.83461201, -1.21165916, +0.83461201, -1.51925185, +0.97681672}
};

const biquadFloat_t IIR_elliptic_resampling_ratio4_biquadFloat =
{
  sizeof(IIR_elliptic_resampling_ratio4_BiquadCellFloat) / sizeof(IIR_elliptic_resampling_ratio4_BiquadCellFloat[0]), IIR_elliptic_resampling_ratio4_BiquadCellFloat
};

static const biquadCellInt32_t IIR_elliptic_resampling_ratio4_BiquadCellInt32[] =
{
  {(int32_t)0x117DA06CUL, (int32_t)0x117DA06CUL, (int32_t)0x00000000UL, (int32_t)0xD17DA06CUL, (int32_t)0x00000000UL},
  {(int32_t)0x0C721764UL, (int32_t)0xFB9D457AUL, (int32_t)0x0C721764UL, (int32_t)0xA24DB862UL, (int32_t)0x4FE6037EUL},
  {(int32_t)0x31189C6DUL, (int32_t)0xC5950B24UL, (int32_t)0x31189C6DUL, (int32_t)0xA10BB02EUL, (int32_t)0x65AEE3A2UL},
  {(int32_t)0x571A414DUL, (int32_t)0x8688D9A9UL, (int32_t)0x571A414DUL, (int32_t)0x9FF55513UL, (int32_t)0x74D2B21CUL},
  {(int32_t)0x356A4883UL, (int32_t)0xB2742D27UL, (int32_t)0x356A4883UL, (int32_t)0x9EC493E8UL, (int32_t)0x7D08548BUL}
};

const biquadInt32_t IIR_elliptic_resampling_ratio4_biquadInt32 =
{
  (int32_t)0x40000000UL, 2, sizeof(IIR_elliptic_resampling_ratio4_BiquadCellInt32) / sizeof(IIR_elliptic_resampling_ratio4_BiquadCellInt32[0]), IIR_elliptic_resampling_ratio4_BiquadCellInt32
};

#endif /* AC_SUPPORT_RATIO_4 */

#ifdef AC_SUPPORT_RATIO_6
// 48000 Hz low-pass IIR
//     pass frequency :   3500 Hz
//     stop frequency :   4000 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_butterworth_resampling_ratio6_BiquadCellFloat[] =
{
  {+0.04320722, +0.08641443, +0.04320722, -1.16895896, +0.34178783},
  {+0.04303706, +0.08607413, +0.04303706, -1.17388623, +0.34603449},
  {+0.04268659, +0.08537318, +0.04268659, -1.18377206, +0.35451842},
  {+0.04213324, +0.08426649, +0.04213324, -1.19870302, +0.36723599},
  {+0.04131076, +0.08262151, +0.04131076, -1.21921231, +0.38445533},
  {+0.04009408, +0.08018817, +0.04009408, -1.24793496, +0.40831129},
  {+0.03782322, +0.07564643, +0.03782322, -1.27100257, +0.42229543},
  {+0.04032451, +0.08064901, +0.04032451, -1.26626460, +0.42756263},
  {+0.03472373, +0.06944746, +0.03472373, -1.29432529, +0.43322021},
  {+0.03029672, +0.06059343, +0.03029672, -1.31846431, +0.43965118},
  {+0.02852463, +0.05704926, +0.02852463, -1.32609703, +0.44019555},
  {+0.03934237, +0.07868474, +0.03934237, -1.29546078, +0.45283026},
  {+0.03931915, +0.07863830, +0.03931915, -1.29849582, +0.45577243},
  {+0.04054564, +0.08109128, +0.04054564, -1.30834821, +0.47053077},
  {+0.04069572, +0.08139145, +0.04069572, -1.32565812, +0.48844101},
  {+0.04107984, +0.08215969, +0.04107984, -1.34032051, +0.50463988},
  {+0.04165012, +0.08330024, +0.04165012, -1.35578832, +0.52238879},
  {+0.04217302, +0.08434604, +0.04217302, -1.37331013, +0.54200220},
  {+0.04273292, +0.08546584, +0.04273292, -1.39185110, +0.56278277},
  {+0.04334301, +0.08668602, +0.04334301, -1.41159135, +0.58496339},
  {+0.04399026, +0.08798052, +0.04399026, -1.43266831, +0.60862935},
  {+0.04467794, +0.08935587, +0.04467794, -1.45507609, +0.63378784},
  {+0.04540816, +0.09081632, +0.04540816, -1.47885634, +0.66048899},
  {+0.04618183, +0.09236366, +0.04618183, -1.50405272, +0.68878004},
  {+0.04700023, +0.09400046, +0.04700023, -1.53070649, +0.71870741},
  {+0.04786466, +0.09572932, +0.04786466, -1.55885960, +0.75031825},
  {+0.04877644, +0.09755289, +0.04877644, -1.58855440, +0.78366018},
  {+0.04973684, +0.09947369, +0.04973684, -1.61983285, +0.81878022},
  {+0.05074713, +0.10149425, +0.05074713, -1.65273576, +0.85572426},
  {+0.05180849, +0.10361698, +0.05180849, -1.68730239, +0.89453635},
  {+0.05292207, +0.10584414, +0.05292207, -1.72356944, +0.93525772},
  {+0.05408888, +0.10817776, +0.05408888, -1.76157019, +0.97792571}
};

const biquadFloat_t IIR_butterworth_resampling_ratio6_biquadFloat =
{
  sizeof(IIR_butterworth_resampling_ratio6_BiquadCellFloat) / sizeof(IIR_butterworth_resampling_ratio6_BiquadCellFloat[0]), IIR_butterworth_resampling_ratio6_BiquadCellFloat
};

static const biquadCellInt32_t IIR_butterworth_resampling_ratio6_BiquadCellInt32[] =
{
  {(int32_t)0x0587D068UL, (int32_t)0x0B0FA0CFUL, (int32_t)0x0587D068UL, (int32_t)0xB52FC6C5UL, (int32_t)0x2BBFB415UL},
  {(int32_t)0x05823D11UL, (int32_t)0x0B047A22UL, (int32_t)0x05823D11UL, (int32_t)0xB4DF0C4FUL, (int32_t)0x2C4ADBA8UL},
  {(int32_t)0x0576C116UL, (int32_t)0x0AED822DUL, (int32_t)0x0576C116UL, (int32_t)0xB43D1421UL, (int32_t)0x2D60DC18UL},
  {(int32_t)0x05649F43UL, (int32_t)0x0AC93E86UL, (int32_t)0x05649F43UL, (int32_t)0xB3487320UL, (int32_t)0x2F0196CCUL},
  {(int32_t)0x0549ABBBUL, (int32_t)0x0A935776UL, (int32_t)0x0549ABBBUL, (int32_t)0xB1F86CF2UL, (int32_t)0x3135D509UL},
  {(int32_t)0x0521CD8CUL, (int32_t)0x0A439B17UL, (int32_t)0x0521CD8CUL, (int32_t)0xB021D567UL, (int32_t)0x34438B61UL},
  {(int32_t)0x04D76423UL, (int32_t)0x09AEC847UL, (int32_t)0x04D76423UL, (int32_t)0xAEA7E4D8UL, (int32_t)0x360DC6DDUL},
  {(int32_t)0x05295A79UL, (int32_t)0x0A52B4F1UL, (int32_t)0x05295A79UL, (int32_t)0xAEF5854DUL, (int32_t)0x36BA5F48UL},
  {(int32_t)0x0471D3C7UL, (int32_t)0x08E3A78DUL, (int32_t)0x0471D3C7UL, (int32_t)0xAD29C646UL, (int32_t)0x3773C28EUL},
  {(int32_t)0x03E0C349UL, (int32_t)0x07C18692UL, (int32_t)0x03E0C349UL, (int32_t)0xAB9E47DCUL, (int32_t)0x38467D6BUL},
  {(int32_t)0x03A6B1EDUL, (int32_t)0x074D63DBUL, (int32_t)0x03A6B1EDUL, (int32_t)0xAB2139E7UL, (int32_t)0x385853E7UL},
  {(int32_t)0x05092BB9UL, (int32_t)0x0A125772UL, (int32_t)0x05092BB9UL, (int32_t)0xAD172BACUL, (int32_t)0x39F6578CUL},
  {(int32_t)0x050868F0UL, (int32_t)0x0A10D1E1UL, (int32_t)0x050868F0UL, (int32_t)0xACE571C4UL, (int32_t)0x3A56C039UL},
  {(int32_t)0x0530997BUL, (int32_t)0x0A6132F5UL, (int32_t)0x0530997BUL, (int32_t)0xAC4405DDUL, (int32_t)0x3C3A5A30UL},
  {(int32_t)0x05358478UL, (int32_t)0x0A6B08F1UL, (int32_t)0x05358478UL, (int32_t)0xAB286AD8UL, (int32_t)0x3E853C30UL},
  {(int32_t)0x05421AB4UL, (int32_t)0x0A843568UL, (int32_t)0x05421AB4UL, (int32_t)0xAA383058UL, (int32_t)0x40980A20UL},
  {(int32_t)0x0554CA86UL, (int32_t)0x0AA9950CUL, (int32_t)0x0554CA86UL, (int32_t)0xA93AC3A6UL, (int32_t)0x42DDA2CEUL},
  {(int32_t)0x0565ECEAUL, (int32_t)0x0ACBD9D4UL, (int32_t)0x0565ECEAUL, (int32_t)0xA81BAFD7UL, (int32_t)0x456053FCUL},
  {(int32_t)0x057845B2UL, (int32_t)0x0AF08B64UL, (int32_t)0x057845B2UL, (int32_t)0xA6EBE95FUL, (int32_t)0x4809440BUL},
  {(int32_t)0x058C4381UL, (int32_t)0x0B188702UL, (int32_t)0x058C4381UL, (int32_t)0xA5A87CBDUL, (int32_t)0x4AE01489UL},
  {(int32_t)0x05A1790DUL, (int32_t)0x0B42F219UL, (int32_t)0x05A1790DUL, (int32_t)0xA44F2999UL, (int32_t)0x4DE79101UL},
  {(int32_t)0x05B801B1UL, (int32_t)0x0B700361UL, (int32_t)0x05B801B1UL, (int32_t)0xA2E00884UL, (int32_t)0x511FF5B9UL},
  {(int32_t)0x05CFEF45UL, (int32_t)0x0B9FDE8AUL, (int32_t)0x05CFEF45UL, (int32_t)0xA15A6AEEUL, (int32_t)0x548AE737UL},
  {(int32_t)0x05E94943UL, (int32_t)0x0BD29287UL, (int32_t)0x05E94943UL, (int32_t)0x9FBD99A7UL, (int32_t)0x5829F1BEUL},
  {(int32_t)0x06041A82UL, (int32_t)0x0C083505UL, (int32_t)0x06041A82UL, (int32_t)0x9E08E7A9UL, (int32_t)0x5BFE9AB8UL},
  {(int32_t)0x06206DE2UL, (int32_t)0x0C40DBC5UL, (int32_t)0x06206DE2UL, (int32_t)0x9C3BA4EDUL, (int32_t)0x600A6DB0UL},
  {(int32_t)0x063E4E78UL, (int32_t)0x0C7C9CF0UL, (int32_t)0x063E4E78UL, (int32_t)0x9A551FECUL, (int32_t)0x644EFA09UL},
  {(int32_t)0x065DC6E4UL, (int32_t)0x0CBB8DC9UL, (int32_t)0x065DC6E4UL, (int32_t)0x9854A89DUL, (int32_t)0x68CDCA57UL},
  {(int32_t)0x067EE1BFUL, (int32_t)0x0CFDC37FUL, (int32_t)0x067EE1BFUL, (int32_t)0x963993CBUL, (int32_t)0x6D885F68UL},
  {(int32_t)0x06A1A921UL, (int32_t)0x0D435242UL, (int32_t)0x06A1A921UL, (int32_t)0x94033CD9UL, (int32_t)0x72802AD2UL},
  {(int32_t)0x06C6267EUL, (int32_t)0x0D8C4CFBUL, (int32_t)0x06C6267EUL, (int32_t)0x91B109CDUL, (int32_t)0x77B6865DUL},
  {(int32_t)0x06EC6267UL, (int32_t)0x0DD8C4CFUL, (int32_t)0x06EC6267UL, (int32_t)0x8F426F1BUL, (int32_t)0x7D2CAB67UL}
};

const biquadInt32_t IIR_butterworth_resampling_ratio6_biquadInt32 =
{
  (int32_t)0x40000000UL, 1, sizeof(IIR_butterworth_resampling_ratio6_BiquadCellInt32) / sizeof(IIR_butterworth_resampling_ratio6_BiquadCellInt32[0]), IIR_butterworth_resampling_ratio6_BiquadCellInt32
};

// 48000 Hz low-pass IIR
//     pass frequency :   3500 Hz
//     stop frequency :   4000 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_chebyshev1_resampling_ratio6_BiquadCellFloat[] =
{
  {+0.00143821, +0.00287642, +0.00143821, -1.86922480, +0.87497764},
  {+0.00445564, +0.00891128, +0.00445564, -1.86108982, +0.87891237},
  {+0.01011000, +0.02021999, +0.01011000, -1.84616571, +0.88660569},
  {+0.01770115, +0.03540231, +0.01770115, -1.82692839, +0.89773301},
  {+0.02631471, +0.05262943, +0.02631471, -1.80660902, +0.91186787},
  {+0.03494112, +0.06988225, +0.03494112, -1.78874804, +0.92853376},
  {+0.04254490, +0.08508980, +0.04254490, -1.77676162, +0.94724749},
  {+0.04831609, +0.09663218, +0.04831609, -1.77357332, +0.96754498},
  {+0.05160893, +0.10321786, +0.05160893, -1.78132633, +0.98898569}
};

const biquadFloat_t IIR_chebyshev1_resampling_ratio6_biquadFloat =
{
  sizeof(IIR_chebyshev1_resampling_ratio6_BiquadCellFloat) / sizeof(IIR_chebyshev1_resampling_ratio6_BiquadCellFloat[0]), IIR_chebyshev1_resampling_ratio6_BiquadCellFloat
};

static const biquadCellInt32_t IIR_chebyshev1_resampling_ratio6_BiquadCellInt32[] =
{
  {(int32_t)0x002F2094UL, (int32_t)0x005E4129UL, (int32_t)0x002F2094UL, (int32_t)0x885E9EF5UL, (int32_t)0x6FFF4468UL},
  {(int32_t)0x0092009AUL, (int32_t)0x01240133UL, (int32_t)0x0092009AUL, (int32_t)0x88E3E78BUL, (int32_t)0x70803351UL},
  {(int32_t)0x014B48CAUL, (int32_t)0x02969194UL, (int32_t)0x014B48CAUL, (int32_t)0x89D86BC8UL, (int32_t)0x717C4B97UL},
  {(int32_t)0x0244080CUL, (int32_t)0x04881019UL, (int32_t)0x0244080CUL, (int32_t)0x8B139AF1UL, (int32_t)0x72E8EA50UL},
  {(int32_t)0x035E47D3UL, (int32_t)0x06BC8FA6UL, (int32_t)0x035E47D3UL, (int32_t)0x8C608493UL, (int32_t)0x74B81626UL},
  {(int32_t)0x0478F363UL, (int32_t)0x08F1E6C5UL, (int32_t)0x0478F363UL, (int32_t)0x8D8526F5UL, (int32_t)0x76DA31B7UL},
  {(int32_t)0x05721C7AUL, (int32_t)0x0AE438F4UL, (int32_t)0x05721C7AUL, (int32_t)0x8E49899EUL, (int32_t)0x793F67DAUL},
  {(int32_t)0x062F38C1UL, (int32_t)0x0C5E7181UL, (int32_t)0x062F38C1UL, (int32_t)0x8E7DC656UL, (int32_t)0x7BD88398UL},
  {(int32_t)0x069B1F18UL, (int32_t)0x0D363E30UL, (int32_t)0x069B1F18UL, (int32_t)0x8DFEBFD5UL, (int32_t)0x7E97153DUL}
};

const biquadInt32_t IIR_chebyshev1_resampling_ratio6_biquadInt32 =
{
  (int32_t)0x7FFFFFFAUL, 0, sizeof(IIR_chebyshev1_resampling_ratio6_BiquadCellInt32) / sizeof(IIR_chebyshev1_resampling_ratio6_BiquadCellInt32[0]), IIR_chebyshev1_resampling_ratio6_BiquadCellInt32
};

// 48000 Hz low-pass IIR
//     pass frequency :   3500 Hz
//     stop frequency :   4000 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_chebyshev2_resampling_ratio6_BiquadCellFloat[] =
{
  {+0.15854661, +0.25641577, +0.15854661, -0.49911557, +0.07262457},
  {+0.25092477, +0.01739127, +0.25092477, -0.66664804, +0.18588884},
  {+0.38475383, -0.32823361, +0.38475383, -0.90956079, +0.35083483},
  {+0.51324816, -0.65872403, +0.51324816, -1.14322121, +0.51099349},
  {+0.61763960, -0.92506739, +0.61763960, -1.33373534, +0.64394716},
  {+0.69703655, -1.12458192, +0.69703655, -1.47960517, +0.74909635},
  {+0.75638833, -1.26957554, +0.75638833, -1.58996472, +0.83316584},
  {+0.80108995, -1.37321840, +0.80108995, -1.67484853, +0.90381002},
  {+0.83553041, -1.44558309, +0.83553041, -1.74262374, +0.96810146}
};

const biquadFloat_t IIR_chebyshev2_resampling_ratio6_biquadFloat =
{
  sizeof(IIR_chebyshev2_resampling_ratio6_BiquadCellFloat) / sizeof(IIR_chebyshev2_resampling_ratio6_BiquadCellFloat[0]), IIR_chebyshev2_resampling_ratio6_BiquadCellFloat
};

static const biquadCellInt32_t IIR_chebyshev2_resampling_ratio6_BiquadCellInt32[] =
{
  {(int32_t)0x144B4164UL, (int32_t)0x20D23B67UL, (int32_t)0x144B4164UL, (int32_t)0xE00E7D91UL, (int32_t)0x094BC30DUL},
  {(int32_t)0x201E4D7EUL, (int32_t)0x0239E08AUL, (int32_t)0x201E4D7EUL, (int32_t)0xD555A378UL, (int32_t)0x17CB3495UL},
  {(int32_t)0x313F9D07UL, (int32_t)0xD5FC70E0UL, (int32_t)0x313F9D07UL, (int32_t)0xC5C9C18AUL, (int32_t)0x2CE827DBUL},
  {(int32_t)0x41B21D9EUL, (int32_t)0xABAEEE4CUL, (int32_t)0x41B21D9EUL, (int32_t)0xB6D576B9UL, (int32_t)0x41683C17UL},
  {(int32_t)0x4F0ED082UL, (int32_t)0x89976449UL, (int32_t)0x4F0ED082UL, (int32_t)0xAAA41484UL, (int32_t)0x526CDC46UL},
  {(int32_t)0x2C9C3F2FUL, (int32_t)0xB806D98EUL, (int32_t)0x2C9C3F2FUL, (int32_t)0xA14E2619UL, (int32_t)0x5FE263A7UL},
  {(int32_t)0x3068AA9EUL, (int32_t)0xAEBF463DUL, (int32_t)0x3068AA9EUL, (int32_t)0x9A3E04A3UL, (int32_t)0x6AA52DAAUL},
  {(int32_t)0x33450ECAUL, (int32_t)0xA81D308EUL, (int32_t)0x33450ECAUL, (int32_t)0x94CF4821UL, (int32_t)0x73B00C01UL},
  {(int32_t)0x35795487UL, (int32_t)0xA37B910EUL, (int32_t)0x35795487UL, (int32_t)0x9078DA45UL, (int32_t)0x7BEABFADUL}
};

const biquadInt32_t IIR_chebyshev2_resampling_ratio6_biquadInt32 =
{
  (int32_t)0x40000000UL, 5, sizeof(IIR_chebyshev2_resampling_ratio6_BiquadCellInt32) / sizeof(IIR_chebyshev2_resampling_ratio6_BiquadCellInt32[0]), IIR_chebyshev2_resampling_ratio6_BiquadCellInt32
};

// 48000 Hz low-pass IIR
//     pass frequency :   3500 Hz
//     stop frequency :   4000 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_elliptic_resampling_ratio6_BiquadCellFloat[] =
{
  {+0.09349722, +0.09349722, +0.00000000, -0.81300556, +0.00000000},
  {+0.08139206, -0.08814859, +0.08139206, -1.65670025, +0.73133579},
  {+0.36853746, -0.59356378, +0.36853746, -1.71077712, +0.85428826},
  {+0.67121152, -1.15337198, +0.67121152, -1.74946732, +0.93851838},
  {+0.82653288, -1.44416441, +0.82653288, -1.77485653, +0.98375787}
};

const biquadFloat_t IIR_elliptic_resampling_ratio6_biquadFloat =
{
  sizeof(IIR_elliptic_resampling_ratio6_BiquadCellFloat) / sizeof(IIR_elliptic_resampling_ratio6_BiquadCellFloat[0]), IIR_elliptic_resampling_ratio6_BiquadCellFloat
};

static const biquadCellInt32_t IIR_elliptic_resampling_ratio6_BiquadCellInt32[] =
{
  {(int32_t)0x0BF7B787UL, (int32_t)0x0BF7B787UL, (int32_t)0x00000000UL, (int32_t)0xCBF7B787UL, (int32_t)0x00000000UL},
  {(int32_t)0x0A6B0E20UL, (int32_t)0xF4B78C02UL, (int32_t)0x0A6B0E20UL, (int32_t)0x95F89F85UL, (int32_t)0x5D9C6938UL},
  {(int32_t)0x2F2C3C49UL, (int32_t)0xB4061A2BUL, (int32_t)0x2F2C3C49UL, (int32_t)0x9282A0B3UL, (int32_t)0x6D595158UL},
  {(int32_t)0x2AF5212CUL, (int32_t)0xB62F274CUL, (int32_t)0x2AF5212CUL, (int32_t)0x9008BA3CUL, (int32_t)0x78215ED0UL},
  {(int32_t)0x34E5EA25UL, (int32_t)0xA392CF6EUL, (int32_t)0x34E5EA25UL, (int32_t)0x8E68C02BUL, (int32_t)0x7DEBC71BUL}
};

const biquadInt32_t IIR_elliptic_resampling_ratio6_biquadInt32 =
{
  (int32_t)0x40000000UL, 3, sizeof(IIR_elliptic_resampling_ratio6_BiquadCellInt32) / sizeof(IIR_elliptic_resampling_ratio6_BiquadCellInt32[0]), IIR_elliptic_resampling_ratio6_BiquadCellInt32
};

#endif /* AC_SUPPORT_RATIO_6 */

#ifdef AC_SUPPORT_RATIO_8
// 64000 Hz low-pass IIR
//     pass frequency :   3500 Hz
//     stop frequency :   4000 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_butterworth_resampling_ratio8_BiquadCellFloat[] =
{
  {+0.16313444, +0.16313444, +0.00000000, -0.67373112, +0.00000000},
  {+0.01972824, +0.03945648, +0.01972824, -1.43649850, +0.51541145},
  {+0.02676017, +0.05352035, +0.02676017, -1.34744959, +0.45449029},
  {+0.02683156, +0.05366312, +0.02683156, -1.35146025, +0.45878650},
  {+0.02675103, +0.05350206, +0.02675103, -1.36007649, +0.46708061},
  {+0.02655395, +0.05310791, +0.02655395, -1.37264496, +0.47886078},
  {+0.02626267, +0.05252534, +0.02626267, -1.38868021, +0.49373089},
  {+0.02588279, +0.05176558, +0.02588279, -1.40788319, +0.51141435},
  {+0.02540340, +0.05080680, +0.02540340, -1.43001722, +0.53163083},
  {+0.01711874, +0.03423747, +0.01711874, -1.48278731, +0.55126225},
  {+0.02480311, +0.04960622, +0.02480311, -1.45530621, +0.55451864},
  {+0.01807398, +0.03614797, +0.01807398, -1.48518008, +0.55747601},
  {+0.01989770, +0.03979539, +0.01989770, -1.48450570, +0.56409649},
  {+0.02269833, +0.04539665, +0.02269833, -1.47858422, +0.56937753},
  {+0.02453599, +0.04907197, +0.02453599, -1.47584751, +0.57399145},
  {+0.02462457, +0.04924913, +0.02462457, -1.48952209, +0.58802036},
  {+0.02483049, +0.04966098, +0.02483049, -1.50339000, +0.60271197},
  {+0.02506899, +0.05013798, +0.02506899, -1.51775853, +0.61803449},
  {+0.02533995, +0.05067989, +0.02533995, -1.53322759, +0.63458738},
  {+0.02560889, +0.05121777, +0.02560889, -1.54984486, +0.65228041},
  {+0.02589834, +0.05179668, +0.02589834, -1.56738144, +0.67097480},
  {+0.02620570, +0.05241140, +0.02620570, -1.58594354, +0.69076635},
  {+0.02652950, +0.05305901, +0.02652950, -1.60554423, +0.71166224},
  {+0.02687055, +0.05374109, +0.02687055, -1.62618788, +0.73367006},
  {+0.02722913, +0.05445826, +0.02722913, -1.64788801, +0.75680453},
  {+0.02760535, +0.05521070, +0.02760535, -1.67065631, +0.78107770},
  {+0.02799936, +0.05599873, +0.02799936, -1.69450183, +0.80649929},
  {+0.02841130, +0.05682260, +0.02841130, -1.71943208, +0.83307729},
  {+0.02884125, +0.05768250, +0.02884125, -1.74545215, +0.86081715},
  {+0.02928924, +0.05857848, +0.02928924, -1.77256438, +0.88972135},
  {+0.02975526, +0.05951053, +0.02975526, -1.80076773, +0.91978879},
  {+0.03023923, +0.06047847, +0.03023923, -1.83005728, +0.95101421},
  {+0.03074100, +0.06148199, +0.03074100, -1.86042355, +0.98338753}
};

const biquadFloat_t IIR_butterworth_resampling_ratio8_biquadFloat =
{
  sizeof(IIR_butterworth_resampling_ratio8_BiquadCellFloat) / sizeof(IIR_butterworth_resampling_ratio8_BiquadCellFloat[0]), IIR_butterworth_resampling_ratio8_BiquadCellFloat
};

static const biquadCellInt32_t IIR_butterworth_resampling_ratio8_BiquadCellInt32[] =
{
  {(int32_t)0x14E196DDUL, (int32_t)0x14E196DDUL, (int32_t)0x00000000UL, (int32_t)0xD4E196DDUL, (int32_t)0x00000000UL},
  {(int32_t)0x02867474UL, (int32_t)0x050CE8E8UL, (int32_t)0x02867474UL, (int32_t)0xA410689DUL, (int32_t)0x41F90096UL},
  {(int32_t)0x036CE09CUL, (int32_t)0x06D9C139UL, (int32_t)0x036CE09CUL, (int32_t)0xA9C362CBUL, (int32_t)0x3A2CBCDCUL},
  {(int32_t)0x036F3775UL, (int32_t)0x06DE6EEAUL, (int32_t)0x036F3775UL, (int32_t)0xA981ACDDUL, (int32_t)0x3AB98419UL},
  {(int32_t)0x036C93E8UL, (int32_t)0x06D927D0UL, (int32_t)0x036C93E8UL, (int32_t)0xA8F481C1UL, (int32_t)0x3BC94C1DUL},
  {(int32_t)0x03661EB6UL, (int32_t)0x06CC3D6CUL, (int32_t)0x03661EB6UL, (int32_t)0xA82695C1UL, (int32_t)0x3D4B4F56UL},
  {(int32_t)0x035C9342UL, (int32_t)0x06B92684UL, (int32_t)0x035C9342UL, (int32_t)0xA71FDD0EUL, (int32_t)0x3F3292ECUL},
  {(int32_t)0x03502093UL, (int32_t)0x06A04126UL, (int32_t)0x03502093UL, (int32_t)0xA5E53DEAUL, (int32_t)0x4176067AUL},
  {(int32_t)0x03406B2FUL, (int32_t)0x0680D65EUL, (int32_t)0x03406B2FUL, (int32_t)0xA47A9911UL, (int32_t)0x440C7A9AUL},
  {(int32_t)0x0230F25CUL, (int32_t)0x0461E4B9UL, (int32_t)0x0230F25CUL, (int32_t)0xA11A0346UL, (int32_t)0x468FC2E4UL},
  {(int32_t)0x032CBF8FUL, (int32_t)0x06597F1DUL, (int32_t)0x032CBF8FUL, (int32_t)0xA2DC4359UL, (int32_t)0x46FA7788UL},
  {(int32_t)0x02503F91UL, (int32_t)0x04A07F22UL, (int32_t)0x02503F91UL, (int32_t)0xA0F2CF44UL, (int32_t)0x475B5FBCUL},
  {(int32_t)0x028C01FBUL, (int32_t)0x051803F6UL, (int32_t)0x028C01FBUL, (int32_t)0xA0FDDBCBUL, (int32_t)0x48345056UL},
  {(int32_t)0x02E7C75BUL, (int32_t)0x05CF8EB7UL, (int32_t)0x02E7C75BUL, (int32_t)0xA15EE047UL, (int32_t)0x48E15CE1UL},
  {(int32_t)0x0323FEC4UL, (int32_t)0x0647FD89UL, (int32_t)0x0323FEC4UL, (int32_t)0xA18BB6E5UL, (int32_t)0x49788D47UL},
  {(int32_t)0x0326E5D7UL, (int32_t)0x064DCBAEUL, (int32_t)0x0326E5D7UL, (int32_t)0xA0ABAB88UL, (int32_t)0x4B44404CUL},
  {(int32_t)0x032DA543UL, (int32_t)0x065B4A87UL, (int32_t)0x032DA543UL, (int32_t)0x9FC8754DUL, (int32_t)0x4D25AA72UL},
  {(int32_t)0x033575F0UL, (int32_t)0x066AEBE0UL, (int32_t)0x033575F0UL, (int32_t)0x9EDD0B58UL, (int32_t)0x4F1BC110UL},
  {(int32_t)0x033E56DEUL, (int32_t)0x067CADBDUL, (int32_t)0x033E56DEUL, (int32_t)0x9DDF995EUL, (int32_t)0x513A28BEUL},
  {(int32_t)0x034726EBUL, (int32_t)0x068E4DD6UL, (int32_t)0x034726EBUL, (int32_t)0x9CCF5785UL, (int32_t)0x537DECA1UL},
  {(int32_t)0x0350A309UL, (int32_t)0x06A14612UL, (int32_t)0x0350A309UL, (int32_t)0x9BB005C6UL, (int32_t)0x55E28099UL},
  {(int32_t)0x035AB55AUL, (int32_t)0x06B56AB4UL, (int32_t)0x035AB55AUL, (int32_t)0x9A7FE6A7UL, (int32_t)0x586B081AUL},
  {(int32_t)0x0365519DUL, (int32_t)0x06CAA339UL, (int32_t)0x0365519DUL, (int32_t)0x993EC36DUL, (int32_t)0x5B17BF98UL},
  {(int32_t)0x03707E79UL, (int32_t)0x06E0FCF1UL, (int32_t)0x03707E79UL, (int32_t)0x97EC89AEUL, (int32_t)0x5DE8E687UL},
  {(int32_t)0x037C3E81UL, (int32_t)0x06F87D01UL, (int32_t)0x037C3E81UL, (int32_t)0x968900BAUL, (int32_t)0x60DEF88EUL},
  {(int32_t)0x03889271UL, (int32_t)0x071124E2UL, (int32_t)0x03889271UL, (int32_t)0x9513F790UL, (int32_t)0x63FA5AA5UL},
  {(int32_t)0x03957BAFUL, (int32_t)0x072AF75FUL, (int32_t)0x03957BAFUL, (int32_t)0x938D482DUL, (int32_t)0x673B5E63UL},
  {(int32_t)0x03A2FB45UL, (int32_t)0x0745F68AUL, (int32_t)0x03A2FB45UL, (int32_t)0x91F4D323UL, (int32_t)0x6AA246CEUL},
  {(int32_t)0x03B111ECUL, (int32_t)0x076223D9UL, (int32_t)0x03B111ECUL, (int32_t)0x904A830CUL, (int32_t)0x6E2F4199UL},
  {(int32_t)0x03BFBFF7UL, (int32_t)0x077F7FEDUL, (int32_t)0x03BFBFF7UL, (int32_t)0x8E8E4E1FUL, (int32_t)0x71E2639CUL},
  {(int32_t)0x03CF053EUL, (int32_t)0x079E0A7CUL, (int32_t)0x03CF053EUL, (int32_t)0x8CC038B1UL, (int32_t)0x75BBA397UL},
  {(int32_t)0x03DEE114UL, (int32_t)0x07BDC229UL, (int32_t)0x03DEE114UL, (int32_t)0x8AE05772UL, (int32_t)0x79BAD56DUL},
  {(int32_t)0x03EF5229UL, (int32_t)0x07DEA453UL, (int32_t)0x03EF5229UL, (int32_t)0x88EED213UL, (int32_t)0x7DDFA47FUL}
};

const biquadInt32_t IIR_butterworth_resampling_ratio8_biquadInt32 =
{
  (int32_t)0x40000000UL, 1, sizeof(IIR_butterworth_resampling_ratio8_BiquadCellInt32) / sizeof(IIR_butterworth_resampling_ratio8_BiquadCellInt32[0]), IIR_butterworth_resampling_ratio8_BiquadCellInt32
};

// 64000 Hz low-pass IIR
//     pass frequency :   3500 Hz
//     stop frequency :   4000 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_chebyshev1_resampling_ratio8_BiquadCellFloat[] =
{
  {+0.00080994, +0.00161988, +0.00080994, -1.90214839, +0.90538816},
  {+0.00251138, +0.00502277, +0.00251138, -1.89824163, +0.90828717},
  {+0.00570736, +0.01141472, +0.00570736, -1.89115000, +0.91397943},
  {+0.01001281, +0.02002562, +0.01001281, -1.88221371, +0.92226495},
  {+0.01491616, +0.02983231, +0.01491616, -1.87320493, +0.93286956},
  {+0.01984085, +0.03968170, +0.01984085, -1.86609256, +0.94546801},
  {+0.02418412, +0.04836823, +0.02418412, -1.86279428, +0.95970484},
  {+0.02746534, +0.05493069, +0.02746534, -1.86494510, +0.97520854},
  {+0.02930060, +0.05860120, +0.02930060, -1.87369986, +0.99159697}
};

const biquadFloat_t IIR_chebyshev1_resampling_ratio8_biquadFloat =
{
  sizeof(IIR_chebyshev1_resampling_ratio8_BiquadCellFloat) / sizeof(IIR_chebyshev1_resampling_ratio8_BiquadCellFloat[0]), IIR_chebyshev1_resampling_ratio8_BiquadCellFloat
};

static const biquadCellInt32_t IIR_chebyshev1_resampling_ratio8_BiquadCellInt32[] =
{
  {(int32_t)0x001A8A46UL, (int32_t)0x0035148CUL, (int32_t)0x001A8A46UL, (int32_t)0x86433361UL, (int32_t)0x73E3C256UL},
  {(int32_t)0x00524B05UL, (int32_t)0x00A4960BUL, (int32_t)0x00524B05UL, (int32_t)0x86833588UL, (int32_t)0x7442C106UL},
  {(int32_t)0x00BB04C9UL, (int32_t)0x01760992UL, (int32_t)0x00BB04C9UL, (int32_t)0x86F76603UL, (int32_t)0x74FD471EUL},
  {(int32_t)0x0148198DUL, (int32_t)0x02903319UL, (int32_t)0x0148198DUL, (int32_t)0x8789CF85UL, (int32_t)0x760CC729UL},
  {(int32_t)0x01E8C5CBUL, (int32_t)0x03D18B96UL, (int32_t)0x01E8C5CBUL, (int32_t)0x881D690DUL, (int32_t)0x77684512UL},
  {(int32_t)0x028A251DUL, (int32_t)0x05144A3BUL, (int32_t)0x028A251DUL, (int32_t)0x8891F087UL, (int32_t)0x7905188AUL},
  {(int32_t)0x03187715UL, (int32_t)0x0630EE29UL, (int32_t)0x03187715UL, (int32_t)0x88C7FA80UL, (int32_t)0x7AD79BBAUL},
  {(int32_t)0x0383FC03UL, (int32_t)0x0707F806UL, (int32_t)0x0383FC03UL, (int32_t)0x88A4BD4AUL, (int32_t)0x7CD3A232UL},
  {(int32_t)0x03C01F3CUL, (int32_t)0x07803E77UL, (int32_t)0x03C01F3CUL, (int32_t)0x88154D2AUL, (int32_t)0x7EECA641UL}
};

const biquadInt32_t IIR_chebyshev1_resampling_ratio8_biquadInt32 =
{
  (int32_t)0x7FFFFFE1UL, 0, sizeof(IIR_chebyshev1_resampling_ratio8_BiquadCellInt32) / sizeof(IIR_chebyshev1_resampling_ratio8_BiquadCellInt32[0]), IIR_chebyshev1_resampling_ratio8_BiquadCellInt32
};

// 64000 Hz low-pass IIR
//     pass frequency :   3500 Hz
//     stop frequency :   4000 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_chebyshev2_resampling_ratio8_BiquadCellFloat[] =
{
  {+0.11539600, +0.15644761, +0.11539600, -0.76925900, +0.15649862},
  {+0.23021008, -0.11848759, +0.23021008, -0.93588648, +0.27781905},
  {+0.38672316, -0.49291323, +0.38672316, -1.16353503, +0.44406811},
  {+0.52691533, -0.82756386, +0.52691533, -1.36846068, +0.59472748},
  {+0.63397601, -1.08199537, +0.63397601, -1.52652968, +0.71248633},
  {+0.71129566, -1.26416656, +0.71129566, -1.64288517, +0.80130993},
  {+0.76648845, -1.39207844, +0.76648845, -1.72889995, +0.86979841},
  {+0.80609623, -1.48101910, +0.80609623, -1.79459250, +0.92576586},
  {+0.83477987, -1.54153660, +0.83477987, -1.84757931, +0.97560245}
};

const biquadFloat_t IIR_chebyshev2_resampling_ratio8_biquadFloat =
{
  sizeof(IIR_chebyshev2_resampling_ratio8_BiquadCellFloat) / sizeof(IIR_chebyshev2_resampling_ratio8_BiquadCellFloat[0]), IIR_chebyshev2_resampling_ratio8_BiquadCellFloat
};

static const biquadCellInt32_t IIR_chebyshev2_resampling_ratio8_BiquadCellInt32[] =
{
  {(int32_t)0x0EC54BD6UL, (int32_t)0x140679B1UL, (int32_t)0x0EC54BD6UL, (int32_t)0xCEC475E8UL, (int32_t)0x1408258FUL},
  {(int32_t)0x1D778615UL, (int32_t)0xF0D56618UL, (int32_t)0x1D778615UL, (int32_t)0xC41A6F98UL, (int32_t)0x238F9310UL},
  {(int32_t)0x318024FBUL, (int32_t)0xC0E83819UL, (int32_t)0x318024FBUL, (int32_t)0xB588A45DUL, (int32_t)0x38D73956UL},
  {(int32_t)0x4371F62DUL, (int32_t)0x96126330UL, (int32_t)0x4371F62DUL, (int32_t)0xA86B23E8UL, (int32_t)0x4C2007B9UL},
  {(int32_t)0x2893101CUL, (int32_t)0xBAC0967DUL, (int32_t)0x2893101CUL, (int32_t)0x9E4D5672UL, (int32_t)0x5B32C088UL},
  {(int32_t)0x2D85DE3DUL, (int32_t)0xAF17E523UL, (int32_t)0x2D85DE3DUL, (int32_t)0x96DAF828UL, (int32_t)0x669152EBUL},
  {(int32_t)0x310E2593UL, (int32_t)0xA6E82FD0UL, (int32_t)0x310E2593UL, (int32_t)0x9159B402UL, (int32_t)0x6F558DE8UL},
  {(int32_t)0x339714A4UL, (int32_t)0xA136FBABUL, (int32_t)0x339714A4UL, (int32_t)0x8D25657BUL, (int32_t)0x767F7EEEUL},
  {(int32_t)0x356D088CUL, (int32_t)0x9D5776DBUL, (int32_t)0x356D088CUL, (int32_t)0x89C142B2UL, (int32_t)0x7CE08A80UL}
};

const biquadInt32_t IIR_chebyshev2_resampling_ratio8_biquadInt32 =
{
  (int32_t)0x40000000UL, 6, sizeof(IIR_chebyshev2_resampling_ratio8_BiquadCellInt32) / sizeof(IIR_chebyshev2_resampling_ratio8_BiquadCellInt32[0]), IIR_chebyshev2_resampling_ratio8_BiquadCellInt32
};

// 64000 Hz low-pass IIR
//     pass frequency :   3500 Hz
//     stop frequency :   4000 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_elliptic_resampling_ratio8_BiquadCellFloat[] =
{
  {+0.07128300, +0.07128300, +0.00000000, -0.85743399, +0.00000000},
  {+0.07632474, -0.10947373, +0.07632474, -1.74797589, +0.79115165},
  {+0.36518112, -0.64803458, +0.36518112, -1.80534597, +0.88767364},
  {+0.66950699, -1.23119750, +0.66950699, -1.84506645, +0.95288293},
  {+0.82414715, -1.52959384, +0.82414715, -1.86889783, +0.98759830}
};

const biquadFloat_t IIR_elliptic_resampling_ratio8_biquadFloat =
{
  sizeof(IIR_elliptic_resampling_ratio8_BiquadCellFloat) / sizeof(IIR_elliptic_resampling_ratio8_BiquadCellFloat[0]), IIR_elliptic_resampling_ratio8_BiquadCellFloat
};

static const biquadCellInt32_t IIR_elliptic_resampling_ratio8_BiquadCellInt32[] =
{
  {(int32_t)0x091FCD2DUL, (int32_t)0x091FCD2DUL, (int32_t)0x00000000UL, (int32_t)0xC91FCD2DUL, (int32_t)0x00000000UL},
  {(int32_t)0x09C5025AUL, (int32_t)0xF1FCC3C7UL, (int32_t)0x09C5025AUL, (int32_t)0x902129BAUL, (int32_t)0x65447507UL},
  {(int32_t)0x2EBE414AUL, (int32_t)0xAD0D33F9UL, (int32_t)0x2EBE414AUL, (int32_t)0x8C753630UL, (int32_t)0x719F4A2DUL},
  {(int32_t)0x2AD933D7UL, (int32_t)0xB1340F6BUL, (int32_t)0x2AD933D7UL, (int32_t)0x89EA6E6AUL, (int32_t)0x79F81160UL},
  {(int32_t)0x34BED3B5UL, (int32_t)0x9E1B2270UL, (int32_t)0x34BED3B5UL, (int32_t)0x8863FA5AUL, (int32_t)0x7E699EFEUL}
};

const biquadInt32_t IIR_elliptic_resampling_ratio8_biquadInt32 =
{
  (int32_t)0x40000000UL, 3, sizeof(IIR_elliptic_resampling_ratio8_BiquadCellInt32) / sizeof(IIR_elliptic_resampling_ratio8_BiquadCellInt32[0]), IIR_elliptic_resampling_ratio8_BiquadCellInt32
};

#endif /* AC_SUPPORT_RATIO_8 */

#ifdef AC_SUPPORT_RATIO_12
// 96000 Hz low-pass IIR
//     pass frequency :   3500 Hz
//     stop frequency :   4000 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_butterworth_resampling_ratio12_BiquadCellFloat[] =
{
  {+0.00986202, +0.01972404, +0.00986202, -1.59742365, +0.63687174},
  {+0.01365645, +0.02731290, +0.01365645, -1.53354959, +0.58817539},
  {+0.01357894, +0.02715787, +0.01357894, -1.53778126, +0.59209700},
  {+0.01348642, +0.02697285, +0.01348642, -1.54405699, +0.59800268},
  {+0.01338845, +0.02677691, +0.01338845, -1.55220493, +0.60575874},
  {+0.01326179, +0.02652357, +0.01326179, -1.56260762, +0.61565476},
  {+0.01309654, +0.02619307, +0.01309654, -1.57524675, +0.62763290},
  {+0.01290628, +0.02581255, +0.01290628, -1.58965274, +0.64127785},
  {+0.01270768, +0.02541536, +0.01270768, -1.60540689, +0.65623761},
  {+0.01250289, +0.02500578, +0.01250289, -1.62245623, +0.67246778},
  {+0.01228563, +0.02457126, +0.01228563, -1.64065285, +0.68979537},
  {+0.00732157, +0.01464314, +0.00732157, -1.66241207, +0.69169836},
  {+0.00791177, +0.01582354, +0.00791177, -1.66213911, +0.69378620},
  {+0.00878772, +0.01757544, +0.00878772, -1.66375589, +0.69890677},
  {+0.00999967, +0.01999934, +0.00999967, -1.66700251, +0.70700119},
  {+0.01193777, +0.02387554, +0.01193777, -1.66092874, +0.70867982},
  {+0.01163989, +0.02327978, +0.01163989, -1.67128088, +0.71784045},
  {+0.01201207, +0.02402413, +0.01201207, -1.67933809, +0.72738635},
  {+0.01207804, +0.02415607, +0.01207804, -1.69246940, +0.74078155},
  {+0.01215990, +0.02431981, +0.01215990, -1.70557816, +0.75421778},
  {+0.01225563, +0.02451126, +0.01225563, -1.71922487, +0.76824739},
  {+0.01235943, +0.02471886, +0.01235943, -1.73354057, +0.78297830},
  {+0.01246701, +0.02493403, +0.01246701, -1.74857320, +0.79844126},
  {+0.01257857, +0.02515713, +0.01257857, -1.76427196, +0.81458622},
  {+0.01269525, +0.02539050, +0.01269525, -1.78062655, +0.83140756},
  {+0.01281652, +0.02563303, +0.01281652, -1.79763634, +0.84890240},
  {+0.01294236, +0.02588471, +0.01294236, -1.81528711, +0.86705653},
  {+0.01307269, +0.02614538, +0.01307269, -1.83356695, +0.88585770},
  {+0.01320739, +0.02641479, +0.01320739, -1.85246064, +0.90529021},
  {+0.01334635, +0.02669270, +0.01334635, -1.87195063, +0.92533603},
  {+0.01348941, +0.02697883, +0.01348941, -1.89201681, +0.94597447},
  {+0.01363642, +0.02727285, +0.01363642, -1.91263616, +0.96718185},
  {+0.01378719, +0.02757438, +0.01378719, -1.93378255, +0.98893131}
};

const biquadFloat_t IIR_butterworth_resampling_ratio12_biquadFloat =
{
  sizeof(IIR_butterworth_resampling_ratio12_BiquadCellFloat) / sizeof(IIR_butterworth_resampling_ratio12_BiquadCellFloat[0]), IIR_butterworth_resampling_ratio12_BiquadCellFloat
};

static const biquadCellInt32_t IIR_butterworth_resampling_ratio12_BiquadCellInt32[] =
{
  {(int32_t)0x014328A4UL, (int32_t)0x02865148UL, (int32_t)0x014328A4UL, (int32_t)0x99C3CF96UL, (int32_t)0x51850365UL},
  {(int32_t)0x01BF7E9CUL, (int32_t)0x037EFD37UL, (int32_t)0x01BF7E9CUL, (int32_t)0x9DDA52D0UL, (int32_t)0x4B4954CEUL},
  {(int32_t)0x01BCF45DUL, (int32_t)0x0379E8BAUL, (int32_t)0x01BCF45DUL, (int32_t)0x9D94FDE9UL, (int32_t)0x4BC9D5A2UL},
  {(int32_t)0x01B9EC52UL, (int32_t)0x0373D8A4UL, (int32_t)0x01B9EC52UL, (int32_t)0x9D2E2B9AUL, (int32_t)0x4C8B5A13UL},
  {(int32_t)0x01B6B67CUL, (int32_t)0x036D6CF8UL, (int32_t)0x01B6B67CUL, (int32_t)0x9CA8ACA6UL, (int32_t)0x4D8980A3UL},
  {(int32_t)0x01B28FEBUL, (int32_t)0x03651FD6UL, (int32_t)0x01B28FEBUL, (int32_t)0x9BFE3CA0UL, (int32_t)0x4ECDC66DUL},
  {(int32_t)0x01AD25B7UL, (int32_t)0x035A4B6DUL, (int32_t)0x01AD25B7UL, (int32_t)0x9B2F283CUL, (int32_t)0x50564661UL},
  {(int32_t)0x01A6E9B2UL, (int32_t)0x034DD363UL, (int32_t)0x01A6E9B2UL, (int32_t)0x9A432126UL, (int32_t)0x5215647BUL},
  {(int32_t)0x01A067BEUL, (int32_t)0x0340CF7BUL, (int32_t)0x01A067BEUL, (int32_t)0x99410372UL, (int32_t)0x53FF9814UL},
  {(int32_t)0x0199B1D3UL, (int32_t)0x033363A6UL, (int32_t)0x0199B1D3UL, (int32_t)0x9829AD5AUL, (int32_t)0x56136C97UL},
  {(int32_t)0x01929351UL, (int32_t)0x032526A2UL, (int32_t)0x01929351UL, (int32_t)0x96FF8B2BUL, (int32_t)0x584B36EEUL},
  {(int32_t)0x00EFE9CCUL, (int32_t)0x01DFD397UL, (int32_t)0x00EFE9CCUL, (int32_t)0x959B0A64UL, (int32_t)0x58899267UL},
  {(int32_t)0x010340C1UL, (int32_t)0x02068183UL, (int32_t)0x010340C1UL, (int32_t)0x959F8344UL, (int32_t)0x58CDFC7DUL},
  {(int32_t)0x011FF4BDUL, (int32_t)0x023FE97AUL, (int32_t)0x011FF4BDUL, (int32_t)0x95850603UL, (int32_t)0x5975C6EEUL},
  {(int32_t)0x0147AB50UL, (int32_t)0x028F56A0UL, (int32_t)0x0147AB50UL, (int32_t)0x954FD4B0UL, (int32_t)0x5A7F03E0UL},
  {(int32_t)0x01872D43UL, (int32_t)0x030E5A87UL, (int32_t)0x01872D43UL, (int32_t)0x95B357EDUL, (int32_t)0x5AB60533UL},
  {(int32_t)0x017D6A7BUL, (int32_t)0x02FAD4F6UL, (int32_t)0x017D6A7BUL, (int32_t)0x9509BBE9UL, (int32_t)0x5BE2321BUL},
  {(int32_t)0x01899C84UL, (int32_t)0x03133908UL, (int32_t)0x01899C84UL, (int32_t)0x9485B98CUL, (int32_t)0x5D1AFEF9UL},
  {(int32_t)0x018BC5ECUL, (int32_t)0x03178BD7UL, (int32_t)0x018BC5ECUL, (int32_t)0x93AE94D7UL, (int32_t)0x5ED1EE01UL},
  {(int32_t)0x018E74ACUL, (int32_t)0x031CE957UL, (int32_t)0x018E74ACUL, (int32_t)0x92D7CEAFUL, (int32_t)0x608A3551UL},
  {(int32_t)0x019197ADUL, (int32_t)0x03232F59UL, (int32_t)0x019197ADUL, (int32_t)0x91F83841UL, (int32_t)0x6255EE31UL},
  {(int32_t)0x0194FE6EUL, (int32_t)0x0329FCDCUL, (int32_t)0x0194FE6EUL, (int32_t)0x910DABD7UL, (int32_t)0x6438A20AUL},
  {(int32_t)0x019884E6UL, (int32_t)0x033109CCUL, (int32_t)0x019884E6UL, (int32_t)0x9017606AUL, (int32_t)0x663352C4UL},
  {(int32_t)0x019C2CA8UL, (int32_t)0x03385951UL, (int32_t)0x019C2CA8UL, (int32_t)0x8F162B14UL, (int32_t)0x68445C78UL},
  {(int32_t)0x019FFF7EUL, (int32_t)0x033FFEFBUL, (int32_t)0x019FFF7EUL, (int32_t)0x8E0A36F0UL, (int32_t)0x6A6B9016UL},
  {(int32_t)0x01A3F8BBUL, (int32_t)0x0347F176UL, (int32_t)0x01A3F8BBUL, (int32_t)0x8CF386B7UL, (int32_t)0x6CA8D57EUL},
  {(int32_t)0x01A81859UL, (int32_t)0x035030B2UL, (int32_t)0x01A81859UL, (int32_t)0x8BD25606UL, (int32_t)0x6EFBB557UL},
  {(int32_t)0x01AC5DABUL, (int32_t)0x0358BB56UL, (int32_t)0x01AC5DABUL, (int32_t)0x8AA6D6D3UL, (int32_t)0x7163C906UL},
  {(int32_t)0x01B0C7A5UL, (int32_t)0x03618F4AUL, (int32_t)0x01B0C7A5UL, (int32_t)0x897148ECUL, (int32_t)0x73E08CBBUL},
  {(int32_t)0x01B5554CUL, (int32_t)0x036AAA99UL, (int32_t)0x01B5554CUL, (int32_t)0x8831F5FAUL, (int32_t)0x7671693EUL},
  {(int32_t)0x01BA056AUL, (int32_t)0x03740AD4UL, (int32_t)0x01BA056AUL, (int32_t)0x86E9324FUL, (int32_t)0x7915B10AUL},
  {(int32_t)0x01BED69DUL, (int32_t)0x037DAD39UL, (int32_t)0x01BED69DUL, (int32_t)0x85975E82UL, (int32_t)0x7BCC9D6FUL},
  {(int32_t)0x01C3C755UL, (int32_t)0x03878EAAUL, (int32_t)0x01C3C755UL, (int32_t)0x843CE81BUL, (int32_t)0x7E954D1EUL}
};

const biquadInt32_t IIR_butterworth_resampling_ratio12_biquadInt32 =
{
  (int32_t)0x40000000UL, 1, sizeof(IIR_butterworth_resampling_ratio12_BiquadCellInt32) / sizeof(IIR_butterworth_resampling_ratio12_BiquadCellInt32[0]), IIR_butterworth_resampling_ratio12_BiquadCellInt32
};

// 96000 Hz low-pass IIR
//     pass frequency :   3500 Hz
//     stop frequency :   4000 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_chebyshev1_resampling_ratio12_BiquadCellFloat[] =
{
  {+0.00036196, +0.00072391, +0.00036196, -1.93477926, +0.93622708},
  {+0.00112285, +0.00224569, +0.00112285, -1.93366053, +0.93815191},
  {+0.00255394, +0.00510787, +0.00255394, -1.93172571, +0.94194145},
  {+0.00448519, +0.00897038, +0.00448519, -1.92953855, +0.94747930},
  {+0.00668817, +0.01337633, +0.00668817, -1.92784704, +0.95459970},
  {+0.00890212, +0.01780425, +0.00890212, -1.92748209, +0.96309599},
  {+0.01085148, +0.02170297, +0.01085148, -1.92924496, +0.97272902},
  {+0.01231446, +0.02462893, +0.01231446, -1.93379620, +0.98323431},
  {+0.01311468, +0.02622936, +0.01311468, -1.94155739, +0.99432708}
};

const biquadFloat_t IIR_chebyshev1_resampling_ratio12_biquadFloat =
{
  sizeof(IIR_chebyshev1_resampling_ratio12_BiquadCellFloat) / sizeof(IIR_chebyshev1_resampling_ratio12_BiquadCellFloat[0]), IIR_chebyshev1_resampling_ratio12_BiquadCellFloat
};

static const biquadCellInt32_t IIR_chebyshev1_resampling_ratio12_BiquadCellInt32[] =
{
  {(int32_t)0x000BDC4DUL, (int32_t)0x0017B89AUL, (int32_t)0x000BDC4DUL, (int32_t)0x842C939AUL, (int32_t)0x77D64A00UL},
  {(int32_t)0x0024CB1EUL, (int32_t)0x0049963DUL, (int32_t)0x0024CB1EUL, (int32_t)0x843EE7E9UL, (int32_t)0x78155CA7UL},
  {(int32_t)0x0053AFF6UL, (int32_t)0x00A75FEDUL, (int32_t)0x0053AFF6UL, (int32_t)0x845E9B22UL, (int32_t)0x78918995UL},
  {(int32_t)0x0092F87CUL, (int32_t)0x0125F0F7UL, (int32_t)0x0092F87CUL, (int32_t)0x848270BEUL, (int32_t)0x79470073UL},
  {(int32_t)0x00DB286AUL, (int32_t)0x01B650D3UL, (int32_t)0x00DB286AUL, (int32_t)0x849E2778UL, (int32_t)0x7A3052B7UL},
  {(int32_t)0x0123B46CUL, (int32_t)0x024768D7UL, (int32_t)0x0123B46CUL, (int32_t)0x84A4222BUL, (int32_t)0x7B46BAB1UL},
  {(int32_t)0x016394DBUL, (int32_t)0x02C729B5UL, (int32_t)0x016394DBUL, (int32_t)0x84874025UL, (int32_t)0x7C826269UL},
  {(int32_t)0x01938535UL, (int32_t)0x03270A69UL, (int32_t)0x01938535UL, (int32_t)0x843CAED8UL, (int32_t)0x7DDA9F39UL},
  {(int32_t)0x01ADBDECUL, (int32_t)0x035B7BD8UL, (int32_t)0x01ADBDECUL, (int32_t)0x83BD8616UL, (int32_t)0x7F461C1DUL}
};

const biquadInt32_t IIR_chebyshev1_resampling_ratio12_biquadInt32 =
{
  (int32_t)0x7FFFFFF8UL, 0, sizeof(IIR_chebyshev1_resampling_ratio12_BiquadCellInt32) / sizeof(IIR_chebyshev1_resampling_ratio12_BiquadCellInt32[0]), IIR_chebyshev1_resampling_ratio12_BiquadCellInt32
};

// 96000 Hz low-pass IIR
//     pass frequency :   3500 Hz
//     stop frequency :   4000 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_chebyshev2_resampling_ratio12_BiquadCellFloat[] =
{
  {+0.07595966, +0.05933470, +0.07595966, -1.09349171, +0.30474572},
  {+0.21952212, -0.25854825, +0.21952212, -1.24352616, +0.42402214},
  {+0.40077677, -0.65974552, +0.40077677, -1.43360079, +0.57540881},
  {+0.55032972, -0.99049012, +0.55032972, -1.59169105, +0.70186038},
  {+0.65694813, -1.22585635, +0.65694813, -1.70629570, +0.79433561},
  {+0.72979758, -1.38609030, +0.72979758, -1.78721094, +0.86071581},
  {+0.77927357, -1.49413201, +0.77927357, -1.84564946, +0.91006460},
  {+0.81284927, -1.56639960, +0.81284927, -1.88999761, +0.94929654},
  {+0.83526236, -1.61317476, +0.83526236, -1.92613712, +0.98348709}
};

const biquadFloat_t IIR_chebyshev2_resampling_ratio12_biquadFloat =
{
  sizeof(IIR_chebyshev2_resampling_ratio12_BiquadCellFloat) / sizeof(IIR_chebyshev2_resampling_ratio12_BiquadCellFloat[0]), IIR_chebyshev2_resampling_ratio12_BiquadCellFloat
};

static const biquadCellInt32_t IIR_chebyshev2_resampling_ratio12_BiquadCellInt32[] =
{
  {(int32_t)0x09B90BC5UL, (int32_t)0x07984793UL, (int32_t)0x09B90BC5UL, (int32_t)0xBA043B59UL, (int32_t)0x2701E86CUL},
  {(int32_t)0x1C194CFEUL, (int32_t)0xDEE7E40EUL, (int32_t)0x1C194CFEUL, (int32_t)0xB06A1143UL, (int32_t)0x36465B84UL},
  {(int32_t)0x334CA73DUL, (int32_t)0xAB8D756AUL, (int32_t)0x334CA73DUL, (int32_t)0xA43FE278UL, (int32_t)0x49A6FEF5UL},
  {(int32_t)0x46713453UL, (int32_t)0x81379EA8UL, (int32_t)0x46713453UL, (int32_t)0x9A21BBD8UL, (int32_t)0x59D68F9EUL},
  {(int32_t)0x2A0B7030UL, (int32_t)0xB18B91CBUL, (int32_t)0x2A0B7030UL, (int32_t)0x92CC0D20UL, (int32_t)0x65ACCA17UL},
  {(int32_t)0x2EB500EBUL, (int32_t)0xA74A4BE5UL, (int32_t)0x2EB500EBUL, (int32_t)0x8D9E55FCUL, (int32_t)0x6E2BEF7FUL},
  {(int32_t)0x31DF9E44UL, (int32_t)0xA0602424UL, (int32_t)0x31DF9E44UL, (int32_t)0x89E0E115UL, (int32_t)0x747CFF2DUL},
  {(int32_t)0x3405B8F1UL, (int32_t)0x9BC01BE0UL, (int32_t)0x3405B8F1UL, (int32_t)0x870A477AUL, (int32_t)0x79828C8FUL},
  {(int32_t)0x3574F047UL, (int32_t)0x98C1BEA2UL, (int32_t)0x3574F047UL, (int32_t)0x84BA2B5BUL, (int32_t)0x7DE2E7AAUL}
};

const biquadInt32_t IIR_chebyshev2_resampling_ratio12_biquadInt32 =
{
  (int32_t)0x40000000UL, 6, sizeof(IIR_chebyshev2_resampling_ratio12_BiquadCellInt32) / sizeof(IIR_chebyshev2_resampling_ratio12_BiquadCellInt32[0]), IIR_chebyshev2_resampling_ratio12_BiquadCellInt32
};

// 96000 Hz low-pass IIR
//     pass frequency :   3500 Hz
//     stop frequency :   4000 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_elliptic_resampling_ratio12_BiquadCellFloat[] =
{
  {+0.04842362, +0.04842362, +0.00000000, -0.90315276, +0.00000000},
  {+0.07329830, -0.12680175, +0.07329830, -1.83578345, +0.85557831},
  {+0.36475754, -0.69218981, +0.36475754, -1.88586294, +0.92318821},
  {+0.66983759, -1.29116501, +0.66983759, -1.91951452, +0.96802468},
  {+0.82292183, -1.59267182, +0.82292183, -1.93844901, +0.99162086}
};

const biquadFloat_t IIR_elliptic_resampling_ratio12_biquadFloat =
{
  sizeof(IIR_elliptic_resampling_ratio12_BiquadCellFloat) / sizeof(IIR_elliptic_resampling_ratio12_BiquadCellFloat[0]), IIR_elliptic_resampling_ratio12_BiquadCellFloat
};

static const biquadCellInt32_t IIR_elliptic_resampling_ratio12_BiquadCellInt32[] =
{
  {(int32_t)0x0632BEBFUL, (int32_t)0x0632BEBFUL, (int32_t)0x00000000UL, (int32_t)0xC632BEBFUL, (int32_t)0x00000000UL},
  {(int32_t)0x0961D6B5UL, (int32_t)0xEFC4F5D8UL, (int32_t)0x0961D6B5UL, (int32_t)0x8A82861DUL, (int32_t)0x6D839709UL},
  {(int32_t)0x2EB06002UL, (int32_t)0xA766530DUL, (int32_t)0x2EB06002UL, (int32_t)0x874E0584UL, (int32_t)0x762B0809UL},
  {(int32_t)0x2ADE9E79UL, (int32_t)0xAD5D8D6AUL, (int32_t)0x2ADE9E79UL, (int32_t)0x8526AC90UL, (int32_t)0x7BE83B9AUL},
  {(int32_t)0x34AAC057UL, (int32_t)0x9A11AA37UL, (int32_t)0x34AAC057UL, (int32_t)0x83F07392UL, (int32_t)0x7EED6EA7UL}
};

const biquadInt32_t IIR_elliptic_resampling_ratio12_biquadInt32 =
{
  (int32_t)0x40000000UL, 3, sizeof(IIR_elliptic_resampling_ratio12_BiquadCellInt32) / sizeof(IIR_elliptic_resampling_ratio12_BiquadCellInt32[0]), IIR_elliptic_resampling_ratio12_BiquadCellInt32
};

#endif /* AC_SUPPORT_RATIO_12 */

