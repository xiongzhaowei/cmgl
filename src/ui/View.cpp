#include "windows/defines.h"
#include <windowsx.h>

OMP_UI_USING_NAMESPACE

Layer* Layer::createLayer() {
	return new OMP_UI_WINDOWS_NAMESPACE_PREFIX gdiplus::Layer;
}

Color View::backgroundColor() const {
	return layer()->backgroundColor();
}

void View::setBackgroundColor(Color color) {
	layer()->setBackgroundColor(color);
}

Rect View::frame() const {
	return layer()->frame();
}

void View::setFrame(Rect frame) {
	layer()->setFrame(frame);
}

Rect View::bounds() const {
	return layer()->bounds();
}

void View::setBounds(Rect bounds) {
	layer()->setBounds(bounds);
}

Point View::anchorPoint() const {
	return layer()->anchorPoint();
}

void View::setAnchorPoint(Point anchor) {
	layer()->setAnchorPoint(anchor);
}

Point View::position() const {
	return layer()->position();
}

void View::setPosition(Point position) {
	layer()->setPosition(position);
}

Size View::scale() const {
	return layer()->scale();
}

void View::setScale(Size scale) {
	layer()->setScale(scale);
}

Point View::contentOffset() const {
	return layer()->contentOffset();
}

void View::setContentOffset(Point offset) {
	layer()->setContentOffset(offset);
}

Size View::contentSize() const {
	return layer()->contentSize();
}

void View::setContentSize(Size size) {
	layer()->setContentSize(size);
}

float View::rotate() const {
	return layer()->rotate();
}

void View::setRotate(float rotate) {
	layer()->setRotate(rotate);
}

void View::addSubview(View* view) {
	assert(view != nullptr);
	view->removeFromSuperview();
	_subviews.push_back(view);
	view->_parent = this;
	layer()->addLayer(view->layer());
}

void View::removeFromSuperview() {
	if (_parent) {
		for (auto it = _parent->_subviews.begin(); it != _parent->_subviews.end(); it++) {
			if (*it == this) {
				_parent->_subviews.erase(it);
				break;
			}
		}
		_parent = nullptr;
		layer()->removeFromSuperlayer();
	}
}

bool View::isFocused() const {
	return window()->_focusView == this;
}

void View::setFocus() {
	window()->_focusView = this;
}

void View::killFocus() {
	window()->_focusView = nullptr;
}

std::optional<intptr_t> View::handleMouseEvent(const MouseEvent& event) {
	return std::nullopt;
}

std::optional<intptr_t> View::handleKeyboardEvent(const KeyboardEvent& event) {
	return std::nullopt;
}

View* Window::hitTest(float x, float y) {
	if (_captureView) return _captureView;

	return View::hitTest(x, y);
}

View* Window::focusView() const {
	return _focusView;
}

std::optional<intptr_t> Window::handleNativeEvent(const NativeEvent& event) {
	if (event.message >= WM_MOUSEFIRST && event.message <= WM_MOUSELAST) {
		ui::Size scale = this->scale();
		float x = GET_X_LPARAM(event.lParam) / scale.width;
		float y = GET_Y_LPARAM(event.lParam) / scale.height;
		View* view = hitTest(x, y);
		if (view) {
			MouseEvent mouse(event);
			return view->handleMouseEvent(mouse);
		}
	} else if (event.message >= WM_KEYFIRST && event.message <= WM_KEYLAST) {
		View* view = focusView();
		if (view) {
			KeyboardEvent keyboard(event);
			return view->handleKeyboardEvent(keyboard);
		}
	}
	return std::nullopt;
}
