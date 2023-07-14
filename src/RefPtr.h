//
// Created by 熊朝伟 on 2020-03-16.
//

#pragma once

OMP_NAMESPACE_BEGIN

template <typename T>
class WeakPtr;

template <typename T>
class RefPtr {
    template <typename> friend class RefPtr;
    T *_value;
public:
    RefPtr() : _value(nullptr) {}
    RefPtr(std::nullptr_t) : _value(nullptr) {}
    RefPtr(T *other) : _value(other) { retain(); }
    RefPtr(const RefPtr<T> &other) : _value(other._value) { retain(); }
    RefPtr(const WeakPtr<T> &other) : _value(other.value()) { retain(); }
    RefPtr(RefPtr<T> &&other) : _value(other._value) { other._value = nullptr; }
    template <typename S, typename = typename std::enable_if<std::is_base_of<T, S>::value>::type>
    RefPtr(const RefPtr<S> &other) : _value(other._value) { retain(); }
    ~RefPtr(void) { release(); }

    void retain() { if (_value) { retain(_value); } }
    void release() {
        if (_value) {
            if (release(_value)) {
                destroy(_value);
            }
            _value = nullptr;
        }
    }
    static void retain(T *value);
    static bool release(T *value);
    static void destroy(T *value);

    T *value() const { return _value; }
    operator bool() const { return value() != nullptr; }
    operator T *() const { return value(); }

    template <typename S>
    bool operator ==(S *other) const { return value() == other; }
    template <typename S>
    bool operator ==(const S *other) const { return value() == other; }
    template <typename S>
    bool operator ==(const RefPtr<S> &other) const { return value() == other.value(); }
    template <typename S>
    bool operator ==(WeakPtr<S> &other) const { return value() == other.value(); }
    template <typename S>
    bool operator ==(const WeakPtr<S> &other) const { return value() == other.value(); }
    template <typename S>
    bool operator !=(S *other) const { return value() != other; }
    template <typename S>
    bool operator !=(const S *other) const { return value() != other; }
    template <typename S>
    bool operator !=(const RefPtr<S> &other) const { return value() != other.value(); }
    template <typename S>
    bool operator !=(const WeakPtr<S> &other) const { return value() != other.value(); }
    template <typename S>
    bool operator <(const S *other) const { return value() < other; }
    template <typename S>
    bool operator <(const RefPtr<S> &other) const { return value() < other.value(); }
    bool operator !() const { return !value(); }

    T &operator *() const { return *value(); }
    T *operator ->() const { return value(); }
    T *operator +(const int num) const { return value() + num; }
    T *operator -(const int num) const { return value() - num; }
    T *operator =(std::nullptr_t) {
        release();
        return nullptr;
    }
    T *operator =(T *other) {
        if (other != value()) {
            release();
            _value = other;
            retain();
        }
        return value();
    }
    T *operator =(const RefPtr<T> &other) {
        if (other.value() != value()) {
            release();
            _value = other._value;
            retain();
        }
        return value();
    }
    T *operator =(const WeakPtr<T> &other) {
        if (other.value() != value()) {
            release();
            _value = other.value();
            retain();
        }
        return value();
    }
    T *operator =(RefPtr<T> &&other) {
        _value = other._value;
        other._value = nullptr;
        return value();
    }
    template <typename S, typename = typename std::enable_if<std::is_base_of<T, S>::value>::type>
    T *operator =(RefPtr<S> &other) {
        if (other.value() != value()) {
            release();
            _value = other._value;
            retain();
        }
        return value();
    }
    template <typename S, typename = typename std::enable_if<std::is_base_of<T, S>::value>::type>
    T* operator =(RefPtr<S>&& other) {
        if (other.value() != value()) {
            release();
            _value = other._value;
            other._value = nullptr;
        } else {
            other.release();
        }
        return value();
    }
    template <typename S, typename = typename std::enable_if<std::is_base_of<T, S>::value>::type>
    T* operator =(S* other) {
        if (other != value()) {
            release();
            _value = other;
            retain();
        }
        return value();
    }

    // as 和 cast 都是强制类型转换，区别在于as会根据虚表做动态类型检查
    // 在cast能保证安全性的场合，应尽量使用cast替代as。
    template <typename S>
    S *as() const // object->as<String>() => (String *)
    { return reinterpret_cast<const void *>(this) ? dynamic_cast<S *>(_value) : nullptr; }

    template <typename S>
    S *cast() const {
        assert(_value == nullptr || as<S>() != nullptr);
        return static_cast<S *>(_value);
    }

    template <typename S>
    S *cast() {
        assert(_value == nullptr || as<S>() != nullptr);
        return static_cast<S *>(_value);
    }

    template <typename S>
    bool is() const { return as<S>() != nullptr; } // object->is<String>() => true || false

};

template <typename T>
void RefPtr<T>::retain(T *value) { value->retain(); }

template <typename T>
bool RefPtr<T>::release(T *value) { return value->release(); }

template <typename T>
void RefPtr<T>::destroy(T *value) { delete value; }

OMP_NAMESPACE_END
