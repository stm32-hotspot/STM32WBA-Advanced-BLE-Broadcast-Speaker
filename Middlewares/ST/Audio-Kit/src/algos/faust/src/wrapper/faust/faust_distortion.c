/* ------------------------------------------------------------
author: "JOS, revised by RM"
name: "faust_distortion", "distortion"
version: "0.0"
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

static float mydsp_faustpower2_f(float value)
{
  return value * value;
}

#ifndef FAUSTCLASS
#define FAUSTCLASS mydsp
#endif

#ifdef __APPLE__
#define exp10f __exp10f
#define exp10 __exp10
#endif

typedef struct
{
  FAUSTFLOAT fCheckbox0;
  int fSampleRate;
  float fConst0;
  float fConst1;
  FAUSTFLOAT fHslider0;
  float fRec1[2];
  FAUSTFLOAT fHslider1;
  float fRec2[2];
  float fVec0[2];
  float fRec0[2];
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
  m->declare(m->metaInterface, "author", "JOS, revised by RM");
  m->declare(m->metaInterface, "basics.lib/bypass1:author", "Julius Smith");
  m->declare(m->metaInterface, "basics.lib/name", "Faust Basic Element Library");
  m->declare(m->metaInterface, "basics.lib/tabulateNd", "Copyright (C) 2023 Bart Brouns <bart@magnetophon.nl>");
  m->declare(m->metaInterface, "basics.lib/version", "0.10");
  m->declare(m->metaInterface, "compile_options", "-lang c -ct 1 -es 1 -mcd 16 -single -ftz 0");
  m->declare(m->metaInterface, "demos.lib/cubicnl_demo:author", "Julius O. Smith III");
  m->declare(m->metaInterface, "demos.lib/cubicnl_demo:licence", "MIT");
  m->declare(m->metaInterface, "demos.lib/name", "Faust Demos Library");
  m->declare(m->metaInterface, "demos.lib/version", "0.1");
  m->declare(m->metaInterface, "description", "Distortion demo application.");
  m->declare(m->metaInterface, "filename", "faust_distortion.dsp");
  m->declare(m->metaInterface, "filters.lib/dcblocker:author", "Julius O. Smith III");
  m->declare(m->metaInterface, "filters.lib/dcblocker:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
  m->declare(m->metaInterface, "filters.lib/dcblocker:license", "MIT-style STK-4.3 license");
  m->declare(m->metaInterface, "filters.lib/lowpass0_highpass1", "MIT-style STK-4.3 license");
  m->declare(m->metaInterface, "filters.lib/name", "Faust Filters Library");
  m->declare(m->metaInterface, "filters.lib/pole:author", "Julius O. Smith III");
  m->declare(m->metaInterface, "filters.lib/pole:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
  m->declare(m->metaInterface, "filters.lib/pole:license", "MIT-style STK-4.3 license");
  m->declare(m->metaInterface, "filters.lib/version", "0.3");
  m->declare(m->metaInterface, "filters.lib/zero:author", "Julius O. Smith III");
  m->declare(m->metaInterface, "filters.lib/zero:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
  m->declare(m->metaInterface, "filters.lib/zero:license", "MIT-style STK-4.3 license");
  m->declare(m->metaInterface, "maths.lib/author", "GRAME");
  m->declare(m->metaInterface, "maths.lib/copyright", "GRAME");
  m->declare(m->metaInterface, "maths.lib/license", "LGPL with exception");
  m->declare(m->metaInterface, "maths.lib/name", "Faust Math Library");
  m->declare(m->metaInterface, "maths.lib/version", "2.6");
  m->declare(m->metaInterface, "misceffects.lib/name", "Misc Effects Library");
  m->declare(m->metaInterface, "misceffects.lib/version", "2.1");
  m->declare(m->metaInterface, "name", "faust_distortion");
  m->declare(m->metaInterface, "platform.lib/name", "Generic Platform Library");
  m->declare(m->metaInterface, "platform.lib/version", "0.3");
  m->declare(m->metaInterface, "signals.lib/name", "Faust Signal Routing Library");
  m->declare(m->metaInterface, "signals.lib/version", "0.3");
  m->declare(m->metaInterface, "version", "0.0");
}
#endif


static int getSampleRatemydsp(mydsp *RESTRICT dsp)
{
  return dsp->fSampleRate;
}

static int getNumInputsmydsp(mydsp *RESTRICT dsp)
{
  return 1;
}
static int getNumOutputsmydsp(mydsp *RESTRICT dsp)
{
  return 1;
}

static void classInitmydsp(int sample_rate)
{
}

static void instanceResetUserInterfacemydsp(mydsp *dsp)
{
  dsp->fCheckbox0 = (FAUSTFLOAT)(0.0f);
  dsp->fHslider0 = (FAUSTFLOAT)(0.0f);
  dsp->fHslider1 = (FAUSTFLOAT)(0.0f);
}

static void instanceClearmydsp(mydsp *dsp)
{
  /* C99 loop */
  {
    int l0;
    for (l0 = 0; l0 < 2; l0 = l0 + 1)
    {
      dsp->fRec1[l0] = 0.0f;
    }
  }
  /* C99 loop */
  {
    int l1;
    for (l1 = 0; l1 < 2; l1 = l1 + 1)
    {
      dsp->fRec2[l1] = 0.0f;
    }
  }
  /* C99 loop */
  {
    int l2;
    for (l2 = 0; l2 < 2; l2 = l2 + 1)
    {
      dsp->fVec0[l2] = 0.0f;
    }
  }
  /* C99 loop */
  {
    int l3;
    for (l3 = 0; l3 < 2; l3 = l3 + 1)
    {
      dsp->fRec0[l3] = 0.0f;
    }
  }
}

