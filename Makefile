CC = gcc
CFLAGS = -Wall -O2 $(shell pkg-config --cflags sdl2)
LDFLAGS = $(shell pkg-config --libs sdl2)
FFMPEG = $(shell pkg-config --cflags --libs libavcodec libavformat \
         libswscale libswresample libavutil)

all: video_yuv video_mp4 audio_pcm audio_mp4

video_yuv: video_yuv.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

video_mp4: video_mp4.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS) $(FFMPEG)

audio_pcm: audio_pcm.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

audio_mp4: audio_mp4.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS) $(FFMPEG)

clean:
	rm -f video_yuv video_mp4 audio_pcm audio_mp4
