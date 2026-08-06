#include "../src/dft.h"

#include <math.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>

#include <criterion/criterion.h>
#include <criterion/internal/assert.h>
#include <criterion/internal/test.h>
#include <criterion/redirect.h>

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

    u64 sample_count = 16, sample_rate = 16, window_size = 8, hop_size = 4;
    f64 freqs[1] = { 2.0 };
    f64  amps[1] = { 1.0 };
    f64 *samples = mock_amplitude_data(arena, freqs, amps, 1, (f64)sample_count/sample_rate, sample_count);

    stft_data_t stft = short_time_fourier_transform(arena, window_size, hop_size, samples, sample_count, sample_rate);

    cr_assert_eq(stft.segment_count, 3);

    u64 expected_starts[3] = { 0, 4, 8 };

    // short_time_fourier_transform hann-windows each segment before transforming it, then
    // scales amplitudes by 1/HANN_COHERENT_GAIN (0.5) to compensate for the window's average
    // attenuation. Mirror both steps here to build a comparable direct transform of the same
    // windowed data, rather than comparing against a direct transform of the raw slice.
    for (u64 s = 0; s < stft.segment_count; s++) {
        stft_segment_t seg = stft.segments[s];
        cr_expect_eq(seg.start_index, expected_starts[s]);
        cr_expect_eq(seg.sample_count, window_size);

        f64 windowed[8];
        for (u64 h = 0; h < window_size; h++) {
            f64 hann = 0.5 * (1.0 - cos((f64)h / window_size * 2.0 * PI));
            windowed[h] = samples[seg.start_index + h] * hann;
        }

        dft_data_t direct = discrete_fourier_transform(arena, windowed, window_size, sample_rate);

        cr_assert_eq(seg.data.freq_count, direct.freq_count);
        for (u64 i = 0; i < direct.freq_count; i++) {
            f64 expected_amplitude = direct.amplitudes[i] / 0.5; // undo HANN_COHERENT_GAIN

            cr_expect(F64_EQ(seg.data.frequencies[i], direct.frequencies[i], 1e-9));
            cr_expect(F64_EQ(seg.data.amplitudes[i],  expected_amplitude,    1e-9));

            // phase is only meaningful at bins with non-negligible amplitude. Compare
            // angles modulo 2*PI: atan2 can return either +PI or -PI for the same angle,
            // which a plain subtraction would wrongly see as a ~2*PI difference.
            if (direct.amplitudes[i] > 1e-6) {
                f64 phase_diff = seg.data.phases[i] - direct.phases[i];
                phase_diff -= 2.0 * PI * round(phase_diff / (2.0 * PI));
                cr_expect(F64_EQ(phase_diff, 0.0, 1e-9));
            }
        }
    }

    smrt_arena_destroy(arena);
}

Test(dft, stft_handles_unaligned_sample_count_with_wav_load_style_buffer) {
    smrt_arena_t *arena = smrt_arena_create(KiB(64), KiB(4), false);

    u64 sample_count = 13, sample_rate = 16, window_size = 8, hop_size = 4;

    // Mirrors how wav_load/load_wav_file size their sample buffer: byte-aligned to
    // STFT_SAMPLE_ALIGN_BYTES(window_size) and zero-initialized, not element-aligned.
    f64 *samples = smrt_arena_push(arena, ALIGN_UP_POW2(sample_count * sizeof(f64), STFT_SAMPLE_ALIGN_BYTES(window_size)), true);
    for (u64 i = 0; i < sample_count; i++) samples[i] = (f64)(i + 1);

    stft_data_t stft = short_time_fourier_transform(arena, window_size, hop_size, samples, sample_count, sample_rate);

    cr_assert_eq(stft.segment_count, 3);
    cr_assert_neq(stft.segments, NULL);

    smrt_arena_destroy(arena);
}

Test(dft, fast_fourier_transform_matches_direct_transform) {
    smrt_arena_t *arena = smrt_arena_create(KiB(16), KiB(4), false);

    u64 sample_count = 16, sample_rate = 16;
    u64 k1 = 2, k2 = 5;
    f64 freqs[2] = { (f64)k1 * sample_rate / sample_count, (f64)k2 * sample_rate / sample_count };
    f64  amps[2] = { 1.5, 0.5 };
    f64 *samples = mock_amplitude_data(arena, freqs, amps, 2, (f64)sample_count/sample_rate, sample_count);

    dft_data_t fft = fast_fourier_transform(arena, samples, sample_count, sample_rate, NULL, 0);
    dft_data_t dft = discrete_fourier_transform(arena, samples, sample_count, sample_rate);

    cr_assert_eq(fft.freq_count, dft.freq_count);
    for (u64 i = 0; i < dft.freq_count; i++) {
        cr_expect(F64_EQ(fft.frequencies[i], dft.frequencies[i], 1e-9));
        cr_expect(F64_EQ(fft.amplitudes[i],  dft.amplitudes[i],  1e-9));
    }

    // phase is only meaningful at bins with non-negligible amplitude
    cr_expect(F64_EQ(fft.phases[k1], dft.phases[k1], 1e-9));
    cr_expect(F64_EQ(fft.phases[k2], dft.phases[k2], 1e-9));

    smrt_arena_destroy(arena);
}

