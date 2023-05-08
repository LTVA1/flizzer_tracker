#include "diskop.h"

#define CFG_FILENAME "settings.cfg"

void save_instrument_inner(Stream* stream, Instrument* inst) {
    size_t rwops = stream_write(stream, (uint8_t*)inst->name, sizeof(inst->name));
    rwops = stream_write(stream, (uint8_t*)&inst->waveform, sizeof(inst->waveform));
    rwops = stream_write(stream, (uint8_t*)&inst->flags, sizeof(inst->flags));
    rwops = stream_write(
        stream, (uint8_t*)&inst->sound_engine_flags, sizeof(inst->sound_engine_flags));

    rwops = stream_write(stream, (uint8_t*)&inst->base_note, sizeof(inst->base_note));
    rwops = stream_write(stream, (uint8_t*)&inst->finetune, sizeof(inst->finetune));

    rwops = stream_write(stream, (uint8_t*)&inst->slide_speed, sizeof(inst->slide_speed));

    rwops = stream_write(stream, (uint8_t*)&inst->adsr, sizeof(inst->adsr));
    rwops = stream_write(stream, (uint8_t*)&inst->pw, sizeof(inst->pw));

    if(inst->sound_engine_flags & SE_ENABLE_RING_MOD) {
        rwops = stream_write(stream, (uint8_t*)&inst->ring_mod, sizeof(inst->ring_mod));
    }

    if(inst->sound_engine_flags & SE_ENABLE_HARD_SYNC) {
        rwops = stream_write(stream, (uint8_t*)&inst->hard_sync, sizeof(inst->hard_sync));
    }

    uint8_t progsteps = 0;

    for(uint8_t i = 0; i < INST_PROG_LEN; i++) {
        if((inst->program[i] & 0x7fff) != TE_PROGRAM_NOP) {
            progsteps = i + 1;
        }
    }

    rwops = stream_write(stream, (uint8_t*)&progsteps, sizeof(progsteps));

    if(progsteps > 0) {
        rwops =
            stream_write(stream, (uint8_t*)inst->program, progsteps * sizeof(inst->program[0]));
    }

    rwops = stream_write(stream, (uint8_t*)&inst->program_period, sizeof(inst->program_period));

    if(inst->flags & TE_ENABLE_VIBRATO) {
        rwops = stream_write(stream, (uint8_t*)&inst->vibrato_speed, sizeof(inst->vibrato_speed));
        rwops = stream_write(stream, (uint8_t*)&inst->vibrato_depth, sizeof(inst->vibrato_depth));
        rwops = stream_write(stream, (uint8_t*)&inst->vibrato_delay, sizeof(inst->vibrato_delay));
    }

    if(inst->flags & TE_ENABLE_PWM) {
        rwops = stream_write(stream, (uint8_t*)&inst->pwm_speed, sizeof(inst->pwm_speed));
        rwops = stream_write(stream, (uint8_t*)&inst->pwm_depth, sizeof(inst->pwm_depth));
        rwops = stream_write(stream, (uint8_t*)&inst->pwm_delay, sizeof(inst->pwm_delay));
    }

    if(inst->sound_engine_flags & SE_ENABLE_FILTER) {
        rwops = stream_write(stream, (uint8_t*)&inst->filter_cutoff, sizeof(inst->filter_cutoff));
        rwops = stream_write(
            stream, (uint8_t*)&inst->filter_resonance, sizeof(inst->filter_resonance));
        rwops = stream_write(stream, (uint8_t*)&inst->filter_type, sizeof(inst->filter_type));
    }

    if(inst->sound_engine_flags & SE_ENABLE_SAMPLE) {
        rwops = stream_write(stream, (uint8_t*)&inst->sample, sizeof(inst->sample));
    }

    UNUSED(rwops);
}

void save_sample_inner(Stream* stream, SoundEngineDPCMsample* sample) {
    size_t rwops = stream_write(stream, (uint8_t*)sample->name, sizeof(sample->name));
    rwops = stream_write(stream, (uint8_t*)&sample->flags, sizeof(sample->flags));
    rwops = stream_write(
        stream,
        (uint8_t*)&sample->initial_delta_counter_position,
        sizeof(sample->initial_delta_counter_position));

    uint32_t length = sample->length;

    if(sample->data == NULL) {
        length = 0;
    }

    rwops = stream_write(stream, (uint8_t*)&length, sizeof(length));

    if(sample->flags & SE_SAMPLE_LOOP) {
        rwops = stream_write(stream, (uint8_t*)&sample->loop_start, sizeof(sample->loop_start));
        rwops = stream_write(stream, (uint8_t*)&sample->loop_end, sizeof(sample->loop_end));
        rwops = stream_write(
            stream,
            (uint8_t*)&sample->delta_counter_position_on_loop_start,
            sizeof(sample->delta_counter_position_on_loop_start));
    }

    if(length > 0) {
        for(uint32_t i = 0; i < sample->length / 8 + 1; i++) {
            rwops = stream_write(stream, &sample->data[i], 1);
        }
    }

    UNUSED(rwops);
}

