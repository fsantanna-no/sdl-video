#include <stdio.h>
#include <SDL2/SDL.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>

#define AUDIO_FILE   "audio.mp4"
#define SAMPLE_RATE  44100
#define CHANNELS     2

int main(void)
{
    AVFormatContext *fmt_ctx = NULL;
    AVCodecContext *codec_ctx = NULL;
    const AVCodec *codec = NULL;
    AVFrame *frame = NULL;
    AVPacket *packet = NULL;
    SwrContext *swr_ctx = NULL;
    SDL_AudioDeviceID dev = 0;
    uint8_t *out_buffer = NULL;
    int out_buffer_size = 0;
    int audio_stream = -1;
    int ret = 1;

    /* Open audio file */
    if (avformat_open_input(&fmt_ctx, AUDIO_FILE, NULL, NULL) < 0) {
        fprintf(stderr, "Could not open %s\n", AUDIO_FILE);
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

    /* Find audio stream */
    for (int i = 0; i < (int)fmt_ctx->nb_streams; i++) {
        if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audio_stream = i;
            break;
        }
        else {
            /* continue searching */
        }
    }

    if (audio_stream < 0) {
        fprintf(stderr, "No audio stream found\n");
        goto cleanup;
    }
    else {
        /* nothing */
    }

    /* Set up decoder */
    codec = avcodec_find_decoder(
        fmt_ctx->streams[audio_stream]->codecpar->codec_id);
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
            fmt_ctx->streams[audio_stream]->codecpar) < 0) {
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

    /* Set up resampler */
    swr_ctx = swr_alloc_set_opts(NULL,
        AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16, SAMPLE_RATE,
        codec_ctx->channel_layout, codec_ctx->sample_fmt,
        codec_ctx->sample_rate, 0, NULL);
    if (!swr_ctx || swr_init(swr_ctx) < 0) {
        fprintf(stderr, "Could not init resampler\n");
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

    /* Start playback */
    SDL_PauseAudioDevice(dev, 0);

    /* Main loop */
    SDL_Event event;
    int quit = 0;

    while (!quit && av_read_frame(fmt_ctx, packet) >= 0) {
        if (packet->stream_index == audio_stream) {
            if (avcodec_send_packet(codec_ctx, packet) >= 0) {
                while (avcodec_receive_frame(codec_ctx, frame) >= 0) {
                    /* Calculate output size */
                    int out_samples = swr_get_out_samples(swr_ctx,
                        frame->nb_samples);
                    int needed_size = out_samples * CHANNELS * 2;

                    if (needed_size > out_buffer_size) {
                        free(out_buffer);
                        out_buffer = malloc(needed_size);
                        out_buffer_size = needed_size;
                    }
                    else {
                        /* buffer is big enough */
                    }

                    /* Resample */
                    uint8_t *out_planes[] = { out_buffer };
                    int converted = swr_convert(swr_ctx, out_planes,
                        out_samples,
                        (const uint8_t **)frame->data, frame->nb_samples);

                    if (converted > 0) {
                        SDL_QueueAudio(dev, out_buffer,
                            converted * CHANNELS * 2);
                    }
                    else {
                        /* nothing */
                    }

                    /* Limit queue size */
                    while (SDL_GetQueuedAudioSize(dev) > SAMPLE_RATE * 4) {
                        SDL_Delay(10);
                    }
                }
            }
            else {
                /* decode error, skip */
            }
        }
        else {
            /* not audio packet */
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

    /* Wait for audio to finish */
    if (!quit) {
        while (SDL_GetQueuedAudioSize(dev) > 0) {
            SDL_Delay(10);
        }
    }
    else {
        /* nothing */
    }

    ret = 0;

cleanup:
    if (out_buffer) {
        free(out_buffer);
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

    if (dev) {
        SDL_CloseAudioDevice(dev);
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
