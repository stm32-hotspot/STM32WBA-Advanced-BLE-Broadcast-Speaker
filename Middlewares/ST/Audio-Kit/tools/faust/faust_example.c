/* ------------------------------------------------------------
name: "faust_example.faust"
Code generated with Faust 2.70.3 (https://faust.grame.fr)
Compilation options: -lang c -ct 1 -es 1 -mcd 16 -mdd 1024 -mdy 33 -single -ftz 0
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

typedef struct {
	int iRec3[2];
} mydspSIG0;

static mydspSIG0* newmydspSIG0() { return (mydspSIG0*)calloc(1, sizeof(mydspSIG0)); }
static void deletemydspSIG0(mydspSIG0* dsp) { free(dsp); }

int getNumInputsmydspSIG0(mydspSIG0* RESTRICT dsp) {
	return 0;
}
int getNumOutputsmydspSIG0(mydspSIG0* RESTRICT dsp) {
	return 1;
}

static void instanceInitmydspSIG0(mydspSIG0* dsp, int sample_rate) {
	/* C99 loop */
	{
		int l4;
		for (l4 = 0; l4 < 2; l4 = l4 + 1) {
			dsp->iRec3[l4] = 0;
		}
	}
}

static void fillmydspSIG0(mydspSIG0* dsp, int count, float* table) {
	/* C99 loop */
	{
		int i1;
		for (i1 = 0; i1 < count; i1 = i1 + 1) {
			dsp->iRec3[0] = dsp->iRec3[1] + 1;
			table[i1] = fabsf(fmodf(0.00390625f * (float)(dsp->iRec3[0] + -1) + 1.0f, 4.0f) + -2.0f) + -1.0f;
			dsp->iRec3[1] = dsp->iRec3[0];
		}
	}
}

typedef struct {
	int iRec5[2];
} mydspSIG1;

static mydspSIG1* newmydspSIG1() { return (mydspSIG1*)calloc(1, sizeof(mydspSIG1)); }
static void deletemydspSIG1(mydspSIG1* dsp) { free(dsp); }

int getNumInputsmydspSIG1(mydspSIG1* RESTRICT dsp) {
	return 0;
}
int getNumOutputsmydspSIG1(mydspSIG1* RESTRICT dsp) {
	return 1;
}

static void instanceInitmydspSIG1(mydspSIG1* dsp, int sample_rate) {
	/* C99 loop */
	{
		int l6;
		for (l6 = 0; l6 < 2; l6 = l6 + 1) {
			dsp->iRec5[l6] = 0;
		}
	}
}

static void fillmydspSIG1(mydspSIG1* dsp, int count, float* table) {
	/* C99 loop */
	{
		int i2;
		for (i2 = 0; i2 < count; i2 = i2 + 1) {
			dsp->iRec5[0] = dsp->iRec5[1] + 1;
			table[i2] = fabsf(fmodf(0.005859375f * (float)(dsp->iRec5[0] + -1) + 1.0f, 4.0f) + -2.0f) + -1.0f;
			dsp->iRec5[1] = dsp->iRec5[0];
		}
	}
}

typedef struct {
	int iRec6[2];
} mydspSIG2;

static mydspSIG2* newmydspSIG2() { return (mydspSIG2*)calloc(1, sizeof(mydspSIG2)); }
static void deletemydspSIG2(mydspSIG2* dsp) { free(dsp); }

int getNumInputsmydspSIG2(mydspSIG2* RESTRICT dsp) {
	return 0;
}
int getNumOutputsmydspSIG2(mydspSIG2* RESTRICT dsp) {
	return 1;
}

static void instanceInitmydspSIG2(mydspSIG2* dsp, int sample_rate) {
	/* C99 loop */
	{
		int l7;
		for (l7 = 0; l7 < 2; l7 = l7 + 1) {
			dsp->iRec6[l7] = 0;
		}
	}
}

static void fillmydspSIG2(mydspSIG2* dsp, int count, float* table) {
	/* C99 loop */
	{
		int i3;
		for (i3 = 0; i3 < count; i3 = i3 + 1) {
			dsp->iRec6[0] = dsp->iRec6[1] + 1;
			table[i3] = fabsf(fmodf(0.0078125f * (float)(dsp->iRec6[0] + -1) + 1.0f, 4.0f) + -2.0f) + -1.0f;
			dsp->iRec6[1] = dsp->iRec6[0];
		}
	}
}

typedef struct {
	int iRec7[2];
} mydspSIG3;

