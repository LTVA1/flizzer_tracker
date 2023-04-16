#include "sound_engine_dpcm.h"

int32_t sound_engine_get_dpcm(SoundEngineDPCMsample* sample) {
    if(!(sample->playing)) return 0;

    if(sample->flags & SE_SAMPLE_LOOP) {
        if(sample->position > sample->loop_end) {
            sample->position = sample->loop_start;
            sample->delta_counter = sample->delta_counter_position_on_loop_start;
        }
    }

    if(sample->position >= sample->length) {
        sample->playing = false;
        return 0;
    }

    if(sample->data[sample->position >> 3] & (1 << (sample->position & 7))) {
        sample->delta_counter++;
    }

    else {
        sample->delta_counter--;
    }

    sample->position++;

    return ((int32_t)sample->delta_counter - DELTA_COUNTER_MIDDLE) * 4 * 256;
}