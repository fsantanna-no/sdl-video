#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

#define VIDEO_FILE "video.yuv"
#define WIDTH      640
#define HEIGHT     480
#define FPS        30

int main(void)
{
    FILE *fp = NULL;
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    SDL_Texture *texture = NULL;
    unsigned char *y_plane = NULL;
    unsigned char *u_plane = NULL;
    unsigned char *v_plane = NULL;
    int ret = 1;

    /* Calculate plane sizes */
    int y_size = WIDTH * HEIGHT;
    int uv_size = (WIDTH / 2) * (HEIGHT / 2);

    /* Open video file */
    fp = fopen(VIDEO_FILE, "rb");
    if (!fp) {
        fprintf(stderr, "Could not open %s\n", VIDEO_FILE);
        return 1;
    }
    else {
        /* nothing */
    }

    /* Allocate plane buffers */
    y_plane = malloc(y_size);
    u_plane = malloc(uv_size);
    v_plane = malloc(uv_size);
    if (!y_plane || !u_plane || !v_plane) {
        fprintf(stderr, "Could not allocate buffers\n");
        goto cleanup;
    }
    else {
        /* nothing */
    }

    /* Initialize SDL */
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL init failed: %s\n", SDL_GetError());
        goto cleanup;
    }
    else {
        /* nothing */
    }

    window = SDL_CreateWindow("Video Player",
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

    /* Main loop */
    SDL_Event event;
    int quit = 0;
    int frame_delay_ms = 1000 / FPS;

    while (!quit) {
        /* Read one frame (Y, then U, then V) */
        if (fread(y_plane, 1, y_size, fp) != (size_t)y_size ||
            fread(u_plane, 1, uv_size, fp) != (size_t)uv_size ||
            fread(v_plane, 1, uv_size, fp) != (size_t)uv_size) {
            break;
        }
        else {
            /* nothing */
        }

        /* Update texture with YUV data */
        SDL_UpdateYUVTexture(texture, NULL,
            y_plane, WIDTH,
            u_plane, WIDTH / 2,
            v_plane, WIDTH / 2);

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
        SDL_Delay(frame_delay_ms);

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

    SDL_Quit();

    if (fp) {
        fclose(fp);
    }
    else {
        /* nothing */
    }

    return ret;
}
