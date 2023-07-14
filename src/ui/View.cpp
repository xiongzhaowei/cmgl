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
	setNeedsLayout();
}

Rect View::bounds() const {
	return layer()->bounds();
}

void View::setBounds(Rect bounds) {
	layer()->setBounds(bounds);
	setNeedsLayout();
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

void View::setNeedsLayout() {
	_isNeedsLayout = true;
	if (RefPtr<Window> win = window()) win->setNeedsDisplay();
}

void View::layoutIfNeeded() {
	if (_isNeedsLayout) {
		layoutSubviews();
		_isNeedsLayout = false;
	}
	for (RefPtr<View> view : _subviews) {
		view->layoutIfNeeded();
	}
}

void View::layoutSubviews() {

}

bool View::isFocused() const {
	return window()->focusView() == this;
}

void View::setFocus() {
	window()->_focusView = this;
}

void View::killFocus() {
	window()->_focusView = nullptr;
}

void View::onMouseEnter() {}

void View::onMouseHover() {}

void View::onMouseLeave() {}

void View::onMouseMove(const MouseEvent& event) {}

void View::onMouseWheel(const MouseEvent& event) {}

void View::onMouseDown(const MouseEvent& event) {}

void View::onMouseUp(const MouseEvent& event) {}

void View::onDoubleClick(const MouseEvent& event) {}

Label::Label() : _textColor(0xFF000000), _fontSize(15) {}

float Label::fontSize() const {
	return _fontSize;
}

void Label::setFontSize(float fontSize) {
	_fontSize = fontSize;
	setNeedsLayout();
}

std::wstring Label::text() const {
	return _text;
}

void Label::setText(const std::wstring& text) {
	if (_text != text) {
		_text = text;
		setNeedsLayout();
	}
}

Color Label::textColor() const {
	return _textColor;
}

void Label::setTextColor(Color color) {
	_textColor = color;
	setNeedsLayout();
}

void Label::layoutSubviews() {
	_layer->drawText(_text, _fontSize, _textColor);
}

ImageView::ImageView() {
	_isMouseEnabled = false;
	_isKeyboardEnabled = false;
}

RefPtr<Image> ImageView::image() const {
	return static_cast<Image*>(_layer->content());
}

void ImageView::setImage(RefPtr<Image> image) {
	_layer->setContent(image.value());
	_layer->setContentSize(ui::Size(image->Width(), image->Height()));
	ui::Rect rect = _layer->bounds();
	_layer->setAnchorPoint(ui::Point(0, 0));
	rect.size = ui::Size(image->Width(), image->Height());
	_layer->setBounds(rect);
	_layer->setNeedsDisplay();
}

Button::Button() : _imageView(new ImageView) {
	addSubview(_imageView);
}

void Button::setImage(State state, RefPtr<Image> image) {
	_stateImages[(int)state] = image;
	setNeedsLayout();
}

void Button::onClicked(std::function<void()> action) {
	_clickedActions.push_back(action);
}

void Button::onMouseEnter() {
	if (!_hoverState) {
		_hoverState = true;
		setNeedsLayout();
	}
}

void Button::onMouseLeave() {
	if (_hoverState) {
		_hoverState = false;
		setNeedsLayout();
	}
}

void Button::onMouseDown(const MouseEvent& event) {
	if (!_pressedState) {
		_pressedState = true;
		setNeedsLayout();
	}
}

void Button::onMouseUp(const MouseEvent& event) {
	if (_pressedState) {
		_pressedState = false;
		setNeedsLayout();
	}
	for (auto action : _clickedActions) {
		action();
	}
}

void Button::onDoubleClick(const MouseEvent& event) {
	if (!_pressedState) {
		_pressedState = true;
		setNeedsLayout();
	}
}

void Button::layoutSubviews() {
	State state = _pressedState ? State::pressed : _hoverState ? State::hover : State::normal;
	RefPtr<Image> image = _stateImages[(int)state];
	if (image == nullptr) image = _stateImages[0];
	if (image) _imageView->setImage(image);
	if (Window* wnd = window()) wnd->setNeedsDisplay();
}

Progress::Progress() : _maxValue(0), _minValue(0), _value(0), _foregroundImageView(new ImageView), _backgroundImageView(new ImageView) {
	addSubview(_backgroundImageView);
	addSubview(_foregroundImageView);
}

int64_t Progress::maxValue() const {
	return _maxValue;
}

void Progress::setMaxValue(int64_t value) {
	if (_maxValue != value) {
		_maxValue = value;
		setNeedsLayout();
	}
}

int64_t Progress::minValue() const {
	return _minValue;
}

void Progress::setMinValue(int64_t value) {
	if (_minValue != value) {
		_minValue = value;
		setNeedsLayout();
	}
}

int64_t Progress::value() const {
	return _value;
}

void Progress::setValue(int64_t value) {
	if (_value != value) {
		_value = value;
		setNeedsLayout();
	}
}

RefPtr<ImageView> Progress::foregroundImageView() {
	return _foregroundImageView;
}

RefPtr<ImageView> Progress::backgroundImageView() {
	return _backgroundImageView;
}

void Progress::setForegroundImage(RefPtr<Image> image) {
	_foregroundImageView->setImage(image);
	setNeedsLayout();
}

void Progress::setBackgroundImage(RefPtr<Image> image) {
	_backgroundImageView->setImage(image);
	setNeedsLayout();
}

void Progress::layoutSubviews() {
	ui::Rect frame = bounds();
	_backgroundImageView->setFrame(frame);
	frame.size.width *= float(_value - _minValue) / float(_maxValue - _minValue);
	_foregroundImageView->setFrame(frame);
}

void Slider::onMouseUp(const MouseEvent& event) {
	Progress::onMouseUp(event);

	Point pt = event.pt;
	for (RefPtr<View> view = this; view->superview(); view = view->superview()) {
		Rect frame = view->frame();
		Size scale = view->scale();
		pt.x *= scale.width;
		pt.y *= scale.height;
		pt.x -= frame.origin.x;
		pt.y -= frame.origin.y;
	}
	int64_t value = pt.x / frame().size.width * (_maxValue - _minValue) + _minValue;
	if (_value != value) {
		_value = value;
		for (auto action : _valueChangedActions) {
			action();
		}
	}
}

void Slider::onValueChanged(std::function<void()> action) {
	_valueChangedActions.push_back(action);
}

View* Window::hitTest(float x, float y) {
	if (_captureView) return _captureView;

	return View::hitTest(x, y);
}

View* Window::focusView() const {
	return _focusView;
}

Window* WindowController::window() {
	return _window;
}

void WindowController::load() {

}

void WindowController::unload() {

}

void WindowController::onInitWindow() {

}

void WindowController::onDestroyWindow() {

}

void WindowController::layoutWindow() {

}
