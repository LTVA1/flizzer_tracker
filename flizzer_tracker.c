#include "flizzer_tracker.h"
#include "diskop.h"
#include "init_deinit.h"
#include "input_event.h"
#include "util.h"
#include "view/instrument_editor.h"
#include "view/pattern_editor.h"
#include "view/sample_editor.h"

#include "font.h"
#include <flizzer_tracker_icons.h>

void draw_callback(Canvas* canvas, void* ctx) {
    TrackerViewModel* model = (TrackerViewModel*)ctx;
    FlizzerTrackerApp* tracker = (FlizzerTrackerApp*)(model->tracker);

    canvas_set_color(canvas, ColorXOR);

    if(tracker->is_loading || tracker->is_loading_instrument) {
        canvas_draw_str(canvas, 10, 10, "Loading...");
        return;
    }

    if(tracker->is_loading_sample) {
        canvas_draw_str(canvas, 10, 10, "Importing sample...");

        canvas_draw_frame(canvas, 0, 20, 128, 10);

        uint8_t progress = 0;

        if(tracker->song.samples[tracker->current_sample]->length > 0) {
            progress = (uint64_t)tracker->sample_import_progress * 126 /
                       tracker->song.samples[tracker->current_sample]->length;
        }

        canvas_draw_box(canvas, 1, 21, progress, 8);

        return;
    }

    if(tracker->is_saving || tracker->is_saving_instrument) {
        canvas_draw_str(canvas, 10, 10, "Saving...");
        return;
    }

    if(tracker->showing_help) {
        canvas_draw_icon(canvas, 0, 0, &I_help);
        return;
    }

    canvas_set_custom_u8g2_font(canvas, u8g2_font_tom_thumb_4x6_tr);

    switch(tracker->mode) {
    case PATTERN_VIEW: {
        if(tracker->tracker_engine.song == NULL) {
            stop();
            tracker_engine_set_song(&tracker->tracker_engine, &tracker->song);
        }

        if(tracker->focus != EDIT_PATTERN) {
            draw_songinfo_view(canvas, tracker);
        }

        if(tracker->focus != EDIT_PATTERN) {
            draw_sequence_view(canvas, tracker);
        }

        draw_pattern_view(canvas, tracker);
        break;
    }

    case INST_EDITOR_VIEW: {
        draw_instrument_view(canvas, tracker);
        draw_instrument_program_view(canvas, tracker);
        break;
    }

    case SAMPLE_EDITOR_VIEW: {
        draw_sample_view(canvas, tracker);
        break;
    }

    default:
        break;
    }
}

bool input_callback(InputEvent* input_event, void* ctx) {
    // Проверяем, что контекст не нулевой
    furi_assert(ctx);
    TrackerView* tracker_view = (TrackerView*)ctx;
    FlizzerTrackerApp* tracker = (FlizzerTrackerApp*)(tracker_view->context);

    bool consumed = false;

    if(input_event->key == InputKeyBack && input_event->type == InputTypeShort) {
        tracker->period = furi_get_tick() - tracker->current_time;
        tracker->current_time = furi_get_tick();

        tracker->was_it_back_keypress = true;
    }

    else if(input_event->type == InputTypeShort || input_event->type == InputTypeLong) {
        tracker->was_it_back_keypress = false;
        tracker->period = 0;
    }

    uint32_t final_period = (tracker->was_it_back_keypress ? tracker->period : 0);

    FlizzerTrackerEvent event = {
        .type = EventTypeInput, .input = *input_event, .period = final_period};

    if(!(tracker->is_loading) && !(tracker->is_saving)) {
        furi_message_queue_put(tracker->event_queue, &event, FuriWaitForever);
    }

    consumed = true;
    return consumed;
}

