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

    wav_data_t data = load_wav_file(arena, "./tests/fixtures/sine.wav", &mchunk, &fmtchunk);

    u8 idcheck[4] = { 'f', 'm', 't', ' ' };
    cr_expect(memcmp(fmtchunk.chunk_id, idcheck, 4) == 0);
    cr_expect(fmtchunk.audio_format == 1);
    cr_expect(fmtchunk.num_channels == 1);
    cr_expect(fmtchunk.bits_per_sample == 16);
    cr_expect(data.sample_count == 48000);

    smrt_arena_destroy(arena);
}

Test(wav, sample_data_matches_source) {
    smrt_arena_t *arena = smrt_arena_create(MiB(4), KiB(4), false);

    wav_master_chunk_t mchunk;
    wav_fmt_chunk_t  fmtchunk;

    wav_data_t data = load_wav_file(arena, "./tests/fixtures/sine.wav", &mchunk, &fmtchunk);

    i16 expected[8] = { 0, 2147, 4242, 6291, 8185, 9935, 11580, 12979 };
    i16 *samples = (i16*)((u8*)data.samples + WAV_DATA_BASE_POS);

    for (int i = 0; i < 8; i++) {
        cr_expect_eq(samples[i], expected[i]);
    }

    smrt_arena_destroy(arena);
}

Test(wav, missing_file_returns_null) {
    smrt_arena_t *arena = smrt_arena_create(MiB(4), KiB(4), false);

    wav_master_chunk_t mchunk;
    wav_fmt_chunk_t  fmtchunk;

    wav_data_t data = load_wav_file(arena, "./tests/fixtures/does_not_exist.wav", &mchunk, &fmtchunk);

    cr_expect_eq(data.samples, NULL);

    smrt_arena_destroy(arena);
}

Test(wav, stereo_16bit) {
    smrt_arena_t *arena = smrt_arena_create(MiB(4), KiB(4), false);

    wav_master_chunk_t mchunk;
    wav_fmt_chunk_t  fmtchunk;

    wav_data_t data = load_wav_file(arena, "./tests/fixtures/stereo16.wav", &mchunk, &fmtchunk);

    cr_expect(fmtchunk.num_channels == 2);
    cr_expect(fmtchunk.bits_per_sample == 16);
    cr_expect(data.sample_count == 5);

    i16 *samples = (i16*)((u8*)data.samples + WAV_DATA_BASE_POS);
    cr_expect_eq(samples[0], 0);
    cr_expect_eq(samples[1], 0);
    cr_expect_eq(samples[2], 100);
    cr_expect_eq(samples[3], -100);

    smrt_arena_destroy(arena);
}

Test(wav, mono_8bit) {
    smrt_arena_t *arena = smrt_arena_create(MiB(4), KiB(4), false);

    wav_master_chunk_t mchunk;
    wav_fmt_chunk_t  fmtchunk;

    wav_data_t data = load_wav_file(arena, "./tests/fixtures/mono8.wav", &mchunk, &fmtchunk);

    cr_expect(fmtchunk.num_channels == 1);
    cr_expect(fmtchunk.bits_per_sample == 8);
    cr_expect(data.sample_count == 6);

    u8 *samples = (u8*)data.samples + WAV_DATA_BASE_POS;
    u8 expected[6] = { 0, 64, 128, 192, 255, 32 };
    for (int i = 0; i < 6; i++) {
        cr_expect_eq(samples[i], expected[i]);
    }

    smrt_arena_destroy(arena);
}

Test(wav, skips_unknown_chunk_before_fmt) {
    smrt_arena_t *arena = smrt_arena_create(MiB(4), KiB(4), false);

    wav_master_chunk_t mchunk;
    wav_fmt_chunk_t  fmtchunk;

    wav_data_t data = load_wav_file(arena, "./tests/fixtures/extra_chunk.wav", &mchunk, &fmtchunk);

    u8 idcheck[4] = { 'f', 'm', 't', ' ' };
    cr_expect(memcmp(fmtchunk.chunk_id, idcheck, 4) == 0);
    cr_expect(fmtchunk.num_channels == 1);
    cr_expect(fmtchunk.sample_rate == 22050);
    cr_expect(data.sample_count == 4);

    smrt_arena_destroy(arena);
}

Test(wav, odd_sized_chunk_before_fmt, .timeout = 2) {
    smrt_arena_t *arena = smrt_arena_create(MiB(4), KiB(4), false);

    wav_master_chunk_t mchunk;
    wav_fmt_chunk_t  fmtchunk;

    wav_data_t data = load_wav_file(arena, "./tests/fixtures/odd_chunk.wav", &mchunk, &fmtchunk);

    u8 idcheck[4] = { 'f', 'm', 't', ' ' };
    cr_expect(memcmp(fmtchunk.chunk_id, idcheck, 4) == 0);
    cr_expect(fmtchunk.sample_rate == 16000);
    cr_expect(data.sample_count == 4);

    smrt_arena_destroy(arena);
}

Test(wav, missing_data_chunk, .timeout = 2) {
    smrt_arena_t *arena = smrt_arena_create(MiB(4), KiB(4), false);

    wav_master_chunk_t mchunk;
    wav_fmt_chunk_t  fmtchunk;

    wav_data_t data = load_wav_file(arena, "./tests/fixtures/missing_data_chunk.wav", &mchunk, &fmtchunk);

    cr_expect_eq(data.samples, NULL);

    smrt_arena_destroy(arena);
}

Test(wav, arena_too_small_for_data) {
    smrt_arena_t *arena = smrt_arena_create(KiB(1), KiB(4), false);

    wav_master_chunk_t mchunk;
    wav_fmt_chunk_t  fmtchunk;

    wav_data_t data = load_wav_file(arena, "./tests/fixtures/sine.wav", &mchunk, &fmtchunk);

    cr_expect_eq(data.samples, NULL);
    cr_expect_eq(data.sample_count, 0);

    smrt_arena_destroy(arena);
}
