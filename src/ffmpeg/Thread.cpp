//
// Created by 熊朝伟 on 2023-05-31.
//

#include "defines.h"

OMP_FFMPEG_USING_NAMESPACE

void MovieThread::run() {
    AVPacket* packet = av_packet_alloc();
    RefPtr<Frame> frame = Frame::alloc();

    _isRunning = true;
    while (_isRunning) {
        std::list<RefPtr<Movie>> movies;
        _event->wait([this, &movies]() -> bool { return detect(movies) || !_tasks->empty(); });

        while (!movies.empty()) {
            std::list<RefPtr<Movie>> finished;
            for (RefPtr<Movie> movie : movies) {
                if (movie->detect()) {
                    if (!movie->decode(packet, frame)) {
                        _movies.remove(movie);
                        finished.push_back(movie);
                    }
                } else {
                    finished.push_back(movie);
                }
            }
            for (RefPtr<Movie> movie : finished) {
                movies.remove(movie);
            }
        }

        _tasks->exec();
    }
    av_packet_free(&packet);

    if (_thread && _thread->get_id() == std::this_thread::get_id()) {
        RefPtr<MovieThread> thread = this; // std::thread 持有 MovieThread，防止过早释放自己；
        _thread->detach();
        _thread = nullptr;
    }
}

void MovieThread::add(RefPtr<Movie> movie) {
    runOnThread([this, movie]() {
        _movies.push_back(movie);
        _event->signal();
    });
}

void MovieThread::remove(RefPtr<Movie> movie) {
    runOnThread([this, movie]() { _movies.remove(movie); });
}

RefPtr<Movie> MovieThread::movie(const std::string& url, AVPixelFormat pixel, AVSampleFormat sample) {
    RefPtr<Movie> movie = Movie::from(url, this, pixel, sample);
    if (movie != nullptr) add(movie);
    return movie;
}

bool MovieThread::detect(std::list<RefPtr<Movie>>& list) {
    std::list<RefPtr<Movie>> result;
    for (RefPtr<Movie> movie : _movies) {
        if (movie->detect()) {
            result.push_back(movie);
        }
    }
    if (result.empty()) return false;
    list = std::move(result);
    return true;
}

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
        RefPtr<DecodeThread> thread = this; // std::thread 持有 MovieThread，防止过早释放自己；
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

RefPtr<Decoder> DecodeThread::decode(const std::string& url, AVPixelFormat pixel, AVSampleFormat sample) {
    RefPtr<Decoder> decoder = Decoder::from(url, this, pixel, sample);
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
