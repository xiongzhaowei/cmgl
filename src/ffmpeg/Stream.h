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
};

OMP_FFMPEG_NAMESPACE_END

#include "Stream.inl"
