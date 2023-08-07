//
// Created by 熊朝伟 on 2020-03-16.
//

#pragma once

OMP_RENDER_GLES2_NAMESPACE_BEGIN

class Program : public Object {
    GLuint _program;
    GLuint _vertShader;
    GLuint _fragShader;
public:
    Program();
    ~Program();

    void setVertexShader(const char *vertexShader);
    void setFragmentShader(const char *fragmentShader);

    GLint attribLocation(const GLchar *attributeName);
    GLint uniformLocation(const GLchar *uniformName);
    void bindAttribLocation(GLuint index, const GLchar *name);
    bool link();
    void use();
    void validate();

    void setUniformTexture(GLint location, GLuint index, GLuint texture);
    void setUniform(GLint location, GLfloat value);
    void setUniform(GLint location, const vec2 &vec);
    void setUniform(GLint location, const vec4 &vec);
    void setUniform(GLint location, const mat3 &matrix);
    void setUniform(GLint location, const mat4 &matrix);

    void setVertexAttribPointer(GLint location, const vec2 *attrib);
    void drawElements(GLenum mode, const GLubyte *indices, GLsizei count);

    static GLuint compile(const char *source, GLenum type);
};

OMP_RENDER_GLES2_NAMESPACE_END
