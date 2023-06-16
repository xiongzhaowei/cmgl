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
    typedef typename std::conditional<std::is_base_of<RefCounted, T>::value, RefPtr<T>, T>::type Element;

    virtual RefPtr<StreamSubscription<T>> listen(RefPtr<Consumer<T>> consumer) = 0;

    template <typename Target>
    RefPtr<Stream<Target>> convert(std::function<typename Stream<Target>::Element(typename Element)> convert);
};

template <typename T>
class Consumer : public Object {
public:
    typedef typename Stream<T>::Element Element;

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
    static RefPtr<StreamController<T>> sync(
        const std::function<void()>& onPause = std::function<void()>(),
        const std::function<void()>& onResume = std::function<void()>()
    );
};

template <typename T>
template <typename Target>
RefPtr<Stream<Target>> Stream<T>::convert(std::function<typename Stream<Target>::Element(typename Element)> convert) {
    class Converter : public Consumer<T>, public Stream<Target> {
        RefPtr<StreamController<Target>> _controller = StreamController<Target>::sync();
        std::function<Consumer<Target>::Element(Consumer<T>::Element)> _convert;
    public:
        Converter(std::function<Consumer<Target>::Element(Consumer<T>::Element)> convert) : _convert(convert) {}
        void add(Consumer<T>::Element object) override { _controller->add(_convert(object)); }
        void addError() override { _controller->addError(); }
        void close() override { _controller->close(); }
        RefPtr<StreamSubscription<Target>> listen(RefPtr<Consumer<Target>> consumer) override {
            return _controller->stream()->listen(consumer);
        }
    };
    RefPtr<Converter> converter = new Converter(convert);
    listen(converter);
    return converter;
}

OMP_FFMPEG_NAMESPACE_END

#include "Stream.inl"
