//
// Created by 熊朝伟 on 2023-05-30.
//

#include "defines.h"

OMP_FFMPEG_USING_NAMESPACE

Packet::Packet() : _packet(av_packet_alloc()) {
}

Packet::~Packet() {
	av_packet_free(&_packet);
}

AVPacket* Packet::packet() const {
	return _packet;
}

void Packet::reset() {
	av_packet_unref(_packet);
}

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

bool Frame::setAudioBuffer(AVSampleFormat format, AVChannelLayout ch_layout, int32_t nb_samples, int32_t sample_rate) {
    _frame->format = format;
    _frame->ch_layout = ch_layout;
    _frame->nb_samples = nb_samples;
    _frame->sample_rate = sample_rate;

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

RefPtr<Frame> Frame::alloc(AVSampleFormat format, AVChannelLayout ch_layout, int32_t nb_samples, int32_t sample_rate) {
    RefPtr<Frame> frame = new Frame();
    return frame && frame->setAudioBuffer(format, ch_layout, nb_samples, sample_rate) ? frame : nullptr;
}

RefPtr<Frame> Frame::alloc(AVPixelFormat format, int32_t width, int32_t height) {
    RefPtr<Frame> frame = new Frame();
    return frame && frame->setVideoBuffer(format, width, height) ? frame : nullptr;
}

Dictionary::Dictionary(std::map<std::string, std::string> dict) {
    for (auto item : dict) {
        av_dict_set(&_dictionary, item.first.c_str(), item.second.c_str(), 0);
    }
}

Dictionary::Dictionary(std::vector<std::pair<std::string, std::string>> dict) {
    for (auto item : dict) {
        av_dict_set(&_dictionary, item.first.c_str(), item.second.c_str(), AV_DICT_MULTIKEY);
    }
}

Dictionary::~Dictionary() {
    av_dict_free(&_dictionary);
}

AVDictionary* Dictionary::dictionary() const {
    return _dictionary;
}

int32_t Dictionary::count() const {
    return av_dict_count(_dictionary);
}

std::vector<std::string> Dictionary::get(const std::string& key) {
    std::vector<std::string> result;
    AVDictionaryEntry* entry = av_dict_get(_dictionary, key.c_str(), nullptr, 0);
    while (entry != nullptr) {
        if (entry->value != nullptr) result.push_back(entry->value);
        entry = av_dict_get(_dictionary, key.c_str(), entry, 0);
    }
    return result;
}

void Dictionary::add(const std::string& key, const std::string& value) {
    av_dict_set(&_dictionary, key.c_str(), value.c_str(), AV_DICT_MULTIKEY);
}

void Dictionary::set(const std::string& key, const std::string& value) {
    av_dict_set(&_dictionary, key.c_str(), value.c_str(), 0);
}

void Dictionary::remove(const std::string& key) {
    av_dict_set(&_dictionary, key.c_str(), nullptr, 0);
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
