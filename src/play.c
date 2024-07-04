#include <alsa/asoundlib.h>
#include <alsa/pcm.h>
#include <stdio.h>

#include "utils.h"

#define PCM_DEVICE "default"

#define PCM_DEVICE_2 "default"


int main(int argc, char *argv[])
{
    unsigned int pcm, tmp, dir;
    int rate, channels, format;
    snd_pcm_t *pcm_handle;
    snd_pcm_hw_params_t *params;
    snd_pcm_uframes_t frames;
    char *buff = NULL;
    int buff_size;
    int readfd, readval = 0;

    src_sound_cards();
    if (argc < 2) {
        printf("Usage: %s <wav_file> <opt:card number> <opt:device number>\n", argv[0]);
        return -1;
    }
    int opt_card = 0, opt_device = 0;
    const char device[7];

    if (argc >= 3)
        opt_card = atoi(argv[2]);
    if (argc == 4)
        opt_device = atoi(argv[3]);
    snprintf(device, 7, "hw:%d,%d", opt_card, opt_device);

    if (opt_card != 0 || opt_device != 0) {
        /* Open the PCM device in playback mode */
        if (pcm = snd_pcm_open(&pcm_handle, device,
                    SND_PCM_STREAM_PLAYBACK, 0) < 0)
            printf("ERROR: Can't open \"%s\" PCM device. %s\n",
                    device, snd_strerror(pcm));
    } else
        if (pcm = snd_pcm_open(&pcm_handle, PCM_DEVICE,
                    SND_PCM_STREAM_PLAYBACK, 0) < 0)
            printf("ERROR: Can't open \"%s\" PCM device. %s\n",
                    PCM_DEVICE, snd_strerror(pcm));    

    /* Allocate parameters object and fill it with default values*/
    snd_pcm_hw_params_alloca(&params);
    snd_pcm_hw_params_any(pcm_handle, params);

    // // set nonblocking writei
    // snd_pcm_nonblock(pcm_handle, 1);

    /* Set parameters */
    FILE *file = fopen(argv[1], "r");
    wav_header *header = read_wav_header(file);
    set_params_from_wav_header(params, header, pcm_handle);
    fclose(file);
    channels = header->num_channels;
    int frame_size = header->bits_per_sample / 8;
    frames = header->data_length / frame_size / channels;

    /* Write parameters */
    if (pcm = snd_pcm_hw_params(pcm_handle, params) < 0)
    printf("ERROR: Can't set harware parameters. %s\n", snd_strerror(pcm));

    /* Resume information */
    printf("PCM name: '%s'\n", snd_pcm_name(pcm_handle));

    printf("PCM state: %s\n",  snd_pcm_state_name(snd_pcm_state(pcm_handle)));

    snd_pcm_hw_params_get_channels(params, &tmp);
    printf("channels: %i ", tmp);

    if (tmp == 1)
        printf("(mono)\n");
    else if (tmp == 2)
        printf("(stereo)\n");

    snd_pcm_hw_params_get_rate(params, &tmp, 0);
    printf("rate: %d bps\n", tmp);

    /* Allocate buffer to hold single period */
    snd_pcm_hw_params_get_period_size(params, &frames, 0);
    printf("frames: %d\n", (int)frames);

    buff_size = frames * channels * header->bits_per_sample / 8; /* 2 -> sample size */;
    buff = (char *) malloc(buff_size);
    memset(buff, 0, buff_size);

    snd_pcm_hw_params_get_period_time(params, &tmp, NULL);

    readfd = open(argv[1], O_RDONLY);
    if (readfd < 0) {
          perror("wavread");
          exit(1);
    }

    printf("starting playback now: buffer size is %d\n", buff_size);

    lseek(readfd, 128, SEEK_SET);
    // while((readval = read(readfd, buff, buff_size)) > 0) {
    //     if ((pcm = snd_pcm_writei(pcm_handle, buff, frames)) == -EPIPE) {
    //             // fprintf(stderr, "Underrun!\n");
    //             snd_pcm_prepare(pcm_handle);
    //     } else if (pcm < 0) {
    //             fprintf(stderr, "Error writing to PCM device: %s\n", snd_strerror(pcm));
    //     }
    // }

    int fds[MAX_DEVICES];
    fds[0] = 0; // stdin
    fds[1] = readfd; // wav file
    set_fds(fds);
    loop_async(buff, buff_size, frames, pcm_handle);

    snd_pcm_drain(pcm_handle);

    printf("ended playback.\n");
    snd_pcm_close(pcm_handle);
    free(buff);
    return 0;
}