int32_t flizzer_tracker_app(void* p) {
    UNUSED(p);

    Storage* storage = furi_record_open(RECORD_STORAGE);
    bool st = storage_simply_mkdir(storage, APPSDATA_FOLDER);
    st = storage_simply_mkdir(storage, FLIZZER_TRACKER_FOLDER);
    st = storage_simply_mkdir(storage, FLIZZER_TRACKER_INSTRUMENTS_FOLDER);
    UNUSED(st);
    furi_record_close(RECORD_STORAGE);

    FlizzerTrackerApp* tracker = init_tracker(44100, 50, true, 1024);

    //tracker->song.samples[0] = (SoundEngineDPCMsample*)malloc(sizeof(SoundEngineDPCMsample));

    //memset(tracker->song.samples[0], 0, sizeof(SoundEngineDPCMsample));

    tracker->song.num_samples = 1;
    tracker->song.samples[0]->data = (uint8_t*)malloc(16);

    strcpy((char*)&tracker->song.samples[0]->name, "TEXT");

    tracker->song.samples[0]->initial_delta_counter_position = 32;
    tracker->song.samples[0]->delta_counter_position_on_loop_start = 32;
    tracker->song.samples[0]->length = 8 * 16;
    tracker->song.samples[0]->flags |= SE_SAMPLE_LOOP;
    tracker->song.samples[0]->loop_start = 0;
    tracker->song.samples[0]->loop_end = 8 * 16 - 1;

    for(uint8_t i = 0; i < 4; i++) {
        tracker->song.samples[0]->data[i] = 0;
    }

    for(uint8_t i = 4; i < 12; i++) {
        tracker->song.samples[0]->data[i] = 0xff;
    }

    for(uint8_t i = 12; i < 16; i++) {
        tracker->song.samples[0]->data[i] = 0;
    }

    tracker->song.instrument[0]->waveform = 0;

    tracker->song.instrument[0]->adsr.a = 0;
    tracker->song.instrument[0]->adsr.s = 0xff;

    tracker->song.instrument[0]->sample = 0;
    tracker->song.instrument[0]->sample_pointer = tracker->song.samples[0];
    tracker->song.instrument[0]->sound_engine_flags |= SE_ENABLE_SAMPLE |
                                                       SE_SAMPLE_OVERRIDE_ENVELOPE;

    // Текущее событие типа кастомного типа FlizzerTrackerEvent
    FlizzerTrackerEvent event;

    view_dispatcher_switch_to_view(tracker->view_dispatcher, VIEW_TRACKER);

    // Бесконечный цикл обработки очереди событий
    while(!(tracker->quit)) {
        // Выбираем событие из очереди в переменную event (ждём бесконечно долго, если очередь пуста)
        // и проверяем, что у нас получилось это сделать
        furi_check(
            furi_message_queue_get(tracker->event_queue, &event, FuriWaitForever) == FuriStatusOk);

        // Наше событие — это нажатие кнопки
        if(event.type == EventTypeInput) {
            process_input_event(tracker, &event);
        }

        if(event.type == EventTypeSaveSong) {
            save_song(tracker, tracker->filepath);
        }

        if(event.type == EventTypeSaveInstrument) {
            save_instrument(tracker, tracker->filepath);
        }

        if(event.type == EventTypeLoadSong) {
            stop_song(tracker);

            tracker->tracker_engine.sequence_position = tracker->tracker_engine.pattern_position =
                tracker->current_instrument = 0;

            tracker->dialogs = furi_record_open(RECORD_DIALOGS);
            tracker->is_loading = true;

            FuriString* path;
            path = furi_string_alloc();
            furi_string_set(path, FLIZZER_TRACKER_FOLDER);

            DialogsFileBrowserOptions browser_options;
            dialog_file_browser_set_basic_options(
                &browser_options, SONG_FILE_EXT, &I_flizzer_tracker_module);
            browser_options.base_path = FLIZZER_TRACKER_FOLDER;
            browser_options.hide_ext = false;

            bool ret = dialog_file_browser_show(tracker->dialogs, path, path, &browser_options);

            furi_record_close(RECORD_DIALOGS);

            const char* cpath = furi_string_get_cstr(path);

            if(ret && strcmp(&cpath[strlen(cpath) - 4], SONG_FILE_EXT) == 0) {
                bool result = load_song_util(tracker, path);
                tracker->current_sample = 0;
                UNUSED(result);
            }

            else {
                furi_string_free(path);
                tracker->is_loading = false;
            }
        }

        if(event.type == EventTypeLoadInstrument) {
            stop_song(tracker);

            tracker->dialogs = furi_record_open(RECORD_DIALOGS);
            tracker->is_loading_instrument = true;

            FuriString* path;
            path = furi_string_alloc();
            furi_string_set(path, FLIZZER_TRACKER_INSTRUMENTS_FOLDER);

            DialogsFileBrowserOptions browser_options;
            dialog_file_browser_set_basic_options(
                &browser_options, INST_FILE_EXT, &I_flizzer_tracker_instrument);
            browser_options.base_path = FLIZZER_TRACKER_FOLDER;
            browser_options.hide_ext = false;

            bool ret = dialog_file_browser_show(tracker->dialogs, path, path, &browser_options);

            furi_record_close(RECORD_DIALOGS);

            const char* cpath = furi_string_get_cstr(path);

            if(ret && strcmp(&cpath[strlen(cpath) - 4], INST_FILE_EXT) == 0) {
                bool result = load_instrument_util(tracker, path);
                UNUSED(result);
            }

            else {
                furi_string_free(path);
                tracker->is_loading_instrument = false;
            }
        }

        if(event.type == EventTypeLoadSample) {
            stop_song(tracker);

            tracker->dialogs = furi_record_open(RECORD_DIALOGS);
            tracker->is_loading_sample = true;

            FuriString* path;
            path = furi_string_alloc();
            furi_string_set(path, FLIZZER_TRACKER_FOLDER);

            DialogsFileBrowserOptions browser_options;
            dialog_file_browser_set_basic_options(&browser_options, ".wav", &I_wav_icon);
            browser_options.base_path = FLIZZER_TRACKER_FOLDER;
            browser_options.hide_ext = false;

            bool ret = dialog_file_browser_show(tracker->dialogs, path, path, &browser_options);

            furi_record_close(RECORD_DIALOGS);

            const char* cpath = furi_string_get_cstr(path);

            if(ret && strcmp(&cpath[strlen(cpath) - 4], ".wav") == 0) {
                bool result = load_sample_util(tracker, path);
                UNUSED(result);
            }

            else {
                furi_string_free(path);
                tracker->is_loading_sample = false;
            }
        }

        if(event.type == EventTypeSetAudioMode) {
            sound_engine_PWM_timer_init(tracker->external_audio);

            tracker->sound_engine.external_audio_output = tracker->external_audio;
        }
    }

    stop();

    save_config(tracker);

    deinit_tracker(tracker);

    return 0;
}