bool save_instrument(FlizzerTrackerApp* tracker, FuriString* filepath) {
    bool file_removed =
        storage_simply_remove(tracker->storage, furi_string_get_cstr(filepath)); // just in case
    bool open_file = file_stream_open(
        tracker->stream, furi_string_get_cstr(filepath), FSAM_WRITE, FSOM_OPEN_ALWAYS);

    uint8_t version = TRACKER_ENGINE_VERSION;
    size_t rwops =
        stream_write(tracker->stream, (uint8_t*)INST_FILE_SIG, sizeof(INST_FILE_SIG) - 1);
    rwops = stream_write(tracker->stream, (uint8_t*)&version, sizeof(uint8_t));

    Instrument* inst = tracker->song.instrument[tracker->current_instrument];

    save_instrument_inner(tracker->stream, inst);

    file_stream_close(tracker->stream);
    tracker->is_saving_instrument = false;
    furi_string_free(filepath);

    UNUSED(file_removed);
    UNUSED(open_file);
    UNUSED(rwops);
    return false;
}

bool save_song(FlizzerTrackerApp* tracker, FuriString* filepath) {
    bool file_removed =
        storage_simply_remove(tracker->storage, furi_string_get_cstr(filepath)); // just in case
    bool open_file = file_stream_open(
        tracker->stream, furi_string_get_cstr(filepath), FSAM_WRITE, FSOM_OPEN_ALWAYS);

    uint8_t version = TRACKER_ENGINE_VERSION;
    size_t rwops =
        stream_write(tracker->stream, (uint8_t*)SONG_FILE_SIG, sizeof(SONG_FILE_SIG) - 1);
    rwops = stream_write(tracker->stream, (uint8_t*)&version, sizeof(uint8_t));

    TrackerSong* song = &tracker->song;

    /*for(uint32_t i = 0; i < 23444; i++)
    {
        rwops = stream_write(tracker->stream, (uint8_t*)&song->loop_end, sizeof(uint8_t));
    }*/

    rwops = stream_write(tracker->stream, (uint8_t*)song->song_name, sizeof(song->song_name));
    rwops = stream_write(tracker->stream, (uint8_t*)&song->loop_start, sizeof(song->loop_start));
    rwops = stream_write(tracker->stream, (uint8_t*)&song->loop_end, sizeof(song->loop_end));
    rwops = stream_write(
        tracker->stream, (uint8_t*)&song->pattern_length, sizeof(song->pattern_length));

    rwops = stream_write(tracker->stream, (uint8_t*)&song->speed, sizeof(song->speed));
    rwops = stream_write(tracker->stream, (uint8_t*)&song->rate, sizeof(song->rate));

    rwops = stream_write(
        tracker->stream, (uint8_t*)&song->num_sequence_steps, sizeof(song->num_sequence_steps));

    for(uint16_t i = 0; i < song->num_sequence_steps; i++) {
        rwops = stream_write(
            tracker->stream,
            (uint8_t*)&song->sequence.sequence_step[i],
            sizeof(song->sequence.sequence_step[0]));
    }

    rwops =
        stream_write(tracker->stream, (uint8_t*)&song->num_patterns, sizeof(song->num_patterns));

    for(uint16_t i = 0; i < song->num_patterns; i++) {
        rwops = stream_write(
            tracker->stream,
            (uint8_t*)song->pattern[i].step,
            sizeof(TrackerSongPatternStep) * (song->pattern_length));
    }

    rwops = stream_write(
        tracker->stream, (uint8_t*)&song->num_instruments, sizeof(song->num_instruments));

    for(uint16_t i = 0; i < song->num_instruments; i++) {
        save_instrument_inner(tracker->stream, song->instrument[i]);
    }

    rwops = stream_write(tracker->stream, (uint8_t*)&song->num_samples, sizeof(song->num_samples));

    for(uint16_t i = 0; i < song->num_samples; i++) {
        save_sample_inner(tracker->stream, song->samples[i]);
    }

    file_stream_close(tracker->stream);
    tracker->is_saving = false;
    furi_string_free(filepath);

    UNUSED(file_removed);
    UNUSED(open_file);
    UNUSED(rwops);
    return false;
}

