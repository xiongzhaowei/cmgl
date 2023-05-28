//
//  URLVideoSource.cpp
//  omrender
//
//  Created by 熊朝伟 on 2020/4/16.
//

#include "defines.h"
#include "URLVideoSource.h"

OMP_RENDER_USING_NAMESPACE

class URLVideoSource::VideoConverter {
    int32_t _width = 0;
    int32_t _height = 0;
    AVPixelFormat _format = AV_PIX_FMT_NONE;
    SwsContext *_context = nullptr;
public:

    static AVPixelFormat formatConvert(const Type &type) {
        switch (type) {
        case RGBA:
            return AV_PIX_FMT_RGBA;
        case RGB24:
            return AV_PIX_FMT_RGB24;
        case YUV420P:
            return AV_PIX_FMT_YUV420P;
        case YUV420SP:
            return AV_PIX_FMT_NV12;
        default:
            return AV_PIX_FMT_NONE;
        }
    }

    static bool equals(AVStream *stream, const Type &type) {
        return (AVPixelFormat)stream->codecpar->format == formatConvert(type);
    }

    bool open(AVStream *stream, const Type &type) {
        if (!stream) return false;
        if (!stream->codecpar) return false;
        if (stream->codecpar->codec_type != AVMEDIA_TYPE_VIDEO) return false;

        AVCodecParameters *codecpar = stream->codecpar;
        _width = codecpar->width;
        _height = codecpar->height;
        _format = formatConvert(type);
        _context = sws_getContext(
            _width, _height, (AVPixelFormat)codecpar->format,
            _width, _height, _format,
            SWS_FAST_BILINEAR, NULL, NULL, NULL
        );

        return _context != nullptr;
    }

    void close() {
        if (!_context) {
            sws_freeContext(_context);
            _context = nullptr;
            _width = 0;
            _height = 0;
        }
    }

    static AVFrame *alloc(AVPixelFormat format, int32_t width, int32_t height) {
        AVFrame *frame = av_frame_alloc();
        if (!frame) return nullptr;

        frame->format = format;
        frame->width = width;
        frame->height = height;

        if (av_frame_get_buffer(frame, 1) < 0) {
            av_frame_free(&frame);
            return nullptr;
        }

        if (av_frame_make_writable(frame) < 0) {
            av_frame_free(&frame);
            return nullptr;
        }

        return frame;
    }

    static void free(AVFrame *&frame) {
        av_frame_free(&frame);
    }

    AVFrame *alloc() { return alloc(_format, _width, _height); }

    bool convert(AVFrame *input, AVFrame *&output) {
        if (nullptr == output) {
            output = alloc(_format, _width, _height);
        }
        if (nullptr == output) return false;

        output->pts = input->pts;
        return sws_scale(_context, (const uint8_t* const*)input->data, input->linesize, 0, _height, output->data, output->linesize) > 0;
    }

    bool isValid() const { return _context != nullptr; }
};

class URLVideoSource::Decoder {
    AVFormatContext *_format = nullptr;
    AVStream *_videoStream = nullptr;
    AVStream *_audioStream = nullptr;
    AVCodecContext *_videoContext = nullptr;
    AVCodecContext *_audioContext = nullptr;
    VideoConverter _videoConvert;
	AVFrame *_frame = nullptr;
	AVPacket _packet = { nullptr };
    SwrContext *_audioConvert = nullptr;
    URLVideoSource *_source;

    std::atomic<bool> _isRunning;
    std::string _url;
    Type _type;
    std::unique_ptr<std::thread> _thread;
public:

    Decoder(URLVideoSource *source, const std::string &url, const Type &type) : _source(source), _url(url), _type(type) {}

    bool open() {
        avformat_network_init();
        return avformat_open_input(&_format, _url.c_str(), nullptr, nullptr) >= 0;
    }

    void close() {
        avformat_close_input(&_format);
    }

    bool findStreams() {
        if (avformat_find_stream_info(_format, NULL) < 0) return false;

        for (uint32_t i = 0; i < _format->nb_streams; i++) {
            switch (_format->streams[i]->codecpar->codec_type) {
                case AVMEDIA_TYPE_UNKNOWN:
                    break;
                case AVMEDIA_TYPE_VIDEO:
                    if (NULL == _videoStream) {
                        AVStream *s = _format->streams[i];
                        const AVCodec *codec = avcodec_find_decoder(s->codecpar->codec_id);
                        if (!codec) break;
                        _videoContext = avcodec_alloc_context3(codec);
                        avcodec_parameters_to_context(_videoContext, s->codecpar);
                        if (avcodec_open2(_videoContext, codec, NULL) >= 0) {
                            _videoStream = s;
                        }
                    }
                    break;
                case AVMEDIA_TYPE_AUDIO:
                    if (NULL == _audioStream) {
                        AVStream *s = _format->streams[i];
                        const AVCodec *codec = avcodec_find_decoder(s->codecpar->codec_id);
                        if (!codec) break;
                        _audioContext = avcodec_alloc_context3(codec);
                        avcodec_parameters_to_context(_audioContext, s->codecpar);
                        if (avcodec_open2(_audioContext, codec, NULL) >= 0) {
                            _audioStream = s;
                        }
                    }
                    break;
                case AVMEDIA_TYPE_DATA:
                    break;
                case AVMEDIA_TYPE_SUBTITLE:
                    break;
                case AVMEDIA_TYPE_ATTACHMENT:
                    break;
                case AVMEDIA_TYPE_NB:
                    break;
            }
        }

        if (_videoStream || _audioStream) {
            _frame = av_frame_alloc();
            av_init_packet(&_packet);
            return true;
        }

        return false;
    }

