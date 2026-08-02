#include "../src/dft.h"

#include <math.h>
#include <stddef.h>
#include <stdint.h>

#include <criterion/criterion.h>
#include <criterion/internal/assert.h>
#include <criterion/internal/test.h>

f64 *mock_amplitude_data(smrt_arena_t *arena, f64 *freqs, f64 *amps, u64 n, f64 duration, u64 sample_count) {
    f64 *samples = SMRTA_ALLOC_ARRAY(arena, f64, sample_count);

    for (u64 f = 0; f < n; f++) {
        f64 freq = freqs[f];
        f64  amp =  amps[f];
        for (u64 i = 0; i < sample_count; i++) {
            f64 t = ((f64)i/sample_count) * duration;
            samples[i] += cos(freq * t * 2.0 * PI) * amp;
        }
    }

    return samples;
}

Test(dft, dc_component) {
    smrt_arena_t *arena = smrt_arena_create(KiB(16), KiB(4), false);

    u64 sample_count = 16, sample_rate = 16;
    f64 freqs[1] = { 0.0 };
    f64  amps[1] = { 3.0 };
    f64 *samples = mock_amplitude_data(arena, freqs, amps, 1, (f64)sample_count/sample_rate, sample_count);

    dft_data_t dft = discrete_fourier_transform(arena, samples, sample_count, sample_rate);

    cr_expect_eq(dft.frequencies[0], 0.0);
    cr_expect(F64_EQ(dft.amplitudes[0], 3.0, 1e-9));

    for (u64 i = 1; i < dft.freq_count; i++) {
        cr_expect(F64_EQ(dft.amplitudes[i], 0.0, 1e-9));
    }

    smrt_arena_destroy(arena);
}

Test(dft, single_tone_at_bin) {
    smrt_arena_t *arena = smrt_arena_create(KiB(16), KiB(4), false);

    u64 sample_count = 16, sample_rate = 16;
    u64 k = 3;
    f64 freqs[1] = { (f64)k * sample_rate / sample_count };
    f64  amps[1] = { 2.0 };
    f64 *samples = mock_amplitude_data(arena, freqs, amps, 1, (f64)sample_count/sample_rate, sample_count);

    dft_data_t dft = discrete_fourier_transform(arena, samples, sample_count, sample_rate);

    cr_expect(F64_EQ(dft.amplitudes[k], 2.0, 1e-9));
    cr_expect(F64_EQ(dft.phases[k], 0.0, 1e-9));

    for (u64 i = 0; i < dft.freq_count; i++) {
        if (i == k) continue;
        cr_expect(F64_EQ(dft.amplitudes[i], 0.0, 1e-9));
    }

    smrt_arena_destroy(arena);
}

Test(dft, separates_superposed_tones) {
    smrt_arena_t *arena = smrt_arena_create(KiB(16), KiB(4), false);

    u64 sample_count = 16, sample_rate = 16;
    u64 k1 = 2, k2 = 5;
    f64 freqs[2] = { (f64)k1 * sample_rate / sample_count, (f64)k2 * sample_rate / sample_count };
    f64  amps[2] = { 1.5, 0.5 };
    f64 *samples = mock_amplitude_data(arena, freqs, amps, 2, (f64)sample_count/sample_rate, sample_count);

    dft_data_t dft = discrete_fourier_transform(arena, samples, sample_count, sample_rate);

    cr_expect(F64_EQ(dft.amplitudes[k1], 1.5, 1e-9));
    cr_expect(F64_EQ(dft.amplitudes[k2], 0.5, 1e-9));

    for (u64 i = 0; i < dft.freq_count; i++) {
        if (i == k1 || i == k2) continue;
        cr_expect(F64_EQ(dft.amplitudes[i], 0.0, 1e-9));
    }

    smrt_arena_destroy(arena);
}

Test(dft, nyquist_bin_uses_unit_coefficient) {
    smrt_arena_t *arena = smrt_arena_create(KiB(16), KiB(4), false);

    u64 sample_count = 16, sample_rate = 16;
    f64 freqs[1] = { (f64)sample_rate / 2.0 };
    f64  amps[1] = { 1.0 };
    f64 *samples = mock_amplitude_data(arena, freqs, amps, 1, (f64)sample_count/sample_rate, sample_count);

    dft_data_t dft = discrete_fourier_transform(arena, samples, sample_count, sample_rate);

    cr_expect_eq(dft.freq_count, sample_count/2 + 1);
    cr_expect(F64_EQ(dft.amplitudes[dft.freq_count-1], 1.0, 1e-9));

    smrt_arena_destroy(arena);
}

Test(dft, odd_sample_count_has_no_nyquist_bin) {
    smrt_arena_t *arena = smrt_arena_create(KiB(16), KiB(4), false);

    u64 sample_count = 15, sample_rate = 15;
    u64 last_bin = sample_count/2;
    f64 freqs[1] = { (f64)last_bin };
    f64  amps[1] = { 1.0 };
    f64 *samples = mock_amplitude_data(arena, freqs, amps, 1, (f64)sample_count/sample_rate, sample_count);

    dft_data_t dft = discrete_fourier_transform(arena, samples, sample_count, sample_rate);

    cr_expect_eq(dft.freq_count, last_bin + 1);
    cr_expect(F64_EQ(dft.amplitudes[last_bin], 1.0, 1e-9));

    smrt_arena_destroy(arena);
}

