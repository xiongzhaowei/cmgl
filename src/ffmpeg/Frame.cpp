//
// Created by 熊朝伟 on 2023-05-30.
//

#include "defines.h"

OMP_FFMPEG_USING_NAMESPACE

Frame::Frame() {
    _frame = av_frame_alloc();
}

Frame::~Frame() {
    if (_frame) av_frame_free(&_frame);
}

AVFrame* Frame::frame() const {
    return _frame;
}

bool Frame::setAudioBuffer(AVSampleFormat format, AVChannelLayout ch_layout, int32_t nb_samples) {
    _frame->format = format;
    _frame->ch_layout = ch_layout;
    _frame->nb_samples = nb_samples;

    if (av_frame_get_buffer(_frame, 1) < 0) return false;
    if (av_frame_make_writable(_frame) < 0) return false;

    return true;
}

bool Frame::setVideoBuffer(AVPixelFormat format, int32_t width, int32_t height) {
    _frame->format = format;
    _frame->width = width;
    _frame->height = height;

    if (av_frame_get_buffer(_frame, 1) < 0) return false;
    if (av_frame_make_writable(_frame) < 0) return false;

    return true;
}

RefPtr<Frame> Frame::alloc() {
    RefPtr<Frame> frame = new Frame();
    return frame->_frame != nullptr ? frame : nullptr;
}

RefPtr<Frame> Frame::alloc(AVSampleFormat format, AVChannelLayout ch_layout, int32_t nb_samples) {
    RefPtr<Frame> frame = new Frame();
    return frame && frame->setAudioBuffer(format, ch_layout, nb_samples) ? frame : nullptr;
}

RefPtr<Frame> Frame::alloc(AVPixelFormat format, int32_t width, int32_t height) {
    RefPtr<Frame> frame = new Frame();
    return frame && frame->setVideoBuffer(format, width, height) ? frame : nullptr;
}
