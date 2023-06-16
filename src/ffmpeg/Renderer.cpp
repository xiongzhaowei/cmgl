//
// Created by 熊朝伟 on 2023-05-16.
//

#include "defines.h"
#include "Converter.h"
#include "Renderer.h"

OMP_FFMPEG_USING_NAMESPACE

Renderer::Renderer(DecoderStream* stream) {
    AVCodecParameters* codecpar = stream->stream()->codecpar;
    _timebase = av_q2d(stream->stream()->time_base);
    if (codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
        _timebase *= codecpar->frame_size;
    }
    _subscription = stream->listen(this);
}

void Renderer::add(RefPtr<Frame> frame) {
    RefPtr<Frame> video;
    if (_converter) {
        video = _converter->convert(frame);
    } else {
        video = Frame::alloc();
        video->swap(frame);
    }
    _frameList.push(video);
    if (duration() >= 0.1) {
        _subscription->pause();
    }
}

void Renderer::addError() {
}

void Renderer::close() {
    if (_subscription != nullptr) {
        _subscription->cancel();
        _subscription = nullptr;
    }
}

double Renderer::duration() {
    return _frameList.size() * _timebase;
}

static void SDLCALL __AudioRendererCallback(void* userdata, Uint8* stream, int len) {
    reinterpret_cast<AudioRenderer*>(userdata)->fill(stream, len);
}

RefPtr<AudioRenderer> AudioRenderer::from(DecoderStream* stream, SDL_AudioFormat format) {
    SDL_Init(SDL_INIT_AUDIO);

    AVCodecParameters* codecpar = stream->stream()->codecpar;
    RefPtr<AudioRenderer> renderer = new AudioRenderer(stream);

    SDL_AudioSpec spec = { 0 };
    spec.freq = codecpar->sample_rate;
    spec.format = format;
    spec.channels = codecpar->channels;
    spec.silence = 0;
    spec.samples = codecpar->frame_size;
    spec.callback = __AudioRendererCallback;
    spec.userdata = renderer.value();

    renderer->_audioDeviceID = SDL_OpenAudioDevice(NULL, 0, &spec, &renderer->_audioSpec, 0);
    return (renderer->_audioDeviceID != 0) ? renderer : nullptr;
}

AudioRenderer::AudioRenderer(DecoderStream* stream) : Renderer(stream) {
}

AudioRenderer::~AudioRenderer() {
    if (_audioDeviceID != 0) {
        SDL_CloseAudioDevice(_audioDeviceID);
        _audioDeviceID = 0;
    }
}

void AudioRenderer::fill(uint8_t* stream, int len) {
    RefPtr<Frame> frame = _frameList.pop();

    if (frame != nullptr) {
        double pts = frame->timestamp() * _timebase;

        SDL_memset(stream, 0, len);
        SDL_MixAudioFormat(stream, frame->frame()->extended_data[0], _audioSpec.format, std::min<uint32_t>(frame->frame()->linesize[0], len), SDL_MIX_MAXVOLUME);
        frame = nullptr;

        if (_video) _video->sync(pts);
    }
    if (duration() < 0.1) {
        _subscription->resume();
    }
}

void AudioRenderer::attach(Renderer* renderer) {
    _video = renderer;
}

void AudioRenderer::play(bool state) {
    SDL_PauseAudioDevice(_audioDeviceID, state ? 0 : 1);
}

RefPtr<VideoRenderer> VideoRenderer::from(DecoderStream* stream, RefPtr<render::VideoSource> source, std::function<void()> callback) {
    if (!stream) return nullptr;

    RefPtr<VideoRenderer> renderer = new VideoRenderer(stream);
    renderer->_source = source;
    renderer->_callback = callback;
    return renderer;
}

VideoRenderer::VideoRenderer(DecoderStream* stream) : Renderer(stream) {
}

VideoRenderer::~VideoRenderer() {
}

void VideoRenderer::sync(double pts) {
    int64_t timestamp = pts / _timebase;
    while (RefPtr<Frame> frame = _frameList.pop(timestamp)) {
        _source->update(frame->frame());
        frame = nullptr;
        _callback();
    }
    if (duration() < 0.1) {
        _subscription->resume();
    }
}
