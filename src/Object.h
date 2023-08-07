//
// Created by 熊朝伟 on 2020-03-16.
//

#pragma once

OMP_NAMESPACE_BEGIN

class RefCounted {
    mutable std::atomic<int> _refCount;
public:
    RefCounted();
    ~RefCounted();
    void retain() const;
    bool release() const;
};

class Object;

class WeakOwner final : public RefCounted {
    std::atomic_bool _valid = true;
    WeakOwner(const WeakOwner &) = delete;
    WeakOwner(const WeakOwner &&) = delete;
public:
    WeakOwner();
    bool valid() const;
    void clear();
};

class WeakSupported {
    RefPtr<WeakOwner> _weak;
    WeakSupported(const WeakSupported&) = delete;
    WeakSupported(const WeakSupported&&) = delete;
public:
    WeakSupported();
    ~WeakSupported();

    WeakOwner* weak() const;
};

class Object : public virtual RefCounted, public virtual WeakSupported {
    Object(const Object &) = delete;
    Object(const Object &&) = delete;
public:
    Object() = default;
    virtual ~Object() = default;
};

class Interface : public virtual RefCounted, public virtual WeakSupported {
    Interface(const Interface&) = delete;
    Interface(const Interface&&) = delete;
public:
    Interface() = default;
    virtual ~Interface() = default;
    RefPtr<Object> asObject() const;
};

OMP_NAMESPACE_END
