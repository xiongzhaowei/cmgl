﻿//
// Created by 熊朝伟 on 2020-03-18.
//

#include "defines.h"

OMP_RENDER_USING_NAMESPACE

vec4 RenderLayer::backgroundColor() const {
    return _backgroundColor;
}

void RenderLayer::setBackgroundColor(vec4 color) {
    _backgroundColor = color;
}

vec2 RenderLayer::anchorPoint() const {
    return _anchorPoint;
}

void RenderLayer::setAnchorPoint(const vec2 &anchorPoint) {
    _anchorPoint = anchorPoint;
}

vec2 RenderLayer::position() const {
    return _position;
}

void RenderLayer::setPosition(const vec2 &position) {
    _position = position;
}

vec2 RenderLayer::offset() const {
    return _offset;
}

void RenderLayer::setOffset(const vec2 &offset) {
    _offset = offset;
}

vec2 RenderLayer::size() const {
    return _size;
}

void RenderLayer::setSize(const vec2 &size) {
    _size = size;
}

vec3 RenderLayer::rotate() const {
    return _rotate;
}

void RenderLayer::setRotate(const vec3 &rotate) {
    _rotate = rotate;
}

vec2 RenderLayer::scale() const {
    return _scale;
}

void RenderLayer::setScale(const vec2 &scale) {
    _scale = scale;
}

mat4 RenderLayer::transform() const {
    return _transform;
}

void RenderLayer::setTransform(const mat4 &transform) {
    _transform = transform;
}

void RenderLayer::applyTransform(const mat4 &transform) {
    setTransform(transform * _transform);
}

void RenderLayer::applyTransformScale(const vec2 &scale) {
    setTransform(glm::scale(_transform, vec3(scale, 1)));
}

void RenderLayer::applyTransformRotate(const vec3 &rotate) {
    setTransform(mat4_cast(quat(rotate)) * _transform);
}

void RenderLayer::applyTransformTranslate(const vec2 &translate) {
    setTransform(glm::translate(_transform, vec3(translate, 0)));
}

float RenderLayer::alpha() const {
    return _alpha;
}

void RenderLayer::setAlpha(float alpha) {
    _alpha = alpha;
}

bool RenderLayer::visible() const {
    return _visible;
}

void RenderLayer::setVisible(bool visible) {
    _visible = visible;
}

bool RenderLayer::maskToBounds() const {
    return _maskToBounds;
}

void RenderLayer::setMaskToBounds(bool maskToBounds) {
    _maskToBounds = maskToBounds;
}

RefPtr<DataSource> RenderLayer::source() const {
    return _source;
}

void RenderLayer::setSource(RefPtr<DataSource> source) {
    _source = source;
}

void RenderLayer::load(RefPtr<RenderContext> context) {
    if (_source) _source->load(context);
}

void RenderLayer::unload(RefPtr<RenderContext> context) {
    if (_source) _source->unload(context);
}

void RenderLayer::render(RefPtr<RenderContext> context, RefPtr<Framebuffer> framebuffer, const mat4 &globalMatrix) {
    if (!_visible) return;
    if (_alpha < 0.0000001) return; // 内容过于接近全透明，直接忽略
    
    vec4 bkgColor = this->backgroundColor();
    mat4 localMatrix = this->localMatrix(this);
    mat4 clipMatrix = this->clipMatrix(this);
    vec2 size = this->size();
    float alpha = this->alpha();

    if (bkgColor.a > 0) {
        context->draw(Renderer::RECT, framebuffer, globalMatrix, localMatrix, clipMatrix, Renderer::kColorConversionNone, size, alpha, bkgColor);
    }
    if (_source) {
        _source->draw(context, framebuffer, globalMatrix, localMatrix, clipMatrix, size, alpha);
    }
    mat4 matrix = globalMatrix * localMatrix;
    for (RefPtr<RenderLayer> layer : _sublayers) {
        layer->render(context, framebuffer, matrix);
    }
}

RefPtr<RenderLayer> RenderLayer::superlayer() const {
    return _superlayer;
}

void RenderLayer::addLayer(RefPtr<RenderLayer> layer) {
    if (layer->superlayer()) {
        layer->removeFromSuperlayer();
    }
    _sublayers.push_back(layer);
    layer->_superlayer = this;
}

void RenderLayer::removeFromSuperlayer() {
    _superlayer->_sublayers.remove(this);
    _superlayer = nullptr;
}

mat4 RenderLayer::localMatrix(RefPtr<RenderLayer> layer) {
    assert(layer != nullptr);
    assert(layer->visible());

    vec2 anchorPoint    = layer->anchorPoint();
    vec2 position       = layer->position();
    vec2 offset         = layer->offset();
    vec2 size           = layer->size();
    vec3 rotate         = layer->rotate();
    vec2 scale          = layer->scale();
    mat4 transform      = layer->transform();

    // 矩阵定位顺序：
    // 1. 缩放
    // 2. 点平移（移动锚点至原点）
    // 3. 旋转
    // 4. 对象平移
    // 父矩阵在右，子矩阵在左。
    mat4 matrix = glm::scale(identity<mat4>(), vec3(scale, 1));
    matrix = translate(matrix, vec3(-offset - anchorPoint * size, 0));
    matrix = mat4_cast(quat(rotate)) * matrix;
    matrix = translate(identity<mat4>(), vec3(position, 0)) * matrix;
    return transform * matrix;
}

mat4 RenderLayer::clipMatrix(RefPtr<RenderLayer> layer) {
    if (!layer->source()) return glm::identity<mat4>();
    if (!layer->maskToBounds()) return glm::identity<mat4>();
    
    vec2 anchorPoint    = layer->anchorPoint();
    vec2 offset         = layer->offset();
    vec2 size           = layer->size();
    mat4 transform      = layer->transform();
    
    mat4 matrix = translate(identity<mat4>(), vec3(-offset / size - anchorPoint, 0)); // 移动锚点
    matrix = transform * matrix;
    matrix = translate(identity<mat4>(), vec3(offset / size + anchorPoint, 0)) * matrix;
    
    return matrix;
}
