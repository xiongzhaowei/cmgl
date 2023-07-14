//
// Created by 熊朝伟 on 2020-03-18.
//

#pragma once

OMP_RENDER_NAMESPACE_BEGIN

/// RenderLayer主要负责内容布局，负责将DataSource提供的内容绘制到指定区域。
struct RenderLayer : public RenderSource {
    virtual bool enabled() const;
    virtual void setEnabled(bool enabled);

    /// 锚点位置，取值范围0-1，决定旋转的圆心，但不会改变原点的位置。
    virtual vec2 anchorPoint() const;
    virtual void setAnchorPoint(const vec2 &anchorPoint);

    /// 当前layer基于父坐标系的所在位置。
    virtual vec2 position() const;
    virtual void setPosition(const vec2 &position);

    /// 基于layer自身的坐标系，展示内容的偏移位置。
    virtual vec2 offset() const;
    virtual void setOffset(const vec2 &offset);

    /// 当前layer的宽高信息。
    virtual vec2 size() const override;
    virtual void setSize(const vec2 &size);

    /// 当前layer基于父坐标系的旋转方位，基于欧拉角旋转。
    virtual vec3 rotate() const;
    virtual void setRotate(const vec3 &rotate);

    /// 当前layer基于父坐标系的缩放比例，数值越大，尺寸越大。
    virtual vec2 scale() const;
    virtual void setScale(const vec2 &scale);

    /// 当前layer基于自身的坐标系，展示内容的变换矩阵，内容的变换不会改变裁切边框的位置。
    virtual mat4 transform() const;
    virtual void setTransform(const mat4 &transform);
    void applyTransform(const mat4 &transform);
    void applyTransformScale(const vec2 &scale);
    void applyTransformRotate(const vec3 &rotate);
    void applyTransformTranslate(const vec2 &translate);

    /// 当前alpha的透明度，取值范围0-1，0表示完全透明，1表示完全不透明。
    virtual float alpha() const;
    virtual void setAlpha(float alpha);

    /// 当前layer是否可见。
    virtual bool visible() const;
    virtual void setVisible(bool visible);

    /// 是否需要对layer的内容进行裁切，只保留边框内的内容展示。
    virtual bool maskToBounds() const;
    virtual void setMaskToBounds(bool maskToBounds);

    /// 当前layer所绘制的数据源。
    virtual RefPtr<DataSource> source() const;
    virtual void setSource(RefPtr<DataSource> source);

    /// 当前layer的初始化、反初始化事件，不需要手动调用。
    virtual void load(RefPtr<RenderContext> context) override;
    virtual void unload(RefPtr<RenderContext> context) override;

    /// 当前layer需要渲染时触发，执行绘制操作。
    virtual void render(RefPtr<RenderContext> context, RefPtr<Framebuffer> framebuffer, const mat4 &globalMatrix) override;

    static mat4 localMatrix(RefPtr<RenderLayer> layer);
    static mat4 clipMatrix(RefPtr<RenderLayer> layer);
protected:
    vec2 _anchorPoint   = vec2(0, 0);
    vec2 _position      = vec2(0, 0);
    vec2 _offset        = vec2(0, 0);
    vec2 _size          = vec2(0, 0);
    vec3 _rotate        = vec3(0, 0, 0);
    vec2 _scale         = vec2(1, 1);
    mat4 _transform     = glm::identity<mat4>();
    float _alpha        = 1;
    bool _maskToBounds  = true;
    bool _visible       = true;
    bool _enabled       = true;
    RefPtr<DataSource> _source;
    std::vector<RefPtr<RenderLayer>> _sublayers;
};

OMP_RENDER_NAMESPACE_END
