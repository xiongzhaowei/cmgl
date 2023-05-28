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

class WeakOwner : public virtual RefCounted {
    Object *_value;
    WeakOwner(const WeakOwner &) = delete;
    WeakOwner(const WeakOwner &&) = delete;
public:
    WeakOwner(Object *value);
    Object *value();
    Object *value() const;

    void clear();
};

class Object : public virtual RefCounted {
    WeakOwner *_weakOwner;
    Object(const Object &) = delete;
    Object(const Object &&) = delete;
public:
    Object();
    virtual ~Object();

    WeakOwner *weakOwner() const;
};

OMP_NAMESPACE_END
