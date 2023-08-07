﻿//
//  RGBARenderer.cpp
//  omrender
//
//  Created by 熊朝伟 on 2020/4/14.
//

#include "../defines.h"

OMP_RENDER_GLES2_USING_NAMESPACE

void RGBARenderer::load(RefPtr<RenderContext> context) {
    _program = new Program;
    _program->setVertexShader(
        #include "../shaders/RGBA.vs"
    );
    _program->setFragmentShader(
        #include "../shaders/RGBA.fs"
    );
    _program->link();
    _positionLocation = _program->attribLocation("position");
    _textureLocation = _program->uniformLocation("texture");
    _globalMatrixLocation = _program->uniformLocation("globalMatrix");
    _localMatrixLocation = _program->uniformLocation("localMatrix");
    _clipMatrixLocation = _program->uniformLocation("clipMatrix");
    _sizeLocation = _program->uniformLocation("size");
    _alphaLocation = _program->uniformLocation("alpha");
    _colorConversionLocation = _program->uniformLocation("colorConversion");

    _program->use();
    GL_ERROR(glEnableVertexAttribArray(_positionLocation));
}

void RGBARenderer::unload(RefPtr<RenderContext> context) {
    _positionLocation = 0;
    _textureLocation = 0;
    _globalMatrixLocation = 0;
    _localMatrixLocation = 0;
    _clipMatrixLocation = 0;
    _sizeLocation = 0;
    _alphaLocation = 0;
    _colorConversionLocation = 0;
    _program = nullptr;
}

void RGBARenderer::draw(
    RefPtr<RenderContext> context,
    RefPtr<Framebuffer> framebuffer,
    const mat4 &globalMatrix,
    const mat4 &localMatrix,
    const mat4 &clipMatrix,
    const mat4 &colorConversion,
    const vec2 &size,
    GLfloat alpha,
    va_list list
) {
    context->setFramebuffer(framebuffer);
    static const vec2 coordinates[] = {
            vec2(0.0f, 0.0f),
            vec2(0.0f, 1.0f),
            vec2(1.0f, 0.0f),
            vec2(1.0f, 1.0f),
    };
    static const GLubyte indices[] = {
            0, 1, 2, 1, 2, 3
    };
    _program->use();
    _program->setUniformTexture(_textureLocation, 1, va_arg(list, GLuint));
    _program->setUniform(_globalMatrixLocation, globalMatrix);
    _program->setUniform(_localMatrixLocation, localMatrix);
    _program->setUniform(_clipMatrixLocation, clipMatrix);
    _program->setUniform(_sizeLocation, size);
    _program->setUniform(_alphaLocation, alpha);
    _program->setUniform(_colorConversionLocation, colorConversion);
    _program->setVertexAttribPointer(_positionLocation, coordinates);
    _program->drawElements(GL_TRIANGLES, indices, 6);
}
