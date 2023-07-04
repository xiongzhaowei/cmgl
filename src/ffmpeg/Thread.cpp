//
// Created by 熊朝伟 on 2023-05-31.
//

#include "defines.h"

OMP_FFMPEG_USING_NAMESPACE

void DecodeThread::run() {
    AVPacket* packet = av_packet_alloc();
    RefPtr<Frame> frame = Frame::alloc();

    _isRunning = true;
    while (_isRunning) {
        std::list<RefPtr<Decoder>> decoders;
        _event->wait([this, &decoders]() -> bool { return available(decoders) || !_tasks->empty(); });

        while (!decoders.empty()) {
            std::list<RefPtr<Decoder>> finished;
            for (RefPtr<Decoder> decoder : decoders) {
                if (decoder->available()) {
                    if (!decoder->decode(packet, frame)) {
                        _decoders.remove(decoder);
                        finished.push_back(decoder);
                    }
                } else {
                    finished.push_back(decoder);
                }
            }
            for (RefPtr<Decoder> decoder : finished) {
                decoders.remove(decoder);
            }
        }

        _tasks->exec();
    }
    av_packet_free(&packet);

    if (_thread && _thread->get_id() == std::this_thread::get_id()) {
        RefPtr<DecodeThread> thread = this; // std::thread 持有 DecodeThread，防止过早释放自己；
        _thread->detach();
        _thread = nullptr;
    }
}

void DecodeThread::add(RefPtr<Decoder> decoder) {
    runOnThread([this, decoder]() {
        _decoders.push_back(decoder);
        _event->signal();
    });
}

void DecodeThread::remove(RefPtr<Decoder> movie) {
    runOnThread([this, movie]() { _decoders.remove(movie); });
}

RefPtr<Decoder> DecodeThread::decode(const std::string& url) {
    RefPtr<Decoder> decoder = Decoder::from(url, this);
    if (decoder != nullptr) add(decoder);
    return decoder;
}

bool DecodeThread::available(std::list<RefPtr<Decoder>>& list) {
    std::list<RefPtr<Decoder>> result;
    for (RefPtr<Decoder> decoder : _decoders) {
        if (decoder->available()) {
            result.push_back(decoder);
        }
    }
    if (result.empty()) return false;
    list = std::move(result);
    return true;
}

void MovieThread::run() {
    _isRunning = true;
    while (_isRunning) {
        std::list<RefPtr<MovieSource>> movies;
        _event->wait([this, &movies]() -> bool { return available(movies) || !_tasks->empty(); });

        while (!movies.empty()) {
            std::list<RefPtr<MovieSource>> finished;
            for (RefPtr<MovieSource> movie : movies) {
                if (movie->available()) {
                    if (!movie->read()) {
                        _movies.remove(movie);
                        finished.push_back(movie);
                    }
                } else {
                    finished.push_back(movie);
                }
            }
            for (RefPtr<MovieSource> movie : finished) {
                movies.remove(movie);
            }
        }

        _tasks->exec();
    }

    if (_thread && _thread->get_id() == std::this_thread::get_id()) {
        RefPtr<MovieThread> thread = this; // std::thread 持有 MovieThread，防止过早释放自己；
        _thread->detach();
        _thread = nullptr;
    }
}

void MovieThread::add(RefPtr<MovieSource> movie) {
    runOnThread([this, movie]() {
        _movies.push_back(movie);
        _event->signal();
    });
}

void MovieThread::remove(RefPtr<MovieSource> movie) {
    runOnThread([this, movie]() { _movies.remove(movie); });
}

bool MovieThread::available(std::list<RefPtr<MovieSource>>& list) {
    std::list<RefPtr<MovieSource>> result;
    for (RefPtr<MovieSource> movie : _movies) {
        if (movie->available()) {
            result.push_back(movie);
        }
    }
    if (result.empty()) return false;
    list = std::move(result);
    return true;
}
