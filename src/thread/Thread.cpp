//
//  Thread.cpp
//  omplayer
//
//  Created by 熊朝伟 on 2020/4/2.
//

#include "defines.h"

OMP_THREAD_USING_NAMESPACE

Thread::Thread() : _tasks(new TaskList), _event(new WaitableEvent) {

}

void Thread::start() {
    assert(_thread == nullptr);

    RefPtr<Thread> self = this;
    _thread = std::make_unique<std::thread>([self]() { self->run(); });
}

void Thread::stop() {
    runOnThread([this]() { _isRunning = false; });
}

void Thread::run() {
    _isRunning = true;
    while (_isRunning) {
        _event->wait([this]() { return !_tasks->empty(); });
        _tasks->exec();
    }
}

void Thread::runOnThread(const std::function<void()>& callback) {
    _tasks->push(callback);
    _event->signal();
}

void Thread::TaskList::push(const std::function<void()>& object) {
    std::lock_guard<std::mutex> lock(_mutex);
    _list.push_back(object);
}

void Thread::TaskList::exec() {
    _mutex.lock();
    std::list<std::function<void()>> list = std::move(_list);
    _mutex.unlock();
    for (auto& task : list) {
        task();
    }
}

bool Thread::TaskList::empty() {
    std::lock_guard<std::mutex> lock(_mutex);
    return _list.empty();
}
