#include "dft.h"
#include "common.h"
#include "vec2.h"

#include <string.h>

dft_data_t discrete_fourier_transform(smrt_arena_t *arena, f64 *samples, u64 sample_count, u64 sample_rate) {
    u64 freq_count = (sample_count / 2) + 1;
        f64  freq_step = (f64)sample_rate / (f64)sample_count;

        dft_data_t d = {.freq_count = freq_count};
        d.frequencies = SMRTA_ALLOC_ARRAY(arena, f64, freq_count);
         d.amplitudes = SMRTA_ALLOC_ARRAY(arena, f64, freq_count);
             d.phases = SMRTA_ALLOC_ARRAY(arena, f64, freq_count);

        for (u64 freq_index = 0; freq_index < freq_count; freq_index++) {
            smrta_temp_t scratch = smrta_scratch_start(NULL, 0);

            vec2d_soa_t vs;
            vs.xs = SMRTA_ALLOC_ARRAY(scratch.arena, f64, sample_count);
            vs.ys = SMRTA_ALLOC_ARRAY(scratch.arena, f64, sample_count);
            vs.size = sample_count;

            if (!vs.xs || !vs.ys) { smrta_scratch_end(scratch); return (dft_data_t){ 0 }; }

            for (u64 i = 0; i < sample_count; i++) {
                f64 angle = i / (f64)sample_count * PI * 2.0 * freq_index;

                vs.xs[i] = cos(angle) * samples[i];
                vs.ys[i] = sin(angle) * samples[i];
            }

            vec2d_t average_pos = vec2d_soa_average(&vs);
            smrta_scratch_end(scratch);

            b8 is_zero_hz = freq_index == 0;
            b8 is_nyquist = freq_index == freq_count - 1 && sample_count % 2 == 0;
            f64 amp_coeff = is_zero_hz || is_nyquist ? 1.0 : 2.0;

            d.frequencies[freq_index] = freq_index * freq_step;
             d.amplitudes[freq_index] = vec2d_length(average_pos) * amp_coeff;
                 d.phases[freq_index] = -atan2(average_pos.y, average_pos.x);

        }
        return d;
}

static inline u64 bit_reverse(u64 n, u8 m) {
    u64 out = 0;
    for (u64 i = 0; i < m; i++) {
        out = (out << 1) | (n & 1);
        n >>= 1;
    }
    return out;
}

dft_data_t fast_fourier_transform(smrt_arena_t *arena, f64 *samples, u64 sample_count, u64 sample_rate, smrt_arena_t **conflicts, u64 num_conflicts) {
    assert(F64_EQ(round(log2(sample_count)), log2(sample_count), 1e-9) &&
           "FFT input sample_count must be a power of 2");

    smrta_temp_t scratch = smrta_scratch_start(conflicts, num_conflicts);

    vec2d_soa_t vs;
    vs.xs = SMRTA_ALLOC_ARRAY(scratch.arena, f64, sample_count);
    vs.ys = SMRTA_ALLOC_ARRAY(scratch.arena, f64, sample_count);
    vs.size = sample_count;

    f64 l2 = log2(sample_count);

    for (u64 i = 0; i < sample_count; i++) {
        u64 j = bit_reverse(i, l2);
        vs.xs[j] = samples[i];
    }

    for (u64 s = 1; s <= l2; s++) {
        u64 m = 1 << s;
        u64 n = m / 2;

        f64 angle_step = -2.0 * PI / (f64)m;
        vec2d_t w_step = {.x=cos(angle_step), .y=sin(angle_step)};

        for (u64 k = 0; k < sample_count; k+=m) {
            vec2d_t w = VEC2D_FROM(1.0, 0.0);

            for (u64 j = 0; j < n; j++) {
                vec2d_t t = vec2d_cmul(w, vec2d_soa_get(&vs, k + j + n));
                vec2d_t u = vec2d_soa_get(&vs, k + j);

                vec2d_soa_set(&vs, k + j    , vec2d_add(u, t));
                vec2d_soa_set(&vs, k + j + n, vec2d_sub(u, t));

                w = vec2d_cmul(w, w_step);
            }
        }
    }

    u64 freq_count = (sample_count / 2) + 1;
    f64  freq_step = (f64)sample_rate / (f64)sample_count;

    dft_data_t d = {.freq_count = freq_count};
    d.frequencies = SMRTA_ALLOC_ARRAY(arena, f64, freq_count);
     d.amplitudes = SMRTA_ALLOC_ARRAY(arena, f64, freq_count);
         d.phases = SMRTA_ALLOC_ARRAY(arena, f64, freq_count);

    for (u64 i = 0; i < freq_count; i++) {
        vec2d_t v = vec2d_soa_get(&vs, i);

        f64 amp = vec2d_length(v) / (f64)sample_count;
        if (i != 0 && i != freq_count - 1) amp *= 2;

        f64 phase = atan2(v.y, v.x);

        d.frequencies[i] = i * freq_step;
         d.amplitudes[i] = amp;
             d.phases[i] = phase;
    }

    smrta_scratch_end(scratch);
    return d;
}

