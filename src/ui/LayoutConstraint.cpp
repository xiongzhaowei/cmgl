#include "defines.h"

OMP_UI_USING_NAMESPACE

float LayoutConstraint::left(RefPtr<View> view) {
    return view->frame().origin.x;
}

float LayoutConstraint::right(RefPtr<View> view) {
    ui::Rect frame = view->frame();
    return frame.origin.x + frame.size.width;
}

float LayoutConstraint::top(RefPtr<View> view) {
    return view->frame().origin.y;
}

float LayoutConstraint::bottom(RefPtr<View> view) {
    ui::Rect frame = view->frame();
    return frame.origin.y + frame.size.height;
}

float LayoutConstraint::width(RefPtr<View> view) {
    return view->frame().size.width;
}

float LayoutConstraint::height(RefPtr<View> view) {
    return view->frame().size.height;
}

float LayoutConstraint::centerX(RefPtr<View> view) {
    ui::Rect frame = view->frame();
    return frame.origin.x + frame.size.width / 2;
}

float LayoutConstraint::centerY(RefPtr<View> view) {
    ui::Rect frame = view->frame();
    return frame.origin.y + frame.size.height / 2;
}

void LayoutConstraint::setLeftRight(RefPtr<View> view, float left, float right) {
    ui::Rect frame = view->frame();
    frame.origin.x = left;
    frame.size.width = right - left;
    view->setFrame(frame);
}

void LayoutConstraint::setLeftCenter(RefPtr<View> view, float left, float center) {
    ui::Rect frame = view->frame();
    frame.origin.x = left;
    frame.size.width = (center - left) * 2;
    view->setFrame(frame);
}

void LayoutConstraint::setLeftWidth(RefPtr<View> view, float left, float width) {
    ui::Rect frame = view->frame();
    frame.origin.x = left;
    frame.size.width = width;
    view->setFrame(frame);
}

void LayoutConstraint::setRightCenter(RefPtr<View> view, float right, float center) {
    ui::Rect frame = view->frame();
    frame.size.width = (right - center) * 2;
    frame.origin.x = right - frame.size.width;
    view->setFrame(frame);
}

void LayoutConstraint::setRightWidth(RefPtr<View> view, float right, float width) {
    ui::Rect frame = view->frame();
    frame.origin.x = right - width;
    frame.size.width = width;
    view->setFrame(frame);
}

void LayoutConstraint::setCenterWidth(RefPtr<View> view, float center, float width) {
    ui::Rect frame = view->frame();
    frame.origin.x = center - width / 2;
    frame.size.width = width;
    view->setFrame(frame);
}

void LayoutConstraint::setTopBottom(RefPtr<View> view, float top, float bottom) {
    ui::Rect frame = view->frame();
    frame.origin.y = top;
    frame.size.height = bottom - top;
    view->setFrame(frame);
}

void LayoutConstraint::setTopCenter(RefPtr<View> view, float top, float center) {
    ui::Rect frame = view->frame();
    frame.origin.y = top;
    frame.size.height = (center - top) * 2;
    view->setFrame(frame);
}

void LayoutConstraint::setTopHeight(RefPtr<View> view, float top, float height) {
    ui::Rect frame = view->frame();
    frame.origin.y = top;
    frame.size.height = height;
    view->setFrame(frame);
}

void LayoutConstraint::setBottomCenter(RefPtr<View> view, float bottom, float center) {
    ui::Rect frame = view->frame();
    frame.size.height = (bottom - center) * 2;
    frame.origin.y = bottom - frame.size.height;
    view->setFrame(frame);
}

void LayoutConstraint::setBottomHeight(RefPtr<View> view, float bottom, float height) {
    ui::Rect frame = view->frame();
    frame.origin.y = bottom - height;
    frame.size.height = height;
    view->setFrame(frame);
}

void LayoutConstraint::setCenterHeight(RefPtr<View> view, float center, float height) {
    ui::Rect frame = view->frame();
    frame.origin.y = center - height / 2;
    frame.size.height = height;
    view->setFrame(frame);
}