static mydspSIG3* newmydspSIG3() { return (mydspSIG3*)calloc(1, sizeof(mydspSIG3)); }
static void deletemydspSIG3(mydspSIG3* dsp) { free(dsp); }

int getNumInputsmydspSIG3(mydspSIG3* RESTRICT dsp) {
	return 0;
}
int getNumOutputsmydspSIG3(mydspSIG3* RESTRICT dsp) {
	return 1;
}

static void instanceInitmydspSIG3(mydspSIG3* dsp, int sample_rate) {
	/* C99 loop */
	{
		int l8;
		for (l8 = 0; l8 < 2; l8 = l8 + 1) {
			dsp->iRec7[l8] = 0;
		}
	}
}

static void fillmydspSIG3(mydspSIG3* dsp, int count, float* table) {
	/* C99 loop */
	{
		int i4;
		for (i4 = 0; i4 < count; i4 = i4 + 1) {
			dsp->iRec7[0] = dsp->iRec7[1] + 1;
			table[i4] = fabsf(fmodf(0.015625f * (float)(dsp->iRec7[0] + -1) + 1.0f, 4.0f) + -2.0f) + -1.0f;
			dsp->iRec7[1] = dsp->iRec7[0];
		}
	}
}

static float ftbl0mydspSIG0[1024];
static float ftbl1mydspSIG1[1024];
static float ftbl2mydspSIG2[1024];
static float ftbl3mydspSIG3[1024];

#ifndef FAUSTCLASS 
#define FAUSTCLASS mydsp
#endif

#ifdef __APPLE__ 
#define exp10f __exp10f
#define exp10 __exp10
#endif

typedef struct {
	int iVec0[2];
	int fSampleRate;
	float fConst0;
	float fConst1;
	float fConst2;
	FAUSTFLOAT fHslider0;
	float fRec1[2];
	float fConst3;
	float fRec0[2];
	FAUSTFLOAT fHslider1;
	float fRec2[2];
	FAUSTFLOAT fHslider2;
	FAUSTFLOAT fHslider3;
	FAUSTFLOAT fEntry0;
	float fRec4[2];
	FAUSTFLOAT fButton0;
	int iVec1[2];
	FAUSTFLOAT fHslider4;
	FAUSTFLOAT fHslider5;
	int iRec9[2];
	FAUSTFLOAT fHslider6;
	FAUSTFLOAT fHslider7;
	float fRec8[2];
	FAUSTFLOAT fEntry1;
} mydsp;

mydsp* newmydsp() { 
	mydsp* dsp = (mydsp*)calloc(1, sizeof(mydsp));
	return dsp;
}

void deletemydsp(mydsp* dsp) { 
	free(dsp);
}

void metadatamydsp(MetaGlue* m) { 
	m->declare(m->metaInterface, "basics.lib/name", "Faust Basic Element Library");
	m->declare(m->metaInterface, "basics.lib/tabulateNd", "Copyright (C) 2023 Bart Brouns <bart@magnetophon.nl>");
	m->declare(m->metaInterface, "basics.lib/version", "1.12.0");
	m->declare(m->metaInterface, "compile_options", "-lang c -ct 1 -es 1 -mcd 16 -mdd 1024 -mdy 33 -single -ftz 0");
	m->declare(m->metaInterface, "envelopes.lib/adsre:author", "Julius O. Smith III");
	m->declare(m->metaInterface, "envelopes.lib/adsre:licence", "STK-4.3");
	m->declare(m->metaInterface, "envelopes.lib/author", "GRAME");
	m->declare(m->metaInterface, "envelopes.lib/copyright", "GRAME");
	m->declare(m->metaInterface, "envelopes.lib/license", "LGPL with exception");
	m->declare(m->metaInterface, "envelopes.lib/name", "Faust Envelope Library");
	m->declare(m->metaInterface, "envelopes.lib/version", "1.3.0");
	m->declare(m->metaInterface, "filename", "faust_example.faust");
	m->declare(m->metaInterface, "maths.lib/author", "GRAME");
	m->declare(m->metaInterface, "maths.lib/copyright", "GRAME");
	m->declare(m->metaInterface, "maths.lib/license", "LGPL with exception");
	m->declare(m->metaInterface, "maths.lib/name", "Faust Math Library");
	m->declare(m->metaInterface, "maths.lib/version", "2.7.0");
	m->declare(m->metaInterface, "name", "faust_example.faust");
	m->declare(m->metaInterface, "oscillators.lib/lf_sawpos:author", "Bart Brouns, revised by Stéphane Letz");
	m->declare(m->metaInterface, "oscillators.lib/lf_sawpos:licence", "STK-4.3");
	m->declare(m->metaInterface, "oscillators.lib/name", "Faust Oscillator Library");
	m->declare(m->metaInterface, "oscillators.lib/saw1:author", "Bart Brouns");
	m->declare(m->metaInterface, "oscillators.lib/saw1:licence", "STK-4.3");
	m->declare(m->metaInterface, "oscillators.lib/version", "1.5.0");
	m->declare(m->metaInterface, "platform.lib/name", "Generic Platform Library");
	m->declare(m->metaInterface, "platform.lib/version", "1.3.0");
	m->declare(m->metaInterface, "signals.lib/name", "Faust Signal Routing Library");
	m->declare(m->metaInterface, "signals.lib/version", "1.5.0");
}

