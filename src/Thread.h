//
//  Thread.h
//  omplayer
//
//  Created by 熊朝伟 on 2020/4/2.
//

#pragma once

OMP_NAMESPACE_BEGIN

class Thread : public Object {
public:
    class ScheduledTask : public Object {
    public:
        virtual std::chrono::steady_clock::time_point next() = 0;
        virtual bool available() const = 0;
        virtual bool exec() = 0;
    };
    class TaskList;

    Thread();
    ~Thread();

    virtual void start();
    virtual void stop();
    virtual void run();

    virtual void runOnThread(const std::function<void()>& callback);

    virtual RefPtr<ScheduledTask> schedule(double timeInterval, const std::function<bool()>& callback);
    virtual RefPtr<ScheduledTask> delay(double duration, const std::function<void()>& callback);
    virtual void cancel(RefPtr<ScheduledTask> task);

    static Thread* current();

    // 当从非Thread创建的线程中使用future对象时，使用此线程作为默认线程；
    static Thread* future();
protected:
    volatile bool _isRunning = false;
    std::unique_ptr<std::thread> _thread;
    std::mutex _mutex;
    std::list<std::function<void()>> _tasks;
    std::list<RefPtr<ScheduledTask>> _schedules;
    RefPtr<WaitableEvent> _event;

    virtual void doWork();
};

OMP_NAMESPACE_END
