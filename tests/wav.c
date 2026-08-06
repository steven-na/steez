#define _DEFAULT_SOURCE

#include "../src/wav.h"

#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <criterion/criterion.h>
#include <criterion/internal/assert.h>
#include <criterion/internal/test.h>
#include <criterion/redirect.h>
#include <string.h>

Test(wav, read_file) {
    smrt_arena_t *arena = smrt_arena_create(MiB(4), KiB(4), false);

    wav_master_chunk_t mchunk;
    wav_fmt_chunk_t  fmtchunk;

    FILE *wav_file = fopen("./tests/fixtures/sine.wav", "rb");
    cr_assert_not_null(wav_file);

    wav_data_t data = load_wav_file(arena, wav_file, &mchunk, &fmtchunk, 1);

    fclose(wav_file);

    u8 idcheck[4] = { 'f', 'm', 't', ' ' };
    cr_expect(memcmp(fmtchunk.chunk_id, idcheck, 4) == 0);
    cr_expect(fmtchunk.audio_format == 1);
    cr_expect(fmtchunk.num_channels == 1);
    cr_expect(fmtchunk.bits_per_sample == 16);
    cr_expect(data.sample_count == 48000);

    smrt_arena_destroy(arena);
}

Test(wav, read_sample_data_matches_source) {
    smrt_arena_t *arena = smrt_arena_create(MiB(4), KiB(4), false);

    wav_master_chunk_t mchunk;
    wav_fmt_chunk_t  fmtchunk;

    FILE *wav_file = fopen("./tests/fixtures/sine.wav", "rb");
    cr_assert_not_null(wav_file);

    wav_data_t data = load_wav_file(arena, wav_file, &mchunk, &fmtchunk, 1);

    fclose(wav_file);

    i16 expected[8] = { 0, 2147, 4242, 6291, 8185, 9935, 11580, 12979 };
    i16 *samples = (i16*)((u8*)data.samples);

    for (int i = 0; i < 8; i++) {
        cr_expect_eq(samples[i], expected[i]);
    }

    smrt_arena_destroy(arena);
}

Test(wav, read_missing_file_returns_null) {
    smrt_arena_t *arena = smrt_arena_create(MiB(4), KiB(4), false);

    wav_master_chunk_t mchunk;
    wav_fmt_chunk_t  fmtchunk;

    FILE *wav_file = fopen("./tests/fixtures/does_not_exist.wav", "rb");
    cr_assert_null(wav_file);

    wav_data_t data = load_wav_file(arena, wav_file, &mchunk, &fmtchunk, 1);

    cr_expect_eq(data.samples, NULL);

    smrt_arena_destroy(arena);
}

Test(wav, read_stereo_16bit) {
    smrt_arena_t *arena = smrt_arena_create(MiB(4), KiB(4), false);

    wav_master_chunk_t mchunk;
    wav_fmt_chunk_t  fmtchunk;

    FILE *wav_file = fopen("./tests/fixtures/stereo16.wav", "rb");
    cr_assert_not_null(wav_file);

    wav_data_t data = load_wav_file(arena, wav_file, &mchunk, &fmtchunk, 1);

    fclose(wav_file);

    cr_expect(fmtchunk.num_channels == 2);
    cr_expect(fmtchunk.bits_per_sample == 16);
    cr_expect(data.sample_count == 5);

    i16 *samples = (i16*)((u8*)data.samples);
    cr_expect_eq(samples[0], 0);
    cr_expect_eq(samples[1], 0);
    cr_expect_eq(samples[2], 100);
    cr_expect_eq(samples[3], -100);

    smrt_arena_destroy(arena);
}

Test(wav, read_mono_8bit) {
    smrt_arena_t *arena = smrt_arena_create(MiB(4), KiB(4), false);

    wav_master_chunk_t mchunk;
    wav_fmt_chunk_t  fmtchunk;

    FILE *wav_file = fopen("./tests/fixtures/mono8.wav", "rb");
    cr_assert_not_null(wav_file);

    wav_data_t data = load_wav_file(arena, wav_file, &mchunk, &fmtchunk, 1);

    fclose(wav_file);

    cr_expect(fmtchunk.num_channels == 1);
    cr_expect(fmtchunk.bits_per_sample == 8);
    cr_expect(data.sample_count == 6);

    u8 *samples = (u8*)data.samples;
    u8 expected[6] = { 0, 64, 128, 192, 255, 32 };
    for (int i = 0; i < 6; i++) {
        cr_expect_eq(samples[i], expected[i]);
    }

    smrt_arena_destroy(arena);
}

