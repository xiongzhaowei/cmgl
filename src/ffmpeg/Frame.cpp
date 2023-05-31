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

int64_t Frame::timestamp() const {
    return _frame->best_effort_timestamp;
}

void Frame::swap(RefPtr<Frame> frame) {
    AVFrame* temp = frame->_frame;
    frame->_frame = _frame;
    _frame = temp;
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

void FrameList::push(RefPtr<Frame> frame) {
    std::lock_guard<std::mutex> lock(_mutex);
    auto pred = [](RefPtr<Frame> left, RefPtr<Frame> right) {
        return left->timestamp() < right->timestamp();
    };
    _list.insert(std::lower_bound(_list.begin(), _list.end(), frame, pred), frame);
}

RefPtr<Frame> FrameList::pop() {
    std::lock_guard<std::mutex> lock(_mutex);
    RefPtr<Frame> frame;
    if (!_list.empty()) {
        frame = _list.front();
        _list.pop_front();
    }
    return frame;
}

RefPtr<Frame> FrameList::pop(int64_t timestamp) {
    std::lock_guard<std::mutex> lock(_mutex);
    RefPtr<Frame> frame;
    if (!_list.empty()) {
        if (_list.front()->timestamp() < timestamp) {
            frame = _list.front();
            _list.pop_front();
        }
    }
    return frame;
}

size_t FrameList::size() {
    std::lock_guard<std::mutex> lock(_mutex);
    return _list.size();
}

void FrameList::clear() {
    std::lock_guard<std::mutex> lock(_mutex);
    _list.clear();
}
