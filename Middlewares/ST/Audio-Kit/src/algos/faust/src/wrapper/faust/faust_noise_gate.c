/* ------------------------------------------------------------
author: "JOS, revised by RM"
name: "faust_noise_gate", "noiseGate"
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
  FAUSTFLOAT fHslider0;
  int fSampleRate;
  float fConst0;
  float fConst1;
  FAUSTFLOAT fHslider1;
  FAUSTFLOAT fHslider2;
  float fRec1[2];
  int iVec0[2];
  FAUSTFLOAT fHslider3;
  int iRec2[2];
  float fRec0[2];
  FAUSTFLOAT fHbargraph0;
  float fRec4[2];
  int iVec1[2];
  int iRec5[2];
  float fRec3[2];
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
  m->declare(m->metaInterface, "analyzers.lib/amp_follower_ar:author", "Jonatan Liljedahl, revised by Romain Michon");
  m->declare(m->metaInterface, "analyzers.lib/name", "Faust Analyzer Library");
  m->declare(m->metaInterface, "analyzers.lib/version", "0.2");
  m->declare(m->metaInterface, "author", "JOS, revised by RM");
  m->declare(m->metaInterface, "basics.lib/bypass2:author", "Julius Smith");
  m->declare(m->metaInterface, "basics.lib/name", "Faust Basic Element Library");
  m->declare(m->metaInterface, "basics.lib/tabulateNd", "Copyright (C) 2023 Bart Brouns <bart@magnetophon.nl>");
  m->declare(m->metaInterface, "basics.lib/version", "0.10");
  m->declare(m->metaInterface, "compile_options", "-lang c -ct 1 -es 1 -mcd 16 -single -ftz 0");
  m->declare(m->metaInterface, "demos.lib/gate_demo:author", "Julius O. Smith III");
  m->declare(m->metaInterface, "demos.lib/gate_demo:licence", "MIT");
  m->declare(m->metaInterface, "demos.lib/name", "Faust Demos Library");
  m->declare(m->metaInterface, "demos.lib/version", "0.1");
  m->declare(m->metaInterface, "description", "Gate demo application.");
  m->declare(m->metaInterface, "filename", "faust_noise_gate.dsp");
  m->declare(m->metaInterface, "maths.lib/author", "GRAME");
  m->declare(m->metaInterface, "maths.lib/copyright", "GRAME");
  m->declare(m->metaInterface, "maths.lib/license", "LGPL with exception");
  m->declare(m->metaInterface, "maths.lib/name", "Faust Math Library");
  m->declare(m->metaInterface, "maths.lib/version", "2.6");
  m->declare(m->metaInterface, "misceffects.lib/name", "Misc Effects Library");
  m->declare(m->metaInterface, "misceffects.lib/version", "2.1");
  m->declare(m->metaInterface, "name", "faust_noise_gate");
  m->declare(m->metaInterface, "platform.lib/name", "Generic Platform Library");
  m->declare(m->metaInterface, "platform.lib/version", "0.3");
  m->declare(m->metaInterface, "routes.lib/name", "Faust Signal Routing Library");
  m->declare(m->metaInterface, "routes.lib/version", "0.2");
  m->declare(m->metaInterface, "signals.lib/name", "Faust Signal Routing Library");
  m->declare(m->metaInterface, "signals.lib/onePoleSwitching:author", "Jonatan Liljedahl, revised by Dario Sanfilippo");
  m->declare(m->metaInterface, "signals.lib/onePoleSwitching:licence", "STK-4.3");
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
  dsp->fCheckbox0 = (FAUSTFLOAT)(0.0f);
  dsp->fHslider0 = (FAUSTFLOAT)(-3e+01f);
  dsp->fHslider1 = (FAUSTFLOAT)(1e+01f);
  dsp->fHslider2 = (FAUSTFLOAT)(1e+02f);
  dsp->fHslider3 = (FAUSTFLOAT)(2e+02f);
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
      dsp->iVec0[l1] = 0;
    }
  }
  /* C99 loop */
  {
    int l2;
    for (l2 = 0; l2 < 2; l2 = l2 + 1)
    {
      dsp->iRec2[l2] = 0;
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
  /* C99 loop */
  {
    int l4;
    for (l4 = 0; l4 < 2; l4 = l4 + 1)
    {
      dsp->fRec4[l4] = 0.0f;
    }
  }
  /* C99 loop */
  {
    int l5;
    for (l5 = 0; l5 < 2; l5 = l5 + 1)
    {
      dsp->iVec1[l5] = 0;
    }
  }
  /* C99 loop */
  {
    int l6;
    for (l6 = 0; l6 < 2; l6 = l6 + 1)
    {
      dsp->iRec5[l6] = 0;
    }
  }
  /* C99 loop */
  {
    int l7;
    for (l7 = 0; l7 < 2; l7 = l7 + 1)
    {
      dsp->fRec3[l7] = 0.0f;
    }
  }
}

static void instanceConstantsmydsp(mydsp *dsp, int sample_rate)
{
  dsp->fSampleRate = sample_rate;
  dsp->fConst0 = fminf(1.92e+05f, fmaxf(1.0f, (float)(dsp->fSampleRate)));
  dsp->fConst1 = 1.0f / dsp->fConst0;
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
  ui_interface->declare(ui_interface->uiInterface, 0, "tooltip", "Reference: http://en.wikipedia.org/wiki/Noise_gate");
  ui_interface->openVerticalBox(ui_interface->uiInterface, "GATE");
  ui_interface->declare(ui_interface->uiInterface, 0, "0", "");
  ui_interface->openHorizontalBox(ui_interface->uiInterface, "0x00");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fCheckbox0, "0", "");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fCheckbox0, "tooltip", "When this is checked, the gate has no effect");
  ui_interface->addCheckButton(ui_interface->uiInterface, "Bypass", &dsp->fCheckbox0);
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHbargraph0, "1", "");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHbargraph0, "tooltip", "Current gain of the gate in dB");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHbargraph0, "unit", "dB");
  ui_interface->addHorizontalBargraph(ui_interface->uiInterface, "Gate Gain", &dsp->fHbargraph0, (FAUSTFLOAT) - 5e+01f, (FAUSTFLOAT)1e+01f);
  ui_interface->closeBox(ui_interface->uiInterface);
  ui_interface->declare(ui_interface->uiInterface, 0, "1", "");
  ui_interface->openHorizontalBox(ui_interface->uiInterface, "0x00");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider0, "1", "");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider0, "style", "knob");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider0, "tooltip", "When the signal level falls below the Threshold (expressed in dB), the signal is muted");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider0, "unit", "dB");
  ui_interface->addHorizontalSlider(ui_interface->uiInterface, "Threshold", &dsp->fHslider0, (FAUSTFLOAT) - 3e+01f, (FAUSTFLOAT) - 1.2e+02f, (FAUSTFLOAT)0.0f, (FAUSTFLOAT)0.1f);
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider1, "2", "");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider1, "scale", "log");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider1, "style", "knob");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider1, "tooltip", "Time constant in MICROseconds (1/e smoothing time) for the gate gain to go (exponentially) from 0 (muted) to 1 (unmuted)");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider1, "unit", "us");
  ui_interface->addHorizontalSlider(ui_interface->uiInterface, "Attack", &dsp->fHslider1, (FAUSTFLOAT)1e+01f, (FAUSTFLOAT)1e+01f, (FAUSTFLOAT)1e+04f, (FAUSTFLOAT)1.0f);
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider3, "3", "");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider3, "scale", "log");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider3, "style", "knob");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider3, "tooltip", "Time in ms to keep the gate open (no muting) after the signal level falls below the Threshold");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider3, "unit", "ms");
  ui_interface->addHorizontalSlider(ui_interface->uiInterface, "Hold", &dsp->fHslider3, (FAUSTFLOAT)2e+02f, (FAUSTFLOAT)1.0f, (FAUSTFLOAT)1e+03f, (FAUSTFLOAT)1.0f);
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider2, "4", "");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider2, "scale", "log");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider2, "style", "knob");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider2, "tooltip", "Time constant in ms (1/e smoothing time) for the gain to go (exponentially) from 1 (unmuted) to 0 (muted)");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider2, "unit", "ms");
  ui_interface->addHorizontalSlider(ui_interface->uiInterface, "Release", &dsp->fHslider2, (FAUSTFLOAT)1e+02f, (FAUSTFLOAT)1.0f, (FAUSTFLOAT)1e+03f, (FAUSTFLOAT)1.0f);
  ui_interface->closeBox(ui_interface->uiInterface);
  ui_interface->closeBox(ui_interface->uiInterface);
}
#endif


