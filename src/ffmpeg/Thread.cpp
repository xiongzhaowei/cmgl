//
// Created by 熊朝伟 on 2023-05-31.
//

#include "defines.h"

OMP_FFMPEG_USING_NAMESPACE

MovieThread::MovieThread() : _taskQueue(new TaskQueue) {

}

MovieThread::~MovieThread() {

}

void MovieThread::start() {
    assert(_thread == nullptr);

    RefPtr<MovieThread> self = this;
    _thread = std::make_unique<std::thread>([self]() { self->run(); });
}

void MovieThread::stop() {
    runOnThread([this]() { _isRunning = false; });
}

void MovieThread::run() {
    AVPacket* packet = av_packet_alloc();
    RefPtr<Frame> frame = Frame::alloc();

    _isRunning = true;
    while (_isRunning) {
        std::list<RefPtr<Movie>> movies;
        _event->wait([this, &movies]() -> bool { return detect(movies) || !_taskQueue->empty(); });

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

        _taskQueue->exec();
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

void MovieThread::runOnThread(const std::function<void()>& callback) {
    _taskQueue->push(callback);
    _event->signal();
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

void MovieThread::TaskQueue::push(const std::function<void()>& object) {
    std::lock_guard<std::mutex> lock(_mutex);
    _list.push_back(object);
}

void MovieThread::TaskQueue::exec() {
    _mutex.lock();
    std::list<std::function<void()>> list = std::move(_list);
    _mutex.unlock();
    for (auto& task : list) {
        task();
    }
}

bool MovieThread::TaskQueue::empty() {
    std::lock_guard<std::mutex> lock(_mutex);
    return _list.empty();
}
