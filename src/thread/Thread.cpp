//
//  Thread.cpp
//  omplayer
//
//  Created by 熊朝伟 on 2020/4/2.
//

#include "defines.h"

OMP_THREAD_USING_NAMESPACE

static void run_thread(RefPtr<Thread> thread) { thread->run(); }

void Thread::start() {
    _thread = std::make_unique<std::thread>(run_thread, this);
}

void Thread::stop() {
    _isRunning = false;
}

void Thread::run() {
    _isRunning = true;
    while (_isRunning) {
        _taskQueue.pop();
    }
}

void Thread::postTask(const std::function<void()> &closure) {
    _taskQueue.push(closure);
}

void Thread::postTaskToFront(const std::function<void()> &closure) {
    
}

void Thread::postDelayTask(const std::function<void()> &closure, const std::chrono::milliseconds &duration) {
    
}