Test(dft, inverse_fast_fourier_transform_recovers_time_domain) {
    smrt_arena_t *arena = smrt_arena_create(KiB(16), KiB(4), false);

    u64 sample_count = 8;

    // single-bin spectrum: X_1 = 8 + 0i, all other bins zero
    f64 real[8] = { 0 };
    f64 imag[8] = { 0 };
    real[1] = 8.0;

    f64 *real_o, *imag_o;
    inverse_fast_fourier_transform(arena, real, imag, sample_count, &real_o, &imag_o, NULL, 0);

    for (u64 n = 0; n < sample_count; n++) {
        f64 angle = 2.0 * PI * (f64)n / (f64)sample_count;
        cr_expect(F64_EQ(real_o[n], 8.0 * cos(angle), 1e-9));
        cr_expect(F64_EQ(imag_o[n], 8.0 * sin(angle), 1e-9));
    }

    // input must be untouched
    cr_expect(F64_EQ(real[1], 8.0, 1e-9));
    for (u64 n = 0; n < sample_count; n++) {
        if (n != 1) cr_expect(F64_EQ(real[n], 0.0, 1e-9));
        cr_expect(F64_EQ(imag[n], 0.0, 1e-9));
    }

    smrt_arena_destroy(arena);
}

Test(dft, inverse_fast_fourier_transform_skips_null_outputs) {
    smrt_arena_t *arena = smrt_arena_create(KiB(16), KiB(4), false);

    u64 sample_count = 8;

    f64 real[8] = { 0 };
    f64 imag[8] = { 0 };
    real[1] = 8.0;

    f64 *real_o;
    inverse_fast_fourier_transform(arena, real, imag, sample_count, &real_o, NULL, NULL, 0);

    cr_assert_not_null(real_o);
    for (u64 n = 0; n < sample_count; n++) {
        f64 angle = 2.0 * PI * (f64)n / (f64)sample_count;
        cr_expect(F64_EQ(real_o[n], 8.0 * cos(angle), 1e-9));
    }

    smrt_arena_destroy(arena);
}

Test(dft, reconstruct_spectrum_mirrors_conjugate) {
    smrt_arena_t *arena = smrt_arena_create(KiB(16), KiB(4), false);

    u64 sample_count = 8;

    dft_data_t data = { .freq_count = 5 };
    data.frequencies = SMRTA_ALLOC_ARRAY(arena, f64, 5);
    data.amplitudes  = SMRTA_ALLOC_ARRAY(arena, f64, 5);
    data.phases      = SMRTA_ALLOC_ARRAY(arena, f64, 5);

    data.amplitudes[0] = 2.0; data.phases[0] = 0.0;
    data.amplitudes[1] = 1.0; data.phases[1] = PI / 4.0;
    data.amplitudes[2] = 0.6; data.phases[2] = PI / 3.0;
    data.amplitudes[3] = 0.4; data.phases[3] = -PI / 6.0;
    data.amplitudes[4] = 0.8; data.phases[4] = PI;

    f64 *real_o, *imag_o;
    reconstruct_spectrum(arena, &data, sample_count, &real_o, &imag_o);

    // DC and Nyquist bins keep their full amplitude
    cr_expect(F64_EQ(real_o[0], 2.0, 1e-9));
    cr_expect(F64_EQ(imag_o[0], 0.0, 1e-9));
    cr_expect(F64_EQ(real_o[4], 0.8 * cos(PI), 1e-9));
    cr_expect(F64_EQ(imag_o[4], 0.8 * sin(PI), 1e-9));

    // interior bins are halved before being placed on the unit circle
    cr_expect(F64_EQ(real_o[1], 0.5 * cos(PI / 4.0), 1e-9));
    cr_expect(F64_EQ(imag_o[1], 0.5 * sin(PI / 4.0), 1e-9));

    // mirrored bins are complex conjugates of their positive-frequency counterpart
    cr_expect(F64_EQ(real_o[5], real_o[3], 1e-9));
    cr_expect(F64_EQ(imag_o[5], -imag_o[3], 1e-9));
    cr_expect(F64_EQ(real_o[6], real_o[2], 1e-9));
    cr_expect(F64_EQ(imag_o[6], -imag_o[2], 1e-9));
    cr_expect(F64_EQ(real_o[7], real_o[1], 1e-9));
    cr_expect(F64_EQ(imag_o[7], -imag_o[1], 1e-9));

    smrt_arena_destroy(arena);
}