static void computemydsp(mydsp *dsp, int count, FAUSTFLOAT **RESTRICT inputs, FAUSTFLOAT **RESTRICT outputs)
{
  FAUSTFLOAT *input0 = inputs[0];
  FAUSTFLOAT *input1 = inputs[1];
  FAUSTFLOAT *output0 = outputs[0];
  FAUSTFLOAT *output1 = outputs[1];
  int iSlow0 = (int)((float)(dsp->fCheckbox0));
  float fSlow1 = powf(1e+01f, 0.05f * (float)(dsp->fHslider0));
  float fSlow2 = fmaxf(dsp->fConst1, 1e-06f * (float)(dsp->fHslider1));
  float fSlow3 = fmaxf(dsp->fConst1, 0.001f * (float)(dsp->fHslider2));
  float fSlow4 = fminf(fSlow2, fSlow3);
  int iSlow5 = fabsf(fSlow4) < 1.1920929e-07f;
  float fSlow6 = ((iSlow5) ? 0.0f : expf(0.0f - dsp->fConst1 / ((iSlow5) ? 1.0f : fSlow4)));
  float fSlow7 = 1.0f - fSlow6;
  int iSlow8 = (int)(dsp->fConst0 * fmaxf(dsp->fConst1, 0.001f * (float)(dsp->fHslider3)));
  int iSlow9 = fabsf(fSlow3) < 1.1920929e-07f;
  float fSlow10 = ((iSlow9) ? 0.0f : expf(0.0f - dsp->fConst1 / ((iSlow9) ? 1.0f : fSlow3)));
  int iSlow11 = fabsf(fSlow2) < 1.1920929e-07f;
  float fSlow12 = ((iSlow11) ? 0.0f : expf(0.0f - dsp->fConst1 / ((iSlow11) ? 1.0f : fSlow2)));
  /* C99 loop */
  {
    int i0;
    for (i0 = 0; i0 < count; i0 = i0 + 1)
    {
      float fTemp0 = (float)(input0[i0]);
      float fTemp1 = ((iSlow0) ? 0.0f : fTemp0);
      float fTemp2 = (float)(input1[i0]);
      float fTemp3 = ((iSlow0) ? 0.0f : fTemp2);
      float fTemp4 = fabsf(fTemp3);
      dsp->fRec1[0] = fabsf(fTemp4 + fabsf(fTemp1)) * fSlow7 + dsp->fRec1[1] * fSlow6;
      int iTemp5 = dsp->fRec1[0] > fSlow1;
      dsp->iVec0[0] = iTemp5;
      dsp->iRec2[0] = max(iSlow8 * (iTemp5 < dsp->iVec0[1]), dsp->iRec2[1] + -1);
      float fTemp6 = fabsf(fmaxf((float)(iTemp5), (float)(dsp->iRec2[0] > 0)));
      float fTemp7 = ((fTemp6 > dsp->fRec0[1]) ? fSlow12 : fSlow10);
      dsp->fRec0[0] = fTemp6 * (1.0f - fTemp7) + dsp->fRec0[1] * fTemp7;
      dsp->fHbargraph0 = (FAUSTFLOAT)(2e+01f * log10f(fmaxf(1.1754944e-38f, dsp->fRec0[0])));
      float fTemp8 = fTemp1;
      dsp->fRec4[0] = fSlow7 * fabsf(fTemp4 + fabsf(fTemp8)) + fSlow6 * dsp->fRec4[1];
      int iTemp9 = dsp->fRec4[0] > fSlow1;
      dsp->iVec1[0] = iTemp9;
      dsp->iRec5[0] = max(iSlow8 * (iTemp9 < dsp->iVec1[1]), dsp->iRec5[1] + -1);
      float fTemp10 = fabsf(fmaxf((float)(iTemp9), (float)(dsp->iRec5[0] > 0)));
      float fTemp11 = ((fTemp10 > dsp->fRec3[1]) ? fSlow12 : fSlow10);
      dsp->fRec3[0] = fTemp10 * (1.0f - fTemp11) + dsp->fRec3[1] * fTemp11;
      output0[i0] = (FAUSTFLOAT)(((iSlow0) ? fTemp0 : dsp->fRec3[0] * fTemp8));
      output1[i0] = (FAUSTFLOAT)(((iSlow0) ? fTemp2 : dsp->fRec3[0] * fTemp3));
      dsp->fRec1[1] = dsp->fRec1[0];
      dsp->iVec0[1] = dsp->iVec0[0];
      dsp->iRec2[1] = dsp->iRec2[0];
      dsp->fRec0[1] = dsp->fRec0[0];
      dsp->fRec4[1] = dsp->fRec4[0];
      dsp->iVec1[1] = dsp->iVec1[0];
      dsp->iRec5[1] = dsp->iRec5[0];
      dsp->fRec3[1] = dsp->fRec3[0];
    }
  }
}

#ifdef __cplusplus
}
#endif

#endif
