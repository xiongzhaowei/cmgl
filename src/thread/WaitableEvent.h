//
// Created by 熊朝伟 on 2023-06-03.
//

#pragma once

OMP_THREAD_NAMESPACE_BEGIN

class WaitableEvent : public Object {
    std::mutex _mutex;
    std::condition_variable _notify;
public:
    void signal();
    void wait(const std::function<bool()>& pred);
};

OMP_THREAD_NAMESPACE_END
