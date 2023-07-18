//
// Created by 熊朝伟 on 2023-05-31.
//

#include "defines.h"

OMP_FFMPEG_USING_NAMESPACE

MovieThread::ScheduledTask::ScheduledTask(double timeInterval, const std::function<bool()>& callback) : _schedule(std::chrono::steady_clock::now()), _timeInterval(int64_t(timeInterval* std::chrono::nanoseconds::period::den)), _callback(callback) {}

std::chrono::steady_clock::time_point MovieThread::ScheduledTask::next() {
    while (_schedule < std::chrono::steady_clock::now()) {
        _schedule += _timeInterval;
    }
    return _schedule;
}

bool MovieThread::ScheduledTask::available() const {
    return _schedule < std::chrono::steady_clock::now();
}

bool MovieThread::ScheduledTask::exec() {
    return _callback();
}

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
        _event->wait([this, &movies]() -> bool { return available(movies) || !_tasks->empty(); }, timeout);

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
        std::list<RefPtr<ScheduledTask>> schedules = _schedules;
        for (RefPtr<ScheduledTask> schedule : schedules) {
            if (schedule->available() && !schedule->exec()) {
                _schedules.remove(schedule);
            }
        }
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

RefPtr<MovieThread::ScheduledTask> MovieThread::schedule(double timeInterval, const std::function<bool()>& callback) {
    RefPtr<ScheduledTask> task = new ScheduledTask(timeInterval, callback);
    runOnThread([this, task]() {
        _schedules.push_back(task);
        _event->signal();
    });
    return task;
}

void MovieThread::cancel(RefPtr<ScheduledTask> task) {
    runOnThread([this, task]() { _schedules.remove(task); });
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
