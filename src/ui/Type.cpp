//
//  Thread.cpp
//  omplayer
//
//  Created by 熊朝伟 on 2023/5/11.
//

#include "defines.h"

OMP_UI_USING_NAMESPACE

Point::Point() : x(0), y(0) {
}

Point::Point(float x, float y) : x(x), y(y) {
}

Size::Size() : width(0), height(0) {
}

Size::Size(float width, float height) : width(width), height(height) {
}

Rect::Rect() : origin(0, 0), size(0, 0) {
}

Rect::Rect(float x, float y, float width, float height) : origin(x, y), size(width, height) {
}


MouseEvent::MouseEvent(const NativeEvent& native) : native(native) {
}

KeyboardEvent::KeyboardEvent(const NativeEvent& native) : native(native) {
}
