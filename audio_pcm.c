#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

#define AUDIO_FILE   "audio.pcm"
#define SAMPLE_RATE  44100
#define CHANNELS     2
#define BUFFER_SIZE  4096

int main(void)
{
    FILE *fp = NULL;
    SDL_AudioDeviceID dev = 0;
    unsigned char *buffer = NULL;
    int ret = 1;

    /* Open audio file */
    fp = fopen(AUDIO_FILE, "rb");
    if (!fp) {
        fprintf(stderr, "Could not open %s\n", AUDIO_FILE);
        return 1;
    }
    else {
        /* nothing */
    }

    /* Allocate buffer */
    buffer = malloc(BUFFER_SIZE);
    if (!buffer) {
        fprintf(stderr, "Could not allocate buffer\n");
        goto cleanup;
    }
    else {
        /* nothing */
    }

    /* Initialize SDL */
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "SDL init failed: %s\n", SDL_GetError());
        goto cleanup;
    }
    else {
        /* nothing */
    }

    /* Open audio device */
    SDL_AudioSpec spec;
    spec.freq = SAMPLE_RATE;
    spec.format = AUDIO_S16LSB;
    spec.channels = CHANNELS;
    spec.samples = 1024;
    spec.callback = NULL;

    dev = SDL_OpenAudioDevice(NULL, 0, &spec, NULL, 0);
    if (!dev) {
        fprintf(stderr, "Could not open audio: %s\n", SDL_GetError());
        goto cleanup;
    }
    else {
        /* nothing */
    }

    /* Start playback */
    SDL_PauseAudioDevice(dev, 0);

    /* Main loop */
    SDL_Event event;
    int quit = 0;
    size_t bytes_read;

    while (!quit) {
        /* Keep audio buffer filled */
        while (SDL_GetQueuedAudioSize(dev) < BUFFER_SIZE * 4) {
            bytes_read = fread(buffer, 1, BUFFER_SIZE, fp);
            if (bytes_read == 0) {
                /* Wait for remaining audio to play */
                while (SDL_GetQueuedAudioSize(dev) > 0) {
                    SDL_Delay(10);
                }
                quit = 1;
                break;
            }
            else {
                SDL_QueueAudio(dev, buffer, bytes_read);
            }
        }

        SDL_Delay(10);

        /* Handle events */
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = 1;
            }
            else if (event.type == SDL_KEYDOWN &&
                     event.key.keysym.sym == SDLK_ESCAPE) {
                quit = 1;
            }
            else {
                /* ignore other events */
            }
        }
    }

    ret = 0;

cleanup:
    if (buffer) {
        free(buffer);
    }
    else {
        /* nothing */
    }

    if (dev) {
        SDL_CloseAudioDevice(dev);
    }
    else {
        /* nothing */
    }

    SDL_Quit();

    if (fp) {
        fclose(fp);
    }
    else {
        /* nothing */
    }

    return ret;
}
