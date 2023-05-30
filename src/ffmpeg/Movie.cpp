//
// Created by 熊朝伟 on 2023-05-27.
//

#include "defines.h"
#include "Converter.h"
#include "Renderer.h"

OMP_FFMPEG_USING_NAMESPACE

Movie::Movie(AVFormatContext* format, Stream* audio, Stream* video, Consumer* audioConsumer, Consumer* videoConsumer, RefPtr<WaitableEvent> event) : _format(format), _audio(audio), _video(video), _audioConsumer(audioConsumer), _videoConsumer(videoConsumer), _event(event) {
}

Movie::~Movie() {
    if (_format) avformat_close_input(&_format);
}

Movie* Movie::from(const std::string& url, RefPtr<render::VideoSource> source, AVPixelFormat format, std::function<void()> callback) {
    AVFormatContext* context = nullptr;
    RefPtr<Stream> audio = nullptr;
    RefPtr<Stream> video = nullptr;
    RefPtr<Consumer> audioConsumer = nullptr;
    RefPtr<Consumer> videoConsumer = nullptr;
    RefPtr<WaitableEvent> event = new WaitableEvent;
    if (avformat_open_input(&context, url.c_str(), nullptr, nullptr) >= 0) {
        if (avformat_find_stream_info(context, nullptr) >= 0) {
            for (uint32_t i = 0; i < context->nb_streams; i++) {
                AVStream* stream = context->streams[i];
                AVMediaType codec_type = stream->codecpar->codec_type;
                if (codec_type == AVMEDIA_TYPE_AUDIO && nullptr == audio) {
                    AudioConverter* converter = AudioConverter::create(stream, AV_SAMPLE_FMT_FLT);
                    double timebase = av_q2d(stream->time_base) * stream->codecpar->frame_size;
                    audio = Stream::from(stream, timebase, event, converter);
                    audioConsumer = AudioRenderer::from(audio, AUDIO_F32);
                } else if (codec_type == AVMEDIA_TYPE_VIDEO && nullptr == video) {
                    VideoConverter* converter = nullptr;
                    if (stream->codecpar->format != format) {
                        converter = VideoConverter::create(stream, format);
                    }
                    double timebase = av_q2d(stream->time_base);
                    video = Stream::from(stream, timebase, event, converter);
                    videoConsumer = VideoRenderer::from(video, source, callback);
                }
            }
            if (audio || video) {
                return new Movie(context, audio, video, audioConsumer, videoConsumer, event);
            }
        }
        avformat_close_input(&context);
    }
    return nullptr;
}

void Movie::play(bool state) {
    runOnThread([this]() {
        _audioConsumer->play(true);
        _videoConsumer->play(true);
    });
}

void Movie::seek(double time, std::function<void()> callback) {
    runOnThread([this, time, callback]() {
        if (!_format) return;

        int64_t ts = std::clamp<int64_t>(time * AV_TIME_BASE, 0, _format->duration);

        if (avformat_seek_file(_format, -1, 0, ts, _format->duration, 0) >= 0) {
            _audio->clear();
            _video->clear();
            runOnThread(callback);
        }
    });
}

void Movie::start() {
    assert(_thread == nullptr);

    RefPtr<Movie> self = this;
    _thread = std::make_unique<std::thread>([self]() { self->run(); });
}

void Movie::run() {
    if (!_audio && !_video) return;

    if (_audioConsumer && _videoConsumer) {
        _audioConsumer->attach(_videoConsumer);
    }

    AVPacket* packet = av_packet_alloc();
    RefPtr<Frame> frame = Frame::alloc();

    bool isRunning = true;
    while (isRunning) {
        while (isNeedDecode()) {
            if (av_read_frame(_format, packet) >= 0) {
                AVStream* stream = _format->streams[packet->stream_index];
                if (_audio && _audio->stream() == stream) {
                    _audio->process(packet, frame);
                } else if (_video && _video->stream() == stream) {
                    _video->process(packet, frame);
                }
                av_packet_unref(packet);
            } else {
                isRunning = false;
            }
        }

        std::list<std::function<void()>> list;
        _taskQueue.swap(list);
        for (auto& task : list) {
            task();
        }
        list.clear();
        _event->wait([this]() -> bool {
            return isNeedDecode() || !_taskQueue.empty();
        });
    }
    av_packet_free(&packet);
}

bool Movie::isNeedDecode() {
    static constexpr double kMaxCacheDuration = 0.1;

    return _audio->duration() < kMaxCacheDuration && _video->duration() < kMaxCacheDuration;
}

void Movie::runOnThread(std::function<void()> task) {
    _taskQueue.push(task);
    _event->signal();
}

Movie::Stream::Stream(AVStream* stream, AVCodecContext* context) : _stream(stream), _context(context) {
}

Movie::Stream::~Stream() {
    if (_context) avcodec_free_context(&_context);
}

bool Movie::Stream::decode(AVPacket* packet, AVFrame* frame) {
    if (avcodec_send_packet(_context, packet) < 0) return false;
    if (avcodec_receive_frame(_context, frame) < 0) return false;
    return true;
}

void Movie::Stream::process(AVPacket* packet, RefPtr<Frame> frame) {
    if (decode(packet, frame->frame())) {
        RefPtr<Frame> output;
        if (_converter) {
            output = _converter->convert(frame);
        } else {
            output = Frame::alloc();
            output->swap(frame);
        }
        _frameList.push(output);
    }
}

double Movie::Stream::duration() {
    return _frameList.size() * _timebase;
}

void Movie::Stream::clear() {
    _frameList.clear();
}

RefPtr<Frame> Movie::Stream::pop() {
    RefPtr<Frame> frame = _frameList.pop();
    _event->signal();
    return frame;
}

RefPtr<Frame> Movie::Stream::pop(int64_t pts) {
    if (_frameList.empty()) return nullptr;

    int64_t target_pts = pts;
    RefPtr<Frame> target_frame = nullptr;
    _frameList.foreach([&](RefPtr<Frame>& frame) {
        double current = frame->frame()->best_effort_timestamp;
        if (target_pts > current) {
            target_pts = current;
            target_frame = frame;
        }
    });

    _frameList.remove(target_frame);
    return target_frame;
}

Movie::Stream* Movie::Stream::from(AVStream* stream) {
    const AVCodec* codec = avcodec_find_decoder(stream->codecpar->codec_id);
    if (!codec) return nullptr;

    AVCodecContext* context = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(context, stream->codecpar);
    if (avcodec_open2(context, codec, NULL) < 0) return nullptr;

    return new Stream(stream, context);
}

Movie::Stream* Movie::Stream::from(AVStream* stream, double timebase, RefPtr<WaitableEvent> event, Movie::Converter* converter) {
    Stream* result = from(stream);
    if (result) {
        result->_timebase = timebase;
        result->_converter = converter;
        result->_event = event;
    }
    return result;
}
