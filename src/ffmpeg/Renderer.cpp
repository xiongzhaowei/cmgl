//
// Created by 熊朝伟 on 2023-05-16.
//

#include "defines.h"
#include "Converter.h"
#include "Renderer.h"
#include "SDL2/SDL.h"

OMP_FFMPEG_USING_NAMESPACE

RefPtr<AudioRenderer> AudioRenderer::from(RefPtr<MovieBufferedConsumer> buffer, AVRational time_base, RefPtr<Thread> thread, AVSampleFormat format, AVChannelLayout ch_layout, int32_t sample_rate, int32_t frame_size) {
    SDL_Init(SDL_INIT_AUDIO);

    SDL_AudioSpec spec = { 0 };
    spec.freq = sample_rate;
    spec.channels = ch_layout.nb_channels;
    spec.silence = 0;
    spec.samples = frame_size;
    spec.callback = [](void* userdata, Uint8* stream, int len) {
        reinterpret_cast<AudioRenderer*>(userdata)->fill(stream, len);
    };

    RefPtr<AudioConverter> converter;
    switch (av_get_packed_sample_fmt(format)) {
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
        converter = AudioConverter::from(
            ch_layout, AV_SAMPLE_FMT_FLT, sample_rate,
            ch_layout, format, sample_rate
        );
        format = AV_SAMPLE_FMT_FLT;
        break;
    }
    RefPtr<AudioRenderer> renderer = new AudioRenderer(thread, buffer, converter, format, ch_layout, av_q2d(time_base));
    spec.userdata = renderer.value();
    renderer->retain();
    renderer->_device = SDL_OpenAudioDevice(nullptr, 0, &spec, nullptr, 0);
    return (renderer->_device != 0) ? renderer : nullptr;
}

AudioRenderer::AudioRenderer(
    RefPtr<Thread> thread,
    RefPtr<MovieBufferedConsumer> buffer,
    RefPtr<AudioConverter> converter,
    AVSampleFormat format,
    AVChannelLayout ch_layout,
    double time_base
) : _thread(thread), _buffer(buffer), _converter(converter), _format(format), _ch_layout(ch_layout), _time_base(time_base), _device(0), _timestamp(0), _volume(1) {

}

double AudioRenderer::volume() const {
    return _volume;
}

void AudioRenderer::setVolume(double volume) {
    _volume = volume;
}

template <typename T>
static typename std::enable_if<std::is_unsigned<T>::value>::type
mixAudio(T* dst, T* src, size_t count, double volume) {
    typedef typename std::make_signed<T>::type signed_t;
    constexpr signed_t minValue = static_cast<signed_t>(T(1) << (sizeof(T) * 8 - 1));
    constexpr signed_t maxValue = ~minValue;

    for (size_t i = 0; i < count; i++) {
        dst[i] = std::clamp(signed_t(signed_t(src[i] + minValue) * volume), minValue, maxValue) - minValue;
    }
}

template <typename T>
static typename std::enable_if<std::is_integral<T>::value && std::is_signed<T>::value>::type
mixAudio(T* dst, T* src, size_t count, double volume) {
    constexpr T minValue = static_cast<T>(typename std::make_unsigned<T>::type(1) << (sizeof(T) * 8 - 1));
    constexpr T maxValue = ~minValue;

    for (size_t i = 0; i < count; i++) {
        dst[i] = std::clamp(T(src[i] * volume), minValue, maxValue);
    }
}

template <typename T>
static typename std::enable_if<std::is_floating_point<T>::value>::type
mixAudio(T* dst, T* src, size_t count, double volume) {
    for (size_t i = 0; i < count; i++) {
        dst[i] = std::clamp<T>(T(src[i] * volume), -1, 1);
    }
}

static void mixAudio(AVSampleFormat format, uint8_t* dst, uint8_t* src, size_t count, double volume) {
    switch (av_get_alt_sample_fmt(format, 0)) {
    case AV_SAMPLE_FMT_U8: mixAudio<uint8_t>(dst, src, count, volume); break;
    case AV_SAMPLE_FMT_S16: mixAudio<int16_t>((int16_t*)dst, (int16_t*)src, count, volume); break;
    case AV_SAMPLE_FMT_S32: mixAudio<int32_t>((int32_t*)dst, (int32_t*)src, count, volume); break;
    case AV_SAMPLE_FMT_S64: mixAudio<int64_t>((int64_t*)dst, (int64_t*)src, count, volume); break;
    case AV_SAMPLE_FMT_FLT: mixAudio<float>((float*)dst, (float*)src, count, volume); break;
    case AV_SAMPLE_FMT_DBL: mixAudio<double>((double*)dst, (double*)src, count, volume); break;
    default: memcpy(dst, src, count * av_get_bytes_per_sample(format)); break;
    }
}

