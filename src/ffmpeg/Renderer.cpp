﻿//
// Created by 熊朝伟 on 2023-05-16.
//

#include "defines.h"
#include "Converter.h"
#include "Renderer.h"

OMP_FFMPEG_USING_NAMESPACE

static void SDLCALL __AudioRendererCallback(void* userdata, Uint8* stream, int len) {
    reinterpret_cast<AudioRenderer*>(userdata)->fill(stream, len);
}

RefPtr<AudioRenderer> AudioRenderer::from(SDL_AudioFormat format, int sample_rate, int channels, int frame_size, double timebase, RefPtr<Mutex> mutex, RefPtr<AudioConverter> converter) {
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
    renderer->_converter = converter;
    renderer->_mutex = mutex;
    renderer->_audioDeviceID = SDL_OpenAudioDevice(NULL, 0, &spec, &renderer->_audioSpec, 0);
    return (renderer->_audioDeviceID != 0) ? renderer : nullptr;
}

RefPtr<AudioRenderer> AudioRenderer::from(AVStream* stream, RefPtr<Mutex> mutex) {
    if (!stream) return nullptr;

    AVCodecParameters* codecpar = stream->codecpar;
    if (!codecpar) return nullptr;

    AVSampleFormat format1;
    SDL_AudioFormat format2;

    switch (stream->codecpar->format) {
    case AV_SAMPLE_FMT_U8:
        format1 = AV_SAMPLE_FMT_U8;
        format2 = AUDIO_U8;
        break;
    case AV_SAMPLE_FMT_S16:
        format1 = AV_SAMPLE_FMT_S16;
        format2 = AUDIO_S16;
        break;
    case AV_SAMPLE_FMT_S32:
        format1 = AV_SAMPLE_FMT_S32;
        format2 = AUDIO_S32;
        break;
    case AV_SAMPLE_FMT_FLT:
        format1 = AV_SAMPLE_FMT_FLT;
        format2 = AUDIO_F32;
        break;
    case AV_SAMPLE_FMT_DBL:
        format1 = AV_SAMPLE_FMT_FLT;
        format2 = AUDIO_F32;
        break;
    case AV_SAMPLE_FMT_U8P:
        format1 = AV_SAMPLE_FMT_U8;
        format2 = AUDIO_U8;
        break;
    case AV_SAMPLE_FMT_S16P:
        format1 = AV_SAMPLE_FMT_S16;
        format2 = AUDIO_S16;
        break;
    case AV_SAMPLE_FMT_S32P:
        format1 = AV_SAMPLE_FMT_S32;
        format2 = AUDIO_S32;
        break;
    case AV_SAMPLE_FMT_FLTP:
        format1 = AV_SAMPLE_FMT_FLT;
        format2 = AUDIO_F32;
        break;
    case AV_SAMPLE_FMT_DBLP:
        format1 = AV_SAMPLE_FMT_FLT;
        format2 = AUDIO_F32;
        break;
    case AV_SAMPLE_FMT_S64:
    case AV_SAMPLE_FMT_S64P:
        format1 = AV_SAMPLE_FMT_S32;
        format2 = AUDIO_S32;
        break;
    default:
        format1 = AV_SAMPLE_FMT_S16;
        format2 = AUDIO_S16;
        break;
    }

    RefPtr<AudioConverter> converter = AudioConverter::create(stream, format1);
    if (!converter) return nullptr;

    return from(
        format2,
        codecpar->sample_rate,
        codecpar->channels,
        codecpar->frame_size,
        av_q2d(stream->time_base),
        mutex,
        converter
    );
}

AudioRenderer::~AudioRenderer() {
    SDL_CloseAudioDevice(_audioDeviceID);
}

void AudioRenderer::fill(uint8_t* stream, int len) {
    RefPtr<Frame> frame = nullptr;
    {
        std::unique_lock<std::mutex> lock = _mutex->lock();
        if (!_frameList.empty()) {
            frame = _frameList.front();
            _frameList.pop_front();
        }
    }

    _mutex->notify();

    if (frame != nullptr) {
        double pts = frame->frame()->best_effort_timestamp * _timebase;

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

void AudioRenderer::push(RefPtr<Frame> frame) {
    RefPtr<Frame> converted = _converter->convert(frame);
    if (converted) {
        std::unique_lock<std::mutex> lock = _mutex->lock();
        _frameList.push_back(converted);
    }
}

void AudioRenderer::clear() {
    std::list<RefPtr<Frame>> queue;
    {
        std::unique_lock<std::mutex> lock = _mutex->lock();
        queue = std::move(_frameList);
    }
}

double AudioRenderer::duration() const {
    return _frameList.size() * _timebase * _audioSpec.samples;
}

VideoRenderer::~VideoRenderer() {
}

void VideoRenderer::attach(Consumer* consumer) {
    // 只考虑音频同步视频，反过来未实现；
}

void VideoRenderer::sync(double pts) {
    while (!_frameList.empty()) {
        double display_pts = DBL_MAX;
        RefPtr<Frame> display_frame = nullptr;
        for (RefPtr<Frame> frame : _frameList) {
            double current = frame->frame()->best_effort_timestamp * _timebase;
            if (display_pts > current) {
                display_frame = frame;
                display_pts = current;
            }
        }
        if (display_frame == nullptr) break;
        if (display_pts > pts) break;

        _frameList.remove(display_frame);
        _source->update(display_frame->frame());
        display_frame = nullptr;
        _callback();
    }
}

void VideoRenderer::play(bool state) {
    // TODO: 视频暂时只能被动等待音频的通知
}

void VideoRenderer::push(RefPtr<Frame> frame) {
    RefPtr<Frame> converted = _converter->convert(frame);
    if (converted) _frameList.push_back(converted);
}

void VideoRenderer::clear() {
    std::list<RefPtr<Frame>> queue = std::move(_frameList);
}

double VideoRenderer::duration() const {
    return _frameList.size() * _timebase;
}

RefPtr<VideoRenderer> VideoRenderer::from(AVStream* stream, RefPtr<render::VideoSource> source, std::function<void()> callback) {
    if (!stream) return nullptr;

    AVCodecParameters* codecpar = stream->codecpar;
    if (!codecpar) return nullptr;

    RefPtr<VideoRenderer> renderer = new VideoRenderer;
    renderer->_timebase = av_q2d(stream->time_base);
    renderer->_converter = VideoConverter::create(stream, AV_PIX_FMT_YUV420P);
    renderer->_source = source;
    renderer->_callback = callback;
    return renderer;
}
