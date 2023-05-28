//
// Created by 熊朝伟 on 2020-05-07.
//

#include "defines.h"

OMP_RENDER_USING_NAMESPACE

#if 0
static void __renderThread(RefPtr<RenderThread<>> thread) { thread->run(); }

RenderThread<>::RenderThread(const std::string &name) {

}

void RenderThread<>::start() {
    assert(!_isRunning);
    if (!_isRunning) {
        _isRunning = true;
        _thread = std::make_unique<std::thread>(__renderThread, this);
    }
}

void RenderThread<>::stop() {
    _isRunning = false;
}

void RenderThread<>::run() {
    using clock = std::chrono::system_clock;
    std::chrono::duration<double> interval(1.0 / 60.0);
    std::chrono::time_point<clock, decltype(interval)> next = clock::now();
    onThreadStart();

    while (_isRunning) {
        std::queue<std::function<void(RefPtr<RenderContext>)>> taskQueue;

        _mutex.lock();
        std::swap(taskQueue, _taskQueue);
        _mutex.unlock();
        
        while (!taskQueue.empty()) {
            taskQueue.front()(_context);
            taskQueue.pop();
        }

        if (next < clock::now()) {
            render();
            next += interval;
        }

        std::unique_lock<std::mutex> l(_mutex);
        _event.wait_until(l, next);
    }

    onThreadStop();
}

void RenderThread<>::runOnRenderThread(const std::function<void(RefPtr<RenderContext>)> &callback) {
    if (_thread && std::this_thread::get_id() == _thread->get_id()) {
        callback(_context);
    } else {
        _mutex.lock();
        _taskQueue.push(callback);
        _mutex.unlock();
        _event.notify_all();
    }
}

void RenderThread<>::render() {
    if (_context) _context->render();
}

void RenderThread<>::onThreadStart() {
    if (_context) _context->load();
}

void RenderThread<>::onThreadStop() {
    if (_context) _context->unload();
}
#endif