Test(wav, read_skips_unknown_chunk_before_fmt) {
    smrt_arena_t *arena = smrt_arena_create(MiB(4), KiB(4), false);

    wav_master_chunk_t mchunk;
    wav_fmt_chunk_t  fmtchunk;

    FILE *wav_file = fopen("./tests/fixtures/extra_chunk.wav", "rb");
    cr_assert_not_null(wav_file);

    wav_data_t data = load_wav_file(arena, wav_file, &mchunk, &fmtchunk, 1);

    fclose(wav_file);

    u8 idcheck[4] = { 'f', 'm', 't', ' ' };
    cr_expect(memcmp(fmtchunk.chunk_id, idcheck, 4) == 0);
    cr_expect(fmtchunk.num_channels == 1);
    cr_expect(fmtchunk.sample_rate == 22050);
    cr_expect(data.sample_count == 4);

    smrt_arena_destroy(arena);
}

Test(wav, read_odd_sized_chunk_before_fmt, .timeout = 2) {
    smrt_arena_t *arena = smrt_arena_create(MiB(4), KiB(4), false);

    wav_master_chunk_t mchunk;
    wav_fmt_chunk_t  fmtchunk;

    FILE *wav_file = fopen("./tests/fixtures/odd_chunk.wav", "rb");
    cr_assert_not_null(wav_file);

    wav_data_t data = load_wav_file(arena, wav_file, &mchunk, &fmtchunk, 1);

    fclose(wav_file);

    u8 idcheck[4] = { 'f', 'm', 't', ' ' };
    cr_expect(memcmp(fmtchunk.chunk_id, idcheck, 4) == 0);
    cr_expect(fmtchunk.sample_rate == 16000);
    cr_expect(data.sample_count == 4);

    smrt_arena_destroy(arena);
}

Test(wav, read_missing_data_chunk, .timeout = 2) {
    smrt_arena_t *arena = smrt_arena_create(MiB(4), KiB(4), false);

    wav_master_chunk_t mchunk;
    wav_fmt_chunk_t  fmtchunk;

    FILE *wav_file = fopen("./tests/fixtures/missing_data_chunk.wav", "rb");
    cr_assert_not_null(wav_file);

    wav_data_t data = load_wav_file(arena, wav_file, &mchunk, &fmtchunk, 1);

    fclose(wav_file);

    cr_expect_eq(data.samples, NULL);

    smrt_arena_destroy(arena);
}

Test(wav, read_arena_too_small_for_data) {
    smrt_arena_t *arena = smrt_arena_create(KiB(1), KiB(4), false);

    wav_master_chunk_t mchunk;
    wav_fmt_chunk_t  fmtchunk;

    FILE *wav_file = fopen("./tests/fixtures/sine.wav", "rb");
    cr_assert_not_null(wav_file);

    wav_data_t data = load_wav_file(arena, wav_file, &mchunk, &fmtchunk, 1);

    fclose(wav_file);

    cr_expect_eq(data.samples, NULL);
    cr_expect_eq(data.sample_count, 0);

    smrt_arena_destroy(arena);
}

Test(wav, generate_header) {
    smrt_arena_t *arena = smrt_arena_create(KiB(64), KiB(4), false);

    wav_fmt_chunk_t fmtchunk = make_wav_fmt_chunk(1, 44100, 8);

    u8 idcheck[4] = { 'f', 'm', 't', ' ' };
    cr_expect(memcmp(fmtchunk.chunk_id, idcheck, 4) == 0);
    cr_expect(fmtchunk.audio_format == 1);
    cr_expect(fmtchunk.num_channels == 1);
    cr_expect(fmtchunk.bits_per_sample == 8);

    smrt_arena_destroy(arena);
}

static char write_sine_path[] = "/tmp/steez_testwav_XXXXXX";
static int write_sine_fd;

void write_sine_setup(void) {
    write_sine_fd = mkstemp(write_sine_path);
    cr_assert_geq(write_sine_fd, 0, "mkstemp failed");
}

void write_sine_teardown(void) {
    unlink(write_sine_path);
    close(write_sine_fd);
}

static char non_pcm_path[] = "/tmp/steez_testwav_nonpcm_XXXXXX";
static int non_pcm_fd;

void non_pcm_setup(void) {
    non_pcm_fd = mkstemp(non_pcm_path);
    cr_assert_geq(non_pcm_fd, 0, "mkstemp failed");
    cr_redirect_stderr(); // load_wav_file perror()s on the rejected format; that's expected
}

