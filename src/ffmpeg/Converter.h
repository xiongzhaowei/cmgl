//
// Created by 熊朝伟 on 2023-05-27.
//

#pragma once

OMP_FFMPEG_NAMESPACE_BEGIN

struct Converter : public Object {
    virtual RefPtr<Frame> convert(RefPtr<Frame> frame) = 0;
};

class AudioConverter : public Converter {
    int32_t _sample_rate;
    AVChannelLayout _ch_layout;
    AVSampleFormat _format;
    SwrContext* _context;
    AudioConverter(SwrContext* context, AVSampleFormat format, AVChannelLayout ch_layout, int32_t sample_rate);
public:
    ~AudioConverter();

    RefPtr<Frame> convert(RefPtr<Frame> frame) override;

    static AudioConverter* create(AVStream* stream, AVSampleFormat format);
};

class VideoConverter : public Converter {
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
