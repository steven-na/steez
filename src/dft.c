#include "dft.h"
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

stft_data_t short_time_fourier_transform(smrt_arena_t *arena, u64 samples_per_segment, f64 *samples, u64 sample_count, u64 sample_rate) {
    u64 segment_count = (u64)ceil((f64)sample_count / samples_per_segment);

    stft_segment_t *segments = SMRTA_ALLOC_ARRAY(arena, stft_segment_t, segment_count);
    if (!segments) return (stft_data_t){ 0 };

    for (u64 seg_num = 0; seg_num < segment_count; seg_num++) {

        u64 samples_in_this_seg = samples_per_segment;
        if (seg_num == segment_count - 1 && !(sample_count % samples_per_segment == 0))
            samples_in_this_seg = sample_count % samples_per_segment;

        stft_segment_t seg = {
            .start_index = samples_per_segment * seg_num,
            .sample_count = samples_in_this_seg,
        };

        seg.data = discrete_fourier_transform(
            arena,
            samples + seg.start_index,
            samples_in_this_seg,
            sample_rate
        );

        segments[seg_num] = seg;
    }

    stft_data_t data = {
        .segment_count = segment_count,
        .sample_rate = sample_rate,
        .segments = segments
    };

    return data;
}

wav_data_t stft_data_to_wav(smrt_arena_t *arena, stft_data_t stft, u64 sample_count) {
    u8 *data = SMRTA_ALLOC_ARRAY(arena, u8, sample_count);
    if (!data) return (wav_data_t){ 0 };

    wav_data_t d = {
        .sample_count=sample_count,
        .samples = data,
    };

    u64 offset = 0;
    for (u64 seg_num = 0; seg_num < stft.segment_count; seg_num++) {
        smrta_temp_t scratch = smrta_scratch_start(NULL, 0);

        stft_segment_t seg = stft.segments[seg_num];

        wav_data_t wav_data = dft_data_to_wav(
            scratch.arena,
            seg.data,
            stft.sample_rate,
            (f64)seg.sample_count / stft.sample_rate
        );

        memcpy(data + offset, wav_data.samples, wav_data.sample_count);

        offset += wav_data.sample_count * sizeof(u8);

        smrta_scratch_end(scratch);
    }

    return d;
}
