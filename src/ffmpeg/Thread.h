//
// Created by 熊朝伟 on 2023-05-31.
//

#pragma once

OMP_FFMPEG_NAMESPACE_BEGIN

class WaitableEvent : public Object {
    std::mutex _mutex;
    std::condition_variable _notify;
public:
    void signal() { _notify.notify_one(); }
    void wait(std::function<bool()> pred) {
        std::unique_lock<std::mutex> lock(_mutex);
        _notify.wait(lock, pred);
    }
};

class MovieThread : public Object {
    friend class Movie;
    class TaskQueue;

    std::unique_ptr<std::thread> _thread;
    std::list<RefPtr<Movie>> _movies;
    RefPtr<TaskQueue> _taskQueue;
    RefPtr<WaitableEvent> _event = new WaitableEvent;
    volatile bool _isRunning = false;
public:
    MovieThread();
    ~MovieThread();

    void start();
    void stop();
    void run();

    void add(RefPtr<Movie> movie);
    void remove(RefPtr<Movie> movie);
    void runOnThread(const std::function<void()>& callback);

    RefPtr<Movie> movie(const std::string& url, AVPixelFormat pixel, AVSampleFormat sample);
private:
    bool detect(std::list<RefPtr<Movie>>& list);
};

class MovieThread::TaskQueue : public Object {
    std::list<std::function<void()>> _list;
    std::mutex _mutex;
public:
    void push(const std::function<void()>& object);
    void exec();
    bool empty();
};

OMP_FFMPEG_NAMESPACE_END
