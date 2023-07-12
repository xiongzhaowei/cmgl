//
// Created by 熊朝伟 on 2023-06-03.
//

#pragma once

OMP_NAMESPACE_BEGIN

template <typename T>
class _AsyncFuture : public Future<T> {
    Type _result;
    bool _isCompleted = false;
    std::list<std::function<void(Type)>> _callbacks;
public:
    _AsyncFuture(Thread* thread) : Future<T>(thread) {}

    bool isCompleted() const { return _isCompleted; }

    void _whenCompleted(Type result) {
        RefPtr<_AsyncFuture<T>> self = this;
        this->_thread->runOnThread([self, result]() {
            self->_result = result;
            self->_isCompleted = true;

            std::list<std::function<void(Type)>> callbacks = std::move(self->_callbacks);
            for (std::function<void(Type)>& callback : callbacks) {
                callback(result);
            }
        });
    }

    void then(const std::function<void(Type)>& onValue) override {
        RefPtr<_AsyncFuture<T>> self = this;
        this->_thread->runOnThread([onValue, self]() {
            if (self->_isCompleted) {
                onValue(self->_result);
            } else {
                self->_callbacks.push_back(onValue);
            }
        });
    }
};

template <typename T>
class _AsyncCompleter : public Completer<T> {
    RefPtr<_AsyncFuture<T>> _future;
public:
    _AsyncCompleter(Thread* thread) : _future(new _AsyncFuture<T>(thread)) {}
    RefPtr<Future<T>> future() const { return _future; }
    bool isCompleted() const { return _future->isCompleted(); }
    void complete(typename Future<T>::Type value) { _future->_whenCompleted(value); }
};

template <typename T>
Future<T>::Future(Thread* thread) : _thread(thread) {}

template <typename T>
template <typename R>
RefPtr<Future<R>> Future<T>::then(const std::function<typename Future<R>::Type(Type)>& onValue) {
    RefPtr<_AsyncFuture<R>> future = new _AsyncFuture<R>(_thread);
    then([future, onValue](Type value) { future->_whenCompleted(onValue(value)); });
    return future;
}

template <typename T>
template <typename R>
RefPtr<Future<R>> Future<T>::then(const std::function<RefPtr<Future<R>>(Type)>& onValue) {
    RefPtr<_AsyncFuture<R>> future = new _AsyncFuture(_thread);
    then([future](Type value) {
        RefPtr<Future<R>> result = onValue(value);
        if (result) {
            result->then([future](Type value) { future->_whenCompleted(value); });
        } else {
            future->_whenCompleted(nullptr);
        }
    });
    return future;
}

template <typename T>
RefPtr<Future<T>> Future<T>::value(Type value, Thread* thread) {
    if (thread == nullptr) thread = Thread::current();
    if (thread == nullptr) thread = Thread::future();
    RefPtr<_AsyncFuture<T>> future = new _AsyncFuture<T>(thread);
    future->_whenCompleted(value);
    return future;
}

template <typename T>
RefPtr<Completer<T>> Completer<T>::async(Thread* thread) {
    if (thread == nullptr) thread = Thread::current();
    if (thread == nullptr) thread = Thread::future();
    return new _AsyncCompleter<T>(thread);
}

OMP_NAMESPACE_END
