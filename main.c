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

typedef struct {
    FILE *fp;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    unsigned char *y_plane;
    unsigned char *u_plane;
    unsigned char *v_plane;
    int y_size;
    int uv_size;
    int frame_num;
    int done;
} VideoResource;

typedef struct {
    FILE *fp;
    SDL_AudioDeviceID dev;
    unsigned char *buffer;
    int buffer_size;
    int done;
} AudioResource;

/* Video functions */

VideoResource *video_open(SDL_Renderer *renderer)
{
    VideoResource *res = calloc(1, sizeof(VideoResource));
    if (!res) {
        return NULL;
    }
    else {
        /* nothing */
    }

    res->renderer = renderer;
    res->y_size = WIDTH * HEIGHT;
    res->uv_size = (WIDTH / 2) * (HEIGHT / 2);

    res->fp = fopen(VIDEO_FILE, "rb");
    if (!res->fp) {
        fprintf(stderr, "Could not open %s\n", VIDEO_FILE);
        free(res);
        return NULL;
    }
    else {
        /* nothing */
    }

    res->y_plane = malloc(res->y_size);
    res->u_plane = malloc(res->uv_size);
    res->v_plane = malloc(res->uv_size);
    if (!res->y_plane || !res->u_plane || !res->v_plane) {
        fprintf(stderr, "Could not allocate video buffers\n");
        fclose(res->fp);
        free(res->y_plane);
        free(res->u_plane);
        free(res->v_plane);
        free(res);
        return NULL;
    }
    else {
        /* nothing */
    }

    res->texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_YV12,
        SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);
    if (!res->texture) {
        fprintf(stderr, "Could not create texture: %s\n", SDL_GetError());
        fclose(res->fp);
        free(res->y_plane);
        free(res->u_plane);
        free(res->v_plane);
        free(res);
        return NULL;
    }
    else {
        /* nothing */
    }

    return res;
}

void video_close(VideoResource *res)
{
    if (!res) {
        return;
    }
    else {
        /* nothing */
    }

    if (res->texture) {
        SDL_DestroyTexture(res->texture);
    }
    else {
        /* nothing */
    }

    if (res->fp) {
        fclose(res->fp);
    }
    else {
        /* nothing */
    }

    free(res->y_plane);
    free(res->u_plane);
    free(res->v_plane);
    free(res);
}

void video_sync(VideoResource *res, double dt)
{
    if (!res || res->done) {
        return;
    }
    else {
        /* nothing */
    }

    /* Calculate expected frame based on elapsed time */
    int expected_frame = (int)(dt * FPS);

    /* Read frames to catch up */
    while (res->frame_num <= expected_frame && !res->done) {
        if (fread(res->y_plane, 1, res->y_size, res->fp) !=
                (size_t)res->y_size ||
            fread(res->u_plane, 1, res->uv_size, res->fp) !=
                (size_t)res->uv_size ||
            fread(res->v_plane, 1, res->uv_size, res->fp) !=
                (size_t)res->uv_size) {
            res->done = 1;
            break;
        }
        else {
            res->frame_num++;
        }
    }
}

void video_present(VideoResource *res)
{
    if (!res || res->done) {
        return;
    }
    else {
        /* nothing */
    }

    SDL_UpdateYUVTexture(res->texture, NULL,
        res->y_plane, WIDTH,
        res->u_plane, WIDTH / 2,
        res->v_plane, WIDTH / 2);

    SDL_RenderClear(res->renderer);
    SDL_RenderCopy(res->renderer, res->texture, NULL, NULL);
    SDL_RenderPresent(res->renderer);
}

/* Audio functions */

AudioResource *audio_open(void)
{
    AudioResource *res = calloc(1, sizeof(AudioResource));
    if (!res) {
        return NULL;
    }
    else {
        /* nothing */
    }

    res->buffer_size = (SAMPLE_RATE * CHANNELS * 2) / FPS;

    res->fp = fopen(AUDIO_FILE, "rb");
    if (!res->fp) {
        fprintf(stderr, "Could not open %s\n", AUDIO_FILE);
        free(res);
        return NULL;
    }
    else {
        /* nothing */
    }

    res->buffer = malloc(res->buffer_size);
    if (!res->buffer) {
        fprintf(stderr, "Could not allocate audio buffer\n");
        fclose(res->fp);
        free(res);
        return NULL;
    }
    else {
        /* nothing */
    }

    SDL_AudioSpec spec;
    spec.freq = SAMPLE_RATE;
    spec.format = AUDIO_S16LSB;
    spec.channels = CHANNELS;
    spec.samples = 1024;
    spec.callback = NULL;

    res->dev = SDL_OpenAudioDevice(NULL, 0, &spec, NULL, 0);
    if (!res->dev) {
        fprintf(stderr, "Could not open audio: %s\n", SDL_GetError());
        fclose(res->fp);
        free(res->buffer);
        free(res);
        return NULL;
    }
    else {
        /* nothing */
    }

    SDL_PauseAudioDevice(res->dev, 0);

    return res;
}

void audio_close(AudioResource *res)
{
    if (!res) {
        return;
    }
    else {
        /* nothing */
    }

    /* Wait for audio to finish */
    if (res->dev) {
        while (SDL_GetQueuedAudioSize(res->dev) > 0) {
            SDL_Delay(10);
        }
        SDL_CloseAudioDevice(res->dev);
    }
    else {
        /* nothing */
    }

    if (res->fp) {
        fclose(res->fp);
    }
    else {
        /* nothing */
    }

    free(res->buffer);
    free(res);
}

void audio_sync(AudioResource *res, double dt)
{
    if (!res || res->done) {
        return;
    }
    else {
        /* nothing */
    }

    int queued = SDL_GetQueuedAudioSize(res->dev);
    (void)dt;

    /* Keep buffer filled ahead of playback */
    while (!res->done && queued < res->buffer_size * 4) {
        size_t bytes_read = fread(res->buffer, 1, res->buffer_size, res->fp);
        if (bytes_read == 0) {
            res->done = 1;
            break;
        }
        else {
            SDL_QueueAudio(res->dev, res->buffer, bytes_read);
            queued += bytes_read;
        }
    }
}

void audio_present(AudioResource *res)
{
    /* Audio is presented automatically by SDL audio device */
    (void)res;
}

/* Main */

int main(void)
{
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    VideoResource *video = NULL;
    AudioResource *audio = NULL;
    int ret = 1;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "SDL init failed: %s\n", SDL_GetError());
        return 1;
    }
    else {
        /* nothing */
    }

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

    video = video_open(renderer);
    if (!video) {
        goto cleanup;
    }
    else {
        /* nothing */
    }

    audio = audio_open();
    if (!audio) {
        goto cleanup;
    }
    else {
        /* nothing */
    }

    /* Main loop */
    SDL_Event event;
    int quit = 0;
    Uint32 start_time = SDL_GetTicks();

    while (!quit && (!video->done || !audio->done)) {
        double dt = (SDL_GetTicks() - start_time) / 1000.0;

        video_sync(video, dt);
        audio_sync(audio, dt);

        video_present(video);
        audio_present(audio);

        SDL_Delay(1);

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = 1;
            }
            else if (event.type == SDL_KEYDOWN &&
                     event.key.keysym.sym == SDLK_ESCAPE) {
                quit = 1;
            }
            else {
                /* ignore */
            }
        }
    }

    ret = 0;

cleanup:
    video_close(video);
    audio_close(audio);

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

    SDL_Quit();

    return ret;
}
