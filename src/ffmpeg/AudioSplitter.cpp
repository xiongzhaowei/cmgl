//
// Created by 熊朝伟 on 2023-07-04.
//

#include "defines.h"

OMP_FFMPEG_USING_NAMESPACE

AudioSplitter::AudioSplitter(RefPtr<StreamConsumer<Frame>> output, AVSampleFormat format, AVChannelLayout ch_layout, int32_t sample_rate, int32_t nb_samples) : _output(output), _format(format), _ch_layout(ch_layout), _sample_rate(sample_rate), _sampleTotal(nb_samples) {
    assert(output != nullptr);
    assert(nb_samples > 0);
}

void AudioSplitter::copyData(uint8_t** data, int32_t offset, int32_t count, int32_t nb_channels) {
    assert(_frame != nullptr);
    if (_sampleCount + count >= _sampleTotal) {
        int32_t copyCount = _sampleTotal - _sampleCount;
        copy(data, offset, copyCount, nb_channels);
        offset += copyCount;
        count -= copyCount;

        _frame->frame()->pts = _timestamp;
        _output->add(_frame);

        if (count > 0) {
            _frame = Frame::alloc(_format, _ch_layout, _sampleTotal, _sample_rate);
            _sampleCount = 0;
            copyData(data, offset, count, nb_channels);
        } else {
            _frame = nullptr;
        }
    } else {
        copy(data, offset, count, nb_channels);
    }
}

void AudioSplitter::copy(uint8_t** data, int32_t offset, int32_t count, int32_t nb_channels) {
    int32_t bytesPerSample = _bytesPerSample;
    if (!_planar) {
        bytesPerSample *= nb_channels;
        nb_channels = 1;
    }
    for (int32_t channel = 0; channel < nb_channels; channel++) {
        memcpy(_frame->frame()->extended_data[channel] + _sampleCount * bytesPerSample, data[channel] + offset * bytesPerSample, count * bytesPerSample);
    }
    _timestamp += count;
    _sampleCount += count;
}

void AudioSplitter::add(RefPtr<Frame> frame) {
    if (frame == nullptr) return;
    if (frame->frame() == nullptr) return;

    if (_frame == nullptr) {
        AVSampleFormat format = (AVSampleFormat)frame->frame()->format;
        _frame = Frame::alloc(format, frame->frame()->ch_layout, _sampleTotal, frame->frame()->sample_rate);
        _planar = av_sample_fmt_is_planar(format);
        _bytesPerSample = av_get_bytes_per_sample(format);
        _sampleCount = 0;
    } else {
        assert(av_channel_layout_compare(&_frame->frame()->ch_layout, &frame->frame()->ch_layout) == 0);
        assert(_frame->frame()->format == frame->frame()->format);
        assert(_frame->frame()->sample_rate == frame->frame()->sample_rate);
    }
    copyData(frame->frame()->extended_data, 0, frame->frame()->nb_samples, frame->frame()->ch_layout.nb_channels);
}

void AudioSplitter::addError() {
    _output->addError();
}

void AudioSplitter::close() {
    _output->close();
}

bool AudioSplitter::available() const {
    return _output->available();
}
