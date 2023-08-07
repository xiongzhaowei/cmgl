//
// Created by 熊朝伟 on 2023-06-03.
//

#include "defines.h"

OMP_USING_NAMESPACE

void WaitableEvent::signal() { _notify.notify_one(); }

void WaitableEvent::wait(const std::function<bool()>& pred) {
    std::unique_lock<std::mutex> lock(_mutex);
    _notify.wait(lock, pred);
}

bool WaitableEvent::wait(const std::function<bool()>& pred, double timeout) {
    std::unique_lock<std::mutex> lock(_mutex);
    return _notify.wait_for(lock, std::chrono::microseconds(int64_t(timeout * std::chrono::microseconds::period::den)), pred);
}

bool WaitableEvent::wait(const std::function<bool()>& pred, std::chrono::steady_clock::time_point timeout) {
    std::unique_lock<std::mutex> lock(_mutex);
    return _notify.wait_until(lock, timeout, pred);
}

bool WaitableEvent::wait(const std::function<void(RefPtr<WaitableEvent> event)>& callback, double timeout) {
    RefPtr<WaitableEvent> event = new WaitableEvent;
    std::unique_lock<std::mutex> lock(event->_mutex);
    callback(event);
    return event->_notify.wait_for(lock, std::chrono::microseconds(int64_t(timeout * std::chrono::microseconds::period::den))) == std::cv_status::no_timeout;
}
