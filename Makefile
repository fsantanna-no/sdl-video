CC = gcc
CFLAGS = -Wall -O2 $(shell pkg-config --cflags sdl2)
LDFLAGS = $(shell pkg-config --libs sdl2)
FFMPEG = $(shell pkg-config --cflags --libs libavcodec libavformat \
         libswscale libswresample libavutil)

all: main.exe video_yuv.exe video_mp4.exe audio_pcm.exe audio_mp4.exe \
     both_raw.exe both_mp4.exe

main.exe: main.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

video_yuv.exe: video_yuv.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

video_mp4.exe: video_mp4.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS) $(FFMPEG)

audio_pcm.exe: audio_pcm.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

audio_mp4.exe: audio_mp4.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS) $(FFMPEG)

both_raw.exe: both_raw.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

both_mp4.exe: both_mp4.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS) $(FFMPEG)

clean:
	rm -f *.exe
