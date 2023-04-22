#pragma once

#include "sound_engine_defs.h"

int32_t sound_engine_get_dpcm(SoundEngineDPCMsample* sample);
void recalculate_dpcm_sample_delta_counter_at_loop_start(SoundEngineDPCMsample* sample);