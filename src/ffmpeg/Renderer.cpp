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
        break;
    }
    RefPtr<AudioRenderer> renderer = new AudioRenderer(thread, buffer, converter, av_q2d(time_base) * frame_size);
    spec.userdata = renderer.value();
    renderer->_device = SDL_OpenAudioDevice(nullptr, 0, &spec, nullptr, 0);
    return (renderer->_device != 0) ? renderer : nullptr;
}

AudioRenderer::AudioRenderer(
    RefPtr<Thread> thread,
    RefPtr<MovieBufferedConsumer> buffer,
    RefPtr<AudioConverter> converter,
    double time_base
) : _thread(thread), _buffer(buffer), _converter(converter), _time_base(time_base), _device(0) {

}

AudioRenderer::~AudioRenderer() {
    if (_device != 0) {
        SDL_CloseAudioDevice(_device);
        _device = 0;
    }
}

void AudioRenderer::fill(uint8_t* stream, int len) {
    RefPtr<Frame> frame = _buffer->pop();
    if (frame != nullptr && _converter != nullptr) {
        frame = _converter->convert(frame);
    }

    if (frame != nullptr) {
        double pts = frame->timestamp() * _time_base;

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
        _thread->runOnThread([=]() {
            if (_attached) _attached->sync(pts);
        });
        frame = nullptr;
    }
}

void AudioRenderer::attach(VideoRenderer* renderer) {
    _attached = renderer;
}

void AudioRenderer::play(bool state) {
    SDL_PauseAudioDevice(_device, state ? 0 : 1);
}

RefPtr<VideoRenderer> VideoRenderer::from(
    RefPtr<MovieBufferedConsumer> buffer,
    AVRational time_base,
    std::function<void(RefPtr<Frame>)> callback
) {
    RefPtr<VideoRenderer> renderer = new VideoRenderer(buffer, av_q2d(time_base));
    renderer->_callback = callback;
    return renderer;
}

VideoRenderer::VideoRenderer(RefPtr<MovieBufferedConsumer> buffer, double time_base) : _buffer(buffer), _time_base(time_base) {

}

VideoRenderer::~VideoRenderer() {

}

void VideoRenderer::sync(double pts) {
    int64_t timestamp = pts / _time_base;
    while (RefPtr<Frame> frame = _buffer->pop(timestamp)) {
        _callback(frame);
    }
}
