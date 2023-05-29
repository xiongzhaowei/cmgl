//
// Created by 熊朝伟 on 2023-05-16.
//

#pragma once

#include "SDL2/SDL.h"

OMP_FFMPEG_NAMESPACE_BEGIN

class VideoRenderer : public Movie::Consumer {
public:
    ~VideoRenderer();

    void attach(Consumer* consumer) override;
    void sync(double pts) override;
    void play(bool state) override;
    void push(AVFrame* frame) override;
    void clear() override;
    double duration() const override;

    static RefPtr<VideoRenderer> from(AVStream* stream, RefPtr<render::VideoSource> source, std::function<void()> callback);
private:
    double _timebase;
    RefPtr<VideoConverter> _converter;
    std::list<AVFrame*> _frameList;
    RefPtr<render::VideoSource> _source;
    std::function<void()> _callback;
};

class AudioRenderer : public Movie::Consumer {
public:
    ~AudioRenderer();

    void fill(uint8_t* stream, int len);
    void attach(Consumer* consumer) override;
    void sync(double pts) override;
    void play(bool state) override;
    void push(AVFrame* frame) override;
    void clear() override;
    double duration() const override;

    static RefPtr<AudioRenderer> from(
        SDL_AudioFormat format,
        int sample_rate,
        int channels,
        int frame_size,
        double timebase,
        RefPtr<Mutex> mutex,
        RefPtr<AudioConverter> converter
    );
    static RefPtr<AudioRenderer> from(AVStream* stream, RefPtr<Mutex> mutex);
private:
    double _timebase;
    RefPtr<AudioConverter> _converter;
    std::list<AVFrame*> _frameList;
    RefPtr<Mutex> _mutex;
    RefPtr<Consumer> _video;
    SDL_AudioSpec _audioSpec = { 0 };
    SDL_AudioDeviceID _audioDeviceID = 0;
};

OMP_FFMPEG_NAMESPACE_END
