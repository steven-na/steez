#include "wav.h"
#include "smrt_arena.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

void seek_to_chunk(FILE *file, char *chunk_name) {
    char cur_name[4];
     u32  chunk_size;

    fseek(file, sizeof(wav_master_chunk_t), SEEK_SET);
    while (true) {
        fread(cur_name, 4, 1, file);
        fread(&chunk_size, 4, 1, file);

        if (strcmp(cur_name, chunk_name) == 0) {
            break;
        }

        fseek(file, chunk_size, SEEK_CUR);
    }

    fseek(file, -8, SEEK_CUR);
}

wav_data_t *load_wav_file(smrt_arena_t *arena, char *filename, wav_master_chunk_t *master_o, wav_fmt_chunk_t *format_o) {
    FILE *wav_file = fopen(filename, "rb");

    if (!wav_file) {
        return NULL;
    }

    fread(master_o, sizeof(wav_master_chunk_t), 1, wav_file);

    seek_to_chunk(wav_file, "fmt ");
    fread(format_o, sizeof(wav_fmt_chunk_t), 1, wav_file);

    seek_to_chunk(wav_file, "data");
    fseek(wav_file, 4, SEEK_CUR);
    u32 sampled_data_size;
    fread(&sampled_data_size, 4, 1, wav_file);

    wav_data_t *data = smrt_arena_push(arena, sizeof(wav_data_t) + sampled_data_size, true);
    fread((u8*)data+WAV_DATA_BASE_POS, sampled_data_size, 1, wav_file);
    data->sample_count = sampled_data_size / ((format_o->bits_per_sample / 8) * format_o->num_channels);

    return data;
}

void write_wav_file(char *filename, wav_fmt_chunk_t *header_i, wav_data_t *data_i);
wav_fmt_chunk_t make_wav_fmt_chunk(u32 num_channels, u32 sample_rate, u16 bits_per_sample, wav_data_t *data_i);
