#include "wav.h"
#include "common.h"
#include "smrt_arena.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

b32 seek_to_chunk(FILE *file, char const *chunk_name) {
    char cur_name[4];
     u32 chunk_size;

    fseek(file, 0, SEEK_END);
    const i64 max_pos = ftell(file);
    fseek(file, sizeof(wav_master_chunk_t), SEEK_SET);
    while (true) {
        fread(cur_name, 4, 1, file);
        fread(&chunk_size, 4, 1, file);

        if (memcmp(cur_name, chunk_name, 4) == 0) break;

        if (chunk_size % 2 == 1) chunk_size++;

        if (ftell(file) + (i64)chunk_size >= max_pos) return false;

        fseek(file, chunk_size, SEEK_CUR);
    }

    fseek(file, -8, SEEK_CUR);
    return true;
}

wav_data_t load_wav_file(smrt_arena_t *arena, FILE *wav_file, wav_master_chunk_t *master_o, wav_fmt_chunk_t *format_o, u64 align_up_memoryn) {
    wav_data_t data = { 0 };

    if (!wav_file) return data;

    fread(master_o, sizeof(wav_master_chunk_t), 1, wav_file);

    if (seek_to_chunk(wav_file, "fmt ") == false) return data;
    fread(format_o, sizeof(wav_fmt_chunk_t), 1, wav_file);

    {
        u16 f = format_o->audio_format;
        if (f != 1 && f != 3) {
            perror("Attempted to read non pcm integer wave file.");
            return data;
        }
    }

    if (seek_to_chunk(wav_file, "data") == false) return data;
    fseek(wav_file, 4, SEEK_CUR);

    u32 sampled_data_size;
    fread(&sampled_data_size, 4, 1, wav_file);

    data.samples = smrt_arena_push(arena, ALIGN_UP_POW2(sampled_data_size, align_up_memoryn), true);
    if (!data.samples) return data;

    fread((u8*)data.samples, sampled_data_size, 1, wav_file);
    data.sample_count = sampled_data_size / ((format_o->bits_per_sample / 8) * format_o->num_channels);

    return data;
}

b32 write_wav_file(FILE *wav_file, wav_fmt_chunk_t *fmt_chunk_i, wav_data_t data_i) {
    if (!wav_file) return false;

    u32 data_size_bytes = data_i.sample_count * fmt_chunk_i->bytes_per_block;

    wav_master_chunk_t mchunk = { // MASTER + FMT + "data" + sizeof(data_size_bytes) + data_size_bytes (-8 bytes)
        .file_size = sizeof(wav_master_chunk_t) + sizeof(wav_fmt_chunk_t) + data_size_bytes
    };
    memcpy(&mchunk.chunk_id, "RIFF", 4);
    memcpy(&mchunk.file_format_id, "WAVE", 4);

    fwrite(&mchunk, sizeof(wav_master_chunk_t), 1, wav_file);
    fwrite(fmt_chunk_i, sizeof(wav_fmt_chunk_t), 1, wav_file);

    fwrite("data", 4, 1, wav_file);
    fwrite(&data_size_bytes, 4, 1, wav_file);
    fwrite(data_i.samples, sizeof(u8), data_size_bytes, wav_file);

    return true;
}

wav_fmt_chunk_t make_wav_fmt_chunk(u32 num_channels, u32 sample_rate, u16 bits_per_sample) {
    wav_fmt_chunk_t o = {
        .bits_per_sample = bits_per_sample,
        .num_channels = num_channels,
        .sample_rate = sample_rate,
        .audio_format = 1,
        .bytes_per_block = num_channels * (bits_per_sample / 8),
        .chunk_size = sizeof(wav_fmt_chunk_t) - 8,
    };
    o.bytes_per_sec = (u32)o.bytes_per_block * sample_rate;
    memcpy(&o.chunk_id, "fmt ", 4);

    return o;
}

void wav_load(smrt_arena_t *arena, FILE *wav, f64 ***samples_o ,u16 *channel_count_o ,u64 *sample_count_o, u32 *sample_rate_o, u64 align_up_memoryn, smrt_arena_t **conflicts, u64 num_conflicts) {
    smrta_temp_t scratch = smrta_scratch_start(conflicts, num_conflicts);

    wav_master_chunk_t m;
    wav_fmt_chunk_t    f;

    smrt_arena_mark(arena);

    wav_data_t data = load_wav_file(scratch.arena,
                                    wav,
                                    &m,
                                    &f,
                                    align_up_memoryn);

    if (!data.samples) {
        perror("Failed to load data.");
        goto failed;
    }

    *samples_o = smrt_arena_push(arena, ALIGN_UP_POW2(sizeof(f64*) * f.num_channels, align_up_memoryn), true);
    *channel_count_o = f.num_channels;


    for (u16 c = 0; c < f.num_channels; c++) {
        f64 *d;
        switch (f.audio_format) {
            case 1:
                switch (f.bits_per_sample) {
                    case 8:
                        d = read_8bps_data(arena,
                                           data,
                                           f.num_channels,
                                           c,
                                           align_up_memoryn);
                        break;
                    case 16:
                        d = read_16bps_data(arena,
                                            data,
                                            f.num_channels,
                                            c,
                                            align_up_memoryn);
                        break;
                    case 24:
                        d = read_24bps_data(arena,
                                            data,
                                            f.num_channels,
                                            c,
                                            align_up_memoryn);
                        break;
                    default:
                        perror("Only PCM integer 8,16,24 bits data can be read.");
                        goto failed;
                }
                break;
            case 3:
                switch (f.bits_per_sample) {
                    case 32:
                        d = read_32bps_float_data(arena,
                                                  data,
                                                  f.num_channels,
                                                  c,
                                                  align_up_memoryn);
                        break;
                    default:
                        perror("Only 32 bits float data can be read.");
                        goto failed;
                }
                break;
            default:
                perror("Expected audio_format=1|3");
                goto failed;
        }
        (*samples_o)[c] = d;
    }

    smrta_scratch_end(scratch);
    *sample_count_o = data.sample_count;
    *sample_rate_o = f.sample_rate;
    return;

failed:
    smrta_scratch_end(scratch);
    smrt_arena_pop_to_mark(arena);
    *samples_o = NULL;
    *sample_count_o = 0;
    *sample_rate_o = 0;
    return;
}

