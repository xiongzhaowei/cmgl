//
// Created by 熊朝伟 on 2023-06-03.
//

#pragma once

OMP_NAMESPACE_BEGIN

class WaitableEvent : public Object {
    std::mutex _mutex;
    std::condition_variable _notify;
public:
    void signal();
    void wait(const std::function<bool()>& pred);
    bool wait(const std::function<bool()>& pred, double timeout);
    bool wait(const std::function<bool()>& pred, std::chrono::steady_clock::time_point timeout);

    static bool wait(const std::function<void(RefPtr<WaitableEvent> event)>& callback, double timeout);
};

OMP_NAMESPACE_END
