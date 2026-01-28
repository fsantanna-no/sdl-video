#include <stdio.h>
#include <SDL2/SDL.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

#define VIDEO_FILE "video.mp4"
#define WIDTH  640
#define HEIGHT 480

int main(void)
{
    AVFormatContext *fmt_ctx = NULL;
    AVCodecContext *codec_ctx = NULL;
    const AVCodec *codec = NULL;
    AVFrame *frame = NULL;
    AVPacket *packet = NULL;
    struct SwsContext *sws_ctx = NULL;
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    SDL_Texture *texture = NULL;
    int video_stream = -1;
    int ret = 1;

    /* Open video file */
    if (avformat_open_input(&fmt_ctx, VIDEO_FILE, NULL, NULL) < 0) {
        fprintf(stderr, "Could not open %s\n", VIDEO_FILE);
        return 1;
    }
    else {
        /* nothing */
    }

    if (avformat_find_stream_info(fmt_ctx, NULL) < 0) {
        fprintf(stderr, "Could not find stream info\n");
        goto cleanup;
    }
    else {
        /* nothing */
    }

    /* Find video stream */
    for (int i = 0; i < (int)fmt_ctx->nb_streams; i++) {
        if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream = i;
            break;
        }
        else {
            /* continue searching */
        }
    }

    if (video_stream < 0) {
        fprintf(stderr, "No video stream found\n");
        goto cleanup;
    }
    else {
        /* nothing */
    }

    /* Set up decoder */
    codec = avcodec_find_decoder(fmt_ctx->streams[video_stream]->codecpar->codec_id);
    if (!codec) {
        fprintf(stderr, "Unsupported codec\n");
        goto cleanup;
    }
    else {
        /* nothing */
    }

    codec_ctx = avcodec_alloc_context3(codec);
    if (!codec_ctx) {
        fprintf(stderr, "Could not allocate codec context\n");
        goto cleanup;
    }
    else {
        /* nothing */
    }

    if (avcodec_parameters_to_context(codec_ctx,
            fmt_ctx->streams[video_stream]->codecpar) < 0) {
        fprintf(stderr, "Could not copy codec params\n");
        goto cleanup;
    }
    else {
        /* nothing */
    }

    if (avcodec_open2(codec_ctx, codec, NULL) < 0) {
        fprintf(stderr, "Could not open codec\n");
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
        SDL_TEXTUREACCESS_STREAMING, codec_ctx->width, codec_ctx->height);
    if (!texture) {
        fprintf(stderr, "Could not create texture: %s\n", SDL_GetError());
        goto cleanup;
    }
    else {
        /* nothing */
    }

    /* Allocate frame and packet */
    frame = av_frame_alloc();
    packet = av_packet_alloc();
    if (!frame || !packet) {
        fprintf(stderr, "Could not allocate frame/packet\n");
        goto cleanup;
    }
    else {
        /* nothing */
    }

    /* Calculate frame delay from framerate */
    AVRational fr = fmt_ctx->streams[video_stream]->avg_frame_rate;
    int frame_delay_ms = (fr.num > 0) ? (1000 * fr.den / fr.num) : 33;

    /* Main loop */
    SDL_Event event;
    int quit = 0;

    while (!quit && av_read_frame(fmt_ctx, packet) >= 0) {
        if (packet->stream_index == video_stream) {
            if (avcodec_send_packet(codec_ctx, packet) >= 0) {
                while (avcodec_receive_frame(codec_ctx, frame) >= 0) {
                    /* Update texture with YUV data */
                    SDL_UpdateYUVTexture(texture, NULL,
                        frame->data[0], frame->linesize[0],
                        frame->data[1], frame->linesize[1],
                        frame->data[2], frame->linesize[2]);

                    SDL_RenderClear(renderer);
                    SDL_RenderCopy(renderer, texture, NULL, NULL);
                    SDL_RenderPresent(renderer);
                    SDL_Delay(frame_delay_ms);
                }
            }
            else {
                /* decode error, skip frame */
            }
        }
        else {
            /* not video packet */
        }

        av_packet_unref(packet);

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
    if (packet) {
        av_packet_free(&packet);
    }
    else {
        /* nothing */
    }

    if (frame) {
        av_frame_free(&frame);
    }
    else {
        /* nothing */
    }

    if (sws_ctx) {
        sws_freeContext(sws_ctx);
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

    if (codec_ctx) {
        avcodec_free_context(&codec_ctx);
    }
    else {
        /* nothing */
    }

    if (fmt_ctx) {
        avformat_close_input(&fmt_ctx);
    }
    else {
        /* nothing */
    }

    return ret;
}