wav_data_t dft_data_to_wav(smrt_arena_t *arena, dft_data_t dft, u64 sample_rate, f64 duration) {
    u64 sample_count = (u64)(sample_rate * duration);

    u8 *data = SMRTA_ALLOC_ARRAY(arena, u8, sample_count);
    if (!data) return (wav_data_t){ 0 };

    wav_data_t d = {
        .sample_count=sample_count,
        .samples = data,
    };

    for (u64 s_num = 0; s_num < sample_count; s_num++) {
        f64 amp = 0.0;
        f64 t = ((f64)s_num / sample_count) * duration;
        for (u64 freq_index = 0; freq_index < dft.freq_count; freq_index++) {
            f64 f = dft.frequencies[freq_index];
            f64 a = dft.amplitudes[freq_index];
            f64 p = dft.phases[freq_index];
            amp += cos((t * f * 2.0 * PI) + p) * a;
        }
        data[s_num] = (u8)((amp+1.0) * (UINT8_MAX/2));
    }

    return d;
}

stft_data_t short_time_fourier_transform(smrt_arena_t *arena, u64 window_size, u64 hop_size, f64 *samples, u64 sample_count, u64 sample_rate) {
    assert(F64_EQ(round(log2(window_size)), log2(window_size), 1e-9) &&
           "STFT input window_size must be a power of 2");

    sample_count = ALIGN_UP_POW2(sample_count, window_size);

    u64 segment_count = ((sample_count - window_size) / hop_size) + 1;

    stft_segment_t *segments = SMRTA_ALLOC_ARRAY(arena, stft_segment_t, segment_count);
    if (!segments) return (stft_data_t){ 0 };

    for (u64 i = 0; i < segment_count; i++) {
        u64 start = hop_size * i;

        f64 *seg_samples = samples + start;

        stft_segment_t seg = {
            .start_index = start,
            .sample_count = window_size,
        };

        seg.data = fast_fourier_transform(
            arena,
            seg_samples,
            window_size,
            sample_rate,
            NULL, 0
        );

        segments[i] = seg;
    }

    return (stft_data_t){
        .sample_rate=sample_rate,
        .segment_count=segment_count,
        .segments=segments,
    };
}

