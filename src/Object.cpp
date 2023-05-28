//
// Created by 熊朝伟 on 2020-03-16.
//

#include "defines.h"

OMP_USING_NAMESPACE

RefCounted::RefCounted() : _refCount(0) {

}

RefCounted::~RefCounted() {

}

void RefCounted::retain() const {
    _refCount++;
}

bool RefCounted::release() const {
    return --_refCount <= 0;
}

WeakOwner::WeakOwner(Object *value) : _value(value) {

}

Object *WeakOwner::value() {
    return _value;
}

Object *WeakOwner::value() const {
    return _value;
}

void WeakOwner::clear() {
    _value = nullptr;
}

Object::Object() : _weakOwner(new WeakOwner(this)) {
    _weakOwner->retain();
}

Object::~Object() {
    _weakOwner->clear();
    _weakOwner->release();
}

WeakOwner *Object::weakOwner() const {
    return _weakOwner;
}
