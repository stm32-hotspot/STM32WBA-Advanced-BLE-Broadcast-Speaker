/* ------------------------------------------------------------
author: "Jakob Zerbian"
name: "reverb_dattorro", "dattorro"
version: "0.1"
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
  int fSampleRate;
  float fConst0;
  float fConst1;
  FAUSTFLOAT fVslider0;
  float fRec4[2];
  FAUSTFLOAT fVslider1;
  float fRec5[2];
  FAUSTFLOAT fVslider2;
  float fRec9[2];
  int IOTA0;
  float fVec0[1024];
  float fRec7[2];
  float fVec1[8192];
  FAUSTFLOAT fVslider3;
  float fRec10[2];
  float fRec6[2];
  float fVec2[4096];
  float fRec2[2];
  float fVec3[4096];
  FAUSTFLOAT fVslider4;
  float fRec13[2];
  FAUSTFLOAT fVslider5;
  float fRec18[2];
  FAUSTFLOAT fVslider6;
  float fRec22[2];
  float fRec21[3];
  float fVec4[256];
  float fRec19[2];
  float fVec5[128];
  float fRec16[2];
  float fVec6[512];
  float fRec14[2];
  float fVec7[512];
  float fRec11[2];
  float fRec0[2];
  float fVec8[1024];
  float fRec26[2];
  float fVec9[8192];
  float fRec25[2];
  float fVec10[2048];
  float fRec23[2];
  float fVec11[2048];
  float fRec1[2];
  FAUSTFLOAT fVslider7;
  float fRec28[2];
  FAUSTFLOAT fVslider8;
  float fRec29[2];
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
  m->declare(m->metaInterface, "author", "Jakob Zerbian");
  m->declare(m->metaInterface, "basics.lib/name", "Faust Basic Element Library");
  m->declare(m->metaInterface, "basics.lib/tabulateNd", "Copyright (C) 2023 Bart Brouns <bart@magnetophon.nl>");
  m->declare(m->metaInterface, "basics.lib/version", "0.10");
  m->declare(m->metaInterface, "compile_options", "-lang c -ct 1 -es 1 -mcd 16 -single -ftz 0");
  m->declare(m->metaInterface, "demos.lib/dattorro_rev_demo:author", "Jakob Zerbian");
  m->declare(m->metaInterface, "demos.lib/dattorro_rev_demo:license", "MIT-style STK-4.3 license");
  m->declare(m->metaInterface, "demos.lib/name", "Faust Demos Library");
  m->declare(m->metaInterface, "demos.lib/version", "0.1");
  m->declare(m->metaInterface, "description", "Dattorro demo application.");
  m->declare(m->metaInterface, "filename", "reverb_dattorro.dsp");
  m->declare(m->metaInterface, "maths.lib/author", "GRAME");
  m->declare(m->metaInterface, "maths.lib/copyright", "GRAME");
  m->declare(m->metaInterface, "maths.lib/license", "LGPL with exception");
  m->declare(m->metaInterface, "maths.lib/name", "Faust Math Library");
  m->declare(m->metaInterface, "maths.lib/version", "2.6");
  m->declare(m->metaInterface, "name", "reverb_dattorro");
  m->declare(m->metaInterface, "platform.lib/name", "Generic Platform Library");
  m->declare(m->metaInterface, "platform.lib/version", "0.3");
  m->declare(m->metaInterface, "reverbs.lib/dattorro_rev:author", "Jakob Zerbian");
  m->declare(m->metaInterface, "reverbs.lib/dattorro_rev:licence", "MIT-style STK-4.3 license");
  m->declare(m->metaInterface, "reverbs.lib/name", "Faust Reverb Library");
  m->declare(m->metaInterface, "reverbs.lib/version", "0.2");
  m->declare(m->metaInterface, "routes.lib/name", "Faust Signal Routing Library");
  m->declare(m->metaInterface, "routes.lib/version", "0.2");
  m->declare(m->metaInterface, "signals.lib/name", "Faust Signal Routing Library");
  m->declare(m->metaInterface, "signals.lib/version", "0.3");
  m->declare(m->metaInterface, "version", "0.1");
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
  dsp->fVslider0 = (FAUSTFLOAT)(0.625f);
  dsp->fVslider1 = (FAUSTFLOAT)(0.7f);
  dsp->fVslider2 = (FAUSTFLOAT)(0.625f);
  dsp->fVslider3 = (FAUSTFLOAT)(0.625f);
  dsp->fVslider4 = (FAUSTFLOAT)(0.625f);
  dsp->fVslider5 = (FAUSTFLOAT)(0.625f);
  dsp->fVslider6 = (FAUSTFLOAT)(0.7f);
  dsp->fVslider7 = (FAUSTFLOAT)(0.0f);
  dsp->fVslider8 = (FAUSTFLOAT)(-6.0f);
}

static void instanceClearmydsp(mydsp *dsp)
{
  /* C99 loop */
  {
    int l0;
    for (l0 = 0; l0 < 2; l0 = l0 + 1)
    {
      dsp->fRec4[l0] = 0.0f;
    }
  }
  /* C99 loop */
  {
    int l1;
    for (l1 = 0; l1 < 2; l1 = l1 + 1)
    {
      dsp->fRec5[l1] = 0.0f;
    }
  }
  /* C99 loop */
  {
    int l2;
    for (l2 = 0; l2 < 2; l2 = l2 + 1)
    {
      dsp->fRec9[l2] = 0.0f;
    }
  }
  dsp->IOTA0 = 0;
  /* C99 loop */
  {
    int l3;
    for (l3 = 0; l3 < 1024; l3 = l3 + 1)
    {
      dsp->fVec0[l3] = 0.0f;
    }
  }
  /* C99 loop */
  {
    int l4;
    for (l4 = 0; l4 < 2; l4 = l4 + 1)
    {
      dsp->fRec7[l4] = 0.0f;
    }
  }
  /* C99 loop */
  {
    int l5;
    for (l5 = 0; l5 < 8192; l5 = l5 + 1)
    {
      dsp->fVec1[l5] = 0.0f;
    }
  }
  /* C99 loop */
  {
    int l6;
    for (l6 = 0; l6 < 2; l6 = l6 + 1)
    {
      dsp->fRec10[l6] = 0.0f;
    }
  }
  /* C99 loop */
  {
    int l7;
    for (l7 = 0; l7 < 2; l7 = l7 + 1)
    {
      dsp->fRec6[l7] = 0.0f;
    }
  }
  /* C99 loop */
  {
    int l8;
    for (l8 = 0; l8 < 4096; l8 = l8 + 1)
    {
      dsp->fVec2[l8] = 0.0f;
    }
  }
  /* C99 loop */
  {
    int l9;
    for (l9 = 0; l9 < 2; l9 = l9 + 1)
    {
      dsp->fRec2[l9] = 0.0f;
    }
  }
  /* C99 loop */
  {
    int l10;
    for (l10 = 0; l10 < 4096; l10 = l10 + 1)
    {
      dsp->fVec3[l10] = 0.0f;
    }
  }
  /* C99 loop */
  {
    int l11;
    for (l11 = 0; l11 < 2; l11 = l11 + 1)
    {
      dsp->fRec13[l11] = 0.0f;
    }
  }
  /* C99 loop */
  {
    int l12;
    for (l12 = 0; l12 < 2; l12 = l12 + 1)
    {
      dsp->fRec18[l12] = 0.0f;
    }
  }
  /* C99 loop */
  {
    int l13;
    for (l13 = 0; l13 < 2; l13 = l13 + 1)
    {
      dsp->fRec22[l13] = 0.0f;
    }
  }
  /* C99 loop */
  {
    int l14;
    for (l14 = 0; l14 < 3; l14 = l14 + 1)
    {
      dsp->fRec21[l14] = 0.0f;
    }
  }
  /* C99 loop */
  {
    int l15;
    for (l15 = 0; l15 < 256; l15 = l15 + 1)
    {
      dsp->fVec4[l15] = 0.0f;
    }
  }
  /* C99 loop */
  {
    int l16;
    for (l16 = 0; l16 < 2; l16 = l16 + 1)
    {
      dsp->fRec19[l16] = 0.0f;
    }
  }
  /* C99 loop */
  {
    int l17;
    for (l17 = 0; l17 < 128; l17 = l17 + 1)
    {
      dsp->fVec5[l17] = 0.0f;
    }
  }
  /* C99 loop */
  {
    int l18;
    for (l18 = 0; l18 < 2; l18 = l18 + 1)
    {
      dsp->fRec16[l18] = 0.0f;
    }
  }
  /* C99 loop */
  {
    int l19;
    for (l19 = 0; l19 < 512; l19 = l19 + 1)
    {
      dsp->fVec6[l19] = 0.0f;
    }
  }
  /* C99 loop */
  {
    int l20;
    for (l20 = 0; l20 < 2; l20 = l20 + 1)
    {
      dsp->fRec14[l20] = 0.0f;
    }
  }
  /* C99 loop */
  {
    int l21;
    for (l21 = 0; l21 < 512; l21 = l21 + 1)
    {
      dsp->fVec7[l21] = 0.0f;
    }
  }
  /* C99 loop */
  {
    int l22;
    for (l22 = 0; l22 < 2; l22 = l22 + 1)
    {
      dsp->fRec11[l22] = 0.0f;
    }
  }
  /* C99 loop */
  {
    int l23;
    for (l23 = 0; l23 < 2; l23 = l23 + 1)
    {
      dsp->fRec0[l23] = 0.0f;
    }
  }
  /* C99 loop */
  {
    int l24;
    for (l24 = 0; l24 < 1024; l24 = l24 + 1)
    {
      dsp->fVec8[l24] = 0.0f;
    }
  }
  /* C99 loop */
  {
    int l25;
    for (l25 = 0; l25 < 2; l25 = l25 + 1)
    {
      dsp->fRec26[l25] = 0.0f;
    }
  }
  /* C99 loop */
  {
    int l26;
    for (l26 = 0; l26 < 8192; l26 = l26 + 1)
    {
      dsp->fVec9[l26] = 0.0f;
    }
  }
  /* C99 loop */
  {
    int l27;
    for (l27 = 0; l27 < 2; l27 = l27 + 1)
    {
      dsp->fRec25[l27] = 0.0f;
    }
  }
  /* C99 loop */
  {
    int l28;
    for (l28 = 0; l28 < 2048; l28 = l28 + 1)
    {
      dsp->fVec10[l28] = 0.0f;
    }
  }
  /* C99 loop */
  {
    int l29;
    for (l29 = 0; l29 < 2; l29 = l29 + 1)
    {
      dsp->fRec23[l29] = 0.0f;
    }
  }
  /* C99 loop */
  {
    int l30;
    for (l30 = 0; l30 < 2048; l30 = l30 + 1)
    {
      dsp->fVec11[l30] = 0.0f;
    }
  }
  /* C99 loop */
  {
    int l31;
    for (l31 = 0; l31 < 2; l31 = l31 + 1)
    {
      dsp->fRec1[l31] = 0.0f;
    }
  }
  /* C99 loop */
  {
    int l32;
    for (l32 = 0; l32 < 2; l32 = l32 + 1)
    {
      dsp->fRec28[l32] = 0.0f;
    }
  }
  /* C99 loop */
  {
    int l33;
    for (l33 = 0; l33 < 2; l33 = l33 + 1)
    {
      dsp->fRec29[l33] = 0.0f;
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
  ui_interface->declare(ui_interface->uiInterface, 0, "0", "");
  ui_interface->openHorizontalBox(ui_interface->uiInterface, "Dattorro Reverb");
  ui_interface->declare(ui_interface->uiInterface, 0, "0", "");
  ui_interface->openHorizontalBox(ui_interface->uiInterface, "Input");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fVslider6, "1", "");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fVslider6, "style", "knob");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fVslider6, "tooltip", "lowpass-like filter, 0 = no signal, 1 = no filtering");
  ui_interface->addVerticalSlider(ui_interface->uiInterface, "Prefilter", &dsp->fVslider6, (FAUSTFLOAT)0.7f, (FAUSTFLOAT)0.0f, (FAUSTFLOAT)1.0f, (FAUSTFLOAT)0.001f);
  ui_interface->declare(ui_interface->uiInterface, &dsp->fVslider5, "2", "");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fVslider5, "style", "knob");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fVslider5, "tooltip", "diffusion factor, influences reverb color and density");
  ui_interface->addVerticalSlider(ui_interface->uiInterface, "Diffusion 1", &dsp->fVslider5, (FAUSTFLOAT)0.625f, (FAUSTFLOAT)0.0f, (FAUSTFLOAT)1.0f, (FAUSTFLOAT)0.001f);
  ui_interface->declare(ui_interface->uiInterface, &dsp->fVslider4, "3", "");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fVslider4, "style", "knob");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fVslider4, "tooltip", "diffusion factor, influences reverb color and density");
  ui_interface->addVerticalSlider(ui_interface->uiInterface, "Diffusion 2", &dsp->fVslider4, (FAUSTFLOAT)0.625f, (FAUSTFLOAT)0.0f, (FAUSTFLOAT)1.0f, (FAUSTFLOAT)0.001f);
  ui_interface->closeBox(ui_interface->uiInterface);
  ui_interface->declare(ui_interface->uiInterface, 0, "1", "");
  ui_interface->openHorizontalBox(ui_interface->uiInterface, "Feedback");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fVslider2, "1", "");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fVslider2, "style", "knob");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fVslider2, "tooltip", "diffusion factor, influences reverb color and density");
  ui_interface->addVerticalSlider(ui_interface->uiInterface, "Diffusion 1", &dsp->fVslider2, (FAUSTFLOAT)0.625f, (FAUSTFLOAT)0.0f, (FAUSTFLOAT)1.0f, (FAUSTFLOAT)0.001f);
  ui_interface->declare(ui_interface->uiInterface, &dsp->fVslider0, "2", "");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fVslider0, "style", "knob");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fVslider0, "tooltip", "diffusion factor, influences reverb color and density");
  ui_interface->addVerticalSlider(ui_interface->uiInterface, "Diffusion 2", &dsp->fVslider0, (FAUSTFLOAT)0.625f, (FAUSTFLOAT)0.0f, (FAUSTFLOAT)1.0f, (FAUSTFLOAT)0.001f);
  ui_interface->declare(ui_interface->uiInterface, &dsp->fVslider1, "3", "");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fVslider1, "style", "knob");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fVslider1, "tooltip", "decay length, 1 = infinite");
  ui_interface->addVerticalSlider(ui_interface->uiInterface, "Decay Rate", &dsp->fVslider1, (FAUSTFLOAT)0.7f, (FAUSTFLOAT)0.0f, (FAUSTFLOAT)1.0f, (FAUSTFLOAT)0.001f);
  ui_interface->declare(ui_interface->uiInterface, &dsp->fVslider3, "4", "");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fVslider3, "style", "knob");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fVslider3, "tooltip", "dampening in feedback network");
  ui_interface->addVerticalSlider(ui_interface->uiInterface, "Damping", &dsp->fVslider3, (FAUSTFLOAT)0.625f, (FAUSTFLOAT)0.0f, (FAUSTFLOAT)1.0f, (FAUSTFLOAT)0.001f);
  ui_interface->closeBox(ui_interface->uiInterface);
  ui_interface->declare(ui_interface->uiInterface, 0, "2", "");
  ui_interface->openHorizontalBox(ui_interface->uiInterface, "Output");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fVslider7, "1", "");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fVslider7, "style", "knob");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fVslider7, "tooltip", "-1 = dry, 1 = wet");
  ui_interface->addVerticalSlider(ui_interface->uiInterface, "Dry/Wet Mix", &dsp->fVslider7, (FAUSTFLOAT)0.0f, (FAUSTFLOAT) - 1.0f, (FAUSTFLOAT)1.0f, (FAUSTFLOAT)0.01f);
  ui_interface->declare(ui_interface->uiInterface, &dsp->fVslider8, "2", "");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fVslider8, "style", "knob");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fVslider8, "tooltip", "Output Gain");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fVslider8, "unit", "dB");
  ui_interface->addVerticalSlider(ui_interface->uiInterface, "Level", &dsp->fVslider8, (FAUSTFLOAT) - 6.0f, (FAUSTFLOAT) - 7e+01f, (FAUSTFLOAT)4e+01f, (FAUSTFLOAT)0.1f);
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
  float fSlow0 = dsp->fConst0 * (float)(dsp->fVslider0);
  float fSlow1 = dsp->fConst0 * (float)(dsp->fVslider1);
  float fSlow2 = dsp->fConst0 * (float)(dsp->fVslider2);
  float fSlow3 = dsp->fConst0 * (float)(dsp->fVslider3);
  float fSlow4 = dsp->fConst0 * (float)(dsp->fVslider4);
  float fSlow5 = dsp->fConst0 * (float)(dsp->fVslider5);
  float fSlow6 = dsp->fConst0 * (float)(dsp->fVslider6);
  float fSlow7 = dsp->fConst0 * (float)(dsp->fVslider7);
  float fSlow8 = dsp->fConst0 * powf(1e+01f, 0.05f * (float)(dsp->fVslider8));
  /* C99 loop */
  {
    int i0;
    for (i0 = 0; i0 < count; i0 = i0 + 1)
    {
      dsp->fRec4[0] = fSlow0 + dsp->fConst1 * dsp->fRec4[1];
      dsp->fRec5[0] = fSlow1 + dsp->fConst1 * dsp->fRec5[1];
      dsp->fRec9[0] = fSlow2 + dsp->fConst1 * dsp->fRec9[1];
      float fTemp0 = dsp->fRec9[0] * dsp->fRec7[1] + dsp->fRec1[1];
      dsp->fVec0[dsp->IOTA0 & 1023] = fTemp0;
      dsp->fRec7[0] = dsp->fVec0[(dsp->IOTA0 - 908) & 1023];
      float fRec8 = 0.0f - dsp->fRec9[0] * fTemp0;
      dsp->fVec1[dsp->IOTA0 & 8191] = fRec8 + dsp->fRec7[1];
      dsp->fRec10[0] = fSlow3 + dsp->fConst1 * dsp->fRec10[1];
      float fTemp1 = 1.0f - dsp->fRec10[0];
      dsp->fRec6[0] = dsp->fRec10[0] * dsp->fRec6[1] + fTemp1 * dsp->fVec1[(dsp->IOTA0 - 4217) & 8191];
      float fTemp2 = dsp->fRec6[0] * dsp->fRec5[0] - dsp->fRec4[0] * dsp->fRec2[1];
      dsp->fVec2[dsp->IOTA0 & 4095] = fTemp2;
      dsp->fRec2[0] = dsp->fVec2[(dsp->IOTA0 - 2656) & 4095];
      float fRec3 = dsp->fRec4[0] * fTemp2;
      dsp->fVec3[dsp->IOTA0 & 4095] = fRec3 + dsp->fRec2[1];
      dsp->fRec13[0] = fSlow4 + dsp->fConst1 * dsp->fRec13[1];
      dsp->fRec18[0] = fSlow5 + dsp->fConst1 * dsp->fRec18[1];
      dsp->fRec22[0] = fSlow6 + dsp->fConst1 * dsp->fRec22[1];
      float fTemp3 = (float)(input1[i0]);
      float fTemp4 = (float)(input0[i0]);
      dsp->fRec21[0] = (1.0f - dsp->fRec22[0]) * dsp->fRec21[2] + 0.5f * (fTemp4 + fTemp3) * dsp->fRec22[0];
      float fTemp5 = dsp->fRec21[0] - dsp->fRec18[0] * dsp->fRec19[1];
      dsp->fVec4[dsp->IOTA0 & 255] = fTemp5;
      dsp->fRec19[0] = dsp->fVec4[(dsp->IOTA0 - 142) & 255];
      float fRec20 = dsp->fRec18[0] * fTemp5;
      float fTemp6 = fRec20 + dsp->fRec19[1] - dsp->fRec18[0] * dsp->fRec16[1];
      dsp->fVec5[dsp->IOTA0 & 127] = fTemp6;
      dsp->fRec16[0] = dsp->fVec5[(dsp->IOTA0 - 107) & 127];
      float fRec17 = dsp->fRec18[0] * fTemp6;
      float fTemp7 = fRec17 + dsp->fRec16[1] - dsp->fRec13[0] * dsp->fRec14[1];
      dsp->fVec6[dsp->IOTA0 & 511] = fTemp7;
      dsp->fRec14[0] = dsp->fVec6[(dsp->IOTA0 - 379) & 511];
      float fRec15 = dsp->fRec13[0] * fTemp7;
      float fTemp8 = fRec15 + dsp->fRec14[1] - dsp->fRec13[0] * dsp->fRec11[1];
      dsp->fVec7[dsp->IOTA0 & 511] = fTemp8;
      dsp->fRec11[0] = dsp->fVec7[(dsp->IOTA0 - 277) & 511];
      float fRec12 = dsp->fRec13[0] * fTemp8;
      dsp->fRec0[0] = dsp->fRec11[1] + fRec12 + dsp->fRec5[0] * dsp->fVec3[(dsp->IOTA0 - 2656) & 4095];
      float fTemp9 = dsp->fRec9[0] * dsp->fRec26[1] + dsp->fRec0[1];
      dsp->fVec8[dsp->IOTA0 & 1023] = fTemp9;
      dsp->fRec26[0] = dsp->fVec8[(dsp->IOTA0 - 672) & 1023];
      float fRec27 = 0.0f - dsp->fRec9[0] * fTemp9;
      dsp->fVec9[dsp->IOTA0 & 8191] = fRec27 + dsp->fRec26[1];
      dsp->fRec25[0] = dsp->fRec10[0] * dsp->fRec25[1] + fTemp1 * dsp->fVec9[(dsp->IOTA0 - 4453) & 8191];
      float fTemp10 = dsp->fRec5[0] * dsp->fRec25[0] - dsp->fRec4[0] * dsp->fRec23[1];
      dsp->fVec10[dsp->IOTA0 & 2047] = fTemp10;
      dsp->fRec23[0] = dsp->fVec10[(dsp->IOTA0 - 1800) & 2047];
      float fRec24 = dsp->fRec4[0] * fTemp10;
      dsp->fVec11[dsp->IOTA0 & 2047] = fRec24 + dsp->fRec23[1];
      dsp->fRec1[0] = fRec12 + dsp->fRec11[1] + dsp->fRec5[0] * dsp->fVec11[(dsp->IOTA0 - 1800) & 2047];
      dsp->fRec28[0] = fSlow7 + dsp->fConst1 * dsp->fRec28[1];
      float fTemp11 = dsp->fRec28[0] + 1.0f;
      float fTemp12 = 1.0f - 0.5f * fTemp11;
      dsp->fRec29[0] = fSlow8 + dsp->fConst1 * dsp->fRec29[1];
      output0[i0] = (FAUSTFLOAT)(dsp->fRec29[0] * (fTemp4 * fTemp12 + 0.5f * fTemp11 * dsp->fRec0[0]));
      output1[i0] = (FAUSTFLOAT)(dsp->fRec29[0] * (fTemp3 * fTemp12 + 0.5f * fTemp11 * dsp->fRec1[0]));
      dsp->fRec4[1] = dsp->fRec4[0];
      dsp->fRec5[1] = dsp->fRec5[0];
      dsp->fRec9[1] = dsp->fRec9[0];
      dsp->IOTA0 = dsp->IOTA0 + 1;
      dsp->fRec7[1] = dsp->fRec7[0];
      dsp->fRec10[1] = dsp->fRec10[0];
      dsp->fRec6[1] = dsp->fRec6[0];
      dsp->fRec2[1] = dsp->fRec2[0];
      dsp->fRec13[1] = dsp->fRec13[0];
      dsp->fRec18[1] = dsp->fRec18[0];
      dsp->fRec22[1] = dsp->fRec22[0];
      dsp->fRec21[2] = dsp->fRec21[1];
      dsp->fRec21[1] = dsp->fRec21[0];
      dsp->fRec19[1] = dsp->fRec19[0];
      dsp->fRec16[1] = dsp->fRec16[0];
      dsp->fRec14[1] = dsp->fRec14[0];
      dsp->fRec11[1] = dsp->fRec11[0];
      dsp->fRec0[1] = dsp->fRec0[0];
      dsp->fRec26[1] = dsp->fRec26[0];
      dsp->fRec25[1] = dsp->fRec25[0];
      dsp->fRec23[1] = dsp->fRec23[0];
      dsp->fRec1[1] = dsp->fRec1[0];
      dsp->fRec28[1] = dsp->fRec28[0];
      dsp->fRec29[1] = dsp->fRec29[0];
    }
  }
}

#ifdef __cplusplus
}
#endif

#endif
