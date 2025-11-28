/* ------------------------------------------------------------
author: "JOS, revised by RM"
name: "faust_phaser", "phaser"
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

static float mydsp_faustpower4_f(float value)
{
  return value * value * value * value;
}
static float mydsp_faustpower3_f(float value)
{
  return value * value * value;
}
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
  FAUSTFLOAT fCheckbox1;
  FAUSTFLOAT fCheckbox2;
  FAUSTFLOAT fHslider0;
  int iVec0[2];
  FAUSTFLOAT fHslider1;
  int fSampleRate;
  float fConst1;
  float fRec2[2];
  float fRec3[2];
  FAUSTFLOAT fHslider2;
  FAUSTFLOAT fHslider3;
  FAUSTFLOAT fHslider4;
  FAUSTFLOAT fHslider5;
  float fConst2;
  FAUSTFLOAT fHslider6;
  FAUSTFLOAT fHslider7;
  float fRec6[3];
  float fRec5[3];
  float fRec4[3];
  float fRec1[3];
  float fRec0[2];
  float fRec11[3];
  float fRec10[3];
  float fRec9[3];
  float fRec8[3];
  float fRec7[2];
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

#ifdef ALGO_FULL_BUILD
void metadatamydsp(MetaGlue *m)
{
  m->declare(m->metaInterface, "author", "JOS, revised by RM");
  m->declare(m->metaInterface, "basics.lib/bypass2:author", "Julius Smith");
  m->declare(m->metaInterface, "basics.lib/name", "Faust Basic Element Library");
  m->declare(m->metaInterface, "basics.lib/tabulateNd", "Copyright (C) 2023 Bart Brouns <bart@magnetophon.nl>");
  m->declare(m->metaInterface, "basics.lib/version", "0.10");
  m->declare(m->metaInterface, "compile_options", "-lang c -ct 1 -es 1 -mcd 16 -single -ftz 0");
  m->declare(m->metaInterface, "demos.lib/name", "Faust Demos Library");
  m->declare(m->metaInterface, "demos.lib/phaser2_demo:author", "Julius O. Smith III");
  m->declare(m->metaInterface, "demos.lib/phaser2_demo:licence", "MIT");
  m->declare(m->metaInterface, "demos.lib/version", "0.1");
  m->declare(m->metaInterface, "description", "Phaser demo application.");
  m->declare(m->metaInterface, "filename", "faust_phaser.dsp");
  m->declare(m->metaInterface, "filters.lib/fir:author", "Julius O. Smith III");
  m->declare(m->metaInterface, "filters.lib/fir:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
  m->declare(m->metaInterface, "filters.lib/fir:license", "MIT-style STK-4.3 license");
  m->declare(m->metaInterface, "filters.lib/iir:author", "Julius O. Smith III");
  m->declare(m->metaInterface, "filters.lib/iir:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
  m->declare(m->metaInterface, "filters.lib/iir:license", "MIT-style STK-4.3 license");
  m->declare(m->metaInterface, "filters.lib/lowpass0_highpass1", "MIT-style STK-4.3 license");
  m->declare(m->metaInterface, "filters.lib/name", "Faust Filters Library");
  m->declare(m->metaInterface, "filters.lib/nlf2:author", "Julius O. Smith III");
  m->declare(m->metaInterface, "filters.lib/nlf2:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
  m->declare(m->metaInterface, "filters.lib/nlf2:license", "MIT-style STK-4.3 license");
  m->declare(m->metaInterface, "filters.lib/tf2:author", "Julius O. Smith III");
  m->declare(m->metaInterface, "filters.lib/tf2:copyright", "Copyright (C) 2003-2019 by Julius O. Smith III <jos@ccrma.stanford.edu>");
  m->declare(m->metaInterface, "filters.lib/tf2:license", "MIT-style STK-4.3 license");
  m->declare(m->metaInterface, "filters.lib/version", "0.3");
  m->declare(m->metaInterface, "maths.lib/author", "GRAME");
  m->declare(m->metaInterface, "maths.lib/copyright", "GRAME");
  m->declare(m->metaInterface, "maths.lib/license", "LGPL with exception");
  m->declare(m->metaInterface, "maths.lib/name", "Faust Math Library");
  m->declare(m->metaInterface, "maths.lib/version", "2.6");
  m->declare(m->metaInterface, "name", "faust_phaser");
  m->declare(m->metaInterface, "oscillators.lib/name", "Faust Oscillator Library");
  m->declare(m->metaInterface, "oscillators.lib/version", "0.4");
  m->declare(m->metaInterface, "phaflangers.lib/name", "Faust Phaser and Flanger Library");
  m->declare(m->metaInterface, "phaflangers.lib/version", "0.1");
  m->declare(m->metaInterface, "platform.lib/name", "Generic Platform Library");
  m->declare(m->metaInterface, "platform.lib/version", "0.3");
  m->declare(m->metaInterface, "routes.lib/name", "Faust Signal Routing Library");
  m->declare(m->metaInterface, "routes.lib/version", "0.2");
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
  dsp->fCheckbox1 = (FAUSTFLOAT)(0.0f);
  dsp->fCheckbox2 = (FAUSTFLOAT)(0.0f);
  dsp->fHslider0 = (FAUSTFLOAT)(1.0f);
  dsp->fHslider1 = (FAUSTFLOAT)(0.5f);
  dsp->fHslider2 = (FAUSTFLOAT)(1e+02f);
  dsp->fHslider3 = (FAUSTFLOAT)(8e+02f);
  dsp->fHslider4 = (FAUSTFLOAT)(1.5f);
  dsp->fHslider5 = (FAUSTFLOAT)(1e+03f);
  dsp->fHslider6 = (FAUSTFLOAT)(0.0f);
  dsp->fHslider7 = (FAUSTFLOAT)(0.0f);
}

static void instanceClearmydsp(mydsp *dsp)
{
  /* C99 loop */
  {
    int l0;
    for (l0 = 0; l0 < 2; l0 = l0 + 1)
    {
      dsp->iVec0[l0] = 0;
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
      dsp->fRec3[l2] = 0.0f;
    }
  }
  /* C99 loop */
  {
    int l3;
    for (l3 = 0; l3 < 3; l3 = l3 + 1)
    {
      dsp->fRec6[l3] = 0.0f;
    }
  }
  /* C99 loop */
  {
    int l4;
    for (l4 = 0; l4 < 3; l4 = l4 + 1)
    {
      dsp->fRec5[l4] = 0.0f;
    }
  }
  /* C99 loop */
  {
    int l5;
    for (l5 = 0; l5 < 3; l5 = l5 + 1)
    {
      dsp->fRec4[l5] = 0.0f;
    }
  }
  /* C99 loop */
  {
    int l6;
    for (l6 = 0; l6 < 3; l6 = l6 + 1)
    {
      dsp->fRec1[l6] = 0.0f;
    }
  }
  /* C99 loop */
  {
    int l7;
    for (l7 = 0; l7 < 2; l7 = l7 + 1)
    {
      dsp->fRec0[l7] = 0.0f;
    }
  }
  /* C99 loop */
  {
    int l8;
    for (l8 = 0; l8 < 3; l8 = l8 + 1)
    {
      dsp->fRec11[l8] = 0.0f;
    }
  }
  /* C99 loop */
  {
    int l9;
    for (l9 = 0; l9 < 3; l9 = l9 + 1)
    {
      dsp->fRec10[l9] = 0.0f;
    }
  }
  /* C99 loop */
  {
    int l10;
    for (l10 = 0; l10 < 3; l10 = l10 + 1)
    {
      dsp->fRec9[l10] = 0.0f;
    }
  }
  /* C99 loop */
  {
    int l11;
    for (l11 = 0; l11 < 3; l11 = l11 + 1)
    {
      dsp->fRec8[l11] = 0.0f;
    }
  }
  /* C99 loop */
  {
    int l12;
    for (l12 = 0; l12 < 2; l12 = l12 + 1)
    {
      dsp->fRec7[l12] = 0.0f;
    }
  }
}

