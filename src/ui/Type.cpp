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

Color::Color(uint32_t color) : argb(color) {
}

MouseEvent::MouseEvent(const NativeEvent& native, float x, float y) : native(native), pt(x, y) {
}

static void localMatrix(const View* view, glm::mat4& matrix) {
	if (view->superview() == nullptr) {
		matrix = glm::identity<glm::mat4>();
	} else {
		localMatrix(view->superview(), matrix);
	}
	constexpr float pi = 3.14159265358979323846264338327950288f;
	ui::Rect bounds = view->bounds();
	ui::Point anchorPoint = view->anchorPoint();
	ui::Point position = view->position();
	ui::Size scale = view->scale();
	float rotate = view->rotate();

	matrix = glm::translate(matrix, glm::vec3(position.x, position.y, 0));
	matrix = glm::rotate(matrix, rotate * pi / 180, glm::vec3(0, 0, 1));
	matrix = glm::scale(matrix, glm::vec3(scale.width, scale.height, 1));
	matrix = glm::translate(matrix, glm::vec3(-(bounds.size.width * anchorPoint.x + bounds.origin.x), -(bounds.size.height * anchorPoint.y + bounds.origin.y), 0));
}

Point MouseEvent::localPoint(RefPtr<View> view) const {
	glm::mat4 matrix;
	localMatrix(view, matrix);
	glm::vec4 local = matrix / glm::vec4(pt.x, pt.y, 0, 1);
	return Point(local.x, local.y);
}

Point View::localPoint(Point global) const {
	glm::mat4 matrix;
	localMatrix(this, matrix);
	glm::vec4 local = matrix / glm::vec4(global.x, global.y, 0, 1);
	return Point(local.x, local.y);
}

KeyboardEvent::KeyboardEvent(const NativeEvent& native) : native(native) {
}
