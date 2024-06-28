#include <sys/select.h>
#include <signal.h>

#include "utils.h"
// always use stdin as 0
int fds[MAX_DEVICES];
fd_set rfds;

volatile sig_atomic_t stop = 0;

void handle_sigint(int sig) {
    // upon receiving an interruption, stop the playback and return to caller function
    stop = 1;
    return;
}

void set_fds(int* ref) {
    memcpy(fds, ref, sizeof(fds));
}

void reset_rfds() {
    FD_ZERO(&rfds);
    for (int i = 0; i < MAX_DEVICES; i++) {
        FD_SET(fds[i], &rfds);
    }
}

void loop_async(char *buff, int buff_size, snd_pcm_uframes_t frames, snd_pcm_t *pcm_handle) {
    signal_snd signal_s;


    signal(SIGINT, handle_sigint);
    while (1) {
        reset_rfds();
        select(fds[MAX_DEVICES - 1] + 1, &rfds, NULL, NULL, NULL);
        
        if (FD_ISSET(fds[0], &rfds)) {
            signal_s = parse_async_input();
            switch (signal_s) {
                case STOP:
                    raise(SIGINT);
                    printf("STOP\n");
                    break;
                case PAUSE:
                    raise(SIGINT);
                    printf("PAUSE\n");
                    break;
                case PLAY:
                    raise(SIGINT);
                    printf("PLAY\n");
                    break;
                case REWIND_5:
                    raise(SIGINT);
                    printf("REWIND_5\n");
                    break;
                case FORWARD_5:
                    raise(SIGINT);
                    printf("FORWARD_5\n");
                    break;
            }
        }
        {
            stop = 0;
            if (signal_s == STOP) {
                snd_pcm_pause(pcm_handle, 1);
                snd_pcm_drop(pcm_handle);
                break;
            }
            if (signal_s == PAUSE) {
                snd_pcm_pause(pcm_handle, 1);
                continue;
            }
            if (signal_s == REWIND_5) {
                snd_pcm_pause(pcm_handle, 1);
                snd_pcm_forward(pcm_handle, 5);
                lseek(fds[1], -5 * 2, SEEK_CUR);
                signal_s = PLAY;
            }
            if (signal_s == FORWARD_5) {
                snd_pcm_pause(pcm_handle, 1);
                snd_pcm_rewind(pcm_handle, 5);
                lseek(fds[1], 5 * 2, SEEK_CUR);
                signal_s = PLAY;
            }
            if (signal_s == PLAY) {
                snd_pcm_pause(pcm_handle, 0);
            }
        }
        play_frame(fds[1], buff, buff_size, pcm_handle, frames);

    }
}

