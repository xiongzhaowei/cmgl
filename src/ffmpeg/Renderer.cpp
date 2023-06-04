//
// Created by 熊朝伟 on 2023-05-16.
//

#include "defines.h"
#include "Converter.h"
#include "Renderer.h"

OMP_FFMPEG_USING_NAMESPACE

static void SDLCALL __AudioRendererCallback(void* userdata, Uint8* stream, int len) {
    reinterpret_cast<AudioRenderer1*>(userdata)->fill(stream, len);
}

RefPtr<AudioRenderer> AudioRenderer::from(RefPtr<Movie::Stream> stream, SDL_AudioFormat format, int sample_rate, int channels, int frame_size, double timebase) {
    SDL_Init(SDL_INIT_AUDIO);

    RefPtr<AudioRenderer> renderer = new AudioRenderer;

    SDL_AudioSpec spec = { 0 };
    spec.freq = sample_rate;
    spec.format = format;
    spec.channels = channels;
    spec.silence = 0;
    spec.samples = frame_size;
    spec.callback = __AudioRendererCallback;
    spec.userdata = renderer.value();

    renderer->_timebase = timebase;
    renderer->_stream = stream;
    renderer->_audioDeviceID = SDL_OpenAudioDevice(NULL, 0, &spec, &renderer->_audioSpec, 0);
    return (renderer->_audioDeviceID != 0) ? renderer : nullptr;
}

RefPtr<AudioRenderer> AudioRenderer::from(RefPtr<Movie::Stream> stream, SDL_AudioFormat format) {
    if (!stream) return nullptr;

    AVCodecParameters* codecpar = stream->stream()->codecpar;
    if (!codecpar) return nullptr;

    return from(
        stream,
        format,
        codecpar->sample_rate,
        codecpar->channels,
        codecpar->frame_size,
        av_q2d(stream->stream()->time_base)
    );
}

AudioRenderer::~AudioRenderer() {
    SDL_CloseAudioDevice(_audioDeviceID);
}

void AudioRenderer::fill(uint8_t* stream, int len) {
    RefPtr<Frame> frame = _stream->pop();

    if (frame != nullptr) {
        double pts = frame->timestamp() * _timebase;

        SDL_memset(stream, 0, len);
        SDL_MixAudioFormat(stream, frame->frame()->extended_data[0], _audioSpec.format, std::min<uint32_t>(frame->frame()->linesize[0], len), SDL_MIX_MAXVOLUME);
        frame = nullptr;

        if (_video) _video->sync(pts);
    }
}

void AudioRenderer::attach(Consumer* consumer) {
    _video = consumer;
}

void AudioRenderer::sync(double pts) {
    // 只考虑音频同步视频，反过来未实现。
}

void AudioRenderer::play(bool state) {
    SDL_PauseAudioDevice(_audioDeviceID, state ? 0 : 1);
}

VideoRenderer::~VideoRenderer() {
}

void VideoRenderer::attach(Consumer* consumer) {
    // 只考虑音频同步视频，反过来未实现；
}

void VideoRenderer::sync(double pts) {
    int64_t timestamp = pts / _timebase;
    while (RefPtr<Frame> frame = _stream->pop(timestamp)) {
        _source->update(frame->frame());
        frame = nullptr;
        _callback();
    }
}

void VideoRenderer::play(bool state) {
    // TODO: 视频暂时只能被动等待音频的通知
}

RefPtr<VideoRenderer> VideoRenderer::from(RefPtr<Movie::Stream> stream, RefPtr<render::VideoSource> source, std::function<void()> callback) {
    if (!stream) return nullptr;

    AVCodecParameters* codecpar = stream->stream()->codecpar;
    if (!codecpar) return nullptr;

    RefPtr<VideoRenderer> renderer = new VideoRenderer;
    renderer->_timebase = av_q2d(stream->stream()->time_base);
    renderer->_stream = stream;
    renderer->_source = source;
    renderer->_callback = callback;
    return renderer;
}

