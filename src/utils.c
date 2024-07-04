#include <alsa/asoundlib.h>

#include "utils.h"

wav_header* read_wav_header(FILE *file) {
    wav_header *header = (wav_header*) malloc(sizeof(wav_header));
    fread(header, sizeof(wav_header), 1, file);
    return header;
}

void src_sound_cards(void) {
    int card_num = -1;
    int err;
    snd_ctl_t *handle = NULL;

    while (1) {
        err = snd_card_next(&card_num);
        if (err < 0) {
            printf("Error: %s\n", snd_strerror(err));
            break;
        }
        if (card_num < 0)
            break;
        printf("card num %d///", card_num);

        char name[32];
        sprintf(name, "hw:%d", card_num);
        if ((err = snd_ctl_open(&handle, name, 0)) < 0) {
            printf("Error: %s\n", snd_strerror(err));
            continue;
        }

        snd_ctl_card_info_t *card_info;
        snd_ctl_card_info_alloca(&card_info);
        if ((err = snd_ctl_card_info(handle, card_info)) < 0) {
            printf("Error: %s\n", snd_strerror(err));
            continue;
        }
        printf("Card %d: %s\n", card_num, snd_ctl_card_info_get_name(card_info));
        snd_ctl_close(handle);
    }
}

void set_params_from_wav_header(snd_pcm_hw_params_t *params, wav_header *header, snd_pcm_t *pcm_handle) {
    unsigned int rate = header->sample_rate;
    snd_pcm_hw_params_set_access(pcm_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(pcm_handle, params, (header->audio_format == 1) ? SND_PCM_FORMAT_S16_LE : SND_PCM_FORMAT_FLOAT);
    snd_pcm_hw_params_set_channels(pcm_handle, params, header->num_channels);
    snd_pcm_hw_params_set_rate_near(pcm_handle, params, &rate, 0);
}

signal_snd parse_async_input() {
    char line[256];
    char c;
    fgets(line, sizeof(line), stdin);
    c = line[0];
    switch (c) {
        case 'p':
            return PAUSE;
        case 's':
            return STOP;
        case 'r':
            return REWIND_5;
        case 'f':
            return FORWARD_5;
        case 'x':
            return PLAY;
            break;
        default:
            return PAUSE;
    }
}

int play_frame(int fd, char *buff, int buff_size, snd_pcm_t *pcm_handle, snd_pcm_uframes_t frames) {
    int pcm;
    int retval = read(fd, buff, buff_size);
    if (retval == 0)
        return 0;

    if ((pcm = snd_pcm_writei(pcm_handle, buff, frames)) == -EPIPE) {
            // fprintf(stderr, "Underrun!\n");
            snd_pcm_prepare(pcm_handle);
    } else if (pcm < 0) {
            fprintf(stderr, "Error writing to PCM device: %s\n", snd_strerror(pcm));
    }
    return 1;
}
