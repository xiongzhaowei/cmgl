//
// Created by 熊朝伟 on 2023-06-01.
//

#pragma once

OMP_FFMPEG_NAMESPACE_BEGIN

template <typename T>
class _Stream;

template <typename T>
class _StreamSubscription : public StreamSubscription<typename std::enable_if<std::is_base_of<RefCounted, T>::value, T>::type> {
    friend class _Stream<T>;
    WeakPtr<_Stream<T>> _stream;
    RefPtr<Consumer<T>> _consumer;
public:
    void cancel() override;
    void pause() override;
    void resume() override;
    void _add(RefPtr<T> object);
    void _error();
};

template <typename T>
class _Stream : public Stream<T> {
    volatile bool _isPaused = false;
    RefPtr<Thread> _thread;
    std::list<RefPtr<_StreamSubscription<T>>> _subscriptions;
public:
    _Stream(Thread* thread);

    RefPtr<StreamSubscription<T>> listen(RefPtr<Consumer<T>> consumer) override;
    bool isPaused() const;

    void _cancel(RefPtr<_StreamSubscription<T>> subscription);
    void _pause(bool state);
    void _add(RefPtr<T> object);
    void _error();
    void _close();
};

template <typename T>
void _StreamSubscription<T>::cancel() {
    RefPtr<_StreamSubscription<T>> self = this;
    _stream->_cancel(this);
    _stream = nullptr;
    _consumer->close();
    _consumer = nullptr;
}

template <typename T>
void _StreamSubscription<T>::pause() {
    _stream->_pause(true);
}

template <typename T>
void _StreamSubscription<T>::resume() {
    _stream->_pause(false);
}

template <typename T>
void _StreamSubscription<T>::_add(RefPtr<T> object) {
    _consumer->add(object);
}

template <typename T>
void _StreamSubscription<T>::_error() {
    _consumer->addError();
}

template <typename T>
class _StreamController : public StreamController<typename std::enable_if<std::is_base_of<RefCounted, T>::value, T>::type> {
    RefPtr<_Stream<T>> _stream;
public:
    _StreamController(Thread* thread);

    RefPtr<Stream<T>> stream() const override;
    void add(RefPtr<T> object) override;
    void addError() override;
    void close() override;
};

template <typename T>
_StreamController<T>::_StreamController(Thread* thread) : _stream(new _Stream<T>(thread)) {

}

template <typename T>
RefPtr<Stream<T>> _StreamController<T>::stream() const {
    return _stream;
}

template <typename T>
void _StreamController<T>::add(RefPtr<T> object) {
    _stream->_add(object);
}

template <typename T>
void _StreamController<T>::addError() {
    _stream->_error();
}

template <typename T>
void _StreamController<T>::close() {
    _stream->_close();
}

template <typename T>
_Stream<T>::_Stream(Thread* thread) : _thread(thread) {

}

template <typename T>
RefPtr<StreamSubscription<T>> _Stream<T>::listen(RefPtr<Consumer<T>> consumer) {
    RefPtr<_StreamSubscription<T>> subscription = new _StreamSubscription<T>;
    subscription->_stream = this;
    subscription->_consumer = consumer;
    _subscriptions.push_back(subscription);
    return subscription;
}

template <typename T>
bool _Stream<T>::isPaused() const {
    return _isPaused;
}

template <typename T>
void _Stream<T>::_cancel(RefPtr<_StreamSubscription<T>> subscription) {
    RefPtr<_Stream<T>> self = this;
    _thread->runOnThread([self, subscription]() {
        self->_subscriptions.remove(subscription);
    });
}

template <typename T>
void _Stream<T>::_pause(bool state) {
    RefPtr<_Stream<T>> self = this;
    _thread->runOnThread([self, state]() {
        self->_isPaused = state;
    });
}

template <typename T>
void _Stream<T>::_add(RefPtr<T> object) {
    assert(_isPaused == false);
    RefPtr<_Stream<T>> self = this;
    _thread->runOnThread([self, object]() {
        std::list<RefPtr<_StreamSubscription<T>>> subscriptions = self->_subscriptions;
        for (RefPtr<_StreamSubscription<T>> subscription : subscriptions) {
            subscription->_add(object);
        }
    });
}

template <typename T>
void _Stream<T>::_error() {
    RefPtr<_Stream<T>> self = this;
    _thread->runOnThread([self]() {
        std::list<RefPtr<_StreamSubscription<T>>> subscriptions = self->_subscriptions;
        for (RefPtr<_StreamSubscription<T>> subscription : subscriptions) {
            subscription->_error();
        }
    });
}

template <typename T>
void _Stream<T>::_close() {
    RefPtr<_Stream<T>> self = this;
    _thread->runOnThread([self]() {
        std::list<RefPtr<_StreamSubscription<T>>> subscriptions = self->_subscriptions;
        for (RefPtr<_StreamSubscription<T>> subscription : subscriptions) {
            subscription->cancel();
        }
    });
}

template <typename T>
RefPtr<StreamController<T>> StreamController<T>::from(Thread* thread) {
    return new _StreamController<T>(thread);
}

OMP_FFMPEG_NAMESPACE_END
