
#pragma once

OMP_UI_NAMESPACE_BEGIN

enum class LayoutAttribute {
	none,
	left,
	right,
	top,
	bottom,
	leading,
	trailing,
	width,
	height,
	centerX,
	centerY,
};

class LayoutConstraint : public Object {
public:
	virtual RefPtr<View> firstItem() const = 0;
	virtual RefPtr<View> secondItem() const = 0;
	virtual LayoutAttribute firstAttribute() const = 0;
	virtual LayoutAttribute secondAttribute() const = 0;
	virtual double multiplier() const = 0;
	virtual double constant() const = 0;

	static float get(RefPtr<View> view, LayoutAttribute attribute);

	static float left(RefPtr<View> view);
	static float right(RefPtr<View> view);
	static float top(RefPtr<View> view);
	static float bottom(RefPtr<View> view);
	static float width(RefPtr<View> view);
	static float height(RefPtr<View> view);
	static float centerX(RefPtr<View> view);
	static float centerY(RefPtr<View> view);
	static void setLeftRight(RefPtr<View> view, float left, float right);
	static void setLeftCenter(RefPtr<View> view, float left, float center);
	static void setLeftWidth(RefPtr<View> view, float left, float width);
	static void setRightCenter(RefPtr<View> view, float right, float center);
	static void setRightWidth(RefPtr<View> view, float right, float width);
	static void setCenterWidth(RefPtr<View> view, float center, float width);
	static void setTopBottom(RefPtr<View> view, float top, float bottom);
	static void setTopCenter(RefPtr<View> view, float top, float center);
	static void setTopHeight(RefPtr<View> view, float top, float height);
	static void setBottomCenter(RefPtr<View> view, float bottom, float center);
	static void setBottomHeight(RefPtr<View> view, float bottom, float height);
	static void setCenterHeight(RefPtr<View> view, float center, float height);
};

OMP_UI_NAMESPACE_END
