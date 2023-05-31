//
// Created by 熊朝伟 on 2023-05-31.
//

#pragma once

OMP_FFMPEG_NAMESPACE_BEGIN

template <typename T>
class SafeQueue {
    std::mutex _mutex;
public:
    std::list<T> _list;
    void push(const T& object) {
        std::lock_guard<std::mutex> lock(_mutex);
        _list.push_back(object);
    }
    T pop() {
        std::lock_guard<std::mutex> lock(_mutex);
        T object;
        if (!_list.empty()) {
            object = _list.front();
            _list.pop_front();
        }
        return object;
    }
    void swap(std::list<T>& list) {
        std::lock_guard<std::mutex> lock(_mutex);
        _list.swap(list);
    }
    void clear() {
        std::lock_guard<std::mutex> lock(_mutex);
        _list.clear();
    }
    bool empty() {
        std::lock_guard<std::mutex> lock(_mutex);
        return _list.empty();
    }
    size_t size() {
        std::lock_guard<std::mutex> lock(_mutex);
        return _list.size();
    }
    void foreach(std::function<void(T&)> callback) {
        std::lock_guard<std::mutex> lock(_mutex);
        for (auto& item : _list) {
            callback(item);
        }
    }
    void remove(const T& object) {
        std::lock_guard<std::mutex> lock(_mutex);
        _list.remove(object);
    }
    std::list<T> find(std::function<bool(T&)> where) {
        std::list<T> result;
        std::lock_guard<std::mutex> lock(_mutex);
        for (auto& item : _list) {
            if (where(item)) result.push_back(item);
        }
        return result;
    }
};

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

class Movie;

class MovieThread : public Object {
    std::unique_ptr<std::thread> _thread;
    std::list<RefPtr<Movie>> _movies;
    SafeQueue<std::function<void()>> _taskQueue;
    RefPtr<WaitableEvent> _event;
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
private:
    bool detect(std::list<RefPtr<Movie>>& list);
};

OMP_FFMPEG_NAMESPACE_END
