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

AVFrame* AudioConverter::convert(AVFrame* frame) {
    AVFrame* output = alloc(_format, _ch_layout, frame->nb_samples);
    if (nullptr == output) return nullptr;

    const int bytesPerSample = av_get_bytes_per_sample(_format);
    int samples_size = frame->nb_samples * frame->channels * bytesPerSample;

    int nb_samples = swr_convert(_context, output->extended_data, samples_size, (const uint8_t**)frame->extended_data, frame->nb_samples);
    if (nb_samples > 0) {
        output->best_effort_timestamp = frame->best_effort_timestamp;
        output->nb_samples = nb_samples;
        output->sample_rate = _sample_rate;
        output->pts = frame->pts;
        output->pkt_dts = frame->pkt_dts;
        output->pkt_pos = frame->pkt_pos;
        output->pkt_duration = frame->pkt_duration;
        return output;
    }

    av_frame_free(&output);
    return nullptr;
}

RefPtr<AudioConverter> AudioConverter::create(AVStream* stream, AVSampleFormat format) {
    if (stream->codecpar->codec_type != AVMEDIA_TYPE_AUDIO) return nullptr;

    SwrContext* swrContext = NULL;
    swr_alloc_set_opts2(&swrContext, &stream->codecpar->ch_layout, format, stream->codecpar->sample_rate, &stream->codecpar->ch_layout, (AVSampleFormat)stream->codecpar->format, stream->codecpar->sample_rate, 0, nullptr);
    if (swrContext) {
        if (swr_init(swrContext) >= 0) {
            return new AudioConverter(swrContext, format, stream->codecpar->ch_layout, stream->codecpar->sample_rate);
        }
        swr_free(&swrContext);
    }
    return nullptr;
}

AVFrame* AudioConverter::alloc(AVSampleFormat format, AVChannelLayout ch_layout, int32_t nb_samples) {
    AVFrame* frame = av_frame_alloc();
    if (frame == nullptr) return nullptr;

    frame->nb_samples = nb_samples;
    frame->format = format;
    frame->ch_layout = ch_layout;

    if (av_frame_get_buffer(frame, 1) < 0) {
        av_frame_free(&frame);
        return nullptr;
    }

    if (av_frame_make_writable(frame) < 0) {
        av_frame_free(&frame);
        return nullptr;
    }

    return frame;
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

AVFrame* VideoConverter::convert(AVFrame* input) {
    AVFrame* output = alloc(_format, _width, _height);
    if (nullptr == output) return nullptr;
    output->best_effort_timestamp = input->best_effort_timestamp;
    output->pts = input->pts;
    output->pkt_dts = input->pkt_dts;
    output->pkt_pos = input->pkt_pos;
    output->pkt_duration = input->pkt_pos;

    if (sws_scale(_context, (const uint8_t* const*)input->data, input->linesize, 0, _height, output->data, output->linesize) > 0) {
        return output;
    }

    av_frame_free(&output);
    return nullptr;
}

RefPtr<VideoConverter> VideoConverter::create(AVStream* stream, AVPixelFormat format) {
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

AVFrame* VideoConverter::alloc(AVPixelFormat format, int32_t width, int32_t height) {
    AVFrame* frame = av_frame_alloc();
    if (!frame) return nullptr;

    frame->format = format;
    frame->width = width;
    frame->height = height;

    if (av_frame_get_buffer(frame, 1) < 0) {
        av_frame_free(&frame);
        return nullptr;
    }

    if (av_frame_make_writable(frame) < 0) {
        av_frame_free(&frame);
        return nullptr;
    }

    return frame;
}
