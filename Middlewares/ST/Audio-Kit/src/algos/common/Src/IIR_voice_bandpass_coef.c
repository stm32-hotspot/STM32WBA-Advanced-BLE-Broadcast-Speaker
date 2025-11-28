#include <stdlib.h> // for NULL definition
#include "common/IIR_voice_bandpass_coef.h"

#ifdef AC_SUPPORT_FS_8000
// 8000 Hz band-pass IIR
//     first  stop frequency :    200 Hz
//     first  pass frequency :    300 Hz
//     second pass frequency :   1000 Hz
//     second stop frequency :   1200 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_butterworth_voice_bandpass_fs8000_BiquadCellFloat[] =
{
  {+0.06150824, +0.12301648, +0.06150824, -1.22114360, +0.47060728},
  {+0.06936530, +0.13873059, +0.06936530, -1.18847454, +0.47604008},
  {+0.05358325, +0.10716650, +0.05358325, -1.26583269, +0.47757888},
  {+0.07744142, +0.15488283, +0.07744142, -1.16671829, +0.49232376},
  {+0.04549767, +0.09099534, +0.04549767, -1.32281783, +0.49840343},
  {+0.08602097, +0.17204194, +0.08602097, -1.15490955, +0.51835408},
  {+0.03759550, +0.07519099, +0.03759550, -1.38926312, +0.53250394},
  {+0.09516976, +0.19033952, +0.09516976, -1.15243435, +0.55347578},
  {+0.03063305, +0.06126610, +0.03063305, -1.45805259, +0.57519061},
  {+0.10457400, +0.20914800, +0.10457400, -1.15901558, +0.59737877},
  {+0.02514538, +0.05029076, +0.02514538, -1.52210682, +0.61995029},
  {+0.11377662, +0.22755324, +0.11377662, -1.17466011, +0.65000399},
  {+0.02103226, +0.04206452, +0.02103226, -1.57870037, +0.66276460},
  {+0.17175665, +0.00000000, -0.17175665, -1.62829585, +0.70247266},
  {+6.21971250, -12.43942499, +6.21971250, -1.19961151, +0.71147077},
  {+0.84115207, -1.68230415, +0.84115207, -1.67226712, +0.73922792},
  {+0.75178673, -1.50357347, +0.75178673, -1.71193118, +0.77354933},
  {+3.56147179, -7.12294359, +3.56147179, -1.23431223, +0.78201656},
  {+0.71033403, -1.42066807, +0.71033403, -1.74834519, +0.80598524},
  {+0.67559277, -1.35118554, +0.67559277, -1.78232805, +0.83702724},
  {+2.70434158, -5.40868316, +2.70434158, -1.27936961, +0.86193787},
  {+0.72438396, -1.44876792, +0.72438396, -1.81452040, +0.86710283},
  {+0.69329470, -1.38658939, +0.69329470, -1.84543796, +0.89658885},
  {+0.64093398, -1.28186797, +0.64093398, -1.87551214, +0.92582879},
  {+1.71728767, -3.43457534, +1.71728767, -1.33551724, +0.95152080},
  {+0.89223228, -1.78446455, +0.89223228, -1.90512035, +0.95514957},
  {+0.90308991, -1.80617981, +0.90308991, -1.93460936, +0.98487753}
};

const biquadFloat_t IIR_butterworth_voice_bandpass_fs8000_biquadFloat =
{
  sizeof(IIR_butterworth_voice_bandpass_fs8000_BiquadCellFloat) / sizeof(IIR_butterworth_voice_bandpass_fs8000_BiquadCellFloat[0]), IIR_butterworth_voice_bandpass_fs8000_BiquadCellFloat
};

static const biquadCellInt32_t IIR_butterworth_voice_bandpass_fs8000_BiquadCellInt32[] =
{
  {(int32_t)0x07DF8087UL, (int32_t)0x0FBF010EUL, (int32_t)0x07DF8087UL, (int32_t)0xB1D8C886UL, (int32_t)0x3C3CDC05UL},
  {(int32_t)0x08E0F648UL, (int32_t)0x11C1EC90UL, (int32_t)0x08E0F648UL, (int32_t)0xB3F00878UL, (int32_t)0x3CEEE19AUL},
  {(int32_t)0x06DBD0E5UL, (int32_t)0x0DB7A1CAUL, (int32_t)0x06DBD0E5UL, (int32_t)0xAEFC98E2UL, (int32_t)0x3D214DFDUL},
  {(int32_t)0x09E999AEUL, (int32_t)0x13D3335CUL, (int32_t)0x09E999AEUL, (int32_t)0xB5547CD1UL, (int32_t)0x3F047706UL},
  {(int32_t)0x05D2DE23UL, (int32_t)0x0BA5BC45UL, (int32_t)0x05D2DE23UL, (int32_t)0xAB56F3E2UL, (int32_t)0x3FCBAEF9UL},
  {(int32_t)0x0B02BC30UL, (int32_t)0x16057860UL, (int32_t)0x0B02BC30UL, (int32_t)0xB615F643UL, (int32_t)0x42596D2EUL},
  {(int32_t)0x04CFEDE0UL, (int32_t)0x099FDBBFUL, (int32_t)0x04CFEDE0UL, (int32_t)0xA7165021UL, (int32_t)0x442916CBUL},
  {(int32_t)0x0C2E85CBUL, (int32_t)0x185D0B97UL, (int32_t)0x0C2E85CBUL, (int32_t)0xB63E83FAUL, (int32_t)0x46D84B5BUL},
  {(int32_t)0x03EBC8A9UL, (int32_t)0x07D79151UL, (int32_t)0x03EBC8A9UL, (int32_t)0xA2AF4430UL, (int32_t)0x499FD894UL},
  {(int32_t)0x0D62AE49UL, (int32_t)0x1AC55C93UL, (int32_t)0x0D62AE49UL, (int32_t)0xB5D2B053UL, (int32_t)0x4C76E84CUL},
  {(int32_t)0x0337F6C1UL, (int32_t)0x066FED83UL, (int32_t)0x0337F6C1UL, (int32_t)0x9E95CD42UL, (int32_t)0x4F5A87FAUL},
  {(int32_t)0x0E903B79UL, (int32_t)0x1D2076F1UL, (int32_t)0x0E903B79UL, (int32_t)0xB4D25E68UL, (int32_t)0x533354AEUL},
  {(int32_t)0x02B12F62UL, (int32_t)0x05625EC4UL, (int32_t)0x02B12F62UL, (int32_t)0x9AF692BEUL, (int32_t)0x54D5786BUL},
  {(int32_t)0x15FC1F39UL, (int32_t)0x00000000UL, (int32_t)0xEA03E0C7UL, (int32_t)0x97CA0031UL, (int32_t)0x59EA9FC0UL},
  {(int32_t)0x31C1F8A0UL, (int32_t)0x9C7C0EC0UL, (int32_t)0x31C1F8A0UL, (int32_t)0xB33990A4UL, (int32_t)0x5B11795DUL},
  {(int32_t)0x35D56F81UL, (int32_t)0x945520FDUL, (int32_t)0x35D56F81UL, (int32_t)0x94F99356UL, (int32_t)0x5E9F0542UL},
  {(int32_t)0x301D461BUL, (int32_t)0x9FC573CAUL, (int32_t)0x301D461BUL, (int32_t)0x926FB838UL, (int32_t)0x6303AA21UL},
  {(int32_t)0x38FBC9D9UL, (int32_t)0x8E086C4EUL, (int32_t)0x38FBC9D9UL, (int32_t)0xB101074CUL, (int32_t)0x64191E5CUL},
  {(int32_t)0x2D761CE2UL, (int32_t)0xA513C63CUL, (int32_t)0x2D761CE2UL, (int32_t)0x901B1CC9UL, (int32_t)0x672A8631UL},
  {(int32_t)0x2B3CE976UL, (int32_t)0xA9862D14UL, (int32_t)0x2B3CE976UL, (int32_t)0x8DEE5651UL, (int32_t)0x6B23B56CUL},
  {(int32_t)0x2B44FBAEUL, (int32_t)0xA97608A5UL, (int32_t)0x2B44FBAEUL, (int32_t)0xAE1ECEEFUL, (int32_t)0x6E53FAE4UL},
  {(int32_t)0x2E5C4E8AUL, (int32_t)0xA34762ECUL, (int32_t)0x2E5C4E8AUL, (int32_t)0x8BDEE5CFUL, (int32_t)0x6EFD39C3UL},
  {(int32_t)0x2C5EF0B9UL, (int32_t)0xA7421E8EUL, (int32_t)0x2C5EF0B9UL, (int32_t)0x89E4582DUL, (int32_t)0x72C36C6DUL},
  {(int32_t)0x29050FF9UL, (int32_t)0xADF5E00DUL, (int32_t)0x29050FF9UL, (int32_t)0x87F79BEBUL, (int32_t)0x76818ED1UL},
  {(int32_t)0x36F40546UL, (int32_t)0x9217F574UL, (int32_t)0x36F40546UL, (int32_t)0xAA86E2B8UL, (int32_t)0x79CB6EF6UL},
  {(int32_t)0x391A5569UL, (int32_t)0x8DCB552EUL, (int32_t)0x391A5569UL, (int32_t)0x86128216UL, (int32_t)0x7A42574FUL},
  {(int32_t)0x39CC399BUL, (int32_t)0x8C678CCBUL, (int32_t)0x39CC399BUL, (int32_t)0x842F5C3CUL, (int32_t)0x7E10778BUL}
};

