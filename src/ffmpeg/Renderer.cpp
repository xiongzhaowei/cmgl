//
// Created by 熊朝伟 on 2023-05-16.
//

#include "defines.h"
#include "Converter.h"
#include "Renderer.h"

OMP_FFMPEG_USING_NAMESPACE

Renderer::Renderer(RefPtr<Thread> thread, RefPtr<Stream<Frame>> stream, AVCodecParameters* codecpar, AVRational time_base) : _thread(thread) {
    _timebase = av_q2d(time_base);
    if (codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
        _timebase *= codecpar->frame_size;
    }
    _subscription = stream->listen(this);
}

void Renderer::add(RefPtr<Frame> frame) {
    if (frame == nullptr) return;

    RefPtr<Frame> video;
    if (_converter) {
        video = _converter->convert(frame);
    } else {
        video = Frame::alloc();
        video->swap(frame);
    }
    _frameList.push(video);
    if (duration() >= 0.1) {
        _isPaused = true;
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

bool Renderer::available() const {
    return !_isPaused;
}

double Renderer::duration() {
    return _frameList.size() * _timebase;
}

static void SDLCALL __AudioRendererCallback(void* userdata, Uint8* stream, int len) {
    reinterpret_cast<AudioRenderer*>(userdata)->fill(stream, len);
}

RefPtr<AudioRenderer> AudioRenderer::from(RefPtr<Thread> thread, RefPtr<Stream<Frame>> stream, AVCodecParameters* codecpar, AVRational time_base) {
    SDL_Init(SDL_INIT_AUDIO);

    SDL_AudioSpec spec = { 0 };
    spec.freq = codecpar->sample_rate;
    spec.channels = codecpar->ch_layout.nb_channels;
    spec.silence = 0;
    spec.samples = codecpar->frame_size;
    spec.callback = __AudioRendererCallback;

    switch (av_get_packed_sample_fmt((AVSampleFormat)codecpar->format)) {
        case AV_SAMPLE_FMT_U8:
            spec.format = AUDIO_U8;
            break;
        case AV_SAMPLE_FMT_S16:
            spec.format = AUDIO_S16SYS;
            break;
        case AV_SAMPLE_FMT_S32:
            spec.format = AUDIO_S32SYS;
            break;
        case AV_SAMPLE_FMT_FLT:
            spec.format = AUDIO_F32SYS;
            break;
        default:
            spec.format = AUDIO_F32SYS;
            stream = stream->convert(AudioConverter::from(
                codecpar->ch_layout, AV_SAMPLE_FMT_FLT, codecpar->sample_rate,
                codecpar->ch_layout, (AVSampleFormat)codecpar->format, codecpar->sample_rate
            ));
            break;
    }
    RefPtr<AudioRenderer> renderer = new AudioRenderer(thread, stream, codecpar, time_base);
    spec.userdata = renderer.value();
    renderer->_audioDeviceID = SDL_OpenAudioDevice(NULL, 0, &spec, &renderer->_audioSpec, 0);
    return (renderer->_audioDeviceID != 0) ? renderer : nullptr;
}

AudioRenderer::AudioRenderer(RefPtr<Thread> thread, RefPtr<Stream<Frame>> stream, AVCodecParameters* codecpar, AVRational time_base) : Renderer(thread, stream, codecpar, time_base) {
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

        int32_t bytesPerSample = av_get_bytes_per_sample((AVSampleFormat)frame->frame()->format);
        int32_t nb_channels = frame->frame()->ch_layout.nb_channels;
        int32_t count = std::max(std::min(frame->frame()->linesize[0], len), 0);
        if (av_sample_fmt_is_planar((AVSampleFormat)frame->frame()->format)) {
            count /= bytesPerSample * nb_channels;
            for (int32_t i = 0; i < count; i++) {
                for (int32_t channel = 0; channel < nb_channels; channel++) {
                    memcpy(
                        stream + i * bytesPerSample * nb_channels + channel * bytesPerSample,
                        frame->frame()->extended_data[channel] + i * bytesPerSample,
                        bytesPerSample
                    );
                }
            }
        } else {
            memcpy(stream, frame->frame()->extended_data[0], count);
        }
        frame = nullptr;

        if (_video) _video->sync(pts);
    }
    if (duration() < 0.1) {
        _isPaused = false;
        _thread->runOnThread([]() {});
    }
}

void AudioRenderer::attach(Renderer* renderer) {
    _video = renderer;
}

void AudioRenderer::play(bool state) {
    SDL_PauseAudioDevice(_audioDeviceID, state ? 0 : 1);
}

RefPtr<VideoRenderer> VideoRenderer::from(RefPtr<Thread> thread, RefPtr<Stream<Frame>> stream, AVCodecParameters* codecpar, AVRational time_base, RefPtr<render::VideoSource> source, AVPixelFormat format, std::function<void()> callback) {
    if (!stream) return nullptr;

    if (codecpar->format != format) {
        stream = stream->convert(VideoConverter::create(codecpar, format));
    }
    RefPtr<VideoRenderer> renderer = new VideoRenderer(thread, stream, codecpar, time_base);
    renderer->_source = source;
    renderer->_callback = callback;
    return renderer;
}

VideoRenderer::VideoRenderer(RefPtr<Thread> thread, RefPtr<Stream<Frame>> stream, AVCodecParameters* codecpar, AVRational time_base) : Renderer(thread, stream, codecpar, time_base) {
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
        _isPaused = false;
        _thread->runOnThread([]() {});
    }
}
