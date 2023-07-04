﻿//
// Created by 熊朝伟 on 2023-06-13.
//

#pragma once

OMP_FFMPEG_NAMESPACE_BEGIN

class MovieSourceStream;

class MovieSource : public Stream<Packet> {
    RefPtr<StreamController<Packet>> _controller;
    RefPtr<Packet> _packet;
    AVFormatContext* _context = nullptr;
public:
    MovieSource();
    bool open(const std::string& filename);
    void close();

    AVFormatContext* context() const;
    RefPtr<MovieSourceStream> stream(AVMediaType codecType);

    bool available() const;
    bool read();

    RefPtr<StreamSubscription> listen(RefPtr<StreamConsumer<Packet>> consumer) override;
};

class MovieDecoder : public StreamConsumer<Packet> {
    RefPtr<StreamConsumer<Frame>> _output;
    RefPtr<Frame> _frame;
    AVStream* _stream;
    AVCodecContext* _context;
    MovieDecoder(RefPtr<StreamConsumer<Frame>> output, AVStream* stream, AVCodecContext* context);
public:
    ~MovieDecoder();

    typedef Frame Target;

    void add(RefPtr<Packet> packet) override;
    void addError() override;
    void close() override;
    bool available() const override;

    AVStream* stream() const;
    AVCodecContext* context() const;

    static RefPtr<MovieDecoder> from(RefPtr<StreamConsumer<Frame>> output, AVStream* stream);
};

class MovieSourceStream : public Stream<Frame> {
    RefPtr<StreamController<Frame>> _controller;
    WeakPtr<MovieDecoder> _decoder;
    MovieSourceStream(RefPtr<StreamController<Frame>> controller, RefPtr<MovieDecoder> decoder);
public:
    RefPtr<StreamSubscription> listen(RefPtr<StreamConsumer<Frame>> consumer) override;

    AVStream* stream() const;
    AVCodecContext* context() const;
    bool available() const;

    RefPtr<Stream<Frame>> convert(AVSampleFormat sample_fmt, AVChannelLayout ch_layout, int32_t sample_rate);
    RefPtr<Stream<Frame>> convert(AVPixelFormat format);

    static RefPtr<MovieSourceStream> from(RefPtr<MovieSource> source, AVStream* stream);
};

class MovieEncoder : public StreamConsumer<Frame> {
    RefPtr<StreamConsumer<Packet>> _output;
    RefPtr<Packet> _packet;
    AVStream* _stream;
    AVCodecContext* _context;
    MovieEncoder(RefPtr<StreamConsumer<Packet>> output, AVStream* stream, AVCodecContext* context);
public:
    ~MovieEncoder();

    typedef Packet Target;

    void add(RefPtr<Frame> frame) override;
    void addError() override;
    void close() override;
    bool available() const override;

    AVStream* stream() const;
    AVCodecContext* context() const;

    static RefPtr<MovieEncoder> audio(RefPtr<StreamConsumer<Packet>> output, const AVCodec* codec, AVStream* stream, int32_t bit_rate, AVSampleFormat format, int32_t sample_rate, AVChannelLayout ch_layout, AVDictionary* options = nullptr);
    static RefPtr<MovieEncoder> video(RefPtr<StreamConsumer<Packet>> output, const AVCodec* codec, AVStream* stream, int32_t bit_rate, AVPixelFormat format, int32_t frame_rate, int32_t width, int32_t height, int32_t gop_size, int32_t max_b_frames, AVDictionary* options = nullptr);
};

class MovieTarget : public StreamConsumer<Packet> {
    AVFormatContext* _context;
    MovieTarget(AVFormatContext* context);
public:
    ~MovieTarget();

    void add(RefPtr<Packet> packet) override;
    void addError() override;
    void close() override;
    bool available() const override;

    AVFormatContext* context() const;
    RefPtr<MovieEncoder> encoder(int32_t bit_rate, AVSampleFormat format, int32_t sample_rate, AVChannelLayout ch_layout, AVDictionary* options = nullptr);
    RefPtr<MovieEncoder> encoder(int32_t bit_rate, AVPixelFormat format, int32_t frame_rate, int32_t width, int32_t height, int32_t gop_size, int32_t max_b_frames, AVDictionary* options = nullptr);

    bool openFile(const std::string& filename);
    bool closeFile();
    bool writeHeader(AVDictionary* options = nullptr);
    bool writeTrailer();

    static MovieTarget* from(const char* format, const char* filename = nullptr);
};

class EmptyTarget : public StreamConsumer<Frame> {
public:
    void add(RefPtr<Frame> frame) override {}
    void addError() override {}
    void close() override {}
    bool available() const override { return true; }
};

OMP_FFMPEG_NAMESPACE_END
