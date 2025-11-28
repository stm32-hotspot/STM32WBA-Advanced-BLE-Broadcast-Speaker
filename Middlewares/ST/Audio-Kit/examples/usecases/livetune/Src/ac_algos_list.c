/**
  ******************************************************************************
  * @file    ac_algos_list.c.
  * @author  MCD Application Team
  * @brief   c file that links the list of algorithms
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2019(-2022) STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */


/* Includes ------------------------------------------------------------------*/
#ifdef USE_ACOUSTIC_BF
#include "acousticbf/audio_chain_acousticBF_cardoid.h"
#include "acousticbf/audio_chain_acousticBF.h"
#endif
#include "capture/audio_chain_capture.h"
#include "constantsource/audio_chain_constantSource.h"
#include "cic/audio_chain_cic.h"
#include "deinterleave/audio_chain_deinterleave.h"
#include "delay/audio_chain_delay.h"
#include "disto/audio_chain_disto.h"
#include "echo/audio_chain_echo.h"
#include "faust/audio_chain_faust_compressor.h"
#include "faust/audio_chain_faust_distortion.h"
#include "faust/audio_chain_faust_flanger.h"
#include "faust/audio_chain_faust_noise_gate.h"
#include "faust/audio_chain_faust_phaser.h"
#include "faust/audio_chain_faust_reverb_dattorro.h"
#include "faust/audio_chain_sound_chorus.h"
#include "fbeamforming/audio_chain_fBeamforming.h"
#include "fdisjoin/audio_chain_fdisjoin.h"
#include "fir_equalizer/audio_chain_fir_equalizer.h"
#include "fir_graphic_equalizer/audio_chain_fir_graphic_equalizer.h"
#include "fjoin/audio_chain_fjoin.h"
#include "fnlms/audio_chain_fnlms.h"
#include "g711/audio_chain_g711_dec.h"
#include "g711/audio_chain_g711_enc.h"
#include "gain/audio_chain_gain.h"
#include "hpf/audio_chain_hpf.h"
#include "iir_equalizer/audio_chain_iir_equalizer.h"
#include "interleave/audio_chain_interleave.h"
#include "linear_mix/audio_chain_linear_mix.h"
#include "mdrc/audio_chain_mdrc.h"
#include "mix/audio_chain_mix.h"
#include "mono2stereo/audio_chain_mono2stereo.h"
#include "nlms/audio_chain_nlms.h"
#include "nr/audio_chain_nr.h"
#include "passthrough/audio_chain_passThrough.h"
#ifdef USE_PDM2PCM_LIB
#include "pdm2pcm/audio_chain_pdm2pcm.h"
#endif
#include "resample/audio_chain_resample.h"
#include "rms/audio_chain_rms.h"
#include "route/audio_chain_route.h"
#include "sam/audio_chain_sam.h"
#include "sfc/audio_chain_sfc.h"
#include "signal_generator/audio_chain_signal_generator.h"
#include "sound_font/audio_chain_sound_font.h"
#include "spectrum/audio_chain_spectrum.h"
#include "speex_aec/audio_chain_speex_aec.h"
#include "speex_nr/audio_chain_speex_nr.h"
#include "split/audio_chain_split.h"
#include "stereo2mono/audio_chain_stereo2mono.h"
#include "switch/audio_chain_switch.h"
#include "voidsink/audio_chain_voidSink.h"
#include "wola/audio_chain_wola_fwd.h"
#include "wola/audio_chain_wola_inv.h"


/* Exported variables --------------------------------------------------------*/
#ifdef USE_ACOUSTIC_BF
#ifndef USE_PDM2PCM_LIB
  #error "USE_PDM2PCM_LIB must be define and library must be added to the project"
#else
  ALGO_FACTORY_DECLARE(AudioChainWrp_acousticBF_cardoid_factory);
  ALGO_FACTORY_DECLARE(AudioChainWrp_acousticBF_factory);
