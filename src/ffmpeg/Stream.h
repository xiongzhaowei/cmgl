//
// Created by 熊朝伟 on 2023-07-03.
//

#pragma once

OMP_FFMPEG_NAMESPACE_BEGIN

template <typename T>
class StreamConsumer : public Object {
public:
    typedef typename std::conditional<std::is_base_of<RefCounted, T>::value, RefPtr<T>, T>::type Element;

    virtual void add(Element object) = 0;
    virtual void addError() = 0;
    virtual void close() = 0;
    virtual bool available() const = 0;
};

class StreamSubscription : public Object {
public:
    virtual void cancel() = 0;
};

template <typename T>
class Stream : public Object {
public:
    virtual RefPtr<StreamSubscription> listen(RefPtr<StreamConsumer<T>> consumer) = 0;


    template <typename Target>
    RefPtr<Stream<Target>> convert(std::function<typename StreamConsumer<Target>::Element(typename StreamConsumer<T>::Element)> convert);
    template <typename Converter>
    RefPtr<Stream<typename Converter::Target>> convert(RefPtr<Converter> converter);

    template <typename Target>
    RefPtr<Stream<Target>> transform(std::function<RefPtr<StreamConsumer<T>>(RefPtr<StreamConsumer<Target>>)> mapper);
    template <typename Transformer, typename ...Args>
    RefPtr<Stream<typename std::enable_if<std::is_base_of<StreamConsumer<T>, Transformer>::value, typename Transformer::Target>::type>>
    transform(RefPtr<Transformer>(*creater)(RefPtr<StreamConsumer<typename Transformer::Target>>, Args...), Args... args);
    template <typename Transformer, typename ...Args>
    RefPtr<Stream<typename std::enable_if<std::is_base_of<StreamConsumer<T>, Transformer>::value &&
        std::is_constructible<Transformer, RefPtr<StreamConsumer<typename Transformer::Target>>, Args...>::value, typename Transformer::Target>::type>>
    transform(Args... args);
};

template <typename Source, typename Target = Source>
class Converter : public Object {
public:
    typedef Target Target;

    virtual typename StreamConsumer<Target>::Element convert(typename StreamConsumer<Source>::Element object) = 0;
};

template <typename T>
class StreamController : public StreamConsumer<T> {
public:
    virtual RefPtr<Stream<T>> stream() = 0;
    virtual bool available() const = 0;
    virtual bool available(bool initial, const std::function<bool(bool, bool)>& every) const = 0;

    static RefPtr<StreamController<T>> sync();
};

template <typename T>
class _SyncStreamController : public StreamController<T> {
    std::list<RefPtr<StreamConsumer<T>>> _consumers;
    mutable std::mutex _mutex;
public:
    void add(Element object) override {
        _mutex.lock();
        std::list<RefPtr<StreamConsumer<T>>> consumers = _consumers;
        _mutex.unlock();
        for (RefPtr<StreamConsumer<T>> consumer : consumers) {
            consumer->add(object);
        }
    }
    void addError() override {
        _mutex.lock();
        std::list<RefPtr<StreamConsumer<T>>> consumers = _consumers;
        _mutex.unlock();
        for (RefPtr<StreamConsumer<T>> consumer : consumers) {
            consumer->addError();
        }
    }
    void close() override {
        _mutex.lock();
        std::list<RefPtr<StreamConsumer<T>>> consumers = std::move(_consumers);
        _mutex.unlock();
        for (RefPtr<StreamConsumer<T>> consumer : consumers) {
            consumer->close();
        }
    }
    bool available() const override {
        _mutex.lock();
        std::list<RefPtr<StreamConsumer<T>>> consumers = _consumers;
        _mutex.unlock();

        if (consumers.empty()) return false;
        for (RefPtr<StreamConsumer<T>> consumer : consumers) {
            if (!consumer->available()) return false;
        }
        return true;
    }
    bool available(bool initial, const std::function<bool(bool, bool)>& every) const override {
        _mutex.lock();
        std::list<RefPtr<StreamConsumer<T>>> consumers = _consumers;
        _mutex.unlock();

        if (consumers.empty()) return false;

        bool value = initial;
        for (RefPtr<StreamConsumer<T>> consumer : consumers) {
            value = every(value, consumer->available());
        }
        return value;
    }
    RefPtr<Stream<T>> stream() override {
        class _Stream : public Stream<T> {
            std::function<RefPtr<StreamSubscription>(RefPtr<StreamConsumer<T>>)> _listen;
        public:
            _Stream(std::function<RefPtr<StreamSubscription>(RefPtr<StreamConsumer<T>>)> listen) : _listen(listen) {}
            RefPtr<StreamSubscription> listen(RefPtr<StreamConsumer<T>> consumer) override { return _listen(consumer); }
        };
        std::function<RefPtr<StreamSubscription>(RefPtr<StreamConsumer<typename T>>)> listen = std::bind(&_SyncStreamController::listen, this, std::placeholders::_1);
        return new _Stream(listen);
    }
    RefPtr<StreamSubscription> listen(RefPtr<StreamConsumer<T>> consumer) {
        _mutex.lock();
        _consumers.push_back(consumer);
        _mutex.unlock();
        class _StreamSubscription : public StreamSubscription {
            RefPtr<_SyncStreamController<T>> _controller;
            RefPtr<StreamConsumer<T>> _consumer;
        public:
            _StreamSubscription(RefPtr<_SyncStreamController<T>> controller, RefPtr<StreamConsumer<T>> consumer) : _controller(controller), _consumer(consumer) {}
            void cancel() {
                if (_controller) {
                    _controller->cancel(_consumer);
                    _controller = nullptr;
                    _consumer = nullptr;
                }
            }
        };
        return new _StreamSubscription(this, consumer);
    }
    void cancel(RefPtr<StreamConsumer<T>> consumer) {
        _mutex.lock();
        _consumers.remove(consumer);
        _mutex.unlock();
    }
};