const biquadInt32_t IIR_butterworth_voice_bandpass_fs8000_biquadInt32 =
{
  (int32_t)0x40000000UL, 22, sizeof(IIR_butterworth_voice_bandpass_fs8000_BiquadCellInt32) / sizeof(IIR_butterworth_voice_bandpass_fs8000_BiquadCellInt32[0]), IIR_butterworth_voice_bandpass_fs8000_BiquadCellInt32
};

// 8000 Hz band-pass IIR
//     first  stop frequency :    200 Hz
//     first  pass frequency :    300 Hz
//     second pass frequency :   1000 Hz
//     second stop frequency :   1200 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_chebyshev1_voice_bandpass_fs8000_BiquadCellFloat[] =
{
  {+0.01847295, +0.03694589, +0.01847295, -1.61004080, +0.86106725},
  {+0.02984825, +0.05969650, +0.02984825, -1.52035762, +0.86539308},
  {+0.02586346, +0.05172692, +0.02586346, -1.69965662, +0.87490074},
  {+0.05732024, +0.11464047, +0.05732024, -1.44424894, +0.88833037},
  {+0.02633539, +0.05267077, +0.02633539, -1.77795648, +0.90019866},
  {+0.38677697, +0.00000000, -0.38677697, -1.39556517, +0.92610030},
  {+0.55631705, -1.11263409, +0.55631705, -1.83910558, +0.92795115},
  {+0.56744718, -1.13489437, +0.56744718, -1.88351369, +0.95270696},
  {+0.50215301, -1.00430602, +0.50215301, -1.91493363, +0.97344670},
  {+2.46952171, -4.93904342, +2.46952171, -1.38619263, +0.97385771},
  {+0.90273668, -1.80547336, +0.90273668, -1.93743995, +0.99143154}
};

const biquadFloat_t IIR_chebyshev1_voice_bandpass_fs8000_biquadFloat =
{
  sizeof(IIR_chebyshev1_voice_bandpass_fs8000_BiquadCellFloat) / sizeof(IIR_chebyshev1_voice_bandpass_fs8000_BiquadCellFloat[0]), IIR_chebyshev1_voice_bandpass_fs8000_BiquadCellFloat
};

static const biquadCellInt32_t IIR_chebyshev1_voice_bandpass_fs8000_BiquadCellInt32[] =
{
  {(int32_t)0x025D524CUL, (int32_t)0x04BAA498UL, (int32_t)0x025D524CUL, (int32_t)0x98F5176BUL, (int32_t)0x6E3773A7UL},
  {(int32_t)0x03D21144UL, (int32_t)0x07A42289UL, (int32_t)0x03D21144UL, (int32_t)0x9EB275F6UL, (int32_t)0x6EC53347UL},
  {(int32_t)0x034F7E70UL, (int32_t)0x069EFCDFUL, (int32_t)0x034F7E70UL, (int32_t)0x9338D376UL, (int32_t)0x6FFCBF5AUL},
  {(int32_t)0x075644FCUL, (int32_t)0x0EAC89F9UL, (int32_t)0x075644FCUL, (int32_t)0xA3916CE5UL, (int32_t)0x71B4CF48UL},
  {(int32_t)0x035EF53CUL, (int32_t)0x06BDEA79UL, (int32_t)0x035EF53CUL, (int32_t)0x8E35F607UL, (int32_t)0x7339B5ACUL},
  {(int32_t)0x3181E86DUL, (int32_t)0x00000000UL, (int32_t)0xCE7E1793UL, (int32_t)0xA6AF0F71UL, (int32_t)0x768A7463UL},
  {(int32_t)0x239AB2D1UL, (int32_t)0xB8CA9A5EUL, (int32_t)0x239AB2D1UL, (int32_t)0x8A4C181EUL, (int32_t)0x76C71A78UL},
  {(int32_t)0x24510DFDUL, (int32_t)0xB75DE406UL, (int32_t)0x24510DFDUL, (int32_t)0x877482FDUL, (int32_t)0x79F24D3CUL},
  {(int32_t)0x2023465FUL, (int32_t)0xBFB97342UL, (int32_t)0x2023465FUL, (int32_t)0x8571BA33UL, (int32_t)0x7C99E6CCUL},
  {(int32_t)0x27832932UL, (int32_t)0xB0F9AD9BUL, (int32_t)0x27832932UL, (int32_t)0xA7489EB8UL, (int32_t)0x7CA75E94UL},
  {(int32_t)0x39C6702AUL, (int32_t)0x8C731FACUL, (int32_t)0x39C6702AUL, (int32_t)0x8400FBDEUL, (int32_t)0x7EE73A8DUL}
};

const biquadInt32_t IIR_chebyshev1_voice_bandpass_fs8000_biquadInt32 =
{
  (int32_t)0x7FFFFFC4UL, 7, sizeof(IIR_chebyshev1_voice_bandpass_fs8000_BiquadCellInt32) / sizeof(IIR_chebyshev1_voice_bandpass_fs8000_BiquadCellInt32[0]), IIR_chebyshev1_voice_bandpass_fs8000_BiquadCellInt32
};

// 8000 Hz band-pass IIR
//     first  stop frequency :    200 Hz
//     first  pass frequency :    300 Hz
//     second pass frequency :   1000 Hz
//     second stop frequency :   1200 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_chebyshev2_voice_bandpass_fs8000_BiquadCellFloat[] =
{
  {+0.35661331, -0.00000000, -0.35661331, -1.18755166, +0.28677339},
  {+0.18342500, +0.15219044, +0.18342500, -0.71284914, +0.19975199},
  {+0.32559346, -0.10912042, +0.32559346, -0.84716819, +0.38913057},
  {+0.48544168, -0.41494195, +0.48544168, -1.01804996, +0.58877388},
  {+0.61173248, -0.66002692, +0.61173248, -1.17345161, +0.76370216},
  {+0.94583809, -1.88916177, +0.94583809, -1.74616732, +0.76836716},
  {+0.91494308, -1.82165024, +0.91494308, -1.81100174, +0.83519536},
  {+0.91438076, -1.81415895, +0.91438076, -1.86390915, +0.89149219},
  {+0.63483510, -0.74002430, +0.63483510, -1.30192570, +0.92059529},
  {+0.95270326, -1.88503108, +0.95270326, -1.90748862, +0.93826173},
  {+0.97160460, -1.91963960, +0.97160460, -1.94690012, +0.97979010}
};

const biquadFloat_t IIR_chebyshev2_voice_bandpass_fs8000_biquadFloat =
{
  sizeof(IIR_chebyshev2_voice_bandpass_fs8000_BiquadCellFloat) / sizeof(IIR_chebyshev2_voice_bandpass_fs8000_BiquadCellFloat[0]), IIR_chebyshev2_voice_bandpass_fs8000_BiquadCellFloat
};

