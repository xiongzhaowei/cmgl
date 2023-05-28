
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

	virtual void setNeedsDisplay() = 0;
	virtual void setContent(void* image) = 0;

	virtual void addLayer(Layer* layer) = 0;
	virtual void removeFromSuperlayer() = 0;

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
		Rect frame = this->frame();
		if (x < frame.origin.x || y < frame.origin.y) return nullptr;
		if ((x - frame.origin.x) >= frame.size.width) return nullptr;
		if ((y - frame.origin.y) >= frame.size.height) return nullptr;

		for (auto it = _subviews.rbegin(); it != _subviews.rend(); it++) {
			View* view = *it;
			if (view->isMouseEnabled()) {
				Size scale = view->scale();
				// TODO: 处理很粗糙，没有考虑contentOffset、旋转等复杂情况
				View* subview = view->hitTest((x - frame.origin.x) / scale.width, (y - frame.origin.y) / scale.height);
				if (subview) return subview;
			}
		}

		return isMouseEnabled() ? this : nullptr;
	}
	virtual bool isMouseEnabled() const {
		return _isMouseEnabled;
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

	virtual bool isFocused() const;
	virtual void setFocus();
	virtual void killFocus();

	virtual std::optional<intptr_t> handleMouseEvent(const MouseEvent& event);
	virtual std::optional<intptr_t> handleKeyboardEvent(const KeyboardEvent& event);
};

class Window : public View {
	WeakPtr<View> _captureView;
	WeakPtr<View> _focusView;
public:
	virtual View* hitTest(float x, float y) override;
	virtual View* focusView() const;

	virtual std::optional<intptr_t> handleNativeEvent(const NativeEvent& event);

	friend class View;
};

OMP_UI_NAMESPACE_END
