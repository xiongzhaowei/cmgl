//
// Created by 熊朝伟 on 2023-05-30.
//

#pragma once

OMP_FFMPEG_NAMESPACE_BEGIN

class Frame : public Object {
    AVFrame* _frame;
    Frame();
public:
    ~Frame();

    AVFrame* frame() const;
    bool setAudioBuffer(AVSampleFormat format, AVChannelLayout ch_layout, int32_t nb_samples);
    bool setVideoBuffer(AVPixelFormat format, int32_t width, int32_t height);

    static RefPtr<Frame> alloc();
    static RefPtr<Frame> alloc(AVSampleFormat format, AVChannelLayout ch_layout, int32_t nb_samples);
    static RefPtr<Frame> alloc(AVPixelFormat format, int32_t width, int32_t height);
};

OMP_FFMPEG_NAMESPACE_END