bool load_song_util(FlizzerTrackerApp* tracker, FuriString* filepath) {
    bool open_file = file_stream_open(
        tracker->stream, furi_string_get_cstr(filepath), FSAM_READ, FSOM_OPEN_ALWAYS);

    bool result = load_song(&tracker->song, tracker->stream);

    tracker->is_loading = false;
    file_stream_close(tracker->stream);
    furi_string_free(filepath);
    UNUSED(open_file);
    return result;
}

uint8_t get_sample(uint16_t num_channels, uint16_t bit_depth, Stream* stream) {
    uint8_t sample = 32;

    bool fuck;

    if(bit_depth == 8 && num_channels == 1) //8 bit unsigned mono
    {
        uint8_t temp = 0;
        fuck = stream_read(stream, (uint8_t*)&temp, sizeof(temp));

        sample = temp / 4;
        return sample; //scaled to 0-63
    }

    if(bit_depth == 8 && num_channels == 2) //8 bit unsigned stereo
    {
        uint8_t temp_r = 0;
        uint8_t temp_l = 0;
        fuck = stream_read(stream, (uint8_t*)&temp_r, sizeof(temp_r));
        fuck = stream_read(stream, (uint8_t*)&temp_l, sizeof(temp_l));

        sample = ((uint16_t)temp_r + temp_l) / 2 / 4;
        return sample; //scaled to 0-63
    }

    if(bit_depth == 16 && num_channels == 1) //16 bit signed mono
    {
        int16_t temp = 0;
        fuck = stream_read(stream, (uint8_t*)&temp, sizeof(temp));

        sample = ((int32_t)temp + 32767) / 256 / 4;
        return sample; //scaled to 0-63
    }

    if(bit_depth == 16 && num_channels == 2) //16 bit signed stereo
    {
        int16_t temp_r = 0;
        int16_t temp_l = 0;
        fuck = stream_read(stream, (uint8_t*)&temp_r, sizeof(temp_r));
        fuck = stream_read(stream, (uint8_t*)&temp_l, sizeof(temp_l));

        sample = ((int32_t)temp_r + (int32_t)temp_l + 32767 + 32767) / 2 / 4 / 256;
        return sample; //scaled to 0-63
    }

    return sample; //scaled to 0-63
    UNUSED(fuck);
}

void import_sample_inner(Stream* stream, SoundEngineDPCMsample* sample, FlizzerTrackerApp* tracker) {
    uint16_t num_channels = 0;
    uint16_t bit_depth = 0;
    uint32_t sample_size = 0;

    bool fuck = stream_seek(stream, 22, StreamOffsetFromStart);
    fuck = stream_read(stream, (uint8_t*)&num_channels, sizeof(num_channels));

    fuck = stream_seek(stream, 34, StreamOffsetFromStart);
    fuck = stream_read(stream, (uint8_t*)&bit_depth, sizeof(bit_depth));

    char data_sig[5];
    data_sig[4] = '\0';

    uint16_t pos = 0;

    while(strcmp(data_sig, "data") !=
          0) //because not all wav files' headers are 44 bytes long! ffmpeg no exception
    {
        fuck = stream_seek(stream, pos, StreamOffsetFromStart);
        fuck = stream_read(stream, (uint8_t*)data_sig, 4);
        pos++;
    }

    fuck = stream_seek(
        stream,
        pos + 3,
        StreamOffsetFromStart); //read wav file size (offset 3 because we got one more byr from pos++ above)
    fuck = stream_read(stream, (uint8_t*)&sample_size, sizeof(sample_size));

    pos = stream_tell(stream);

    tracker->bit_depth = bit_depth;
    tracker->num_channels = num_channels;
    tracker->length = sample_size;

    uint32_t num_samples = sample_size / (bit_depth / 8) / num_channels;

    if(num_samples / 8 >
       memmgr_get_free_heap() - 5 * 1024) //the resulting DPCM sample is too big to fit in RAM
    {
        return;
    }

    if(sample->loop_end >= num_samples) {
        sample->loop_end = num_samples - 1;
    }

    if(sample->loop_start >= num_samples) {
        sample->loop_start = 0;
    }

    sample->length = num_samples;

    uint8_t sample_value = get_sample(num_channels, bit_depth, stream);

    sample->initial_delta_counter_position = sample_value;

    //fuck = stream_seek(stream, pos, StreamOffsetFromStart);

    if(sample->data) {
        free(sample->data);
    }

    sample->data = (uint8_t*)malloc(sizeof(uint8_t) * num_samples / 8 + 1);
    memset(sample->data, 0, sizeof(uint8_t) * num_samples / 8 + 1);

    int8_t delta_counter = sample->initial_delta_counter_position;

    for(tracker->sample_import_progress = 0; tracker->sample_import_progress < num_samples;
        tracker->sample_import_progress++) //read other samples
    {
        sample_value = get_sample(num_channels, bit_depth, stream);

        if(delta_counter <= sample_value) {
            if(delta_counter < 63) {
                delta_counter++;
                sample->data[tracker->sample_import_progress / 8] |=
                    (1 << (tracker->sample_import_progress & 7));
            }

            else {
                delta_counter--;
            }
        }

        else {
            if(delta_counter > 0)
                delta_counter--;

            else {
                delta_counter++;
                sample->data[tracker->sample_import_progress / 8] |=
                    (1 << (tracker->sample_import_progress & 7));
            }
        }
    }

    recalculate_dpcm_sample_delta_counter_at_loop_start(sample);

    UNUSED(fuck);
    UNUSED(sample);
}

