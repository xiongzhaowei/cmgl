//
// Created by 熊朝伟 on 2023-06-01.
//

#pragma once

OMP_FFMPEG_NAMESPACE_BEGIN

class DecoderStream : public Object {
    friend class Decoder;
    AVStream* _stream;
    AVCodecContext* _context;
    RefPtr<StreamController<Frame>> _controller;
public:
    static DecoderStream* from(AVStream* stream, Thread* thread);

    DecoderStream(AVStream* stream, AVCodecContext* context, Thread* thread);
    ~DecoderStream();

    RefPtr<Stream<Frame>> stream() const;
    AVCodecParameters* codecpar() const;
    bool available() const;
    bool match(AVFormatContext* format, AVPacket* packet) const;
    bool decode(AVPacket* packet, RefPtr<Frame> frame);
};

class Decoder : public Object {
    AVFormatContext* _format = nullptr;
    RefPtr<DecoderStream> _audio;
    RefPtr<DecoderStream> _video;
    WeakPtr<Thread> _thread;
    Decoder(AVFormatContext* format, DecoderStream* audio, DecoderStream* video, Thread* thread);
public:
    ~Decoder();

    RefPtr<Stream<Frame>> audio();
    RefPtr<Stream<Frame>> video();

    AVStream* audioStream();
    AVStream* videoStream();

    bool available() const;
    bool decode(AVPacket* packet, RefPtr<Frame> frame);
    void seek(double time, std::function<void()> callback = std::function<void()>());

    static Decoder* from(const std::string& url, Thread* thread, AVPixelFormat pixel, AVSampleFormat sample);
};

OMP_FFMPEG_NAMESPACE_END
