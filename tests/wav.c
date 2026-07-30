#include "../src/wav.h"

#include <stddef.h>
#include <stdio.h>

#include <criterion/criterion.h>
#include <criterion/internal/assert.h>
#include <criterion/internal/test.h>
#include <string.h>

Test(wav, read_file) {
    smrt_arena_t *arena = smrt_arena_create(MiB(4), KiB(4), false);

    wav_master_chunk_t mchunk;
    wav_fmt_chunk_t  fmtchunk;

    wav_data_t *data = load_wav_file(arena, "./tests/fixtures/sine.wav", &mchunk, &fmtchunk);

    u8 idcheck[4] = { 'f', 'm', 't', ' ' };
    cr_expect(memcmp(fmtchunk.chunk_id, idcheck, 4) == 0);
    cr_expect(fmtchunk.audio_format == 1);
    cr_expect(fmtchunk.num_channels == 1);
    cr_expect(fmtchunk.bits_per_sample == 16);
    cr_expect(data->sample_count == 48000);

    smrt_arena_destroy(arena);
}
