/*
 * file : WaveGenerator.h
 *
 */

#ifndef WaveGenerator_H
#define WaveGenerator_H
#include "stdint.h"
#include "math.h"


//enum {  kSinus, kHalfSinus, kAbsSinus, kOddSinusSuite, kEvenSinusSuite, kSinusSuite,
//    kSquare, kSmoothSquare, kAdditiveSquare,
//    kElectronicBass,
//    kTriangle, kAdditiveTriangle, kSaw, kAdditiveSaw, kHalfSaw,
//    kOrgan, kOctaveOrgan, kBassOrgan, kElectronicOrgan,
//    kNoise, kTest };

typedef enum waveType
{
  kAdditiveSquare
  , kAdditiveTriangle
  , kAdditiveSaw
  , kOrgan
  , kOctaveOrgan
  , kBassOrgan
  , kSmoothSquare
  , kSinus
} waveType;


void  generateWave(float_t *outWaveForm, int32_t inLength, int16_t inWaveType, int16_t inHarmoCount, float_t inVolume);



#endif
