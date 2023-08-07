//
// Created by 熊朝伟 on 2023-07-07.
//

#include "defines.h"

OMP_FFMPEG_USING_NAMESPACE

bool MoviePlayer::supported(const std::string& path) {
    RefPtr<MovieSource> source = new MovieSource;
    if (!source->open(path)) return false;
    if (source->stream(AVMEDIA_TYPE_AUDIO)) return true;
    if (source->stream(AVMEDIA_TYPE_VIDEO)) return true;
    return false;
}

RefPtr<MoviePlayer> MoviePlayer::file(const std::string& path, RefPtr<MovieThread> thread) {
    RefPtr<MoviePlayer> player = new MoviePlayer;
    return player->open(path, thread) ? player : nullptr;
}

MoviePlayer::~MoviePlayer() {
}

bool MoviePlayer::open(RefPtr<MovieFile> file, RefPtr<MovieThread> thread) {
    RefPtr<MovieSource> source = new MovieSource;
    if (!source->open(file)) return false;
    
    RefPtr<MovieSourceStream> audioSource = MovieSourceStream::audio(source);
    RefPtr<MovieSourceStream> videoSource = MovieSourceStream::video(source);

    if (audioSource == nullptr && videoSource == nullptr) return false;

    _source = source;
    _audioSource = audioSource;
    _videoSource = videoSource;
    _thread = thread != nullptr ? thread : new MovieThread;
    _self = this;

    return true;
}

bool MoviePlayer::open(const std::string& path, RefPtr<MovieThread> thread) {
    RefPtr<MovieSource> source = new MovieSource;
    if (!source->open(path)) return false;

    RefPtr<MovieSourceStream> audioSource = MovieSourceStream::audio(source);
    RefPtr<MovieSourceStream> videoSource = MovieSourceStream::video(source);

    if (audioSource == nullptr && videoSource == nullptr) return false;

    _source = source;
    _audioSource = audioSource;
    _videoSource = videoSource;
    _thread = thread != nullptr ? thread : new MovieThread;
    _self = this;

    return true;
}

void MoviePlayer::asyncOpen(const std::string& path, const std::function<void(bool)>& callback) {
    asyncOpen(path, nullptr, callback);
}

void MoviePlayer::asyncOpen(const std::string& path, RefPtr<MovieThread> thread, const std::function<void(bool)>& callback) {
    if (thread == nullptr) thread = new MovieThread;

    thread->runOnThread([this, self = RefPtr<MoviePlayer>(this), path, thread, callback]() {
        bool result = false;

        RefPtr<MovieSource> source = new MovieSource;
        if (source->open(path)) {
            RefPtr<MovieSourceStream> audioSource = MovieSourceStream::audio(source);
            RefPtr<MovieSourceStream> videoSource = MovieSourceStream::video(source);

            if (audioSource != nullptr || videoSource != nullptr) {
                _source = source;
                _audioSource = audioSource;
                _videoSource = videoSource;
                _thread = thread;
                _self = self;

                result = true;
            }
        }
        callback(result);
    });
}

void MoviePlayer::close() {
    _thread->runOnThread([this]() {
        _thread->remove(_source);
        _thread->stop();
        _thread = nullptr;
        _source->close();
        _source = nullptr;
        if (_audioRenderer) {
            _audioRenderer->play(false);
            _audioRenderer->attach(nullptr);
            _audioRenderer->close();
            _audioRenderer = nullptr;
        }
        _videoRenderer = nullptr;
        _audioSource = nullptr;
        _videoSource = nullptr;
        _self = nullptr;
    });
}

bool MoviePlayer::ready() const {
    return _source != nullptr && _source->context() != nullptr;
}

bool MoviePlayer::isPlaying() const {
    return _isPlaying;
}

int32_t MoviePlayer::width() const {
    return _videoSource ? _videoSource->context()->width : 0;
}

int32_t MoviePlayer::height() const {
    return _videoSource ? _videoSource->context()->height : 0;
}

