//
// Created by 熊朝伟 on 2023-06-01.
//

#pragma once

OMP_FFMPEG_NAMESPACE_BEGIN

template <typename T>
class Consumer;

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

    virtual RefPtr<StreamSubscription> listen(RefPtr<Consumer<T>> consumer) = 0;

    template <typename Target>
    RefPtr<Stream<Target>> convert(std::function<typename Stream<Target>::Element(typename Element)> convert);

    template <typename Converter>
    RefPtr<Stream<typename Converter::Target>> convert(RefPtr<Converter> converter);

    template <typename Target>
    RefPtr<Stream<Target>> transform(std::function<RefPtr<Consumer<T>>(RefPtr<Consumer<Target>>)> mapper);

    template <typename Transformer, typename ...Args>
    RefPtr<Stream<typename std::enable_if<std::is_base_of<Consumer<T>, Transformer>::value, typename Transformer::Target>::type>>
    transform(RefPtr<Transformer>(*creater)(RefPtr<Consumer<typename Transformer::Target>>, Args...), Args... args);

    template <typename Transformer, typename ...Args>
    RefPtr<Stream<typename std::enable_if<
        std::is_base_of<Consumer<T>, Transformer>::value &&
        std::is_constructible<Transformer, RefPtr<Consumer<typename Transformer::Target>>, Args...>::value
        , typename Transformer::Target>::type>>
    transform(Args... args);
};

template <typename T>
class Consumer : public Object {
public:
    typedef typename Stream<T>::Element Element;

    virtual void add(Element object) = 0;
    virtual void addError() = 0;
    virtual void close() = 0;
};

template <typename Source, typename Target = Source>
class Converter : public Object {
public:
    typedef Target Target;

    virtual typename Stream<Target>::Element convert(typename Stream<Source>::Element object) = 0;
};

template <typename Source, typename Target = Source>
class Transformer : public Consumer<Source> {
public:
    typedef Target Target;
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
        RefPtr<StreamSubscription> listen(RefPtr<Consumer<Target>> consumer) override {
            return _controller->stream()->listen(consumer);
        }
    };
    RefPtr<Converter> converter = new Converter(convert);
    listen(converter);
    return converter;
}

template <typename T>
template <typename Converter>
RefPtr<Stream<typename Converter::Target>> Stream<T>::convert(RefPtr<Converter> converter) {
    class _Converter : public Consumer<T>, public Stream<typename Converter::Target> {
        RefPtr<StreamController<typename Converter::Target>> _controller = StreamController<typename Converter::Target>::sync();
        RefPtr<Converter> _converter;
    public:
        _Converter(RefPtr<Converter> converter) : _converter(converter) {}
        void add(Consumer<T>::Element object) override { _controller->add(_converter->convert(object)); }
        void addError() override { _controller->addError(); }
        void close() override { _controller->close(); }
        RefPtr<StreamSubscription> listen(RefPtr<Consumer<typename Converter::Target>> consumer) override {
            return _controller->stream()->listen(consumer);
        }
    };
    RefPtr<_Converter> stream = new _Converter(converter);
    listen(stream);
    return stream;
}

template <typename T>
template <typename Target>
RefPtr<Stream<Target>> Stream<T>::transform(std::function<RefPtr<Consumer<T>>(RefPtr<Consumer<Target>>)> mapper) {
    class _Stream : public Stream<Target> {
        RefPtr<Stream<T>> _stream;
        std::function<RefPtr<Consumer<T>>(RefPtr<Consumer<Target>>)> _mapper;
    public:
        _Stream(RefPtr<Stream<T>> stream, std::function<RefPtr<Consumer<T>>(RefPtr<Consumer<Target>>)> mapper) : _stream(stream), _mapper(mapper) {}
        RefPtr<StreamSubscription> listen(RefPtr<Consumer<Target>> consumer) override {
            return _stream->listen(_mapper(consumer));
        }
    };
    return new _Stream(this, mapper);
}

template <typename T>
template <typename Transformer, typename ...Args>
RefPtr<Stream<typename std::enable_if<
    std::is_base_of<Consumer<T>, Transformer>::value &&
    std::is_constructible<Transformer, RefPtr<Consumer<typename Transformer::Target>>, Args...>::value
    , typename Transformer::Target>::type>>
Stream<T>::transform(Args... args) {
    return transform(
        std::function<RefPtr<Consumer<T>>(RefPtr<Consumer<typename Transformer::Target>>)>(
            [args...](RefPtr<Consumer<Frame>> consumer) { return new Transformer(consumer, args...); }
        )
    );
}

template <typename T>
template <typename Transformer, typename ...Args>
RefPtr<Stream<typename std::enable_if<std::is_base_of<Consumer<T>, Transformer>::value, typename Transformer::Target>::type>>
Stream<T>::transform(RefPtr<Transformer>(*creater)(RefPtr<Consumer<typename Transformer::Target>>, Args...), Args... args) {
    return transform(
        std::function<RefPtr<Consumer<T>>(RefPtr<Consumer<typename Transformer::Target>>)>(
            [creater, args...](RefPtr<Consumer<Frame>> consumer) { return creater(consumer, args...); }
        )
    );
}

OMP_FFMPEG_NAMESPACE_END

#include "Stream.inl"
