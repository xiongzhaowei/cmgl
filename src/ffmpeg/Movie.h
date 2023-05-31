//
// Created by 熊朝伟 on 2023-05-27.
//

#pragma once

OMP_FFMPEG_NAMESPACE_BEGIN

class Movie : public Object {
public:
    struct Converter;
    struct Consumer;
    class Stream;

    ~Movie();

    RefPtr<Stream> audio();
    RefPtr<Stream> video();

    bool detect();
    bool decode(AVPacket* packet, RefPtr<Frame> frame);
    void seek(double time, std::function<void()> callback = std::function<void()>());

    static Movie* from(const std::string& url, MovieThread* thread, AVPixelFormat pixel, AVSampleFormat sample);
private:
    Movie(AVFormatContext* format, Stream* audio, Stream* video, MovieThread* thread);

    AVFormatContext* _format = nullptr;
    RefPtr<Stream> _audio;
    RefPtr<Stream> _video;
    WeakPtr<MovieThread> _thread;
};

class Movie::Stream : public Object {
    AVStream* _stream = nullptr;
    AVCodecContext* _context = nullptr;
    RefPtr<Converter> _converter;
    FrameList _frameList;
    RefPtr<WaitableEvent> _event;
    double _timebase;
    Stream(AVStream* stream, AVCodecContext* context);
public:
    ~Stream();

    AVStream* stream() const;
    bool decode(AVPacket* packet, RefPtr<Frame> frame);
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