void MoviePlayer::bind(AVPixelFormat format, std::function<void(RefPtr<Frame>)> render) {
    RefPtr<MovieBufferedConsumer> audioBuffer;
    if (_audioSource) {
        AVStream* stream = _audioSource->stream();
        AVCodecParameters* codecpar = stream->codecpar;
        if (codecpar->ch_layout.nb_channels > 0 && codecpar->format >= 0) {
            RefPtr<MovieBufferedConsumer> buffer = new MovieBufferedConsumer(_audioSource, _maxCacheFrames);
            _audioRenderer = AudioRenderer::from(buffer, stream->time_base, _thread, (AVSampleFormat)codecpar->format, codecpar->ch_layout, codecpar->sample_rate, codecpar->frame_size);
        }
    }
    if (_videoSource) {
        AVStream* stream = _videoSource->stream();
        RefPtr<MovieBufferedConsumer> buffer = new MovieBufferedConsumer(_videoSource->convert(format), _maxCacheFrames);
        _videoRenderer = VideoRenderer::from(buffer, stream->time_base, _thread, stream->r_frame_rate, render);
    }
#if 0
    if (_audioSource && _audioRenderer == nullptr) {
        RefPtr<MovieBufferedConsumer> buffer = new MovieBufferedConsumer(_audioSource, _maxCacheFrames);
        while (_source->available()) {
            _source->read();
        }
        AVStream* stream = _audioSource->stream();
        RefPtr<Frame> frame = buffer->pop();
        if (frame != nullptr) {
            AVSampleFormat format1 = (AVSampleFormat)frame->frame()->format;
            auto bytesPerSample = av_get_bytes_per_sample(format1);
            _audioRenderer = AudioRenderer::from(buffer, stream->time_base, _thread, format1, frame->frame()->ch_layout, frame->frame()->sample_rate, frame->frame()->nb_samples);
        }
    }
#endif
    while (_source->available()) {
        _source->read();
    }
    if (_audioRenderer && _videoRenderer) _audioRenderer->attach(_videoRenderer);
    _thread->add(_source);
}

void MoviePlayer::play(bool state) {
    _isPlaying = state;
    if (_audioRenderer) {
        _thread->runOnThread([_audioRenderer = _audioRenderer, state]() { _audioRenderer->play(state); });
    } else if (_videoRenderer) {
        _thread->runOnThread([_videoRenderer = _videoRenderer, state]() { _videoRenderer->play(state); });
    }
}

void MoviePlayer::seek(double time, std::function<void(bool)> callback) {
    if (_thread == nullptr) return;

    _thread->runOnThread([this, time, callback]() {
        bool result = _source->seek(time);
        if (result) {
            // 确保跳过一帧视频和所属的全部音频，避免seek完成后，播放时间不正确。
            _source->skip([this](AVPacket* packet) { return packet->stream_index == _videoSource->stream()->index; });
            _source->skip([this](AVPacket* packet) { return packet->stream_index == _videoSource->stream()->index; });
            if (_audioRenderer) _audioRenderer->clear();
            if (_videoRenderer) _videoRenderer->clear();
        }
        if (callback) {
            if (result) {
                _thread->runOnThread([callback, result]() { callback(result); });
            } else {
                callback(false);
            }
        }
    });
}

double MoviePlayer::time() const {
    int64_t timestamp = -1;
    if (_audioRenderer != nullptr) timestamp = _audioRenderer->timestamp();
    if (timestamp == -1 && _videoRenderer != nullptr) timestamp = _videoRenderer->timestamp();
    if (timestamp != -1) return timestamp * av_q2d(AVRational{ 1, AV_TIME_BASE });
    return 0;
}

double MoviePlayer::duration() const {
    if (_source == nullptr) return 0;
    if (_source->context() == nullptr) return 0;
    return _source->context()->duration * av_q2d(AVRational{ 1, AV_TIME_BASE });
}

double MoviePlayer::volume() const {
    return _audioRenderer != nullptr ? _audioRenderer->volume() : 0;
}

void MoviePlayer::setVolume(double volume) {
    if (_audioRenderer) _audioRenderer->setVolume(volume);
}
