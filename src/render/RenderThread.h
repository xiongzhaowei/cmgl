//
// Created by 熊朝伟 on 2020-05-07.
//

#pragma once

OMP_RENDER_NAMESPACE_BEGIN

#if 0
template <typename T = void>
class RenderThread;

template <>
class RenderThread<void> : public Object {
public:
    RenderThread(const std::string &name);
    
    void start();
    void stop();
    void run();
    void render();
    void runOnRenderThread(const std::function<void(RefPtr<RenderContext>)> &callback);
    
    virtual void onThreadStart();
    virtual void onThreadStop();
protected:
    std::unique_ptr<std::thread> _thread;
    std::atomic<bool> _isRunning;
    std::queue<std::function<void(RefPtr<RenderContext>)>> _taskQueue;
    std::mutex _mutex;
    std::condition_variable _event;
    RefPtr<RenderContext> _context;
};

template <typename T>
class RenderThread : public RenderThread<> {
public:
    RenderThread(const std::string &name) : RenderThread<>(name) {}
    void onThreadStart() override;
    void onThreadStop() override;
};

template <typename T>
void RenderThread<T>::onThreadStart() {
    _context = new T;
    RenderThread<>::onThreadStart();
}

template <typename T>
void RenderThread<T>::onThreadStop() {
    RenderThread<>::onThreadStop();
}
#endif

OMP_RENDER_NAMESPACE_END
