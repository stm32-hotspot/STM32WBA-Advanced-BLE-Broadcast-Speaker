/* ------------------------------------------------------------
author: "JOS, revised by RM"
name: "faust_compressor", "compressor"
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
  int fSampleRate;
  float fConst0;
  FAUSTFLOAT fHslider0;
  FAUSTFLOAT fHslider1;
  FAUSTFLOAT fHslider2;
  float fRec1[2];
  FAUSTFLOAT fHslider3;
  float fRec0[2];
  FAUSTFLOAT fHslider4;
  float fRec3[2];
  float fRec2[2];
  FAUSTFLOAT fHbargraph0;
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
  m->declare(m->metaInterface, "compressors.lib/compression_gain_mono:author", "Julius O. Smith III");
  m->declare(m->metaInterface, "compressors.lib/compression_gain_mono:copyright", "Copyright (C) 2014-2020 by Julius O. Smith III <jos@ccrma.stanford.edu>");
  m->declare(m->metaInterface, "compressors.lib/compression_gain_mono:license", "MIT-style STK-4.3 license");
  m->declare(m->metaInterface, "compressors.lib/compressor_stereo:author", "Julius O. Smith III");
  m->declare(m->metaInterface, "compressors.lib/compressor_stereo:copyright", "Copyright (C) 2014-2020 by Julius O. Smith III <jos@ccrma.stanford.edu>");
  m->declare(m->metaInterface, "compressors.lib/compressor_stereo:license", "MIT-style STK-4.3 license");
  m->declare(m->metaInterface, "compressors.lib/name", "Faust Compressor Effect Library");
  m->declare(m->metaInterface, "compressors.lib/version", "0.4");
  m->declare(m->metaInterface, "demos.lib/compressor_demo:author", "Julius O. Smith III");
  m->declare(m->metaInterface, "demos.lib/compressor_demo:licence", "MIT");
  m->declare(m->metaInterface, "demos.lib/name", "Faust Demos Library");
  m->declare(m->metaInterface, "demos.lib/version", "0.1");
  m->declare(m->metaInterface, "description", "Compressor demo application");
  m->declare(m->metaInterface, "filename", "faust_compressor.dsp");
  m->declare(m->metaInterface, "maths.lib/author", "GRAME");
  m->declare(m->metaInterface, "maths.lib/copyright", "GRAME");
  m->declare(m->metaInterface, "maths.lib/license", "LGPL with exception");
  m->declare(m->metaInterface, "maths.lib/name", "Faust Math Library");
  m->declare(m->metaInterface, "maths.lib/version", "2.6");
  m->declare(m->metaInterface, "name", "faust_compressor");
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
  dsp->fHslider0 = (FAUSTFLOAT)(5e+01f);
  dsp->fHslider1 = (FAUSTFLOAT)(-3e+01f);
  dsp->fHslider2 = (FAUSTFLOAT)(5e+02f);
  dsp->fHslider3 = (FAUSTFLOAT)(5.0f);
  dsp->fHslider4 = (FAUSTFLOAT)(4e+01f);
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
      dsp->fRec0[l1] = 0.0f;
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
    for (l3 = 0; l3 < 2; l3 = l3 + 1)
    {
      dsp->fRec2[l3] = 0.0f;
    }
  }
}

static void instanceConstantsmydsp(mydsp *dsp, int sample_rate)
{
  dsp->fSampleRate = sample_rate;
  dsp->fConst0 = 1.0f / fminf(1.92e+05f, fmaxf(1.0f, (float)(dsp->fSampleRate)));
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
  ui_interface->declare(ui_interface->uiInterface, 0, "tooltip", "Reference: http://en.wikipedia.org/wiki/Dynamic_range_compression");
  ui_interface->openVerticalBox(ui_interface->uiInterface, "COMPRESSOR");
  ui_interface->declare(ui_interface->uiInterface, 0, "0", "");
  ui_interface->openHorizontalBox(ui_interface->uiInterface, "0x00");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fCheckbox0, "0", "");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fCheckbox0, "tooltip", "When this is checked, the compressor has no effect");
  ui_interface->addCheckButton(ui_interface->uiInterface, "Bypass", &dsp->fCheckbox0);
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHbargraph0, "1", "");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHbargraph0, "tooltip", "Current gain of the compressor in dB");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHbargraph0, "unit", "dB");
  ui_interface->addHorizontalBargraph(ui_interface->uiInterface, "Compressor Gain", &dsp->fHbargraph0, (FAUSTFLOAT) - 5e+01f, (FAUSTFLOAT)1e+01f);
  ui_interface->closeBox(ui_interface->uiInterface);
  ui_interface->declare(ui_interface->uiInterface, 0, "1", "");
  ui_interface->openHorizontalBox(ui_interface->uiInterface, "0x00");
  ui_interface->declare(ui_interface->uiInterface, 0, "3", "");
  ui_interface->openHorizontalBox(ui_interface->uiInterface, "Compression Control");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider3, "0", "");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider3, "style", "knob");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider3, "tooltip", "A compression Ratio of N means that for each N dB increase in input signal level above Threshold, the output level goes up 1 dB");
  ui_interface->addHorizontalSlider(ui_interface->uiInterface, "Ratio", &dsp->fHslider3, (FAUSTFLOAT)5.0f, (FAUSTFLOAT)1.0f, (FAUSTFLOAT)2e+01f, (FAUSTFLOAT)0.1f);
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider1, "1", "");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider1, "style", "knob");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider1, "tooltip", "When the signal level exceeds the Threshold (in dB), its level is compressed according to the Ratio");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider1, "unit", "dB");
  ui_interface->addHorizontalSlider(ui_interface->uiInterface, "Threshold", &dsp->fHslider1, (FAUSTFLOAT) - 3e+01f, (FAUSTFLOAT) - 1e+02f, (FAUSTFLOAT)1e+01f, (FAUSTFLOAT)0.1f);
  ui_interface->closeBox(ui_interface->uiInterface);
  ui_interface->declare(ui_interface->uiInterface, 0, "4", "");
  ui_interface->openHorizontalBox(ui_interface->uiInterface, "Compression Response");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider0, "1", "");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider0, "scale", "log");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider0, "style", "knob");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider0, "tooltip", "Time constant in ms (1/e smoothing time) for the compression gain to approach (exponentially) a new lower target level (the compression 'kicking in')");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider0, "unit", "ms");
  ui_interface->addHorizontalSlider(ui_interface->uiInterface, "Attack", &dsp->fHslider0, (FAUSTFLOAT)5e+01f, (FAUSTFLOAT)1.0f, (FAUSTFLOAT)1e+03f, (FAUSTFLOAT)0.1f);
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider2, "2", "");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider2, "scale", "log");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider2, "style", "knob");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider2, "tooltip", "Time constant in ms (1/e smoothing time) for the compression gain to approach (exponentially) a new higher target level (the compression 'releasing')");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider2, "unit", "ms");
  ui_interface->addHorizontalSlider(ui_interface->uiInterface, "Release", &dsp->fHslider2, (FAUSTFLOAT)5e+02f, (FAUSTFLOAT)1.0f, (FAUSTFLOAT)1e+03f, (FAUSTFLOAT)0.1f);
  ui_interface->closeBox(ui_interface->uiInterface);
  ui_interface->closeBox(ui_interface->uiInterface);
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider4, "5", "");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider4, "tooltip", "The compressed-signal output level is increased by this amount (in dB) to make up for the level lost due to compression");
  ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider4, "unit", "dB");
  ui_interface->addHorizontalSlider(ui_interface->uiInterface, "Makeup Gain", &dsp->fHslider4, (FAUSTFLOAT)4e+01f, (FAUSTFLOAT) - 96.0f, (FAUSTFLOAT)96.0f, (FAUSTFLOAT)0.1f);
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
  float fSlow1 = fmaxf(dsp->fConst0, 0.001f * (float)(dsp->fHslider0));
  float fSlow2 = 0.5f * fSlow1;
  int iSlow3 = fabsf(fSlow2) < 1.1920929e-07f;
  float fSlow4 = ((iSlow3) ? 0.0f : expf(0.0f - dsp->fConst0 / ((iSlow3) ? 1.0f : fSlow2)));
  float fSlow5 = 1.0f - fSlow4;
  float fSlow6 = (float)(dsp->fHslider1);
  float fSlow7 = fmaxf(dsp->fConst0, 0.001f * (float)(dsp->fHslider2));
  int iSlow8 = fabsf(fSlow7) < 1.1920929e-07f;
  float fSlow9 = ((iSlow8) ? 0.0f : expf(0.0f - dsp->fConst0 / ((iSlow8) ? 1.0f : fSlow7)));
  int iSlow10 = fabsf(fSlow1) < 1.1920929e-07f;
  float fSlow11 = ((iSlow10) ? 0.0f : expf(0.0f - dsp->fConst0 / ((iSlow10) ? 1.0f : fSlow1)));
  float fSlow12 = 1.0f / fmaxf(1.1920929e-07f, (float)(dsp->fHslider3)) + -1.0f;
  float fSlow13 = powf(1e+01f, 0.05f * (float)(dsp->fHslider4));
  /* C99 loop */
  {
    int i0;
    for (i0 = 0; i0 < count; i0 = i0 + 1)
    {
      float fTemp0 = (float)(input0[i0]);
      float fTemp1 = ((iSlow0) ? 0.0f : fTemp0);
      float fTemp2 = (float)(input1[i0]);
      float fTemp3 = ((iSlow0) ? 0.0f : fTemp2);
      float fTemp4 = fabsf(fabsf(fTemp3) + fabsf(fTemp1));
      float fTemp5 = ((fTemp4 > dsp->fRec1[1]) ? fSlow11 : fSlow9);
      dsp->fRec1[0] = fTemp4 * (1.0f - fTemp5) + dsp->fRec1[1] * fTemp5;
      dsp->fRec0[0] = fSlow12 * fmaxf(2e+01f * log10f(fmaxf(1.1754944e-38f, dsp->fRec1[0])) - fSlow6, 0.0f) * fSlow5 + fSlow4 * dsp->fRec0[1];
      float fTemp6 = powf(1e+01f, 0.05f * dsp->fRec0[0]);
      float fTemp7 = fTemp1 * fTemp6;
      output0[i0] = (FAUSTFLOAT)(((iSlow0) ? fTemp0 : fSlow13 * fTemp7));
      float fTemp8 = fTemp3 * fTemp6;
      float fTemp9 = fabsf(fabsf(fTemp7) + fabsf(fTemp8));
      float fTemp10 = ((fTemp9 > dsp->fRec3[1]) ? fSlow11 : fSlow9);
      dsp->fRec3[0] = fTemp9 * (1.0f - fTemp10) + dsp->fRec3[1] * fTemp10;
      dsp->fRec2[0] = fSlow12 * fmaxf(2e+01f * log10f(fmaxf(1.1754944e-38f, dsp->fRec3[0])) - fSlow6, 0.0f) * fSlow5 + fSlow4 * dsp->fRec2[1];
      dsp->fHbargraph0 = (FAUSTFLOAT)(2e+01f * log10f(fmaxf(1.1754944e-38f, powf(1e+01f, 0.05f * dsp->fRec2[0]))));
      output1[i0] = (FAUSTFLOAT)(((iSlow0) ? fTemp2 : fSlow13 * fTemp8));
      dsp->fRec1[1] = dsp->fRec1[0];
      dsp->fRec0[1] = dsp->fRec0[0];
      dsp->fRec3[1] = dsp->fRec3[0];
      dsp->fRec2[1] = dsp->fRec2[0];
    }
  }
}

#ifdef __cplusplus
}
#endif

#endif