static void instanceConstantsmydsp(mydsp *dsp, int sample_rate)
{
  dsp->fSampleRate = sample_rate;
  float fConst0 = fminf(1.92e+05f, fmaxf(1.0f, (float)(dsp->fSampleRate)));
  dsp->fConst1 = 6.2831855f / fConst0;
  dsp->fConst2 = 1.0f / fConst0;
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
#ifdef ALGO_FULL_BUILD
static void buildUserInterfacemydsp(mydsp *dsp, UIGlue *ui_interface)
{
  ui_interface->declare(ui_interface->uiInterface, 0, "tooltip", "Reference: https://ccrma.stanford.edu/~jos/pasp/Flanging.html");
  ui_interface->openVerticalBox(ui_interface->uiInterface, "PHASER2");
  ui_interface->declare(ui_interface->uiInterface, 0, "0", "");
  ui_interface->openHorizontalBox(ui_interface->uiInterface, "0x00");

  ui_interface->declare(ui_interface->uiInterface, &dsp->fCheckbox0, "0", "");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fCheckbox0, "tooltip", "When this is checked, the phaser has no effect");
  ui_interface->addCheckButton(ui_interface->uiInterface, "Bypass", &dsp->fCheckbox0);

  ui_interface->declare(ui_interface->uiInterface, &dsp->fCheckbox1, "1", "");
  ui_interface->addCheckButton(ui_interface->uiInterface, "Invert Internal Phaser Sum", &dsp->fCheckbox1);

  ui_interface->declare(ui_interface->uiInterface, &dsp->fCheckbox2, "2", "");
  ui_interface->addCheckButton(ui_interface->uiInterface, "Vibrato Mode", &dsp->fCheckbox2);

  ui_interface->closeBox(ui_interface->uiInterface);

  ui_interface->declare(ui_interface->uiInterface, 0, "1", "");

  ui_interface->openHorizontalBox(ui_interface->uiInterface, "0x00");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider1, "1", "");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider1, "style", "knob");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider1, "unit", "Hz");
  ui_interface->addHorizontalSlider(ui_interface->uiInterface, "Speed", &dsp->fHslider1, (FAUSTFLOAT)0.5f, (FAUSTFLOAT)0.0f, (FAUSTFLOAT)1e+01f, (FAUSTFLOAT)0.001f);

  ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider0, "2", "");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider0, "style", "knob");
  ui_interface->addHorizontalSlider(ui_interface->uiInterface, "Notch Depth (Intensity)", &dsp->fHslider0, (FAUSTFLOAT)1.0f, (FAUSTFLOAT)0.0f, (FAUSTFLOAT)1.0f, (FAUSTFLOAT)0.001f);

  ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider6, "3", "");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider6, "style", "knob");
  ui_interface->addHorizontalSlider(ui_interface->uiInterface, "Feedback Gain", &dsp->fHslider6, (FAUSTFLOAT)0.0f, (FAUSTFLOAT) - 0.999f, (FAUSTFLOAT)0.999f, (FAUSTFLOAT)0.001f);
  ui_interface->closeBox(ui_interface->uiInterface);

  ui_interface->declare(ui_interface->uiInterface, 0, "2", "");

  ui_interface->openHorizontalBox(ui_interface->uiInterface, "0x00");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider5, "1", "");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider5, "scale", "log");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider5, "style", "knob");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider5, "unit", "Hz");
  ui_interface->addHorizontalSlider(ui_interface->uiInterface, "Notch width", &dsp->fHslider5, (FAUSTFLOAT)1e+03f, (FAUSTFLOAT)1e+01f, (FAUSTFLOAT)5e+03f, (FAUSTFLOAT)1.0f);

  ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider2, "2", "");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider2, "scale", "log");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider2, "style", "knob");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider2, "unit", "Hz");
  ui_interface->addHorizontalSlider(ui_interface->uiInterface, "Min Notch1 Freq", &dsp->fHslider2, (FAUSTFLOAT)1e+02f, (FAUSTFLOAT)2e+01f, (FAUSTFLOAT)5e+03f, (FAUSTFLOAT)1.0f);

  ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider3, "3", "");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider3, "scale", "log");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider3, "style", "knob");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider3, "unit", "Hz");
  ui_interface->addHorizontalSlider(ui_interface->uiInterface, "Max Notch1 Freq", &dsp->fHslider3, (FAUSTFLOAT)8e+02f, (FAUSTFLOAT)2e+01f, (FAUSTFLOAT)1e+04f, (FAUSTFLOAT)1.0f);

  ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider4, "4", "");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider4, "style", "knob");
  ui_interface->addHorizontalSlider(ui_interface->uiInterface, "Notch Freq Ratio: NotchFreq(n+1)/NotchFreq(n)", &dsp->fHslider4, (FAUSTFLOAT)1.5f, (FAUSTFLOAT)1.1f, (FAUSTFLOAT)4.0f, (FAUSTFLOAT)0.001f);
  ui_interface->closeBox(ui_interface->uiInterface);

  ui_interface->declare(ui_interface->uiInterface, 0, "3", "");
  ui_interface->openHorizontalBox(ui_interface->uiInterface, "0x00");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider7, "unit", "dB");
  ui_interface->addHorizontalSlider(ui_interface->uiInterface, "Phaser Output Level", &dsp->fHslider7, (FAUSTFLOAT)0.0f, (FAUSTFLOAT) - 6e+01f, (FAUSTFLOAT)1e+01f, (FAUSTFLOAT)0.1f);
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
  float fSlow1 = 0.5f * (((int)((float)(dsp->fCheckbox2))) ? 2.0f : (float)(dsp->fHslider0));
  float fSlow2 = (((int)((float)(dsp->fCheckbox1))) ? -1.0f * fSlow1 : fSlow1);
  float fSlow3 = dsp->fConst1 * (float)(dsp->fHslider1);
  float fSlow4 = cosf(fSlow3);
  float fSlow5 = sinf(fSlow3);
  float fSlow6 = (float)(dsp->fHslider2);
  float fSlow7 = 0.5f * (0.0f - dsp->fConst1 * (fSlow6 - fmaxf(fSlow6, (float)(dsp->fHslider3))));
  float fSlow8 = dsp->fConst1 * fSlow6;
  float fSlow9 = (float)(dsp->fHslider4);
  float fSlow10 = mydsp_faustpower4_f(fSlow9);
  float fSlow11 = mydsp_faustpower3_f(fSlow9);
  float fSlow12 = mydsp_faustpower2_f(fSlow9);
  float fSlow13 = expf(dsp->fConst2 * (0.0f - 3.1415927f * (float)(dsp->fHslider5)));
  float fSlow14 = mydsp_faustpower2_f(fSlow13);
  float fSlow15 = 0.0f - 2.0f * fSlow13;
  float fSlow16 = (float)(dsp->fHslider6);
  float fSlow17 = powf(1e+01f, 0.05f * (float)(dsp->fHslider7));
  float fSlow18 = 1.0f - fSlow1;
  /* C99 loop */
  {
    int i0;
    for (i0 = 0; i0 < count; i0 = i0 + 1)
    {
      dsp->iVec0[0] = 1;
      dsp->fRec2[0] = fSlow5 * dsp->fRec3[1] + fSlow4 * dsp->fRec2[1];
      dsp->fRec3[0] = (float)(1 - dsp->iVec0[1]) + fSlow4 * dsp->fRec3[1] - fSlow5 * dsp->fRec2[1];
      float fTemp0 = fSlow8 + fSlow7 * (1.0f - dsp->fRec2[0]);
      float fTemp1 = dsp->fRec1[1] * cosf(fSlow10 * fTemp0);
      float fTemp2 = dsp->fRec4[1] * cosf(fSlow11 * fTemp0);
      float fTemp3 = dsp->fRec5[1] * cosf(fSlow12 * fTemp0);
      float fTemp4 = dsp->fRec6[1] * cosf(fSlow9 * fTemp0);
      float fTemp5 = (float)(input0[i0]);
      float fTemp6 = ((iSlow0) ? 0.0f : fTemp5);
      dsp->fRec6[0] = fSlow17 * fTemp6 + fSlow16 * dsp->fRec0[1] - (fSlow15 * fTemp4 + fSlow14 * dsp->fRec6[2]);
      dsp->fRec5[0] = fSlow14 * (dsp->fRec6[0] - dsp->fRec5[2]) + dsp->fRec6[2] + fSlow15 * (fTemp4 - fTemp3);
      dsp->fRec4[0] = fSlow14 * (dsp->fRec5[0] - dsp->fRec4[2]) + dsp->fRec5[2] + fSlow15 * (fTemp3 - fTemp2);
      dsp->fRec1[0] = fSlow14 * (dsp->fRec4[0] - dsp->fRec1[2]) + dsp->fRec4[2] + fSlow15 * (fTemp2 - fTemp1);
      dsp->fRec0[0] = fSlow14 * dsp->fRec1[0] + fSlow15 * fTemp1 + dsp->fRec1[2];
      output0[i0] = (FAUSTFLOAT)(((iSlow0) ? fTemp5 : fSlow17 * fTemp6 * fSlow18 + dsp->fRec0[0] * fSlow2));
      float fTemp7 = fSlow8 + fSlow7 * (1.0f - dsp->fRec3[0]);
      float fTemp8 = dsp->fRec8[1] * cosf(fSlow10 * fTemp7);
      float fTemp9 = dsp->fRec9[1] * cosf(fSlow11 * fTemp7);
      float fTemp10 = dsp->fRec10[1] * cosf(fSlow12 * fTemp7);
      float fTemp11 = dsp->fRec11[1] * cosf(fSlow9 * fTemp7);
      float fTemp12 = (float)(input1[i0]);
      float fTemp13 = ((iSlow0) ? 0.0f : fTemp12);
      dsp->fRec11[0] = fSlow17 * fTemp13 + fSlow16 * dsp->fRec7[1] - (fSlow15 * fTemp11 + fSlow14 * dsp->fRec11[2]);
      dsp->fRec10[0] = fSlow14 * (dsp->fRec11[0] - dsp->fRec10[2]) + dsp->fRec11[2] + fSlow15 * (fTemp11 - fTemp10);
      dsp->fRec9[0] = fSlow14 * (dsp->fRec10[0] - dsp->fRec9[2]) + dsp->fRec10[2] + fSlow15 * (fTemp10 - fTemp9);
      dsp->fRec8[0] = fSlow14 * (dsp->fRec9[0] - dsp->fRec8[2]) + dsp->fRec9[2] + fSlow15 * (fTemp9 - fTemp8);
      dsp->fRec7[0] = fSlow14 * dsp->fRec8[0] + fSlow15 * fTemp8 + dsp->fRec8[2];
      output1[i0] = (FAUSTFLOAT)(((iSlow0) ? fTemp12 : fSlow17 * fTemp13 * fSlow18 + dsp->fRec7[0] * fSlow2));
      dsp->iVec0[1] = dsp->iVec0[0];
      dsp->fRec2[1] = dsp->fRec2[0];
      dsp->fRec3[1] = dsp->fRec3[0];
      dsp->fRec6[2] = dsp->fRec6[1];
      dsp->fRec6[1] = dsp->fRec6[0];
      dsp->fRec5[2] = dsp->fRec5[1];
      dsp->fRec5[1] = dsp->fRec5[0];
      dsp->fRec4[2] = dsp->fRec4[1];
      dsp->fRec4[1] = dsp->fRec4[0];
      dsp->fRec1[2] = dsp->fRec1[1];
      dsp->fRec1[1] = dsp->fRec1[0];
      dsp->fRec0[1] = dsp->fRec0[0];
      dsp->fRec11[2] = dsp->fRec11[1];
      dsp->fRec11[1] = dsp->fRec11[0];
      dsp->fRec10[2] = dsp->fRec10[1];
      dsp->fRec10[1] = dsp->fRec10[0];
      dsp->fRec9[2] = dsp->fRec9[1];
      dsp->fRec9[1] = dsp->fRec9[0];
      dsp->fRec8[2] = dsp->fRec8[1];
      dsp->fRec8[1] = dsp->fRec8[0];
      dsp->fRec7[1] = dsp->fRec7[0];
    }
  }
}

#ifdef __cplusplus
}
#endif

#endif
