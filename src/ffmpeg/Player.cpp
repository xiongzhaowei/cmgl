//
// Created by 熊朝伟 on 2023-07-07.
//

#include "defines.h"

OMP_FFMPEG_USING_NAMESPACE

RefPtr<MoviePlayer> MoviePlayer::file(const std::string& path, RefPtr<MovieThread> thread) {
    RefPtr<MoviePlayer> player = new MoviePlayer;
    return player->open(path, thread) ? player : nullptr;
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
    _thread->add(_source);

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
    _thread->add(_source);

    return true;
}

void MoviePlayer::close() {
    _thread->runOnThread([this]() {
        _audioRenderer->play(false);
        _audioRenderer->attach(nullptr);
        _thread->remove(_source);
        _thread = nullptr;
        _source->close();
        _source = nullptr;
        _audioRenderer = nullptr;
        _videoRenderer = nullptr;
        _audioSource = nullptr;
        _videoSource = nullptr;
    });
}

bool MoviePlayer::ready() const {
    return _source != nullptr;
}

bool MoviePlayer::isPlaying() const {
    return _isPlaying;
}

int32_t MoviePlayer::width() const {
    return _videoSource->context()->width;
}

int32_t MoviePlayer::height() const {
    return _videoSource->context()->height;
}

void MoviePlayer::bind(AVPixelFormat format, std::function<void(RefPtr<Frame>)> render) {
    if (_audioSource) {
        AVStream* stream = _audioSource->stream();
        AVCodecParameters* codecpar = stream->codecpar;
        RefPtr<MovieBufferedConsumer> buffer = new MovieBufferedConsumer(_audioSource, _maxCacheFrames);
        _audioRenderer = AudioRenderer::from(buffer, stream->time_base, _thread, (AVSampleFormat)codecpar->format, codecpar->ch_layout, codecpar->sample_rate, codecpar->frame_size);
    }
    if (_videoSource) {
        AVStream* stream = _videoSource->stream();
        RefPtr<MovieBufferedConsumer> buffer = new MovieBufferedConsumer(_videoSource->convert(format), _maxCacheFrames);
        _videoRenderer = VideoRenderer::from(buffer, stream->time_base, _thread, stream->r_frame_rate, render);
    }
    if (_audioRenderer && _videoRenderer) _audioRenderer->attach(_videoRenderer);
}

void MoviePlayer::play(bool state) {
    _isPlaying = state;
    if (_audioRenderer) {
        _thread->runOnThread([_audioRenderer = _audioRenderer, state]() { _audioRenderer->play(state); });
    } else if (_videoRenderer) {
        _thread->runOnThread([_videoRenderer = _videoRenderer, state]() { _videoRenderer->play(state); });
    }
}

void MoviePlayer::seek(double time) {
    if (_thread == nullptr) return;

    _thread->runOnThread([this, time]() {
        bool result = _source->seek(time);
        if (result) {
            // 确保跳过一帧视频和所属的全部音频，避免seek完成后，播放时间不正确。
            _source->skip([this](AVPacket* packet) { return packet->stream_index == _videoSource->stream()->index; });
            _source->skip([this](AVPacket* packet) { return packet->stream_index == _videoSource->stream()->index; });
            if (_audioRenderer) _audioRenderer->clear();
            if (_videoRenderer) _videoRenderer->clear();
        }
    });
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
    return _source->context()->duration * av_q2d(AVRational{ 1, AV_TIME_BASE });
}

double MoviePlayer::volume() const {
    return _audioRenderer != nullptr ? _audioRenderer->volume() : 0;
}

void MoviePlayer::setVolume(double volume) {
    if (_audioRenderer) _audioRenderer->setVolume(volume);
}
