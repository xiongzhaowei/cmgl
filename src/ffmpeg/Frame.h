//
// Created by 熊朝伟 on 2023-05-30.
//

#pragma once

OMP_FFMPEG_NAMESPACE_BEGIN

class Packet : public Object {
    AVPacket* _packet;
public:
    Packet();
    ~Packet();

    AVPacket* packet() const;
    void reset();
};

class Frame : public Object {
    AVFrame* _frame;
    Frame();
public:
    ~Frame();

    AVFrame* frame() const;
    int64_t timestamp() const;
    void swap(RefPtr<Frame> frame);
    bool setAudioBuffer(AVSampleFormat format, AVChannelLayout ch_layout, int32_t nb_samples, int32_t sample_rate);
    bool setVideoBuffer(AVPixelFormat format, int32_t width, int32_t height);

    static RefPtr<Frame> alloc();
    static RefPtr<Frame> alloc(AVSampleFormat format, AVChannelLayout ch_layout, int32_t nb_samples, int32_t sample_rate);
    static RefPtr<Frame> alloc(AVPixelFormat format, int32_t width, int32_t height);
};

class Dictionary : public Object {
    AVDictionary* _dictionary = nullptr;
public:
    Dictionary() = default;
    Dictionary(std::map<std::string, std::string> dict);
    Dictionary(std::vector<std::pair<std::string, std::string>> dict);
    ~Dictionary();

    int32_t count() const;
    AVDictionary* dictionary() const;
    std::vector<std::string> get(const std::string& key);
    void add(const std::string& key, const std::string& value);
    void set(const std::string& key, const std::string& value);
    void remove(const std::string& key);
};

class FrameList {
    std::list<RefPtr<Frame>> _list;
    std::mutex _mutex;
public:
    void push(RefPtr<Frame> frame);
    RefPtr<Frame> pop();
    RefPtr<Frame> pop(int64_t timestamp);
    size_t size();
    void clear();
};

OMP_FFMPEG_NAMESPACE_END
