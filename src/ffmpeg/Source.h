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

class FileSource : public Stream<Packet> {
    friend class FileSourceStream;
    AVFormatContext* _context = nullptr;
    RefPtr<Packet> _packet;
    RefPtr<StreamController<Packet>> _controller;
    RefPtr<Thread> _thread;
public:
    FileSource(Thread* thread);
    bool open(const std::string& filename);
    void close();

    bool available() const;
    bool read();

    RefPtr<StreamSubscription<Packet>> listen(RefPtr<Consumer<Packet>> consumer) override;
};

class FileSourceStream : public Consumer<Packet>, public Stream<Frame> {
    friend class FileTargetStream;
    AVStream* _stream;
    AVCodecContext* _context;
    RefPtr<Frame> _frame = Frame::alloc();
    RefPtr<StreamController<Frame>> _controller;
    FileSourceStream(AVStream* stream, AVCodecContext* context, Thread* thread);
public:
    ~FileSourceStream();

    RefPtr<StreamSubscription<Frame>> listen(RefPtr<Consumer<Frame>> consumer) override;

    void add(RefPtr<Packet> packet) override;
    void addError() override;
    void close() override;

    AVStream* stream() const;
    bool available() const;
    bool match(AVFormatContext* format, AVPacket* packet) const;
    bool decode(AVPacket* packet, RefPtr<Frame> frame);

    static FileSourceStream* from(AVStream* stream, Thread* thread);
    static FileSourceStream* audio(FileSource* source);
    static FileSourceStream* video(FileSource* source);
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

class FileTargetStream : public Consumer<Frame> {
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

    static FileTargetStream* from(FileTarget* target, const AVCodec* codec);
    static FileTargetStream* audio(FileTarget* target);
    static FileTargetStream* video(FileTarget* target);
};

OMP_FFMPEG_NAMESPACE_END
