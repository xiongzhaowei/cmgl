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
    class TaskList;

    Thread();

    virtual void start();
    virtual void stop();
    virtual void run();

    virtual void runOnThread(const std::function<void()>& callback);

    static Thread* current();

    // 当从非Thread创建的线程中使用future对象时，使用此线程作为默认线程；
    static Thread* future();
protected:
    volatile bool _isRunning = false;
    std::unique_ptr<std::thread> _thread;
    RefPtr<TaskList> _tasks;
    RefPtr<WaitableEvent> _event;
};

class Thread::TaskList : public Object {
    std::list<std::function<void()>> _list;
    std::mutex _mutex;
public:
    void push(const std::function<void()>& object);
    void exec();
    bool empty();
};

OMP_NAMESPACE_END