void non_pcm_teardown(void) {
    unlink(non_pcm_path);
    close(non_pcm_fd);
}

Test(wav, read_rejects_non_pcm_format, .init = non_pcm_setup, .fini = non_pcm_teardown) {
    FILE *wav_file = fdopen(non_pcm_fd, "wb");
    cr_assert_not_null(wav_file);

    smrt_arena_t *arena = smrt_arena_create(KiB(64), KiB(4), false);

    u8 sample = 128;
    wav_data_t data = { .samples = &sample, .sample_count = 1 };

    wav_fmt_chunk_t fmtchunk = make_wav_fmt_chunk(1, 8000, 8);
    fmtchunk.audio_format = 6; // A-law, neither PCM integer (1) nor IEEE float (3)

    cr_assert(write_wav_file(wav_file, &fmtchunk, data));

    fclose(wav_file);

    FILE *readback_file = fopen(non_pcm_path, "rb");
    cr_assert_not_null(readback_file);

    wav_master_chunk_t mchunk;
    wav_fmt_chunk_t  read_fmtchunk;

    wav_data_t read_data = load_wav_file(arena, readback_file, &mchunk, &read_fmtchunk, 1);

    fclose(readback_file);

    cr_expect_eq(read_fmtchunk.audio_format, 6);
    cr_expect_eq(read_data.samples, NULL);

    smrt_arena_destroy(arena);
}


Test(wav, write_sine, .init = write_sine_setup, .fini = write_sine_teardown) {
    FILE *wav_file = fdopen(write_sine_fd, "wb");
    cr_assert_not_null(wav_file);

    smrt_arena_t *arena = smrt_arena_create(KiB(256), KiB(4), false);

    u64 sample_count = 44100;
    wav_data_t data;
    data.samples = smrt_arena_push(arena, sizeof(u8) * sample_count, true);
    data.sample_count = sample_count;

    const f64 freq = 220.0;
    for (u64 i = 0; i < sample_count; i++) {
        f64 progress =  (f64)i / sample_count;

        data.samples[i] = (u8)(((sin(progress * freq) + 1.0 ) / 2.0 ) * UINT8_MAX);
    }

    wav_fmt_chunk_t fmtchunk = make_wav_fmt_chunk(1, sample_count, 8);

    cr_assert(write_wav_file(wav_file, &fmtchunk, data));

    fclose(wav_file);

    FILE *readback_file = fopen(write_sine_path, "rb");
    cr_assert_not_null(readback_file);

    wav_master_chunk_t mchunk;
    wav_fmt_chunk_t  read_fmtchunk;

    wav_data_t read_data = load_wav_file(arena, readback_file, &mchunk, &read_fmtchunk, 1);

    fclose(readback_file);

    u8 riffcheck[4] = { 'R', 'I', 'F', 'F' };
    u8 wavecheck[4] = { 'W', 'A', 'V', 'E' };
    cr_expect(memcmp(mchunk.chunk_id, riffcheck, 4) == 0);
    cr_expect(memcmp(mchunk.file_format_id, wavecheck, 4) == 0);
    cr_expect_eq(mchunk.file_size, sizeof(wav_master_chunk_t) + sizeof(wav_fmt_chunk_t) + sample_count);

    u8 fmtcheck[4] = { 'f', 'm', 't', ' ' };
    cr_expect(memcmp(read_fmtchunk.chunk_id, fmtcheck, 4) == 0);
    cr_expect_eq(read_fmtchunk.audio_format, 1);
    cr_expect_eq(read_fmtchunk.num_channels, 1);
    cr_expect_eq(read_fmtchunk.sample_rate, sample_count);
    cr_expect_eq(read_fmtchunk.bits_per_sample, 8);

    cr_assert_not_null(read_data.samples);
    cr_expect_eq(read_data.sample_count, sample_count);

    u8 *read_samples = (u8*)read_data.samples;
    for (u64 i = 0; i < sample_count; i++) {
        cr_expect_eq(read_samples[i], data.samples[i]);
    }

    smrt_arena_destroy(arena);
}

static char float32_wav_path[] = "/tmp/steez_testwav_f32_XXXXXX";
static int float32_wav_fd;

void float32_wav_setup(void) {
    float32_wav_fd = mkstemp(float32_wav_path);
    cr_assert_geq(float32_wav_fd, 0, "mkstemp failed");
}

