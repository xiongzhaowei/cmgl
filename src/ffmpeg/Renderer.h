//
// Created by 熊朝伟 on 2023-05-16.
//

#pragma once

#include "SDL2/SDL.h"

OMP_FFMPEG_NAMESPACE_BEGIN

class VideoRenderer : public Object {
public:
    static RefPtr<VideoRenderer> from(RefPtr<MovieBufferedConsumer> buffer, AVRational time_base, RefPtr<render::VideoSource> source, std::function<void()> callback);
    VideoRenderer(RefPtr<MovieBufferedConsumer> buffer, double time_base);
    ~VideoRenderer();

    void sync(double pts);
private:
    double _time_base;
    RefPtr<MovieBufferedConsumer> _buffer;
    RefPtr<render::VideoSource> _source;
    std::function<void()> _callback;
};

class AudioRenderer : public Object {
public:
    static RefPtr<AudioRenderer> from(
        RefPtr<MovieBufferedConsumer> buffer,
        AVRational time_base,
        RefPtr<Thread> thread,
        AVSampleFormat format,
        AVChannelLayout ch_layout,
        int32_t sample_rate,
        int32_t frame_size
    );
    AudioRenderer(RefPtr<Thread> thread, RefPtr<MovieBufferedConsumer> buffer, RefPtr<AudioConverter> converter, double time_base);
    ~AudioRenderer();

    void fill(uint8_t* stream, int len);
    void attach(VideoRenderer* renderer);
    void play(bool state);
private:
    double _time_base;
    RefPtr<Thread> _thread;
    RefPtr<MovieBufferedConsumer> _buffer;
    RefPtr<AudioConverter> _converter;
    RefPtr<VideoRenderer> _attached;
    uint32_t _device;
};

OMP_FFMPEG_NAMESPACE_END
