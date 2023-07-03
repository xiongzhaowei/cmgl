//
// Created by 熊朝伟 on 2023-05-27.
//

#include "defines.h"
#include "Converter.h"

OMP_FFMPEG_USING_NAMESPACE

AudioConverter::AudioConverter(SwrContext* context, AVChannelLayout out_ch_layout, AVSampleFormat out_sample_fmt, int32_t out_sample_rate, AVChannelLayout in_ch_layout, AVSampleFormat in_sample_fmt, int32_t in_sample_rate) : _context(context), _out_ch_layout(out_ch_layout), _out_sample_fmt(out_sample_fmt), _out_sample_rate(out_sample_rate), _in_ch_layout(in_ch_layout), _in_sample_fmt(in_sample_fmt), _in_sample_rate(in_sample_rate) {

}

AudioConverter::~AudioConverter() {
    swr_free(&_context);
}

RefPtr<Frame> AudioConverter::convert(RefPtr<Frame> frame) {
    if (frame == nullptr) return nullptr;
    if (frame->frame() == nullptr) return nullptr;

    int32_t delay = (int32_t)swr_get_delay(_context, _in_sample_rate);
    int32_t out_nb_samples = (int32_t)av_rescale_rnd(frame->frame()->nb_samples + delay, _out_sample_rate, _in_sample_rate, AV_ROUND_UP);
    RefPtr<Frame> output = Frame::alloc(_out_sample_fmt, _out_ch_layout, out_nb_samples, _out_sample_rate);
    if (nullptr == output) return nullptr;

    if (Error::verify(swr_convert_frame(_context, output->frame(), frame->frame()), __FUNCSIG__, __LINE__)) {
        output->frame()->best_effort_timestamp = frame->frame()->best_effort_timestamp;
        output->frame()->pts = frame->frame()->pts;
        output->frame()->pkt_dts = frame->frame()->pkt_dts;
        output->frame()->pkt_pos = frame->frame()->pkt_pos;
        output->frame()->pkt_duration = frame->frame()->pkt_duration;
    } else {
        output = nullptr;
    }
    return output;
}

RefPtr<AudioConverter> AudioConverter::from(AVChannelLayout out_ch_layout, AVSampleFormat out_sample_fmt, int32_t out_sample_rate, AVChannelLayout in_ch_layout, AVSampleFormat in_sample_fmt, int32_t in_sample_rate) {
    SwrContext* context = nullptr;
    swr_alloc_set_opts2(&context, &out_ch_layout, out_sample_fmt, out_sample_rate, &in_ch_layout, in_sample_fmt, in_sample_rate, 0, nullptr);
    if (context) {
        if (swr_init(context) >= 0) {
            return new AudioConverter(context, out_ch_layout, out_sample_fmt, out_sample_rate, in_ch_layout, in_sample_fmt, in_sample_rate);
        }
        swr_free(&context);
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

    if (sws_scale(_context, (const uint8_t* const*)input->frame()->data, input->frame()->linesize, 0, _height, output->frame()->data, output->frame()->linesize) > 0) {
        output->frame()->best_effort_timestamp = input->frame()->best_effort_timestamp;
        output->frame()->pts = input->frame()->pts;
        output->frame()->pkt_dts = input->frame()->pkt_dts;
        output->frame()->pkt_pos = input->frame()->pkt_pos;
        output->frame()->pkt_duration = input->frame()->pkt_pos;
        return output;
    }

    return nullptr;
}

RefPtr<VideoConverter> VideoConverter::create(AVCodecParameters* codecpar, AVPixelFormat format) {
    if (codecpar == nullptr) return nullptr;
    if (codecpar->codec_type != AVMEDIA_TYPE_VIDEO) return nullptr;

    int width = codecpar->width;
    int height = codecpar->height;
    SwsContext* context = sws_getContext(
        width, height, (AVPixelFormat)codecpar->format,
        width, height, format,
        SWS_FAST_BILINEAR, NULL, NULL, NULL
    );

    return new VideoConverter(context, format, width, height);
}
