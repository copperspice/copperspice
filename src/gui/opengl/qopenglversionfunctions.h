/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
*
* Copyright (c) 2013 Klarälvdalens Datakonsult AB, a KDAB Group company
* Copyright (c) 2015 The Qt Company Ltd.
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
*
* This file is part of CopperSpice.
*
* CopperSpice is free software. You can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* https://www.gnu.org/licenses/
*
***********************************************************************/

#ifndef QOPENGLVERSIONFUNCTIONS_H
#define QOPENGLVERSIONFUNCTIONS_H

#include <qglobal.h>

#ifndef QT_NO_OPENGL

#include <qhashfunc.h>
#include <qpair.h>
#include <qopengl.h>

class QOpenGLContext;

struct QOpenGLVersionStatus
{
    enum OpenGLStatus {
        CoreStatus,
        DeprecatedStatus,
        InvalidStatus
    };

    constexpr QOpenGLVersionStatus()
        : version(0, 0),
          status(InvalidStatus)
    {}

    constexpr QOpenGLVersionStatus(int majorVersion, int minorVersion, QOpenGLVersionStatus::OpenGLStatus functionStatus)
        : version(majorVersion, minorVersion),
          status(functionStatus)
    {}

    QPair<int, int> version;
    OpenGLStatus status;
};

inline uint qHash(const QOpenGLVersionStatus &v, uint seed = 0)
{
    return qHash(static_cast<int>(v.status * 1000)
               + v.version.first * 100 + v.version.second * 10, seed);
}

constexpr inline bool operator==(const QOpenGLVersionStatus &lhs, const QOpenGLVersionStatus &rhs)
{
    return lhs.status == rhs.status && lhs.version == rhs.version;
}

constexpr inline bool operator!=(const QOpenGLVersionStatus &lhs, const QOpenGLVersionStatus &rhs)
{
    return !operator==(lhs, rhs);
}

class QOpenGLVersionFunctionsBackend
{
public:
    QOpenGLVersionFunctionsBackend(QOpenGLContext *ctx)
        : context(ctx)
    {}

    QOpenGLContext *context;
    QAtomicInt refs;
};

class QAbstractOpenGLFunctions;

class QAbstractOpenGLFunctionsPrivate
{
public:
    QAbstractOpenGLFunctionsPrivate()
        : owningContext(nullptr),
          initialized(false)
    {}

    static QOpenGLVersionFunctionsBackend *functionsBackend(QOpenGLContext *context,
                                                            const QOpenGLVersionStatus &v);
    static void insertFunctionsBackend(QOpenGLContext *context,
                                       const QOpenGLVersionStatus &v,
                                       QOpenGLVersionFunctionsBackend *backend);
    static void removeFunctionsBackend(QOpenGLContext *context, const QOpenGLVersionStatus &v);
    static void insertExternalFunctions(QOpenGLContext *context, QAbstractOpenGLFunctions *f);
    static void removeExternalFunctions(QOpenGLContext *context, QAbstractOpenGLFunctions *f);

    static QAbstractOpenGLFunctionsPrivate *get(QAbstractOpenGLFunctions *q);

    QOpenGLContext *owningContext;
    bool initialized;
};

class Q_GUI_EXPORT QAbstractOpenGLFunctions
{
public:
    virtual ~QAbstractOpenGLFunctions();

    virtual bool initializeOpenGLFunctions();

    Q_DECLARE_PRIVATE(QAbstractOpenGLFunctions)

protected:
    QAbstractOpenGLFunctions();
    QAbstractOpenGLFunctionsPrivate *d_ptr;

    bool isInitialized() const;

    void setOwningContext(const QOpenGLContext *context);
    QOpenGLContext *owningContext() const;

    friend class QOpenGLContext;
};

inline QAbstractOpenGLFunctionsPrivate *QAbstractOpenGLFunctionsPrivate::get(QAbstractOpenGLFunctions *q)
{
    return q->d_func();
}

#if !defined(QT_OPENGL_ES_2)

class QOpenGLFunctions_1_0_CoreBackend : public QOpenGLVersionFunctionsBackend
{
public:
    QOpenGLFunctions_1_0_CoreBackend(QOpenGLContext *context);

    static QOpenGLVersionStatus versionStatus();

