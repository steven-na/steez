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
               u64   sample_rate;
} stft_data_t;

/// Run STFT algorithm on samples.
stft_data_t short_time_fourier_transform(smrt_arena_t *             arena ,
                                                  u64 samples_per_segment ,
                                                  f64 *           samples ,
                                                  u64        sample_count ,
                                                  u64         sample_rate);

/// Convert STFT frequency data to unsigned 8-bit PCM WAV amplitude data
wav_data_t stft_data_to_wav(smrt_arena_t *      arena ,
                             stft_data_t         stft ,
                                     u64 sample_count);
