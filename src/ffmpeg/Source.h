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

class MovieSource : public Stream<Packet> {
    friend class FileSourceStream;
    AVFormatContext* _context = nullptr;
    RefPtr<Packet> _packet;
    RefPtr<StreamController<Packet>> _controller;
    RefPtr<Thread> _thread;
public:
    MovieSource(Thread* thread);
    bool open(const std::string& filename);
    void close();

    RefPtr<Thread> thread() const;
    AVFormatContext* context() const;
    AVStream* audio() const;
    AVStream* video() const;

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

    bool available() const;

    static MovieSourceStream* from(AVStream* stream, Thread* thread);
    static MovieSourceStream* audio(MovieSource* source);
    static MovieSourceStream* video(MovieSource* source);
};

class FileTarget : public Consumer<Packet> {
    friend class FileTargetStream;
    friend class StreamEncoder;
    AVFormatContext* _context;
    FileTarget(AVFormatContext* context);
public:
    ~FileTarget();

    void add(RefPtr<Packet> packet) override;
    void addError() override;
    void close() override;

    bool openFile(const std::string& filename);
    bool closeFile();
    bool writeHeader(AVDictionary* options = nullptr);
    bool writeTrailer();

    static FileTarget* from(const char* format, const char* filename = nullptr);
};

class FileTargetStream : public Transformer<Frame, Packet> {
    RefPtr<FileTarget> _owner;
    AVStream* _stream;
    AVCodecContext* _context;
    FileTargetStream(FileTarget* owner, AVStream* stream);
public:
    void add(RefPtr<Frame> frame) override;
    void addError() override;
    void close() override;

    bool open(AVSampleFormat format, int32_t bit_rate, int32_t sample_rate, const AVChannelLayout& ch_layout, AVDictionary* options = nullptr);
    bool open(AVPixelFormat format, int32_t bit_rate, int32_t frame_rate, int32_t width, int32_t height, int32_t gop_size, int32_t max_b_frames, AVDictionary* options = nullptr);

    AVCodecContext* context() const;

    static FileTargetStream* from(FileTarget* target, const AVCodec* codec);
    static FileTargetStream* audio(FileTarget* target);
    static FileTargetStream* video(FileTarget* target);
};

OMP_FFMPEG_NAMESPACE_END
