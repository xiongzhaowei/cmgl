//
// Created by 熊朝伟 on 2023-06-01.
//

#pragma once

OMP_FFMPEG_NAMESPACE_BEGIN

template <typename T>
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
class Consumer : public Object {
public:
    typedef typename std::conditional<std::is_base_of<RefCounted, T>::value, RefPtr<T>, T>::type Element;

    virtual void add(Element object) = 0;
    virtual void addError() = 0;
    virtual void close() = 0;
};

template <typename T>
class StreamController : public Consumer<T> {
public:
    virtual bool isPaused() const = 0;
    virtual bool isClosed() const = 0;

    virtual RefPtr<Stream<T>> stream() const = 0;
    static RefPtr<StreamController<T>> sync(Thread* thread);
};

OMP_FFMPEG_NAMESPACE_END

#include "Stream.inl"
