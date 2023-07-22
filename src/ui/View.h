
OMP_UI_NAMESPACE_BEGIN

class Responder : public Object {
public:
	virtual Responder* next() const = 0;
};

class Layer : public Object {
public:
	virtual Color backgroundColor() const = 0;
	virtual void setBackgroundColor(Color color) = 0;

	virtual Rect frame() const = 0;
	virtual void setFrame(Rect frame) = 0;

	virtual Rect bounds() const = 0;
	virtual void setBounds(Rect bounds) = 0;

	virtual Point anchorPoint() const = 0;
	virtual void setAnchorPoint(Point anchor) = 0;

	virtual Point position() const = 0;
	virtual void setPosition(Point position) = 0;

	virtual Size scale() const = 0;
	virtual void setScale(Size scale) = 0;

	virtual Point contentOffset() const = 0;
	virtual void setContentOffset(Point offset) = 0;

	virtual Size contentSize() const = 0;
	virtual void setContentSize(Size size) = 0;

	virtual float rotate() const = 0;
	virtual void setRotate(float rotate) = 0;

	virtual float cornerRadius() const = 0;
	virtual void setCornerRadius(float radius) = 0;

	virtual float shadowOpacity() const = 0;
	virtual void setShadowOpacity(float opacity) = 0;

	virtual float shadowRadius() const = 0;
	virtual void setShadowRadius(float radius) = 0;

	virtual void* content() const = 0;
	virtual void setContent(void* image) = 0;

	virtual void setNeedsDisplay() = 0;

	virtual void addLayer(Layer* layer) = 0;
	virtual void removeFromSuperlayer() = 0;

	virtual void drawText(const std::wstring& text, float fontSize, Color color, TextAlignment textAlignment, TextVerticalAlignment textVerticalAlignment) = 0;
	static Layer* createLayer();
};

class Window;

class View : public Responder {
protected:
	WeakPtr<Responder> _next;
	WeakPtr<View> _parent;
	RefPtr<Layer> _layer = makeLayer();
	std::vector<RefPtr<View>> _subviews;
	bool _isMouseEnabled = true;
	bool _isKeyboardEnabled = true;
	bool _isNeedsLayout = false;
	std::list<std::function<void(const MouseEvent&)>> _onMouseEnterActions;
	std::list<std::function<void(const MouseEvent&)>> _onMouseLeaveActions;
public:
	virtual Layer* makeLayer() { return Layer::createLayer(); }
	virtual Layer* layer() const { return _layer; }

	virtual Window* window() const {
		return _parent ? _parent->window() : nullptr;
	}
	virtual Responder* next() const {
		return _next != nullptr ? _next.value() : _parent;
	}
	virtual View* hitTest(float x, float y) {
		Point local = localPoint(Point(x, y));
		Rect bounds = this->bounds();

		if (local.x < bounds.origin.x || local.y < bounds.origin.y) return nullptr;
		if ((local.x - bounds.origin.x) >= bounds.size.width) return nullptr;
		if ((local.y - bounds.origin.y) >= bounds.size.height) return nullptr;

		for (auto it = _subviews.rbegin(); it != _subviews.rend(); it++) {
			View* view = *it;
			if (view->isMouseEnabled()) {
				View* subview = view->hitTest(x, y);
				if (subview) return subview;
			}
		}

		return isMouseEnabled() ? this : nullptr;
	}
	virtual bool isMouseEnabled() const {
		return _isMouseEnabled;
	}
	virtual void setMouseEnabled(bool enabled) {
		_isMouseEnabled = enabled;
	}
	virtual bool isKeyboardEnabled() const {
		return _isKeyboardEnabled;
	}

	virtual View* superview() const { return _parent; }

	virtual Color backgroundColor() const;
	virtual void setBackgroundColor(Color color);

	virtual Rect frame() const;
	virtual void setFrame(Rect frame);

	virtual Rect bounds() const;
	virtual void setBounds(Rect bounds);

	virtual Point anchorPoint() const;
	virtual void setAnchorPoint(Point anchor);

	virtual Point position() const;
	virtual void setPosition(Point position);

	virtual Size scale() const;
	virtual void setScale(Size scale);

	virtual Point contentOffset() const;
	virtual void setContentOffset(Point offset);

	virtual Size contentSize() const;
	virtual void setContentSize(Size size);

	virtual float rotate() const;
	virtual void setRotate(float rotate);

	virtual void addSubview(View* view);
	virtual void removeFromSuperview();

	virtual void setNeedsLayout();
	virtual void layoutIfNeeded();
	virtual void layoutSubviews();

	virtual bool isFocused() const;
	virtual void setFocus();
	virtual void killFocus();

	virtual void onMouseEnter(const MouseEvent& event);
	virtual void onMouseHover(const MouseEvent& event);
	virtual void onMouseLeave(const MouseEvent& event);
	virtual void onMouseMove(const MouseEvent& event);
	virtual void onMouseWheel(const MouseEvent& event);
	virtual void onMouseDown(const MouseEvent& event);
	virtual void onMouseUp(const MouseEvent& event);
	virtual void onDoubleClick(const MouseEvent& event);

