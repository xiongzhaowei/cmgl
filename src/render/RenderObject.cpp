//
//  RenderObject.cpp
//  omrender
//
//  Created by 熊朝伟 on 2020/4/28.
//

#include "defines.h"

OMP_RENDER_USING_NAMESPACE

vec2 RenderObject::size() const {
    return _size;
}

bool RenderObject::hitTest(float x, float y) {
    return false;
}

void RenderObject::resize(float width, float height) {
    _size = vec2(width, height);
}

bool RenderObject::init() {
    return true;
}

EventController &RenderObject::events() {
    return _events;
}
