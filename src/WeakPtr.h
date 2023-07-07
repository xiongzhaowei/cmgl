//
// Created by 熊朝伟 on 2020-03-16.
//

#pragma once

OMP_NAMESPACE_BEGIN

template <typename T>
class WeakPtr {
    RefPtr<WeakOwner> _weak;
    T* _value;
public:
    WeakPtr(std::nullptr_t = nullptr) : _weak(nullptr), _value(nullptr) {}
    WeakPtr(T *other) : _weak(other ? other->weak() : nullptr), _value(other) {}
    WeakPtr(const RefPtr<T> &other) : _weak(other ? other->weak() : nullptr), _value(other) {}
    WeakPtr(const WeakPtr<T> &other) : _weak(other._weak), _value(other) {}
    WeakPtr(WeakPtr<T> &&other) : _weak(other._weak), _value(other) {
        other._weak = nullptr;
        other._value = nullptr;
    }
    
    T *value() const { return _weak != nullptr && _weak->valid() ? _value : nullptr; }
    operator bool() const { return value() != nullptr; }
    operator T *() const { return value(); }
    
    bool operator ==(const T *other) const { return value() == other; }
    bool operator ==(const RefPtr<T> &other) const { return value() == other.value(); }
    bool operator ==(decltype(nullptr)) const { return value() == nullptr; }
    bool operator !=(const T *other) const { return value() != other; }
    bool operator !=(const RefPtr<T>& other) const { return value() != other.value(); }
    bool operator !=(decltype(nullptr)) const { return value() != nullptr; }
    bool operator !() const { return !value(); }
    
    T &operator *() const { return *value(); }
    T *operator ->() const { return value(); }
    T *operator +(const int num) const { return value() + num; }
    T *operator -(const int num) const { return value() - num; }
    T *operator =(std::nullptr_t) {
        _weak = nullptr;
        _value = nullptr;
        return nullptr;
    }
    T *operator =(T *other) {
        _weak = other ? other->weak() : nullptr;
        _value = other;
        return value();
    }
    T *operator =(const RefPtr<T> &other) {
        _weak = other ? other->weak() : nullptr;
        _value = other;
        return value();
    }
    T *operator =(const WeakPtr<T> &other) {
        _weak = other._weak;
        _value = other;
        return value();
    }
    T *operator =(WeakPtr<T> &&other) {
        _weak = other._weak;
        _value = other;
        other._weak = nullptr;
        other._value = nullptr;
        return value();
    }
    
    template <typename S>
    S *as() const // object->as<String>() => (String *)
    { return reinterpret_cast<const void *>(this) ? dynamic_cast<S *>(value()) : nullptr; }
    
    template <typename S>
    S *cast() const {
        assert(value() == nullptr || as<S>() != nullptr);
        return static_cast<S *>(value());
    }

    template <typename S>
    S *cast() {
        assert(value() == nullptr || as<S>() != nullptr);
        return static_cast<S *>(value());
    }

    template <typename S>
    bool is() const { return as<S>() != nullptr; } // object->is<String>() => true || false
};

OMP_NAMESPACE_END
