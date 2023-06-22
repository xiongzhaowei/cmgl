//
// Created by 熊朝伟 on 2023-05-16.
//

#pragma once

#include "SDL2/SDL.h"

OMP_FFMPEG_NAMESPACE_BEGIN

class Renderer : public Consumer<Frame> {
public:
    Renderer(DecoderStream* stream);

    void add(RefPtr<Frame> frame) override;
    void addError() override;
    void close() override;

    double duration();
    virtual void sync(double pts) {}
protected:
    double _timebase;
    FrameList _frameList;
    RefPtr<Converter<Frame>> _converter;
    RefPtr<StreamSubscription> _subscription;
};

class VideoRenderer;

class AudioRenderer : public Renderer {
public:
    static RefPtr<AudioRenderer> from(
        DecoderStream* stream,
        SDL_AudioFormat format
    );
    AudioRenderer(DecoderStream* stream);
    ~AudioRenderer();

    void fill(uint8_t* stream, int len);
    void attach(Renderer* renderer);
    void play(bool state);
private:
    RefPtr<Renderer> _video;
    SDL_AudioSpec _audioSpec = { 0 };
    SDL_AudioDeviceID _audioDeviceID = 0;
};

class VideoRenderer : public Renderer {
public:
    static RefPtr<VideoRenderer> from(
        DecoderStream* stream,
        RefPtr<render::VideoSource> source,
        std::function<void()> callback
    );
    VideoRenderer(DecoderStream* stream);
    ~VideoRenderer();

    void sync(double pts);
    //void play(bool state);
private:
    RefPtr<render::VideoSource> _source;
    std::function<void()> _callback;
};

OMP_FFMPEG_NAMESPACE_END
