//
// Created by 熊朝伟 on 2023-05-16.
//

#pragma once

#include "SDL2/SDL.h"

OMP_FFMPEG_NAMESPACE_BEGIN

class Renderer : public StreamConsumer<Frame> {
public:
    Renderer(RefPtr<Thread> thread, RefPtr<Stream<Frame>> stream, AVCodecParameters* codecpar, AVRational time_base);

    void add(RefPtr<Frame> frame) override;
    void addError() override;
    void close() override;
    bool available() const override;

    double duration();
    virtual void sync(double pts) {}
protected:
    volatile bool _isPaused = false;
    double _timebase;
    FrameList _frameList;
    RefPtr<Thread> _thread;
    RefPtr<Converter<Frame>> _converter;
    RefPtr<StreamSubscription> _subscription;
};

class VideoRenderer;

class AudioRenderer : public Renderer {
public:
    static RefPtr<AudioRenderer> from(
        RefPtr<Thread> thread,
        RefPtr<Stream<Frame>> stream,
        AVCodecParameters* codecpar,
        AVRational time_base
    );
    AudioRenderer(RefPtr<Thread> thread, RefPtr<Stream<Frame>> stream, AVCodecParameters* codecpar, AVRational time_base);
    ~AudioRenderer();

    void fill(uint8_t* stream, int len);
    void attach(Renderer* renderer);
    void play(bool state);
private:
    RefPtr<Stream<Frame>> _filter;
    RefPtr<Renderer> _video;
    SDL_AudioSpec _audioSpec = { 0 };
    uint32_t _audioDeviceID = 0;
};

class VideoRenderer : public Renderer {
public:
    static RefPtr<VideoRenderer> from(
        RefPtr<Thread> thread,
        RefPtr<Stream<Frame>> stream,
        AVCodecParameters* codecpar,
        AVRational time_base,
        RefPtr<render::VideoSource> source,
        AVPixelFormat format,
        std::function<void()> callback
    );
    VideoRenderer(RefPtr<Thread> thread, RefPtr<Stream<Frame>> stream, AVCodecParameters* codecpar, AVRational time_base);
    ~VideoRenderer();

    void sync(double pts);
    //void play(bool state);
private:
    RefPtr<Stream<Frame>> _filter;
    RefPtr<render::VideoSource> _source;
    std::function<void()> _callback;
};

OMP_FFMPEG_NAMESPACE_END