int getSampleRatemydsp(mydsp* RESTRICT dsp) {
	return dsp->fSampleRate;
}

int getNumInputsmydsp(mydsp* RESTRICT dsp) {
	return 0;
}
int getNumOutputsmydsp(mydsp* RESTRICT dsp) {
	return 1;
}

void classInitmydsp(int sample_rate) {
	mydspSIG0* sig0 = newmydspSIG0();
	instanceInitmydspSIG0(sig0, sample_rate);
	fillmydspSIG0(sig0, 1024, ftbl0mydspSIG0);
	mydspSIG1* sig1 = newmydspSIG1();
	instanceInitmydspSIG1(sig1, sample_rate);
	fillmydspSIG1(sig1, 1024, ftbl1mydspSIG1);
	mydspSIG2* sig2 = newmydspSIG2();
	instanceInitmydspSIG2(sig2, sample_rate);
	fillmydspSIG2(sig2, 1024, ftbl2mydspSIG2);
	mydspSIG3* sig3 = newmydspSIG3();
	instanceInitmydspSIG3(sig3, sample_rate);
	fillmydspSIG3(sig3, 1024, ftbl3mydspSIG3);
	deletemydspSIG0(sig0);
	deletemydspSIG1(sig1);
	deletemydspSIG2(sig2);
	deletemydspSIG3(sig3);
}

void instanceResetUserInterfacemydsp(mydsp* dsp) {
	dsp->fHslider0 = (FAUSTFLOAT)(0.1f);
	dsp->fHslider1 = (FAUSTFLOAT)(0.0f);
	dsp->fHslider2 = (FAUSTFLOAT)(0.0f);
	dsp->fHslider3 = (FAUSTFLOAT)(0.0f);
	dsp->fEntry0 = (FAUSTFLOAT)(4.4e+02f);
	dsp->fButton0 = (FAUSTFLOAT)(0.0f);
	dsp->fHslider4 = (FAUSTFLOAT)(0.8f);
	dsp->fHslider5 = (FAUSTFLOAT)(0.01f);
	dsp->fHslider6 = (FAUSTFLOAT)(0.6f);
	dsp->fHslider7 = (FAUSTFLOAT)(0.2f);
	dsp->fEntry1 = (FAUSTFLOAT)(0.5f);
}

void instanceClearmydsp(mydsp* dsp) {
	/* C99 loop */
	{
		int l0;
		for (l0 = 0; l0 < 2; l0 = l0 + 1) {
			dsp->iVec0[l0] = 0;
		}
	}
	/* C99 loop */
	{
		int l1;
		for (l1 = 0; l1 < 2; l1 = l1 + 1) {
			dsp->fRec1[l1] = 0.0f;
		}
	}
	/* C99 loop */
	{
		int l2;
		for (l2 = 0; l2 < 2; l2 = l2 + 1) {
			dsp->fRec0[l2] = 0.0f;
		}
	}
	/* C99 loop */
	{
		int l3;
		for (l3 = 0; l3 < 2; l3 = l3 + 1) {
			dsp->fRec2[l3] = 0.0f;
		}
	}
	/* C99 loop */
	{
		int l5;
		for (l5 = 0; l5 < 2; l5 = l5 + 1) {
			dsp->fRec4[l5] = 0.0f;
		}
	}
	/* C99 loop */
	{
		int l9;
		for (l9 = 0; l9 < 2; l9 = l9 + 1) {
			dsp->iVec1[l9] = 0;
		}
	}
	/* C99 loop */
	{
		int l10;
		for (l10 = 0; l10 < 2; l10 = l10 + 1) {
			dsp->iRec9[l10] = 0;
		}
	}
	/* C99 loop */
	{
		int l11;
		for (l11 = 0; l11 < 2; l11 = l11 + 1) {
			dsp->fRec8[l11] = 0.0f;
		}
	}
}

