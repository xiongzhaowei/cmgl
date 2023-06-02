//
// Created by 熊朝伟 on 2023-06-02.
//

#pragma once

OMP_THREAD_NAMESPACE_BEGIN

template <typename T>
class Future : public Object {
public:
    Future(Thread* thread);

    virtual void then(const std::function<void(T)>& onValue) = 0;

    template <typename R>
    RefPtr<Future<R>> then(const std::function<R(T)>& onValue);

    template <typename R>
    RefPtr<Future<R>> then(const std::function<RefPtr<Future<R>>(T)>& onValue);

    static RefPtr<Future<T>> value(Thread* thread, T value);
protected:
    RefPtr<Thread> _thread;
};

template <typename T>
class Completer : public Object {
public:
    virtual RefPtr<Future<T>> future() const = 0;
    virtual bool isCompleted() const = 0;
    virtual void complete(T value) = 0;
};

OMP_THREAD_NAMESPACE_END

#include "Thread.h"
#include "Future.inl"
