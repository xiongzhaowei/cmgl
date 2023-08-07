//
// Created by 熊朝伟 on 2023-05-16.
//

#pragma once

OMP_FFMPEG_NAMESPACE_BEGIN

class VideoRenderer : public Object {
public:
    static RefPtr<VideoRenderer> from(
        RefPtr<MovieBufferedConsumer> buffer,
        AVRational time_base,
        RefPtr<MovieThread> thread,
        AVRational frame_rate,
        std::function<void(RefPtr<Frame>)> callback
    );
    VideoRenderer(RefPtr<MovieBufferedConsumer> buffer, double time_base, RefPtr<MovieThread> thread, AVRational frame_rate, std::function<void(RefPtr<Frame>)> callback);
    ~VideoRenderer();

    void play(bool state);
    void sync(double pts);
    void clear();
    size_t size() const;
    int64_t timestamp() const;
private:
    double _time_base;
    RefPtr<MovieBufferedConsumer> _buffer;
    std::function<void(RefPtr<Frame>)> _callback;
    AVRational _frame_rate;
    RefPtr<Thread> _thread;
    RefPtr<Thread::ScheduledTask> _schedule;
};

class AudioRenderer : public Object {
public:
    static RefPtr<AudioRenderer> from(
        RefPtr<MovieBufferedConsumer> buffer,
        AVRational time_base,
        RefPtr<Thread> thread,
        AVSampleFormat format,
        AVChannelLayout ch_layout,
        int32_t sample_rate,
        int32_t frame_size
    );
    AudioRenderer(RefPtr<Thread> thread, RefPtr<MovieBufferedConsumer> buffer, RefPtr<AudioConverter> converter, AVSampleFormat format, AVChannelLayout ch_layout, double time_base);

    double volume() const;
    void setVolume(double volume);

    void fill(AVSampleFormat format, uint8_t* dst, uint8_t* src, size_t count, double volume);
    void fill(uint8_t* stream, int len);
    void attach(VideoRenderer* renderer);
    void play(bool state);
    void clear();
    void close();
    size_t size() const;
    int64_t timestamp() const;
private:
    double _time_base;
    RefPtr<MovieBufferedConsumer> _buffer;
    RefPtr<AudioConverter> _converter;
    RefPtr<VideoRenderer> _attached;
    RefPtr<Thread> _thread;
    RefPtr<Frame> _frame;
    int32_t _frameOffset = 0;
    AVSampleFormat _format;
    AVChannelLayout _ch_layout;
    uint32_t _device;
    double _timestamp;
    double _volume;
    std::atomic_bool _syncing = false;
};

OMP_FFMPEG_NAMESPACE_END
