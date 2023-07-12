//
// Created by 熊朝伟 on 2023-06-02.
//

#pragma once

OMP_NAMESPACE_BEGIN

template <typename T>
class Future : public Object {
public:
    typedef typename std::conditional<std::is_base_of<RefCounted, T>::value, RefPtr<T>, T>::type Type;

    Future(Thread* thread);

    virtual void then(const std::function<void(Type)>& onValue) = 0;

    template <typename R>
    RefPtr<Future<R>> then(const std::function<typename Future<R>::Type(Type)>& onValue);

    template <typename R>
    RefPtr<Future<R>> then(const std::function<RefPtr<Future<R>>(Type)>& onValue);

    static RefPtr<Future<T>> value(Type value, Thread* thread = nullptr);
protected:
    RefPtr<Thread> _thread;
};

template <typename T>
class Completer : public Object {
public:
    virtual RefPtr<Future<T>> future() const = 0;
    virtual bool isCompleted() const = 0;
    virtual void complete(typename Future<T>::Type value) = 0;

    static RefPtr<Completer<T>> async(Thread* thread = nullptr);
};

OMP_NAMESPACE_END

#include "Future.inl"
