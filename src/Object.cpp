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

WeakOwner::WeakOwner() {

}

bool WeakOwner::valid() const {
    return _valid;
}

void WeakOwner::clear() {
    _valid = false;
}

WeakSupported::WeakSupported() : _weak(new WeakOwner()) {
}

WeakSupported::~WeakSupported() {
    _weak->clear();
}

WeakOwner* WeakSupported::weak() const {
    return _weak;
}

RefPtr<Object> Interface::asObject() const {
    return dynamic_cast<Object*>(const_cast<Interface*>(this));
}
