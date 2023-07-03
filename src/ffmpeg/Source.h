//
// Created by 熊朝伟 on 2023-06-13.
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

class MovieSourceStream;

class MovieSource : public Stream<Packet> {
    AVFormatContext* _context = nullptr;
    RefPtr<Packet> _packet;
    RefPtr<StreamController<Packet>> _controller;
public:
    MovieSource();
    bool open(const std::string& filename);
    void close();

    AVFormatContext* context() const;
    AVStream* audio() const;
    AVStream* video() const;
    RefPtr<MovieSourceStream> stream(AVMediaType codecType);

    bool available() const;
    bool read();

    RefPtr<StreamSubscription> listen(RefPtr<Consumer<Packet>> consumer) override;
};

class MovieDecoder : public Transformer<Packet, Frame> {
    RefPtr<Consumer<Frame>> _output;
    RefPtr<Frame> _frame;
    AVStream* _stream;
    AVCodecContext* _context;
public:
    MovieDecoder(RefPtr<Consumer<Frame>> output, AVStream* stream, AVCodecContext* context);
    ~MovieDecoder();

    void add(RefPtr<Packet> packet) override;
    void addError() override;
    void close() override;

    AVStream* stream() const;
    AVCodecContext* context() const;

    bool available() const;

    static RefPtr<MovieDecoder> from(RefPtr<Consumer<Frame>> output, AVStream* stream);
};

class MovieSourceStream : public Stream<Frame> {
    RefPtr<StreamController<Frame>> _controller;
    RefPtr<MovieDecoder> _decoder;
public:
    MovieSourceStream(RefPtr<StreamController<Frame>> controller, RefPtr<MovieDecoder> decoder);

    RefPtr<StreamSubscription> listen(RefPtr<Consumer<Frame>> consumer) override;

    AVStream* stream() const;
    AVCodecContext* context() const;
    RefPtr<MovieDecoder> decoder() const;
    RefPtr<Stream<Frame>> convert(AVSampleFormat sample_fmt, AVChannelLayout ch_layout, int32_t sample_rate);
    RefPtr<Stream<Frame>> convert(AVPixelFormat format);

    bool available() const;

    static MovieSourceStream* from(AVStream* stream);
};

class MovieEncoder : public Transformer<Frame, Packet> {
    RefPtr<Consumer<Packet>> _output;
    RefPtr<Packet> _packet;
    AVStream* _stream;
    AVCodecContext* _context;
    MovieEncoder(RefPtr<Consumer<Packet>> output, AVStream* stream, AVCodecContext* context);
public:
    ~MovieEncoder();

    void add(RefPtr<Frame> frame) override;
    void addError() override;
    void close() override;

    AVStream* stream() const;
    AVCodecContext* context() const;

    static RefPtr<MovieEncoder> audio(RefPtr<Consumer<Packet>> output, const AVCodec* codec, AVStream* stream, int32_t bit_rate, AVSampleFormat format, int32_t sample_rate, AVChannelLayout ch_layout, AVDictionary* options = nullptr);
    static RefPtr<MovieEncoder> video(RefPtr<Consumer<Packet>> output, const AVCodec* codec, AVStream* stream, int32_t bit_rate, AVPixelFormat format, int32_t frame_rate, int32_t width, int32_t height, int32_t gop_size, int32_t max_b_frames, AVDictionary* options = nullptr);
};

class MovieTarget : public Consumer<Packet> {
    AVFormatContext* _context;
    MovieTarget(AVFormatContext* context);
public:
    ~MovieTarget();

    void add(RefPtr<Packet> packet) override;
    void addError() override;
    void close() override;

    AVFormatContext* context() const;
    RefPtr<MovieEncoder> audio(int32_t bit_rate, AVSampleFormat format, int32_t sample_rate, AVChannelLayout ch_layout, AVDictionary* options = nullptr);
    RefPtr<MovieEncoder> video(int32_t bit_rate, AVPixelFormat format, int32_t frame_rate, int32_t width, int32_t height, int32_t gop_size, int32_t max_b_frames, AVDictionary* options = nullptr);

    bool openFile(const std::string& filename);
    bool closeFile();
    bool writeHeader(AVDictionary* options = nullptr);
    bool writeTrailer();

    static MovieTarget* from(const char* format, const char* filename = nullptr);
};

OMP_FFMPEG_NAMESPACE_END