static const biquadCellInt32_t IIR_chebyshev2_voice_bandpass_fs8000_BiquadCellInt32[] =
{
  {(int32_t)0x2DA58141UL, (int32_t)0x00000000UL, (int32_t)0xD25A7EBFUL, (int32_t)0xB3FF274FUL, (int32_t)0x24B4FD8FUL},
  {(int32_t)0x177A7867UL, (int32_t)0x137AF9F7UL, (int32_t)0x177A7867UL, (int32_t)0xD260ADFCUL, (int32_t)0x1991792DUL},
  {(int32_t)0x29AD0BE8UL, (int32_t)0xF2085799UL, (int32_t)0x29AD0BE8UL, (int32_t)0xC9C7FF17UL, (int32_t)0x31CF07CCUL},
  {(int32_t)0x3E22F3EEUL, (int32_t)0xCAE32EABUL, (int32_t)0x3E22F3EEUL, (int32_t)0xBED844FAUL, (int32_t)0x4B5CF140UL},
  {(int32_t)0x4E4D3FFFUL, (int32_t)0xAB843CE9UL, (int32_t)0x4E4D3FFFUL, (int32_t)0xB4E62B3CUL, (int32_t)0x61C0FE0EUL},
  {(int32_t)0x3C889C7FUL, (int32_t)0x8717F93CUL, (int32_t)0x3C889C7FUL, (int32_t)0x903ECB6FUL, (int32_t)0x6259DAF0UL},
  {(int32_t)0x3A8E6D70UL, (int32_t)0x8B6A1517UL, (int32_t)0x3A8E6D70UL, (int32_t)0x8C188C26UL, (int32_t)0x6AE7AE83UL},
  {(int32_t)0x3A8536DEUL, (int32_t)0x8BE4D1DEUL, (int32_t)0x3A8536DEUL, (int32_t)0x88B5B663UL, (int32_t)0x721C6A7CUL},
  {(int32_t)0x514246CCUL, (int32_t)0xA146E23FUL, (int32_t)0x514246CCUL, (int32_t)0xACAD3FD5UL, (int32_t)0x75D6110BUL},
  {(int32_t)0x3CF9171BUL, (int32_t)0x875BA695UL, (int32_t)0x3CF9171BUL, (int32_t)0x85EBB4DFUL, (int32_t)0x7818F5D7UL},
  {(int32_t)0x3E2EC511UL, (int32_t)0x85249FEEUL, (int32_t)0x3E2EC511UL, (int32_t)0x8365FD0FUL, (int32_t)0x7D69C314UL}
};

const biquadInt32_t IIR_chebyshev2_voice_bandpass_fs8000_biquadInt32 =
{
  (int32_t)0x40000000UL, 6, sizeof(IIR_chebyshev2_voice_bandpass_fs8000_BiquadCellInt32) / sizeof(IIR_chebyshev2_voice_bandpass_fs8000_BiquadCellInt32[0]), IIR_chebyshev2_voice_bandpass_fs8000_BiquadCellInt32
};

// 8000 Hz band-pass IIR
//     first  stop frequency :    200 Hz
//     first  pass frequency :    300 Hz
//     second pass frequency :   1000 Hz
//     second stop frequency :   1200 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_elliptic_voice_bandpass_fs8000_BiquadCellFloat[] =
{
  {+0.11913641, -0.00000000, -0.11913641, -1.59706123, +0.76172720},
  {+0.13629742, -0.05446600, +0.13629742, -1.42220937, +0.76318078},
  {+0.39072620, -0.41990363, +0.39072620, -1.36538074, +0.86886419},
  {+0.73181796, -1.45312457, +0.73181796, -1.79275161, +0.87510848},
  {+0.82596531, -1.62575395, +0.82596531, -1.89160802, +0.95093453},
  {+0.63572938, -0.77454113, +0.63572938, -1.37115106, +0.96147679},
  {+0.96734949, -1.89678229, +0.96734949, -1.93424120, +0.98745341}
};

const biquadFloat_t IIR_elliptic_voice_bandpass_fs8000_biquadFloat =
{
  sizeof(IIR_elliptic_voice_bandpass_fs8000_BiquadCellFloat) / sizeof(IIR_elliptic_voice_bandpass_fs8000_BiquadCellFloat[0]), IIR_elliptic_voice_bandpass_fs8000_BiquadCellFloat
};

static const biquadCellInt32_t IIR_elliptic_voice_bandpass_fs8000_BiquadCellInt32[] =
{
  {(int32_t)0x0F3FDC9DUL, (int32_t)0x00000000UL, (int32_t)0xF0C02363UL, (int32_t)0x99C9BFB1UL, (int32_t)0x618046DCUL},
  {(int32_t)0x11723199UL, (int32_t)0xF9074215UL, (int32_t)0x11723199UL, (int32_t)0xA4FA8588UL, (int32_t)0x61AFE869UL},
  {(int32_t)0x320350F4UL, (int32_t)0xCA409903UL, (int32_t)0x320350F4UL, (int32_t)0xA89D9A1EUL, (int32_t)0x6F36F117UL},
  {(int32_t)0x2ED61B01UL, (int32_t)0xA30001CDUL, (int32_t)0x2ED61B01UL, (int32_t)0x8D438EBCUL, (int32_t)0x70038E05UL},
  {(int32_t)0x34DC9D99UL, (int32_t)0x97F3A5B7UL, (int32_t)0x34DC9D99UL, (int32_t)0x86EFE4EAUL, (int32_t)0x79B838FCUL},
  {(int32_t)0x515F9490UL, (int32_t)0x9CDBD613UL, (int32_t)0x515F9490UL, (int32_t)0xA83F0F9DUL, (int32_t)0x7B11ABEAUL},
  {(int32_t)0x3DE90DD6UL, (int32_t)0x869B1E68UL, (int32_t)0x3DE90DD6UL, (int32_t)0x84356463UL, (int32_t)0x7E64DF94UL}
};

const biquadInt32_t IIR_elliptic_voice_bandpass_fs8000_biquadInt32 =
{
  (int32_t)0x7FFFFFF5UL, 3, sizeof(IIR_elliptic_voice_bandpass_fs8000_BiquadCellInt32) / sizeof(IIR_elliptic_voice_bandpass_fs8000_BiquadCellInt32[0]), IIR_elliptic_voice_bandpass_fs8000_BiquadCellInt32
};

#endif /* AC_SUPPORT_FS_8000 */

#ifdef AC_SUPPORT_FS_16000
// 16000 Hz band-pass IIR
//     first  stop frequency :    200 Hz
//     first  pass frequency :    300 Hz
//     second pass frequency :   1000 Hz
//     second stop frequency :   1200 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_butterworth_voice_bandpass_fs16000_BiquadCellFloat[] =
{
  {+0.01889484, +0.03778969, +0.01889484, -1.61703811, +0.69615197},
  {+0.01692978, +0.03385957, +0.01692978, -1.62830224, +0.69704622},
  {+0.02084711, +0.04169423, +0.02084711, -1.61231241, +0.70174411},
  {+0.01491143, +0.02982285, +0.01491143, -1.64637145, +0.70492315},
  {+0.02284564, +0.04569128, +0.02284564, -1.61372451, +0.71328174},
  {+0.01284450, +0.02568899, +0.01284450, -1.67092678, +0.71984404},
  {+0.02494450, +0.04988901, +0.02494450, -1.62090867, +0.73030042},
  {+0.01080773, +0.02161546, +0.01080773, -1.70036718, +0.74073292},
  {+0.02715767, +0.05431535, +0.02715767, -1.63357056, +0.75242240},
  {+0.00894234, +0.01788469, +0.00894234, -1.73172437, +0.76507765},
  {+0.02942142, +0.05884283, +0.02942142, -1.65147846, +0.77933415},
  {+0.00737076, +0.01474153, +0.00737076, -1.76203798, +0.79001386},
  {+0.03160945, +0.06321891, +0.03160945, -1.67444183, +0.81075784},
  {+0.00612334, +0.01224668, +0.00612334, -1.78973038, +0.81371605},
  {+0.08953868, +0.00000000, -0.08953868, -1.81452051, +0.83555475},
  {+6.15960993, -12.31921986, +6.15960993, -1.70228728, +0.84642331},
  {+0.90385828, -1.80771656, +0.90385828, -1.83672573, +0.85555329},
  {+0.81504151, -1.63008302, +0.81504151, -1.85680757, +0.87396336},
  {+3.55382039, -7.10764077, +3.55382039, -1.73483407, +0.88604138},
  {+0.77005120, -1.54010241, +0.77005120, -1.87519934, +0.89107604},
  {+0.73455074, -1.46910148, +0.73455074, -1.89226460, +0.90715921},
  {+0.69682183, -1.39364367, +0.69682183, -1.90829966, +0.92244428},
  {+2.83660173, -5.67320345, +2.83660173, -1.77186903, +0.92927751},
  {+0.74979135, -1.49958269, +0.74979135, -1.92354669, +0.93712905},
  {+0.71261374, -1.42522748, +0.71261374, -1.93820725, +0.95138476},
  {+0.65413316, -1.30826632, +0.65413316, -1.95245378, +0.96536348},
  {+1.79587847, -3.59175695, +1.79587847, -1.81312013, +0.97572522},
  {+0.90572995, -1.81145989, +0.90572995, -1.96643874, +0.97920481},
  {+0.90936207, -1.81872415, +0.90936207, -1.98030214, +0.99304186}
};

const biquadFloat_t IIR_butterworth_voice_bandpass_fs16000_biquadFloat =
{
  sizeof(IIR_butterworth_voice_bandpass_fs16000_BiquadCellFloat) / sizeof(IIR_butterworth_voice_bandpass_fs16000_BiquadCellFloat[0]), IIR_butterworth_voice_bandpass_fs16000_BiquadCellFloat
};