#endif
#endif
ALGO_FACTORY_DECLARE(AudioChainWrp_capture_factory);
ALGO_FACTORY_DECLARE(AudioChainWrp_constantSource_factory);
ALGO_FACTORY_DECLARE(AudioChainWrp_cic_factory);
ALGO_FACTORY_DECLARE(AudioChainWrp_deinterleave_factory);
ALGO_FACTORY_DECLARE(AudioChainWrp_delay_factory);
ALGO_FACTORY_DECLARE(AudioChainWrp_disto_factory);
ALGO_FACTORY_DECLARE(AudioChainWrp_echo_factory);
ALGO_FACTORY_DECLARE(AudioChainWrp_faust_compressor_factory);
ALGO_FACTORY_DECLARE(AudioChainWrp_faust_distortion_factory);
ALGO_FACTORY_DECLARE(AudioChainWrp_faust_flanger_factory);
ALGO_FACTORY_DECLARE(AudioChainWrp_faust_noise_gate_factory);
ALGO_FACTORY_DECLARE(AudioChainWrp_faust_phaser_factory);
ALGO_FACTORY_DECLARE(AudioChainWrp_faust_reverb_dattorro_factory);
ALGO_FACTORY_DECLARE(AudioChainWrp_sound_chorus_factory);
ALGO_FACTORY_DECLARE(AudioChainWrp_fBeamforming_factory);
ALGO_FACTORY_DECLARE(AudioChainWrp_fdisjoin_factory);
ALGO_FACTORY_DECLARE(AudioChainWrp_fir_equalizer_factory);
ALGO_FACTORY_DECLARE(AudioChainWrp_fir_graphic_equalizer_factory);
ALGO_FACTORY_DECLARE(AudioChainWrp_fjoin_factory);
ALGO_FACTORY_DECLARE(AudioChainWrp_fnlms_factory);
ALGO_FACTORY_DECLARE(AudioChainWrp_g711_dec_factory);
ALGO_FACTORY_DECLARE(AudioChainWrp_g711_enc_factory);
ALGO_FACTORY_DECLARE(AudioChainWrp_gain_factory);
ALGO_FACTORY_DECLARE(AudioChainWrp_hpf_factory);
ALGO_FACTORY_DECLARE(AudioChainWrp_iir_equalizer_factory);
ALGO_FACTORY_DECLARE(AudioChainWrp_interleave_factory);
ALGO_FACTORY_DECLARE(AudioChainWrp_linear_mix_factory);
ALGO_FACTORY_DECLARE(AudioChainWrp_mdrc_factory);
ALGO_FACTORY_DECLARE(AudioChainWrp_mix_factory);
ALGO_FACTORY_DECLARE(AudioChainWrp_mono2stereo_factory);
ALGO_FACTORY_DECLARE(AudioChainWrp_nlms_factory);
ALGO_FACTORY_DECLARE(AudioChainWrp_nr_factory);
ALGO_FACTORY_DECLARE(AudioChainWrp_passThrough_factory);
#ifdef USE_PDM2PCM_LIB
  ALGO_FACTORY_DECLARE(AudioChainWrp_pdm2pcm_factory);
#endif
ALGO_FACTORY_DECLARE(AudioChainWrp_resample_factory);
ALGO_FACTORY_DECLARE(AudioChainWrp_rms_factory);
ALGO_FACTORY_DECLARE(AudioChainWrp_route_factory);
ALGO_FACTORY_DECLARE(AudioChainWrp_sam_factory);
ALGO_FACTORY_DECLARE(AudioChainWrp_sfc_factory);
ALGO_FACTORY_DECLARE(AudioChainWrp_signal_generator_factory);
#ifdef USE_SOUND_FONT
  ALGO_FACTORY_DECLARE(AudioChainWrp_sound_font_factory);
#endif
ALGO_FACTORY_DECLARE(AudioChainWrp_spectrum_factory);
ALGO_FACTORY_DECLARE(AudioChainWrp_speex_aec_factory);
ALGO_FACTORY_DECLARE(AudioChainWrp_speex_nr_factory);
ALGO_FACTORY_DECLARE(AudioChainWrp_split_factory);
ALGO_FACTORY_DECLARE(AudioChainWrp_stereo2mono_factory);
ALGO_FACTORY_DECLARE(AudioChainWrp_switch_factory);
ALGO_FACTORY_DECLARE(AudioChainWrp_voidSink_factory);
ALGO_FACTORY_DECLARE(AudioChainWrp_wola_fwd_factory);
ALGO_FACTORY_DECLARE(AudioChainWrp_wola_inv_factory);