RefPtr<AudioRenderer1> AudioRenderer1::from(
    RefPtr<Stream<Frame>> stream,
    SDL_AudioFormat format,
    AVCodecParameters* codecpar,
    double timebase
) {
    SDL_Init(SDL_INIT_AUDIO);

    RefPtr<AudioRenderer1> renderer = new AudioRenderer1;

    SDL_AudioSpec spec = { 0 };
    spec.freq = codecpar->sample_rate;
    spec.format = format;
    spec.channels = codecpar->channels;
    spec.silence = 0;
    spec.samples = codecpar->frame_size;
    spec.callback = __AudioRendererCallback;
    spec.userdata = renderer.value();

    renderer->_timebase = timebase;
    renderer->_subscription = stream->listen(renderer);
    renderer->_audioDeviceID = SDL_OpenAudioDevice(NULL, 0, &spec, &renderer->_audioSpec, 0);
    return (renderer->_audioDeviceID != 0) ? renderer : nullptr;
}

AudioRenderer1::~AudioRenderer1() {
    if (_audioDeviceID != 0) {
        SDL_CloseAudioDevice(_audioDeviceID);
        _audioDeviceID = 0;
    }
}

void AudioRenderer1::add(RefPtr<Frame> frame) {
    RefPtr<Frame> audio;
    if (_converter) {
        audio = _converter->convert(frame);
    } else {
        audio = Frame::alloc();
        audio->swap(frame);
    }
    _frameList.push(frame);
    if (_frameList.size() >= 5) {
        _subscription->pause();
    }
}

void AudioRenderer1::addError() {

}

void AudioRenderer1::close() {
    if (_subscription != nullptr) {
        _subscription->cancel();
        _subscription = nullptr;
    }
}

void AudioRenderer1::fill(uint8_t* stream, int len) {
    RefPtr<Frame> frame = _frameList.pop();

    if (frame != nullptr) {
        double pts = frame->timestamp() * _timebase;

        SDL_memset(stream, 0, len);
        SDL_MixAudioFormat(stream, frame->frame()->extended_data[0], _audioSpec.format, std::min<uint32_t>(frame->frame()->linesize[0], len), SDL_MIX_MAXVOLUME);
        frame = nullptr;

        if (_video) _video->sync(pts);
    }
    if (_frameList.size() < 5) {
        _subscription->resume();
    }
}

void AudioRenderer1::attach(VideoRenderer1* renderer) {
    _video = renderer;
}

void AudioRenderer1::play(bool state) {
    SDL_PauseAudioDevice(_audioDeviceID, state ? 0 : 1);
}

RefPtr<VideoRenderer1> VideoRenderer1::from(
    RefPtr<Stream<Frame>> stream,
    RefPtr<render::VideoSource> source,
    AVCodecParameters* codecpar,
    double timebase,
    std::function<void()> callback
) {
    if (!stream) return nullptr;
    if (!codecpar) return nullptr;

    RefPtr<VideoRenderer1> renderer = new VideoRenderer1;
    renderer->_timebase = timebase;
    renderer->_subscription = stream->listen(renderer);
    renderer->_source = source;
    renderer->_callback = callback;
    return renderer;
}

VideoRenderer1::~VideoRenderer1() {
}

void VideoRenderer1::add(RefPtr<Frame> frame) {
    RefPtr<Frame> audio;
    if (_converter) {
        audio = _converter->convert(frame);
    } else {
        audio = Frame::alloc();
        audio->swap(frame);
    }
    _frameList.push(frame);
    if (_frameList.size() >= 5) {
        _subscription->pause();
    }
}

void VideoRenderer1::addError() {
}

void VideoRenderer1::close() {
    if (_subscription != nullptr) {
        _subscription->cancel();
        _subscription = nullptr;
    }
}

void VideoRenderer1::sync(double pts) {
    int64_t timestamp = pts / _timebase;
    while (RefPtr<Frame> frame = _frameList.pop(timestamp)) {
        _source->update(frame->frame());
        frame = nullptr;
        _callback();
    }
}
