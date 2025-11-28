/* ------------------------------------------------------------
author: "Albert Graef"
name: "faust_echo", "echo -- stereo delay effect"
version: "1.0"
Code generated with Faust 2.60.0 (https://faust.grame.fr)
Compilation options: -lang c -ct 1 -es 1 -mcd 16 -single -ftz 0
------------------------------------------------------------ */

#ifndef  __mydsp_H__
#define  __mydsp_H__

#ifndef FAUSTFLOAT
  #define FAUSTFLOAT float
#endif


#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32)
#define RESTRICT __restrict
#else
#define RESTRICT __restrict__
#endif

#include <math.h>
#include <stdint.h>
#include <stdlib.h>


#ifndef FAUSTCLASS
#define FAUSTCLASS mydsp
#endif

#ifdef __APPLE__
#define exp10f __exp10f
#define exp10 __exp10
#endif

typedef struct
{
  FAUSTFLOAT fHslider0;
  FAUSTFLOAT fHslider1;
  int IOTA0;
  float fVec0[8192];
  FAUSTFLOAT fHslider2;
  int fSampleRate;
  float fConst0;
  float fRec0[2];
  float fVec1[8192];
  float fRec1[2];
  FAUSTFLOAT fHslider3;
} mydsp;

static mydsp *newmydsp()
{
  mydsp *dsp = (mydsp *)calloc(1, sizeof(mydsp));
  return dsp;
}

static void deletemydsp(mydsp *dsp)
{
  free(dsp);
}

#if FULL_BUILD
static void metadatamydsp(MetaGlue *m)
{
  m->declare(m->metaInterface, "author", "Albert Graef");
  m->declare(m->metaInterface, "compile_options", "-lang c -ct 1 -es 1 -mcd 16 -single -ftz 0");
  m->declare(m->metaInterface, "filename", "faust_echo.dsp");
  m->declare(m->metaInterface, "math.lib/author", "GRAME");
  m->declare(m->metaInterface, "math.lib/copyright", "GRAME");
  m->declare(m->metaInterface, "math.lib/deprecated", "This library is deprecated and is not maintained anymore. It will be removed in August 2017.");
  m->declare(m->metaInterface, "math.lib/license", "LGPL with exception");
  m->declare(m->metaInterface, "math.lib/name", "Math Library");
  m->declare(m->metaInterface, "math.lib/version", "1.0");
  m->declare(m->metaInterface, "music.lib/author", "GRAME");
  m->declare(m->metaInterface, "music.lib/copyright", "GRAME");
  m->declare(m->metaInterface, "music.lib/deprecated", "This library is deprecated and is not maintained anymore. It will be removed in August 2017.");
  m->declare(m->metaInterface, "music.lib/license", "LGPL with exception");
  m->declare(m->metaInterface, "music.lib/name", "Music Library");
  m->declare(m->metaInterface, "music.lib/version", "1.0");
  m->declare(m->metaInterface, "name", "faust_echo");
  m->declare(m->metaInterface, "version", "1.0");
}
#endif


static int getSampleRatemydsp(mydsp *RESTRICT dsp)
{
  return dsp->fSampleRate;
}

static int getNumInputsmydsp(mydsp *RESTRICT dsp)
{
  return 2;
}
static int getNumOutputsmydsp(mydsp *RESTRICT dsp)
{
  return 2;
}

static void classInitmydsp(int sample_rate)
{
}

static void instanceResetUserInterfacemydsp(mydsp *dsp)
{
  dsp->fHslider0 = (FAUSTFLOAT)(1.0f);
  dsp->fHslider1 = (FAUSTFLOAT)(0.0f);
  dsp->fHslider2 = (FAUSTFLOAT)(0.04f);
  dsp->fHslider3 = (FAUSTFLOAT)(1.0f);
}

static void instanceClearmydsp(mydsp *dsp)
{
  dsp->IOTA0 = 0;
  /* C99 loop */
  {
    int l0;
    for (l0 = 0; l0 < 8192; l0 = l0 + 1)
    {
      dsp->fVec0[l0] = 0.0f;
    }
  }
  /* C99 loop */
  {
    int l1;
    for (l1 = 0; l1 < 2; l1 = l1 + 1)
    {
      dsp->fRec0[l1] = 0.0f;
    }
  }
  /* C99 loop */
  {
    int l2;
    for (l2 = 0; l2 < 8192; l2 = l2 + 1)
    {
      dsp->fVec1[l2] = 0.0f;
    }
  }
  /* C99 loop */
  {
    int l3;
    for (l3 = 0; l3 < 2; l3 = l3 + 1)
    {
      dsp->fRec1[l3] = 0.0f;
    }
  }
}

static void instanceConstantsmydsp(mydsp *dsp, int sample_rate)
{
  dsp->fSampleRate = sample_rate;
  dsp->fConst0 = fminf(1.92e+05f, fmaxf(1.0f, (float)(dsp->fSampleRate)));
}