static const biquadCellInt32_t IIR_butterworth_voice_bandpass_fs16000_BiquadCellInt32[] =
{
  {(int32_t)0x026B2572UL, (int32_t)0x04D64AE3UL, (int32_t)0x026B2572UL, (int32_t)0x98827294UL, (int32_t)0x591B81F9UL},
  {(int32_t)0x022AC14FUL, (int32_t)0x0455829FUL, (int32_t)0x022AC14FUL, (int32_t)0x97C9E563UL, (int32_t)0x5938CF7EUL},
  {(int32_t)0x02AB1E46UL, (int32_t)0x05563C8CUL, (int32_t)0x02AB1E46UL, (int32_t)0x98CFDFA0UL, (int32_t)0x59D2C04BUL},
  {(int32_t)0x01E89E1DUL, (int32_t)0x03D13C3AUL, (int32_t)0x01E89E1DUL, (int32_t)0x96A1D9A2UL, (int32_t)0x5A3AEBF0UL},
  {(int32_t)0x02EC9B1DUL, (int32_t)0x05D9363BUL, (int32_t)0x02EC9B1DUL, (int32_t)0x98B8BCD3UL, (int32_t)0x5B4CD0DFUL},
  {(int32_t)0x01A4E372UL, (int32_t)0x0349C6E5UL, (int32_t)0x01A4E372UL, (int32_t)0x950F8924UL, (int32_t)0x5C23D984UL},
  {(int32_t)0x033161A8UL, (int32_t)0x0662C350UL, (int32_t)0x033161A8UL, (int32_t)0x98430848UL, (int32_t)0x5D7A7BEDUL},
  {(int32_t)0x016225D5UL, (int32_t)0x02C44BA9UL, (int32_t)0x016225D5UL, (int32_t)0x932D2F27UL, (int32_t)0x5ED05614UL},
  {(int32_t)0x0379E716UL, (int32_t)0x06F3CE2DUL, (int32_t)0x0379E716UL, (int32_t)0x97739478UL, (int32_t)0x604F608EUL},
  {(int32_t)0x012505D0UL, (int32_t)0x024A0B9FUL, (int32_t)0x012505D0UL, (int32_t)0x912B6D8DUL, (int32_t)0x61EE1080UL},
  {(int32_t)0x03C414B9UL, (int32_t)0x07882972UL, (int32_t)0x03C414B9UL, (int32_t)0x964E2D49UL, (int32_t)0x63C138B9UL},
  {(int32_t)0x00F18671UL, (int32_t)0x01E30CE2UL, (int32_t)0x00F18671UL, (int32_t)0x8F3AC50AUL, (int32_t)0x651F2C99UL},
  {(int32_t)0x040BC754UL, (int32_t)0x08178EA7UL, (int32_t)0x040BC754UL, (int32_t)0x94D5F1EBUL, (int32_t)0x67C6E9B7UL},
  {(int32_t)0x00C8A647UL, (int32_t)0x01914C8FUL, (int32_t)0x00C8A647UL, (int32_t)0x8D750EB5UL, (int32_t)0x6827D8F9UL},
  {(int32_t)0x0B7600DEUL, (int32_t)0x00000000UL, (int32_t)0xF489FF22UL, (int32_t)0x8BDEE560UL, (int32_t)0x6AF37545UL},
  {(int32_t)0x3146E192UL, (int32_t)0x9D723CDBUL, (int32_t)0x3146E192UL, (int32_t)0x930DB9A6UL, (int32_t)0x6C579962UL},
  {(int32_t)0x39D8D064UL, (int32_t)0x8C4E5F38UL, (int32_t)0x39D8D064UL, (int32_t)0x8A7315F2UL, (int32_t)0x6D82C52FUL},
  {(int32_t)0x3429A3E0UL, (int32_t)0x97ACB841UL, (int32_t)0x3429A3E0UL, (int32_t)0x892A1092UL, (int32_t)0x6FDE080EUL},
  {(int32_t)0x38DC72C4UL, (int32_t)0x8E471A78UL, (int32_t)0x38DC72C4UL, (int32_t)0x90F87A85UL, (int32_t)0x7169CDCAUL},
  {(int32_t)0x314884D8UL, (int32_t)0x9D6EF64FUL, (int32_t)0x314884D8UL, (int32_t)0x87FCBBECUL, (int32_t)0x720EC797UL},
  {(int32_t)0x2F02E11DUL, (int32_t)0xA1FA3DC5UL, (int32_t)0x2F02E11DUL, (int32_t)0x86E52305UL, (int32_t)0x741DCB04UL},
  {(int32_t)0x2C98BA9AUL, (int32_t)0xA6CE8ACCUL, (int32_t)0x2C98BA9AUL, (int32_t)0x85DE6B18UL, (int32_t)0x7612A780UL},
  {(int32_t)0x2D62B87EUL, (int32_t)0xA53A8F05UL, (int32_t)0x2D62B87EUL, (int32_t)0x8E99B2A2UL, (int32_t)0x76F290BCUL},
  {(int32_t)0x2FFC94D9UL, (int32_t)0xA006D64EUL, (int32_t)0x2FFC94D9UL, (int32_t)0x84E49C70UL, (int32_t)0x77F3D845UL},
  {(int32_t)0x2D9B76AAUL, (int32_t)0xA4C912ADUL, (int32_t)0x2D9B76AAUL, (int32_t)0x83F46992UL, (int32_t)0x79C6F9D3UL},
  {(int32_t)0x29DD5153UL, (int32_t)0xAC455D5AUL, (int32_t)0x29DD5153UL, (int32_t)0x830AFF4FUL, (int32_t)0x7B9107D4UL},
  {(int32_t)0x3977D623UL, (int32_t)0x8D1053BBUL, (int32_t)0x3977D623UL, (int32_t)0x8BF5D6FFUL, (int32_t)0x7CE4906AUL},
  {(int32_t)0x39F77ABDUL, (int32_t)0x8C110A87UL, (int32_t)0x39F77ABDUL, (int32_t)0x8225DE25UL, (int32_t)0x7D56954AUL},
  {(int32_t)0x3A32FCFCUL, (int32_t)0x8B9A0607UL, (int32_t)0x3A32FCFCUL, (int32_t)0x8142BAD5UL, (int32_t)0x7F1BFEEDUL}
};

const biquadInt32_t IIR_butterworth_voice_bandpass_fs16000_biquadInt32 =
{
  (int32_t)0x40000000UL, 23, sizeof(IIR_butterworth_voice_bandpass_fs16000_BiquadCellInt32) / sizeof(IIR_butterworth_voice_bandpass_fs16000_BiquadCellInt32[0]), IIR_butterworth_voice_bandpass_fs16000_BiquadCellInt32
};

// 16000 Hz band-pass IIR
//     first  stop frequency :    200 Hz
//     first  pass frequency :    300 Hz
//     second pass frequency :   1000 Hz
//     second stop frequency :   1200 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_chebyshev1_voice_bandpass_fs16000_BiquadCellFloat[] =
{
  {+0.00457940, +0.00915879, +0.00457940, -1.86006180, +0.93377063},
  {+0.00700487, +0.01400974, +0.00700487, -1.84006687, +0.93755988},
  {+0.00698938, +0.01397876, +0.00698938, -1.88425079, +0.93760400},
  {+0.00630516, +0.01261032, +0.00630516, -1.90908784, +0.94709828},
  {+0.01735808, +0.03471617, +0.01735808, -1.82750286, +0.94895230},
  {+0.00567789, +0.01135578, +0.00567789, -1.93144003, +0.95899424},
  {+2.01295291, -4.02590583, +2.01295291, -1.82508806, +0.96654723},
  {+0.60389325, -1.20778649, +0.60389325, -1.94964625, +0.97054809},
  {+0.58285008, -1.16570017, +0.58285008, -1.96367625, +0.98055274},
  {+1.46548414, -2.93096829, +1.46548414, -1.83463496, +0.98829466},
  {+0.89813000, -1.79626001, +0.89813000, -1.97438709, +0.98900529},
  {+0.90799718, -1.81599436, +0.90799718, -1.98282123, +0.99644416}
};

const biquadFloat_t IIR_chebyshev1_voice_bandpass_fs16000_biquadFloat =
{
  sizeof(IIR_chebyshev1_voice_bandpass_fs16000_BiquadCellFloat) / sizeof(IIR_chebyshev1_voice_bandpass_fs16000_BiquadCellFloat[0]), IIR_chebyshev1_voice_bandpass_fs16000_BiquadCellFloat
};