void instanceConstantsmydsp(mydsp* dsp, int sample_rate) {
	dsp->fSampleRate = sample_rate;
	dsp->fConst0 = fminf(1.92e+05f, fmaxf(1.0f, (float)(dsp->fSampleRate)));
	dsp->fConst1 = 44.1f / dsp->fConst0;
	dsp->fConst2 = 1.0f - dsp->fConst1;
	dsp->fConst3 = 1.0f / dsp->fConst0;
}

void instanceInitmydsp(mydsp* dsp, int sample_rate) {
	instanceConstantsmydsp(dsp, sample_rate);
	instanceResetUserInterfacemydsp(dsp);
	instanceClearmydsp(dsp);
}

void initmydsp(mydsp* dsp, int sample_rate) {
	classInitmydsp(sample_rate);
	instanceInitmydsp(dsp, sample_rate);
}

void buildUserInterfacemydsp(mydsp* dsp, UIGlue* ui_interface) {
	ui_interface->openVerticalBox(ui_interface->uiInterface, "faust_example.faust");
	ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider5, "midi", "ctrl 73");
	ui_interface->addHorizontalSlider(ui_interface->uiInterface, "A", &dsp->fHslider5, (FAUSTFLOAT)0.01f, (FAUSTFLOAT)0.01f, (FAUSTFLOAT)4.0f, (FAUSTFLOAT)0.01f);
	ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider6, "midi", "ctrl 76");
	ui_interface->addHorizontalSlider(ui_interface->uiInterface, "D", &dsp->fHslider6, (FAUSTFLOAT)0.6f, (FAUSTFLOAT)0.01f, (FAUSTFLOAT)8.0f, (FAUSTFLOAT)0.01f);
	ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider4, "midi", "ctrl 72");
	ui_interface->addHorizontalSlider(ui_interface->uiInterface, "R", &dsp->fHslider4, (FAUSTFLOAT)0.8f, (FAUSTFLOAT)0.01f, (FAUSTFLOAT)8.0f, (FAUSTFLOAT)0.01f);
	ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider7, "midi", "ctrl 77");
	ui_interface->addHorizontalSlider(ui_interface->uiInterface, "S", &dsp->fHslider7, (FAUSTFLOAT)0.2f, (FAUSTFLOAT)0.0f, (FAUSTFLOAT)1.0f, (FAUSTFLOAT)0.01f);
	ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider3, "midi", "pitchwheel");
	ui_interface->addHorizontalSlider(ui_interface->uiInterface, "bend", &dsp->fHslider3, (FAUSTFLOAT)0.0f, (FAUSTFLOAT)-2.0f, (FAUSTFLOAT)2.0f, (FAUSTFLOAT)0.01f);
	ui_interface->declare(ui_interface->uiInterface, &dsp->fEntry0, "unit", "Hz");
	ui_interface->addNumEntry(ui_interface->uiInterface, "freq", &dsp->fEntry0, (FAUSTFLOAT)4.4e+02f, (FAUSTFLOAT)2e+01f, (FAUSTFLOAT)2e+04f, (FAUSTFLOAT)1.0f);
	ui_interface->addNumEntry(ui_interface->uiInterface, "gain", &dsp->fEntry1, (FAUSTFLOAT)0.5f, (FAUSTFLOAT)0.0f, (FAUSTFLOAT)1.0f, (FAUSTFLOAT)0.01f);
	ui_interface->addButton(ui_interface->uiInterface, "gate", &dsp->fButton0);
	ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider1, "midi", "ctrl 1");
	ui_interface->addHorizontalSlider(ui_interface->uiInterface, "lfoDepth", &dsp->fHslider1, (FAUSTFLOAT)0.0f, (FAUSTFLOAT)0.0f, (FAUSTFLOAT)1.0f, (FAUSTFLOAT)0.001f);
	ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider0, "midi", "ctrl 14");
	ui_interface->addHorizontalSlider(ui_interface->uiInterface, "lfoFreq", &dsp->fHslider0, (FAUSTFLOAT)0.1f, (FAUSTFLOAT)0.01f, (FAUSTFLOAT)1e+01f, (FAUSTFLOAT)0.001f);
	ui_interface->declare(ui_interface->uiInterface, &dsp->fHslider2, "midi", "ctrl");
	ui_interface->addHorizontalSlider(ui_interface->uiInterface, "waveTravel", &dsp->fHslider2, (FAUSTFLOAT)0.0f, (FAUSTFLOAT)0.0f, (FAUSTFLOAT)1.0f, (FAUSTFLOAT)0.01f);
	ui_interface->closeBox(ui_interface->uiInterface);
}

