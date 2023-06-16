//
// Created by 熊朝伟 on 2023-06-01.
//

#pragma once

OMP_FFMPEG_NAMESPACE_BEGIN

template <typename T>
class _SyncStreamController;

template <typename T>
class _StreamSubscription : public StreamSubscription<T> {
public:
    WeakPtr<_SyncStreamController<T>> _controller;
    RefPtr<Consumer<T>> _consumer;

    void cancel() override;
    void pause() override;
    void resume() override;
};

template <typename T>
class _SyncStreamController : public StreamController<T> {
    volatile bool _isPaused = false;
    volatile bool _isClosed = false;
    std::mutex _mutex;
    std::list<RefPtr<_StreamSubscription<T>>> _subscriptions;
    RefPtr<Stream<T>> _stream;
    std::function<void()> _onPauseCallback;
    std::function<void()> _onResumeCallback;
public:
    _SyncStreamController(const std::function<void()>& pause, const std::function<void()>& resume);

    void add(Consumer<T>::Element object) override {
        _mutex.lock();
        std::list<RefPtr<_StreamSubscription<T>>> subscriptions = _subscriptions;
        _mutex.unlock();
        for (RefPtr<_StreamSubscription<T>> subscription : subscriptions) {
            RefPtr<Consumer<T>> consumer = subscription->_consumer;
            if (consumer) consumer->add(object);
        }
    }

    void addError() override {
        _mutex.lock();
        std::list<RefPtr<_StreamSubscription<T>>> subscriptions = _subscriptions;
        _mutex.unlock();
        for (RefPtr<_StreamSubscription<T>> subscription : subscriptions) {
            RefPtr<Consumer<T>> consumer = subscription->_consumer;
            if (consumer) consumer->addError();
        }
    }

    void close() override {
        _mutex.lock();
        std::list<RefPtr<_StreamSubscription<T>>> subscriptions = _subscriptions;
        _mutex.unlock();
        for (RefPtr<_StreamSubscription<T>> subscription : subscriptions) {
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
        RefPtr<_StreamSubscription<T>> subscription = new _StreamSubscription<T>;
        subscription->_controller = this;
        subscription->_consumer = consumer;
        _mutex.lock();
        _subscriptions.push_back(subscription);
        _mutex.unlock();
        return subscription;
    }

    void _onCancel(_StreamSubscription<T>* subscription) {
        _mutex.lock();
        subscription->_consumer = nullptr;
        subscription->_controller = nullptr;
        _subscriptions.remove(subscription);
        _mutex.unlock();
    }

    void _onPause(_StreamSubscription<T>* subscription) {
        _mutex.lock();
        _isPaused = true;
        _mutex.unlock();
        if (_onPauseCallback) _onPauseCallback();
    }

    void _onResume(_StreamSubscription<T>* subscription) {
        _mutex.lock();
        _isPaused = false;
        _mutex.unlock();
        if (_onResumeCallback) _onResumeCallback();
    }

};

template <typename T>
class _Stream : public Stream<T> {
    WeakPtr<_SyncStreamController<T>> _controller;
public:
    _Stream(_SyncStreamController<T>* controller) : _controller(controller) {}
    RefPtr<StreamSubscription<T>> listen(RefPtr<Consumer<T>> consumer) {
        return _controller->_listen(consumer);
    }
};

template <typename T>
_SyncStreamController<T>::_SyncStreamController(
    const std::function<void()>& pause,
    const std::function<void()>& resume
) : _stream(new _Stream<T>(this)), _onPauseCallback(pause), _onResumeCallback(resume) {}

template <typename T>
void _StreamSubscription<T>::cancel() {
    _controller->_onCancel(this);
}

template <typename T>
void _StreamSubscription<T>::pause() {
    _controller->_onPause(this);
}

template <typename T>
void _StreamSubscription<T>::resume() {
    _controller->_onResume(this);
}

template <typename T>
RefPtr<StreamController<T>> StreamController<T>::sync(const std::function<void()>& onPause, const std::function<void()>& onResume) {
    RefPtr<_SyncStreamController<T>> controller = new _SyncStreamController<T>(onPause, onResume);
    return controller;
}

OMP_FFMPEG_NAMESPACE_END