void inverse_fast_fourier_transform(smrt_arena_t *arena, f64 const *real, f64 const *imag, u64 sample_count, f64 **real_o, f64 **imag_o, smrt_arena_t **conflicts, u64 num_conflicts) {
    assert(F64_EQ(round(log2(sample_count)), log2(sample_count), 1e-9) &&
           "iFFT input sample_count must be a power of 2");

    smrta_temp_t scratch = smrta_scratch_start(conflicts, num_conflicts);

    vec2d_soa_t vs;
    vs.xs = real_o ? (*real_o = SMRTA_ALLOC_ARRAY(arena, f64, sample_count)) : SMRTA_ALLOC_ARRAY(scratch.arena, f64, sample_count);
    vs.ys = imag_o ? (*imag_o = SMRTA_ALLOC_ARRAY(arena, f64, sample_count)) : SMRTA_ALLOC_ARRAY(scratch.arena, f64, sample_count);
    vs.size = sample_count;

    memcpy(vs.xs, real, sample_count * sizeof(f64));
    memcpy(vs.ys, imag, sample_count * sizeof(f64));

    u64 l2 = (u64)log2(sample_count);

    for (u64 i = 0; i < sample_count; i++) {
        u64 j = bit_reverse(i, l2);
        if (j > i) {
            f64 tx = vs.xs[i]; vs.xs[i] = vs.xs[j]; vs.xs[j] = tx;
            f64 ty = vs.ys[i]; vs.ys[i] = vs.ys[j]; vs.ys[j] = ty;
        }
    }

    for (u64 s = 1; s <= l2; s++) {
        u64 m = 1 << s;
        u64 n = m / 2;

        f64 angle_step = 2.0 * PI / (f64)m;
        vec2d_t w_step = {.x=cos(angle_step), .y=sin(angle_step)};

        for (u64 k = 0; k < sample_count; k+=m) {
            vec2d_t w = VEC2D_FROM(1.0, 0.0);

            for (u64 j = 0; j < n; j++) {
                vec2d_t t = vec2d_cmul(w, vec2d_soa_get(&vs, k + j + n));
                vec2d_t u = vec2d_soa_get(&vs, k + j);

                vec2d_soa_set(&vs, k + j    , vec2d_add(u, t));
                vec2d_soa_set(&vs, k + j + n, vec2d_sub(u, t));

                w = vec2d_cmul(w, w_step);
            }
        }
    }

    smrta_scratch_end(scratch);
}

f64 *inverse_short_time_fourier_transform(smrt_arena_t *arena, stft_data_t stft,  u64 window_size, u64 hop_size, smrt_arena_t **conflicts, u64 num_conflicts) {
    smrta_temp_t scratch = smrta_scratch_start(conflicts, num_conflicts);

    u64 output_len = (stft.segment_count - 1) * hop_size + window_size;

    dft_data_t *frames = SMRTA_ALLOC_ARRAY(scratch.arena, dft_data_t, stft.segment_count);

    for (u64 i = 0; i < stft.segment_count; i++) {
        frames[i] = stft.segments[i].data;
    }

    f64 * output = SMRTA_ALLOC_ARRAY(arena, f64, output_len);
    f64 *weights = SMRTA_ALLOC_ARRAY(scratch.arena, f64, output_len);

    for (u64 k = 0;  k < stft.segment_count; k++) {
        smrt_arena_mark(scratch.arena);

        f64 *spec_real, *spec_imag;
        reconstruct_spectrum(scratch.arena, &frames[k], window_size, &spec_real, &spec_imag);

        f64 *real_o;
        inverse_fast_fourier_transform(scratch.arena, spec_real, spec_imag, window_size, &real_o, NULL, NULL, 0);

        u64 start = k * hop_size;

        for (u64 j = 0; j < window_size; j++) {
            output[start + j] += real_o[j];
            weights[start + j] += 1.0;
        }

        smrt_arena_pop_to_mark(scratch.arena);
    }

    for (u64 w = 0; w < output_len; w++) {
        if (weights[w] > 1e-9) {
            output[w] /= weights[w];
        }
    }

    smrta_scratch_end(scratch);
    return output;
}

void reconstruct_spectrum(smrt_arena_t *arena, dft_data_t const *data, u64 sample_count, f64 **real_o, f64 **imag_o) {
    f64 *real = *real_o = SMRTA_ALLOC_ARRAY(arena, f64, sample_count);
    f64 *imag = *imag_o = SMRTA_ALLOC_ARRAY(arena, f64, sample_count);

    for (u64 k = 0; k < data->freq_count; k++) {
        f64 amp = data->amplitudes[k];
        if (k != 0 && k != sample_count / 2) amp /= 2.0;

        f64 phase = data->phases[k];
        real[k] = amp * cos(phase);
        imag[k] = amp * sin(phase);
    }

    for (u64 j = (sample_count/2) + 1; j < sample_count; j++) {
        u64 mirror = sample_count - j;
        real[j] =  real[mirror];
        imag[j] = -imag[mirror];
    }
}