    // OpenGL 1.0 core functions
    void (QOPENGLF_APIENTRYP Viewport)(GLint x, GLint y, GLsizei width, GLsizei height);
    void (QOPENGLF_APIENTRYP DepthRange)(GLdouble nearVal, GLdouble farVal);
    GLboolean (QOPENGLF_APIENTRYP IsEnabled)(GLenum cap);
    void (QOPENGLF_APIENTRYP GetTexLevelParameteriv)(GLenum target, GLint level, GLenum pname, GLint *params);
    void (QOPENGLF_APIENTRYP GetTexLevelParameterfv)(GLenum target, GLint level, GLenum pname, GLfloat *params);
    void (QOPENGLF_APIENTRYP GetTexParameteriv)(GLenum target, GLenum pname, GLint *params);
    void (QOPENGLF_APIENTRYP GetTexParameterfv)(GLenum target, GLenum pname, GLfloat *params);
    void (QOPENGLF_APIENTRYP GetTexImage)(GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels);
    const GLubyte * (QOPENGLF_APIENTRYP GetString)(GLenum name);
    void (QOPENGLF_APIENTRYP GetIntegerv)(GLenum pname, GLint *data);
    void (QOPENGLF_APIENTRYP GetFloatv)(GLenum pname, GLfloat *data);
    GLenum (QOPENGLF_APIENTRYP GetError)();
    void (QOPENGLF_APIENTRYP GetDoublev)(GLenum pname, GLdouble *data);
    void (QOPENGLF_APIENTRYP GetBooleanv)(GLenum pname, GLboolean *data);
    void (QOPENGLF_APIENTRYP ReadPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels);
    void (QOPENGLF_APIENTRYP ReadBuffer)(GLenum src);
    void (QOPENGLF_APIENTRYP PixelStorei)(GLenum pname, GLint param);
    void (QOPENGLF_APIENTRYP PixelStoref)(GLenum pname, GLfloat param);
    void (QOPENGLF_APIENTRYP DepthFunc)(GLenum func);
    void (QOPENGLF_APIENTRYP StencilOp)(GLenum fail, GLenum zfail, GLenum zpass);
    void (QOPENGLF_APIENTRYP StencilFunc)(GLenum func, GLint ref, GLuint mask);
    void (QOPENGLF_APIENTRYP LogicOp)(GLenum opcode);
    void (QOPENGLF_APIENTRYP BlendFunc)(GLenum sfactor, GLenum dfactor);
    void (QOPENGLF_APIENTRYP Flush)();
    void (QOPENGLF_APIENTRYP Finish)();
    void (QOPENGLF_APIENTRYP Enable)(GLenum cap);
    void (QOPENGLF_APIENTRYP Disable)(GLenum cap);
    void (QOPENGLF_APIENTRYP DepthMask)(GLboolean flag);
    void (QOPENGLF_APIENTRYP ColorMask)(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
    void (QOPENGLF_APIENTRYP StencilMask)(GLuint mask);
    void (QOPENGLF_APIENTRYP ClearDepth)(GLdouble depth);
    void (QOPENGLF_APIENTRYP ClearStencil)(GLint s);
    void (QOPENGLF_APIENTRYP ClearColor)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
    void (QOPENGLF_APIENTRYP Clear)(GLbitfield mask);
    void (QOPENGLF_APIENTRYP DrawBuffer)(GLenum buf);
    void (QOPENGLF_APIENTRYP TexImage2D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
    void (QOPENGLF_APIENTRYP TexImage1D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
    void (QOPENGLF_APIENTRYP TexParameteriv)(GLenum target, GLenum pname, const GLint *params);
    void (QOPENGLF_APIENTRYP TexParameteri)(GLenum target, GLenum pname, GLint param);
    void (QOPENGLF_APIENTRYP TexParameterfv)(GLenum target, GLenum pname, const GLfloat *params);
    void (QOPENGLF_APIENTRYP TexParameterf)(GLenum target, GLenum pname, GLfloat param);
    void (QOPENGLF_APIENTRYP Scissor)(GLint x, GLint y, GLsizei width, GLsizei height);
    void (QOPENGLF_APIENTRYP PolygonMode)(GLenum face, GLenum mode);
    void (QOPENGLF_APIENTRYP PointSize)(GLfloat size);
    void (QOPENGLF_APIENTRYP LineWidth)(GLfloat width);
    void (QOPENGLF_APIENTRYP Hint)(GLenum target, GLenum mode);
    void (QOPENGLF_APIENTRYP FrontFace)(GLenum mode);
    void (QOPENGLF_APIENTRYP CullFace)(GLenum mode);

};

class QOpenGLFunctions_1_1_CoreBackend : public QOpenGLVersionFunctionsBackend
{
public:
    QOpenGLFunctions_1_1_CoreBackend(QOpenGLContext *context);

    static QOpenGLVersionStatus versionStatus();

    // OpenGL 1.1 core functions
    void (QOPENGLF_APIENTRYP Indexubv)(const GLubyte *c);
    void (QOPENGLF_APIENTRYP Indexub)(GLubyte c);
    GLboolean (QOPENGLF_APIENTRYP IsTexture)(GLuint texture);
    void (QOPENGLF_APIENTRYP GenTextures)(GLsizei n, GLuint *textures);
    void (QOPENGLF_APIENTRYP DeleteTextures)(GLsizei n, const GLuint *textures);
    void (QOPENGLF_APIENTRYP BindTexture)(GLenum target, GLuint texture);
    void (QOPENGLF_APIENTRYP TexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
    void (QOPENGLF_APIENTRYP TexSubImage1D)(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels);
    void (QOPENGLF_APIENTRYP CopyTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
    void (QOPENGLF_APIENTRYP CopyTexSubImage1D)(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
    void (QOPENGLF_APIENTRYP CopyTexImage2D)(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
    void (QOPENGLF_APIENTRYP CopyTexImage1D)(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);
    void (QOPENGLF_APIENTRYP PolygonOffset)(GLfloat factor, GLfloat units);
    void (QOPENGLF_APIENTRYP GetPointerv)(GLenum pname, GLvoid* *params);
    void (QOPENGLF_APIENTRYP DrawElements)(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
    void (QOPENGLF_APIENTRYP DrawArrays)(GLenum mode, GLint first, GLsizei count);

};

class QOpenGLFunctions_1_2_CoreBackend : public QOpenGLVersionFunctionsBackend
{
public:
    QOpenGLFunctions_1_2_CoreBackend(QOpenGLContext *context);

    static QOpenGLVersionStatus versionStatus();

    // OpenGL 1.2 core functions
    void (QOPENGLF_APIENTRYP CopyTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
    void (QOPENGLF_APIENTRYP TexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels);
    void (QOPENGLF_APIENTRYP TexImage3D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
    void (QOPENGLF_APIENTRYP DrawRangeElements)(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices);
    void (QOPENGLF_APIENTRYP BlendEquation)(GLenum mode);
    void (QOPENGLF_APIENTRYP BlendColor)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);

};

class QOpenGLFunctions_1_3_CoreBackend : public QOpenGLVersionFunctionsBackend
{
public:
    QOpenGLFunctions_1_3_CoreBackend(QOpenGLContext *context);

    static QOpenGLVersionStatus versionStatus();

    // OpenGL 1.3 core functions
    void (QOPENGLF_APIENTRYP GetCompressedTexImage)(GLenum target, GLint level, GLvoid *img);
    void (QOPENGLF_APIENTRYP CompressedTexSubImage1D)(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data);
    void (QOPENGLF_APIENTRYP CompressedTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data);
    void (QOPENGLF_APIENTRYP CompressedTexSubImage3D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *data);
    void (QOPENGLF_APIENTRYP CompressedTexImage1D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data);
    void (QOPENGLF_APIENTRYP CompressedTexImage2D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data);
    void (QOPENGLF_APIENTRYP CompressedTexImage3D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *data);
    void (QOPENGLF_APIENTRYP SampleCoverage)(GLfloat value, GLboolean invert);
    void (QOPENGLF_APIENTRYP ActiveTexture)(GLenum texture);

};

class QOpenGLFunctions_1_4_CoreBackend : public QOpenGLVersionFunctionsBackend
{
public:
    QOpenGLFunctions_1_4_CoreBackend(QOpenGLContext *context);

    static QOpenGLVersionStatus versionStatus();

    // OpenGL 1.4 core functions
    void (QOPENGLF_APIENTRYP PointParameteriv)(GLenum pname, const GLint *params);
    void (QOPENGLF_APIENTRYP PointParameteri)(GLenum pname, GLint param);
    void (QOPENGLF_APIENTRYP PointParameterfv)(GLenum pname, const GLfloat *params);
    void (QOPENGLF_APIENTRYP PointParameterf)(GLenum pname, GLfloat param);
    void (QOPENGLF_APIENTRYP MultiDrawElements)(GLenum mode, const GLsizei *count, GLenum type, const GLvoid* const *indices, GLsizei drawcount);
    void (QOPENGLF_APIENTRYP MultiDrawArrays)(GLenum mode, const GLint *first, const GLsizei *count, GLsizei drawcount);
    void (QOPENGLF_APIENTRYP BlendFuncSeparate)(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);

};

class QOpenGLFunctions_1_5_CoreBackend : public QOpenGLVersionFunctionsBackend
{
public:
    QOpenGLFunctions_1_5_CoreBackend(QOpenGLContext *context);

    static QOpenGLVersionStatus versionStatus();

    // OpenGL 1.5 core functions
    void (QOPENGLF_APIENTRYP GetBufferPointerv)(GLenum target, GLenum pname, GLvoid* *params);
    void (QOPENGLF_APIENTRYP GetBufferParameteriv)(GLenum target, GLenum pname, GLint *params);
    GLboolean (QOPENGLF_APIENTRYP UnmapBuffer)(GLenum target);
    GLvoid* (QOPENGLF_APIENTRYP MapBuffer)(GLenum target, GLenum access);
    void (QOPENGLF_APIENTRYP GetBufferSubData)(GLenum target, GLintptr offset, GLsizeiptr size, GLvoid *data);
    void (QOPENGLF_APIENTRYP BufferSubData)(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid *data);
    void (QOPENGLF_APIENTRYP BufferData)(GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage);
    GLboolean (QOPENGLF_APIENTRYP IsBuffer)(GLuint buffer);
    void (QOPENGLF_APIENTRYP GenBuffers)(GLsizei n, GLuint *buffers);
    void (QOPENGLF_APIENTRYP DeleteBuffers)(GLsizei n, const GLuint *buffers);
    void (QOPENGLF_APIENTRYP BindBuffer)(GLenum target, GLuint buffer);
    void (QOPENGLF_APIENTRYP GetQueryObjectuiv)(GLuint id, GLenum pname, GLuint *params);
    void (QOPENGLF_APIENTRYP GetQueryObjectiv)(GLuint id, GLenum pname, GLint *params);
    void (QOPENGLF_APIENTRYP GetQueryiv)(GLenum target, GLenum pname, GLint *params);
    void (QOPENGLF_APIENTRYP EndQuery)(GLenum target);
    void (QOPENGLF_APIENTRYP BeginQuery)(GLenum target, GLuint id);
    GLboolean (QOPENGLF_APIENTRYP IsQuery)(GLuint id);
    void (QOPENGLF_APIENTRYP DeleteQueries)(GLsizei n, const GLuint *ids);
    void (QOPENGLF_APIENTRYP GenQueries)(GLsizei n, GLuint *ids);

};

class QOpenGLFunctions_2_0_CoreBackend : public QOpenGLVersionFunctionsBackend
{
public:
    QOpenGLFunctions_2_0_CoreBackend(QOpenGLContext *context);

    static QOpenGLVersionStatus versionStatus();

    // OpenGL 2.0 core functions
    void (QOPENGLF_APIENTRYP VertexAttribPointer)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer);
    void (QOPENGLF_APIENTRYP ValidateProgram)(GLuint program);
    void (QOPENGLF_APIENTRYP UniformMatrix4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    void (QOPENGLF_APIENTRYP UniformMatrix3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    void (QOPENGLF_APIENTRYP UniformMatrix2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    void (QOPENGLF_APIENTRYP Uniform4iv)(GLint location, GLsizei count, const GLint *value);
    void (QOPENGLF_APIENTRYP Uniform3iv)(GLint location, GLsizei count, const GLint *value);
    void (QOPENGLF_APIENTRYP Uniform2iv)(GLint location, GLsizei count, const GLint *value);
    void (QOPENGLF_APIENTRYP Uniform1iv)(GLint location, GLsizei count, const GLint *value);
    void (QOPENGLF_APIENTRYP Uniform4fv)(GLint location, GLsizei count, const GLfloat *value);
    void (QOPENGLF_APIENTRYP Uniform3fv)(GLint location, GLsizei count, const GLfloat *value);
    void (QOPENGLF_APIENTRYP Uniform2fv)(GLint location, GLsizei count, const GLfloat *value);
    void (QOPENGLF_APIENTRYP Uniform1fv)(GLint location, GLsizei count, const GLfloat *value);
    void (QOPENGLF_APIENTRYP Uniform4i)(GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
    void (QOPENGLF_APIENTRYP Uniform3i)(GLint location, GLint v0, GLint v1, GLint v2);
    void (QOPENGLF_APIENTRYP Uniform2i)(GLint location, GLint v0, GLint v1);
    void (QOPENGLF_APIENTRYP Uniform1i)(GLint location, GLint v0);
    void (QOPENGLF_APIENTRYP Uniform4f)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
    void (QOPENGLF_APIENTRYP Uniform3f)(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
    void (QOPENGLF_APIENTRYP Uniform2f)(GLint location, GLfloat v0, GLfloat v1);
    void (QOPENGLF_APIENTRYP Uniform1f)(GLint location, GLfloat v0);
    void (QOPENGLF_APIENTRYP UseProgram)(GLuint program);
    void (QOPENGLF_APIENTRYP ShaderSource)(GLuint shader, GLsizei count, const GLchar* const *string, const GLint *length);
    void (QOPENGLF_APIENTRYP LinkProgram)(GLuint program);
    GLboolean (QOPENGLF_APIENTRYP IsShader)(GLuint shader);
    GLboolean (QOPENGLF_APIENTRYP IsProgram)(GLuint program);
    void (QOPENGLF_APIENTRYP GetVertexAttribPointerv)(GLuint index, GLenum pname, GLvoid* *pointer);
    void (QOPENGLF_APIENTRYP GetVertexAttribiv)(GLuint index, GLenum pname, GLint *params);
    void (QOPENGLF_APIENTRYP GetVertexAttribfv)(GLuint index, GLenum pname, GLfloat *params);
    void (QOPENGLF_APIENTRYP GetVertexAttribdv)(GLuint index, GLenum pname, GLdouble *params);
    void (QOPENGLF_APIENTRYP GetUniformiv)(GLuint program, GLint location, GLint *params);
    void (QOPENGLF_APIENTRYP GetUniformfv)(GLuint program, GLint location, GLfloat *params);
    GLint (QOPENGLF_APIENTRYP GetUniformLocation)(GLuint program, const GLchar *name);
    void (QOPENGLF_APIENTRYP GetShaderSource)(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source);
    void (QOPENGLF_APIENTRYP GetShaderInfoLog)(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
    void (QOPENGLF_APIENTRYP GetShaderiv)(GLuint shader, GLenum pname, GLint *params);
    void (QOPENGLF_APIENTRYP GetProgramInfoLog)(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
    void (QOPENGLF_APIENTRYP GetProgramiv)(GLuint program, GLenum pname, GLint *params);
    GLint (QOPENGLF_APIENTRYP GetAttribLocation)(GLuint program, const GLchar *name);
    void (QOPENGLF_APIENTRYP GetAttachedShaders)(GLuint program, GLsizei maxCount, GLsizei *count, GLuint *shaders);
    void (QOPENGLF_APIENTRYP GetActiveUniform)(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
    void (QOPENGLF_APIENTRYP GetActiveAttrib)(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
    void (QOPENGLF_APIENTRYP EnableVertexAttribArray)(GLuint index);
    void (QOPENGLF_APIENTRYP DisableVertexAttribArray)(GLuint index);
    void (QOPENGLF_APIENTRYP DetachShader)(GLuint program, GLuint shader);
    void (QOPENGLF_APIENTRYP DeleteShader)(GLuint shader);
    void (QOPENGLF_APIENTRYP DeleteProgram)(GLuint program);
    GLuint (QOPENGLF_APIENTRYP CreateShader)(GLenum type);
    GLuint (QOPENGLF_APIENTRYP CreateProgram)();
    void (QOPENGLF_APIENTRYP CompileShader)(GLuint shader);
    void (QOPENGLF_APIENTRYP BindAttribLocation)(GLuint program, GLuint index, const GLchar *name);
    void (QOPENGLF_APIENTRYP AttachShader)(GLuint program, GLuint shader);
    void (QOPENGLF_APIENTRYP StencilMaskSeparate)(GLenum face, GLuint mask);
    void (QOPENGLF_APIENTRYP StencilFuncSeparate)(GLenum face, GLenum func, GLint ref, GLuint mask);
    void (QOPENGLF_APIENTRYP StencilOpSeparate)(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass);
    void (QOPENGLF_APIENTRYP DrawBuffers)(GLsizei n, const GLenum *bufs);
    void (QOPENGLF_APIENTRYP BlendEquationSeparate)(GLenum modeRGB, GLenum modeAlpha);
    void (QOPENGLF_APIENTRYP VertexAttrib4usv)(GLuint index, const GLushort *v);
    void (QOPENGLF_APIENTRYP VertexAttrib4uiv)(GLuint index, const GLuint *v);
    void (QOPENGLF_APIENTRYP VertexAttrib4ubv)(GLuint index, const GLubyte *v);
    void (QOPENGLF_APIENTRYP VertexAttrib4sv)(GLuint index, const GLshort *v);
    void (QOPENGLF_APIENTRYP VertexAttrib4s)(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w);
    void (QOPENGLF_APIENTRYP VertexAttrib4iv)(GLuint index, const GLint *v);
    void (QOPENGLF_APIENTRYP VertexAttrib4fv)(GLuint index, const GLfloat *v);
    void (QOPENGLF_APIENTRYP VertexAttrib4f)(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
    void (QOPENGLF_APIENTRYP VertexAttrib4dv)(GLuint index, const GLdouble *v);
    void (QOPENGLF_APIENTRYP VertexAttrib4d)(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
    void (QOPENGLF_APIENTRYP VertexAttrib4bv)(GLuint index, const GLbyte *v);
    void (QOPENGLF_APIENTRYP VertexAttrib4Nusv)(GLuint index, const GLushort *v);
    void (QOPENGLF_APIENTRYP VertexAttrib4Nuiv)(GLuint index, const GLuint *v);
    void (QOPENGLF_APIENTRYP VertexAttrib4Nubv)(GLuint index, const GLubyte *v);
    void (QOPENGLF_APIENTRYP VertexAttrib4Nub)(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w);
    void (QOPENGLF_APIENTRYP VertexAttrib4Nsv)(GLuint index, const GLshort *v);
    void (QOPENGLF_APIENTRYP VertexAttrib4Niv)(GLuint index, const GLint *v);
    void (QOPENGLF_APIENTRYP VertexAttrib4Nbv)(GLuint index, const GLbyte *v);
    void (QOPENGLF_APIENTRYP VertexAttrib3sv)(GLuint index, const GLshort *v);
    void (QOPENGLF_APIENTRYP VertexAttrib3s)(GLuint index, GLshort x, GLshort y, GLshort z);
    void (QOPENGLF_APIENTRYP VertexAttrib3fv)(GLuint index, const GLfloat *v);
    void (QOPENGLF_APIENTRYP VertexAttrib3f)(GLuint index, GLfloat x, GLfloat y, GLfloat z);
    void (QOPENGLF_APIENTRYP VertexAttrib3dv)(GLuint index, const GLdouble *v);
    void (QOPENGLF_APIENTRYP VertexAttrib3d)(GLuint index, GLdouble x, GLdouble y, GLdouble z);
    void (QOPENGLF_APIENTRYP VertexAttrib2sv)(GLuint index, const GLshort *v);
    void (QOPENGLF_APIENTRYP VertexAttrib2s)(GLuint index, GLshort x, GLshort y);
    void (QOPENGLF_APIENTRYP VertexAttrib2fv)(GLuint index, const GLfloat *v);
    void (QOPENGLF_APIENTRYP VertexAttrib2f)(GLuint index, GLfloat x, GLfloat y);
    void (QOPENGLF_APIENTRYP VertexAttrib2dv)(GLuint index, const GLdouble *v);
    void (QOPENGLF_APIENTRYP VertexAttrib2d)(GLuint index, GLdouble x, GLdouble y);
    void (QOPENGLF_APIENTRYP VertexAttrib1sv)(GLuint index, const GLshort *v);
    void (QOPENGLF_APIENTRYP VertexAttrib1s)(GLuint index, GLshort x);
    void (QOPENGLF_APIENTRYP VertexAttrib1fv)(GLuint index, const GLfloat *v);
    void (QOPENGLF_APIENTRYP VertexAttrib1f)(GLuint index, GLfloat x);
    void (QOPENGLF_APIENTRYP VertexAttrib1dv)(GLuint index, const GLdouble *v);
    void (QOPENGLF_APIENTRYP VertexAttrib1d)(GLuint index, GLdouble x);
};

class QOpenGLFunctions_2_1_CoreBackend : public QOpenGLVersionFunctionsBackend
{
public:
    QOpenGLFunctions_2_1_CoreBackend(QOpenGLContext *context);

    static QOpenGLVersionStatus versionStatus();

    // OpenGL 2.1 core functions
    void (QOPENGLF_APIENTRYP UniformMatrix4x3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    void (QOPENGLF_APIENTRYP UniformMatrix3x4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    void (QOPENGLF_APIENTRYP UniformMatrix4x2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    void (QOPENGLF_APIENTRYP UniformMatrix2x4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    void (QOPENGLF_APIENTRYP UniformMatrix3x2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    void (QOPENGLF_APIENTRYP UniformMatrix2x3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);

};

class QOpenGLFunctions_3_0_CoreBackend : public QOpenGLVersionFunctionsBackend
{
public:
    QOpenGLFunctions_3_0_CoreBackend(QOpenGLContext *context);

    static QOpenGLVersionStatus versionStatus();

    // OpenGL 3.0 core functions
    GLboolean (QOPENGLF_APIENTRYP IsVertexArray)(GLuint array);
    void (QOPENGLF_APIENTRYP GenVertexArrays)(GLsizei n, GLuint *arrays);
    void (QOPENGLF_APIENTRYP DeleteVertexArrays)(GLsizei n, const GLuint *arrays);
    void (QOPENGLF_APIENTRYP BindVertexArray)(GLuint array);
    void (QOPENGLF_APIENTRYP FlushMappedBufferRange)(GLenum target, GLintptr offset, GLsizeiptr length);
    GLvoid* (QOPENGLF_APIENTRYP MapBufferRange)(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
    void (QOPENGLF_APIENTRYP FramebufferTextureLayer)(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer);
    void (QOPENGLF_APIENTRYP RenderbufferStorageMultisample)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
    void (QOPENGLF_APIENTRYP BlitFramebuffer)(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
    void (QOPENGLF_APIENTRYP GenerateMipmap)(GLenum target);
    void (QOPENGLF_APIENTRYP GetFramebufferAttachmentParameteriv)(GLenum target, GLenum attachment, GLenum pname, GLint *params);
    void (QOPENGLF_APIENTRYP FramebufferRenderbuffer)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
    void (QOPENGLF_APIENTRYP FramebufferTexture3D)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset);
    void (QOPENGLF_APIENTRYP FramebufferTexture2D)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
    void (QOPENGLF_APIENTRYP FramebufferTexture1D)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
    GLenum (QOPENGLF_APIENTRYP CheckFramebufferStatus)(GLenum target);
    void (QOPENGLF_APIENTRYP GenFramebuffers)(GLsizei n, GLuint *framebuffers);
    void (QOPENGLF_APIENTRYP DeleteFramebuffers)(GLsizei n, const GLuint *framebuffers);
    void (QOPENGLF_APIENTRYP BindFramebuffer)(GLenum target, GLuint framebuffer);
    GLboolean (QOPENGLF_APIENTRYP IsFramebuffer)(GLuint framebuffer);
    void (QOPENGLF_APIENTRYP GetRenderbufferParameteriv)(GLenum target, GLenum pname, GLint *params);
    void (QOPENGLF_APIENTRYP RenderbufferStorage)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
    void (QOPENGLF_APIENTRYP GenRenderbuffers)(GLsizei n, GLuint *renderbuffers);
    void (QOPENGLF_APIENTRYP DeleteRenderbuffers)(GLsizei n, const GLuint *renderbuffers);
    void (QOPENGLF_APIENTRYP BindRenderbuffer)(GLenum target, GLuint renderbuffer);
    GLboolean (QOPENGLF_APIENTRYP IsRenderbuffer)(GLuint renderbuffer);
    const GLubyte * (QOPENGLF_APIENTRYP GetStringi)(GLenum name, GLuint index);
    void (QOPENGLF_APIENTRYP ClearBufferfi)(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil);
    void (QOPENGLF_APIENTRYP ClearBufferfv)(GLenum buffer, GLint drawbuffer, const GLfloat *value);
    void (QOPENGLF_APIENTRYP ClearBufferuiv)(GLenum buffer, GLint drawbuffer, const GLuint *value);
    void (QOPENGLF_APIENTRYP ClearBufferiv)(GLenum buffer, GLint drawbuffer, const GLint *value);
    void (QOPENGLF_APIENTRYP GetTexParameterIuiv)(GLenum target, GLenum pname, GLuint *params);
    void (QOPENGLF_APIENTRYP GetTexParameterIiv)(GLenum target, GLenum pname, GLint *params);
    void (QOPENGLF_APIENTRYP TexParameterIuiv)(GLenum target, GLenum pname, const GLuint *params);
    void (QOPENGLF_APIENTRYP TexParameterIiv)(GLenum target, GLenum pname, const GLint *params);
    void (QOPENGLF_APIENTRYP Uniform4uiv)(GLint location, GLsizei count, const GLuint *value);
    void (QOPENGLF_APIENTRYP Uniform3uiv)(GLint location, GLsizei count, const GLuint *value);
    void (QOPENGLF_APIENTRYP Uniform2uiv)(GLint location, GLsizei count, const GLuint *value);
    void (QOPENGLF_APIENTRYP Uniform1uiv)(GLint location, GLsizei count, const GLuint *value);
    void (QOPENGLF_APIENTRYP Uniform4ui)(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
    void (QOPENGLF_APIENTRYP Uniform3ui)(GLint location, GLuint v0, GLuint v1, GLuint v2);
    void (QOPENGLF_APIENTRYP Uniform2ui)(GLint location, GLuint v0, GLuint v1);
    void (QOPENGLF_APIENTRYP Uniform1ui)(GLint location, GLuint v0);
    GLint (QOPENGLF_APIENTRYP GetFragDataLocation)(GLuint program, const GLchar *name);
    void (QOPENGLF_APIENTRYP BindFragDataLocation)(GLuint program, GLuint color, const GLchar *name);
    void (QOPENGLF_APIENTRYP GetUniformuiv)(GLuint program, GLint location, GLuint *params);
    void (QOPENGLF_APIENTRYP GetVertexAttribIuiv)(GLuint index, GLenum pname, GLuint *params);
    void (QOPENGLF_APIENTRYP GetVertexAttribIiv)(GLuint index, GLenum pname, GLint *params);
    void (QOPENGLF_APIENTRYP VertexAttribIPointer)(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
    void (QOPENGLF_APIENTRYP EndConditionalRender)();
    void (QOPENGLF_APIENTRYP BeginConditionalRender)(GLuint id, GLenum mode);
    void (QOPENGLF_APIENTRYP ClampColor)(GLenum target, GLenum clamp);
    void (QOPENGLF_APIENTRYP GetTransformFeedbackVarying)(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLsizei *size, GLenum *type, GLchar *name);
    void (QOPENGLF_APIENTRYP TransformFeedbackVaryings)(GLuint program, GLsizei count, const GLchar* const *varyings, GLenum bufferMode);
    void (QOPENGLF_APIENTRYP BindBufferBase)(GLenum target, GLuint index, GLuint buffer);
    void (QOPENGLF_APIENTRYP BindBufferRange)(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
    void (QOPENGLF_APIENTRYP EndTransformFeedback)();
    void (QOPENGLF_APIENTRYP BeginTransformFeedback)(GLenum primitiveMode);
    GLboolean (QOPENGLF_APIENTRYP IsEnabledi)(GLenum target, GLuint index);
    void (QOPENGLF_APIENTRYP Disablei)(GLenum target, GLuint index);
    void (QOPENGLF_APIENTRYP Enablei)(GLenum target, GLuint index);
    void (QOPENGLF_APIENTRYP GetIntegeri_v)(GLenum target, GLuint index, GLint *data);
    void (QOPENGLF_APIENTRYP GetBooleani_v)(GLenum target, GLuint index, GLboolean *data);
    void (QOPENGLF_APIENTRYP ColorMaski)(GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a);
    void (QOPENGLF_APIENTRYP VertexAttribI4usv)(GLuint index, const GLushort *v);
    void (QOPENGLF_APIENTRYP VertexAttribI4ubv)(GLuint index, const GLubyte *v);
    void (QOPENGLF_APIENTRYP VertexAttribI4sv)(GLuint index, const GLshort *v);
    void (QOPENGLF_APIENTRYP VertexAttribI4bv)(GLuint index, const GLbyte *v);
    void (QOPENGLF_APIENTRYP VertexAttribI4uiv)(GLuint index, const GLuint *v);
    void (QOPENGLF_APIENTRYP VertexAttribI3uiv)(GLuint index, const GLuint *v);
    void (QOPENGLF_APIENTRYP VertexAttribI2uiv)(GLuint index, const GLuint *v);
    void (QOPENGLF_APIENTRYP VertexAttribI1uiv)(GLuint index, const GLuint *v);
    void (QOPENGLF_APIENTRYP VertexAttribI4iv)(GLuint index, const GLint *v);
    void (QOPENGLF_APIENTRYP VertexAttribI3iv)(GLuint index, const GLint *v);
    void (QOPENGLF_APIENTRYP VertexAttribI2iv)(GLuint index, const GLint *v);
    void (QOPENGLF_APIENTRYP VertexAttribI1iv)(GLuint index, const GLint *v);
    void (QOPENGLF_APIENTRYP VertexAttribI4ui)(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w);
    void (QOPENGLF_APIENTRYP VertexAttribI3ui)(GLuint index, GLuint x, GLuint y, GLuint z);
    void (QOPENGLF_APIENTRYP VertexAttribI2ui)(GLuint index, GLuint x, GLuint y);
    void (QOPENGLF_APIENTRYP VertexAttribI1ui)(GLuint index, GLuint x);
    void (QOPENGLF_APIENTRYP VertexAttribI4i)(GLuint index, GLint x, GLint y, GLint z, GLint w);
    void (QOPENGLF_APIENTRYP VertexAttribI3i)(GLuint index, GLint x, GLint y, GLint z);
    void (QOPENGLF_APIENTRYP VertexAttribI2i)(GLuint index, GLint x, GLint y);
    void (QOPENGLF_APIENTRYP VertexAttribI1i)(GLuint index, GLint x);
};

class QOpenGLFunctions_3_1_CoreBackend : public QOpenGLVersionFunctionsBackend
{
public:
    QOpenGLFunctions_3_1_CoreBackend(QOpenGLContext *context);

    static QOpenGLVersionStatus versionStatus();

    // OpenGL 3.1 core functions
    void (QOPENGLF_APIENTRYP CopyBufferSubData)(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
    void (QOPENGLF_APIENTRYP UniformBlockBinding)(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);
    void (QOPENGLF_APIENTRYP GetActiveUniformBlockName)(GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformBlockName);
    void (QOPENGLF_APIENTRYP GetActiveUniformBlockiv)(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint *params);
    GLuint (QOPENGLF_APIENTRYP GetUniformBlockIndex)(GLuint program, const GLchar *uniformBlockName);
    void (QOPENGLF_APIENTRYP GetActiveUniformName)(GLuint program, GLuint uniformIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformName);
    void (QOPENGLF_APIENTRYP GetActiveUniformsiv)(GLuint program, GLsizei uniformCount, const GLuint *uniformIndices, GLenum pname, GLint *params);
    void (QOPENGLF_APIENTRYP GetUniformIndices)(GLuint program, GLsizei uniformCount, const GLchar* const *uniformNames, GLuint *uniformIndices);
    void (QOPENGLF_APIENTRYP PrimitiveRestartIndex)(GLuint index);
    void (QOPENGLF_APIENTRYP TexBuffer)(GLenum target, GLenum internalformat, GLuint buffer);
    void (QOPENGLF_APIENTRYP DrawElementsInstanced)(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLsizei instancecount);
    void (QOPENGLF_APIENTRYP DrawArraysInstanced)(GLenum mode, GLint first, GLsizei count, GLsizei instancecount);

};

class QOpenGLFunctions_3_2_CoreBackend : public QOpenGLVersionFunctionsBackend
{
public:
    QOpenGLFunctions_3_2_CoreBackend(QOpenGLContext *context);

    static QOpenGLVersionStatus versionStatus();

    // OpenGL 3.2 core functions
    void (QOPENGLF_APIENTRYP SampleMaski)(GLuint maskNumber, GLbitfield mask);
    void (QOPENGLF_APIENTRYP GetMultisamplefv)(GLenum pname, GLuint index, GLfloat *val);
    void (QOPENGLF_APIENTRYP TexImage3DMultisample)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations);
    void (QOPENGLF_APIENTRYP TexImage2DMultisample)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
    void (QOPENGLF_APIENTRYP GetSynciv)(GLsync sync, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *values);
    void (QOPENGLF_APIENTRYP GetInteger64v)(GLenum pname, GLint64 *data);
    void (QOPENGLF_APIENTRYP WaitSync)(GLsync sync, GLbitfield flags, GLuint64 timeout);
    GLenum (QOPENGLF_APIENTRYP ClientWaitSync)(GLsync sync, GLbitfield flags, GLuint64 timeout);
    void (QOPENGLF_APIENTRYP DeleteSync)(GLsync sync);
    GLboolean (QOPENGLF_APIENTRYP IsSync)(GLsync sync);
    GLsync (QOPENGLF_APIENTRYP FenceSync)(GLenum condition, GLbitfield flags);
    void (QOPENGLF_APIENTRYP ProvokingVertex)(GLenum mode);
    void (QOPENGLF_APIENTRYP MultiDrawElementsBaseVertex)(GLenum mode, const GLsizei *count, GLenum type, const GLvoid* const *indices, GLsizei drawcount, const GLint *basevertex);
    void (QOPENGLF_APIENTRYP DrawElementsInstancedBaseVertex)(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLsizei instancecount, GLint basevertex);
    void (QOPENGLF_APIENTRYP DrawRangeElementsBaseVertex)(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices, GLint basevertex);
    void (QOPENGLF_APIENTRYP DrawElementsBaseVertex)(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLint basevertex);
    void (QOPENGLF_APIENTRYP FramebufferTexture)(GLenum target, GLenum attachment, GLuint texture, GLint level);
    void (QOPENGLF_APIENTRYP GetBufferParameteri64v)(GLenum target, GLenum pname, GLint64 *params);
    void (QOPENGLF_APIENTRYP GetInteger64i_v)(GLenum target, GLuint index, GLint64 *data);

};

class QOpenGLFunctions_3_3_CoreBackend : public QOpenGLVersionFunctionsBackend
{
public:
    QOpenGLFunctions_3_3_CoreBackend(QOpenGLContext *context);

    static QOpenGLVersionStatus versionStatus();

    // OpenGL 3.3 core functions
    void (QOPENGLF_APIENTRYP VertexAttribP4uiv)(GLuint index, GLenum type, GLboolean normalized, const GLuint *value);
    void (QOPENGLF_APIENTRYP VertexAttribP4ui)(GLuint index, GLenum type, GLboolean normalized, GLuint value);
    void (QOPENGLF_APIENTRYP VertexAttribP3uiv)(GLuint index, GLenum type, GLboolean normalized, const GLuint *value);
    void (QOPENGLF_APIENTRYP VertexAttribP3ui)(GLuint index, GLenum type, GLboolean normalized, GLuint value);
    void (QOPENGLF_APIENTRYP VertexAttribP2uiv)(GLuint index, GLenum type, GLboolean normalized, const GLuint *value);
    void (QOPENGLF_APIENTRYP VertexAttribP2ui)(GLuint index, GLenum type, GLboolean normalized, GLuint value);
    void (QOPENGLF_APIENTRYP VertexAttribP1uiv)(GLuint index, GLenum type, GLboolean normalized, const GLuint *value);
    void (QOPENGLF_APIENTRYP VertexAttribP1ui)(GLuint index, GLenum type, GLboolean normalized, GLuint value);
    void (QOPENGLF_APIENTRYP SecondaryColorP3uiv)(GLenum type, const GLuint *color);
    void (QOPENGLF_APIENTRYP SecondaryColorP3ui)(GLenum type, GLuint color);
    void (QOPENGLF_APIENTRYP ColorP4uiv)(GLenum type, const GLuint *color);
    void (QOPENGLF_APIENTRYP ColorP4ui)(GLenum type, GLuint color);
    void (QOPENGLF_APIENTRYP ColorP3uiv)(GLenum type, const GLuint *color);
    void (QOPENGLF_APIENTRYP ColorP3ui)(GLenum type, GLuint color);
    void (QOPENGLF_APIENTRYP NormalP3uiv)(GLenum type, const GLuint *coords);
    void (QOPENGLF_APIENTRYP NormalP3ui)(GLenum type, GLuint coords);
    void (QOPENGLF_APIENTRYP MultiTexCoordP4uiv)(GLenum texture, GLenum type, const GLuint *coords);
    void (QOPENGLF_APIENTRYP MultiTexCoordP4ui)(GLenum texture, GLenum type, GLuint coords);
    void (QOPENGLF_APIENTRYP MultiTexCoordP3uiv)(GLenum texture, GLenum type, const GLuint *coords);
    void (QOPENGLF_APIENTRYP MultiTexCoordP3ui)(GLenum texture, GLenum type, GLuint coords);
    void (QOPENGLF_APIENTRYP MultiTexCoordP2uiv)(GLenum texture, GLenum type, const GLuint *coords);
    void (QOPENGLF_APIENTRYP MultiTexCoordP2ui)(GLenum texture, GLenum type, GLuint coords);
    void (QOPENGLF_APIENTRYP MultiTexCoordP1uiv)(GLenum texture, GLenum type, const GLuint *coords);
    void (QOPENGLF_APIENTRYP MultiTexCoordP1ui)(GLenum texture, GLenum type, GLuint coords);
    void (QOPENGLF_APIENTRYP TexCoordP4uiv)(GLenum type, const GLuint *coords);
    void (QOPENGLF_APIENTRYP TexCoordP4ui)(GLenum type, GLuint coords);
    void (QOPENGLF_APIENTRYP TexCoordP3uiv)(GLenum type, const GLuint *coords);
    void (QOPENGLF_APIENTRYP TexCoordP3ui)(GLenum type, GLuint coords);
    void (QOPENGLF_APIENTRYP TexCoordP2uiv)(GLenum type, const GLuint *coords);
    void (QOPENGLF_APIENTRYP TexCoordP2ui)(GLenum type, GLuint coords);
    void (QOPENGLF_APIENTRYP TexCoordP1uiv)(GLenum type, const GLuint *coords);
    void (QOPENGLF_APIENTRYP TexCoordP1ui)(GLenum type, GLuint coords);
    void (QOPENGLF_APIENTRYP VertexP4uiv)(GLenum type, const GLuint *value);
    void (QOPENGLF_APIENTRYP VertexP4ui)(GLenum type, GLuint value);
    void (QOPENGLF_APIENTRYP VertexP3uiv)(GLenum type, const GLuint *value);
    void (QOPENGLF_APIENTRYP VertexP3ui)(GLenum type, GLuint value);
    void (QOPENGLF_APIENTRYP VertexP2uiv)(GLenum type, const GLuint *value);
    void (QOPENGLF_APIENTRYP VertexP2ui)(GLenum type, GLuint value);
    void (QOPENGLF_APIENTRYP GetQueryObjectui64v)(GLuint id, GLenum pname, GLuint64 *params);
    void (QOPENGLF_APIENTRYP GetQueryObjecti64v)(GLuint id, GLenum pname, GLint64 *params);
    void (QOPENGLF_APIENTRYP QueryCounter)(GLuint id, GLenum target);
    void (QOPENGLF_APIENTRYP GetSamplerParameterIuiv)(GLuint sampler, GLenum pname, GLuint *params);
    void (QOPENGLF_APIENTRYP GetSamplerParameterfv)(GLuint sampler, GLenum pname, GLfloat *params);
    void (QOPENGLF_APIENTRYP GetSamplerParameterIiv)(GLuint sampler, GLenum pname, GLint *params);
    void (QOPENGLF_APIENTRYP GetSamplerParameteriv)(GLuint sampler, GLenum pname, GLint *params);
    void (QOPENGLF_APIENTRYP SamplerParameterIuiv)(GLuint sampler, GLenum pname, const GLuint *param);
    void (QOPENGLF_APIENTRYP SamplerParameterIiv)(GLuint sampler, GLenum pname, const GLint *param);
    void (QOPENGLF_APIENTRYP SamplerParameterfv)(GLuint sampler, GLenum pname, const GLfloat *param);
    void (QOPENGLF_APIENTRYP SamplerParameterf)(GLuint sampler, GLenum pname, GLfloat param);
    void (QOPENGLF_APIENTRYP SamplerParameteriv)(GLuint sampler, GLenum pname, const GLint *param);
    void (QOPENGLF_APIENTRYP SamplerParameteri)(GLuint sampler, GLenum pname, GLint param);
    void (QOPENGLF_APIENTRYP BindSampler)(GLuint unit, GLuint sampler);
    GLboolean (QOPENGLF_APIENTRYP IsSampler)(GLuint sampler);
    void (QOPENGLF_APIENTRYP DeleteSamplers)(GLsizei count, const GLuint *samplers);
    void (QOPENGLF_APIENTRYP GenSamplers)(GLsizei count, GLuint *samplers);
    GLint (QOPENGLF_APIENTRYP GetFragDataIndex)(GLuint program, const GLchar *name);
    void (QOPENGLF_APIENTRYP BindFragDataLocationIndexed)(GLuint program, GLuint colorNumber, GLuint index, const GLchar *name);
    void (QOPENGLF_APIENTRYP VertexAttribDivisor)(GLuint index, GLuint divisor);

};

class QOpenGLFunctions_4_0_CoreBackend : public QOpenGLVersionFunctionsBackend
{
public:
    QOpenGLFunctions_4_0_CoreBackend(QOpenGLContext *context);

    static QOpenGLVersionStatus versionStatus();

    // OpenGL 4.0 core functions
    void (QOPENGLF_APIENTRYP GetQueryIndexediv)(GLenum target, GLuint index, GLenum pname, GLint *params);
    void (QOPENGLF_APIENTRYP EndQueryIndexed)(GLenum target, GLuint index);
    void (QOPENGLF_APIENTRYP BeginQueryIndexed)(GLenum target, GLuint index, GLuint id);
    void (QOPENGLF_APIENTRYP DrawTransformFeedbackStream)(GLenum mode, GLuint id, GLuint stream);
    void (QOPENGLF_APIENTRYP DrawTransformFeedback)(GLenum mode, GLuint id);
    void (QOPENGLF_APIENTRYP ResumeTransformFeedback)();
    void (QOPENGLF_APIENTRYP PauseTransformFeedback)();
    GLboolean (QOPENGLF_APIENTRYP IsTransformFeedback)(GLuint id);
    void (QOPENGLF_APIENTRYP GenTransformFeedbacks)(GLsizei n, GLuint *ids);
    void (QOPENGLF_APIENTRYP DeleteTransformFeedbacks)(GLsizei n, const GLuint *ids);
    void (QOPENGLF_APIENTRYP BindTransformFeedback)(GLenum target, GLuint id);
    void (QOPENGLF_APIENTRYP PatchParameterfv)(GLenum pname, const GLfloat *values);
    void (QOPENGLF_APIENTRYP PatchParameteri)(GLenum pname, GLint value);
    void (QOPENGLF_APIENTRYP GetProgramStageiv)(GLuint program, GLenum shadertype, GLenum pname, GLint *values);
    void (QOPENGLF_APIENTRYP GetUniformSubroutineuiv)(GLenum shadertype, GLint location, GLuint *params);
    void (QOPENGLF_APIENTRYP UniformSubroutinesuiv)(GLenum shadertype, GLsizei count, const GLuint *indices);
    void (QOPENGLF_APIENTRYP GetActiveSubroutineName)(GLuint program, GLenum shadertype, GLuint index, GLsizei bufsize, GLsizei *length, GLchar *name);
    void (QOPENGLF_APIENTRYP GetActiveSubroutineUniformName)(GLuint program, GLenum shadertype, GLuint index, GLsizei bufsize, GLsizei *length, GLchar *name);
    void (QOPENGLF_APIENTRYP GetActiveSubroutineUniformiv)(GLuint program, GLenum shadertype, GLuint index, GLenum pname, GLint *values);
    GLuint (QOPENGLF_APIENTRYP GetSubroutineIndex)(GLuint program, GLenum shadertype, const GLchar *name);
    GLint (QOPENGLF_APIENTRYP GetSubroutineUniformLocation)(GLuint program, GLenum shadertype, const GLchar *name);
    void (QOPENGLF_APIENTRYP GetUniformdv)(GLuint program, GLint location, GLdouble *params);
    void (QOPENGLF_APIENTRYP UniformMatrix4x3dv)(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
    void (QOPENGLF_APIENTRYP UniformMatrix4x2dv)(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
    void (QOPENGLF_APIENTRYP UniformMatrix3x4dv)(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
    void (QOPENGLF_APIENTRYP UniformMatrix3x2dv)(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
    void (QOPENGLF_APIENTRYP UniformMatrix2x4dv)(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
    void (QOPENGLF_APIENTRYP UniformMatrix2x3dv)(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
    void (QOPENGLF_APIENTRYP UniformMatrix4dv)(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
    void (QOPENGLF_APIENTRYP UniformMatrix3dv)(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
    void (QOPENGLF_APIENTRYP UniformMatrix2dv)(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
    void (QOPENGLF_APIENTRYP Uniform4dv)(GLint location, GLsizei count, const GLdouble *value);
    void (QOPENGLF_APIENTRYP Uniform3dv)(GLint location, GLsizei count, const GLdouble *value);
    void (QOPENGLF_APIENTRYP Uniform2dv)(GLint location, GLsizei count, const GLdouble *value);
    void (QOPENGLF_APIENTRYP Uniform1dv)(GLint location, GLsizei count, const GLdouble *value);
    void (QOPENGLF_APIENTRYP Uniform4d)(GLint location, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
    void (QOPENGLF_APIENTRYP Uniform3d)(GLint location, GLdouble x, GLdouble y, GLdouble z);
    void (QOPENGLF_APIENTRYP Uniform2d)(GLint location, GLdouble x, GLdouble y);
    void (QOPENGLF_APIENTRYP Uniform1d)(GLint location, GLdouble x);
    void (QOPENGLF_APIENTRYP DrawElementsIndirect)(GLenum mode, GLenum type, const GLvoid *indirect);
    void (QOPENGLF_APIENTRYP DrawArraysIndirect)(GLenum mode, const GLvoid *indirect);
    void (QOPENGLF_APIENTRYP BlendFuncSeparatei)(GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);
    void (QOPENGLF_APIENTRYP BlendFunci)(GLuint buf, GLenum src, GLenum dst);
    void (QOPENGLF_APIENTRYP BlendEquationSeparatei)(GLuint buf, GLenum modeRGB, GLenum modeAlpha);
    void (QOPENGLF_APIENTRYP BlendEquationi)(GLuint buf, GLenum mode);
    void (QOPENGLF_APIENTRYP MinSampleShading)(GLfloat value);

};

class QOpenGLFunctions_4_1_CoreBackend : public QOpenGLVersionFunctionsBackend
{
public:
    QOpenGLFunctions_4_1_CoreBackend(QOpenGLContext *context);

    static QOpenGLVersionStatus versionStatus();

    // OpenGL 4.1 core functions
    void (QOPENGLF_APIENTRYP GetDoublei_v)(GLenum target, GLuint index, GLdouble *data);
    void (QOPENGLF_APIENTRYP GetFloati_v)(GLenum target, GLuint index, GLfloat *data);
    void (QOPENGLF_APIENTRYP DepthRangeIndexed)(GLuint index, GLdouble n, GLdouble f);
    void (QOPENGLF_APIENTRYP DepthRangeArrayv)(GLuint first, GLsizei count, const GLdouble *v);
    void (QOPENGLF_APIENTRYP ScissorIndexedv)(GLuint index, const GLint *v);
    void (QOPENGLF_APIENTRYP ScissorIndexed)(GLuint index, GLint left, GLint bottom, GLsizei width, GLsizei height);
    void (QOPENGLF_APIENTRYP ScissorArrayv)(GLuint first, GLsizei count, const GLint *v);
    void (QOPENGLF_APIENTRYP ViewportIndexedfv)(GLuint index, const GLfloat *v);
    void (QOPENGLF_APIENTRYP ViewportIndexedf)(GLuint index, GLfloat x, GLfloat y, GLfloat w, GLfloat h);
    void (QOPENGLF_APIENTRYP ViewportArrayv)(GLuint first, GLsizei count, const GLfloat *v);
    void (QOPENGLF_APIENTRYP GetVertexAttribLdv)(GLuint index, GLenum pname, GLdouble *params);
    void (QOPENGLF_APIENTRYP VertexAttribLPointer)(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
    void (QOPENGLF_APIENTRYP VertexAttribL4dv)(GLuint index, const GLdouble *v);
    void (QOPENGLF_APIENTRYP VertexAttribL3dv)(GLuint index, const GLdouble *v);
    void (QOPENGLF_APIENTRYP VertexAttribL2dv)(GLuint index, const GLdouble *v);
    void (QOPENGLF_APIENTRYP VertexAttribL1dv)(GLuint index, const GLdouble *v);
    void (QOPENGLF_APIENTRYP VertexAttribL4d)(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
    void (QOPENGLF_APIENTRYP VertexAttribL3d)(GLuint index, GLdouble x, GLdouble y, GLdouble z);
    void (QOPENGLF_APIENTRYP VertexAttribL2d)(GLuint index, GLdouble x, GLdouble y);
    void (QOPENGLF_APIENTRYP VertexAttribL1d)(GLuint index, GLdouble x);
    void (QOPENGLF_APIENTRYP GetProgramPipelineInfoLog)(GLuint pipeline, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
    void (QOPENGLF_APIENTRYP ValidateProgramPipeline)(GLuint pipeline);
    void (QOPENGLF_APIENTRYP ProgramUniformMatrix4x3dv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
    void (QOPENGLF_APIENTRYP ProgramUniformMatrix3x4dv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
    void (QOPENGLF_APIENTRYP ProgramUniformMatrix4x2dv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
    void (QOPENGLF_APIENTRYP ProgramUniformMatrix2x4dv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
    void (QOPENGLF_APIENTRYP ProgramUniformMatrix3x2dv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
    void (QOPENGLF_APIENTRYP ProgramUniformMatrix2x3dv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
    void (QOPENGLF_APIENTRYP ProgramUniformMatrix4x3fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    void (QOPENGLF_APIENTRYP ProgramUniformMatrix3x4fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    void (QOPENGLF_APIENTRYP ProgramUniformMatrix4x2fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    void (QOPENGLF_APIENTRYP ProgramUniformMatrix2x4fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    void (QOPENGLF_APIENTRYP ProgramUniformMatrix3x2fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    void (QOPENGLF_APIENTRYP ProgramUniformMatrix2x3fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    void (QOPENGLF_APIENTRYP ProgramUniformMatrix4dv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
    void (QOPENGLF_APIENTRYP ProgramUniformMatrix3dv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
    void (QOPENGLF_APIENTRYP ProgramUniformMatrix2dv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
    void (QOPENGLF_APIENTRYP ProgramUniformMatrix4fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    void (QOPENGLF_APIENTRYP ProgramUniformMatrix3fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    void (QOPENGLF_APIENTRYP ProgramUniformMatrix2fv)(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    void (QOPENGLF_APIENTRYP ProgramUniform4uiv)(GLuint program, GLint location, GLsizei count, const GLuint *value);
    void (QOPENGLF_APIENTRYP ProgramUniform4ui)(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
    void (QOPENGLF_APIENTRYP ProgramUniform4dv)(GLuint program, GLint location, GLsizei count, const GLdouble *value);
    void (QOPENGLF_APIENTRYP ProgramUniform4d)(GLuint program, GLint location, GLdouble v0, GLdouble v1, GLdouble v2, GLdouble v3);
    void (QOPENGLF_APIENTRYP ProgramUniform4fv)(GLuint program, GLint location, GLsizei count, const GLfloat *value);
    void (QOPENGLF_APIENTRYP ProgramUniform4f)(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
    void (QOPENGLF_APIENTRYP ProgramUniform4iv)(GLuint program, GLint location, GLsizei count, const GLint *value);
    void (QOPENGLF_APIENTRYP ProgramUniform4i)(GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
    void (QOPENGLF_APIENTRYP ProgramUniform3uiv)(GLuint program, GLint location, GLsizei count, const GLuint *value);
    void (QOPENGLF_APIENTRYP ProgramUniform3ui)(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2);
    void (QOPENGLF_APIENTRYP ProgramUniform3dv)(GLuint program, GLint location, GLsizei count, const GLdouble *value);
    void (QOPENGLF_APIENTRYP ProgramUniform3d)(GLuint program, GLint location, GLdouble v0, GLdouble v1, GLdouble v2);
    void (QOPENGLF_APIENTRYP ProgramUniform3fv)(GLuint program, GLint location, GLsizei count, const GLfloat *value);
    void (QOPENGLF_APIENTRYP ProgramUniform3f)(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
    void (QOPENGLF_APIENTRYP ProgramUniform3iv)(GLuint program, GLint location, GLsizei count, const GLint *value);
    void (QOPENGLF_APIENTRYP ProgramUniform3i)(GLuint program, GLint location, GLint v0, GLint v1, GLint v2);
    void (QOPENGLF_APIENTRYP ProgramUniform2uiv)(GLuint program, GLint location, GLsizei count, const GLuint *value);
    void (QOPENGLF_APIENTRYP ProgramUniform2ui)(GLuint program, GLint location, GLuint v0, GLuint v1);
    void (QOPENGLF_APIENTRYP ProgramUniform2dv)(GLuint program, GLint location, GLsizei count, const GLdouble *value);
    void (QOPENGLF_APIENTRYP ProgramUniform2d)(GLuint program, GLint location, GLdouble v0, GLdouble v1);
    void (QOPENGLF_APIENTRYP ProgramUniform2fv)(GLuint program, GLint location, GLsizei count, const GLfloat *value);
    void (QOPENGLF_APIENTRYP ProgramUniform2f)(GLuint program, GLint location, GLfloat v0, GLfloat v1);
    void (QOPENGLF_APIENTRYP ProgramUniform2iv)(GLuint program, GLint location, GLsizei count, const GLint *value);
    void (QOPENGLF_APIENTRYP ProgramUniform2i)(GLuint program, GLint location, GLint v0, GLint v1);
    void (QOPENGLF_APIENTRYP ProgramUniform1uiv)(GLuint program, GLint location, GLsizei count, const GLuint *value);
    void (QOPENGLF_APIENTRYP ProgramUniform1ui)(GLuint program, GLint location, GLuint v0);
    void (QOPENGLF_APIENTRYP ProgramUniform1dv)(GLuint program, GLint location, GLsizei count, const GLdouble *value);
    void (QOPENGLF_APIENTRYP ProgramUniform1d)(GLuint program, GLint location, GLdouble v0);
    void (QOPENGLF_APIENTRYP ProgramUniform1fv)(GLuint program, GLint location, GLsizei count, const GLfloat *value);
    void (QOPENGLF_APIENTRYP ProgramUniform1f)(GLuint program, GLint location, GLfloat v0);
    void (QOPENGLF_APIENTRYP ProgramUniform1iv)(GLuint program, GLint location, GLsizei count, const GLint *value);
    void (QOPENGLF_APIENTRYP ProgramUniform1i)(GLuint program, GLint location, GLint v0);
    void (QOPENGLF_APIENTRYP GetProgramPipelineiv)(GLuint pipeline, GLenum pname, GLint *params);
    GLboolean (QOPENGLF_APIENTRYP IsProgramPipeline)(GLuint pipeline);
    void (QOPENGLF_APIENTRYP GenProgramPipelines)(GLsizei n, GLuint *pipelines);
    void (QOPENGLF_APIENTRYP DeleteProgramPipelines)(GLsizei n, const GLuint *pipelines);
    void (QOPENGLF_APIENTRYP BindProgramPipeline)(GLuint pipeline);
    GLuint (QOPENGLF_APIENTRYP CreateShaderProgramv)(GLenum type, GLsizei count, const GLchar* const *strings);
    void (QOPENGLF_APIENTRYP ActiveShaderProgram)(GLuint pipeline, GLuint program);
    void (QOPENGLF_APIENTRYP UseProgramStages)(GLuint pipeline, GLbitfield stages, GLuint program);
    void (QOPENGLF_APIENTRYP ProgramParameteri)(GLuint program, GLenum pname, GLint value);
    void (QOPENGLF_APIENTRYP ProgramBinary)(GLuint program, GLenum binaryFormat, const GLvoid *binary, GLsizei length);
    void (QOPENGLF_APIENTRYP GetProgramBinary)(GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, GLvoid *binary);
    void (QOPENGLF_APIENTRYP ClearDepthf)(GLfloat dd);
    void (QOPENGLF_APIENTRYP DepthRangef)(GLfloat n, GLfloat f);
    void (QOPENGLF_APIENTRYP GetShaderPrecisionFormat)(GLenum shadertype, GLenum precisiontype, GLint *range, GLint *precision);
    void (QOPENGLF_APIENTRYP ShaderBinary)(GLsizei count, const GLuint *shaders, GLenum binaryformat, const GLvoid *binary, GLsizei length);
    void (QOPENGLF_APIENTRYP ReleaseShaderCompiler)();

};

class QOpenGLFunctions_4_2_CoreBackend : public QOpenGLVersionFunctionsBackend
{
public:
    QOpenGLFunctions_4_2_CoreBackend(QOpenGLContext *context);

    static QOpenGLVersionStatus versionStatus();

    // OpenGL 4.2 core functions
    void (QOPENGLF_APIENTRYP TexStorage3D)(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
    void (QOPENGLF_APIENTRYP TexStorage2D)(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
    void (QOPENGLF_APIENTRYP TexStorage1D)(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width);
    void (QOPENGLF_APIENTRYP MemoryBarrier)(GLbitfield barriers);
    void (QOPENGLF_APIENTRYP BindImageTexture)(GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format);
    void (QOPENGLF_APIENTRYP GetActiveAtomicCounterBufferiv)(GLuint program, GLuint bufferIndex, GLenum pname, GLint *params);
    void (QOPENGLF_APIENTRYP GetInternalformativ)(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint *params);
    void (QOPENGLF_APIENTRYP DrawTransformFeedbackStreamInstanced)(GLenum mode, GLuint id, GLuint stream, GLsizei instancecount);
    void (QOPENGLF_APIENTRYP DrawTransformFeedbackInstanced)(GLenum mode, GLuint id, GLsizei instancecount);
    void (QOPENGLF_APIENTRYP DrawElementsInstancedBaseVertexBaseInstance)(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex, GLuint baseinstance);
    void (QOPENGLF_APIENTRYP DrawElementsInstancedBaseInstance)(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLuint baseinstance);
    void (QOPENGLF_APIENTRYP DrawArraysInstancedBaseInstance)(GLenum mode, GLint first, GLsizei count, GLsizei instancecount, GLuint baseinstance);

};

class QOpenGLFunctions_4_3_CoreBackend : public QOpenGLVersionFunctionsBackend
{
public:
    QOpenGLFunctions_4_3_CoreBackend(QOpenGLContext *context);

    static QOpenGLVersionStatus versionStatus();

    // OpenGL 4.3 core functions
    void (QOPENGLF_APIENTRYP TexStorage3DMultisample)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations);
    void (QOPENGLF_APIENTRYP TexStorage2DMultisample)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
    void (QOPENGLF_APIENTRYP TexBufferRange)(GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size);
    void (QOPENGLF_APIENTRYP ShaderStorageBlockBinding)(GLuint program, GLuint storageBlockIndex, GLuint storageBlockBinding);
    GLint (QOPENGLF_APIENTRYP GetProgramResourceLocationIndex)(GLuint program, GLenum programInterface, const GLchar *name);
    GLint (QOPENGLF_APIENTRYP GetProgramResourceLocation)(GLuint program, GLenum programInterface, const GLchar *name);
    void (QOPENGLF_APIENTRYP GetProgramResourceiv)(GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum *props, GLsizei bufSize, GLsizei *length, GLint *params);
    void (QOPENGLF_APIENTRYP GetProgramResourceName)(GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name);
    GLuint (QOPENGLF_APIENTRYP GetProgramResourceIndex)(GLuint program, GLenum programInterface, const GLchar *name);
    void (QOPENGLF_APIENTRYP GetProgramInterfaceiv)(GLuint program, GLenum programInterface, GLenum pname, GLint *params);
    void (QOPENGLF_APIENTRYP MultiDrawElementsIndirect)(GLenum mode, GLenum type, const void *indirect, GLsizei drawcount, GLsizei stride);
    void (QOPENGLF_APIENTRYP MultiDrawArraysIndirect)(GLenum mode, const void *indirect, GLsizei drawcount, GLsizei stride);
    void (QOPENGLF_APIENTRYP InvalidateSubFramebuffer)(GLenum target, GLsizei numAttachments, const GLenum *attachments, GLint x, GLint y, GLsizei width, GLsizei height);
    void (QOPENGLF_APIENTRYP InvalidateFramebuffer)(GLenum target, GLsizei numAttachments, const GLenum *attachments);
    void (QOPENGLF_APIENTRYP InvalidateBufferData)(GLuint buffer);
    void (QOPENGLF_APIENTRYP InvalidateBufferSubData)(GLuint buffer, GLintptr offset, GLsizeiptr length);
    void (QOPENGLF_APIENTRYP InvalidateTexImage)(GLuint texture, GLint level);
    void (QOPENGLF_APIENTRYP InvalidateTexSubImage)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth);
    void (QOPENGLF_APIENTRYP GetInternalformati64v)(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint64 *params);
    void (QOPENGLF_APIENTRYP GetFramebufferParameteriv)(GLenum target, GLenum pname, GLint *params);
    void (QOPENGLF_APIENTRYP FramebufferParameteri)(GLenum target, GLenum pname, GLint param);
    void (QOPENGLF_APIENTRYP VertexBindingDivisor)(GLuint bindingindex, GLuint divisor);
    void (QOPENGLF_APIENTRYP VertexAttribBinding)(GLuint attribindex, GLuint bindingindex);
    void (QOPENGLF_APIENTRYP VertexAttribLFormat)(GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
    void (QOPENGLF_APIENTRYP VertexAttribIFormat)(GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
    void (QOPENGLF_APIENTRYP VertexAttribFormat)(GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset);
    void (QOPENGLF_APIENTRYP BindVertexBuffer)(GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride);
    void (QOPENGLF_APIENTRYP TextureView)(GLuint texture, GLenum target, GLuint origtexture, GLenum internalformat, GLuint minlevel, GLuint numlevels, GLuint minlayer, GLuint numlayers);
    void (QOPENGLF_APIENTRYP CopyImageSubData)(GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth);
    void (QOPENGLF_APIENTRYP DispatchComputeIndirect)(GLintptr indirect);
    void (QOPENGLF_APIENTRYP DispatchCompute)(GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z);
    void (QOPENGLF_APIENTRYP ClearBufferSubData)(GLenum target, GLenum internalformat, GLintptr offset, GLsizeiptr size, GLenum format, GLenum type, const void *data);
    void (QOPENGLF_APIENTRYP ClearBufferData)(GLenum target, GLenum internalformat, GLenum format, GLenum type, const void *data);
    void (QOPENGLF_APIENTRYP GetObjectPtrLabel)(const GLvoid *ptr, GLsizei bufSize, GLsizei *length, GLchar *label);
    void (QOPENGLF_APIENTRYP ObjectPtrLabel)(const GLvoid *ptr, GLsizei length, const GLchar *label);
    void (QOPENGLF_APIENTRYP GetObjectLabel)(GLenum identifier, GLuint name, GLsizei bufSize, GLsizei *length, GLchar *label);
    void (QOPENGLF_APIENTRYP ObjectLabel)(GLenum identifier, GLuint name, GLsizei length, const GLchar *label);
    void (QOPENGLF_APIENTRYP PopDebugGroup)();
    void (QOPENGLF_APIENTRYP PushDebugGroup)(GLenum source, GLuint id, GLsizei length, const GLchar *message);
    GLuint (QOPENGLF_APIENTRYP GetDebugMessageLog)(GLuint count, GLsizei bufSize, GLenum *sources, GLenum *types, GLuint *ids, GLenum *severities, GLsizei *lengths, GLchar *messageLog);
    void (QOPENGLF_APIENTRYP DebugMessageCallback)(GLDEBUGPROC callback, const GLvoid *userParam);
    void (QOPENGLF_APIENTRYP DebugMessageInsert)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *buf);
    void (QOPENGLF_APIENTRYP DebugMessageControl)(GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint *ids, GLboolean enabled);

};

class QOpenGLFunctions_4_4_CoreBackend : public QOpenGLVersionFunctionsBackend
{
public:
    QOpenGLFunctions_4_4_CoreBackend(QOpenGLContext *context);

    static QOpenGLVersionStatus versionStatus();

    // OpenGL 4.4 core functions
    void (QOPENGLF_APIENTRYP BindVertexBuffers)(GLuint first, GLsizei count, const GLuint *buffers, const GLintptr *offsets, const GLsizei *strides);
    void (QOPENGLF_APIENTRYP BindImageTextures)(GLuint first, GLsizei count, const GLuint *textures);
    void (QOPENGLF_APIENTRYP BindSamplers)(GLuint first, GLsizei count, const GLuint *samplers);
    void (QOPENGLF_APIENTRYP BindTextures)(GLuint first, GLsizei count, const GLuint *textures);
    void (QOPENGLF_APIENTRYP BindBuffersRange)(GLenum target, GLuint first, GLsizei count, const GLuint *buffers, const GLintptr *offsets, const GLsizeiptr *sizes);
    void (QOPENGLF_APIENTRYP BindBuffersBase)(GLenum target, GLuint first, GLsizei count, const GLuint *buffers);
    void (QOPENGLF_APIENTRYP ClearTexSubImage)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *data);
    void (QOPENGLF_APIENTRYP ClearTexImage)(GLuint texture, GLint level, GLenum format, GLenum type, const void *data);
    void (QOPENGLF_APIENTRYP BufferStorage)(GLenum target, GLsizeiptr size, const void *data, GLbitfield flags);

};

class QOpenGLFunctions_4_5_CoreBackend : public QOpenGLVersionFunctionsBackend
{
public:
    QOpenGLFunctions_4_5_CoreBackend(QOpenGLContext *context);

    static QOpenGLVersionStatus versionStatus();

    // OpenGL 4.5 core functions
    void (QOPENGLF_APIENTRYP TextureBarrier)();
    void (QOPENGLF_APIENTRYP ReadnPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, void *data);
    void (QOPENGLF_APIENTRYP GetnUniformuiv)(GLuint program, GLint location, GLsizei bufSize, GLuint *params);
    void (QOPENGLF_APIENTRYP GetnUniformiv)(GLuint program, GLint location, GLsizei bufSize, GLint *params);
    void (QOPENGLF_APIENTRYP GetnUniformfv)(GLuint program, GLint location, GLsizei bufSize, GLfloat *params);
    void (QOPENGLF_APIENTRYP GetnUniformdv)(GLuint program, GLint location, GLsizei bufSize, GLdouble *params);
    void (QOPENGLF_APIENTRYP GetnTexImage)(GLenum target, GLint level, GLenum format, GLenum type, GLsizei bufSize, void *pixels);
    void (QOPENGLF_APIENTRYP GetnCompressedTexImage)(GLenum target, GLint lod, GLsizei bufSize, void *pixels);
    GLenum (QOPENGLF_APIENTRYP GetGraphicsResetStatus)();
    void (QOPENGLF_APIENTRYP GetCompressedTextureSubImage)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLsizei bufSize, void *pixels);
    void (QOPENGLF_APIENTRYP GetTextureSubImage)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, GLsizei bufSize, void *pixels);
    void (QOPENGLF_APIENTRYP MemoryBarrierByRegion)(GLbitfield barriers);
    void (QOPENGLF_APIENTRYP CreateQueries)(GLenum target, GLsizei n, GLuint *ids);
    void (QOPENGLF_APIENTRYP CreateProgramPipelines)(GLsizei n, GLuint *pipelines);
    void (QOPENGLF_APIENTRYP CreateSamplers)(GLsizei n, GLuint *samplers);
    void (QOPENGLF_APIENTRYP GetVertexArrayIndexed64iv)(GLuint vaobj, GLuint index, GLenum pname, GLint64 *param);
    void (QOPENGLF_APIENTRYP GetVertexArrayIndexediv)(GLuint vaobj, GLuint index, GLenum pname, GLint *param);
    void (QOPENGLF_APIENTRYP GetVertexArrayiv)(GLuint vaobj, GLenum pname, GLint *param);
    void (QOPENGLF_APIENTRYP VertexArrayBindingDivisor)(GLuint vaobj, GLuint bindingindex, GLuint divisor);
    void (QOPENGLF_APIENTRYP VertexArrayAttribLFormat)(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
    void (QOPENGLF_APIENTRYP VertexArrayAttribIFormat)(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
    void (QOPENGLF_APIENTRYP VertexArrayAttribFormat)(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset);
    void (QOPENGLF_APIENTRYP VertexArrayAttribBinding)(GLuint vaobj, GLuint attribindex, GLuint bindingindex);
    void (QOPENGLF_APIENTRYP VertexArrayVertexBuffers)(GLuint vaobj, GLuint first, GLsizei count, const GLuint *buffers, const GLintptr *offsets, const GLsizei *strides);
    void (QOPENGLF_APIENTRYP VertexArrayVertexBuffer)(GLuint vaobj, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride);
    void (QOPENGLF_APIENTRYP VertexArrayElementBuffer)(GLuint vaobj, GLuint buffer);
    void (QOPENGLF_APIENTRYP EnableVertexArrayAttrib)(GLuint vaobj, GLuint index);
    void (QOPENGLF_APIENTRYP DisableVertexArrayAttrib)(GLuint vaobj, GLuint index);
    void (QOPENGLF_APIENTRYP CreateVertexArrays)(GLsizei n, GLuint *arrays);
    void (QOPENGLF_APIENTRYP GetTextureParameteriv)(GLuint texture, GLenum pname, GLint *params);
    void (QOPENGLF_APIENTRYP GetTextureParameterIuiv)(GLuint texture, GLenum pname, GLuint *params);
    void (QOPENGLF_APIENTRYP GetTextureParameterIiv)(GLuint texture, GLenum pname, GLint *params);
    void (QOPENGLF_APIENTRYP GetTextureParameterfv)(GLuint texture, GLenum pname, GLfloat *params);
    void (QOPENGLF_APIENTRYP GetTextureLevelParameteriv)(GLuint texture, GLint level, GLenum pname, GLint *params);
    void (QOPENGLF_APIENTRYP GetTextureLevelParameterfv)(GLuint texture, GLint level, GLenum pname, GLfloat *params);
    void (QOPENGLF_APIENTRYP GetCompressedTextureImage)(GLuint texture, GLint level, GLsizei bufSize, void *pixels);
    void (QOPENGLF_APIENTRYP GetTextureImage)(GLuint texture, GLint level, GLenum format, GLenum type, GLsizei bufSize, void *pixels);
    void (QOPENGLF_APIENTRYP BindTextureUnit)(GLuint unit, GLuint texture);
    void (QOPENGLF_APIENTRYP GenerateTextureMipmap)(GLuint texture);
    void (QOPENGLF_APIENTRYP TextureParameteriv)(GLuint texture, GLenum pname, const GLint *param);
    void (QOPENGLF_APIENTRYP TextureParameterIuiv)(GLuint texture, GLenum pname, const GLuint *params);
    void (QOPENGLF_APIENTRYP TextureParameterIiv)(GLuint texture, GLenum pname, const GLint *params);
    void (QOPENGLF_APIENTRYP TextureParameteri)(GLuint texture, GLenum pname, GLint param);
    void (QOPENGLF_APIENTRYP TextureParameterfv)(GLuint texture, GLenum pname, const GLfloat *param);
    void (QOPENGLF_APIENTRYP TextureParameterf)(GLuint texture, GLenum pname, GLfloat param);
    void (QOPENGLF_APIENTRYP CopyTextureSubImage3D)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
    void (QOPENGLF_APIENTRYP CopyTextureSubImage2D)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
    void (QOPENGLF_APIENTRYP CopyTextureSubImage1D)(GLuint texture, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
    void (QOPENGLF_APIENTRYP CompressedTextureSubImage3D)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data);
    void (QOPENGLF_APIENTRYP CompressedTextureSubImage2D)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data);
    void (QOPENGLF_APIENTRYP CompressedTextureSubImage1D)(GLuint texture, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void *data);
    void (QOPENGLF_APIENTRYP TextureSubImage3D)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels);
    void (QOPENGLF_APIENTRYP TextureSubImage2D)(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
    void (QOPENGLF_APIENTRYP TextureSubImage1D)(GLuint texture, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void *pixels);
    void (QOPENGLF_APIENTRYP TextureStorage3DMultisample)(GLuint texture, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations);
    void (QOPENGLF_APIENTRYP TextureStorage2DMultisample)(GLuint texture, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
    void (QOPENGLF_APIENTRYP TextureStorage3D)(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
    void (QOPENGLF_APIENTRYP TextureStorage2D)(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
    void (QOPENGLF_APIENTRYP TextureStorage1D)(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width);
    void (QOPENGLF_APIENTRYP TextureBufferRange)(GLuint texture, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizei size);
    void (QOPENGLF_APIENTRYP TextureBuffer)(GLuint texture, GLenum internalformat, GLuint buffer);
    void (QOPENGLF_APIENTRYP CreateTextures)(GLenum target, GLsizei n, GLuint *textures);
    void (QOPENGLF_APIENTRYP GetNamedRenderbufferParameteriv)(GLuint renderbuffer, GLenum pname, GLint *params);
    void (QOPENGLF_APIENTRYP NamedRenderbufferStorageMultisample)(GLuint renderbuffer, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
    void (QOPENGLF_APIENTRYP NamedRenderbufferStorage)(GLuint renderbuffer, GLenum internalformat, GLsizei width, GLsizei height);
    void (QOPENGLF_APIENTRYP CreateRenderbuffers)(GLsizei n, GLuint *renderbuffers);
    void (QOPENGLF_APIENTRYP GetNamedFramebufferAttachmentParameteriv)(GLuint framebuffer, GLenum attachment, GLenum pname, GLint *params);
    void (QOPENGLF_APIENTRYP GetNamedFramebufferParameteriv)(GLuint framebuffer, GLenum pname, GLint *param);
    GLenum (QOPENGLF_APIENTRYP CheckNamedFramebufferStatus)(GLuint framebuffer, GLenum target);
    void (QOPENGLF_APIENTRYP BlitNamedFramebuffer)(GLuint readFramebuffer, GLuint drawFramebuffer, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
    void (QOPENGLF_APIENTRYP ClearNamedFramebufferfi)(GLuint framebuffer, GLenum buffer, GLfloat depth, GLint stencil);
    void (QOPENGLF_APIENTRYP ClearNamedFramebufferfv)(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLfloat *value);
    void (QOPENGLF_APIENTRYP ClearNamedFramebufferuiv)(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLuint *value);
    void (QOPENGLF_APIENTRYP ClearNamedFramebufferiv)(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLint *value);
    void (QOPENGLF_APIENTRYP InvalidateNamedFramebufferSubData)(GLuint framebuffer, GLsizei numAttachments, const GLenum *attachments, GLint x, GLint y, GLsizei width, GLsizei height);
    void (QOPENGLF_APIENTRYP InvalidateNamedFramebufferData)(GLuint framebuffer, GLsizei numAttachments, const GLenum *attachments);
    void (QOPENGLF_APIENTRYP NamedFramebufferReadBuffer)(GLuint framebuffer, GLenum src);
    void (QOPENGLF_APIENTRYP NamedFramebufferDrawBuffers)(GLuint framebuffer, GLsizei n, const GLenum *bufs);
    void (QOPENGLF_APIENTRYP NamedFramebufferDrawBuffer)(GLuint framebuffer, GLenum buf);
    void (QOPENGLF_APIENTRYP NamedFramebufferTextureLayer)(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level, GLint layer);
    void (QOPENGLF_APIENTRYP NamedFramebufferTexture)(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level);
    void (QOPENGLF_APIENTRYP NamedFramebufferParameteri)(GLuint framebuffer, GLenum pname, GLint param);
    void (QOPENGLF_APIENTRYP NamedFramebufferRenderbuffer)(GLuint framebuffer, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
    void (QOPENGLF_APIENTRYP CreateFramebuffers)(GLsizei n, GLuint *framebuffers);
    void (QOPENGLF_APIENTRYP GetNamedBufferSubData)(GLuint buffer, GLintptr offset, GLsizei size, void *data);
    void (QOPENGLF_APIENTRYP GetNamedBufferPointerv)(GLuint buffer, GLenum pname, GLvoid* *params);
    void (QOPENGLF_APIENTRYP GetNamedBufferParameteri64v)(GLuint buffer, GLenum pname, GLint64 *params);
    void (QOPENGLF_APIENTRYP GetNamedBufferParameteriv)(GLuint buffer, GLenum pname, GLint *params);
    void (QOPENGLF_APIENTRYP FlushMappedNamedBufferRange)(GLuint buffer, GLintptr offset, GLsizei length);
    GLboolean (QOPENGLF_APIENTRYP UnmapNamedBuffer)(GLuint buffer);
    GLvoid* (QOPENGLF_APIENTRYP MapNamedBufferRange)(GLuint buffer, GLintptr offset, GLsizei length, GLbitfield access);
    GLvoid* (QOPENGLF_APIENTRYP MapNamedBuffer)(GLuint buffer, GLenum access);
    void (QOPENGLF_APIENTRYP ClearNamedBufferSubData)(GLuint buffer, GLenum internalformat, GLintptr offset, GLsizei size, GLenum format, GLenum type, const void *data);
    void (QOPENGLF_APIENTRYP ClearNamedBufferData)(GLuint buffer, GLenum internalformat, GLenum format, GLenum type, const void *data);
    void (QOPENGLF_APIENTRYP CopyNamedBufferSubData)(GLuint readBuffer, GLuint writeBuffer, GLintptr readOffset, GLintptr writeOffset, GLsizei size);
    void (QOPENGLF_APIENTRYP NamedBufferSubData)(GLuint buffer, GLintptr offset, GLsizei size, const void *data);
    void (QOPENGLF_APIENTRYP NamedBufferData)(GLuint buffer, GLsizei size, const void *data, GLenum usage);
    void (QOPENGLF_APIENTRYP NamedBufferStorage)(GLuint buffer, GLsizei size, const void *data, GLbitfield flags);
    void (QOPENGLF_APIENTRYP CreateBuffers)(GLsizei n, GLuint *buffers);
    void (QOPENGLF_APIENTRYP GetTransformFeedbacki64_v)(GLuint xfb, GLenum pname, GLuint index, GLint64 *param);
    void (QOPENGLF_APIENTRYP GetTransformFeedbacki_v)(GLuint xfb, GLenum pname, GLuint index, GLint *param);
    void (QOPENGLF_APIENTRYP GetTransformFeedbackiv)(GLuint xfb, GLenum pname, GLint *param);
    void (QOPENGLF_APIENTRYP TransformFeedbackBufferRange)(GLuint xfb, GLuint index, GLuint buffer, GLintptr offset, GLsizei size);
    void (QOPENGLF_APIENTRYP TransformFeedbackBufferBase)(GLuint xfb, GLuint index, GLuint buffer);
    void (QOPENGLF_APIENTRYP CreateTransformFeedbacks)(GLsizei n, GLuint *ids);
    void (QOPENGLF_APIENTRYP ClipControl)(GLenum origin, GLenum depth);

};

class QOpenGLFunctions_1_0_DeprecatedBackend : public QOpenGLVersionFunctionsBackend
{
public:
    QOpenGLFunctions_1_0_DeprecatedBackend(QOpenGLContext *context);

    static QOpenGLVersionStatus versionStatus();

    // OpenGL 1.0 deprecated functions
    void (QOPENGLF_APIENTRYP Translatef)(GLfloat x, GLfloat y, GLfloat z);
    void (QOPENGLF_APIENTRYP Translated)(GLdouble x, GLdouble y, GLdouble z);
    void (QOPENGLF_APIENTRYP Scalef)(GLfloat x, GLfloat y, GLfloat z);
    void (QOPENGLF_APIENTRYP Scaled)(GLdouble x, GLdouble y, GLdouble z);
    void (QOPENGLF_APIENTRYP Rotatef)(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
    void (QOPENGLF_APIENTRYP Rotated)(GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
    void (QOPENGLF_APIENTRYP PushMatrix)();
    void (QOPENGLF_APIENTRYP PopMatrix)();
    void (QOPENGLF_APIENTRYP Ortho)(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
    void (QOPENGLF_APIENTRYP MultMatrixd)(const GLdouble *m);
    void (QOPENGLF_APIENTRYP MultMatrixf)(const GLfloat *m);
    void (QOPENGLF_APIENTRYP MatrixMode)(GLenum mode);
    void (QOPENGLF_APIENTRYP LoadMatrixd)(const GLdouble *m);
    void (QOPENGLF_APIENTRYP LoadMatrixf)(const GLfloat *m);
    void (QOPENGLF_APIENTRYP LoadIdentity)();
    void (QOPENGLF_APIENTRYP Frustum)(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
    GLboolean (QOPENGLF_APIENTRYP IsList)(GLuint list);
    void (QOPENGLF_APIENTRYP GetTexGeniv)(GLenum coord, GLenum pname, GLint *params);
    void (QOPENGLF_APIENTRYP GetTexGenfv)(GLenum coord, GLenum pname, GLfloat *params);
    void (QOPENGLF_APIENTRYP GetTexGendv)(GLenum coord, GLenum pname, GLdouble *params);
    void (QOPENGLF_APIENTRYP GetTexEnviv)(GLenum target, GLenum pname, GLint *params);
    void (QOPENGLF_APIENTRYP GetTexEnvfv)(GLenum target, GLenum pname, GLfloat *params);
    void (QOPENGLF_APIENTRYP GetPolygonStipple)(GLubyte *mask);
    void (QOPENGLF_APIENTRYP GetPixelMapusv)(GLenum map, GLushort *values);
    void (QOPENGLF_APIENTRYP GetPixelMapuiv)(GLenum map, GLuint *values);
    void (QOPENGLF_APIENTRYP GetPixelMapfv)(GLenum map, GLfloat *values);
    void (QOPENGLF_APIENTRYP GetMaterialiv)(GLenum face, GLenum pname, GLint *params);
    void (QOPENGLF_APIENTRYP GetMaterialfv)(GLenum face, GLenum pname, GLfloat *params);
    void (QOPENGLF_APIENTRYP GetMapiv)(GLenum target, GLenum query, GLint *v);
    void (QOPENGLF_APIENTRYP GetMapfv)(GLenum target, GLenum query, GLfloat *v);
    void (QOPENGLF_APIENTRYP GetMapdv)(GLenum target, GLenum query, GLdouble *v);
    void (QOPENGLF_APIENTRYP GetLightiv)(GLenum light, GLenum pname, GLint *params);
    void (QOPENGLF_APIENTRYP GetLightfv)(GLenum light, GLenum pname, GLfloat *params);
    void (QOPENGLF_APIENTRYP GetClipPlane)(GLenum plane, GLdouble *equation);
    void (QOPENGLF_APIENTRYP DrawPixels)(GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
    void (QOPENGLF_APIENTRYP CopyPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type);
    void (QOPENGLF_APIENTRYP PixelMapusv)(GLenum map, GLsizei mapsize, const GLushort *values);
    void (QOPENGLF_APIENTRYP PixelMapuiv)(GLenum map, GLsizei mapsize, const GLuint *values);
    void (QOPENGLF_APIENTRYP PixelMapfv)(GLenum map, GLsizei mapsize, const GLfloat *values);
    void (QOPENGLF_APIENTRYP PixelTransferi)(GLenum pname, GLint param);
    void (QOPENGLF_APIENTRYP PixelTransferf)(GLenum pname, GLfloat param);
    void (QOPENGLF_APIENTRYP PixelZoom)(GLfloat xfactor, GLfloat yfactor);
    void (QOPENGLF_APIENTRYP AlphaFunc)(GLenum func, GLfloat ref);
    void (QOPENGLF_APIENTRYP EvalPoint2)(GLint i, GLint j);
    void (QOPENGLF_APIENTRYP EvalMesh2)(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2);
    void (QOPENGLF_APIENTRYP EvalPoint1)(GLint i);
    void (QOPENGLF_APIENTRYP EvalMesh1)(GLenum mode, GLint i1, GLint i2);
    void (QOPENGLF_APIENTRYP EvalCoord2fv)(const GLfloat *u);
    void (QOPENGLF_APIENTRYP EvalCoord2f)(GLfloat u, GLfloat v);
    void (QOPENGLF_APIENTRYP EvalCoord2dv)(const GLdouble *u);
    void (QOPENGLF_APIENTRYP EvalCoord2d)(GLdouble u, GLdouble v);
    void (QOPENGLF_APIENTRYP EvalCoord1fv)(const GLfloat *u);
    void (QOPENGLF_APIENTRYP EvalCoord1f)(GLfloat u);
    void (QOPENGLF_APIENTRYP EvalCoord1dv)(const GLdouble *u);
    void (QOPENGLF_APIENTRYP EvalCoord1d)(GLdouble u);
    void (QOPENGLF_APIENTRYP MapGrid2f)(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2);
    void (QOPENGLF_APIENTRYP MapGrid2d)(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2);
    void (QOPENGLF_APIENTRYP MapGrid1f)(GLint un, GLfloat u1, GLfloat u2);
    void (QOPENGLF_APIENTRYP MapGrid1d)(GLint un, GLdouble u1, GLdouble u2);
    void (QOPENGLF_APIENTRYP Map2f)(GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points);
    void (QOPENGLF_APIENTRYP Map2d)(GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points);
    void (QOPENGLF_APIENTRYP Map1f)(GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points);
    void (QOPENGLF_APIENTRYP Map1d)(GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points);
    void (QOPENGLF_APIENTRYP PushAttrib)(GLbitfield mask);
    void (QOPENGLF_APIENTRYP PopAttrib)();
    void (QOPENGLF_APIENTRYP Accum)(GLenum op, GLfloat value);
    void (QOPENGLF_APIENTRYP IndexMask)(GLuint mask);
    void (QOPENGLF_APIENTRYP ClearIndex)(GLfloat c);
    void (QOPENGLF_APIENTRYP ClearAccum)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
    void (QOPENGLF_APIENTRYP PushName)(GLuint name);
    void (QOPENGLF_APIENTRYP PopName)();
    void (QOPENGLF_APIENTRYP PassThrough)(GLfloat token);
    void (QOPENGLF_APIENTRYP LoadName)(GLuint name);
    void (QOPENGLF_APIENTRYP InitNames)();
    GLint (QOPENGLF_APIENTRYP RenderMode)(GLenum mode);
    void (QOPENGLF_APIENTRYP SelectBuffer)(GLsizei size, GLuint *buffer);
    void (QOPENGLF_APIENTRYP FeedbackBuffer)(GLsizei size, GLenum type, GLfloat *buffer);
    void (QOPENGLF_APIENTRYP TexGeniv)(GLenum coord, GLenum pname, const GLint *params);
    void (QOPENGLF_APIENTRYP TexGeni)(GLenum coord, GLenum pname, GLint param);
    void (QOPENGLF_APIENTRYP TexGenfv)(GLenum coord, GLenum pname, const GLfloat *params);
    void (QOPENGLF_APIENTRYP TexGenf)(GLenum coord, GLenum pname, GLfloat param);
    void (QOPENGLF_APIENTRYP TexGendv)(GLenum coord, GLenum pname, const GLdouble *params);
    void (QOPENGLF_APIENTRYP TexGend)(GLenum coord, GLenum pname, GLdouble param);
    void (QOPENGLF_APIENTRYP TexEnviv)(GLenum target, GLenum pname, const GLint *params);
    void (QOPENGLF_APIENTRYP TexEnvi)(GLenum target, GLenum pname, GLint param);
    void (QOPENGLF_APIENTRYP TexEnvfv)(GLenum target, GLenum pname, const GLfloat *params);
    void (QOPENGLF_APIENTRYP TexEnvf)(GLenum target, GLenum pname, GLfloat param);
    void (QOPENGLF_APIENTRYP ShadeModel)(GLenum mode);
    void (QOPENGLF_APIENTRYP PolygonStipple)(const GLubyte *mask);
    void (QOPENGLF_APIENTRYP Materialiv)(GLenum face, GLenum pname, const GLint *params);
    void (QOPENGLF_APIENTRYP Materiali)(GLenum face, GLenum pname, GLint param);
    void (QOPENGLF_APIENTRYP Materialfv)(GLenum face, GLenum pname, const GLfloat *params);
    void (QOPENGLF_APIENTRYP Materialf)(GLenum face, GLenum pname, GLfloat param);
    void (QOPENGLF_APIENTRYP LineStipple)(GLint factor, GLushort pattern);
    void (QOPENGLF_APIENTRYP LightModeliv)(GLenum pname, const GLint *params);
    void (QOPENGLF_APIENTRYP LightModeli)(GLenum pname, GLint param);
    void (QOPENGLF_APIENTRYP LightModelfv)(GLenum pname, const GLfloat *params);
    void (QOPENGLF_APIENTRYP LightModelf)(GLenum pname, GLfloat param);
    void (QOPENGLF_APIENTRYP Lightiv)(GLenum light, GLenum pname, const GLint *params);
    void (QOPENGLF_APIENTRYP Lighti)(GLenum light, GLenum pname, GLint param);
    void (QOPENGLF_APIENTRYP Lightfv)(GLenum light, GLenum pname, const GLfloat *params);
    void (QOPENGLF_APIENTRYP Lightf)(GLenum light, GLenum pname, GLfloat param);
    void (QOPENGLF_APIENTRYP Fogiv)(GLenum pname, const GLint *params);
    void (QOPENGLF_APIENTRYP Fogi)(GLenum pname, GLint param);
    void (QOPENGLF_APIENTRYP Fogfv)(GLenum pname, const GLfloat *params);
    void (QOPENGLF_APIENTRYP Fogf)(GLenum pname, GLfloat param);
    void (QOPENGLF_APIENTRYP ColorMaterial)(GLenum face, GLenum mode);
    void (QOPENGLF_APIENTRYP ClipPlane)(GLenum plane, const GLdouble *equation);
    void (QOPENGLF_APIENTRYP Vertex4sv)(const GLshort *v);
    void (QOPENGLF_APIENTRYP Vertex4s)(GLshort x, GLshort y, GLshort z, GLshort w);
    void (QOPENGLF_APIENTRYP Vertex4iv)(const GLint *v);
    void (QOPENGLF_APIENTRYP Vertex4i)(GLint x, GLint y, GLint z, GLint w);
    void (QOPENGLF_APIENTRYP Vertex4fv)(const GLfloat *v);
    void (QOPENGLF_APIENTRYP Vertex4f)(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
    void (QOPENGLF_APIENTRYP Vertex4dv)(const GLdouble *v);
    void (QOPENGLF_APIENTRYP Vertex4d)(GLdouble x, GLdouble y, GLdouble z, GLdouble w);
    void (QOPENGLF_APIENTRYP Vertex3sv)(const GLshort *v);
    void (QOPENGLF_APIENTRYP Vertex3s)(GLshort x, GLshort y, GLshort z);
    void (QOPENGLF_APIENTRYP Vertex3iv)(const GLint *v);
    void (QOPENGLF_APIENTRYP Vertex3i)(GLint x, GLint y, GLint z);
    void (QOPENGLF_APIENTRYP Vertex3fv)(const GLfloat *v);
    void (QOPENGLF_APIENTRYP Vertex3f)(GLfloat x, GLfloat y, GLfloat z);
    void (QOPENGLF_APIENTRYP Vertex3dv)(const GLdouble *v);
    void (QOPENGLF_APIENTRYP Vertex3d)(GLdouble x, GLdouble y, GLdouble z);
    void (QOPENGLF_APIENTRYP Vertex2sv)(const GLshort *v);
    void (QOPENGLF_APIENTRYP Vertex2s)(GLshort x, GLshort y);
    void (QOPENGLF_APIENTRYP Vertex2iv)(const GLint *v);
    void (QOPENGLF_APIENTRYP Vertex2i)(GLint x, GLint y);
    void (QOPENGLF_APIENTRYP Vertex2fv)(const GLfloat *v);
    void (QOPENGLF_APIENTRYP Vertex2f)(GLfloat x, GLfloat y);
    void (QOPENGLF_APIENTRYP Vertex2dv)(const GLdouble *v);
    void (QOPENGLF_APIENTRYP Vertex2d)(GLdouble x, GLdouble y);
    void (QOPENGLF_APIENTRYP TexCoord4sv)(const GLshort *v);
    void (QOPENGLF_APIENTRYP TexCoord4s)(GLshort s, GLshort t, GLshort r, GLshort q);
    void (QOPENGLF_APIENTRYP TexCoord4iv)(const GLint *v);
    void (QOPENGLF_APIENTRYP TexCoord4i)(GLint s, GLint t, GLint r, GLint q);
    void (QOPENGLF_APIENTRYP TexCoord4fv)(const GLfloat *v);
    void (QOPENGLF_APIENTRYP TexCoord4f)(GLfloat s, GLfloat t, GLfloat r, GLfloat q);
    void (QOPENGLF_APIENTRYP TexCoord4dv)(const GLdouble *v);
    void (QOPENGLF_APIENTRYP TexCoord4d)(GLdouble s, GLdouble t, GLdouble r, GLdouble q);
    void (QOPENGLF_APIENTRYP TexCoord3sv)(const GLshort *v);
    void (QOPENGLF_APIENTRYP TexCoord3s)(GLshort s, GLshort t, GLshort r);
    void (QOPENGLF_APIENTRYP TexCoord3iv)(const GLint *v);
    void (QOPENGLF_APIENTRYP TexCoord3i)(GLint s, GLint t, GLint r);
    void (QOPENGLF_APIENTRYP TexCoord3fv)(const GLfloat *v);
    void (QOPENGLF_APIENTRYP TexCoord3f)(GLfloat s, GLfloat t, GLfloat r);
    void (QOPENGLF_APIENTRYP TexCoord3dv)(const GLdouble *v);
    void (QOPENGLF_APIENTRYP TexCoord3d)(GLdouble s, GLdouble t, GLdouble r);
    void (QOPENGLF_APIENTRYP TexCoord2sv)(const GLshort *v);
    void (QOPENGLF_APIENTRYP TexCoord2s)(GLshort s, GLshort t);
    void (QOPENGLF_APIENTRYP TexCoord2iv)(const GLint *v);
    void (QOPENGLF_APIENTRYP TexCoord2i)(GLint s, GLint t);
    void (QOPENGLF_APIENTRYP TexCoord2fv)(const GLfloat *v);
    void (QOPENGLF_APIENTRYP TexCoord2f)(GLfloat s, GLfloat t);
    void (QOPENGLF_APIENTRYP TexCoord2dv)(const GLdouble *v);
    void (QOPENGLF_APIENTRYP TexCoord2d)(GLdouble s, GLdouble t);
    void (QOPENGLF_APIENTRYP TexCoord1sv)(const GLshort *v);
    void (QOPENGLF_APIENTRYP TexCoord1s)(GLshort s);
    void (QOPENGLF_APIENTRYP TexCoord1iv)(const GLint *v);
    void (QOPENGLF_APIENTRYP TexCoord1i)(GLint s);
    void (QOPENGLF_APIENTRYP TexCoord1fv)(const GLfloat *v);
    void (QOPENGLF_APIENTRYP TexCoord1f)(GLfloat s);
    void (QOPENGLF_APIENTRYP TexCoord1dv)(const GLdouble *v);
    void (QOPENGLF_APIENTRYP TexCoord1d)(GLdouble s);
    void (QOPENGLF_APIENTRYP Rectsv)(const GLshort *v1, const GLshort *v2);
    void (QOPENGLF_APIENTRYP Rects)(GLshort x1, GLshort y1, GLshort x2, GLshort y2);
    void (QOPENGLF_APIENTRYP Rectiv)(const GLint *v1, const GLint *v2);
    void (QOPENGLF_APIENTRYP Recti)(GLint x1, GLint y1, GLint x2, GLint y2);
    void (QOPENGLF_APIENTRYP Rectfv)(const GLfloat *v1, const GLfloat *v2);
    void (QOPENGLF_APIENTRYP Rectf)(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
    void (QOPENGLF_APIENTRYP Rectdv)(const GLdouble *v1, const GLdouble *v2);
    void (QOPENGLF_APIENTRYP Rectd)(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2);
    void (QOPENGLF_APIENTRYP RasterPos4sv)(const GLshort *v);
    void (QOPENGLF_APIENTRYP RasterPos4s)(GLshort x, GLshort y, GLshort z, GLshort w);
    void (QOPENGLF_APIENTRYP RasterPos4iv)(const GLint *v);
    void (QOPENGLF_APIENTRYP RasterPos4i)(GLint x, GLint y, GLint z, GLint w);
    void (QOPENGLF_APIENTRYP RasterPos4fv)(const GLfloat *v);
    void (QOPENGLF_APIENTRYP RasterPos4f)(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
    void (QOPENGLF_APIENTRYP RasterPos4dv)(const GLdouble *v);
    void (QOPENGLF_APIENTRYP RasterPos4d)(GLdouble x, GLdouble y, GLdouble z, GLdouble w);
    void (QOPENGLF_APIENTRYP RasterPos3sv)(const GLshort *v);
    void (QOPENGLF_APIENTRYP RasterPos3s)(GLshort x, GLshort y, GLshort z);
    void (QOPENGLF_APIENTRYP RasterPos3iv)(const GLint *v);
    void (QOPENGLF_APIENTRYP RasterPos3i)(GLint x, GLint y, GLint z);
    void (QOPENGLF_APIENTRYP RasterPos3fv)(const GLfloat *v);
    void (QOPENGLF_APIENTRYP RasterPos3f)(GLfloat x, GLfloat y, GLfloat z);
    void (QOPENGLF_APIENTRYP RasterPos3dv)(const GLdouble *v);
    void (QOPENGLF_APIENTRYP RasterPos3d)(GLdouble x, GLdouble y, GLdouble z);
    void (QOPENGLF_APIENTRYP RasterPos2sv)(const GLshort *v);
    void (QOPENGLF_APIENTRYP RasterPos2s)(GLshort x, GLshort y);
    void (QOPENGLF_APIENTRYP RasterPos2iv)(const GLint *v);
    void (QOPENGLF_APIENTRYP RasterPos2i)(GLint x, GLint y);
    void (QOPENGLF_APIENTRYP RasterPos2fv)(const GLfloat *v);
    void (QOPENGLF_APIENTRYP RasterPos2f)(GLfloat x, GLfloat y);
    void (QOPENGLF_APIENTRYP RasterPos2dv)(const GLdouble *v);
    void (QOPENGLF_APIENTRYP RasterPos2d)(GLdouble x, GLdouble y);
    void (QOPENGLF_APIENTRYP Normal3sv)(const GLshort *v);
    void (QOPENGLF_APIENTRYP Normal3s)(GLshort nx, GLshort ny, GLshort nz);
    void (QOPENGLF_APIENTRYP Normal3iv)(const GLint *v);
    void (QOPENGLF_APIENTRYP Normal3i)(GLint nx, GLint ny, GLint nz);
    void (QOPENGLF_APIENTRYP Normal3fv)(const GLfloat *v);
    void (QOPENGLF_APIENTRYP Normal3f)(GLfloat nx, GLfloat ny, GLfloat nz);
    void (QOPENGLF_APIENTRYP Normal3dv)(const GLdouble *v);
    void (QOPENGLF_APIENTRYP Normal3d)(GLdouble nx, GLdouble ny, GLdouble nz);
    void (QOPENGLF_APIENTRYP Normal3bv)(const GLbyte *v);
    void (QOPENGLF_APIENTRYP Normal3b)(GLbyte nx, GLbyte ny, GLbyte nz);
    void (QOPENGLF_APIENTRYP Indexsv)(const GLshort *c);
    void (QOPENGLF_APIENTRYP Indexs)(GLshort c);
    void (QOPENGLF_APIENTRYP Indexiv)(const GLint *c);
    void (QOPENGLF_APIENTRYP Indexi)(GLint c);
    void (QOPENGLF_APIENTRYP Indexfv)(const GLfloat *c);
    void (QOPENGLF_APIENTRYP Indexf)(GLfloat c);
    void (QOPENGLF_APIENTRYP Indexdv)(const GLdouble *c);
    void (QOPENGLF_APIENTRYP Indexd)(GLdouble c);
    void (QOPENGLF_APIENTRYP End)();
    void (QOPENGLF_APIENTRYP EdgeFlagv)(const GLboolean *flag);
    void (QOPENGLF_APIENTRYP EdgeFlag)(GLboolean flag);
    void (QOPENGLF_APIENTRYP Color4usv)(const GLushort *v);
    void (QOPENGLF_APIENTRYP Color4us)(GLushort red, GLushort green, GLushort blue, GLushort alpha);
    void (QOPENGLF_APIENTRYP Color4uiv)(const GLuint *v);
    void (QOPENGLF_APIENTRYP Color4ui)(GLuint red, GLuint green, GLuint blue, GLuint alpha);
    void (QOPENGLF_APIENTRYP Color4ubv)(const GLubyte *v);
    void (QOPENGLF_APIENTRYP Color4ub)(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
    void (QOPENGLF_APIENTRYP Color4sv)(const GLshort *v);
    void (QOPENGLF_APIENTRYP Color4s)(GLshort red, GLshort green, GLshort blue, GLshort alpha);
    void (QOPENGLF_APIENTRYP Color4iv)(const GLint *v);
    void (QOPENGLF_APIENTRYP Color4i)(GLint red, GLint green, GLint blue, GLint alpha);
    void (QOPENGLF_APIENTRYP Color4fv)(const GLfloat *v);
    void (QOPENGLF_APIENTRYP Color4f)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
    void (QOPENGLF_APIENTRYP Color4dv)(const GLdouble *v);
    void (QOPENGLF_APIENTRYP Color4d)(GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha);
    void (QOPENGLF_APIENTRYP Color4bv)(const GLbyte *v);
    void (QOPENGLF_APIENTRYP Color4b)(GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha);
    void (QOPENGLF_APIENTRYP Color3usv)(const GLushort *v);
    void (QOPENGLF_APIENTRYP Color3us)(GLushort red, GLushort green, GLushort blue);
    void (QOPENGLF_APIENTRYP Color3uiv)(const GLuint *v);
    void (QOPENGLF_APIENTRYP Color3ui)(GLuint red, GLuint green, GLuint blue);
    void (QOPENGLF_APIENTRYP Color3ubv)(const GLubyte *v);
    void (QOPENGLF_APIENTRYP Color3ub)(GLubyte red, GLubyte green, GLubyte blue);
    void (QOPENGLF_APIENTRYP Color3sv)(const GLshort *v);
    void (QOPENGLF_APIENTRYP Color3s)(GLshort red, GLshort green, GLshort blue);
    void (QOPENGLF_APIENTRYP Color3iv)(const GLint *v);
    void (QOPENGLF_APIENTRYP Color3i)(GLint red, GLint green, GLint blue);
    void (QOPENGLF_APIENTRYP Color3fv)(const GLfloat *v);
    void (QOPENGLF_APIENTRYP Color3f)(GLfloat red, GLfloat green, GLfloat blue);
    void (QOPENGLF_APIENTRYP Color3dv)(const GLdouble *v);
    void (QOPENGLF_APIENTRYP Color3d)(GLdouble red, GLdouble green, GLdouble blue);
    void (QOPENGLF_APIENTRYP Color3bv)(const GLbyte *v);
    void (QOPENGLF_APIENTRYP Color3b)(GLbyte red, GLbyte green, GLbyte blue);
    void (QOPENGLF_APIENTRYP Bitmap)(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap);
    void (QOPENGLF_APIENTRYP Begin)(GLenum mode);
    void (QOPENGLF_APIENTRYP ListBase)(GLuint base);
    GLuint (QOPENGLF_APIENTRYP GenLists)(GLsizei range);
    void (QOPENGLF_APIENTRYP DeleteLists)(GLuint list, GLsizei range);
    void (QOPENGLF_APIENTRYP CallLists)(GLsizei n, GLenum type, const GLvoid *lists);
    void (QOPENGLF_APIENTRYP CallList)(GLuint list);
    void (QOPENGLF_APIENTRYP EndList)();
    void (QOPENGLF_APIENTRYP NewList)(GLuint list, GLenum mode);

};

class QOpenGLFunctions_1_1_DeprecatedBackend : public QOpenGLVersionFunctionsBackend
{
public:
    QOpenGLFunctions_1_1_DeprecatedBackend(QOpenGLContext *context);

    static QOpenGLVersionStatus versionStatus();

    // OpenGL 1.1 deprecated functions
    void (QOPENGLF_APIENTRYP PushClientAttrib)(GLbitfield mask);
    void (QOPENGLF_APIENTRYP PopClientAttrib)();
    void (QOPENGLF_APIENTRYP PrioritizeTextures)(GLsizei n, const GLuint *textures, const GLfloat *priorities);
    GLboolean (QOPENGLF_APIENTRYP AreTexturesResident)(GLsizei n, const GLuint *textures, GLboolean *residences);
    void (QOPENGLF_APIENTRYP VertexPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
    void (QOPENGLF_APIENTRYP TexCoordPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
    void (QOPENGLF_APIENTRYP NormalPointer)(GLenum type, GLsizei stride, const GLvoid *pointer);
    void (QOPENGLF_APIENTRYP InterleavedArrays)(GLenum format, GLsizei stride, const GLvoid *pointer);
    void (QOPENGLF_APIENTRYP IndexPointer)(GLenum type, GLsizei stride, const GLvoid *pointer);
    void (QOPENGLF_APIENTRYP EnableClientState)(GLenum array);
    void (QOPENGLF_APIENTRYP EdgeFlagPointer)(GLsizei stride, const GLvoid *pointer);
    void (QOPENGLF_APIENTRYP DisableClientState)(GLenum array);
    void (QOPENGLF_APIENTRYP ColorPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
    void (QOPENGLF_APIENTRYP ArrayElement)(GLint i);
    void (QOPENGLF_APIENTRYP Indexubv)(const GLubyte *c);
    void (QOPENGLF_APIENTRYP Indexub)(GLubyte c);
    void (QOPENGLF_APIENTRYP GetPointerv)(GLenum pname, GLvoid* *params);
};

class QOpenGLFunctions_1_2_DeprecatedBackend : public QOpenGLVersionFunctionsBackend
{
public:
    QOpenGLFunctions_1_2_DeprecatedBackend(QOpenGLContext *context);

    static QOpenGLVersionStatus versionStatus();

    // OpenGL 1.2 deprecated functions
    void (QOPENGLF_APIENTRYP ColorTableParameterfv)(GLenum target, GLenum pname, const GLfloat *params);
    void (QOPENGLF_APIENTRYP ColorTableParameteriv)(GLenum target, GLenum pname, const GLint *params);
    void (QOPENGLF_APIENTRYP CopyColorTable)(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width);
    void (QOPENGLF_APIENTRYP GetColorTable)(GLenum target, GLenum format, GLenum type, GLvoid *table);
    void (QOPENGLF_APIENTRYP GetColorTableParameterfv)(GLenum target, GLenum pname, GLfloat *params);
    void (QOPENGLF_APIENTRYP GetColorTableParameteriv)(GLenum target, GLenum pname, GLint *params);
    void (QOPENGLF_APIENTRYP ColorSubTable)(GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const GLvoid *data);
    void (QOPENGLF_APIENTRYP CopyColorSubTable)(GLenum target, GLsizei start, GLint x, GLint y, GLsizei width);
    void (QOPENGLF_APIENTRYP ConvolutionFilter1D)(GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *image);
    void (QOPENGLF_APIENTRYP ConvolutionFilter2D)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *image);
    void (QOPENGLF_APIENTRYP ConvolutionParameterf)(GLenum target, GLenum pname, GLfloat params);
    void (QOPENGLF_APIENTRYP ConvolutionParameterfv)(GLenum target, GLenum pname, const GLfloat *params);
    void (QOPENGLF_APIENTRYP ConvolutionParameteri)(GLenum target, GLenum pname, GLint params);
    void (QOPENGLF_APIENTRYP ConvolutionParameteriv)(GLenum target, GLenum pname, const GLint *params);
    void (QOPENGLF_APIENTRYP CopyConvolutionFilter1D)(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width);
    void (QOPENGLF_APIENTRYP CopyConvolutionFilter2D)(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height);
    void (QOPENGLF_APIENTRYP GetConvolutionFilter)(GLenum target, GLenum format, GLenum type, GLvoid *image);
    void (QOPENGLF_APIENTRYP GetConvolutionParameterfv)(GLenum target, GLenum pname, GLfloat *params);
    void (QOPENGLF_APIENTRYP GetConvolutionParameteriv)(GLenum target, GLenum pname, GLint *params);
    void (QOPENGLF_APIENTRYP GetSeparableFilter)(GLenum target, GLenum format, GLenum type, GLvoid *row, GLvoid *column, GLvoid *span);
    void (QOPENGLF_APIENTRYP SeparableFilter2D)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *row, const GLvoid *column);
    void (QOPENGLF_APIENTRYP GetHistogram)(GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid *values);
    void (QOPENGLF_APIENTRYP GetHistogramParameterfv)(GLenum target, GLenum pname, GLfloat *params);
    void (QOPENGLF_APIENTRYP GetHistogramParameteriv)(GLenum target, GLenum pname, GLint *params);
    void (QOPENGLF_APIENTRYP GetMinmax)(GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid *values);
    void (QOPENGLF_APIENTRYP GetMinmaxParameterfv)(GLenum target, GLenum pname, GLfloat *params);
    void (QOPENGLF_APIENTRYP GetMinmaxParameteriv)(GLenum target, GLenum pname, GLint *params);
    void (QOPENGLF_APIENTRYP Histogram)(GLenum target, GLsizei width, GLenum internalformat, GLboolean sink);
    void (QOPENGLF_APIENTRYP Minmax)(GLenum target, GLenum internalformat, GLboolean sink);
    void (QOPENGLF_APIENTRYP ResetHistogram)(GLenum target);
    void (QOPENGLF_APIENTRYP ResetMinmax)(GLenum target);
    void (QOPENGLF_APIENTRYP ColorTable)(GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *table);

};

class QOpenGLFunctions_1_3_DeprecatedBackend : public QOpenGLVersionFunctionsBackend
{
public:
    QOpenGLFunctions_1_3_DeprecatedBackend(QOpenGLContext *context);

    static QOpenGLVersionStatus versionStatus();

    // OpenGL 1.3 deprecated functions
    void (QOPENGLF_APIENTRYP MultTransposeMatrixd)(const GLdouble *m);
    void (QOPENGLF_APIENTRYP MultTransposeMatrixf)(const GLfloat *m);
    void (QOPENGLF_APIENTRYP LoadTransposeMatrixd)(const GLdouble *m);
    void (QOPENGLF_APIENTRYP LoadTransposeMatrixf)(const GLfloat *m);
    void (QOPENGLF_APIENTRYP MultiTexCoord4sv)(GLenum target, const GLshort *v);
    void (QOPENGLF_APIENTRYP MultiTexCoord4s)(GLenum target, GLshort s, GLshort t, GLshort r, GLshort q);
    void (QOPENGLF_APIENTRYP MultiTexCoord4iv)(GLenum target, const GLint *v);
    void (QOPENGLF_APIENTRYP MultiTexCoord4i)(GLenum target, GLint s, GLint t, GLint r, GLint q);
    void (QOPENGLF_APIENTRYP MultiTexCoord4fv)(GLenum target, const GLfloat *v);
    void (QOPENGLF_APIENTRYP MultiTexCoord4f)(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
    void (QOPENGLF_APIENTRYP MultiTexCoord4dv)(GLenum target, const GLdouble *v);
    void (QOPENGLF_APIENTRYP MultiTexCoord4d)(GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
    void (QOPENGLF_APIENTRYP MultiTexCoord3sv)(GLenum target, const GLshort *v);
    void (QOPENGLF_APIENTRYP MultiTexCoord3s)(GLenum target, GLshort s, GLshort t, GLshort r);
    void (QOPENGLF_APIENTRYP MultiTexCoord3iv)(GLenum target, const GLint *v);
    void (QOPENGLF_APIENTRYP MultiTexCoord3i)(GLenum target, GLint s, GLint t, GLint r);
    void (QOPENGLF_APIENTRYP MultiTexCoord3fv)(GLenum target, const GLfloat *v);
    void (QOPENGLF_APIENTRYP MultiTexCoord3f)(GLenum target, GLfloat s, GLfloat t, GLfloat r);
    void (QOPENGLF_APIENTRYP MultiTexCoord3dv)(GLenum target, const GLdouble *v);
    void (QOPENGLF_APIENTRYP MultiTexCoord3d)(GLenum target, GLdouble s, GLdouble t, GLdouble r);
    void (QOPENGLF_APIENTRYP MultiTexCoord2sv)(GLenum target, const GLshort *v);
    void (QOPENGLF_APIENTRYP MultiTexCoord2s)(GLenum target, GLshort s, GLshort t);
    void (QOPENGLF_APIENTRYP MultiTexCoord2iv)(GLenum target, const GLint *v);
    void (QOPENGLF_APIENTRYP MultiTexCoord2i)(GLenum target, GLint s, GLint t);
    void (QOPENGLF_APIENTRYP MultiTexCoord2fv)(GLenum target, const GLfloat *v);
    void (QOPENGLF_APIENTRYP MultiTexCoord2f)(GLenum target, GLfloat s, GLfloat t);
    void (QOPENGLF_APIENTRYP MultiTexCoord2dv)(GLenum target, const GLdouble *v);
    void (QOPENGLF_APIENTRYP MultiTexCoord2d)(GLenum target, GLdouble s, GLdouble t);
    void (QOPENGLF_APIENTRYP MultiTexCoord1sv)(GLenum target, const GLshort *v);
    void (QOPENGLF_APIENTRYP MultiTexCoord1s)(GLenum target, GLshort s);
    void (QOPENGLF_APIENTRYP MultiTexCoord1iv)(GLenum target, const GLint *v);
    void (QOPENGLF_APIENTRYP MultiTexCoord1i)(GLenum target, GLint s);
    void (QOPENGLF_APIENTRYP MultiTexCoord1fv)(GLenum target, const GLfloat *v);
    void (QOPENGLF_APIENTRYP MultiTexCoord1f)(GLenum target, GLfloat s);
    void (QOPENGLF_APIENTRYP MultiTexCoord1dv)(GLenum target, const GLdouble *v);
    void (QOPENGLF_APIENTRYP MultiTexCoord1d)(GLenum target, GLdouble s);
    void (QOPENGLF_APIENTRYP ClientActiveTexture)(GLenum texture);

};

class QOpenGLFunctions_1_4_DeprecatedBackend : public QOpenGLVersionFunctionsBackend
{
public:
    QOpenGLFunctions_1_4_DeprecatedBackend(QOpenGLContext *context);

    static QOpenGLVersionStatus versionStatus();

    // OpenGL 1.4 deprecated functions
    void (QOPENGLF_APIENTRYP WindowPos3sv)(const GLshort *v);
    void (QOPENGLF_APIENTRYP WindowPos3s)(GLshort x, GLshort y, GLshort z);
    void (QOPENGLF_APIENTRYP WindowPos3iv)(const GLint *v);
    void (QOPENGLF_APIENTRYP WindowPos3i)(GLint x, GLint y, GLint z);
    void (QOPENGLF_APIENTRYP WindowPos3fv)(const GLfloat *v);
    void (QOPENGLF_APIENTRYP WindowPos3f)(GLfloat x, GLfloat y, GLfloat z);
    void (QOPENGLF_APIENTRYP WindowPos3dv)(const GLdouble *v);
    void (QOPENGLF_APIENTRYP WindowPos3d)(GLdouble x, GLdouble y, GLdouble z);
    void (QOPENGLF_APIENTRYP WindowPos2sv)(const GLshort *v);
    void (QOPENGLF_APIENTRYP WindowPos2s)(GLshort x, GLshort y);
    void (QOPENGLF_APIENTRYP WindowPos2iv)(const GLint *v);
    void (QOPENGLF_APIENTRYP WindowPos2i)(GLint x, GLint y);
    void (QOPENGLF_APIENTRYP WindowPos2fv)(const GLfloat *v);
    void (QOPENGLF_APIENTRYP WindowPos2f)(GLfloat x, GLfloat y);
    void (QOPENGLF_APIENTRYP WindowPos2dv)(const GLdouble *v);
    void (QOPENGLF_APIENTRYP WindowPos2d)(GLdouble x, GLdouble y);
    void (QOPENGLF_APIENTRYP SecondaryColorPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
    void (QOPENGLF_APIENTRYP SecondaryColor3usv)(const GLushort *v);
    void (QOPENGLF_APIENTRYP SecondaryColor3us)(GLushort red, GLushort green, GLushort blue);
    void (QOPENGLF_APIENTRYP SecondaryColor3uiv)(const GLuint *v);
    void (QOPENGLF_APIENTRYP SecondaryColor3ui)(GLuint red, GLuint green, GLuint blue);
    void (QOPENGLF_APIENTRYP SecondaryColor3ubv)(const GLubyte *v);
    void (QOPENGLF_APIENTRYP SecondaryColor3ub)(GLubyte red, GLubyte green, GLubyte blue);
    void (QOPENGLF_APIENTRYP SecondaryColor3sv)(const GLshort *v);
    void (QOPENGLF_APIENTRYP SecondaryColor3s)(GLshort red, GLshort green, GLshort blue);
    void (QOPENGLF_APIENTRYP SecondaryColor3iv)(const GLint *v);
    void (QOPENGLF_APIENTRYP SecondaryColor3i)(GLint red, GLint green, GLint blue);
    void (QOPENGLF_APIENTRYP SecondaryColor3fv)(const GLfloat *v);
    void (QOPENGLF_APIENTRYP SecondaryColor3f)(GLfloat red, GLfloat green, GLfloat blue);
    void (QOPENGLF_APIENTRYP SecondaryColor3dv)(const GLdouble *v);
    void (QOPENGLF_APIENTRYP SecondaryColor3d)(GLdouble red, GLdouble green, GLdouble blue);
    void (QOPENGLF_APIENTRYP SecondaryColor3bv)(const GLbyte *v);
    void (QOPENGLF_APIENTRYP SecondaryColor3b)(GLbyte red, GLbyte green, GLbyte blue);
    void (QOPENGLF_APIENTRYP FogCoordPointer)(GLenum type, GLsizei stride, const GLvoid *pointer);
    void (QOPENGLF_APIENTRYP FogCoorddv)(const GLdouble *coord);
    void (QOPENGLF_APIENTRYP FogCoordd)(GLdouble coord);
    void (QOPENGLF_APIENTRYP FogCoordfv)(const GLfloat *coord);
    void (QOPENGLF_APIENTRYP FogCoordf)(GLfloat coord);

};

class QOpenGLFunctions_2_0_DeprecatedBackend : public QOpenGLVersionFunctionsBackend
{
public:
    QOpenGLFunctions_2_0_DeprecatedBackend(QOpenGLContext *context);

    static QOpenGLVersionStatus versionStatus();

    // OpenGL 2.0 deprecated functions
    void (QOPENGLF_APIENTRYP VertexAttrib4usv)(GLuint index, const GLushort *v);
    void (QOPENGLF_APIENTRYP VertexAttrib4uiv)(GLuint index, const GLuint *v);
    void (QOPENGLF_APIENTRYP VertexAttrib4ubv)(GLuint index, const GLubyte *v);
    void (QOPENGLF_APIENTRYP VertexAttrib4sv)(GLuint index, const GLshort *v);
    void (QOPENGLF_APIENTRYP VertexAttrib4s)(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w);
    void (QOPENGLF_APIENTRYP VertexAttrib4iv)(GLuint index, const GLint *v);
    void (QOPENGLF_APIENTRYP VertexAttrib4fv)(GLuint index, const GLfloat *v);
    void (QOPENGLF_APIENTRYP VertexAttrib4f)(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
    void (QOPENGLF_APIENTRYP VertexAttrib4dv)(GLuint index, const GLdouble *v);
    void (QOPENGLF_APIENTRYP VertexAttrib4d)(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
    void (QOPENGLF_APIENTRYP VertexAttrib4bv)(GLuint index, const GLbyte *v);
    void (QOPENGLF_APIENTRYP VertexAttrib4Nusv)(GLuint index, const GLushort *v);
    void (QOPENGLF_APIENTRYP VertexAttrib4Nuiv)(GLuint index, const GLuint *v);
    void (QOPENGLF_APIENTRYP VertexAttrib4Nubv)(GLuint index, const GLubyte *v);
    void (QOPENGLF_APIENTRYP VertexAttrib4Nub)(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w);
    void (QOPENGLF_APIENTRYP VertexAttrib4Nsv)(GLuint index, const GLshort *v);
    void (QOPENGLF_APIENTRYP VertexAttrib4Niv)(GLuint index, const GLint *v);
    void (QOPENGLF_APIENTRYP VertexAttrib4Nbv)(GLuint index, const GLbyte *v);
    void (QOPENGLF_APIENTRYP VertexAttrib3sv)(GLuint index, const GLshort *v);
    void (QOPENGLF_APIENTRYP VertexAttrib3s)(GLuint index, GLshort x, GLshort y, GLshort z);
    void (QOPENGLF_APIENTRYP VertexAttrib3fv)(GLuint index, const GLfloat *v);
    void (QOPENGLF_APIENTRYP VertexAttrib3f)(GLuint index, GLfloat x, GLfloat y, GLfloat z);
    void (QOPENGLF_APIENTRYP VertexAttrib3dv)(GLuint index, const GLdouble *v);
    void (QOPENGLF_APIENTRYP VertexAttrib3d)(GLuint index, GLdouble x, GLdouble y, GLdouble z);
    void (QOPENGLF_APIENTRYP VertexAttrib2sv)(GLuint index, const GLshort *v);
    void (QOPENGLF_APIENTRYP VertexAttrib2s)(GLuint index, GLshort x, GLshort y);
    void (QOPENGLF_APIENTRYP VertexAttrib2fv)(GLuint index, const GLfloat *v);
    void (QOPENGLF_APIENTRYP VertexAttrib2f)(GLuint index, GLfloat x, GLfloat y);
    void (QOPENGLF_APIENTRYP VertexAttrib2dv)(GLuint index, const GLdouble *v);
    void (QOPENGLF_APIENTRYP VertexAttrib2d)(GLuint index, GLdouble x, GLdouble y);
    void (QOPENGLF_APIENTRYP VertexAttrib1sv)(GLuint index, const GLshort *v);
    void (QOPENGLF_APIENTRYP VertexAttrib1s)(GLuint index, GLshort x);
    void (QOPENGLF_APIENTRYP VertexAttrib1fv)(GLuint index, const GLfloat *v);
    void (QOPENGLF_APIENTRYP VertexAttrib1f)(GLuint index, GLfloat x);
    void (QOPENGLF_APIENTRYP VertexAttrib1dv)(GLuint index, const GLdouble *v);
    void (QOPENGLF_APIENTRYP VertexAttrib1d)(GLuint index, GLdouble x);

};

class QOpenGLFunctions_3_0_DeprecatedBackend : public QOpenGLVersionFunctionsBackend
{
public:
    QOpenGLFunctions_3_0_DeprecatedBackend(QOpenGLContext *context);

    static QOpenGLVersionStatus versionStatus();

    // OpenGL 3.0 deprecated functions
    void (QOPENGLF_APIENTRYP VertexAttribI4usv)(GLuint index, const GLushort *v);
    void (QOPENGLF_APIENTRYP VertexAttribI4ubv)(GLuint index, const GLubyte *v);
    void (QOPENGLF_APIENTRYP VertexAttribI4sv)(GLuint index, const GLshort *v);
    void (QOPENGLF_APIENTRYP VertexAttribI4bv)(GLuint index, const GLbyte *v);
    void (QOPENGLF_APIENTRYP VertexAttribI4uiv)(GLuint index, const GLuint *v);
    void (QOPENGLF_APIENTRYP VertexAttribI3uiv)(GLuint index, const GLuint *v);
    void (QOPENGLF_APIENTRYP VertexAttribI2uiv)(GLuint index, const GLuint *v);
    void (QOPENGLF_APIENTRYP VertexAttribI1uiv)(GLuint index, const GLuint *v);
    void (QOPENGLF_APIENTRYP VertexAttribI4iv)(GLuint index, const GLint *v);
    void (QOPENGLF_APIENTRYP VertexAttribI3iv)(GLuint index, const GLint *v);
    void (QOPENGLF_APIENTRYP VertexAttribI2iv)(GLuint index, const GLint *v);
    void (QOPENGLF_APIENTRYP VertexAttribI1iv)(GLuint index, const GLint *v);
    void (QOPENGLF_APIENTRYP VertexAttribI4ui)(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w);
    void (QOPENGLF_APIENTRYP VertexAttribI3ui)(GLuint index, GLuint x, GLuint y, GLuint z);
    void (QOPENGLF_APIENTRYP VertexAttribI2ui)(GLuint index, GLuint x, GLuint y);
    void (QOPENGLF_APIENTRYP VertexAttribI1ui)(GLuint index, GLuint x);
    void (QOPENGLF_APIENTRYP VertexAttribI4i)(GLuint index, GLint x, GLint y, GLint z, GLint w);
    void (QOPENGLF_APIENTRYP VertexAttribI3i)(GLuint index, GLint x, GLint y, GLint z);
    void (QOPENGLF_APIENTRYP VertexAttribI2i)(GLuint index, GLint x, GLint y);
    void (QOPENGLF_APIENTRYP VertexAttribI1i)(GLuint index, GLint x);

};

class QOpenGLFunctions_3_3_DeprecatedBackend : public QOpenGLVersionFunctionsBackend
{
public:
    QOpenGLFunctions_3_3_DeprecatedBackend(QOpenGLContext *context);

    static QOpenGLVersionStatus versionStatus();

    // OpenGL 3.3 deprecated functions
    void (QOPENGLF_APIENTRYP SecondaryColorP3uiv)(GLenum type, const GLuint *color);
    void (QOPENGLF_APIENTRYP SecondaryColorP3ui)(GLenum type, GLuint color);
    void (QOPENGLF_APIENTRYP ColorP4uiv)(GLenum type, const GLuint *color);
    void (QOPENGLF_APIENTRYP ColorP4ui)(GLenum type, GLuint color);
    void (QOPENGLF_APIENTRYP ColorP3uiv)(GLenum type, const GLuint *color);
    void (QOPENGLF_APIENTRYP ColorP3ui)(GLenum type, GLuint color);
    void (QOPENGLF_APIENTRYP NormalP3uiv)(GLenum type, const GLuint *coords);
    void (QOPENGLF_APIENTRYP NormalP3ui)(GLenum type, GLuint coords);
    void (QOPENGLF_APIENTRYP MultiTexCoordP4uiv)(GLenum texture, GLenum type, const GLuint *coords);
    void (QOPENGLF_APIENTRYP MultiTexCoordP4ui)(GLenum texture, GLenum type, GLuint coords);
    void (QOPENGLF_APIENTRYP MultiTexCoordP3uiv)(GLenum texture, GLenum type, const GLuint *coords);
    void (QOPENGLF_APIENTRYP MultiTexCoordP3ui)(GLenum texture, GLenum type, GLuint coords);
    void (QOPENGLF_APIENTRYP MultiTexCoordP2uiv)(GLenum texture, GLenum type, const GLuint *coords);
    void (QOPENGLF_APIENTRYP MultiTexCoordP2ui)(GLenum texture, GLenum type, GLuint coords);
    void (QOPENGLF_APIENTRYP MultiTexCoordP1uiv)(GLenum texture, GLenum type, const GLuint *coords);
    void (QOPENGLF_APIENTRYP MultiTexCoordP1ui)(GLenum texture, GLenum type, GLuint coords);
    void (QOPENGLF_APIENTRYP TexCoordP4uiv)(GLenum type, const GLuint *coords);
    void (QOPENGLF_APIENTRYP TexCoordP4ui)(GLenum type, GLuint coords);
    void (QOPENGLF_APIENTRYP TexCoordP3uiv)(GLenum type, const GLuint *coords);
    void (QOPENGLF_APIENTRYP TexCoordP3ui)(GLenum type, GLuint coords);
    void (QOPENGLF_APIENTRYP TexCoordP2uiv)(GLenum type, const GLuint *coords);
    void (QOPENGLF_APIENTRYP TexCoordP2ui)(GLenum type, GLuint coords);
    void (QOPENGLF_APIENTRYP TexCoordP1uiv)(GLenum type, const GLuint *coords);
    void (QOPENGLF_APIENTRYP TexCoordP1ui)(GLenum type, GLuint coords);
    void (QOPENGLF_APIENTRYP VertexP4uiv)(GLenum type, const GLuint *value);
    void (QOPENGLF_APIENTRYP VertexP4ui)(GLenum type, GLuint value);
    void (QOPENGLF_APIENTRYP VertexP3uiv)(GLenum type, const GLuint *value);
    void (QOPENGLF_APIENTRYP VertexP3ui)(GLenum type, GLuint value);
    void (QOPENGLF_APIENTRYP VertexP2uiv)(GLenum type, const GLuint *value);
    void (QOPENGLF_APIENTRYP VertexP2ui)(GLenum type, GLuint value);

};

class QOpenGLFunctions_4_5_DeprecatedBackend : public QOpenGLVersionFunctionsBackend
{
public:
    QOpenGLFunctions_4_5_DeprecatedBackend(QOpenGLContext *context);

    static QOpenGLVersionStatus versionStatus();

    // OpenGL 4.5 deprecated functions
    void (QOPENGLF_APIENTRYP GetnMinmax)(GLenum target, GLboolean reset, GLenum format, GLenum type, GLsizei bufSize, GLvoid *values);
    void (QOPENGLF_APIENTRYP GetnHistogram)(GLenum target, GLboolean reset, GLenum format, GLenum type, GLsizei bufSize, GLvoid *values);
    void (QOPENGLF_APIENTRYP GetnSeparableFilter)(GLenum target, GLenum format, GLenum type, GLsizei rowBufSize, GLvoid *row, GLsizei columnBufSize, GLvoid *column, GLvoid *span);
    void (QOPENGLF_APIENTRYP GetnConvolutionFilter)(GLenum target, GLenum format, GLenum type, GLsizei bufSize, GLvoid *image);
    void (QOPENGLF_APIENTRYP GetnColorTable)(GLenum target, GLenum format, GLenum type, GLsizei bufSize, GLvoid *table);
    void (QOPENGLF_APIENTRYP GetnPolygonStipple)(GLsizei bufSize, GLubyte *pattern);
    void (QOPENGLF_APIENTRYP GetnPixelMapusv)(GLenum map, GLsizei bufSize, GLushort *values);
    void (QOPENGLF_APIENTRYP GetnPixelMapuiv)(GLenum map, GLsizei bufSize, GLuint *values);
    void (QOPENGLF_APIENTRYP GetnPixelMapfv)(GLenum map, GLsizei bufSize, GLfloat *values);
    void (QOPENGLF_APIENTRYP GetnMapiv)(GLenum target, GLenum query, GLsizei bufSize, GLint *v);
    void (QOPENGLF_APIENTRYP GetnMapfv)(GLenum target, GLenum query, GLsizei bufSize, GLfloat *v);
    void (QOPENGLF_APIENTRYP GetnMapdv)(GLenum target, GLenum query, GLsizei bufSize, GLdouble *v);

};

#else

// No need for backend classes with function pointers with ES2.
// All function addresses are independent of context and display.

#endif // !QT_OPENGL_ES_2

#endif // QT_NO_OPENGL

#endif
