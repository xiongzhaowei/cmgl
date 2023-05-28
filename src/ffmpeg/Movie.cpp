//
// Created by 熊朝伟 on 2023-05-27.
//

#include "defines.h"
#include "Converter.h"
#include "Renderer.h"

OMP_FFMPEG_USING_NAMESPACE

std::unique_lock<std::mutex> Mutex::lock() {
    return std::unique_lock<std::mutex>(_mutex);
}

void Mutex::wait(std::function<bool()> pred) {
    std::unique_lock<std::mutex> lock(_mutex);
    _waiter.wait(lock, pred);
}

void Mutex::notify() {
    _waiter.notify_one();
}

Movie::Movie(AVFormatContext* format, Stream* audio, Stream* video, Mutex* mutex) : _format(format), _audio(audio), _video(video), _mutex(mutex) {
}

Movie::~Movie() {
    if (_format) avformat_close_input(&_format);
}

Movie* Movie::from(const std::string& url, std::function<void(AVFrame*)> callback) {
    AVFormatContext* format = nullptr;
    Stream* audio = nullptr;
    Stream* video = nullptr;
    RefPtr<Mutex> mutex = new Mutex;
    if (avformat_open_input(&format, url.c_str(), nullptr, nullptr) >= 0) {
        if (avformat_find_stream_info(format, nullptr) >= 0) {
            for (uint32_t i = 0; i < format->nb_streams; i++) {
                AVStream* stream = format->streams[i];
                AVMediaType codec_type = stream->codecpar->codec_type;
                if (codec_type == AVMEDIA_TYPE_AUDIO && nullptr == audio) {
                    audio = Stream::from(stream, AudioRenderer::from(stream, mutex));
                } else if (codec_type == AVMEDIA_TYPE_VIDEO && nullptr == video) {
                    video = Stream::from(stream, VideoRenderer::from(stream, callback));
                }
            }
            if (audio || video) {
                return new Movie(format, audio, video, mutex);
            }
        }
        avformat_close_input(&format);
    }
    return nullptr;
}

void Movie::play(bool state) {
    runOnThread([this]() {
        _audio->consumer()->play(true);
        _video->consumer()->play(true);
    });
}

void Movie::seek(double time, std::function<void()> callback) {
    runOnThread([this, time, callback]() {
        if (_format) {
            int64_t ts = std::clamp<int64_t>(time * AV_TIME_BASE, 0, _format->duration);

            if (avformat_seek_file(_format, -1, 0, ts, _format->duration, 0) >= 0) {
                _audio->consumer()->clear();
                _video->consumer()->clear();
                runOnThread(callback);
            }
        }
    });
}

void Movie::start() {
    assert(_thread == nullptr);

    RefPtr<Movie> self = this;
    _thread = std::make_unique<std::thread>([self]() { self->run(); });
}

void Movie::run() {
    static constexpr double kMaxCacheDuration = 0.1;

    if (!_audio && !_video) return;
    if (_audio && _video) {
        _audio->consumer()->attach(_video->consumer());
    } else if (_video) {
        // TODO: 缺少音频时使用视频同步
    }

    AVPacket* packet = av_packet_alloc();
    AVFrame* frame = av_frame_alloc();

    bool isRunning = true;
    while (isRunning) {
        while (_audio->consumer()->duration() < kMaxCacheDuration) {
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
        {
            std::unique_lock<std::mutex> lock = _mutex->lock();
            list = std::move(_taskQueue);
        }
        for (auto& task : list) {
            task();
        }
        list.clear();
        _mutex->wait([this]() -> bool {
            if ((!_audio || _audio->consumer()->duration() < kMaxCacheDuration) && (!_video || _video->consumer()->duration() < kMaxCacheDuration)) return true;
            if (!_taskQueue.empty()) return true;
            return false;
        });
    }
    av_frame_free(&frame);
    av_packet_free(&packet);
}

void Movie::runOnThread(std::function<void()> task) {
    if (_thread && std::this_thread::get_id() == _thread->get_id()) {
        _taskQueue.push_back(task);
    } else {
        std::unique_lock<std::mutex> lock = _mutex->lock();
        _taskQueue.push_back(task);
    }
    _mutex->notify();
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

void Movie::Stream::process(AVPacket* packet, AVFrame* frame) {
    if (decode(packet, frame)) {
        assert(_consumer != nullptr);
        if (_consumer) _consumer->push(frame);
    }
}

Movie::Stream* Movie::Stream::from(AVStream* stream) {
    const AVCodec* codec = avcodec_find_decoder(stream->codecpar->codec_id);
    if (!codec) return nullptr;

    AVCodecContext* context = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(context, stream->codecpar);
    if (avcodec_open2(context, codec, NULL) < 0) return nullptr;

    return new Stream(stream, context);
}

Movie::Stream* Movie::Stream::from(AVStream* stream, Movie::Consumer* consumer) {
    Stream* result = from(stream);
    if (result) result->_consumer = consumer;
    return result;
}
