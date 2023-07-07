//
// Created by 熊朝伟 on 2023-05-31.
//

#include "defines.h"

OMP_FFMPEG_USING_NAMESPACE

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
