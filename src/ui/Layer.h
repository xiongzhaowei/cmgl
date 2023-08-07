
#pragma once

OMP_UI_NAMESPACE_BEGIN

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

	virtual Object* content() const = 0;
	virtual void setContent(Object* image) = 0;

	virtual bool hidden() const = 0;
	virtual void setHidden(bool hidden) = 0;

	virtual void setNeedsDisplay() = 0;

	virtual void addLayer(Layer* layer) = 0;
	virtual void removeFromSuperlayer() = 0;

	virtual void drawText(const std::wstring& text, float fontSize, Color color, TextAlignment textAlignment, TextVerticalAlignment textVerticalAlignment) = 0;
	static Layer* createLayer();
};

class EGLLayer : public Layer {
	RefPtr<render::RenderLayer> _layer;
public:
	EGLLayer();
	RefPtr<render::RenderLayer> layer() const;

	Color backgroundColor() const override;
	void setBackgroundColor(Color color) override;

	Rect frame() const override;
	void setFrame(Rect frame) override;

	Rect bounds() const override;
	void setBounds(Rect bounds) override;

	Point anchorPoint() const override;
	void setAnchorPoint(Point anchor) override;

	Point position() const override;
	void setPosition(Point position) override;

	Size scale() const override;
	void setScale(Size scale) override;

	Point contentOffset() const override;
	void setContentOffset(Point offset) override;

	Size contentSize() const override;
	void setContentSize(Size size) override;

	float rotate() const override;
	void setRotate(float rotate) override;

	float cornerRadius() const override;
	void setCornerRadius(float radius) override;

	float shadowOpacity() const override;
	void setShadowOpacity(float opacity) override;

	float shadowRadius() const override;
	void setShadowRadius(float radius) override;

	Object* content() const override;
	void setContent(Object* image) override;

	bool hidden() const override;
	void setHidden(bool hidden) override;

	void setNeedsDisplay() override;

	void addLayer(Layer* layer) override;
	void removeFromSuperlayer() override;

	void drawText(const std::wstring& text, float fontSize, Color color, TextAlignment textAlignment, TextVerticalAlignment textVerticalAlignment) override;
};

OMP_UI_NAMESPACE_END