    bool setupConverter() {
        if (_videoStream) {
            if (!VideoConverter::equals(_videoStream, _type)) {
                _videoConvert.open(_videoStream, _type);
            }
        }
        if (_audioStream) {
            AVCodecParameters *codecpar = _audioStream->codecpar;
            _audioConvert = swr_alloc_set_opts(
                NULL,
                codecpar->channel_layout,
                AV_SAMPLE_FMT_S16,
                codecpar->sample_rate,
                codecpar->channel_layout,
                (AVSampleFormat)codecpar->format,
                codecpar->sample_rate,
                0,
                NULL
            );
            if (_audioConvert && swr_init(_audioConvert) < 0) {
                swr_free(&_audioConvert);
            }
        }
        return _videoConvert.isValid() || _audioConvert != nullptr;
    }

    AVStream *stream(int32_t index) {
        return _format ? ((index >= 0 && index < (int32_t)_format->nb_streams) ? _format->streams[index] : NULL) : NULL;
    }

    bool readFrame(AVFrame *&frame, AVMediaType &codecType) {
        if (nullptr == _format) return false;

        while (_isRunning) {
            int result = av_read_frame(_format, &_packet);
            if (result < 0) {
                if (AVERROR_EOF == result) {
                    return false;
                }
                LOGD("av_read_frame: result %d", result);
                av_packet_unref(&_packet);
                return false;
            }
            AVStream *s = stream(_packet.stream_index);

            AVCodecContext *context = nullptr;
            if (AVMEDIA_TYPE_VIDEO == s->codecpar->codec_type) {
                context = _videoContext;
            } else if (AVMEDIA_TYPE_AUDIO == s->codecpar->codec_type) {
                context = _audioContext;
            } else {
                continue;
            }

            result = avcodec_send_packet(context, &_packet);
            if (result < 0) {
                if (AVERROR_INVALIDDATA == result) {
                    continue;
                }
                LOGD("avcodec_send_packet: result %d", result);
                av_packet_unref(&_packet);
                return false;
            }

            result = avcodec_receive_frame(context, frame);
            if (result < 0) {
                if (AVERROR(EAGAIN) == result) {
                    continue;
                }
                LOGD("avcodec_receive_frame: result %d", result);
                av_packet_unref(&_packet);
                return false;
            }

            codecType = s->codecpar->codec_type;
            av_packet_unref(&_packet);
            break;
        }
        return true;
    }

    bool decode(AVCodecContext *context, AVFrame *&frame) {
        int result = avcodec_receive_frame(context, frame);
        if (0 != result) {
            LOGD("avcodec_receive_frame: result %d", result);
            return false;
        }
        
        return true;
    }

    void run() {
        if (!open()) return;
        if (!findStreams() || !setupConverter()) {
            close();
            return;
        }

        AVMediaType codecType = AVMEDIA_TYPE_UNKNOWN;
        AVFrame *output = nullptr;
        if (_videoConvert.isValid()) {
            output = _videoConvert.alloc();
        }

        while (_isRunning && readFrame(_frame, codecType)) {
            if (AVMEDIA_TYPE_VIDEO == codecType) {
                if (_videoConvert.isValid()) {
                    _videoConvert.convert(_frame, output);
                    _source->update(output);
                } else {
                    _source->update(_frame);
                }
                std::chrono::milliseconds timespan(33);
                std::this_thread::sleep_for(timespan);
            } else if (AVMEDIA_TYPE_AUDIO == codecType) {
            }
        }

        if (_videoConvert.isValid()) {
            _videoConvert.free(output);
            _videoConvert.close();
        }

        close();

        _thread->detach();
        _thread = nullptr;
    }

    void start() {
        _isRunning = true;
        _thread = std::make_unique<std::thread>(&Decoder::run, this);
    }

    void stop() {
        _isRunning = false;
    }
};

void URLVideoSource::load(RefPtr<RenderContext> context) {
    _context = context;
    if (_source) _source->load(context);
}

void URLVideoSource::unload(RefPtr<RenderContext> context) {
    if (_source) _source->unload(context);
    _context = nullptr;
}

void URLVideoSource::draw(
    RefPtr<RenderContext> context,
    RefPtr<Framebuffer> framebuffer,
    const mat4 &globalMatrix,
    const mat4 &localMatrix,
    const mat4 &clipMatrix,
    const vec2 &size,
    float alpha
) {
    if (_source) _source->draw(context, framebuffer, globalMatrix, localMatrix, clipMatrix, size, alpha);
}

void URLVideoSource::update(const AVFrame *frame) {
    if (_source) _source->update(frame);
    _context->notifySourceChanged();
}

bool URLVideoSource::support(const Type &type) {
    if (_source) {
        return _source->support(type);
    } else {
        return false;
    }
}

bool URLVideoSource::open(const std::string &url, const Type &type) {
    assert(!_decoder);
    assert(!_source);
    if (_decoder) return false;
    if (_source) return false;

    switch (type) {
    case RGBA:
        _source = new RGBAVideoSource;
        break;
    
    case RGB24:
        _source = new RGB24VideoSource;
        break;
    
    case YUV420P:
        _source = new YUV420PVideoSource;
        break;
    
    case YUV420SP:
        _source = new YUV420SPVideoSource;
        break;
    
    default:
        return false;
    }
    if (_context) _source->load(_context);

    _decoder = std::make_unique<Decoder>(this, url, type);
    _decoder->start();
    return true;
}

void URLVideoSource::close() {
    if (_decoder) {
        _decoder->stop();
        _source = nullptr;
    }
}