void float32_wav_teardown(void) {
    unlink(float32_wav_path);
    close(float32_wav_fd);
}

Test(wav, read_32bps_float_round_trip, .init = float32_wav_setup, .fini = float32_wav_teardown) {
    FILE *wav_file = fdopen(float32_wav_fd, "wb");
    cr_assert_not_null(wav_file);

    smrt_arena_t *arena = smrt_arena_create(KiB(64), KiB(4), false);

    f32 expected[5] = { 0.0f, 0.5f, -0.5f, 1.0f, -1.0f };
    u64 sample_count = 5;

    wav_data_t data;
    data.samples = (u8*)expected;
    data.sample_count = sample_count;

    wav_fmt_chunk_t fmtchunk = make_wav_fmt_chunk(1, 44100, 32);
    fmtchunk.audio_format = 3; // IEEE 754 float

    cr_assert(write_wav_file(wav_file, &fmtchunk, data));

    fclose(wav_file);

    FILE *readback_file = fopen(float32_wav_path, "rb");
    cr_assert_not_null(readback_file);

    wav_master_chunk_t mchunk;
    wav_fmt_chunk_t  read_fmtchunk;

    wav_data_t read_data = load_wav_file(arena, readback_file, &mchunk, &read_fmtchunk, sizeof(f64));

    fclose(readback_file);

    cr_expect_eq(read_fmtchunk.audio_format, 3);
    cr_expect_eq(read_fmtchunk.bits_per_sample, 32);
    cr_assert_not_null(read_data.samples);
    cr_expect_eq(read_data.sample_count, sample_count);

    f64 *decoded = read_32bps_float_data(arena, read_data, 1, 0, sizeof(f64));
    cr_assert_not_null(decoded);

    for (u64 i = 0; i < sample_count; i++) {
        cr_expect(F64_EQ(decoded[i], (f64)expected[i], 1e-6));
    }

    smrt_arena_destroy(arena);
}

static char pcm24_wav_path[] = "/tmp/steez_testwav_24bit_XXXXXX";
static int pcm24_wav_fd;

void pcm24_wav_setup(void) {
    pcm24_wav_fd = mkstemp(pcm24_wav_path);
    cr_assert_geq(pcm24_wav_fd, 0, "mkstemp failed");
}

void pcm24_wav_teardown(void) {
    unlink(pcm24_wav_path);
    close(pcm24_wav_fd);
}

Test(wav, read_24bps_round_trip, .init = pcm24_wav_setup, .fini = pcm24_wav_teardown) {
    FILE *wav_file = fdopen(pcm24_wav_fd, "wb");
    cr_assert_not_null(wav_file);

    smrt_arena_t *arena = smrt_arena_create(KiB(64), KiB(4), false);

    i32 expected[4] = { 0, 4194304 /* +0.5 */, -4194304 /* -0.5 */, 8388607 /* max */ };
    u64 sample_count = 4;

    u8 raw[4*3];
    for (u64 i = 0; i < sample_count; i++) {
        i32 v = expected[i];
        raw[i*3+0] = (u8)( v        & 0xFF);
        raw[i*3+1] = (u8)((v >> 8)  & 0xFF);
        raw[i*3+2] = (u8)((v >> 16) & 0xFF);
    }

    wav_data_t data;
    data.samples = raw;
    data.sample_count = sample_count;

    wav_fmt_chunk_t fmtchunk = make_wav_fmt_chunk(1, 44100, 24);

    cr_assert(write_wav_file(wav_file, &fmtchunk, data));

    fclose(wav_file);

    FILE *readback_file = fopen(pcm24_wav_path, "rb");
    cr_assert_not_null(readback_file);

    wav_master_chunk_t mchunk;
    wav_fmt_chunk_t  read_fmtchunk;

    wav_data_t read_data = load_wav_file(arena, readback_file, &mchunk, &read_fmtchunk, sizeof(f64));

    fclose(readback_file);

    cr_expect_eq(read_fmtchunk.bits_per_sample, 24);
    cr_assert_not_null(read_data.samples);
    cr_expect_eq(read_data.sample_count, sample_count);

    f64 *decoded = read_24bps_data(arena, read_data, 1, 0, sizeof(f64));
    cr_assert_not_null(decoded);

    for (u64 i = 0; i < sample_count; i++) {
        f64 expected_amplitude = (f64)expected[i] / (f64)(0b1 << 23);
        cr_expect(F64_EQ(decoded[i], expected_amplitude, 1e-9));
    }

    smrt_arena_destroy(arena);
}
