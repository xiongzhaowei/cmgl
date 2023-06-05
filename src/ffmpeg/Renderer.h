//
// Created by 熊朝伟 on 2023-05-16.
//

#pragma once

#include "SDL2/SDL.h"

OMP_FFMPEG_NAMESPACE_BEGIN

class VideoRenderer;

class AudioRenderer : public Consumer<Frame> {
public:
    static RefPtr<AudioRenderer> from(
        RefPtr<Stream<Frame>> stream,
        SDL_AudioFormat format,
        AVCodecParameters* codecpar,
        double timebase
    );
    ~AudioRenderer();

    void add(RefPtr<Frame> frame);
    void addError();
    void close();

    void fill(uint8_t* stream, int len);
    void attach(VideoRenderer* renderer);
    void play(bool state);

    double duration();
private:
    double _timebase;
    FrameList _frameList;
    RefPtr<StreamSubscription<Frame>> _subscription;
    RefPtr<AudioConverter> _converter;
    RefPtr<VideoRenderer> _video;
    SDL_AudioSpec _audioSpec = { 0 };
    SDL_AudioDeviceID _audioDeviceID = 0;
};

class VideoRenderer : public Consumer<Frame> {
public:
    static RefPtr<VideoRenderer> from(
        RefPtr<Stream<Frame>> stream,
        RefPtr<render::VideoSource> source,
        AVCodecParameters* codecpar,
        double timebase,
        std::function<void()> callback
    );
    ~VideoRenderer();

    void add(RefPtr<Frame> frame) override;
    void addError() override;
    void close() override;

    void sync(double pts);
    //void play(bool state);

    double duration();
private:
    double _timebase;
    FrameList _frameList;
    RefPtr<StreamSubscription<Frame>> _subscription;
    RefPtr<VideoConverter> _converter;
    RefPtr<render::VideoSource> _source;
    std::function<void()> _callback;
};

OMP_FFMPEG_NAMESPACE_END
