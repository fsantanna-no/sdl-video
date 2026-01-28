#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

#define VIDEO_FILE   "video.yuv"
#define AUDIO_FILE   "audio.pcm"
#define WIDTH        640
#define HEIGHT       480
#define FPS          30
#define SAMPLE_RATE  44100
#define CHANNELS     2
#define BUFFER_SIZE  4096

int main(void)
{
    FILE *video_fp = NULL;
    FILE *audio_fp = NULL;
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    SDL_Texture *texture = NULL;
    SDL_AudioDeviceID audio_dev = 0;
    unsigned char *y_plane = NULL;
    unsigned char *u_plane = NULL;
    unsigned char *v_plane = NULL;
    unsigned char *audio_buffer = NULL;
    int ret = 1;

    /* Calculate sizes */
    int y_size = WIDTH * HEIGHT;
    int uv_size = (WIDTH / 2) * (HEIGHT / 2);
    int bytes_per_frame = (SAMPLE_RATE * CHANNELS * 2) / FPS;

    /* Open files */
    video_fp = fopen(VIDEO_FILE, "rb");
    if (!video_fp) {
        fprintf(stderr, "Could not open %s\n", VIDEO_FILE);
        return 1;
    }
    else {
        /* nothing */
    }

    audio_fp = fopen(AUDIO_FILE, "rb");
    if (!audio_fp) {
        fprintf(stderr, "Could not open %s\n", AUDIO_FILE);
        goto cleanup;
    }
    else {
        /* nothing */
    }

    /* Allocate buffers */
    y_plane = malloc(y_size);
    u_plane = malloc(uv_size);
    v_plane = malloc(uv_size);
    audio_buffer = malloc(bytes_per_frame);
    if (!y_plane || !u_plane || !v_plane || !audio_buffer) {
        fprintf(stderr, "Could not allocate buffers\n");
        goto cleanup;
    }
    else {
        /* nothing */
    }

    /* Initialize SDL */
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "SDL init failed: %s\n", SDL_GetError());
        goto cleanup;
    }
    else {
        /* nothing */
    }

    /* Create window */
    window = SDL_CreateWindow("A/V Player",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIDTH, HEIGHT, 0);
    if (!window) {
        fprintf(stderr, "Could not create window: %s\n", SDL_GetError());
        goto cleanup;
    }
    else {
        /* nothing */
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        fprintf(stderr, "Could not create renderer: %s\n", SDL_GetError());
        goto cleanup;
    }
    else {
        /* nothing */
    }

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_YV12,
        SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);
    if (!texture) {
        fprintf(stderr, "Could not create texture: %s\n", SDL_GetError());
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

    audio_dev = SDL_OpenAudioDevice(NULL, 0, &spec, NULL, 0);
    if (!audio_dev) {
        fprintf(stderr, "Could not open audio: %s\n", SDL_GetError());
        goto cleanup;
    }
    else {
        /* nothing */
    }

    /* Start audio playback */
    SDL_PauseAudioDevice(audio_dev, 0);

    /* Main loop */
    SDL_Event event;
    int quit = 0;
    Uint32 start_time = SDL_GetTicks();
    int frame_num = 0;

    while (!quit) {
        /* Calculate expected frame based on elapsed time */
        Uint32 elapsed = SDL_GetTicks() - start_time;
        int expected_frame = (elapsed * FPS) / 1000;

        /* Display frames to catch up */
        while (frame_num <= expected_frame) {
            /* Read video frame */
            if (fread(y_plane, 1, y_size, video_fp) != (size_t)y_size ||
                fread(u_plane, 1, uv_size, video_fp) != (size_t)uv_size ||
                fread(v_plane, 1, uv_size, video_fp) != (size_t)uv_size) {
                quit = 1;
                break;
            }
            else {
                /* nothing */
            }

            /* Read corresponding audio */
            size_t audio_read = fread(audio_buffer, 1, bytes_per_frame,
                audio_fp);
            if (audio_read > 0) {
                SDL_QueueAudio(audio_dev, audio_buffer, audio_read);
            }
            else {
                /* nothing */
            }

            frame_num++;
        }

        /* Update display with latest frame */
        SDL_UpdateYUVTexture(texture, NULL,
            y_plane, WIDTH,
            u_plane, WIDTH / 2,
            v_plane, WIDTH / 2);

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        /* Small delay to avoid busy loop */
        SDL_Delay(1);

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

    /* Wait for audio to finish */
    while (SDL_GetQueuedAudioSize(audio_dev) > 0) {
        SDL_Delay(10);
    }

    ret = 0;

cleanup:
    if (y_plane) {
        free(y_plane);
    }
    else {
        /* nothing */
    }

    if (u_plane) {
        free(u_plane);
    }
    else {
        /* nothing */
    }

    if (v_plane) {
        free(v_plane);
    }
    else {
        /* nothing */
    }

    if (audio_buffer) {
        free(audio_buffer);
    }
    else {
        /* nothing */
    }

    if (texture) {
        SDL_DestroyTexture(texture);
    }
    else {
        /* nothing */
    }

    if (renderer) {
        SDL_DestroyRenderer(renderer);
    }
    else {
        /* nothing */
    }

    if (window) {
        SDL_DestroyWindow(window);
    }
    else {
        /* nothing */
    }

    if (audio_dev) {
        SDL_CloseAudioDevice(audio_dev);
    }
    else {
        /* nothing */
    }

    SDL_Quit();

    if (video_fp) {
        fclose(video_fp);
    }
    else {
        /* nothing */
    }

    if (audio_fp) {
        fclose(audio_fp);
    }
    else {
        /* nothing */
    }

    return ret;
}
