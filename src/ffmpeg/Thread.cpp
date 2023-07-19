//
// Created by 熊朝伟 on 2023-05-31.
//

#include "defines.h"

OMP_FFMPEG_USING_NAMESPACE

void MovieThread::run() {
    _isRunning = true;

    while (_isRunning) {
        std::list<RefPtr<MovieSource>> movies;
        std::chrono::steady_clock::time_point timeout = std::chrono::steady_clock::now() + std::chrono::seconds(1);
        for (RefPtr<ScheduledTask> schedule : _schedules) {
            std::chrono::steady_clock::time_point next = schedule->next();
            if (timeout > next) {
                timeout = next;
            }
        }
        _event->wait([this, &movies]() -> bool {
            if (available(movies)) return true;

            std::lock_guard<std::mutex> lock(_mutex);
            return !_tasks.empty();
        }, timeout);

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
        doWork();
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

bool MovieThread::available(std::list<RefPtr<MovieSource>>& list) const {
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