void AudioRenderer::fill(uint8_t* stream, int len) {
    RefPtr<AudioRenderer> self = this;

    int32_t bytesPerSample = av_get_bytes_per_sample(_format);
    int32_t nb_channels = _ch_layout.nb_channels;
    int32_t total_samples = len / bytesPerSample / nb_channels;

    int32_t offset = 0;

    if (_frame == nullptr) {
        _frame = _buffer->pop();
        if (_frame != nullptr && _converter != nullptr) {
            _frame = _converter->convert(_frame);
        }
        if (_frame != nullptr) {
            _timestamp = _frame->timestamp() * _time_base;
        }
    }
    while (_frame && _frame->frame()->nb_samples - _frameOffset > 0 && total_samples - offset > 0) {
        int32_t nb_samples = std::min(_frame->frame()->nb_samples - _frameOffset, total_samples - offset);
        if (av_sample_fmt_is_planar(_format)) {
            for (int32_t i = 0; i < nb_samples; i++) {
                for (int32_t channel = 0; channel < nb_channels; channel++) {
                    memcpy(
                        stream + (offset + i) * bytesPerSample * nb_channels + channel * bytesPerSample,
                        _frame->frame()->extended_data[channel] + (_frameOffset + i) * bytesPerSample,
                        bytesPerSample
                    );
                }
            }
            if (_volume != 1) {
                mixAudio(
                    _format,
                    stream + offset * bytesPerSample * nb_channels,
                    stream + offset * bytesPerSample * nb_channels,
                    nb_samples * nb_channels,
                    _volume
                );
            }
        } else {
            mixAudio(
                _format,
                stream + offset * bytesPerSample * nb_channels,
                _frame->frame()->extended_data[0] + _frameOffset * bytesPerSample * nb_channels,
                nb_samples * nb_channels,
                _volume
            );
        }
        offset += nb_samples;
        _frameOffset += nb_samples;

        if (_frame->frame()->nb_samples - _frameOffset == 0) {
            _frame = _buffer->pop();
            _frameOffset = 0;
            if (_frame != nullptr && _converter != nullptr) {
                _frame = _converter->convert(_frame);
            }
            if (_frame != nullptr) {
                _timestamp = _frame->timestamp() * _time_base;
            }
        }
    }
    if (total_samples - offset > 0) {
        int32_t count = total_samples - offset;
        _timestamp += count * _time_base;
        mixAudio(
            _format,
            stream + offset * bytesPerSample * nb_channels,
            stream + offset * bytesPerSample * nb_channels,
            count * nb_channels,
            0.8
        );
    }
    _thread->runOnThread([self]() {
        if (self->_attached) self->_attached->sync(self->_timestamp);
    });
}

void AudioRenderer::attach(VideoRenderer* renderer) {
    _attached = renderer;
}

void AudioRenderer::play(bool state) {
    SDL_PauseAudioDevice(_device, state ? 0 : 1);
}

void AudioRenderer::clear() {
    if (_buffer) _buffer->clear();
}

void AudioRenderer::close() {
    if (_device != 0) {
        SDL_CloseAudioDevice(_device);
        _device = 0;
        release();
    }
}

size_t AudioRenderer::size() const {
    return _buffer ? _buffer->size() : 0;
}

int64_t AudioRenderer::timestamp() const {
    return int64_t(_timestamp * AV_TIME_BASE);
}

RefPtr<VideoRenderer> VideoRenderer::from(
    RefPtr<MovieBufferedConsumer> buffer,
    AVRational time_base,
    RefPtr<MovieThread> thread,
    AVRational frame_rate,
    std::function<void(RefPtr<Frame>)> callback
) {
    return new VideoRenderer(buffer, av_q2d(time_base), thread, frame_rate, callback);
}

VideoRenderer::VideoRenderer(RefPtr<MovieBufferedConsumer> buffer, double time_base, RefPtr<MovieThread> thread, AVRational frame_rate, std::function<void(RefPtr<Frame>)> callback) : _buffer(buffer), _time_base(time_base), _thread(thread), _frame_rate(frame_rate), _callback(callback) {

}

VideoRenderer::~VideoRenderer() {

}

void VideoRenderer::play(bool state) {
    if (state) {
        if (_schedule == nullptr) {
            _schedule = _thread->schedule(1.0 / av_q2d(_frame_rate), [this, self = RefPtr<VideoRenderer>(this)]() {
                RefPtr<Frame> frame = _buffer->pop();
                if (frame != nullptr) _callback(frame);
                return true;
            });
        }
    } else {
        if (_schedule) {
            _thread->cancel(_schedule);
            _schedule = nullptr;
        }
    }
}

void VideoRenderer::sync(double pts) {
    int64_t timestamp = pts / _time_base;
    while (RefPtr<Frame> frame = _buffer->pop(timestamp)) {
        _callback(frame);
    }
}

void VideoRenderer::clear() {
    if (_buffer) _buffer->clear();
}

size_t VideoRenderer::size() const {
    return _buffer ? _buffer->size() : 0;
}

int64_t VideoRenderer::timestamp() const {
    int64_t ts = _buffer->timestamp();
    if (ts != -1) {
        return ts * _time_base * AV_TIME_BASE;
    }
    return -1;
}
