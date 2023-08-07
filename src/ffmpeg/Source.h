//
// Created by 熊朝伟 on 2023-06-13.
//

#pragma once

OMP_FFMPEG_NAMESPACE_BEGIN

class MovieSourceStream;

class MovieFile : public Object {
public:
    virtual size_t bufferSize() const = 0;
    virtual int read(uint8_t *buf, int buf_size) = 0;
    virtual int write(const uint8_t* buf, int buf_size) = 0;
    virtual int64_t seek(int64_t offset, int whence) = 0;
};

class MovieSource : public Stream<Packet> {
    RefPtr<StreamController<Packet>> _controller;
    RefPtr<Packet> _packet;
    RefPtr<MovieFile> _file;
    AVFormatContext* _context = nullptr;
    std::mutex _mutex;
    mutable std::function<bool(bool)> _available;
public:
    MovieSource();
    MovieSource(const std::function<bool(bool)>& available);
    bool open(RefPtr<MovieFile> file);
    bool open(const std::string& filename);
    void close();

    AVFormatContext* context() const;
    AVStream* stream(AVMediaType codecType) const;

    bool available() const;
    bool read();
    bool seek(double time);
    void skip(const std::function<bool(AVPacket*)>& skipWhere);
    void flush();

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
    void addError(RefPtr<Error> error) override;
    void close() override;
    bool available() const override;
    bool flush() override;
    void clear() override;

    AVStream* stream() const;
    AVCodecContext* context() const;
    RefPtr<Frame> decode();

    static RefPtr<MovieDecoder> from(RefPtr<StreamConsumer<Frame>> output, AVStream* stream, AVDictionary* options = nullptr);
};

class MovieSourceStream : public Stream<Frame> {
    RefPtr<StreamSubscription> _subscription;
    RefPtr<StreamController<Frame>> _controller;
    RefPtr<MovieDecoder> _decoder;
    MovieSourceStream(RefPtr<StreamController<Frame>> controller, RefPtr<MovieDecoder> decoder, RefPtr<StreamSubscription> subscription);
public:
    ~MovieSourceStream();

    RefPtr<StreamSubscription> listen(RefPtr<StreamConsumer<Frame>> consumer) override;

    AVStream* stream() const;
    AVCodecContext* context() const;
    bool available() const;
    RefPtr<Frame> decode();

    RefPtr<Stream<Frame>> convert(AVSampleFormat sample_fmt, AVChannelLayout ch_layout, int32_t sample_rate);
    RefPtr<Stream<Frame>> convert(AVSampleFormat format);
    RefPtr<Stream<Frame>> convert(AVPixelFormat format);

    static RefPtr<MovieSourceStream> from(RefPtr<Stream<Packet>> source, AVStream* stream, AVDictionary* options = nullptr);
    static RefPtr<MovieSourceStream> audio(RefPtr<MovieSource> source, AVDictionary* options = nullptr);
    static RefPtr<MovieSourceStream> video(RefPtr<MovieSource> source, AVDictionary* options = nullptr);
};

class MovieCachedPacketConsumer : public StreamConsumer<Packet> {
    mutable std::mutex _mutex;
    mutable std::function<bool(size_t)> _available;
    int32_t _index;
    std::list<RefPtr<Packet>> _list;
    RefPtr<StreamConsumer<Packet>> _output;
public:
    typedef Packet Target;

    MovieCachedPacketConsumer(RefPtr<StreamConsumer<Packet>> output, const std::function<bool(size_t)>& available, int32_t index);

    void add(RefPtr<Packet> packet) override;
    void addError(RefPtr<Error> error) override;
    void close() override;
    bool available() const override;

    bool empty();
    bool flush();
    void clear();
};

class MovieBufferedConsumer : public StreamConsumer<Frame> {
    uint32_t _maxCount;
    bool _isEndOfFile;
    mutable std::mutex _mutex;
    std::list<RefPtr<Frame>> _list;
    RefPtr<Converter<Frame>> _converter;
    RefPtr<StreamSubscription> _subscription;
public:
    MovieBufferedConsumer(RefPtr<Stream<Frame>> stream, uint32_t maxCount);

    void add(RefPtr<Frame> frame) override;
    void addError(RefPtr<Error> error) override;
    void close() override;
    bool available() const override;
    bool flush() override;

    bool eof() const;
    size_t size() const;
    int64_t timestamp() const;

    void push(RefPtr<Frame> frame);
    RefPtr<Frame> pop();
    RefPtr<Frame> pop(int64_t timestamp);
    bool empty();
    void clear();
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
    void addError(RefPtr<Error> error) override;
    void close() override;
    bool available() const override;
    bool flush() override;
    void clear() override;

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
    void addError(RefPtr<Error> error) override;
    void close() override;
    bool available() const override;
    bool flush() override;
    void clear() override;

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
    void addError(RefPtr<Error> error) override {}
    void close() override {}
    bool available() const override { return true; }
    bool flush() override { return true; }
    void clear() override {}
};

OMP_FFMPEG_NAMESPACE_END
