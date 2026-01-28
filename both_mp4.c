#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>

#define VIDEO_FILE   "video.mp4"
#define SAMPLE_RATE  44100
#define CHANNELS     2

int main(void)
{
    AVFormatContext *fmt_ctx = NULL;
    AVCodecContext *vcodec_ctx = NULL;
    AVCodecContext *acodec_ctx = NULL;
    const AVCodec *vcodec = NULL;
    const AVCodec *acodec = NULL;
    AVFrame *frame = NULL;
    AVPacket *packet = NULL;
    SwrContext *swr_ctx = NULL;
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    SDL_Texture *texture = NULL;
    SDL_AudioDeviceID audio_dev = 0;
    uint8_t *audio_buffer = NULL;
    int audio_buffer_size = 0;
    int video_stream = -1;
    int audio_stream = -1;
    int ret = 1;

    /* Open file */
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

    /* Find streams */
    for (int i = 0; i < (int)fmt_ctx->nb_streams; i++) {
        if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO &&
            video_stream < 0) {
            video_stream = i;
        }
        else if (fmt_ctx->streams[i]->codecpar->codec_type ==
                 AVMEDIA_TYPE_AUDIO && audio_stream < 0) {
            audio_stream = i;
        }
        else {
            /* continue */
        }
    }

    if (video_stream < 0 || audio_stream < 0) {
        fprintf(stderr, "Could not find audio/video streams\n");
        goto cleanup;
    }
    else {
        /* nothing */
    }

    /* Set up video decoder */
    vcodec = avcodec_find_decoder(
        fmt_ctx->streams[video_stream]->codecpar->codec_id);
    if (!vcodec) {
        fprintf(stderr, "Unsupported video codec\n");
        goto cleanup;
    }
    else {
        /* nothing */
    }

    vcodec_ctx = avcodec_alloc_context3(vcodec);
    if (!vcodec_ctx) {
        fprintf(stderr, "Could not allocate video codec context\n");
        goto cleanup;
    }
    else {
        /* nothing */
    }

    if (avcodec_parameters_to_context(vcodec_ctx,
            fmt_ctx->streams[video_stream]->codecpar) < 0) {
        fprintf(stderr, "Could not copy video codec params\n");
        goto cleanup;
    }
    else {
        /* nothing */
    }

    if (avcodec_open2(vcodec_ctx, vcodec, NULL) < 0) {
        fprintf(stderr, "Could not open video codec\n");
        goto cleanup;
    }
    else {
        /* nothing */
    }

    /* Set up audio decoder */
    acodec = avcodec_find_decoder(
        fmt_ctx->streams[audio_stream]->codecpar->codec_id);
    if (!acodec) {
        fprintf(stderr, "Unsupported audio codec\n");
        goto cleanup;
    }
    else {
        /* nothing */
    }

    acodec_ctx = avcodec_alloc_context3(acodec);
    if (!acodec_ctx) {
        fprintf(stderr, "Could not allocate audio codec context\n");
        goto cleanup;
    }
    else {
        /* nothing */
    }

    if (avcodec_parameters_to_context(acodec_ctx,
            fmt_ctx->streams[audio_stream]->codecpar) < 0) {
        fprintf(stderr, "Could not copy audio codec params\n");
        goto cleanup;
    }
    else {
        /* nothing */
    }

    if (avcodec_open2(acodec_ctx, acodec, NULL) < 0) {
        fprintf(stderr, "Could not open audio codec\n");
        goto cleanup;
    }
    else {
        /* nothing */
    }

    /* Set up audio resampler */
    swr_ctx = swr_alloc_set_opts(NULL,
        AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16, SAMPLE_RATE,
        acodec_ctx->channel_layout, acodec_ctx->sample_fmt,
        acodec_ctx->sample_rate, 0, NULL);
    if (!swr_ctx || swr_init(swr_ctx) < 0) {
        fprintf(stderr, "Could not init resampler\n");
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
        vcodec_ctx->width, vcodec_ctx->height, 0);
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
        SDL_TEXTUREACCESS_STREAMING, vcodec_ctx->width, vcodec_ctx->height);
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

    /* Start audio */
    SDL_PauseAudioDevice(audio_dev, 0);

    /* Get time bases */
    double video_tb = av_q2d(fmt_ctx->streams[video_stream]->time_base);
    double audio_tb = av_q2d(fmt_ctx->streams[audio_stream]->time_base);

    /* Main loop */
    SDL_Event event;
    int quit = 0;
    Uint32 start_time = SDL_GetTicks();
    (void)audio_tb;

    while (!quit && av_read_frame(fmt_ctx, packet) >= 0) {
        if (packet->stream_index == video_stream) {
            if (avcodec_send_packet(vcodec_ctx, packet) >= 0) {
                while (avcodec_receive_frame(vcodec_ctx, frame) >= 0) {
                    /* Calculate frame PTS in seconds */
                    double pts = frame->pts * video_tb;

                    /* Wait for audio to catch up */
                    double audio_pos = (double)(SDL_GetTicks() - start_time) /
                        1000.0;
                    double delay = pts - audio_pos;
                    if (delay > 0.001) {
                        SDL_Delay((Uint32)(delay * 1000));
                    }
                    else {
                        /* no delay needed */
                    }

                    /* Display frame */
                    SDL_UpdateYUVTexture(texture, NULL,
                        frame->data[0], frame->linesize[0],
                        frame->data[1], frame->linesize[1],
                        frame->data[2], frame->linesize[2]);

                    SDL_RenderClear(renderer);
                    SDL_RenderCopy(renderer, texture, NULL, NULL);
                    SDL_RenderPresent(renderer);
                }
            }
            else {
                /* decode error */
            }
        }
        else if (packet->stream_index == audio_stream) {
            if (avcodec_send_packet(acodec_ctx, packet) >= 0) {
                while (avcodec_receive_frame(acodec_ctx, frame) >= 0) {
                    /* Calculate output size */
                    int out_samples = swr_get_out_samples(swr_ctx,
                        frame->nb_samples);
                    int needed_size = out_samples * CHANNELS * 2;

                    if (needed_size > audio_buffer_size) {
                        free(audio_buffer);
                        audio_buffer = malloc(needed_size);
                        audio_buffer_size = needed_size;
                    }
                    else {
                        /* buffer is big enough */
                    }

                    /* Resample */
                    uint8_t *out_planes[] = { audio_buffer };
                    int converted = swr_convert(swr_ctx, out_planes,
                        out_samples,
                        (const uint8_t **)frame->data, frame->nb_samples);

                    if (converted > 0) {
                        SDL_QueueAudio(audio_dev, audio_buffer,
                            converted * CHANNELS * 2);
                    }
                    else {
                        /* nothing */
                    }

                    /* Limit queue size to avoid memory buildup */
                    while (SDL_GetQueuedAudioSize(audio_dev) > SAMPLE_RATE * 4) {
                        SDL_Delay(10);
                    }
                }
            }
            else {
                /* decode error */
            }
        }
        else {
            /* other stream */
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
                /* ignore */
            }
        }
    }

    /* Wait for audio to finish */
    while (SDL_GetQueuedAudioSize(audio_dev) > 0) {
        SDL_Delay(10);
    }

    ret = 0;

cleanup:
    if (audio_buffer) {
        free(audio_buffer);
    }
    else {
        /* nothing */
    }

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

    if (swr_ctx) {
        swr_free(&swr_ctx);
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

    if (vcodec_ctx) {
        avcodec_free_context(&vcodec_ctx);
    }
    else {
        /* nothing */
    }

    if (acodec_ctx) {
        avcodec_free_context(&acodec_ctx);
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
