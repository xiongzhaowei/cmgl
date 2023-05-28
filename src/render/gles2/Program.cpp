//
// Created by 熊朝伟 on 2020-03-16.
//

#include "defines.h"

OMP_RENDER_GLES2_USING_NAMESPACE

Program::Program() {
    GL_ERROR(_program = glCreateProgram());
}

Program::~Program() {
    if (_vertShader) {
        GL_ERROR(glDeleteShader(_vertShader));
        _vertShader = 0;
    }

    if (_fragShader) {
        GL_ERROR(glDeleteShader(_fragShader));
        _fragShader = 0;
    }

    if (_program) {
        GL_ERROR(glDeleteProgram(_program));
        _program = 0;
    }
}

void Program::setVertexShader(const char *shader) {
    _vertShader = compile(shader, GL_VERTEX_SHADER);
}

void Program::setFragmentShader(const char *shader) {
    _fragShader = compile(shader, GL_FRAGMENT_SHADER);
}

bool Program::link() {
    GL_ERROR(glAttachShader(_program, _vertShader));
    GL_ERROR(glAttachShader(_program, _fragShader));
    GL_ERROR(glLinkProgram(_program));

    GLint status;
    GL_ERROR(glGetProgramiv(_program, GL_LINK_STATUS, &status));
    if (status == GL_FALSE) {
        return false;
    }

    if (_vertShader) {
        GL_ERROR(glDeleteShader(_vertShader));
        _vertShader = 0;
    }
    if (_fragShader) {
        GL_ERROR(glDeleteShader(_fragShader));
        _fragShader = 0;
    }

    return true;
}

void Program::use() {
    GL_ERROR(glUseProgram(_program));
}

void Program::validate() {
    GLint logLength;

    glValidateProgram(_program);
    glGetProgramiv(_program, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0) {
#ifdef DEBUG
        GLchar *log = (GLchar *)malloc(logLength);
        glGetProgramInfoLog(_program, logLength, &logLength, log);
        printf("%s\n", log);
        free(log);
#endif
    }
}

GLuint Program::compile(const char *source, GLenum type) {
    if (!source) {
        //NSLog(@"Failed to load vertex shader");
        return 0;
    }

    GL_ERROR(GLuint result = glCreateShader(type));
    GL_ERROR(glShaderSource(result, 1, &source, NULL));
    GL_ERROR(glCompileShader(result));

    GLint status;
    GL_ERROR(glGetShaderiv(result, GL_COMPILE_STATUS, &status));

#ifdef DEBUG
    if (status != GL_TRUE) {
        GLint logLength;
        glGetShaderiv(result, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 0) {
            GLchar *log = (GLchar *)malloc(logLength);
            glGetShaderInfoLog(result, logLength, &logLength, log);
            printf("%s\n", log);
            free(log);
        }
    }
#endif

    return status == GL_TRUE ? result : 0;
}

GLint Program::attribLocation(const GLchar *attributeName) {
    return GL_ERROR(glGetAttribLocation(_program, attributeName));
}

GLint Program::uniformLocation(const GLchar *uniformName) {
    return GL_ERROR(glGetUniformLocation(_program, uniformName));
}

void Program::bindAttribLocation(GLuint index, const GLchar *name) {
    GL_ERROR(glBindAttribLocation(_program, index, name));
}

void Program::setUniformTexture(GLint location, GLuint index, GLuint texture) {
    assert(index < 32);
    GL_ERROR(glActiveTexture(GL_TEXTURE0 + index));
    GL_ERROR(glBindTexture(GL_TEXTURE_2D, texture));
    GL_ERROR(glUniform1i(location, index));
}

void Program::setUniform(GLint location, GLfloat value) {
    GL_ERROR(glUniform1fv(location, 1, &value));
}

void Program::setUniform(GLint location, const vec2 &vec) {
    GL_ERROR(glUniform2fv(location, 1, &vec[0]));
}

void Program::setUniform(GLint location, const mat3 &matrix) {
    GL_ERROR(glUniformMatrix3fv(location, 1, GL_FALSE, &matrix[0][0]));
}

void Program::setUniform(GLint location, const mat4 &matrix) {
    GL_ERROR(glUniformMatrix4fv(location, 1, GL_FALSE, &matrix[0][0]));
}

void Program::setVertexAttribPointer(GLint location, const vec2 *attrib) {
    GL_ERROR(glVertexAttribPointer(location, 2, GL_FLOAT, GL_FALSE, 0, attrib));
}

void Program::drawElements(GLenum mode, const GLubyte *indices, GLsizei count) {
    GL_ERROR(glDrawElements(mode, count, GL_UNSIGNED_BYTE, indices));
}