bool load_sample_util(FlizzerTrackerApp* tracker, FuriString* filepath) {
    bool open_file = file_stream_open(
        tracker->stream, furi_string_get_cstr(filepath), FSAM_READ, FSOM_OPEN_ALWAYS);

    import_sample_inner(tracker->stream, tracker->song.samples[tracker->current_sample], tracker);

    tracker->is_loading_sample = false;

    file_stream_close(tracker->stream);
    furi_string_free(filepath);

    UNUSED(open_file);
    return true;
}

bool load_instrument_disk(TrackerSong* song, uint8_t inst, Stream* stream) {
    set_default_instrument(song->instrument[inst]);

    char header[sizeof(INST_FILE_SIG) + 2] = {0};
    size_t rwops = stream_read(stream, (uint8_t*)&header, sizeof(INST_FILE_SIG) - 1);
    header[sizeof(INST_FILE_SIG)] = '\0';

    uint8_t version = 0;

    if(strcmp(header, INST_FILE_SIG) == 0) {
        rwops = stream_read(stream, (uint8_t*)&version, sizeof(version));

        if(version <= TRACKER_ENGINE_VERSION) {
            load_instrument_inner(stream, song->instrument[inst], version);
        }
    }

    UNUSED(rwops);
    return false;
}

bool load_instrument_util(FlizzerTrackerApp* tracker, FuriString* filepath) {
    bool open_file = file_stream_open(
        tracker->stream, furi_string_get_cstr(filepath), FSAM_READ, FSOM_OPEN_ALWAYS);

    bool result =
        load_instrument_disk(&tracker->song, tracker->current_instrument, tracker->stream);

    tracker->is_loading_instrument = false;
    file_stream_close(tracker->stream);
    furi_string_free(filepath);
    UNUSED(open_file);
    return result;
}

void save_config(FlizzerTrackerApp* tracker) {
    // stream_read_line
    FuriString* filepath = furi_string_alloc();
    FuriString* config_line = furi_string_alloc();
    furi_string_cat_printf(filepath, "%s/%s", FLIZZER_TRACKER_FOLDER, CFG_FILENAME);

    bool open_file = file_stream_open(
        tracker->stream, furi_string_get_cstr(filepath), FSAM_WRITE, FSOM_OPEN_ALWAYS);
    UNUSED(open_file);

    furi_string_cat_printf(
        config_line, "%s = %s\n", "external_audio", tracker->external_audio ? "true" : "false");
    stream_write_string(tracker->stream, config_line);

    file_stream_close(tracker->stream);
    furi_string_free(filepath);
    furi_string_free(config_line);
}

void load_config(FlizzerTrackerApp* tracker) {
    FuriString* filepath = furi_string_alloc();
    FuriString* config_line = furi_string_alloc();
    furi_string_cat_printf(filepath, "%s/%s", FLIZZER_TRACKER_FOLDER, CFG_FILENAME);

    bool open_file = file_stream_open(
        tracker->stream, furi_string_get_cstr(filepath), FSAM_READ, FSOM_OPEN_ALWAYS);
    UNUSED(open_file);

    stream_read_line(tracker->stream, config_line);

    sscanf(
        furi_string_get_cstr(config_line), "%s%s%s", tracker->param, tracker->eq, tracker->value);

    if(strcmp(tracker->param, "external_audio") == 0) {
        if(strcmp(tracker->value, "false") == 0) {
            tracker->external_audio = false;
            // strcpy(tracker->value, "false_");
        }

        if(strcmp(tracker->value, "true") == 0) {
            tracker->external_audio = true;
            // strcpy(tracker->value, "true_");
        }

        sound_engine_init(
            &tracker->sound_engine,
            tracker->sound_engine.sample_rate,
            tracker->external_audio,
            tracker->sound_engine.audio_buffer_size);
        // sound_engine_set_audio_output(tracker->external_audio);
    }

    file_stream_close(tracker->stream);
    furi_string_free(filepath);
    furi_string_free(config_line);
}