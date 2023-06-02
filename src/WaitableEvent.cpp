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
