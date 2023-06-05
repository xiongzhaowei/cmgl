//
// Created by 熊朝伟 on 2023-06-01.
//

#pragma once

OMP_FFMPEG_NAMESPACE_BEGIN

template <typename T, typename = void>
class Consumer;

template <typename T>
class StreamSubscription : public Object {
public:
    virtual void cancel() = 0;
    virtual void pause() = 0;
    virtual void resume() = 0;
};

template <typename T>
class Stream : public Object {
public:
    virtual RefPtr<StreamSubscription<T>> listen(RefPtr<Consumer<T>> consumer) = 0;
};

template <typename T>
class Consumer<T, typename std::enable_if<std::is_base_of<RefCounted, T>::value>::type> : public Object {
public:
    virtual void add(RefPtr<T> object) = 0;
    virtual void addError() = 0;
    virtual void close() = 0;
};

template <typename T>
class Consumer<T, typename std::enable_if<!std::is_base_of<RefCounted, T>::value>::type> : public Object {
public:
    virtual void add(T object) = 0;
    virtual void addError() = 0;
    virtual void close() = 0;
};

template <typename T>
class StreamController : public Consumer<T> {
public:
    virtual bool isPaused() const = 0;
    virtual bool isClosed() const = 0;

    virtual RefPtr<Stream<T>> stream() const = 0;
    static RefPtr<StreamController<T>> from(Thread* thread);
    static RefPtr<StreamController<T>> sync(Thread* thread);
};

template <typename T>
class _SyncStreamController;

template <typename T>
class _SyncStreamSubscription : public StreamSubscription<T> {
public:
    WeakPtr<_SyncStreamController<T>> _controller;
    RefPtr<Consumer<T>> _consumer;

    void cancel() override;
    void pause() override;
    void resume() override;
};

template <typename T>
class __Stream : public Stream<T> {
    WeakPtr<_SyncStreamController<T>> _controller;
public:
    __Stream(_SyncStreamController<T>* controller) : _controller(controller) {}
    RefPtr<StreamSubscription<T>> listen(RefPtr<Consumer<T>> consumer);
};

template <typename T>
class _SyncStreamController : public StreamController<typename std::enable_if<std::is_base_of<RefCounted, T>::value, T>::type> {
    volatile bool _isPaused = false;
    volatile bool _isClosed = false;
    std::mutex _mutex;
    std::list<RefPtr<_SyncStreamSubscription<T>>> _subscriptions;
    RefPtr<Thread> _thread;
    RefPtr<Stream<T>> _stream;
public:
    _SyncStreamController(Thread* thread) : _thread(thread), _stream(new __Stream<T>(this)) { assert(thread != nullptr); }

    void add(RefPtr<T> object) {
        _mutex.lock();
        std::list<RefPtr<_SyncStreamSubscription<T>>> subscriptions = _subscriptions;
        _mutex.unlock();
        for (RefPtr<_SyncStreamSubscription<T>> subscription : subscriptions) {
            RefPtr<Consumer<T>> consumer = subscription->_consumer;
            if (consumer) consumer->add(object);
        }
    }

    void addError() {
        _mutex.lock();
        std::list<RefPtr<_SyncStreamSubscription<T>>> subscriptions = _subscriptions;
        _mutex.unlock();
        for (RefPtr<_SyncStreamSubscription<T>> subscription : subscriptions) {
            RefPtr<Consumer<T>> consumer = subscription->_consumer;
            if (consumer) consumer->addError();
        }
    }

    void close() {
        _mutex.lock();
        std::list<RefPtr<_SyncStreamSubscription<T>>> subscriptions = _subscriptions;
        _mutex.unlock();
        for (RefPtr<_SyncStreamSubscription<T>> subscription : subscriptions) {
            RefPtr<Consumer<T>> consumer = subscription->_consumer;
            if (consumer) consumer->close();
        }
        _mutex.lock();
        _isClosed = true;
        _mutex.unlock();
    }

    bool isPaused() const override { return _isPaused; }
    bool isClosed() const override { return _isClosed; }

    RefPtr<Stream<T>> stream() const {
        return _stream;
    }

    RefPtr<StreamSubscription<T>> _listen(RefPtr<Consumer<T>> consumer) {
        if (isClosed()) {
            consumer->close();
            return nullptr;
        }
        RefPtr<_SyncStreamSubscription<T>> subscription = new _SyncStreamSubscription<T>;
        subscription->_controller = this;
        subscription->_consumer = consumer;
        _mutex.lock();
        _subscriptions.push_back(subscription);
        _mutex.unlock();
        return subscription;
    }

    void _onCancel(_SyncStreamSubscription<T>* subscription) {
        _mutex.lock();
        subscription->_consumer = nullptr;
        subscription->_controller = nullptr;
        _subscriptions.remove(subscription);
        _mutex.unlock();
    }

    void _onPause(_SyncStreamSubscription<T>* subscription) {
        _mutex.lock();
        _isPaused = true;
        _mutex.unlock();
        _thread->runOnThread([]() {});
    }

    void _onResume(_SyncStreamSubscription<T>* subscription) {
        _mutex.lock();
        _isPaused = false;
        _mutex.unlock();
        _thread->runOnThread([]() {});
    }

};

template <typename T>
void _SyncStreamSubscription<T>::cancel() {
    _controller->_onCancel(this);
}

template <typename T>
void _SyncStreamSubscription<T>::pause() {
    _controller->_onPause(this);
}

template <typename T>
void _SyncStreamSubscription<T>::resume() {
    _controller->_onResume(this);
}

template <typename T>
RefPtr<StreamSubscription<T>> __Stream<T>::listen(RefPtr<Consumer<T>> consumer) {
    return _controller->_listen(consumer);
}

template <typename T>
RefPtr<StreamController<T>> StreamController<T>::sync(Thread* thread) {
    RefPtr<_SyncStreamController<T>> controller = new _SyncStreamController<T>(thread);
    return controller;
}

OMP_FFMPEG_NAMESPACE_END

#include "Stream.inl"
