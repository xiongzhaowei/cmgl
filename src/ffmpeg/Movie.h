//
// Created by 熊朝伟 on 2023-05-27.
//

#pragma once

OMP_FFMPEG_NAMESPACE_BEGIN

class Mutex : public Object {
    std::mutex _mutex;
    std::condition_variable _waiter;
public:
    std::unique_lock<std::mutex> lock();
    void wait(std::function<bool()> pred);
    void notify();
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

    static Movie* from(const std::string& url, std::function<void(AVFrame*)> callback);
private:
    Movie(AVFormatContext* format, Stream* audio, Stream* video, Mutex* mutex);

    AVFormatContext* _format = nullptr;
    RefPtr<Stream> _audio;
    RefPtr<Stream> _video;
    std::unique_ptr<std::thread> _thread;
    std::list<std::function<void()>> _taskQueue;
    RefPtr<Mutex> _mutex;
};

class Movie::Stream : public Object {
    AVStream* _stream = nullptr;
    AVCodecContext* _context = nullptr;
    RefPtr<Consumer> _consumer;
    Stream(AVStream* stream, AVCodecContext* context);
public:
    ~Stream();

    AVStream* stream() const { return _stream; }
    RefPtr<Consumer> consumer() const { return _consumer; }
    bool decode(AVPacket* packet, AVFrame* frame);
    void process(AVPacket* packet, AVFrame* frame);

    static Stream* from(AVStream* stream);
    static Stream* from(AVStream* stream, Consumer* consumer);
};

struct Movie::Consumer : public Object {
    virtual void attach(Consumer* consumer) = 0;
    virtual void sync(double pts) = 0;
    virtual void play(bool state) = 0;
    virtual void push(AVFrame* frame) = 0;
    virtual void clear() = 0;
    virtual double duration() const = 0;
};

OMP_FFMPEG_NAMESPACE_END