static const biquadCellInt32_t IIR_chebyshev1_voice_bandpass_fs16000_BiquadCellInt32[] =
{
  {(int32_t)0x00960EC3UL, (int32_t)0x012C1D86UL, (int32_t)0x00960EC3UL, (int32_t)0x88F4BF5CUL, (int32_t)0x7785CBBFUL},
  {(int32_t)0x00E5891FUL, (int32_t)0x01CB123DUL, (int32_t)0x00E5891FUL, (int32_t)0x8A3C5828UL, (int32_t)0x7801F654UL},
  {(int32_t)0x00E5072EUL, (int32_t)0x01CA0E5DUL, (int32_t)0x00E5072EUL, (int32_t)0x87686F5CUL, (int32_t)0x78036872UL},
  {(int32_t)0x00CE9B87UL, (int32_t)0x019D370EUL, (int32_t)0x00CE9B87UL, (int32_t)0x85D1813EUL, (int32_t)0x793A8433UL},
  {(int32_t)0x0238CA29UL, (int32_t)0x04719451UL, (int32_t)0x0238CA29UL, (int32_t)0x8B0A3172UL, (int32_t)0x797744D9UL},
  {(int32_t)0x00BA0D96UL, (int32_t)0x01741B2CUL, (int32_t)0x00BA0D96UL, (int32_t)0x8463495EUL, (int32_t)0x7AC052C1UL},
  {(int32_t)0x20350E1DUL, (int32_t)0xBF95E3C6UL, (int32_t)0x20350E1DUL, (int32_t)0x8B31C1D5UL, (int32_t)0x7BB7D1CFUL},
  {(int32_t)0x26A62FDDUL, (int32_t)0xB2B3A046UL, (int32_t)0x26A62FDDUL, (int32_t)0x8338FEEDUL, (int32_t)0x7C3AEB77UL},
  {(int32_t)0x254D6A6FUL, (int32_t)0xB5652B22UL, (int32_t)0x254D6A6FUL, (int32_t)0x825320DEUL, (int32_t)0x7D82C08EUL},
  {(int32_t)0x2EE53F01UL, (int32_t)0xA23581FEUL, (int32_t)0x2EE53F01UL, (int32_t)0x8A955744UL, (int32_t)0x7E807085UL},
  {(int32_t)0x397AF644UL, (int32_t)0x8D0A1378UL, (int32_t)0x397AF644UL, (int32_t)0x81A3A451UL, (int32_t)0x7E97B9B2UL},
  {(int32_t)0x3A1CA036UL, (int32_t)0x8BC6BF93UL, (int32_t)0x3A1CA036UL, (int32_t)0x811974FDUL, (int32_t)0x7F8B7B6FUL}
};

const biquadInt32_t IIR_chebyshev1_voice_bandpass_fs16000_biquadInt32 =
{
  (int32_t)0x7FFFFFFBUL, 9, sizeof(IIR_chebyshev1_voice_bandpass_fs16000_BiquadCellInt32) / sizeof(IIR_chebyshev1_voice_bandpass_fs16000_BiquadCellInt32[0]), IIR_chebyshev1_voice_bandpass_fs16000_BiquadCellInt32
};

// 16000 Hz band-pass IIR
//     first  stop frequency :    200 Hz
//     first  pass frequency :    300 Hz
//     second pass frequency :   1000 Hz
//     second stop frequency :   1200 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_chebyshev2_voice_bandpass_fs16000_BiquadCellFloat[] =
{
  {+0.06165949, +0.05046126, +0.06165949, -1.19000786, +0.36378811},
  {+0.20088113, -0.21926761, +0.20088113, -1.30926440, +0.49175905},
  {+0.40056630, -0.62063450, +0.40056630, -1.47035455, +0.65085265},
  {+0.57342241, -0.97334551, +0.57342241, -1.60870633, +0.78220564},
  {+1.17048271, -2.34079077, +1.17048271, -1.86606226, +0.87094861},
  {+0.65475636, -1.15030366, +0.65475636, -1.71305229, +0.88138590},
  {+0.98060956, -1.96003183, +0.98060956, -1.89511226, +0.90029547},
  {+0.95572375, -1.90877006, +0.95572375, -1.92376020, +0.92968909},
  {+0.94751745, -1.89091842, +0.94751745, -1.94702701, +0.95381046},
  {+0.66459475, -1.18261190, +0.66459475, -1.79367537, +0.96182107},
  {+0.97401534, -1.94268210, +0.97401534, -1.96632539, +0.97381131},
  {+0.98148456, -1.95699225, +0.98148456, -1.98358675, +0.99148627}
};

const biquadFloat_t IIR_chebyshev2_voice_bandpass_fs16000_biquadFloat =
{
  sizeof(IIR_chebyshev2_voice_bandpass_fs16000_BiquadCellFloat) / sizeof(IIR_chebyshev2_voice_bandpass_fs16000_BiquadCellFloat[0]), IIR_chebyshev2_voice_bandpass_fs16000_BiquadCellFloat
};

static const biquadCellInt32_t IIR_chebyshev2_voice_bandpass_fs16000_BiquadCellInt32[] =
{
  {(int32_t)0x07E4754FUL, (int32_t)0x067583BAUL, (int32_t)0x07E4754FUL, (int32_t)0xB3D6E942UL, (int32_t)0x2E909BD3UL},
  {(int32_t)0x19B6790FUL, (int32_t)0xE3EF09F7UL, (int32_t)0x19B6790FUL, (int32_t)0xAC350315UL, (int32_t)0x3EF1F5EBUL},
  {(int32_t)0x3345C1B1UL, (int32_t)0xB08F0C73UL, (int32_t)0x3345C1B1UL, (int32_t)0xA1E5B609UL, (int32_t)0x534F23C4UL},
  {(int32_t)0x4965E7D5UL, (int32_t)0x83696A19UL, (int32_t)0x4965E7D5UL, (int32_t)0x990AF4A0UL, (int32_t)0x641F5084UL},
  {(int32_t)0x25749826UL, (int32_t)0xB5183DF6UL, (int32_t)0x25749826UL, (int32_t)0x88926F95UL, (int32_t)0x6F7B3E77UL},
  {(int32_t)0x29E7873DUL, (int32_t)0xB6616CC7UL, (int32_t)0x29E7873DUL, (int32_t)0x925D59ECUL, (int32_t)0x70D140CDUL},
  {(int32_t)0x3EC24E9EUL, (int32_t)0x828ED6ABUL, (int32_t)0x3EC24E9EUL, (int32_t)0x86B67B15UL, (int32_t)0x733CE1C3UL},
  {(int32_t)0x3D2A93EFUL, (int32_t)0x85D6B615UL, (int32_t)0x3D2A93EFUL, (int32_t)0x84E11CE5UL, (int32_t)0x77000D5CUL},
  {(int32_t)0x3CA4203AUL, (int32_t)0x86FB3152UL, (int32_t)0x3CA4203AUL, (int32_t)0x8363E8D3UL, (int32_t)0x7A16760AUL},
  {(int32_t)0x2A88B86FUL, (int32_t)0xB450162DUL, (int32_t)0x2A88B86FUL, (int32_t)0x8D346C3AUL, (int32_t)0x7B1CF3EAUL},
  {(int32_t)0x3E56446DUL, (int32_t)0x83AB18B0UL, (int32_t)0x3E56446DUL, (int32_t)0x8227B98EUL, (int32_t)0x7CA5D95EUL},
  {(int32_t)0x3ED0A49BUL, (int32_t)0x82C0A392UL, (int32_t)0x3ED0A49BUL, (int32_t)0x810CEA2BUL, (int32_t)0x7EE905A0UL}
};

const biquadInt32_t IIR_chebyshev2_voice_bandpass_fs16000_biquadInt32 =
{
  (int32_t)0x40000000UL, 10, sizeof(IIR_chebyshev2_voice_bandpass_fs16000_BiquadCellInt32) / sizeof(IIR_chebyshev2_voice_bandpass_fs16000_BiquadCellInt32[0]), IIR_chebyshev2_voice_bandpass_fs16000_BiquadCellInt32
};

// 16000 Hz band-pass IIR
//     first  stop frequency :    200 Hz
//     first  pass frequency :    300 Hz
//     second pass frequency :   1000 Hz
//     second stop frequency :   1200 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_elliptic_voice_bandpass_fs16000_BiquadCellFloat[] =
{
  {+0.06372374, +0.00000000, -0.06372374, -1.78134353, +0.87255262},
  {+0.10684273, -0.15773826, +0.10684273, -1.83209727, +0.87559021},
  {+0.37032466, -0.64550748, +0.37032466, -1.79519700, +0.92917451},
  {+0.75865137, -1.51453307, +0.75865137, -1.91561693, +0.93685371},
  {+0.83630427, -1.66591545, +0.83630427, -1.96053502, +0.97559143},
  {+0.61715247, -1.10427795, +0.61715247, -1.82412381, +0.97937554},
  {+0.97164409, -1.93369319, +0.97164409, -1.98040915, +0.99380683}
};

