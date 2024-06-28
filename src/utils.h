#ifndef UTILS_H
#define UTILS_H

#define MAX_DEVICES         2

#include <stdio.h>
#include <alsa/asoundlib.h>

typedef struct wav_header {
    char riff_tag[4];
    int riff_length;
    char wave_tag[4];
    char fmt_tag[4];
    int fmt_length;
    short audio_format;
    short num_channels;
    int sample_rate;
    int byte_rate;
    short block_align;
    short bits_per_sample;
    char data_tag[4];
    int data_length;
} wav_header;

typedef enum {
    STOP,
    PAUSE,
    PLAY,
    REWIND_5,
    FORWARD_5
} signal_snd;

wav_header* read_wav_header(FILE *file);
void set_params_from_wav_header(snd_pcm_hw_params_t *params, wav_header *header, snd_pcm_t *pcm_handle);
signal_snd parse_async_input();
void set_fds(int* ref);
void reset_rfds();
void loop_async(char *buff, int buff_size, snd_pcm_uframes_t frames, snd_pcm_t *pcm_handle);
int play_frame(int fd, char *buff, int buff_size, snd_pcm_t *pcm_handle, snd_pcm_uframes_t frames);

#endif