#include "sample_editor.h"

void draw_sample_view(Canvas* canvas, FlizzerTrackerApp* tracker) {
    canvas_draw_line(canvas, 0, 32, 127, 32);

    char buffer[30];

    snprintf(
        buffer,
        sizeof(buffer),
        "%d %d %ld",
        tracker->bit_depth,
        tracker->num_channels,
        tracker->length);
    canvas_draw_str(canvas, 0, 10, buffer);

    snprintf(buffer, sizeof(buffer), "WAVE:%02X", tracker->current_sample);
    draw_generic_n_digit_field(tracker, canvas, EDIT_SAMPLE, SAMPLE_NUMBER, buffer, 0, 39, 2);

    snprintf(buffer, sizeof(buffer), "%s", tracker->song.samples[tracker->current_sample]->name);
    draw_generic_n_digit_field(tracker, canvas, EDIT_SAMPLE, SAMPLE_NAME, buffer, 4 * 8, 39, 1);

    snprintf(
        buffer,
        sizeof(buffer),
        "LENGTH:%06lX",
        tracker->song.samples[tracker->current_sample]->length);
    canvas_draw_str(canvas, 0, 39 + 6, buffer);

    snprintf(
        buffer,
        sizeof(buffer),
        "SIZE:%06lX BYTES",
        (tracker->song.samples[tracker->current_sample]->length >> 3));
    canvas_draw_str(canvas, 63 - 2, 39 + 6, buffer);

    draw_inst_flag(
        tracker,
        canvas,
        EDIT_SAMPLE,
        SAMPLE_LOOP,
        "LOOP",
        0,
        39 + 6 + 6,
        tracker->song.samples[tracker->current_sample]->flags,
        SE_SAMPLE_LOOP);

    snprintf(
        buffer,
        sizeof(buffer),
        "START:%06lX",
        tracker->song.samples[tracker->current_sample]->loop_start);
    draw_generic_n_digit_field(
        tracker, canvas, EDIT_SAMPLE, SAMPLE_LOOP_START, buffer, 4 * 7, 39 + 6 + 6, 6);

    snprintf(
        buffer,
        sizeof(buffer),
        "END:%06lX",
        tracker->song.samples[tracker->current_sample]->loop_end);
    draw_generic_n_digit_field(
        tracker,
        canvas,
        EDIT_SAMPLE,
        SAMPLE_LOOP_END,
        buffer,
        4 * 6 + 4 * (6 + 6 + 2),
        39 + 6 + 6,
        6);
}