static void instanceInitmydsp(mydsp *dsp, int sample_rate)
{
  instanceConstantsmydsp(dsp, sample_rate);
  instanceResetUserInterfacemydsp(dsp);
  instanceClearmydsp(dsp);
}

static void initmydsp(mydsp *dsp, int sample_rate)
{
  classInitmydsp(sample_rate);
  instanceInitmydsp(dsp, sample_rate);
}

#if FULL_BUILD
static void buildUserInterfacemydsp(mydsp *dsp, UIGlue *ui_interface)
{
  ui_interface->openVerticalBox(ui_interface->uiInterface, "echo");
  ui_interface->addHorizontalSlider(ui_interface->uiInterface, "delay", &dsp->fHslider2, (FAUSTFLOAT)0.04f, (FAUSTFLOAT)0.0f, (FAUSTFLOAT)5.0f, (FAUSTFLOAT)0.001f);
  ui_interface->addHorizontalSlider(ui_interface->uiInterface, "feedback", &dsp->fHslider1, (FAUSTFLOAT)0.0f, (FAUSTFLOAT)0.0f, (FAUSTFLOAT)1.0f, (FAUSTFLOAT)0.001f);
  ui_interface->addHorizontalSlider(ui_interface->uiInterface, "level", &dsp->fHslider3, (FAUSTFLOAT)1.0f, (FAUSTFLOAT)0.0f, (FAUSTFLOAT)1.0f, (FAUSTFLOAT)0.01f);
  ui_interface->addHorizontalSlider(ui_interface->uiInterface, "stereo", &dsp->fHslider0, (FAUSTFLOAT)1.0f, (FAUSTFLOAT)0.0f, (FAUSTFLOAT)1.0f, (FAUSTFLOAT)0.001f);
  ui_interface->closeBox(ui_interface->uiInterface);
}
#endif


static void computemydsp(mydsp *dsp, int count, FAUSTFLOAT **RESTRICT inputs, FAUSTFLOAT **RESTRICT outputs)
{
  FAUSTFLOAT *input0 = inputs[0];
  FAUSTFLOAT *input1 = inputs[1];
  FAUSTFLOAT *output0 = outputs[0];
  FAUSTFLOAT *output1 = outputs[1];
  float fSlow0 = (float)(dsp->fHslider0);
  float fSlow1 = 1.0f - fSlow0;
  float fSlow2 = (float)(dsp->fHslider1);
  float fSlow3 = dsp->fConst0 * (float)(dsp->fHslider2);
  int iSlow4 = (int)(fSlow3);
  int iSlow5 = (iSlow4 + 1) & 7999;
  float fSlow6 = (float)(iSlow4);
  float fSlow7 = fSlow3 - fSlow6;
  int iSlow8 = iSlow4 & 7999;
  float fSlow9 = fSlow6 + (1.0f - fSlow3);
  float fSlow10 = (float)(dsp->fHslider3);
  float fSlow11 = 1.0f / (fSlow10 + 1.0f);
  /* C99 loop */
  {
    int i0;
    for (i0 = 0; i0 < count; i0 = i0 + 1)
    {
      float fTemp0 = (float)(input0[i0]);
      float fTemp1 = fTemp0 + fSlow2 * (fSlow1 * dsp->fRec0[1] + fSlow0 * dsp->fRec1[1]);
      dsp->fVec0[dsp->IOTA0 & 8191] = fTemp1;
      dsp->fRec0[0] = fSlow9 * dsp->fVec0[(dsp->IOTA0 - iSlow8) & 8191] + fSlow7 * dsp->fVec0[(dsp->IOTA0 - iSlow5) & 8191];
      float fTemp2 = (float)(input1[i0]);
      float fTemp3 = fTemp2 + fSlow2 * (fSlow1 * dsp->fRec1[1] + fSlow0 * dsp->fRec0[1]);
      dsp->fVec1[dsp->IOTA0 & 8191] = fTemp3;
      dsp->fRec1[0] = fSlow9 * dsp->fVec1[(dsp->IOTA0 - iSlow8) & 8191] + fSlow7 * dsp->fVec1[(dsp->IOTA0 - iSlow5) & 8191];
      output0[i0] = (FAUSTFLOAT)(fSlow11 * (fTemp0 + fSlow10 * (fSlow1 * dsp->fRec0[0] + fSlow0 * dsp->fRec1[0])));
      output1[i0] = (FAUSTFLOAT)(fSlow11 * (fTemp2 + fSlow10 * (fSlow1 * dsp->fRec1[0] + fSlow0 * dsp->fRec0[0])));
      dsp->IOTA0 = dsp->IOTA0 + 1;
      dsp->fRec0[1] = dsp->fRec0[0];
      dsp->fRec1[1] = dsp->fRec1[0];
    }
  }
}

#ifdef __cplusplus
}
#endif

#endif
