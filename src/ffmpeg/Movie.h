//
// Created by 熊朝伟 on 2023-05-27.
//

#pragma once

OMP_FFMPEG_NAMESPACE_BEGIN

class WaitableEvent : public Object {
    std::mutex _mutex;
    std::condition_variable _notify;
public:
    void signal() { _notify.notify_one(); }
    void wait(std::function<bool()> pred) {
        std::unique_lock<std::mutex> lock(_mutex);
        _notify.wait(lock, pred);
    }
};

template <typename T>
class SafeQueue {
    std::list<T> _list;
    std::mutex _mutex;
public:
    void push(const T& object) {
        std::lock_guard<std::mutex> lock(_mutex);
        _list.push_back(object);
    }
    T pop() {
        std::lock_guard<std::mutex> lock(_mutex);
        T object;
        if (!_list.empty()) {
            object = _list.front();
            _list.pop_front();
        }
        return object;
    }
    void swap(std::list<T>& list) {
        std::lock_guard<std::mutex> lock(_mutex);
        _list.swap(list);
    }
    void clear() {
        std::lock_guard<std::mutex> lock(_mutex);
        _list.clear();
    }
    bool empty() {
        std::lock_guard<std::mutex> lock(_mutex);
        return _list.empty();
    }
    size_t size() {
        std::lock_guard<std::mutex> lock(_mutex);
        return _list.size();
    }
    void foreach(std::function<void(T&)> callback) {
        std::lock_guard<std::mutex> lock(_mutex);
        for (auto& item : _list) {
            callback(item);
        }
    }
    void remove(const T& object) {
        std::lock_guard<std::mutex> lock(_mutex);
        _list.remove(object);
    }
};

class Movie : public Object {
public:
    struct Converter;
    struct Consumer;
    class Stream;

    ~Movie();
    void play(bool state);
    void seek(double time, std::function<void()> callback);
    void start();
    void run();
    void runOnThread(std::function<void()> task);

    static Movie* from(const std::string& url, RefPtr<render::VideoSource> source, AVPixelFormat format, std::function<void()> callback);
private:
    Movie(AVFormatContext* format, Stream* audio, Stream* video, Consumer* audioConsumer, Consumer* videoConsumer, RefPtr<WaitableEvent> event);
    bool isNeedDecode();

    AVFormatContext* _format = nullptr;
    RefPtr<Stream> _audio;
    RefPtr<Stream> _video;
    RefPtr<Consumer> _audioConsumer;
    RefPtr<Consumer> _videoConsumer;
    std::unique_ptr<std::thread> _thread;
    SafeQueue<std::function<void()>> _taskQueue;
    RefPtr<WaitableEvent> _event;
};

class Movie::Stream : public Object {
    AVStream* _stream = nullptr;
    AVCodecContext* _context = nullptr;
    RefPtr<Converter> _converter;
    SafeQueue<RefPtr<Frame>> _frameList;
    RefPtr<WaitableEvent> _event;
    double _timebase;
    Stream(AVStream* stream, AVCodecContext* context);
public:
    ~Stream();

    AVStream* stream() const { return _stream; }
    bool decode(AVPacket* packet, AVFrame* frame);
    void process(AVPacket* packet, RefPtr<Frame> frame);
    double duration();
    void clear();
    RefPtr<Frame> pop();
    RefPtr<Frame> pop(int64_t pts);

    static Stream* from(AVStream* stream);
    static Stream* from(AVStream* stream, double timebase, RefPtr<WaitableEvent> event, Converter* converter);
};

struct Movie::Consumer : public Object {
    virtual void attach(Consumer* consumer) = 0;
    virtual void sync(double pts) = 0;
    virtual void play(bool state) = 0;
};

OMP_FFMPEG_NAMESPACE_END