template <typename T>
RefPtr<StreamController<T>> StreamController<T>::sync() {
    return new _SyncStreamController<T>();
}

template <typename T>
template <typename Target>
RefPtr<Stream<Target>> Stream<T>::convert(std::function<typename StreamConsumer<Target>::Element(typename StreamConsumer<T>::Element)> convert) {
    class Converter : public StreamConsumer<T>, public Stream<Target> {
        friend class Stream<T>;
        RefPtr<StreamSubscription> _subscription;
        RefPtr<StreamController<Target>> _controller = StreamController<Target>::sync();
        std::function<StreamConsumer<Target>::Element(StreamConsumer<T>::Element)> _convert;
    public:
        Converter(std::function<StreamConsumer<Target>::Element(StreamConsumer<T>::Element)> convert) : _convert(convert) {}
        ~Converter() {
            if (_subscription != nullptr) { _subscription->cancel(); _subscription = nullptr; }
        }
        void add(StreamConsumer<T>::Element object) override { _controller->add(_convert(object)); }
        void addError() override { _controller->addError(); }
        void close() override { _controller->close(); }
        bool available() const override { return _controller->available(); }
        RefPtr<StreamSubscription> listen(RefPtr<StreamConsumer<Target>> consumer) override {
            return _controller->stream()->listen(consumer);
        }
    };
    RefPtr<Converter> converter = new Converter(convert);
    converter->_subscription = listen(converter);
    return converter;
}

template <typename T>
template <typename Converter>
RefPtr<Stream<typename Converter::Target>> Stream<T>::convert(RefPtr<Converter> converter) {
    class _Converter : public StreamConsumer<T>, public Stream<typename Converter::Target> {
        friend class Stream<T>;
        RefPtr<StreamSubscription> _subscription;
        RefPtr<StreamController<typename Converter::Target>> _controller = StreamController<typename Converter::Target>::sync();
        RefPtr<Converter> _converter;
    public:
        _Converter(RefPtr<Converter> converter) : _converter(converter) {}
        ~_Converter() {
            if (_subscription != nullptr) { _subscription->cancel(); _subscription = nullptr; }
        }
        void add(StreamConsumer<T>::Element object) override { _controller->add(_converter->convert(object)); }
        void addError() override { _controller->addError(); }
        void close() override { _controller->close(); }
        bool available() const override { return _controller->available(); }
        RefPtr<StreamSubscription> listen(RefPtr<StreamConsumer<typename Converter::Target>> consumer) override {
            return _controller->stream()->listen(consumer);
        }
    };
    RefPtr<_Converter> stream = new _Converter(converter);
    stream->_subscription = listen(stream);
    return stream;
}

template <typename T>
template <typename Target>
RefPtr<Stream<Target>> Stream<T>::transform(std::function<RefPtr<StreamConsumer<T>>(RefPtr<StreamConsumer<Target>>)> mapper) {
    class _Stream : public Stream<Target> {
        RefPtr<Stream<T>> _stream;
        std::function<RefPtr<StreamConsumer<T>>(RefPtr<StreamConsumer<Target>>)> _mapper;
    public:
        _Stream(RefPtr<Stream<T>> stream, std::function<RefPtr<StreamConsumer<T>>(RefPtr<StreamConsumer<Target>>)> mapper) : _stream(stream), _mapper(mapper) {}
        RefPtr<StreamSubscription> listen(RefPtr<StreamConsumer<Target>> consumer) override {
            return _stream->listen(_mapper(consumer));
        }
    };
    return new _Stream(this, mapper);
}

template <typename T>
template <typename Transformer, typename ...Args>
RefPtr<Stream<typename std::enable_if<
    std::is_base_of<StreamConsumer<T>, Transformer>::value &&
    std::is_constructible<Transformer, RefPtr<StreamConsumer<typename Transformer::Target>>, Args...>::value
    , typename Transformer::Target>::type>>
Stream<T>::transform(Args... args) {
    return transform(
        std::function<RefPtr<StreamConsumer<T>>(RefPtr<StreamConsumer<typename Transformer::Target>>)>(
            [args...](RefPtr<StreamConsumer<Frame>> consumer) { return new Transformer(consumer, args...); }
        )
    );
}

template <typename T>
template <typename Transformer, typename ...Args>
RefPtr<Stream<typename std::enable_if<std::is_base_of<StreamConsumer<T>, Transformer>::value, typename Transformer::Target>::type>>
Stream<T>::transform(RefPtr<Transformer>(*creater)(RefPtr<StreamConsumer<typename Transformer::Target>>, Args...), Args... args) {
    return transform(
        std::function<RefPtr<StreamConsumer<T>>(RefPtr<StreamConsumer<typename Transformer::Target>>)>(
            [creater, args...](RefPtr<StreamConsumer<Frame>> consumer) { return creater(consumer, args...); }
        )
    );
}

OMP_FFMPEG_NAMESPACE_END
