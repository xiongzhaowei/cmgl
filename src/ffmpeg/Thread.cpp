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