static void instanceConstantsmydsp(mydsp *dsp, int sample_rate)
{
  dsp->fSampleRate = sample_rate;
  dsp->fConst0 = 44.1f / fminf(1.92e+05f, fmaxf(1.0f, (float)(dsp->fSampleRate)));
  dsp->fConst1 = 1.0f - dsp->fConst0;
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
  ui_interface->declare(ui_interface->uiInterface, 0, "tooltip", "Reference: https://ccrma.stanford.edu/~jos/pasp/Cubic_Soft_Clipper.html");
  ui_interface->openVerticalBox(ui_interface->uiInterface, "CUBIC NONLINEARITY cubicnl");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fCheckbox0, "0", "");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fCheckbox0, "tooltip", "When this is checked, the nonlinearity has no effect");
  ui_interface->addCheckButton(ui_interface->uiInterface, "Bypass", &dsp->fCheckbox0);
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider0, "1", "");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider0, "tooltip", "Amount of distortion");
  ui_interface->addHorizontalSlider(ui_interface->uiInterface, "Drive", &dsp->fHslider0, (FAUSTFLOAT)0.0f, (FAUSTFLOAT)0.0f, (FAUSTFLOAT)1.0f, (FAUSTFLOAT)0.01f);
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider1, "2", "");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider1, "tooltip", "Brings in even harmonics");
  ui_interface->addHorizontalSlider(ui_interface->uiInterface, "Offset", &dsp->fHslider1, (FAUSTFLOAT)0.0f, (FAUSTFLOAT)0.0f, (FAUSTFLOAT)1.0f, (FAUSTFLOAT)0.01f);
  ui_interface->closeBox(ui_interface->uiInterface);
}
#endif


static void computemydsp(mydsp *dsp, int count, FAUSTFLOAT **RESTRICT inputs, FAUSTFLOAT **RESTRICT outputs)
{
  FAUSTFLOAT *input0 = inputs[0];
  FAUSTFLOAT *output0 = outputs[0];
  int iSlow0 = (int)((float)(dsp->fCheckbox0));
  float fSlow1 = dsp->fConst0 * (float)(dsp->fHslider0);
  float fSlow2 = dsp->fConst0 * (float)(dsp->fHslider1);
  /* C99 loop */
  {
    int i0;
    for (i0 = 0; i0 < count; i0 = i0 + 1)
    {
      float fTemp0 = (float)(input0[i0]);
      dsp->fRec1[0] = fSlow1 + dsp->fConst1 * dsp->fRec1[1];
      dsp->fRec2[0] = fSlow2 + dsp->fConst1 * dsp->fRec2[1];
      float fTemp1 = fmaxf(-1.0f, fminf(1.0f, dsp->fRec2[0] + powf(1e+01f, 2.0f * dsp->fRec1[0]) * ((iSlow0) ? 0.0f : fTemp0)));
      float fTemp2 = fTemp1 * (1.0f - 0.33333334f * mydsp_faustpower2_f(fTemp1));
      dsp->fVec0[0] = fTemp2;
      dsp->fRec0[0] = 0.995f * dsp->fRec0[1] + fTemp2 - dsp->fVec0[1];
      output0[i0] = (FAUSTFLOAT)(((iSlow0) ? fTemp0 : dsp->fRec0[0]));
      dsp->fRec1[1] = dsp->fRec1[0];
      dsp->fRec2[1] = dsp->fRec2[0];
      dsp->fVec0[1] = dsp->fVec0[0];
      dsp->fRec0[1] = dsp->fRec0[0];
    }
  }
}

#ifdef __cplusplus
}
#endif

#endif