const biquadFloat_t IIR_elliptic_voice_bandpass_fs16000_biquadFloat =
{
  sizeof(IIR_elliptic_voice_bandpass_fs16000_BiquadCellFloat) / sizeof(IIR_elliptic_voice_bandpass_fs16000_BiquadCellFloat[0]), IIR_elliptic_voice_bandpass_fs16000_BiquadCellFloat
};

static const biquadCellInt32_t IIR_elliptic_voice_bandpass_fs16000_BiquadCellInt32[] =
{
  {(int32_t)0x08281976UL, (int32_t)0x00000000UL, (int32_t)0xF7D7E68AUL, (int32_t)0x8DFE77B2UL, (int32_t)0x6FAFCDDBUL},
  {(int32_t)0x0DAD05C0UL, (int32_t)0xEBCF3B94UL, (int32_t)0x0DAD05C0UL, (int32_t)0x8ABEEB16UL, (int32_t)0x7013570DUL},
  {(int32_t)0x2F66CC6FUL, (int32_t)0xAD6002C9UL, (int32_t)0x2F66CC6FUL, (int32_t)0x8D1B7E08UL, (int32_t)0x76EF30BAUL},
  {(int32_t)0x308DBE7CUL, (int32_t)0x9F11E3E0UL, (int32_t)0x308DBE7CUL, (int32_t)0x8566883EUL, (int32_t)0x77EAD28FUL},
  {(int32_t)0x35860258UL, (int32_t)0x9561A429UL, (int32_t)0x35860258UL, (int32_t)0x82869822UL, (int32_t)0x7CE02E0EUL},
  {(int32_t)0x277F6D11UL, (int32_t)0xB9538290UL, (int32_t)0x277F6D11UL, (int32_t)0x8B418E33UL, (int32_t)0x7D5C2D80UL},
  {(int32_t)0x3E2F6AF4UL, (int32_t)0x843E5E61UL, (int32_t)0x3E2F6AF4UL, (int32_t)0x8140FA00UL, (int32_t)0x7F350FE6UL}
};

const biquadInt32_t IIR_elliptic_voice_bandpass_fs16000_biquadInt32 =
{
  (int32_t)0x7FFFFF74UL, 4, sizeof(IIR_elliptic_voice_bandpass_fs16000_BiquadCellInt32) / sizeof(IIR_elliptic_voice_bandpass_fs16000_BiquadCellInt32[0]), IIR_elliptic_voice_bandpass_fs16000_BiquadCellInt32
};

#endif /* AC_SUPPORT_FS_16000 */

#ifdef AC_SUPPORT_FS_48000
// 48000 Hz band-pass IIR
//     first  stop frequency :    200 Hz
//     first  pass frequency :    300 Hz
//     second pass frequency :   1000 Hz
//     second stop frequency :   1200 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_butterworth_voice_bandpass_fs48000_BiquadCellFloat[] =
{
  {+0.00230637, +0.00461273, +0.00230637, -1.87823998, +0.88795657},
  {+0.00206682, +0.00413364, +0.00206682, -1.88016794, +0.88860107},
  {+0.00253988, +0.00507975, +0.00253988, -1.87898077, +0.88995807},
  {+0.00181832, +0.00363665, +0.00181832, -1.88486754, +0.89203171},
  {+0.00277281, +0.00554562, +0.00277281, -1.88220396, +0.89439376},
  {+0.00156346, +0.00312693, +0.00156346, -1.89219764, +0.89816190},
  {+0.00301038, +0.00602077, +0.00301038, -1.88770740, +0.90104258},
  {+0.00131291, +0.00262583, +0.00131291, -1.90155005, +0.90645136},
  {+0.00325370, +0.00650740, +0.00325370, -1.89529247, +0.90969040},
  {+0.00108333, +0.00216665, +0.00108333, -1.91184818, +0.91587808},
  {+0.00349543, +0.00699085, +0.00349543, -1.90476521, +0.92012962},
  {+0.00088864, +0.00177728, +0.00088864, -1.92201027, +0.92537047},
  {+0.00372093, +0.00744186, +0.00372093, -1.91593209, +0.93215415},
  {+0.00073281, +0.00146561, +0.00073281, -1.93141247, +0.93427446},
  {+0.03088392, +0.00000000, -0.03088392, -1.93988708, +0.94237998},
  {+6.28751302, -12.57502604, +6.28751302, -1.92859481, +0.94555389},
  {+0.93994052, -1.87988104, +0.93994052, -1.94749900, +0.94971571},
  {+0.84533876, -1.69067753, +0.84533876, -1.95438428, +0.95639151},
  {+3.65560759, -7.31121518, +3.65560759, -1.94254551, +0.96010992},
  {+0.79735585, -1.59471171, +0.79735585, -1.96068148, +0.96252806},
  {+0.75677924, -1.51355848, +0.75677924, -1.96651113, +0.96823379},
  {+0.71439649, -1.42879298, +0.71439649, -1.97197334, +0.97360055},
  {+2.89356783, -5.78713565, +2.89356783, -1.95756287, +0.97559064},
  {+0.76558530, -1.53117060, +0.76558530, -1.97715068, +0.97870524},
  {+0.72412295, -1.44824590, +0.72412295, -1.98211209, +0.98361290},
  {+0.66158280, -1.32316561, +0.66158280, -1.98691647, +0.98837974},
  {+1.80821114, -3.61642229, +1.80821114, -1.97340904, +0.99174885},
  {+0.91181857, -1.82363714, +0.91181857, -1.99161561, +0.99305580},
  {+0.91115247, -1.82230494, +0.91115247, -1.99625667, +0.99768721}
};

const biquadFloat_t IIR_butterworth_voice_bandpass_fs48000_biquadFloat =
{
  sizeof(IIR_butterworth_voice_bandpass_fs48000_BiquadCellFloat) / sizeof(IIR_butterworth_voice_bandpass_fs48000_BiquadCellFloat[0]), IIR_butterworth_voice_bandpass_fs48000_BiquadCellFloat
};

