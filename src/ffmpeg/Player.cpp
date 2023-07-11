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

void MoviePlayer::bind(AVPixelFormat format, std::function<void(RefPtr<Frame>)> render) {
    if (AVStream* audioStream = _source->stream(AVMEDIA_TYPE_AUDIO)) {
        RefPtr<MovieBufferedConsumer> buffer = new MovieBufferedConsumer(_audioSource, 5);
        _audioRenderer = AudioRenderer::from(buffer, audioStream->time_base, _thread, (AVSampleFormat)audioStream->codecpar->format, audioStream->codecpar->ch_layout, audioStream->codecpar->sample_rate, audioStream->codecpar->frame_size);
    }
    if (AVStream* videoStream = _source->stream(AVMEDIA_TYPE_VIDEO)) {
        RefPtr<MovieBufferedConsumer> buffer = new MovieBufferedConsumer(_videoSource->convert(format), 5);
        _videoRenderer = VideoRenderer::from(buffer, videoStream->time_base, render);
    }
    if (_audioRenderer && _videoRenderer) _audioRenderer->attach(_videoRenderer);
}

void MoviePlayer::play(bool state) {
    if (_audioRenderer) {
        _thread->runOnThread([_audioRenderer = _audioRenderer, state]() { _audioRenderer->play(state); });
    }
}

void MoviePlayer::seek(double time, std::function<void()> callback) {
    if (_thread == nullptr) return;

    _thread->runOnThread([this, time, callback]() {
        if (_source->seek(time) && callback) {
            _thread->runOnThread(callback);
        }
    });
}

double MoviePlayer::duration() const {
    assert(_source->context());
    return _source->context()->duration * av_q2d(AVRational{ 1, AV_TIME_BASE });
}

double MoviePlayer::volume() const {
    return _audioRenderer != nullptr ? _audioRenderer->volume() : 0;
}

void MoviePlayer::setVolume(double volume) {
    if (_audioRenderer) _audioRenderer->setVolume(volume);
}
