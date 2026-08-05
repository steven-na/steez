#pragma once

#include "common.h"
#include "smrt_arena.h"

#include <stdio.h>

typedef struct {
    u64    sample_count;
     u8 *       samples;
} wav_data_t;

// [Master RIFF chunk]
//     FileTypeBlocID  (4 bytes) : Identifier « RIFF »  (0x52, 0x49, 0x46, 0x46)
//     FileSize        (4 bytes) : Overall file size minus 8 bytes
//     FileFormatID    (4 bytes) : Format = « WAVE »  (0x57, 0x41, 0x56, 0x45)
#pragma pack(push, 1)
typedef struct {
     u8        chunk_id[4];
    u32          file_size;
     u8  file_format_id[4];
} wav_master_chunk_t;

// [Chunk describing the data format]
//     FormatBlocID    (4 bytes) : Identifier « fmt␣ »  (0x66, 0x6D, 0x74, 0x20)
//     BlocSize        (4 bytes) : Chunk size minus 8 bytes
//     AudioFormat     (2 bytes) : Audio format (1: PCM integer, 3: IEEE 754 float)
//     NbrChannels     (2 bytes) : Number of channels
//     Frequency       (4 bytes) : Sample rate (in hertz)
//     BytePerSec      (4 bytes) : Number of bytes to read per second (Frequency * BytePerBloc).
//     BytePerBloc     (2 bytes) : Number of bytes per block (NbrChannels * BitsPerSample / 8).
//     BitsPerSample   (2 bytes) : Number of bits per sample
typedef struct {
     u8     chunk_id[4];
    u32      chunk_size;
    u16    audio_format;
    u16    num_channels;
    u32     sample_rate;
    u32   bytes_per_sec;
    u16 bytes_per_block;
    u16 bits_per_sample;
} wav_fmt_chunk_t;
#pragma pack(pop)

/// Load WAV data from wav_file. Data will be allocated on arena, with allocation
/// size aligned up to align_up_memoryn.
wav_data_t load_wav_file(smrt_arena_t *   arena ,
                                 FILE *wav_file ,
                   wav_master_chunk_t *master_o ,
                      wav_fmt_chunk_t *format_o ,
                                   u64 align_up_memoryn);

/// Write WAV headers and data to wav_file.
b32 write_wav_file(FILE *   wav_file ,
        wav_fmt_chunk_t *fmt_chunk_i ,
             wav_data_t       data_i);

/// Generate WAV "fmt " chunk for unsigned 8-bit PCM data ready to be written to a file.
wav_fmt_chunk_t make_wav_fmt_chunk(u32    num_channels ,
                                   u32     sample_rate ,
                                   u16 bits_per_sample);

/// Load WAV data into [-1.0, 1.0] f64 amplitudes. Populates -o inputs with information.
/// samples will be amplitude data for each channel from the WAV file, of length sample_count.
void              wav_load(smrt_arena_t *arena, FILE *wav, f64 ***    samples_o ,
                                                           u16 *channel_count_o ,
                                                           u64 * sample_count_o ,
                                                           u32 *  sample_rate_o ,
                                                           u64 align_up_memoryn ,
                                 smrt_arena_t **conflicts, u64    num_conflicts );

f64 *       read_8bps_data(smrt_arena_t *arena, wav_data_t data, u16 num_channels, u16 channel, u64 align_up_memoryn);
f64 *      read_16bps_data(smrt_arena_t *arena, wav_data_t data, u16 num_channels, u16 channel, u64 align_up_memoryn);
f64 *      read_24bps_data(smrt_arena_t *arena, wav_data_t data, u16 num_channels, u16 channel, u64 align_up_memoryn);
f64 *read_32bps_float_data(smrt_arena_t *arena, wav_data_t data, u16 num_channels, u16 channel, u64 align_up_memoryn);
