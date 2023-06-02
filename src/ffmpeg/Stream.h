//
// Created by 熊朝伟 on 2023-06-01.
//

#pragma once

OMP_FFMPEG_NAMESPACE_BEGIN

template <typename T>
class Stream;

template <typename T>
class Consumer : public Object {
public:
    virtual RefPtr<Future<void>> add(RefPtr<T> object) = 0;
    virtual void addError() = 0;
    virtual void close() = 0;

    RefPtr<Future<void>> addStream(RefPtr<Stream<T>> stream);
};

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

OMP_FFMPEG_NAMESPACE_END
