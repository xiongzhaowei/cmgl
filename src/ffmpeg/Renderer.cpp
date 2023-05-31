//
// Created by 熊朝伟 on 2023-05-16.
//

#include "defines.h"
#include "Converter.h"
#include "Renderer.h"

OMP_FFMPEG_USING_NAMESPACE

static void SDLCALL __AudioRendererCallback(void* userdata, Uint8* stream, int len) {
    reinterpret_cast<AudioRenderer*>(userdata)->fill(stream, len);
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
