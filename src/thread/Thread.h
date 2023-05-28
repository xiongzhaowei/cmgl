//
//  Thread.h
//  omplayer
//
//  Created by 熊朝伟 on 2020/4/2.
//

OMP_THREAD_NAMESPACE_BEGIN

class Thread : public Object {
public:
    
    void start();
    void stop();
    void run();
    
    void postTask(const std::function<void()> &closure);
    void postTaskToFront(const std::function<void()> &closure);
    void postDelayTask(const std::function<void()> &closure, const std::chrono::milliseconds &duration);
protected:
    std::atomic<bool> _isRunning;
    std::mutex _lock;
    std::unique_ptr<std::thread> _thread;
    std::queue<std::function<void()>> _taskQueue;
    std::queue<std::pair<std::function<void()>, std::chrono::milliseconds>> _delayTaskQueue;
};

OMP_THREAD_NAMESPACE_END
