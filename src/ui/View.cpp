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

void View::onMouseEnter(const MouseEvent& event) {
	for (auto action : _onMouseEnterActions) {
		action(event);
	}
}

void View::onMouseHover(const MouseEvent& event) {}

void View::onMouseLeave(const MouseEvent& event) {
	for (auto action : _onMouseLeaveActions) {
		action(event);
	}
}

void View::onMouseMove(const MouseEvent& event) {}

void View::onMouseWheel(const MouseEvent& event) {}

void View::onMouseDown(const MouseEvent& event) {}

void View::onMouseUp(const MouseEvent& event) {}

void View::onDoubleClick(const MouseEvent& event) {}

void View::onMouseEnter(const std::function<void(const MouseEvent&)>& action) {
	_onMouseEnterActions.push_back(action);
}

void View::onMouseLeave(const std::function<void(const MouseEvent&)>& action) {
	_onMouseLeaveActions.push_back(action);
}

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

TextAlignment Label::textAlignment() const {
	return _textAlignment;
}

void Label::setTextAlignment(TextAlignment alignment) {
	_textAlignment = alignment;
}

TextVerticalAlignment Label::textVerticalAlignment() const {
	return _textVerticalAlignment;
}

void Label::setTextVerticalAlignment(TextVerticalAlignment alignment) {
	_textVerticalAlignment = alignment;
}

void Label::layoutSubviews() {
	_layer->drawText(_text, _fontSize, _textColor, _textAlignment, _textVerticalAlignment);
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

void Button::onMouseEnter(const MouseEvent& event) {
	if (!_hoverState) {
		_hoverState = true;
		setNeedsLayout();
	}
	View::onMouseEnter(event);
}

void Button::onMouseLeave(const MouseEvent& event) {
	if (_hoverState) {
		_hoverState = false;
		setNeedsLayout();
	}
	View::onMouseLeave(event);
}

void Button::onMouseDown(const MouseEvent& event) {
	View::onMouseDown(event);
	if (!_pressedState) {
		_pressedState = true;
		setNeedsLayout();
	}
}

void Button::onMouseUp(const MouseEvent& event) {
	View::onMouseUp(event);
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
	if (image) {
		_imageView->setImage(image);
		_imageView->setScale(ui::Size(image->Width() / frame().size.width, image->Height() / frame().size.height));
	}
	if (Window* wnd = window()) wnd->setNeedsDisplay();
}

Progress::Progress() {
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

void Progress::onMouseUp(const MouseEvent& event) {
	View::onMouseUp(event);

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
		for (auto action : _onValueChangedActions) {
			action();
		}
	}
}

void Progress::onValueChanged(std::function<void()> action) {
	_onValueChangedActions.push_back(action);
}

Slider::Slider() {
	_button->setMouseEnabled(false);
	addSubview(_button);
}

RefPtr<Button> Slider::button() const {
	return _button;
}

float Slider::progressHeight() const {
	return _progressHeight;
}

void Slider::setProgressHeight(float height) {
	_progressHeight = height;
}

void Slider::layoutSubviews() {
	float progress = float(_value - _minValue) / float(_maxValue - _minValue);

	ui::Rect frame = _button->frame();
	if (!_isDragging) {
		frame.origin.x = (LayoutConstraint::width(this) - frame.size.width) * progress;
		frame.origin.y = (LayoutConstraint::height(this) - frame.size.height) / 2;
		_button->setFrame(frame);
	}

	LayoutConstraint::setLeftWidth(_backgroundImageView, frame.size.width / 2, LayoutConstraint::width(this) - frame.size.width);
	LayoutConstraint::setLeftWidth(_foregroundImageView, frame.size.width / 2, (LayoutConstraint::width(this) - frame.size.width) * progress);
	LayoutConstraint::setCenterHeight(_backgroundImageView, LayoutConstraint::height(this) / 2, _progressHeight);
	LayoutConstraint::setCenterHeight(_foregroundImageView, LayoutConstraint::height(this) / 2, _progressHeight);
}

void Slider::onMouseDown(const MouseEvent& event) {
	Progress::onMouseDown(event);
	_isDragging = true;
	window()->setCaptureView(this);
}

void Slider::onMouseMove(const MouseEvent& event) {
	if (_isDragging) {
		Point pt = event.localPoint(this);
		float progress;
		if (pt.x < _button->frame().size.width / 2) {
			progress = 0;
		} else if (pt.x > (bounds().size.width - _button->frame().size.width / 2)) {
			progress = 1;
		} else {
			progress = (pt.x - _button->frame().size.width / 2) / (bounds().size.width - _button->frame().size.width);
		}
		int64_t value = progress * (_maxValue - _minValue) + _minValue;

		progress = float(value - _minValue) / float(_maxValue - _minValue);

		ui::Rect frame = _button->frame();
		frame.origin.x = (LayoutConstraint::width(this) - frame.size.width) * progress;
		frame.origin.y = (LayoutConstraint::height(this) - frame.size.height) / 2;
		_button->setFrame(frame);

		for (auto action : _onDraggingActions) {
			action(value);
		}
	}
}

void Slider::onMouseUp(const MouseEvent& event) {
	Progress::onMouseUp(event);
	_isDragging = false;
	window()->setCaptureView(nullptr);

	Point pt = event.localPoint(this);
	float progress;
	if (pt.x < _button->frame().size.width / 2) {
		progress = 0;
	} else if (pt.x > (bounds().size.width - _button->frame().size.width / 2)) {
		progress = 1;
	} else {
		progress = (pt.x - _button->frame().size.width / 2) / (bounds().size.width - _button->frame().size.width);
	}
	int64_t value = progress * (_maxValue - _minValue) + _minValue;
	if (_value != value) {
		_value = value;
		for (auto action : _onValueChangedActions) {
			action();
		}
		setNeedsLayout();
	}
}

void Slider::onDragging(const std::function<void(int64_t)> action) {
	_onDraggingActions.push_back(action);
}

View* Window::hitTest(float x, float y) {
	if (_captureView) return _captureView;

	return View::hitTest(x, y);
}

View* Window::focusView() const {
	return _focusView;
}

void Window::setCaptureView(RefPtr<View> view) {
	_captureView = view;
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

std::optional<LRESULT> WindowController::handleMouseEvent(const MouseEvent& event) {
	return std::nullopt;
}
