//
// Created by 熊朝伟 on 2023-05-27.
//

#include "defines.h"
#include "Converter.h"
#include "Renderer.h"

OMP_FFMPEG_USING_NAMESPACE

Movie::Movie(AVFormatContext* format, Stream* audio, Stream* video, MovieThread* thread) : _format(format), _audio(audio), _video(video), _thread(thread) {
}

Movie::~Movie() {
    if (_format) avformat_close_input(&_format);
}

Movie* Movie::from(const std::string& url, MovieThread* thread, AVPixelFormat pixel, AVSampleFormat sample) {
    AVFormatContext* context = nullptr;
    RefPtr<Stream> audio = nullptr;
    RefPtr<Stream> video = nullptr;
    RefPtr<WaitableEvent> event = new WaitableEvent;
    if (avformat_open_input(&context, url.c_str(), nullptr, nullptr) >= 0) {
        if (avformat_find_stream_info(context, nullptr) >= 0) {
            for (uint32_t i = 0; i < context->nb_streams; i++) {
                AVStream* stream = context->streams[i];
                AVMediaType codec_type = stream->codecpar->codec_type;
                if (codec_type == AVMEDIA_TYPE_AUDIO && nullptr == audio) {
                    AudioConverter* converter = AudioConverter::create(stream, sample);
                    double timebase = av_q2d(stream->time_base) * stream->codecpar->frame_size;
                    audio = Stream::from(stream, timebase, thread->_event, converter);
                } else if (codec_type == AVMEDIA_TYPE_VIDEO && nullptr == video) {
                    VideoConverter* converter = nullptr;
                    if (stream->codecpar->format != pixel) {
                        converter = VideoConverter::create(stream, pixel);
                    }
                    double timebase = av_q2d(stream->time_base);
                    video = Stream::from(stream, timebase, thread->_event, converter);
                }
            }
            if (audio || video) {
                return new Movie(context, audio, video, thread);
            }
        }
        avformat_close_input(&context);
    }
    return nullptr;
}

RefPtr<Movie::Stream> Movie::audio() {
    return _audio;
}

RefPtr<Movie::Stream> Movie::video() {
    return _video;
}

void Movie::seek(double time, std::function<void()> callback) {
    if (_thread == nullptr) return;

    _thread->runOnThread([this, time, callback]() {
        if (!_format) return;

        int64_t ts = std::clamp<int64_t>(time * AV_TIME_BASE, 0, _format->duration);
        if (avformat_seek_file(_format, -1, 0, ts, _format->duration, 0) >= 0) {
            _audio->clear();
            _video->clear();
            if (callback) {
                if (_thread != nullptr) {
                    _thread->runOnThread(callback);
                } else {
                    callback();
                }
            }
        }
    });
}

bool Movie::detect() {
    static constexpr double kMaxCacheDuration = 0.1;

    return _audio->duration() < kMaxCacheDuration && _video->duration() < kMaxCacheDuration;
}

bool Movie::decode(AVPacket* packet, RefPtr<Frame> frame) {
    if (av_read_frame(_format, packet) < 0) return false;
    
    AVStream* stream = _format->streams[packet->stream_index];
    if (_audio && _audio->stream() == stream) {
        _audio->process(packet, frame);
    } else if (_video && _video->stream() == stream) {
        _video->process(packet, frame);
    }
    av_packet_unref(packet);
    return true;
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
    RefPtr<Frame> frame = _frameList.pop(pts);
    _event->signal();
    return frame;
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
