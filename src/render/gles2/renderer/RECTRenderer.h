﻿//
//  RECTRenderer.h
//  omrender
//
//  Created by 熊朝伟 on 2023/8/2.
//

OMP_RENDER_GLES2_NAMESPACE_BEGIN

struct RECTRenderer : public Renderer {

    void load(RefPtr<RenderContext> context) override;
    void unload(RefPtr<RenderContext> context) override;
    void draw(
        RefPtr<RenderContext> context,
        RefPtr<Framebuffer> framebuffer,
        const mat4 &globalMatrix,
        const mat4 &localMatrix,
        const mat4 &clipMatrix,
        const mat4 &colorConversion,
        const vec2 &size,
        GLfloat alpha,
        va_list list
    ) override;

protected:
    GLint _colorLocation = 0;
    GLint _positionLocation = 0;
    GLint _globalMatrixLocation = 0;
    GLint _localMatrixLocation = 0;
    GLint _clipMatrixLocation = 0;
    GLint _sizeLocation = 0;
    GLint _alphaLocation = 0;
    GLint _colorConversionLocation = 0;
    RefPtr<Program> _program;
};

OMP_RENDER_GLES2_NAMESPACE_END