void computemydsp(mydsp* dsp, int count, FAUSTFLOAT** RESTRICT inputs, FAUSTFLOAT** RESTRICT outputs) {
	FAUSTFLOAT* output0 = outputs[0];
	float fSlow0 = dsp->fConst1 * (float)(dsp->fHslider0);
	float fSlow1 = dsp->fConst1 * (float)(dsp->fHslider1);
	float fSlow2 = (float)(dsp->fHslider2);
	float fSlow3 = dsp->fConst3 * (float)(dsp->fEntry0) * powf(2.0f, 0.083333336f * (float)(dsp->fHslider3));
	int iSlow4 = (float)(dsp->fButton0) > 0.0f;
	float fSlow5 = (float)(dsp->fHslider4);
	float fSlow6 = (float)(dsp->fHslider5);
	int iSlow7 = (int)(dsp->fConst0 * fSlow6);
	float fSlow8 = (float)(dsp->fHslider6);
	float fSlow9 = (float)(iSlow4);
	float fSlow10 = (float)(dsp->fHslider7) * fSlow9;
	float fSlow11 = (float)(dsp->fEntry1);
	/* C99 loop */
	{
		int i0;
		for (i0 = 0; i0 < count; i0 = i0 + 1) {
			dsp->iVec0[0] = 1;
			int iTemp0 = 1 - dsp->iVec0[1];
			dsp->fRec1[0] = fSlow0 + dsp->fConst2 * dsp->fRec1[1];
			float fTemp1 = ((iTemp0) ? 0.0f : dsp->fRec0[1] + dsp->fConst3 * dsp->fRec1[0]);
			dsp->fRec0[0] = fTemp1 - floorf(fTemp1);
			dsp->fRec2[0] = fSlow1 + dsp->fConst2 * dsp->fRec2[1];
			float fTemp2 = fmaxf(0.0f, fminf(1.0f, fSlow2 + dsp->fRec2[0] * (1.0f - fabsf(2.0f * dsp->fRec0[0] + -1.0f))));
			float fTemp3 = ((iTemp0) ? 0.0f : fSlow3 + dsp->fRec4[1]);
			dsp->fRec4[0] = fTemp3 - floorf(fTemp3);
			int iTemp4 = max(0, min((int)(1024.0f * dsp->fRec4[0]), 1023));
			dsp->iVec1[0] = iSlow4;
			int iTemp5 = iSlow4 - dsp->iVec1[1];
			dsp->iRec9[0] = iSlow4 * (dsp->iRec9[1] + 1);
			int iTemp6 = (dsp->iRec9[0] < iSlow7) | (iTemp5 * (iTemp5 > 0));
			float fTemp7 = 0.1447178f * ((iSlow4) ? ((iTemp6) ? fSlow6 : fSlow8) : fSlow5);
			int iTemp8 = fabsf(fTemp7) < 1.1920929e-07f;
			float fTemp9 = ((iTemp8) ? 0.0f : expf(-(dsp->fConst3 / ((iTemp8) ? 1.0f : fTemp7))));
			dsp->fRec8[0] = (1.0f - fTemp9) * ((iSlow4) ? ((iTemp6) ? fSlow9 : fSlow10) : 0.0f) + fTemp9 * dsp->fRec8[1];
			output0[i0] = (FAUSTFLOAT)(fSlow11 * dsp->fRec8[0] * (ftbl3mydspSIG3[iTemp4] * fmaxf(0.0f, cosf(4.712385f * fTemp2)) + ftbl2mydspSIG2[iTemp4] * fmaxf(0.0f, cosf(4.712385f * (fTemp2 + -0.33333334f))) + ftbl1mydspSIG1[iTemp4] * fmaxf(0.0f, cosf(4.712385f * (fTemp2 + -0.6666667f))) + ftbl0mydspSIG0[iTemp4] * fmaxf(0.0f, cosf(4.712385f * (fTemp2 + -1.0f)))));
			dsp->iVec0[1] = dsp->iVec0[0];
			dsp->fRec1[1] = dsp->fRec1[0];
			dsp->fRec0[1] = dsp->fRec0[0];
			dsp->fRec2[1] = dsp->fRec2[0];
			dsp->fRec4[1] = dsp->fRec4[0];
			dsp->iVec1[1] = dsp->iVec1[0];
			dsp->iRec9[1] = dsp->iRec9[0];
			dsp->fRec8[1] = dsp->fRec8[0];
		}
	}
}

#ifdef __cplusplus
}
#endif

#endif
