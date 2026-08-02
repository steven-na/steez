#include "wav.h"
#include "smrt_arena.h"

#include <assert.h>
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

wav_data_t load_wav_file(smrt_arena_t *arena, FILE *wav_file, wav_master_chunk_t *master_o, wav_fmt_chunk_t *format_o) {
    wav_data_t data = { 0 };

    if (!wav_file) return data;

    fread(master_o, sizeof(wav_master_chunk_t), 1, wav_file);

    if (seek_to_chunk(wav_file, "fmt ") == false) return data;
    fread(format_o, sizeof(wav_fmt_chunk_t), 1, wav_file);

    if (format_o->audio_format != 1) {
        perror("Attempted to read non pcm integer wave file.");
        return data;
    }

    if (seek_to_chunk(wav_file, "data") == false) return data;
    fseek(wav_file, 4, SEEK_CUR);

    u32 sampled_data_size;
    fread(&sampled_data_size, 4, 1, wav_file);

    data.samples = smrt_arena_push(arena, sampled_data_size, true);
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

f64 *read_8bps_data(smrt_arena_t *arena, wav_data_t data) {
    f64 *vs = SMRTA_ALLOC_ARRAY(arena, f64, data.sample_count);
    if (!vs) return NULL;

    for (u64 i = 0; i < data.sample_count; i++) {
        u8 sample = data.samples[i];
        f64 amplitude = (f64)(sample - 128) / (f64)(0b1<<7);

        vs[i] = amplitude;
    }

    return vs;
}

f64 *read_16bps_data(smrt_arena_t *arena, wav_data_t data) {
    f64 *vs = SMRTA_ALLOC_ARRAY(arena, f64, data.sample_count);
    if (!vs) return NULL;

    for (u64 i = 0; i < data.sample_count; i++) {
        u8  low = data.samples[ i*2]   ;
        u8 high = data.samples[(i*2)+1];
        i16 intermediate = low | (high << 8);
        f64 amplitude = (f64)intermediate / (f64)(0b1<<15);

        vs[i] = amplitude;
    }

    return vs;
}

f64 *read_24bps_data(smrt_arena_t *arena, wav_data_t data) {
    f64 *vs = SMRTA_ALLOC_ARRAY(arena, f64, data.sample_count);
    if (!vs) return NULL;

    for (u64 i = 0; i < data.sample_count; i++) {
        u8  low = data.samples[ i*3]   ;
        u8  mid = data.samples[(i*3)+1];
        u8 high = data.samples[(i*3)+2];
        i32 intermediate = low | (mid << 8) | (high << 16);
        if (high & 0b10000000) intermediate |= 0xFF000000;
        f64 amplitude = (f64)intermediate / ((f64)(0b1 << 23));

        vs[i] = amplitude;
    }

    return vs;
}

f64 *read_32bps_float_data(smrt_arena_t *arena, wav_data_t data) {
    f64 *vs = SMRTA_ALLOC_ARRAY(arena, f64, data.sample_count);
    if (!vs) return NULL;

    for (u64 i = 0; i < data.sample_count; i++) {
        u8   one = data.samples[ i*4   ];
        u8   two = data.samples[(i*4)+1];
        u8 three = data.samples[(i*4)+2];
        u8  four = data.samples[(i*4)+3];
        u32 intermediate = one | two << 8 | three << 16 | four << 24;

        memcpy(&vs[i], &intermediate, sizeof(u32));
    }

    return vs;
}
