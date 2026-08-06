#pragma once

#include "common.h"
#include "smrt_arena.h"
#include "wav.h"

typedef struct {
    f64 *frequencies;
    f64 * amplitudes;
    f64 *     phases;
    u64   freq_count;
} dft_data_t;

/// Run DFT algorithm on samples
dft_data_t discrete_fourier_transform(smrt_arena_t *      arena ,
                                               f64 *    samples ,
                                               u64 sample_count ,
                                               u64  sample_rate);

/// Run FFT algorithm on samples. sample_count must be a power of 2.
dft_data_t fast_fourier_transform(smrt_arena_t *         arena ,
                                           f64 *       samples ,
                                             u64  sample_count ,
                                             u64   sample_rate ,
                  smrt_arena_t **conflicts , u64 num_conflicts);

/// Convert DFT frequency data to unsigned 8-bit PCM WAV amplitude data
wav_data_t dft_data_to_wav(smrt_arena_t *     arena ,
                             dft_data_t         dft ,
                                    u64 sample_rate ,
                                    f64    duration);

typedef struct {
    dft_data_t         data;
           u64 sample_count;
           u64  start_index;
} stft_segment_t;

typedef struct {
    stft_segment_t *    segments;
               u64 segment_count;
               u64   window_size;
               u64      hop_size;
               u64   sample_rate;
               u64 total_samples;
} stft_data_t;

/// Byte alignment required for a samples buffer to safely back
/// short_time_fourier_transform with the given window_size. Pass this as
/// align_up_memoryn to wav_load/load_wav_file when loading samples for STFT use.
#define STFT_SAMPLE_ALIGN_BYTES(window_size) ((u64)(window_size) * sizeof(f64))

/// Run STFT algorithm on samples, sliding a window_size window by hop_size each step
/// Warning: This function assumes that samples' allocation is at least
/// STFT_SAMPLE_ALIGN_BYTES(window_size)-byte aligned (e.g. by passing
/// STFT_SAMPLE_ALIGN_BYTES(window_size) as align_up_memoryn to wav_load/load_wav_file).
stft_data_t short_time_fourier_transform(smrt_arena_t *        arena ,
                                                    u64  window_size ,
                                                    u64     hop_size ,
                                                    f64 *    samples ,
                                                    u64 sample_count ,
                                                    u64  sample_rate);

/// Run inverse FFT on a complex spectrum (real/imag, length sample_count).
/// No 1/N normalization is applied here: real/imag must already be a correctly-scaled
/// full N-point spectrum (e.g. as produced by reconstruct_spectrum), not a raw
/// forward-FFT output, or the result will be off by a factor of N.
/// Pass NULL for either out-param to skip allocating/returning that component
void inverse_fast_fourier_transform(smrt_arena_t *      arena ,
                                    f64 const *          real ,
                                    f64 const *          imag ,
                                    u64          sample_count ,
                                    f64 **             real_o ,
                                    f64 **             imag_o ,
                  smrt_arena_t **conflicts, u64 num_conflicts);

/// Reconstruct samples from STFT data via overlap-add inverse FFT synthesis.
f64 *inverse_short_time_fourier_transform( smrt_arena_t *         arena ,
                                              stft_data_t          stft ,
                            smrt_arena_t **conflicts, u64 num_conflicts);

/// Reconstruct a full N-point complex spectrum from one-sided DFT amplitude/phase data
void reconstruct_spectrum(smrt_arena_t *    arena ,
                          dft_data_t const * data ,
                          u64        sample_count ,
                          f64 **           real_o ,
                          f64 **           imag_o);

