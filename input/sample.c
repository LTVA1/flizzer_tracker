#include "sample.h"
#include "songinfo.h"

void edit_sample_param(FlizzerTrackerApp* tracker, uint8_t selected_param, int8_t init_delta) {
    int32_t delta = init_delta;

    if(selected_param == SAMPLE_LOOP_END || selected_param == SAMPLE_LOOP_START) {
        delta *= (int32_t)(1 << ((5 - (int32_t)tracker->current_digit) * 4));
    }

    if(selected_param == SAMPLE_NAME) {
        delta *= (int32_t)(1 << ((1 - (int32_t)tracker->current_digit) * 4));
    }

    switch(selected_param) {
    case SAMPLE_NUMBER: {
        int8_t temp = tracker->current_sample + delta;

        if(temp < MAX_DPCM_SAMPLES && temp >= 0) {
            if(temp == 0 ||
               (temp > 0 &&
                tracker->song.samples[temp - 1]->data)) //do not allow to make empty samples
            {
                tracker->current_sample = temp;
            }
        }

        break;
    }

    case SAMPLE_NAME: {
        text_input_set_header_text(tracker->text_input, "Sample name:");
        text_input_set_result_callback(
            tracker->text_input,
            return_from_keyboard_callback,
            tracker,
            (char*)&tracker->song.samples[tracker->current_sample]->name,
            WAVE_NAME_LEN,
            false);

        view_dispatcher_switch_to_view(tracker->view_dispatcher, VIEW_KEYBOARD);
        break;
    }

    case SAMPLE_LOOP: {
        flipbit(tracker->song.samples[tracker->current_sample]->flags, SE_SAMPLE_LOOP);
        break;
    }

    case SAMPLE_LOOP_START: {
        int32_t temp = tracker->song.samples[tracker->current_sample]->loop_start;

        temp += delta;

        if(temp >= 0) {
            if((uint32_t)temp < tracker->song.samples[tracker->current_sample]->length - 1 &&
               (uint32_t)temp < tracker->song.samples[tracker->current_sample]->loop_end) {
                tracker->song.samples[tracker->current_sample]->loop_start = temp;
            }
        }

        recalculate_dpcm_sample_delta_counter_at_loop_start(
            tracker->song.samples[tracker->current_sample]);

        break;
    }

    case SAMPLE_LOOP_END: {
        int32_t temp = tracker->song.samples[tracker->current_sample]->loop_end;

        temp += delta;

        if(temp >= 0) {
            if((uint32_t)temp < tracker->song.samples[tracker->current_sample]->length &&
               (uint32_t)temp > tracker->song.samples[tracker->current_sample]->loop_start) {
                tracker->song.samples[tracker->current_sample]->loop_end = temp;
            }
        }

        break;
    }

    default:
        break;
    }
}

void sample_edit_event(FlizzerTrackerApp* tracker, FlizzerTrackerEvent* event) {
    if(event->input.key == InputKeyOk && event->input.type == InputTypeShort &&
       !tracker->tracker_engine.playing) {
        tracker->editing = !(tracker->editing);
        return;
    }

    if(event->input.key == InputKeyRight && event->input.type == InputTypeShort) {
        switch(tracker->selected_param) {
        default: {
            tracker->current_digit++;

            if(tracker->current_digit > 0) {
                tracker->selected_param++;

                tracker->current_digit = 0;

                if(tracker->selected_param > SAMPLE_PARAMS - 1) {
                    tracker->selected_param = 0;
                }
            }

            break;
        }

        case SAMPLE_NUMBER: {
            tracker->current_digit++;

            if(tracker->current_digit > 1) {
                tracker->selected_param++;

                tracker->current_digit = 0;

                if(tracker->selected_param > SAMPLE_PARAMS - 1) {
                    tracker->selected_param = 0;
                }
            }

            break;
        }

        case SAMPLE_LOOP_START:
        case SAMPLE_LOOP_END: {
            tracker->current_digit++;

            if(tracker->current_digit > 5) {
                tracker->selected_param++;

                tracker->current_digit = 0;

                if(tracker->selected_param > SAMPLE_PARAMS - 1) {
                    tracker->selected_param = 0;
                }
            }

            break;
        }
        }
    }

    if(event->input.key == InputKeyLeft && event->input.type == InputTypeShort) {
        switch(tracker->selected_param) {
        default: {
            tracker->current_digit--;

            if(tracker->current_digit > 0) // unsigned int overflow
            {
                tracker->selected_param--;

                tracker->current_digit = 0;

                if(tracker->selected_param > SAMPLE_PARAMS - 1) // unsigned int overflow
                {
                    tracker->selected_param = SAMPLE_PARAMS - 1;
                }
            }

            break;
        }

        case SAMPLE_NUMBER: {
            tracker->current_digit--;

            if(tracker->current_digit > 1) // unsigned int overflow
            {
                tracker->selected_param--;

                tracker->current_digit = 0;

                if(tracker->selected_param > SAMPLE_PARAMS - 1) // unsigned int overflow
                {
                    tracker->selected_param = SAMPLE_PARAMS - 1;
                }
            }

            break;
        }

        case SAMPLE_LOOP_START:
        case SAMPLE_LOOP_END: {
            tracker->current_digit--;

            if(tracker->current_digit > 5) // unsigned int overflow
            {
                tracker->selected_param--;

                tracker->current_digit = 0;

                if(tracker->selected_param > SAMPLE_PARAMS - 1) // unsigned int overflow
                {
                    tracker->selected_param = SAMPLE_PARAMS - 1;
                }
            }

            break;
        }
        }

        return;
    }

    if(event->input.key == InputKeyDown && event->input.type == InputTypeShort) {
        if(tracker->editing) {
            edit_sample_param(tracker, tracker->selected_param, -1);
        }

        return;
    }

    if(event->input.key == InputKeyUp && event->input.type == InputTypeShort) {
        if(tracker->editing) {
            edit_sample_param(tracker, tracker->selected_param, 1);
        }

        return;
    }
}