static const biquadCellInt32_t IIR_butterworth_voice_bandpass_fs48000_BiquadCellInt32[] =
{
  {(int32_t)0x004B9333UL, (int32_t)0x00972666UL, (int32_t)0x004B9333UL, (int32_t)0x87CAEA86UL, (int32_t)0x71A88F8CUL},
  {(int32_t)0x0043B9C0UL, (int32_t)0x00877380UL, (int32_t)0x0043B9C0UL, (int32_t)0x87AB5418UL, (int32_t)0x71BDAE03UL},
  {(int32_t)0x00533A07UL, (int32_t)0x00A6740DUL, (int32_t)0x00533A07UL, (int32_t)0x87BEC76FUL, (int32_t)0x71EA2558UL},
  {(int32_t)0x003B9533UL, (int32_t)0x00772A67UL, (int32_t)0x003B9533UL, (int32_t)0x875E5488UL, (int32_t)0x722E1856UL},
  {(int32_t)0x005ADC07UL, (int32_t)0x00B5B80EUL, (int32_t)0x005ADC07UL, (int32_t)0x8789F866UL, (int32_t)0x727B7EA8UL},
  {(int32_t)0x00333B47UL, (int32_t)0x0066768EUL, (int32_t)0x00333B47UL, (int32_t)0x86E63BE0UL, (int32_t)0x72F6F824UL},
  {(int32_t)0x0062A4ECUL, (int32_t)0x00C549D7UL, (int32_t)0x0062A4ECUL, (int32_t)0x872FCD4BUL, (int32_t)0x73555CFFUL},
  {(int32_t)0x002B0583UL, (int32_t)0x00560B05UL, (int32_t)0x002B0583UL, (int32_t)0x864D0107UL, (int32_t)0x7406991DUL},
  {(int32_t)0x006A9E01UL, (int32_t)0x00D53C02UL, (int32_t)0x006A9E01UL, (int32_t)0x86B3873AUL, (int32_t)0x7470BC2DUL},
  {(int32_t)0x00237F9BUL, (int32_t)0x0046FF36UL, (int32_t)0x00237F9BUL, (int32_t)0x85A4478AUL, (int32_t)0x753B7E2FUL},
  {(int32_t)0x007289C2UL, (int32_t)0x00E51383UL, (int32_t)0x007289C2UL, (int32_t)0x861853AEUL, (int32_t)0x75C6CEB8UL},
  {(int32_t)0x001D1E72UL, (int32_t)0x003A3CE4UL, (int32_t)0x001D1E72UL, (int32_t)0x84FDC8A1UL, (int32_t)0x76728A22UL},
  {(int32_t)0x0079ED6CUL, (int32_t)0x00F3DAD9UL, (int32_t)0x0079ED6CUL, (int32_t)0x85615E5FUL, (int32_t)0x7750D3CCUL},
  {(int32_t)0x0018033AUL, (int32_t)0x00300674UL, (int32_t)0x0018033AUL, (int32_t)0x8463BCF2UL, (int32_t)0x77964E34UL},
  {(int32_t)0x03F4010FUL, (int32_t)0x00000000UL, (int32_t)0xFC0BFEF1UL, (int32_t)0x83D8E3DFUL, (int32_t)0x789FE83BUL},
  {(int32_t)0x324CD3A0UL, (int32_t)0x9B6658C0UL, (int32_t)0x324CD3A0UL, (int32_t)0x8491E716UL, (int32_t)0x7907E8F7UL},
  {(int32_t)0x3C27FC4AUL, (int32_t)0x87B0076CUL, (int32_t)0x3C27FC4AUL, (int32_t)0x835C2D28UL, (int32_t)0x799048C6UL},
  {(int32_t)0x361A07C1UL, (int32_t)0x93CBF07EUL, (int32_t)0x361A07C1UL, (int32_t)0x82EB5E2EUL, (int32_t)0x7A6B0983UL},
  {(int32_t)0x3A7D5E62UL, (int32_t)0x8B05433CUL, (int32_t)0x3A7D5E62UL, (int32_t)0x83AD559CUL, (int32_t)0x7AE4E1CAUL},
  {(int32_t)0x3307E0D8UL, (int32_t)0x99F03E51UL, (int32_t)0x3307E0D8UL, (int32_t)0x828431D4UL, (int32_t)0x7B341E9AUL},
  {(int32_t)0x306F122FUL, (int32_t)0x9F21DBA2UL, (int32_t)0x306F122FUL, (int32_t)0x8224AE7EUL, (int32_t)0x7BEF15B9UL},
  {(int32_t)0x2DB8AC0EUL, (int32_t)0xA48EA7E5UL, (int32_t)0x2DB8AC0EUL, (int32_t)0x81CB3055UL, (int32_t)0x7C9EF15DUL},
  {(int32_t)0x2E4C0DC7UL, (int32_t)0xA367E473UL, (int32_t)0x2E4C0DC7UL, (int32_t)0x82B74A3AUL, (int32_t)0x7CE0276CUL},
  {(int32_t)0x30FF597DUL, (int32_t)0x9E014D06UL, (int32_t)0x30FF597DUL, (int32_t)0x81765D02UL, (int32_t)0x7D4636A1UL},
  {(int32_t)0x2E5807CAUL, (int32_t)0xA34FF06CUL, (int32_t)0x2E5807CAUL, (int32_t)0x81251355UL, (int32_t)0x7DE70712UL},
  {(int32_t)0x2A575F66UL, (int32_t)0xAB514133UL, (int32_t)0x2A575F66UL, (int32_t)0x80D65C50UL, (int32_t)0x7E833A38UL},
  {(int32_t)0x39DCDD9DUL, (int32_t)0x8C4644C6UL, (int32_t)0x39DCDD9DUL, (int32_t)0x81B3AA95UL, (int32_t)0x7EF1A04FUL},
  {(int32_t)0x3A5B3C45UL, (int32_t)0x8B498776UL, (int32_t)0x3A5B3C45UL, (int32_t)0x80895EB3UL, (int32_t)0x7F1C73D8UL},
  {(int32_t)0x3A505274UL, (int32_t)0x8B5F5B18UL, (int32_t)0x3A505274UL, (int32_t)0x803D54A6UL, (int32_t)0x7FB436E2UL}
};

const biquadInt32_t IIR_butterworth_voice_bandpass_fs48000_biquadInt32 =
{
  (int32_t)0x40000000UL, 23, sizeof(IIR_butterworth_voice_bandpass_fs48000_BiquadCellInt32) / sizeof(IIR_butterworth_voice_bandpass_fs48000_BiquadCellInt32[0]), IIR_butterworth_voice_bandpass_fs48000_BiquadCellInt32
};

// 48000 Hz band-pass IIR
//     first  stop frequency :    200 Hz
//     first  pass frequency :    300 Hz
//     second pass frequency :   1000 Hz
//     second stop frequency :   1200 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_chebyshev1_voice_bandpass_fs48000_BiquadCellFloat[] =
{
  {+0.00051552, +0.00103104, +0.00051552, -1.96906858, +0.97744093},
  {+0.00079045, +0.00158090, +0.00079045, -1.96755883, +0.97865195},
  {+0.00078410, +0.00156819, +0.00078410, -1.97281022, +0.97885280},
  {+0.00070585, +0.00141170, +0.00070585, -1.97787181, +0.98216209},
  {+0.00196028, +0.00392056, +0.00196028, -1.96870476, +0.98252169},
  {+0.00063611, +0.00127222, +0.00063611, -1.98313425, +0.98623393},
  {+2.05717552, -4.11435103, +2.05717552, -1.97251169, +0.98856349},
  {+0.61109905, -1.22219811, +0.61109905, -1.98780245, +0.99014676},
  {+0.58755968, -1.17511936, +0.58755968, -1.99162351, +0.99351148},
  {+1.48968527, -2.97937054, +1.48968527, -1.97867311, +0.99601733},
  {+0.90250081, -1.80500162, +0.90250081, -1.99470851, +0.99634008},
  {+0.90993404, -1.81986808, +0.90993404, -1.99730183, +0.99881894}
};

const biquadFloat_t IIR_chebyshev1_voice_bandpass_fs48000_biquadFloat =
{
  sizeof(IIR_chebyshev1_voice_bandpass_fs48000_BiquadCellFloat) / sizeof(IIR_chebyshev1_voice_bandpass_fs48000_BiquadCellFloat[0]), IIR_chebyshev1_voice_bandpass_fs48000_BiquadCellFloat
};

static const biquadCellInt32_t IIR_chebyshev1_voice_bandpass_fs48000_BiquadCellInt32[] =
{
  {(int32_t)0x0010E480UL, (int32_t)0x0021C8FFUL, (int32_t)0x0010E480UL, (int32_t)0x81FAC7CBUL, (int32_t)0x7D1CC8D2UL},
  {(int32_t)0x0019E6C7UL, (int32_t)0x0033CD8EUL, (int32_t)0x0019E6C7UL, (int32_t)0x8213841DUL, (int32_t)0x7D447794UL},
  {(int32_t)0x0019B179UL, (int32_t)0x003362F1UL, (int32_t)0x0019B179UL, (int32_t)0x81BD7A36UL, (int32_t)0x7D4B0C78UL},
  {(int32_t)0x00172118UL, (int32_t)0x002E4231UL, (int32_t)0x00172118UL, (int32_t)0x816A8C5CUL, (int32_t)0x7DB77CCCUL},
  {(int32_t)0x00403C06UL, (int32_t)0x0080780BUL, (int32_t)0x00403C06UL, (int32_t)0x8200BDBEUL, (int32_t)0x7DC34558UL},
  {(int32_t)0x0014D814UL, (int32_t)0x0029B028UL, (int32_t)0x0014D814UL, (int32_t)0x81145418UL, (int32_t)0x7E3CE9D4UL},
  {(int32_t)0x20EA30E0UL, (int32_t)0xBE2B9E41UL, (int32_t)0x20EA30E0UL, (int32_t)0x81C25E51UL, (int32_t)0x7E893FA3UL},
  {(int32_t)0x271C3F34UL, (int32_t)0xB1C78198UL, (int32_t)0x271C3F34UL, (int32_t)0x80C7D840UL, (int32_t)0x7EBD210DUL},
  {(int32_t)0x259A93E8UL, (int32_t)0xB4CAD830UL, (int32_t)0x259A93E8UL, (int32_t)0x80893D88UL, (int32_t)0x7F2B6253UL},
  {(int32_t)0x2FAB8071UL, (int32_t)0xA0A8FF1DUL, (int32_t)0x2FAB8071UL, (int32_t)0x815D6B78UL, (int32_t)0x7F7D7EEFUL},
  {(int32_t)0x39C292C4UL, (int32_t)0x8C7ADA78UL, (int32_t)0x39C292C4UL, (int32_t)0x8056B222UL, (int32_t)0x7F88125CUL},
  {(int32_t)0x3A3C5C02UL, (int32_t)0x8B8747FCUL, (int32_t)0x3A3C5C02UL, (int32_t)0x802C34F7UL, (int32_t)0x7FD94C85UL}
};

const biquadInt32_t IIR_chebyshev1_voice_bandpass_fs48000_biquadInt32 =
{
  (int32_t)0x7FFFFFF7UL, 9, sizeof(IIR_chebyshev1_voice_bandpass_fs48000_BiquadCellInt32) / sizeof(IIR_chebyshev1_voice_bandpass_fs48000_BiquadCellInt32[0]), IIR_chebyshev1_voice_bandpass_fs48000_BiquadCellInt32
};