	virtual void onMouseEnter(const std::function<void(const MouseEvent&)>& action);
	virtual void onMouseLeave(const std::function<void(const MouseEvent&)>& action);

	Point localPoint(Point global) const;
};

class Label : public View {
	Color _textColor;
	std::wstring _text;
	float _fontSize;
	TextAlignment _textAlignment = TextAlignment::none;
	TextVerticalAlignment _textVerticalAlignment = TextVerticalAlignment::none;
public:
	Label();

	float fontSize() const;
	virtual void setFontSize(float fontSize);

	virtual std::wstring text() const;
	virtual void setText(const std::wstring& text);

	virtual Color textColor() const;
	virtual void setTextColor(Color color);

	virtual TextAlignment textAlignment() const;
	virtual void setTextAlignment(TextAlignment alignment);

	virtual TextVerticalAlignment textVerticalAlignment() const;
	virtual void setTextVerticalAlignment(TextVerticalAlignment alignment);

	void layoutSubviews() override;
};

class ImageView : public View {
public:
	ImageView();
	virtual RefPtr<Image> image() const;
	virtual void setImage(RefPtr<Image> image);
};

class Button : public View {
protected:
	RefPtr<ImageView> _imageView;
	RefPtr<Image> _stateImages[3];
	bool _hoverState = false;
	bool _pressedState = false;
	std::list<std::function<void()>> _clickedActions;
public:
	enum class State {
		normal,
		hover,
		pressed
	};

	Button();

	void onMouseEnter(const MouseEvent& event) override;
	void onMouseLeave(const MouseEvent& event) override;
	void onMouseDown(const MouseEvent& event) override;
	void onMouseUp(const MouseEvent& event) override;
	void onDoubleClick(const MouseEvent& event) override;

	virtual void setImage(State state, RefPtr<Image> image);
	virtual void onClicked(std::function<void()>);

	void layoutSubviews() override;
};

class Progress : public View {
protected:
	int64_t _maxValue = 0;
	int64_t _minValue = 0;
	int64_t _value = 0;
	RefPtr<ImageView> _foregroundImageView = new ImageView;
	RefPtr<ImageView> _backgroundImageView = new ImageView;
	std::list<std::function<void()>> _onValueChangedActions;
public:
	Progress();

	virtual int64_t maxValue() const;
	virtual void setMaxValue(int64_t value);

	virtual int64_t minValue() const;
	virtual void setMinValue(int64_t value);

	virtual int64_t value() const;
	virtual void setValue(int64_t value);

	virtual RefPtr<ImageView> foregroundImageView();
	virtual RefPtr<ImageView> backgroundImageView();

	virtual void setForegroundImage(RefPtr<Image> image);
	virtual void setBackgroundImage(RefPtr<Image> image);

	void layoutSubviews() override;

	void onMouseUp(const MouseEvent& event) override;
	void onValueChanged(std::function<void()>);
};

class Slider : public Progress {
protected:
	RefPtr<Button> _button = new Button;
	float _progressHeight = 0;
	bool _isDragging = false;
	std::list<std::function<void(int64_t)>> _onDraggingActions;
public:
	Slider();

	virtual RefPtr<Button> button() const;

	virtual float progressHeight() const;
	virtual void setProgressHeight(float height);

	void layoutSubviews() override;
	void onMouseDown(const MouseEvent& event) override;
	void onMouseMove(const MouseEvent& event) override;
	void onMouseUp(const MouseEvent& event) override;

	void onDragging(const std::function<void(int64_t)> action);
};

class Window : public View {
protected:
	WeakPtr<View> _captureView;
	WeakPtr<View> _focusView;
public:
	Window* window() const { return (Window*)this; }
	virtual View* hitTest(float x, float y) override;
	virtual View* focusView() const;
	virtual void setCaptureView(RefPtr<View> view);

	virtual void* handle() const = 0;
	virtual bool create(int32_t width, int32_t height, Window* parent = nullptr, bool isGLESEnabled = false) = 0;
	virtual bool render(std::function<void(render::egl::EGLRenderContext*)> callback) = 0;
	virtual void show() = 0;
	virtual void hide() = 0;
	virtual void close() = 0;
	virtual RefPtr<render::RenderSource> renderLayer() = 0;
	virtual void setNeedsDisplay() = 0;
	virtual void setTitle(const std::wstring& title) = 0;
	virtual std::optional<intptr_t> handleMouseEvent(const MouseEvent& event) = 0;
	virtual std::optional<intptr_t> handleKeyboardEvent(const KeyboardEvent& event) = 0;
	virtual std::optional<intptr_t> handleNativeEvent(const NativeEvent& event) = 0;

	friend class View;
};

class WindowController : public Object {
protected:
	RefPtr<Window> _window;
public:
	WindowController() = default;

	Window* window();

	virtual void load();
	virtual void unload();

	virtual void onInitWindow();
	virtual void onDestroyWindow();

	virtual void layoutWindow();
	virtual std::optional<LRESULT> handleMouseEvent(const MouseEvent& event);
};

OMP_UI_NAMESPACE_END