Test(dft, data_to_wav_dc_amplitude) {
    smrt_arena_t *arena = smrt_arena_create(KiB(16), KiB(4), false);

    dft_data_t dft = { .freq_count = 1 };
    dft.frequencies = SMRTA_ALLOC_ARRAY(arena, f64, 1);
    dft.amplitudes  = SMRTA_ALLOC_ARRAY(arena, f64, 1);
    dft.phases      = SMRTA_ALLOC_ARRAY(arena, f64, 1);
    dft.frequencies[0] = 0.0;
    dft.amplitudes[0]  = 0.5;
    dft.phases[0]      = 0.0;

    wav_data_t wav = dft_data_to_wav(arena, dft, 8, 1.0);

    cr_assert_eq(wav.sample_count, 8);
    for (u64 i = 0; i < wav.sample_count; i++) {
        cr_expect_eq(wav.samples[i], 190);
    }

    smrt_arena_destroy(arena);
}

Test(dft, data_to_wav_respects_phase) {
    smrt_arena_t *arena = smrt_arena_create(KiB(16), KiB(4), false);

    dft_data_t dft = { .freq_count = 1 };
    dft.frequencies = SMRTA_ALLOC_ARRAY(arena, f64, 1);
    dft.amplitudes  = SMRTA_ALLOC_ARRAY(arena, f64, 1);
    dft.phases      = SMRTA_ALLOC_ARRAY(arena, f64, 1);
    dft.frequencies[0] = 0.0;
    dft.amplitudes[0]  = 0.5;
    dft.phases[0]      = PI;

    wav_data_t wav = dft_data_to_wav(arena, dft, 8, 1.0);

    cr_assert_eq(wav.sample_count, 8);
    for (u64 i = 0; i < wav.sample_count; i++) {
        cr_expect_eq(wav.samples[i], 63);
    }

    smrt_arena_destroy(arena);
}

Test(dft, data_to_wav_varies_with_time) {
    smrt_arena_t *arena = smrt_arena_create(KiB(16), KiB(4), false);

    dft_data_t dft = { .freq_count = 1 };
    dft.frequencies = SMRTA_ALLOC_ARRAY(arena, f64, 1);
    dft.amplitudes  = SMRTA_ALLOC_ARRAY(arena, f64, 1);
    dft.phases      = SMRTA_ALLOC_ARRAY(arena, f64, 1);
    dft.frequencies[0] = 1.0;
    dft.amplitudes[0]  = 1.0;
    dft.phases[0]      = 0.0;

    wav_data_t wav = dft_data_to_wav(arena, dft, 8, 1.0);

    cr_assert_eq(wav.sample_count, 8);
    cr_expect_eq(wav.samples[0], 254);
    cr_expect_eq(wav.samples[4], 0);

    smrt_arena_destroy(arena);
}

Test(dft, stft_segments_match_direct_transform) {
    smrt_arena_t *arena = smrt_arena_create(KiB(64), KiB(4), false);

    u64 sample_count = 12, sample_rate = 12, samples_per_segment = 5;
    f64 freqs[1] = { 2.0 };
    f64  amps[1] = { 1.0 };
    f64 *samples = mock_amplitude_data(arena, freqs, amps, 1, (f64)sample_count/sample_rate, sample_count);

    stft_data_t stft = short_time_fourier_transform(arena, samples_per_segment, samples, sample_count, sample_rate);

    cr_assert_eq(stft.segment_count, 3);

    u64 expected_starts[3] = { 0, 5, 10 };
    u64 expected_counts[3] = { 5, 5, 2 };

    for (u64 s = 0; s < stft.segment_count; s++) {
        stft_segment_t seg = stft.segments[s];
        cr_expect_eq(seg.start_index, expected_starts[s]);
        cr_expect_eq(seg.sample_count, expected_counts[s]);

        dft_data_t direct = discrete_fourier_transform(arena, samples + seg.start_index, seg.sample_count, sample_rate);

        cr_assert_eq(seg.data.freq_count, direct.freq_count);
        for (u64 i = 0; i < direct.freq_count; i++) {
            cr_expect(F64_EQ(seg.data.frequencies[i], direct.frequencies[i], 1e-9));
            cr_expect(F64_EQ(seg.data.amplitudes[i],  direct.amplitudes[i],  1e-9));
            cr_expect(F64_EQ(seg.data.phases[i],      direct.phases[i],      1e-9));
        }
    }

    smrt_arena_destroy(arena);
}

Test(dft, stft_data_to_wav_concatenates_segments) {
    smrt_arena_t *arena = smrt_arena_create(KiB(16), KiB(4), false);

    stft_segment_t segs[2] = { 0 };

    segs[0].start_index = 0;
    segs[0].sample_count = 4;
    segs[0].data.freq_count = 1;
    segs[0].data.frequencies = SMRTA_ALLOC_ARRAY(arena, f64, 1);
    segs[0].data.amplitudes  = SMRTA_ALLOC_ARRAY(arena, f64, 1);
    segs[0].data.phases      = SMRTA_ALLOC_ARRAY(arena, f64, 1);
    segs[0].data.amplitudes[0] = 0.5;

    segs[1].start_index = 4;
    segs[1].sample_count = 6;
    segs[1].data.freq_count = 1;
    segs[1].data.frequencies = SMRTA_ALLOC_ARRAY(arena, f64, 1);
    segs[1].data.amplitudes  = SMRTA_ALLOC_ARRAY(arena, f64, 1);
    segs[1].data.phases      = SMRTA_ALLOC_ARRAY(arena, f64, 1);
    segs[1].data.amplitudes[0] = -0.5;

    stft_data_t stft = {
        .segments = segs,
        .segment_count = 2,
        .sample_rate = 10,
    };

    wav_data_t wav = stft_data_to_wav(arena, stft, 10);

    cr_assert_eq(wav.sample_count, 10);
    for (u64 i = 0; i < 4; i++) cr_expect_eq(wav.samples[i], 190);
    for (u64 i = 4; i < 10; i++) cr_expect_eq(wav.samples[i], 63);

    smrt_arena_destroy(arena);
}
