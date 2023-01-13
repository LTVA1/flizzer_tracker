#pragma once

#include "sound_engine_osc.h"
#include "sound_engine_filter.h"
#include "freqs.h"
#include "sound_engine_defs.h"
#include "sound_engine_adsr.h"

void sound_engine_init(SoundEngine* sound_engine, uint32_t sample_rate, bool external_audio_output, uint32_t audio_buffer_size);
void sound_engine_deinit(SoundEngine* sound_engine);
void sound_engine_set_channel_frequency(SoundEngine* sound_engine, SoundEngineChannel* channel, uint16_t note);
void sound_engine_fill_buffer(SoundEngine* sound_engine, uint16_t* audio_buffer, uint32_t audio_buffer_size);
void sound_engine_enable_gate(SoundEngine* sound_engine, SoundEngineChannel* channel, bool enable);