// 48000 Hz band-pass IIR
//     first  stop frequency :    200 Hz
//     first  pass frequency :    300 Hz
//     second pass frequency :   1000 Hz
//     second stop frequency :   1200 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_chebyshev2_voice_bandpass_fs48000_BiquadCellFloat[] =
{
  {+0.03043532, -0.03615533, +0.03043532, -1.69781495, +0.72253025},
  {+0.20312822, -0.38150363, +0.20312822, -1.76505892, +0.78981174},
  {+0.42843524, -0.83387877, +0.42843524, -1.84186342, +0.86485513},
  {+0.60323985, -1.18548057, +0.60323985, -1.89904998, +0.92004911},
  {+1.22980786, -2.45959522, +1.22980786, -1.95421284, +0.95478704},
  {+0.67166928, -1.32479182, +0.67166928, -1.93846045, +0.95809082},
  {+1.01534753, -2.03055788, +1.01534753, -1.96501390, +0.96561308},
  {+0.97887912, -1.95745264, +0.97887912, -1.97539428, +0.97607047},
  {+0.96227462, -1.92408393, +0.96227462, -1.98366886, +0.98443488},
  {+0.66536696, -1.31414874, +0.66536696, -1.96783975, +0.98689639},
  {+0.98274568, -1.96489119, +0.98274568, -1.99040338, +0.99124228},
  {+0.98436454, -1.96806271, +0.98436454, -1.99629128, +0.99717094}
};

const biquadFloat_t IIR_chebyshev2_voice_bandpass_fs48000_biquadFloat =
{
  sizeof(IIR_chebyshev2_voice_bandpass_fs48000_BiquadCellFloat) / sizeof(IIR_chebyshev2_voice_bandpass_fs48000_BiquadCellFloat[0]), IIR_chebyshev2_voice_bandpass_fs48000_BiquadCellFloat
};

static const biquadCellInt32_t IIR_chebyshev2_voice_bandpass_fs48000_BiquadCellInt32[] =
{
  {(int32_t)0x03E54DF4UL, (int32_t)0xFB5F431BUL, (int32_t)0x03E54DF4UL, (int32_t)0x9356FFFBUL, (int32_t)0x5C7BDF0EUL},
  {(int32_t)0x1A001B06UL, (int32_t)0xCF2AE39EUL, (int32_t)0x1A001B06UL, (int32_t)0x8F09464EUL, (int32_t)0x65188D0EUL},
  {(int32_t)0x36D6F747UL, (int32_t)0x954375D7UL, (int32_t)0x36D6F747UL, (int32_t)0x8A1EE8E2UL, (int32_t)0x6EB392A1UL},
  {(int32_t)0x269B7B50UL, (int32_t)0xB421161DUL, (int32_t)0x269B7B50UL, (int32_t)0x8675F710UL, (int32_t)0x75C42B5BUL},
  {(int32_t)0x275A9603UL, (int32_t)0xB14AFEF6UL, (int32_t)0x275A9603UL, (int32_t)0x82EE2D42UL, (int32_t)0x7A36763DUL},
  {(int32_t)0x2AFCA12BUL, (int32_t)0xAB369C5BUL, (int32_t)0x2AFCA12BUL, (int32_t)0x83F04391UL, (int32_t)0x7AA2B84CUL},
  {(int32_t)0x207DBA1DUL, (int32_t)0xBF05AB7DUL, (int32_t)0x207DBA1DUL, (int32_t)0x823D3655UL, (int32_t)0x7B99359CUL},
  {(int32_t)0x3EA5F49EUL, (int32_t)0x82B91890UL, (int32_t)0x3EA5F49EUL, (int32_t)0x819323E0UL, (int32_t)0x7CEFE095UL},
  {(int32_t)0x3D95E84EUL, (int32_t)0x84DBCF18UL, (int32_t)0x3D95E84EUL, (int32_t)0x810B91C3UL, (int32_t)0x7E01F649UL},
  {(int32_t)0x2A955F4BUL, (int32_t)0xABE4FCB2UL, (int32_t)0x2A955F4BUL, (int32_t)0x820EE9DCUL, (int32_t)0x7E529EFDUL},
  {(int32_t)0x3EE54E1FUL, (int32_t)0x823F3905UL, (int32_t)0x3EE54E1FUL, (int32_t)0x809D3B23UL, (int32_t)0x7EE106EDUL},
  {(int32_t)0x3EFFD426UL, (int32_t)0x820B42B8UL, (int32_t)0x3EFFD426UL, (int32_t)0x803CC37EUL, (int32_t)0x7FA34C28UL}
};

const biquadInt32_t IIR_chebyshev2_voice_bandpass_fs48000_biquadInt32 =
{
  (int32_t)0x40000000UL, 12, sizeof(IIR_chebyshev2_voice_bandpass_fs48000_BiquadCellInt32) / sizeof(IIR_chebyshev2_voice_bandpass_fs48000_BiquadCellInt32[0]), IIR_chebyshev2_voice_bandpass_fs48000_BiquadCellInt32
};

// 48000 Hz band-pass IIR
//     first  stop frequency :    200 Hz
//     first  pass frequency :    300 Hz
//     second pass frequency :   1000 Hz
//     second stop frequency :   1200 Hz
//     band-pass ripple :  +0.10 dB
//     band-stop ripple : +60.00 dB

static const biquadCellFloat_t IIR_elliptic_voice_bandpass_fs48000_BiquadCellFloat[] =
{
  {+0.02226210, +0.00000000, -0.02226210, -1.94487078, +0.95547604},
  {+0.10012958, -0.19385273, +0.10012958, -1.95194217, +0.95697336},
  {+0.36913716, -0.72738677, +0.36913716, -1.96012722, +0.97550833},
  {+0.77580436, -1.55129249, +0.77580436, -1.97621977, +0.97862936},
  {+0.84315302, -1.68555406, +0.84315302, -1.99015101, +0.99183895},
  {+0.61392800, -1.21315098, +0.61392800, -1.97537468, +0.99295805},
  {+0.97400348, -1.94693586, +0.97400348, -1.99644757, +0.99794100}
};

const biquadFloat_t IIR_elliptic_voice_bandpass_fs48000_biquadFloat =
{
  sizeof(IIR_elliptic_voice_bandpass_fs48000_BiquadCellFloat) / sizeof(IIR_elliptic_voice_bandpass_fs48000_BiquadCellFloat[0]), IIR_elliptic_voice_bandpass_fs48000_BiquadCellFloat
};

static const biquadCellInt32_t IIR_elliptic_voice_bandpass_fs48000_BiquadCellInt32[] =
{
  {(int32_t)0x02D97C11UL, (int32_t)0x00000000UL, (int32_t)0xFD2683EFUL, (int32_t)0x83873CB5UL, (int32_t)0x7A4D09EFUL},
  {(int32_t)0x0CD10BD0UL, (int32_t)0xE72FD577UL, (int32_t)0x0CD10BD0UL, (int32_t)0x8313612AUL, (int32_t)0x7A7E1A6BUL},
  {(int32_t)0x2F3FE2F6UL, (int32_t)0xA2E4FD88UL, (int32_t)0x2F3FE2F6UL, (int32_t)0x828D468EUL, (int32_t)0x7CDD74F9UL},
  {(int32_t)0x31A6C753UL, (int32_t)0x9CB79FB5UL, (int32_t)0x31A6C753UL, (int32_t)0x81859D81UL, (int32_t)0x7D43BA0DUL},
  {(int32_t)0x35F63813UL, (int32_t)0x941FE1DCUL, (int32_t)0x35F63813UL, (int32_t)0x80A15DABUL, (int32_t)0x7EF49426UL},
  {(int32_t)0x274A98A5UL, (int32_t)0xB25BBBFEUL, (int32_t)0x274A98A5UL, (int32_t)0x81937611UL, (int32_t)0x7F193FE0UL},
  {(int32_t)0x3E5612C2UL, (int32_t)0x8365670DUL, (int32_t)0x3E5612C2UL, (int32_t)0x803A33FCUL, (int32_t)0x7FBC87DFUL}
};

const biquadInt32_t IIR_elliptic_voice_bandpass_fs48000_biquadInt32 =
{
  (int32_t)0x7FFFFFE6UL, 4, sizeof(IIR_elliptic_voice_bandpass_fs48000_BiquadCellInt32) / sizeof(IIR_elliptic_voice_bandpass_fs48000_BiquadCellInt32[0]), IIR_elliptic_voice_bandpass_fs48000_BiquadCellInt32
};

#endif /* AC_SUPPORT_FS_48000 */

