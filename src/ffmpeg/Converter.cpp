//
// Created by 熊朝伟 on 2023-05-27.
//

#include "defines.h"
#include "Converter.h"

OMP_FFMPEG_USING_NAMESPACE

AudioConverter::AudioConverter(SwrContext* context, AVSampleFormat format, AVChannelLayout ch_layout, int32_t sample_rate) : _context(context), _format(format), _ch_layout(ch_layout), _sample_rate(sample_rate) {

}

AudioConverter::~AudioConverter() {
    if (_context) {
        swr_free(&_context);
        _context = nullptr;
    }
}

RefPtr<Frame> AudioConverter::convert(RefPtr<Frame> frame) {
    RefPtr<Frame> output = Frame::alloc(_format, _ch_layout, frame->frame()->nb_samples);
    if (nullptr == output) return nullptr;

    const int bytesPerSample = av_get_bytes_per_sample(_format);
    int samples_size = frame->frame()->nb_samples * frame->frame()->channels * bytesPerSample;

    int nb_samples = swr_convert(_context, output->frame()->extended_data, samples_size, (const uint8_t**)frame->frame()->extended_data, frame->frame()->nb_samples);
    if (nb_samples > 0) {
        output->frame()->best_effort_timestamp = frame->frame()->best_effort_timestamp;
        output->frame()->nb_samples = nb_samples;
        output->frame()->sample_rate = _sample_rate;
        output->frame()->pts = frame->frame()->pts;
        output->frame()->pkt_dts = frame->frame()->pkt_dts;
        output->frame()->pkt_pos = frame->frame()->pkt_pos;
        output->frame()->pkt_duration = frame->frame()->pkt_duration;
        return output;
    }

    return nullptr;
}

AudioConverter* AudioConverter::create(AVStream* stream, AVSampleFormat format, AVChannelLayout ch_layout) {
    if (stream->codecpar->codec_type != AVMEDIA_TYPE_AUDIO) return nullptr;

    SwrContext* swrContext = NULL;
    swr_alloc_set_opts2(&swrContext, &ch_layout, format, stream->codecpar->sample_rate, &stream->codecpar->ch_layout, (AVSampleFormat)stream->codecpar->format, stream->codecpar->sample_rate, 0, nullptr);
    if (swrContext) {
        if (swr_init(swrContext) >= 0) {
            return new AudioConverter(swrContext, format, stream->codecpar->ch_layout, stream->codecpar->sample_rate);
        }
        swr_free(&swrContext);
    }
    return nullptr;
}

VideoConverter::VideoConverter(SwsContext* context, AVPixelFormat format, int width, int height) : _context(context), _format(format), _width(width), _height(height) {

}

VideoConverter::~VideoConverter() {
    if (!_context) {
        sws_freeContext(_context);
        _context = nullptr;
        _width = 0;
        _height = 0;
    }
}

RefPtr<Frame> VideoConverter::convert(RefPtr<Frame> input) {
    RefPtr<Frame> output = Frame::alloc(_format, _width, _height);
    if (nullptr == output) return nullptr;
    output->frame()->best_effort_timestamp = input->frame()->best_effort_timestamp;
    output->frame()->pts = input->frame()->pts;
    output->frame()->pkt_dts = input->frame()->pkt_dts;
    output->frame()->pkt_pos = input->frame()->pkt_pos;
    output->frame()->pkt_duration = input->frame()->pkt_pos;

    if (sws_scale(_context, (const uint8_t* const*)input->frame()->data, input->frame()->linesize, 0, _height, output->frame()->data, output->frame()->linesize) > 0) {
        return output;
    }

    return nullptr;
}

VideoConverter* VideoConverter::create(AVStream* stream, AVPixelFormat format) {
    if (!stream) return nullptr;
    if (!stream->codecpar) return nullptr;
    if (stream->codecpar->codec_type != AVMEDIA_TYPE_VIDEO) return nullptr;

    AVCodecParameters* codecpar = stream->codecpar;
    int width = codecpar->width;
    int height = codecpar->height;
    SwsContext* context = sws_getContext(
        width, height, (AVPixelFormat)codecpar->format,
        width, height, format,
        SWS_FAST_BILINEAR, NULL, NULL, NULL
    );

    return new VideoConverter(context, format, width, height);
}
