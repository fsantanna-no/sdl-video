# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

```bash
make          # Build all executables
make clean    # Remove all .exe files
make main.exe # Build specific target
```

## Dependencies

- SDL2: `libsdl2-dev`
- FFmpeg (for *_mp4 targets): `libavcodec-dev libavformat-dev libswscale-dev libswresample-dev libavutil-dev`

## Architecture

Sample programs for SDL2 audio/video playback:

| File | Format | Libraries | Description |
|------|--------|-----------|-------------|
| main.c | raw | SDL2 | Organized A/V player with resource API |
| video_yuv.c | raw YUV420 | SDL2 | Video only |
| video_mp4.c | MP4 | SDL2 + FFmpeg | Video only |
| audio_pcm.c | raw PCM | SDL2 | Audio only |
| audio_mp4.c | MP4 | SDL2 + FFmpeg | Audio only |
| both_raw.c | raw YUV+PCM | SDL2 | Synced A/V |
| both_mp4.c | MP4 | SDL2 + FFmpeg | Synced A/V |

**Raw formats** (SDL2 only):
- Video: YUV420 planar (`video.yuv`) - WIDTH×HEIGHT Y, then U/V planes
- Audio: 16-bit signed LE stereo PCM (`audio.pcm`) at 44100Hz

**main.c API pattern**:
- `video_open(renderer)` / `audio_open()` - returns resource struct
- `video_close(res)` / `audio_close(res)` - cleanup
- `video_sync(res, dt)` / `audio_sync(res, dt)` - timing/buffering
- `video_present(res)` / `audio_present(res)` - render/play

## Creating Test Media

```bash
ffmpeg -i input.mp4 -s 640x480 -f rawvideo -pix_fmt yuv420p video.yuv
ffmpeg -i input.mp4 -f s16le -acodec pcm_s16le -ac 2 -ar 44100 audio.pcm
```
