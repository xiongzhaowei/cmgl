#include "windows/defines.h"
#include "windows/render/defines.h"

OMP_UI_USING_NAMESPACE

Layer* Layer::createLayer() {
	//return new EGLLayer;
	return new OMP_UI_WINDOWS_NAMESPACE_PREFIX gdiplus::Layer;
}

EGLLayer::EGLLayer() : _layer(new render::RenderLayer) {
}

RefPtr<render::RenderLayer> EGLLayer::layer() const {
	return _layer;
}

Color EGLLayer::backgroundColor() const {
	glm::vec4 color = _layer->backgroundColor();
	color *= 255;
	Color result(0);
	result.a = color.a;
	result.r = color.r;
	result.g = color.g;
	result.b = color.b;
	return result;
}

void EGLLayer::setBackgroundColor(Color color)  {
	glm::vec4 value = glm::vec4(color.r, color.g, color.b, color.a);
	value /= 255.0;
	_layer->setBackgroundColor(value);
}

Rect EGLLayer::frame() const  {
	glm::vec2 size = _layer->size() / _layer->scale();
	glm::vec2 position = _layer->position() - size * _layer->anchorPoint();
	return Rect(position.x, position.y, size.x, size.y);
}

void EGLLayer::setFrame(Rect frame)  {
	glm::vec2 offset = _layer->offset();
	glm::vec2 scale = _layer->scale();
	glm::vec2 anchor = _layer->anchorPoint();

	Rect bounds = Rect(offset.x, offset.y, frame.size.width * scale.x, frame.size.height * scale.y);
	setBounds(bounds);

	Point position = Point{
		frame.origin.x + frame.size.width * anchor.x,
		frame.origin.y + frame.size.height * anchor.y,
	};
	setPosition(position);
}

Rect EGLLayer::bounds() const  {
	glm::vec2 offset = _layer->offset();
	glm::vec2 size = _layer->size();
	return Rect(offset.x, offset.y, size.x, size.y);
}

void EGLLayer::setBounds(Rect bounds)  {
	setContentOffset(bounds.origin);
	setContentSize(bounds.size);
}

Point EGLLayer::anchorPoint() const  {
	glm::vec2 anchor = _layer->anchorPoint();
	return Point(anchor.x, anchor.y);
}

void EGLLayer::setAnchorPoint(Point anchor)  {
	_layer->setAnchorPoint(glm::vec2(anchor.x, anchor.y));
}

Point EGLLayer::position() const  {
	glm::vec2 pos = _layer->position();
	return Point(pos.x, pos.y);
}

void EGLLayer::setPosition(Point position)  {
	_layer->setPosition(glm::vec2(position.x, position.y));
}

Size EGLLayer::scale() const  {
	glm::vec2 scale = _layer->scale();
	return Size(1.0 / scale.x, 1.0 / scale.y);
}

void EGLLayer::setScale(Size scale)  {
	_layer->setScale(glm::vec2(1.0 / scale.width, 1.0 / scale.height));
}

Point EGLLayer::contentOffset() const  {
	glm::vec2 offset = _layer->offset();
	return Point(offset.x, offset.y);
}

void EGLLayer::setContentOffset(Point offset)  {
	_layer->setOffset(glm::vec2(offset.x, offset.y));
}

Size EGLLayer::contentSize() const  {
	glm::vec2 size = _layer->size();
	return Size(size.x, size.y);
}

void EGLLayer::setContentSize(Size size)  {
	_layer->setSize(glm::vec2(size.width, size.height));
}

static constexpr long double pi = 3.14159265358979323846264338327950288;

float EGLLayer::rotate() const  {
	return _layer->rotate().z * 180.0 / pi;
}

void EGLLayer::setRotate(float rotate)  {
	glm::vec3 r = _layer->rotate();
	r.z = rotate * pi / 180.0;;
	_layer->setRotate(r);
}

float EGLLayer::cornerRadius() const  {
	return 0;
}

void EGLLayer::setCornerRadius(float radius)  {

}

float EGLLayer::shadowOpacity() const  {
	return 0;
}

void EGLLayer::setShadowOpacity(float opacity)  {

}

float EGLLayer::shadowRadius() const  {
	return 0;
}

void EGLLayer::setShadowRadius(float radius)  {

}

Object* EGLLayer::content() const  {
	return _layer->source().value();
}

void EGLLayer::setContent(Object* image)  {
	if (RefPtr<windows::Image> img = RefPtr<Object>(image).as<windows::Image>()) {
		_layer->setSource(img->renderObject());
	}
	if (RefPtr<windows::ImageDataSource> dataSource = RefPtr<Object>(image).as<windows::ImageDataSource>()) {
		_layer->setSource(dataSource);
	}
}

bool EGLLayer::hidden() const {
	return !_layer->visible();
}

void EGLLayer::setHidden(bool hidden) {
	_layer->setVisible(!hidden);
}

void EGLLayer::setNeedsDisplay() {

}

void EGLLayer::addLayer(Layer* layer) {
	_layer->addLayer(static_cast<EGLLayer*>(layer)->_layer);
}

void EGLLayer::removeFromSuperlayer() {
	_layer->removeFromSuperlayer();
}

void EGLLayer::drawText(const std::wstring& text, float fontSize, Color color, TextAlignment textAlignment, TextVerticalAlignment textVerticalAlignment) {

}
