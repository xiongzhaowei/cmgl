//
// Created by 熊朝伟 on 2023-05-27.
//

#pragma once

OMP_FFMPEG_NAMESPACE_BEGIN

struct Movie::Converter : public Object {
    virtual AVFrame* convert(AVFrame* frame) = 0;
};

class AudioConverter : public Movie::Converter {
    int32_t _sample_rate;
    AVChannelLayout _ch_layout;
    AVSampleFormat _format;
    SwrContext* _context;
    AudioConverter(SwrContext* context, AVSampleFormat format, AVChannelLayout ch_layout, int32_t sample_rate);
public:
    ~AudioConverter();

    AVFrame* convert(AVFrame* frame) override;

    static RefPtr<AudioConverter> create(AVStream* stream, AVSampleFormat format);
    static AVFrame* alloc(AVSampleFormat format, AVChannelLayout ch_layout, int32_t nb_samples);
};

class VideoConverter : public Movie::Converter {
    int32_t _width;
    int32_t _height;
    AVPixelFormat _format;
    SwsContext* _context = nullptr;
    VideoConverter(SwsContext* context, AVPixelFormat format, int width, int height);
public:
    ~VideoConverter();

    AVFrame* convert(AVFrame* input) override;

    static RefPtr<VideoConverter> create(AVStream* stream, AVPixelFormat format);
    static AVFrame* alloc(AVPixelFormat format, int32_t width, int32_t height);
};

OMP_FFMPEG_NAMESPACE_END
