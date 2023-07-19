//
//  Thread.cpp
//  omplayer
//
//  Created by 熊朝伟 on 2020/4/2.
//

#include "defines.h"
#include <optional>

OMP_USING_NAMESPACE

static thread_local Thread* __thread;

Thread::Thread() : _event(new WaitableEvent) {

}

Thread* Thread::current() {
    return __thread;
}

Thread* Thread::future() {
    static RefPtr<Thread> thread;
    static std::mutex mutex;
    mutex.lock();
    if (thread == nullptr) {
        thread = new Thread;
        thread->start();
    }
    mutex.unlock();
    return thread;
}

void Thread::start() {
    assert(_thread == nullptr);

    RefPtr<Thread> self = this;
    _thread = std::make_unique<std::thread>([self]() {
        __thread = self;
        self->run();
        __thread = nullptr;
    });
}

void Thread::stop() {
    runOnThread([this]() { _isRunning = false; });
}

void Thread::run() {
    _isRunning = true;
    while (_isRunning) {
        std::optional<std::chrono::steady_clock::time_point> timeout;
        for (RefPtr<ScheduledTask> schedule : _schedules) {
            std::chrono::steady_clock::time_point next = schedule->next();
            if (timeout > next) timeout = next;
        }
        if (timeout.has_value()) {
            _event->wait([this]() {
                std::lock_guard<std::mutex> lock(_mutex);
                return !_tasks.empty();
            }, timeout.value());
        } else {
            _event->wait([this]() {
                std::lock_guard<std::mutex> lock(_mutex);
                return !_tasks.empty();
            });
        }
        doWork();
    }
}

void Thread::doWork() {
    _mutex.lock();
    std::list<std::function<void()>> list = std::move(_tasks);
    _mutex.unlock();
    for (auto& task : list) {
        task();
    }
    std::list<RefPtr<ScheduledTask>> schedules = _schedules;
    for (RefPtr<ScheduledTask> schedule : schedules) {
        if (schedule->available() && !schedule->exec()) {
            _schedules.remove(schedule);
        }
    }
}

void Thread::runOnThread(const std::function<void()>& callback) {
    _mutex.lock();
    _tasks.push_back(callback);
    _mutex.unlock();
    _event->signal();
    if (!_isRunning) start();
}

RefPtr<Thread::ScheduledTask> Thread::schedule(double timeInterval, const std::function<bool()>& callback) {
    class _ScheduledTask : public ScheduledTask {
        std::chrono::steady_clock::time_point _schedule;
        std::chrono::nanoseconds _timeInterval;
        std::function<bool()> _callback;
    public:
        _ScheduledTask(double timeInterval, const std::function<bool()>& callback) : _schedule(std::chrono::steady_clock::now()), _timeInterval(int64_t(timeInterval* std::chrono::nanoseconds::period::den)), _callback(callback) {}
        std::chrono::steady_clock::time_point next() override {
            while (_schedule < std::chrono::steady_clock::now()) {
                _schedule += _timeInterval;
            }
            return _schedule;
        }
        bool available() const override { return _schedule < std::chrono::steady_clock::now(); }
        bool exec() { return _callback(); }
    };
    RefPtr<ScheduledTask> task = new _ScheduledTask(timeInterval, callback);
    runOnThread([this, task]() {
        _schedules.push_back(task);
        _event->signal();
    });
    return task;
}

RefPtr<Thread::ScheduledTask> Thread::delay(double duration, const std::function<void()>& callback) {
    class _ScheduledTask : public ScheduledTask {
        std::chrono::steady_clock::time_point _schedule;
        std::function<void()> _callback;
    public:
        _ScheduledTask(double duration, const std::function<void()>& callback) : _schedule(std::chrono::steady_clock::now() + std::chrono::nanoseconds(int64_t(duration * std::chrono::nanoseconds::period::den))), _callback(callback) {}
        std::chrono::steady_clock::time_point next() override { return _schedule; }
        bool available() const override { return _schedule < std::chrono::steady_clock::now(); }
        bool exec() { _callback(); return false; }
    };
    RefPtr<ScheduledTask> task = new _ScheduledTask(duration, callback);
    runOnThread([this, task]() {
        _schedules.push_back(task);
        _event->signal();
    });
    return task;
}

void Thread::cancel(RefPtr<ScheduledTask> task) {
    runOnThread([this, task]() { _schedules.remove(task); });
}
