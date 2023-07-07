//
// Created by 熊朝伟 on 2023-07-07.
//

#include "defines.h"

OMP_FFMPEG_USING_NAMESPACE

RefPtr<MoviePlayer> MoviePlayer::file(const std::string& path, RefPtr<MovieThread> thread) {
    RefPtr<MoviePlayer> player = new MoviePlayer;

    player->_source = new MovieSource;
    if (!player->_source->open(path)) return nullptr;

    player->_audioSource = MovieSourceStream::audio(player->_source);
    player->_videoSource = MovieSourceStream::video(player->_source);
    if (player->_audioSource == nullptr && player->_videoSource == nullptr) return nullptr;

    player->_thread = thread != nullptr ? thread : new MovieThread;
    player->_thread->add(player->_source);

    return player;
}

void MoviePlayer::bind(RefPtr<render::VideoSource> output, AVPixelFormat format, std::function<void()> update) {
    if (AVStream* audioStream = _source->stream(AVMEDIA_TYPE_AUDIO)) {
        _audioRenderer = AudioRenderer::from(_thread, _audioSource, audioStream->codecpar, audioStream->time_base);
    }
    if (AVStream* videoStream = _source->stream(AVMEDIA_TYPE_VIDEO)) {
        _videoRenderer = VideoRenderer::from(_thread, _videoSource, videoStream->codecpar, videoStream->time_base, output, format, update);
    }
    if (_audioRenderer && _videoRenderer) _audioRenderer->attach(_videoRenderer);
}

void MoviePlayer::play(bool state) {
    if (_audioRenderer) _audioRenderer->play(state);
}

void MoviePlayer::seek(double time, std::function<void()> callback) {
    if (_thread == nullptr) return;

    _thread->runOnThread([this, time, callback]() {
        if (!_source->context()) return;

        int64_t ts = std::clamp<int64_t>(int64_t(time * AV_TIME_BASE), 0, _source->context()->duration);
        if (avformat_seek_file(_source->context(), -1, 0, ts, _source->context()->duration, 0) >= 0) {
            if (callback) _thread->runOnThread(callback);
        }
    });
}

double MoviePlayer::duration() const {
    assert(_source->context());
    return _source->context()->duration * av_q2d(AVRational{ 1, AV_TIME_BASE });
}
