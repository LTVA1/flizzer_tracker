#pragma once

#include "../flizzer_tracker.h"
#include "../tracker_engine/tracker_engine_defs.h"
#include "pattern_editor.h"

#include <furi.h>
#include <gui/gui.h>

void draw_instrument_view(Canvas* canvas, FlizzerTrackerApp* tracker);
void draw_instrument_program_view(Canvas* canvas, FlizzerTrackerApp* tracker);
void draw_inst_flag(
    FlizzerTrackerApp* tracker,
    Canvas* canvas,
    uint8_t focus,
    uint8_t param,
    const char* text,
    uint8_t x,
    uint8_t y,
    uint16_t flags,
    uint16_t mask);