Test(dft, data_to_wav_clamps_out_of_range_amplitude) {
    smrt_arena_t *arena = smrt_arena_create(KiB(16), KiB(4), false);

    // two in-phase DC-like bins summing to 1.6, well outside [-1, 1]
    dft_data_t dft = { .freq_count = 2 };
    dft.frequencies = SMRTA_ALLOC_ARRAY(arena, f64, 2);
    dft.amplitudes  = SMRTA_ALLOC_ARRAY(arena, f64, 2);
    dft.phases      = SMRTA_ALLOC_ARRAY(arena, f64, 2);
    dft.frequencies[0] = 0.0; dft.amplitudes[0] = 0.8; dft.phases[0] = 0.0;
    dft.frequencies[1] = 0.0; dft.amplitudes[1] = 0.8; dft.phases[1] = 0.0;

    wav_data_t wav = dft_data_to_wav(arena, dft, 8, 1.0);

    cr_assert_eq(wav.sample_count, 8);
    for (u64 i = 0; i < wav.sample_count; i++) {
        // matches the clamped-amp=1.0 case in data_to_wav_varies_with_time:
        // (1.0 + 1.0) * (UINT8_MAX/2 truncated to 127) = 254
        cr_expect_eq(wav.samples[i], 254);
    }

    smrt_arena_destroy(arena);
}

Test(dft, stft_rejects_zero_hop_size, .signal = SIGABRT) {
    cr_redirect_stderr(); // assert() prints to stderr before aborting; that's expected

    smrt_arena_t *arena = smrt_arena_create(KiB(16), KiB(4), false);

    f64 samples[8] = { 0 };
    short_time_fourier_transform(arena, 8, 0, samples, 8, 8);
}

Test(dft, stft_rejects_zero_sample_count, .signal = SIGABRT) {
    cr_redirect_stderr(); // assert() prints to stderr before aborting; that's expected

    smrt_arena_t *arena = smrt_arena_create(KiB(16), KiB(4), false);

    f64 samples[1] = { 0 };
    short_time_fourier_transform(arena, 8, 4, samples, 0, 8);
}

Test(dft, reconstruct_spectrum_rejects_mismatched_freq_count, .signal = SIGABRT) {
    cr_redirect_stderr(); // assert() prints to stderr before aborting; that's expected

    smrt_arena_t *arena = smrt_arena_create(KiB(16), KiB(4), false);

    // sample_count=8 expects freq_count == 5; give it 3 instead
    dft_data_t data = { .freq_count = 3 };
    data.frequencies = SMRTA_ALLOC_ARRAY(arena, f64, 3);
    data.amplitudes  = SMRTA_ALLOC_ARRAY(arena, f64, 3);
    data.phases      = SMRTA_ALLOC_ARRAY(arena, f64, 3);

    f64 *real_o, *imag_o;
    reconstruct_spectrum(arena, &data, 8, &real_o, &imag_o);
}

Test(dft, istft_round_trip_recovers_samples) {
    smrt_arena_t *arena = smrt_arena_create(MiB(1), KiB(4), false);

    u64 sample_count = 32, sample_rate = 32, window_size = 8, hop_size = 4;
    f64 freqs[1] = { 3.0 };
    f64  amps[1] = { 1.0 };
    f64 *samples = mock_amplitude_data(arena, freqs, amps, 1, (f64)sample_count/sample_rate, sample_count);

    stft_data_t stft = short_time_fourier_transform(arena, window_size, hop_size, samples, sample_count, sample_rate);
    cr_assert_eq(stft.segment_count, 7);

    f64 *output = inverse_short_time_fourier_transform(arena, stft, NULL, 0);

    // Sample 0 is inherently unrecoverable: periodic hann_window(0, window_size) == 0,
    // so the analysis window zeroes out samples[0]'s contribution before it ever reaches
    // the FFT, and no other segment covers index 0 to make up for it. Every other sample
    // is covered by a segment with a nonzero window weight, which weighted overlap-add
    // recovers exactly.
    for (u64 i = 1; i < sample_count; i++) {
        cr_expect(F64_EQ(output[i], samples[i], 1e-9));
    }

    smrt_arena_destroy(arena);
}
