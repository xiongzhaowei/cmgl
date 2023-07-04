//
// Created by 熊朝伟 on 2023-06-01.
//

#include "defines.h"

OMP_FFMPEG_USING_NAMESPACE

Decoder::Decoder(AVFormatContext* format, DecoderStream* audio, DecoderStream* video, Thread* thread) : _format(format), _audio(audio), _video(video), _thread(thread) {
}

Decoder::~Decoder() {
    if (_format) avformat_close_input(&_format);
}

Decoder* Decoder::from(const std::string& url, Thread* thread) {
    AVFormatContext* context = nullptr;
    RefPtr<DecoderStream> audio = nullptr;
    RefPtr<DecoderStream> video = nullptr;
    if (avformat_open_input(&context, url.c_str(), nullptr, nullptr) >= 0) {
        if (avformat_find_stream_info(context, nullptr) >= 0) {
            for (uint32_t i = 0; i < context->nb_streams; i++) {
                AVStream* stream = context->streams[i];
                AVMediaType codec_type = stream->codecpar->codec_type;
                if (codec_type == AVMEDIA_TYPE_AUDIO && nullptr == audio) {
                    audio = DecoderStream::from(stream, thread);
                } else if (codec_type == AVMEDIA_TYPE_VIDEO && nullptr == video) {
                    video = DecoderStream::from(stream, thread);
                }
            }
            if (audio || video) {
                return new Decoder(context, audio, video, thread);
            }
        }
        avformat_close_input(&context);
    }
    return nullptr;
}

RefPtr<DecoderStream> Decoder::audio() {
    return _audio;
}

RefPtr<DecoderStream> Decoder::video() {
    return _video;
}

void Decoder::seek(double time, std::function<void()> callback) {
    if (_thread == nullptr) return;

    _thread->runOnThread([this, time, callback]() {
        if (!_format) return;

        int64_t ts = std::clamp<int64_t>(time * AV_TIME_BASE, 0, _format->duration);
        if (avformat_seek_file(_format, -1, 0, ts, _format->duration, 0) >= 0) {
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

bool Decoder::available() const {
    return _audio->available() && _video->available();
}

bool Decoder::decode(AVPacket* packet, RefPtr<Frame> frame) {
    if (av_read_frame(_format, packet) < 0) return false;

    if (_audio && _audio->match(_format, packet)) {
        _audio->decode(packet, frame);
    } else if (_video && _video->match(_format, packet)) {
        _video->decode(packet, frame);
    }
    av_packet_unref(packet);
    return true;
}

DecoderStream* DecoderStream::from(AVStream* stream, Thread* thread) {
    const AVCodec* codec = avcodec_find_decoder(stream->codecpar->codec_id);
    if (!codec) return nullptr;

    AVCodecContext* context = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(context, stream->codecpar);
    if (avcodec_open2(context, codec, NULL) < 0) return nullptr;

    return new DecoderStream(stream, context, thread);
}

DecoderStream::DecoderStream(AVStream* stream, AVCodecContext* context, Thread* thread) : _stream(stream), _context(context), _controller(StreamController<Frame>::sync()) {

}

DecoderStream::~DecoderStream() {
    if (_context) avcodec_free_context(&_context);
}

RefPtr<StreamSubscription> DecoderStream::listen(RefPtr<StreamConsumer<Frame>> consumer) {
    return _controller->stream()->listen(consumer);
}

AVStream* DecoderStream::stream() const {
    return _stream;
}

bool DecoderStream::available() const {
    return _controller->available();
}

bool DecoderStream::match(AVFormatContext* format, AVPacket* packet) const {
    return format->streams[packet->stream_index] == _stream;
}

bool DecoderStream::decode(AVPacket* packet, RefPtr<Frame> frame) {
    if (avcodec_send_packet(_context, packet) < 0) return false;
    if (avcodec_receive_frame(_context, frame->frame()) < 0) return false;

    _controller->add(frame);
    return true;
}