f64 *read_8bps_data(smrt_arena_t *arena, wav_data_t data, u16 num_channels, u16 channel, u64 align_up_memoryn) {
    assert(num_channels != 0 && "num_channels must be >0");

    f64 *vs = smrt_arena_push(arena, ALIGN_UP_POW2(sizeof(f64) * data.sample_count, align_up_memoryn), true);
    if (!vs) return NULL;

    for (u64 i = 0; i < data.sample_count; i++) {
        u8 sample = data.samples[(i*num_channels)+  channel];
        f64 amplitude = (f64)(sample - 128) / (f64)(0b1<<7);

        vs[i] = amplitude;
    }

    return vs;
}

f64 *read_16bps_data(smrt_arena_t *arena, wav_data_t data, u16 num_channels, u16 channel, u64 align_up_memoryn) {
    assert(num_channels != 0 && "num_channels must be >0");

    f64 *vs = smrt_arena_push(arena, ALIGN_UP_POW2(sizeof(f64) * data.sample_count, align_up_memoryn), true);
    if (!vs) return NULL;

    for (u64 i = 0; i < data.sample_count; i++) {
        u8  low = data.samples[(i*2*num_channels)+  channel*2];
        u8 high = data.samples[(i*2*num_channels)+1+channel*2];
        i16 intermediate = low | (high << 8);
        f64 amplitude = (f64)intermediate / (f64)(0b1<<15);

        vs[i] = amplitude;
    }

    return vs;
}

f64 *read_24bps_data(smrt_arena_t *arena, wav_data_t data, u16 num_channels, u16 channel, u64 align_up_memoryn) {
    assert(num_channels != 0 && "num_channels must be >0");

    f64 *vs = smrt_arena_push(arena, ALIGN_UP_POW2(sizeof(f64) * data.sample_count, align_up_memoryn), true);
    if (!vs) return NULL;

    for (u64 i = 0; i < data.sample_count; i++) {
        u8  low = data.samples[(i*3*num_channels)+  channel*3];
        u8  mid = data.samples[(i*3*num_channels)+1+channel*3];
        u8 high = data.samples[(i*3*num_channels)+2+channel*3];
        i32 intermediate = low | (mid << 8) | (high << 16);
        if (high & 0b10000000) intermediate |= 0xFF000000;
        f64 amplitude = (f64)intermediate / ((f64)(0b1 << 23));

        vs[i] = amplitude;
    }

    return vs;
}

f64 *read_32bps_float_data(smrt_arena_t *arena, wav_data_t data, u16 num_channels, u16 channel, u64 align_up_memoryn) {
    assert(num_channels != 0 && "num_channels must be >0");

    f64 *vs = smrt_arena_push(arena, ALIGN_UP_POW2(sizeof(f64) * data.sample_count, align_up_memoryn), true);
    if (!vs) return NULL;

    for (u64 i = 0; i < data.sample_count; i++) {
        u8   one = data.samples[(i*4*num_channels)+  channel*4];
        u8   two = data.samples[(i*4*num_channels)+1+channel*4];
        u8 three = data.samples[(i*4*num_channels)+2+channel*4];
        u8  four = data.samples[(i*4*num_channels)+3+channel*4];
        u32 intermediate = one | two << 8 | three << 16 | four << 24;

        memcpy(&vs[i], &intermediate, sizeof(u32));
    }

    return vs;
}

f64 *combine_channels(smrt_arena_t *arena, f64 **channels, u64 num_channels, u64 sample_count) {
    assert(num_channels != 0 && "num_channels must be >0");

    f64 *samples = SMRTA_ALLOC_ARRAY(arena, f64, num_channels * sample_count);

    for (u64 s = 0; s < sample_count; s++) {
        for (u64 c = 0; c < num_channels; c++) {
            samples[s*num_channels + c] = channels[c][s];
        }
    }

    return samples;
}

f32 *f64_to_f32(smrt_arena_t *arena, f64 *samples, u64 sample_count) {
    f32 *o = SMRTA_ALLOC_ARRAY(arena, f32, sample_count);

    for (u64 i = 0; i < sample_count; i++) {
        o[i] = (f32)samples[i];
    }

    return o;
}
