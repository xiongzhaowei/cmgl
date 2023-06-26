//
// Created by 熊朝伟 on 2023-05-27.
//

#pragma once

OMP_FFMPEG_NAMESPACE_BEGIN

class AudioConverter : public Converter<Frame> {
    AVChannelLayout _in_ch_layout;
    AVChannelLayout _out_ch_layout;
    AVSampleFormat _in_sample_fmt;
    AVSampleFormat _out_sample_fmt;
    int32_t _in_sample_rate;
    int32_t _out_sample_rate;
    SwrContext* _context;
    AudioConverter(
        SwrContext* context,
        AVChannelLayout out_ch_layout,
        AVSampleFormat out_sample_fmt,
        int32_t out_sample_rate,
        AVChannelLayout in_ch_layout,
        AVSampleFormat in_sample_fmt,
        int32_t in_sample_rate
    );
public:
    ~AudioConverter();

    RefPtr<Frame> convert(RefPtr<Frame> frame) override;

    static RefPtr<AudioConverter> from(
        AVChannelLayout out_ch_layout,
        AVSampleFormat out_sample_fmt,
        int32_t out_sample_rate,
        AVChannelLayout in_ch_layout,
        AVSampleFormat in_sample_fmt,
        int32_t in_sample_rate
    );
};

class VideoConverter : public Converter<Frame> {
    int32_t _width;
    int32_t _height;
    AVPixelFormat _format;
    SwsContext* _context = nullptr;
    VideoConverter(SwsContext* context, AVPixelFormat format, int width, int height);
public:
    ~VideoConverter();

    RefPtr<Frame> convert(RefPtr<Frame> input) override;

    static VideoConverter* create(AVStream* stream, AVPixelFormat format);
};

OMP_FFMPEG_NAMESPACE_END
