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

#ifndef QOPENGLVERSIONFUNCTIONS_4_4_COMPATIBILITY_H
#define QOPENGLVERSIONFUNCTIONS_4_4_COMPATIBILITY_H

#include <QtCore/qglobal.h>

#if !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2)

#include <QtGui/QOpenGLVersionFunctions>
#include <QtGui/qopenglcontext.h>

QT_BEGIN_NAMESPACE

class Q_GUI_EXPORT QOpenGLFunctions_4_4_Compatibility : public QAbstractOpenGLFunctions
{
public:
    QOpenGLFunctions_4_4_Compatibility();
    ~QOpenGLFunctions_4_4_Compatibility();

    bool initializeOpenGLFunctions() override;

    // OpenGL 1.0 core functions
    void glViewport(GLint x, GLint y, GLsizei width, GLsizei height);
    void glDepthRange(GLdouble nearVal, GLdouble farVal);
    GLboolean glIsEnabled(GLenum cap);
    void glGetTexLevelParameteriv(GLenum target, GLint level, GLenum pname, GLint *params);
    void glGetTexLevelParameterfv(GLenum target, GLint level, GLenum pname, GLfloat *params);
    void glGetTexParameteriv(GLenum target, GLenum pname, GLint *params);
    void glGetTexParameterfv(GLenum target, GLenum pname, GLfloat *params);
    void glGetTexImage(GLenum target, GLint level, GLenum format, GLenum type, void *pixels);
    const GLubyte * glGetString(GLenum name);
    void glGetIntegerv(GLenum pname, GLint *data);
    void glGetFloatv(GLenum pname, GLfloat *data);
    GLenum glGetError();
    void glGetDoublev(GLenum pname, GLdouble *data);
    void glGetBooleanv(GLenum pname, GLboolean *data);
    void glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels);
    void glReadBuffer(GLenum src);
    void glPixelStorei(GLenum pname, GLint param);
    void glPixelStoref(GLenum pname, GLfloat param);
    void glDepthFunc(GLenum func);
    void glStencilOp(GLenum fail, GLenum zfail, GLenum zpass);
    void glStencilFunc(GLenum func, GLint ref, GLuint mask);
    void glLogicOp(GLenum opcode);
    void glBlendFunc(GLenum sfactor, GLenum dfactor);
    void glFlush();
    void glFinish();
    void glEnable(GLenum cap);
    void glDisable(GLenum cap);
    void glDepthMask(GLboolean flag);
    void glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
    void glStencilMask(GLuint mask);
    void glClearDepth(GLdouble depth);
    void glClearStencil(GLint s);
    void glClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
    void glClear(GLbitfield mask);
    void glDrawBuffer(GLenum buf);
    void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);
    void glTexImage1D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const void *pixels);
    void glTexParameteriv(GLenum target, GLenum pname, const GLint *params);
    void glTexParameteri(GLenum target, GLenum pname, GLint param);
    void glTexParameterfv(GLenum target, GLenum pname, const GLfloat *params);
    void glTexParameterf(GLenum target, GLenum pname, GLfloat param);
    void glScissor(GLint x, GLint y, GLsizei width, GLsizei height);
    void glPolygonMode(GLenum face, GLenum mode);
    void glPointSize(GLfloat size);
    void glLineWidth(GLfloat width);
    void glHint(GLenum target, GLenum mode);
    void glFrontFace(GLenum mode);
    void glCullFace(GLenum mode);

    // OpenGL 1.1 core functions
    GLboolean glIsTexture(GLuint texture);
    void glGenTextures(GLsizei n, GLuint *textures);
    void glDeleteTextures(GLsizei n, const GLuint *textures);
    void glBindTexture(GLenum target, GLuint texture);
    void glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
    void glTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void *pixels);
    void glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
    void glCopyTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
    void glCopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
    void glCopyTexImage1D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);
    void glPolygonOffset(GLfloat factor, GLfloat units);
    void glDrawElements(GLenum mode, GLsizei count, GLenum type, const void *indices);
    void glDrawArrays(GLenum mode, GLint first, GLsizei count);

    // OpenGL 1.2 core functions
    void glBlendColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
    void glBlendEquation(GLenum mode);
    void glCopyTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
    void glTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels);
    void glTexImage3D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void *pixels);
    void glDrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices);

    // OpenGL 1.3 core functions
    void glGetCompressedTexImage(GLenum target, GLint level, void *img);
    void glCompressedTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void *data);
    void glCompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data);
    void glCompressedTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data);
    void glCompressedTexImage1D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const void *data);
    void glCompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data);
    void glCompressedTexImage3D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void *data);
    void glSampleCoverage(GLfloat value, GLboolean invert);
    void glActiveTexture(GLenum texture);

    // OpenGL 1.4 core functions
    void glPointParameteriv(GLenum pname, const GLint *params);
    void glPointParameteri(GLenum pname, GLint param);
    void glPointParameterfv(GLenum pname, const GLfloat *params);
    void glPointParameterf(GLenum pname, GLfloat param);
    void glMultiDrawElements(GLenum mode, const GLsizei *count, GLenum type, const void *const *indices, GLsizei drawcount);
    void glMultiDrawArrays(GLenum mode, const GLint *first, const GLsizei *count, GLsizei drawcount);
    void glBlendFuncSeparate(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);

    // OpenGL 1.5 core functions
    void glGetBufferPointerv(GLenum target, GLenum pname, void * *params);
    void glGetBufferParameteriv(GLenum target, GLenum pname, GLint *params);
    GLboolean glUnmapBuffer(GLenum target);
    void * glMapBuffer(GLenum target, GLenum access);
    void glGetBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, void *data);
    void glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void *data);
    void glBufferData(GLenum target, GLsizeiptr size, const void *data, GLenum usage);
    GLboolean glIsBuffer(GLuint buffer);
    void glGenBuffers(GLsizei n, GLuint *buffers);
    void glDeleteBuffers(GLsizei n, const GLuint *buffers);
    void glBindBuffer(GLenum target, GLuint buffer);
    void glGetQueryObjectuiv(GLuint id, GLenum pname, GLuint *params);
    void glGetQueryObjectiv(GLuint id, GLenum pname, GLint *params);
    void glGetQueryiv(GLenum target, GLenum pname, GLint *params);
    void glEndQuery(GLenum target);
    void glBeginQuery(GLenum target, GLuint id);
    GLboolean glIsQuery(GLuint id);
    void glDeleteQueries(GLsizei n, const GLuint *ids);
    void glGenQueries(GLsizei n, GLuint *ids);

    // OpenGL 2.0 core functions
    void glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
    void glVertexAttrib4usv(GLuint index, const GLushort *v);
    void glVertexAttrib4uiv(GLuint index, const GLuint *v);
    void glVertexAttrib4ubv(GLuint index, const GLubyte *v);
    void glVertexAttrib4sv(GLuint index, const GLshort *v);
    void glVertexAttrib4s(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w);
    void glVertexAttrib4iv(GLuint index, const GLint *v);
    void glVertexAttrib4fv(GLuint index, const GLfloat *v);
    void glVertexAttrib4f(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
    void glVertexAttrib4dv(GLuint index, const GLdouble *v);
    void glVertexAttrib4d(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
    void glVertexAttrib4bv(GLuint index, const GLbyte *v);
    void glVertexAttrib4Nusv(GLuint index, const GLushort *v);
    void glVertexAttrib4Nuiv(GLuint index, const GLuint *v);
    void glVertexAttrib4Nubv(GLuint index, const GLubyte *v);
    void glVertexAttrib4Nub(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w);
    void glVertexAttrib4Nsv(GLuint index, const GLshort *v);
    void glVertexAttrib4Niv(GLuint index, const GLint *v);
    void glVertexAttrib4Nbv(GLuint index, const GLbyte *v);
    void glVertexAttrib3sv(GLuint index, const GLshort *v);
    void glVertexAttrib3s(GLuint index, GLshort x, GLshort y, GLshort z);
    void glVertexAttrib3fv(GLuint index, const GLfloat *v);
    void glVertexAttrib3f(GLuint index, GLfloat x, GLfloat y, GLfloat z);
    void glVertexAttrib3dv(GLuint index, const GLdouble *v);
    void glVertexAttrib3d(GLuint index, GLdouble x, GLdouble y, GLdouble z);
    void glVertexAttrib2sv(GLuint index, const GLshort *v);
    void glVertexAttrib2s(GLuint index, GLshort x, GLshort y);
    void glVertexAttrib2fv(GLuint index, const GLfloat *v);
    void glVertexAttrib2f(GLuint index, GLfloat x, GLfloat y);
    void glVertexAttrib2dv(GLuint index, const GLdouble *v);
    void glVertexAttrib2d(GLuint index, GLdouble x, GLdouble y);
    void glVertexAttrib1sv(GLuint index, const GLshort *v);
    void glVertexAttrib1s(GLuint index, GLshort x);
    void glVertexAttrib1fv(GLuint index, const GLfloat *v);
    void glVertexAttrib1f(GLuint index, GLfloat x);
    void glVertexAttrib1dv(GLuint index, const GLdouble *v);
    void glVertexAttrib1d(GLuint index, GLdouble x);
    void glValidateProgram(GLuint program);
    void glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    void glUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    void glUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    void glUniform4iv(GLint location, GLsizei count, const GLint *value);
    void glUniform3iv(GLint location, GLsizei count, const GLint *value);
    void glUniform2iv(GLint location, GLsizei count, const GLint *value);
    void glUniform1iv(GLint location, GLsizei count, const GLint *value);
    void glUniform4fv(GLint location, GLsizei count, const GLfloat *value);
    void glUniform3fv(GLint location, GLsizei count, const GLfloat *value);
    void glUniform2fv(GLint location, GLsizei count, const GLfloat *value);
    void glUniform1fv(GLint location, GLsizei count, const GLfloat *value);
    void glUniform4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
    void glUniform3i(GLint location, GLint v0, GLint v1, GLint v2);
    void glUniform2i(GLint location, GLint v0, GLint v1);
    void glUniform1i(GLint location, GLint v0);
    void glUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
    void glUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
    void glUniform2f(GLint location, GLfloat v0, GLfloat v1);
    void glUniform1f(GLint location, GLfloat v0);
    void glUseProgram(GLuint program);
    void glShaderSource(GLuint shader, GLsizei count, const GLchar* const *string, const GLint *length);
    void glLinkProgram(GLuint program);
    GLboolean glIsShader(GLuint shader);
    GLboolean glIsProgram(GLuint program);
    void glGetVertexAttribPointerv(GLuint index, GLenum pname, void * *pointer);
    void glGetVertexAttribiv(GLuint index, GLenum pname, GLint *params);
    void glGetVertexAttribfv(GLuint index, GLenum pname, GLfloat *params);
    void glGetVertexAttribdv(GLuint index, GLenum pname, GLdouble *params);
    void glGetUniformiv(GLuint program, GLint location, GLint *params);
    void glGetUniformfv(GLuint program, GLint location, GLfloat *params);
    GLint glGetUniformLocation(GLuint program, const GLchar *name);
    void glGetShaderSource(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source);
    void glGetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
    void glGetShaderiv(GLuint shader, GLenum pname, GLint *params);
    void glGetProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
    void glGetProgramiv(GLuint program, GLenum pname, GLint *params);
    GLint glGetAttribLocation(GLuint program, const GLchar *name);
    void glGetAttachedShaders(GLuint program, GLsizei maxCount, GLsizei *count, GLuint *shaders);
    void glGetActiveUniform(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
    void glGetActiveAttrib(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
    void glEnableVertexAttribArray(GLuint index);
    void glDisableVertexAttribArray(GLuint index);
    void glDetachShader(GLuint program, GLuint shader);
    void glDeleteShader(GLuint shader);
    void glDeleteProgram(GLuint program);
    GLuint glCreateShader(GLenum type);
    GLuint glCreateProgram();
    void glCompileShader(GLuint shader);
    void glBindAttribLocation(GLuint program, GLuint index, const GLchar *name);
    void glAttachShader(GLuint program, GLuint shader);
    void glStencilMaskSeparate(GLenum face, GLuint mask);
    void glStencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask);
    void glStencilOpSeparate(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass);
    void glDrawBuffers(GLsizei n, const GLenum *bufs);
    void glBlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha);

    // OpenGL 2.1 core functions
    void glUniformMatrix4x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    void glUniformMatrix3x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    void glUniformMatrix4x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    void glUniformMatrix2x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    void glUniformMatrix3x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    void glUniformMatrix2x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);

    // OpenGL 3.0 core functions
    GLboolean glIsVertexArray(GLuint array);
    void glGenVertexArrays(GLsizei n, GLuint *arrays);
    void glDeleteVertexArrays(GLsizei n, const GLuint *arrays);
    void glBindVertexArray(GLuint array);
    void glFlushMappedBufferRange(GLenum target, GLintptr offset, GLsizeiptr length);
    void * glMapBufferRange(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
    void glFramebufferTextureLayer(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer);
    void glRenderbufferStorageMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
    void glBlitFramebuffer(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
    void glGenerateMipmap(GLenum target);
    void glGetFramebufferAttachmentParameteriv(GLenum target, GLenum attachment, GLenum pname, GLint *params);
    void glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
    void glFramebufferTexture3D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset);
    void glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
    void glFramebufferTexture1D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
    GLenum glCheckFramebufferStatus(GLenum target);
    void glGenFramebuffers(GLsizei n, GLuint *framebuffers);
    void glDeleteFramebuffers(GLsizei n, const GLuint *framebuffers);
    void glBindFramebuffer(GLenum target, GLuint framebuffer);
    GLboolean glIsFramebuffer(GLuint framebuffer);
    void glGetRenderbufferParameteriv(GLenum target, GLenum pname, GLint *params);
    void glRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
    void glGenRenderbuffers(GLsizei n, GLuint *renderbuffers);
    void glDeleteRenderbuffers(GLsizei n, const GLuint *renderbuffers);
    void glBindRenderbuffer(GLenum target, GLuint renderbuffer);
    GLboolean glIsRenderbuffer(GLuint renderbuffer);
    const GLubyte * glGetStringi(GLenum name, GLuint index);
    void glClearBufferfi(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil);
    void glClearBufferfv(GLenum buffer, GLint drawbuffer, const GLfloat *value);
    void glClearBufferuiv(GLenum buffer, GLint drawbuffer, const GLuint *value);
    void glClearBufferiv(GLenum buffer, GLint drawbuffer, const GLint *value);
    void glGetTexParameterIuiv(GLenum target, GLenum pname, GLuint *params);
    void glGetTexParameterIiv(GLenum target, GLenum pname, GLint *params);
    void glTexParameterIuiv(GLenum target, GLenum pname, const GLuint *params);
    void glTexParameterIiv(GLenum target, GLenum pname, const GLint *params);
    void glUniform4uiv(GLint location, GLsizei count, const GLuint *value);
    void glUniform3uiv(GLint location, GLsizei count, const GLuint *value);
    void glUniform2uiv(GLint location, GLsizei count, const GLuint *value);
    void glUniform1uiv(GLint location, GLsizei count, const GLuint *value);
    void glUniform4ui(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
    void glUniform3ui(GLint location, GLuint v0, GLuint v1, GLuint v2);
    void glUniform2ui(GLint location, GLuint v0, GLuint v1);
    void glUniform1ui(GLint location, GLuint v0);
    GLint glGetFragDataLocation(GLuint program, const GLchar *name);
    void glBindFragDataLocation(GLuint program, GLuint color, const GLchar *name);
    void glGetUniformuiv(GLuint program, GLint location, GLuint *params);
    void glVertexAttribI4usv(GLuint index, const GLushort *v);
    void glVertexAttribI4ubv(GLuint index, const GLubyte *v);
    void glVertexAttribI4sv(GLuint index, const GLshort *v);
    void glVertexAttribI4bv(GLuint index, const GLbyte *v);
    void glVertexAttribI4uiv(GLuint index, const GLuint *v);
    void glVertexAttribI3uiv(GLuint index, const GLuint *v);
    void glVertexAttribI2uiv(GLuint index, const GLuint *v);
    void glVertexAttribI1uiv(GLuint index, const GLuint *v);
    void glVertexAttribI4iv(GLuint index, const GLint *v);
    void glVertexAttribI3iv(GLuint index, const GLint *v);
    void glVertexAttribI2iv(GLuint index, const GLint *v);
    void glVertexAttribI1iv(GLuint index, const GLint *v);
    void glVertexAttribI4ui(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w);
    void glVertexAttribI3ui(GLuint index, GLuint x, GLuint y, GLuint z);
    void glVertexAttribI2ui(GLuint index, GLuint x, GLuint y);
    void glVertexAttribI1ui(GLuint index, GLuint x);
    void glVertexAttribI4i(GLuint index, GLint x, GLint y, GLint z, GLint w);
    void glVertexAttribI3i(GLuint index, GLint x, GLint y, GLint z);
    void glVertexAttribI2i(GLuint index, GLint x, GLint y);
    void glVertexAttribI1i(GLuint index, GLint x);
    void glGetVertexAttribIuiv(GLuint index, GLenum pname, GLuint *params);
    void glGetVertexAttribIiv(GLuint index, GLenum pname, GLint *params);
    void glVertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer);
    void glEndConditionalRender();
    void glBeginConditionalRender(GLuint id, GLenum mode);
    void glClampColor(GLenum target, GLenum clamp);
    void glGetTransformFeedbackVarying(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLsizei *size, GLenum *type, GLchar *name);
    void glTransformFeedbackVaryings(GLuint program, GLsizei count, const GLchar* const *varyings, GLenum bufferMode);
    void glBindBufferBase(GLenum target, GLuint index, GLuint buffer);
    void glBindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
    void glEndTransformFeedback();
    void glBeginTransformFeedback(GLenum primitiveMode);
    GLboolean glIsEnabledi(GLenum target, GLuint index);
    void glDisablei(GLenum target, GLuint index);
    void glEnablei(GLenum target, GLuint index);
    void glGetIntegeri_v(GLenum target, GLuint index, GLint *data);
    void glGetBooleani_v(GLenum target, GLuint index, GLboolean *data);
    void glColorMaski(GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a);

    // OpenGL 3.1 core functions
    void glUniformBlockBinding(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);
    void glGetActiveUniformBlockName(GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformBlockName);
    void glGetActiveUniformBlockiv(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint *params);
    GLuint glGetUniformBlockIndex(GLuint program, const GLchar *uniformBlockName);
    void glGetActiveUniformName(GLuint program, GLuint uniformIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformName);
    void glGetActiveUniformsiv(GLuint program, GLsizei uniformCount, const GLuint *uniformIndices, GLenum pname, GLint *params);
    void glGetUniformIndices(GLuint program, GLsizei uniformCount, const GLchar *const *uniformNames, GLuint *uniformIndices);
    void glCopyBufferSubData(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
    void glPrimitiveRestartIndex(GLuint index);
    void glTexBuffer(GLenum target, GLenum internalformat, GLuint buffer);
    void glDrawElementsInstanced(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount);
    void glDrawArraysInstanced(GLenum mode, GLint first, GLsizei count, GLsizei instancecount);

    // OpenGL 3.2 core functions
    void glSampleMaski(GLuint maskNumber, GLbitfield mask);
    void glGetMultisamplefv(GLenum pname, GLuint index, GLfloat *val);
    void glTexImage3DMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations);
    void glTexImage2DMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
    void glFramebufferTexture(GLenum target, GLenum attachment, GLuint texture, GLint level);
    void glGetBufferParameteri64v(GLenum target, GLenum pname, GLint64 *params);
    void glGetInteger64i_v(GLenum target, GLuint index, GLint64 *data);
    void glGetSynciv(GLsync sync, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *values);
    void glGetInteger64v(GLenum pname, GLint64 *data);
    void glWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout);
    GLenum glClientWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout);
    void glDeleteSync(GLsync sync);
    GLboolean glIsSync(GLsync sync);
    GLsync glFenceSync(GLenum condition, GLbitfield flags);
    void glProvokingVertex(GLenum mode);
    void glMultiDrawElementsBaseVertex(GLenum mode, const GLsizei *count, GLenum type, const void *const *indices, GLsizei drawcount, const GLint *basevertex);
    void glDrawElementsInstancedBaseVertex(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex);
    void glDrawRangeElementsBaseVertex(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices, GLint basevertex);
    void glDrawElementsBaseVertex(GLenum mode, GLsizei count, GLenum type, const void *indices, GLint basevertex);

    // OpenGL 3.3 core functions
    void glVertexAttribP4uiv(GLuint index, GLenum type, GLboolean normalized, const GLuint *value);
    void glVertexAttribP4ui(GLuint index, GLenum type, GLboolean normalized, GLuint value);
    void glVertexAttribP3uiv(GLuint index, GLenum type, GLboolean normalized, const GLuint *value);
    void glVertexAttribP3ui(GLuint index, GLenum type, GLboolean normalized, GLuint value);
    void glVertexAttribP2uiv(GLuint index, GLenum type, GLboolean normalized, const GLuint *value);
    void glVertexAttribP2ui(GLuint index, GLenum type, GLboolean normalized, GLuint value);
    void glVertexAttribP1uiv(GLuint index, GLenum type, GLboolean normalized, const GLuint *value);
    void glVertexAttribP1ui(GLuint index, GLenum type, GLboolean normalized, GLuint value);
    void glVertexAttribDivisor(GLuint index, GLuint divisor);
    void glGetQueryObjectui64v(GLuint id, GLenum pname, GLuint64 *params);
    void glGetQueryObjecti64v(GLuint id, GLenum pname, GLint64 *params);
    void glQueryCounter(GLuint id, GLenum target);
    void glGetSamplerParameterIuiv(GLuint sampler, GLenum pname, GLuint *params);
    void glGetSamplerParameterfv(GLuint sampler, GLenum pname, GLfloat *params);
    void glGetSamplerParameterIiv(GLuint sampler, GLenum pname, GLint *params);
    void glGetSamplerParameteriv(GLuint sampler, GLenum pname, GLint *params);
    void glSamplerParameterIuiv(GLuint sampler, GLenum pname, const GLuint *param);
    void glSamplerParameterIiv(GLuint sampler, GLenum pname, const GLint *param);
    void glSamplerParameterfv(GLuint sampler, GLenum pname, const GLfloat *param);
    void glSamplerParameterf(GLuint sampler, GLenum pname, GLfloat param);
    void glSamplerParameteriv(GLuint sampler, GLenum pname, const GLint *param);
    void glSamplerParameteri(GLuint sampler, GLenum pname, GLint param);
    void glBindSampler(GLuint unit, GLuint sampler);
    GLboolean glIsSampler(GLuint sampler);
    void glDeleteSamplers(GLsizei count, const GLuint *samplers);
    void glGenSamplers(GLsizei count, GLuint *samplers);
    GLint glGetFragDataIndex(GLuint program, const GLchar *name);
    void glBindFragDataLocationIndexed(GLuint program, GLuint colorNumber, GLuint index, const GLchar *name);

    // OpenGL 4.0 core functions
    void glGetQueryIndexediv(GLenum target, GLuint index, GLenum pname, GLint *params);
    void glEndQueryIndexed(GLenum target, GLuint index);
    void glBeginQueryIndexed(GLenum target, GLuint index, GLuint id);
    void glDrawTransformFeedbackStream(GLenum mode, GLuint id, GLuint stream);
    void glDrawTransformFeedback(GLenum mode, GLuint id);
    void glResumeTransformFeedback();
    void glPauseTransformFeedback();
    GLboolean glIsTransformFeedback(GLuint id);
    void glGenTransformFeedbacks(GLsizei n, GLuint *ids);
    void glDeleteTransformFeedbacks(GLsizei n, const GLuint *ids);
    void glBindTransformFeedback(GLenum target, GLuint id);
    void glPatchParameterfv(GLenum pname, const GLfloat *values);
    void glPatchParameteri(GLenum pname, GLint value);
    void glGetProgramStageiv(GLuint program, GLenum shadertype, GLenum pname, GLint *values);
    void glGetUniformSubroutineuiv(GLenum shadertype, GLint location, GLuint *params);
    void glUniformSubroutinesuiv(GLenum shadertype, GLsizei count, const GLuint *indices);
    void glGetActiveSubroutineName(GLuint program, GLenum shadertype, GLuint index, GLsizei bufsize, GLsizei *length, GLchar *name);
    void glGetActiveSubroutineUniformName(GLuint program, GLenum shadertype, GLuint index, GLsizei bufsize, GLsizei *length, GLchar *name);
    void glGetActiveSubroutineUniformiv(GLuint program, GLenum shadertype, GLuint index, GLenum pname, GLint *values);
    GLuint glGetSubroutineIndex(GLuint program, GLenum shadertype, const GLchar *name);
    GLint glGetSubroutineUniformLocation(GLuint program, GLenum shadertype, const GLchar *name);
    void glGetUniformdv(GLuint program, GLint location, GLdouble *params);
    void glUniformMatrix4x3dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
    void glUniformMatrix4x2dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
    void glUniformMatrix3x4dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
    void glUniformMatrix3x2dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
    void glUniformMatrix2x4dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
    void glUniformMatrix2x3dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
    void glUniformMatrix4dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
    void glUniformMatrix3dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
    void glUniformMatrix2dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
    void glUniform4dv(GLint location, GLsizei count, const GLdouble *value);
    void glUniform3dv(GLint location, GLsizei count, const GLdouble *value);
    void glUniform2dv(GLint location, GLsizei count, const GLdouble *value);
    void glUniform1dv(GLint location, GLsizei count, const GLdouble *value);
    void glUniform4d(GLint location, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
    void glUniform3d(GLint location, GLdouble x, GLdouble y, GLdouble z);
    void glUniform2d(GLint location, GLdouble x, GLdouble y);
    void glUniform1d(GLint location, GLdouble x);
    void glDrawElementsIndirect(GLenum mode, GLenum type, const void *indirect);
    void glDrawArraysIndirect(GLenum mode, const void *indirect);
    void glBlendFuncSeparatei(GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);
    void glBlendFunci(GLuint buf, GLenum src, GLenum dst);
    void glBlendEquationSeparatei(GLuint buf, GLenum modeRGB, GLenum modeAlpha);
    void glBlendEquationi(GLuint buf, GLenum mode);
    void glMinSampleShading(GLfloat value);

    // OpenGL 4.1 core functions
    void glGetDoublei_v(GLenum target, GLuint index, GLdouble *data);
    void glGetFloati_v(GLenum target, GLuint index, GLfloat *data);
    void glDepthRangeIndexed(GLuint index, GLdouble n, GLdouble f);
    void glDepthRangeArrayv(GLuint first, GLsizei count, const GLdouble *v);
    void glScissorIndexedv(GLuint index, const GLint *v);
    void glScissorIndexed(GLuint index, GLint left, GLint bottom, GLsizei width, GLsizei height);
    void glScissorArrayv(GLuint first, GLsizei count, const GLint *v);
    void glViewportIndexedfv(GLuint index, const GLfloat *v);
    void glViewportIndexedf(GLuint index, GLfloat x, GLfloat y, GLfloat w, GLfloat h);
    void glViewportArrayv(GLuint first, GLsizei count, const GLfloat *v);
    void glGetVertexAttribLdv(GLuint index, GLenum pname, GLdouble *params);
    void glVertexAttribLPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer);
    void glVertexAttribL4dv(GLuint index, const GLdouble *v);
    void glVertexAttribL3dv(GLuint index, const GLdouble *v);
    void glVertexAttribL2dv(GLuint index, const GLdouble *v);
    void glVertexAttribL1dv(GLuint index, const GLdouble *v);
    void glVertexAttribL4d(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
    void glVertexAttribL3d(GLuint index, GLdouble x, GLdouble y, GLdouble z);
    void glVertexAttribL2d(GLuint index, GLdouble x, GLdouble y);
    void glVertexAttribL1d(GLuint index, GLdouble x);
    void glGetProgramPipelineInfoLog(GLuint pipeline, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
    void glValidateProgramPipeline(GLuint pipeline);
    void glProgramUniformMatrix4x3dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
    void glProgramUniformMatrix3x4dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
    void glProgramUniformMatrix4x2dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
    void glProgramUniformMatrix2x4dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
    void glProgramUniformMatrix3x2dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
    void glProgramUniformMatrix2x3dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
    void glProgramUniformMatrix4x3fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    void glProgramUniformMatrix3x4fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    void glProgramUniformMatrix4x2fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    void glProgramUniformMatrix2x4fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    void glProgramUniformMatrix3x2fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    void glProgramUniformMatrix2x3fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    void glProgramUniformMatrix4dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
    void glProgramUniformMatrix3dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
    void glProgramUniformMatrix2dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value);
    void glProgramUniformMatrix4fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    void glProgramUniformMatrix3fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    void glProgramUniformMatrix2fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
    void glProgramUniform4uiv(GLuint program, GLint location, GLsizei count, const GLuint *value);
    void glProgramUniform4ui(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
    void glProgramUniform4dv(GLuint program, GLint location, GLsizei count, const GLdouble *value);
    void glProgramUniform4d(GLuint program, GLint location, GLdouble v0, GLdouble v1, GLdouble v2, GLdouble v3);
    void glProgramUniform4fv(GLuint program, GLint location, GLsizei count, const GLfloat *value);
    void glProgramUniform4f(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
    void glProgramUniform4iv(GLuint program, GLint location, GLsizei count, const GLint *value);
    void glProgramUniform4i(GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
    void glProgramUniform3uiv(GLuint program, GLint location, GLsizei count, const GLuint *value);
    void glProgramUniform3ui(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2);
    void glProgramUniform3dv(GLuint program, GLint location, GLsizei count, const GLdouble *value);
    void glProgramUniform3d(GLuint program, GLint location, GLdouble v0, GLdouble v1, GLdouble v2);
    void glProgramUniform3fv(GLuint program, GLint location, GLsizei count, const GLfloat *value);
    void glProgramUniform3f(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
    void glProgramUniform3iv(GLuint program, GLint location, GLsizei count, const GLint *value);
    void glProgramUniform3i(GLuint program, GLint location, GLint v0, GLint v1, GLint v2);
    void glProgramUniform2uiv(GLuint program, GLint location, GLsizei count, const GLuint *value);
    void glProgramUniform2ui(GLuint program, GLint location, GLuint v0, GLuint v1);
    void glProgramUniform2dv(GLuint program, GLint location, GLsizei count, const GLdouble *value);
    void glProgramUniform2d(GLuint program, GLint location, GLdouble v0, GLdouble v1);
    void glProgramUniform2fv(GLuint program, GLint location, GLsizei count, const GLfloat *value);
    void glProgramUniform2f(GLuint program, GLint location, GLfloat v0, GLfloat v1);
    void glProgramUniform2iv(GLuint program, GLint location, GLsizei count, const GLint *value);
    void glProgramUniform2i(GLuint program, GLint location, GLint v0, GLint v1);
    void glProgramUniform1uiv(GLuint program, GLint location, GLsizei count, const GLuint *value);
    void glProgramUniform1ui(GLuint program, GLint location, GLuint v0);
    void glProgramUniform1dv(GLuint program, GLint location, GLsizei count, const GLdouble *value);
    void glProgramUniform1d(GLuint program, GLint location, GLdouble v0);
    void glProgramUniform1fv(GLuint program, GLint location, GLsizei count, const GLfloat *value);
    void glProgramUniform1f(GLuint program, GLint location, GLfloat v0);
    void glProgramUniform1iv(GLuint program, GLint location, GLsizei count, const GLint *value);
    void glProgramUniform1i(GLuint program, GLint location, GLint v0);
    void glGetProgramPipelineiv(GLuint pipeline, GLenum pname, GLint *params);
    GLboolean glIsProgramPipeline(GLuint pipeline);
    void glGenProgramPipelines(GLsizei n, GLuint *pipelines);
    void glDeleteProgramPipelines(GLsizei n, const GLuint *pipelines);
    void glBindProgramPipeline(GLuint pipeline);
    GLuint glCreateShaderProgramv(GLenum type, GLsizei count, const GLchar* const *strings);
    void glActiveShaderProgram(GLuint pipeline, GLuint program);
    void glUseProgramStages(GLuint pipeline, GLbitfield stages, GLuint program);
    void glProgramParameteri(GLuint program, GLenum pname, GLint value);
    void glProgramBinary(GLuint program, GLenum binaryFormat, const void *binary, GLsizei length);
    void glGetProgramBinary(GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, void *binary);
    void glClearDepthf(GLfloat dd);
    void glDepthRangef(GLfloat n, GLfloat f);
    void glGetShaderPrecisionFormat(GLenum shadertype, GLenum precisiontype, GLint *range, GLint *precision);
    void glShaderBinary(GLsizei count, const GLuint *shaders, GLenum binaryformat, const void *binary, GLsizei length);
    void glReleaseShaderCompiler();

    // OpenGL 4.2 core functions
    void glDrawTransformFeedbackStreamInstanced(GLenum mode, GLuint id, GLuint stream, GLsizei instancecount);
    void glDrawTransformFeedbackInstanced(GLenum mode, GLuint id, GLsizei instancecount);
    void glTexStorage3D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
    void glTexStorage2D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
    void glTexStorage1D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width);
    void glMemoryBarrier(GLbitfield barriers);
    void glBindImageTexture(GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format);
    void glGetActiveAtomicCounterBufferiv(GLuint program, GLuint bufferIndex, GLenum pname, GLint *params);
    void glGetInternalformativ(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint *params);
    void glDrawElementsInstancedBaseVertexBaseInstance(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex, GLuint baseinstance);
    void glDrawElementsInstancedBaseInstance(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLuint baseinstance);
    void glDrawArraysInstancedBaseInstance(GLenum mode, GLint first, GLsizei count, GLsizei instancecount, GLuint baseinstance);

    // OpenGL 4.3 core functions
    void glGetObjectPtrLabel(const void *ptr, GLsizei bufSize, GLsizei *length, GLchar *label);
    void glObjectPtrLabel(const void *ptr, GLsizei length, const GLchar *label);
    void glGetObjectLabel(GLenum identifier, GLuint name, GLsizei bufSize, GLsizei *length, GLchar *label);
    void glObjectLabel(GLenum identifier, GLuint name, GLsizei length, const GLchar *label);
    void glPopDebugGroup();
    void glPushDebugGroup(GLenum source, GLuint id, GLsizei length, const GLchar *message);
    GLuint glGetDebugMessageLog(GLuint count, GLsizei bufSize, GLenum *sources, GLenum *types, GLuint *ids, GLenum *severities, GLsizei *lengths, GLchar *messageLog);
    void glDebugMessageCallback(GLDEBUGPROC callback, const void *userParam);
    void glDebugMessageInsert(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *buf);
    void glDebugMessageControl(GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint *ids, GLboolean enabled);
    void glVertexBindingDivisor(GLuint bindingindex, GLuint divisor);
    void glVertexAttribBinding(GLuint attribindex, GLuint bindingindex);
    void glVertexAttribLFormat(GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
    void glVertexAttribIFormat(GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
    void glVertexAttribFormat(GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset);
    void glBindVertexBuffer(GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride);
    void glTextureView(GLuint texture, GLenum target, GLuint origtexture, GLenum internalformat, GLuint minlevel, GLuint numlevels, GLuint minlayer, GLuint numlayers);
    void glTexStorage3DMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations);
    void glTexStorage2DMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
    void glTexBufferRange(GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size);
    void glShaderStorageBlockBinding(GLuint program, GLuint storageBlockIndex, GLuint storageBlockBinding);
    GLint glGetProgramResourceLocationIndex(GLuint program, GLenum programInterface, const GLchar *name);
    GLint glGetProgramResourceLocation(GLuint program, GLenum programInterface, const GLchar *name);
    void glGetProgramResourceiv(GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum *props, GLsizei bufSize, GLsizei *length, GLint *params);
    void glGetProgramResourceName(GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name);
    GLuint glGetProgramResourceIndex(GLuint program, GLenum programInterface, const GLchar *name);
    void glGetProgramInterfaceiv(GLuint program, GLenum programInterface, GLenum pname, GLint *params);
    void glMultiDrawElementsIndirect(GLenum mode, GLenum type, const void *indirect, GLsizei drawcount, GLsizei stride);
    void glMultiDrawArraysIndirect(GLenum mode, const void *indirect, GLsizei drawcount, GLsizei stride);
    void glInvalidateSubFramebuffer(GLenum target, GLsizei numAttachments, const GLenum *attachments, GLint x, GLint y, GLsizei width, GLsizei height);
    void glInvalidateFramebuffer(GLenum target, GLsizei numAttachments, const GLenum *attachments);
    void glInvalidateBufferData(GLuint buffer);
    void glInvalidateBufferSubData(GLuint buffer, GLintptr offset, GLsizeiptr length);
    void glInvalidateTexImage(GLuint texture, GLint level);
    void glInvalidateTexSubImage(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth);
    void glGetInternalformati64v(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint64 *params);
    void glGetFramebufferParameteriv(GLenum target, GLenum pname, GLint *params);
    void glFramebufferParameteri(GLenum target, GLenum pname, GLint param);
    void glCopyImageSubData(GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth);
    void glDispatchComputeIndirect(GLintptr indirect);
    void glDispatchCompute(GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z);
    void glClearBufferSubData(GLenum target, GLenum internalformat, GLintptr offset, GLsizeiptr size, GLenum format, GLenum type, const void *data);
    void glClearBufferData(GLenum target, GLenum internalformat, GLenum format, GLenum type, const void *data);

    // OpenGL 4.4 core functions
    void glBindVertexBuffers(GLuint first, GLsizei count, const GLuint *buffers, const GLintptr *offsets, const GLsizei *strides);
    void glBindImageTextures(GLuint first, GLsizei count, const GLuint *textures);
    void glBindSamplers(GLuint first, GLsizei count, const GLuint *samplers);
    void glBindTextures(GLuint first, GLsizei count, const GLuint *textures);
    void glBindBuffersRange(GLenum target, GLuint first, GLsizei count, const GLuint *buffers, const GLintptr *offsets, const GLsizeiptr *sizes);
    void glBindBuffersBase(GLenum target, GLuint first, GLsizei count, const GLuint *buffers);
    void glClearTexSubImage(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *data);
    void glClearTexImage(GLuint texture, GLint level, GLenum format, GLenum type, const void *data);
    void glBufferStorage(GLenum target, GLsizeiptr size, const void *data, GLbitfield flags);

    // OpenGL 1.0 deprecated functions
    void glTranslatef(GLfloat x, GLfloat y, GLfloat z);
    void glTranslated(GLdouble x, GLdouble y, GLdouble z);
    void glScalef(GLfloat x, GLfloat y, GLfloat z);
    void glScaled(GLdouble x, GLdouble y, GLdouble z);
    void glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
    void glRotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
    void glPushMatrix();
    void glPopMatrix();
    void glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
    void glMultMatrixd(const GLdouble *m);
    void glMultMatrixf(const GLfloat *m);
    void glMatrixMode(GLenum mode);
    void glLoadMatrixd(const GLdouble *m);
    void glLoadMatrixf(const GLfloat *m);
    void glLoadIdentity();
    void glFrustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar);
    GLboolean glIsList(GLuint list);
    void glGetTexGeniv(GLenum coord, GLenum pname, GLint *params);
    void glGetTexGenfv(GLenum coord, GLenum pname, GLfloat *params);
    void glGetTexGendv(GLenum coord, GLenum pname, GLdouble *params);
    void glGetTexEnviv(GLenum target, GLenum pname, GLint *params);
    void glGetTexEnvfv(GLenum target, GLenum pname, GLfloat *params);
    void glGetPolygonStipple(GLubyte *mask);
    void glGetPixelMapusv(GLenum map, GLushort *values);
    void glGetPixelMapuiv(GLenum map, GLuint *values);
    void glGetPixelMapfv(GLenum map, GLfloat *values);
    void glGetMaterialiv(GLenum face, GLenum pname, GLint *params);
    void glGetMaterialfv(GLenum face, GLenum pname, GLfloat *params);
    void glGetMapiv(GLenum target, GLenum query, GLint *v);
    void glGetMapfv(GLenum target, GLenum query, GLfloat *v);
    void glGetMapdv(GLenum target, GLenum query, GLdouble *v);
    void glGetLightiv(GLenum light, GLenum pname, GLint *params);
    void glGetLightfv(GLenum light, GLenum pname, GLfloat *params);
    void glGetClipPlane(GLenum plane, GLdouble *equation);
    void glDrawPixels(GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
    void glCopyPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type);
    void glPixelMapusv(GLenum map, GLsizei mapsize, const GLushort *values);
    void glPixelMapuiv(GLenum map, GLsizei mapsize, const GLuint *values);
    void glPixelMapfv(GLenum map, GLsizei mapsize, const GLfloat *values);
    void glPixelTransferi(GLenum pname, GLint param);
    void glPixelTransferf(GLenum pname, GLfloat param);
    void glPixelZoom(GLfloat xfactor, GLfloat yfactor);
    void glAlphaFunc(GLenum func, GLfloat ref);
    void glEvalPoint2(GLint i, GLint j);
    void glEvalMesh2(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2);
    void glEvalPoint1(GLint i);
    void glEvalMesh1(GLenum mode, GLint i1, GLint i2);
    void glEvalCoord2fv(const GLfloat *u);
    void glEvalCoord2f(GLfloat u, GLfloat v);
    void glEvalCoord2dv(const GLdouble *u);
    void glEvalCoord2d(GLdouble u, GLdouble v);
    void glEvalCoord1fv(const GLfloat *u);
    void glEvalCoord1f(GLfloat u);
    void glEvalCoord1dv(const GLdouble *u);
    void glEvalCoord1d(GLdouble u);
    void glMapGrid2f(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2);
    void glMapGrid2d(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2);
    void glMapGrid1f(GLint un, GLfloat u1, GLfloat u2);
    void glMapGrid1d(GLint un, GLdouble u1, GLdouble u2);
    void glMap2f(GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points);
    void glMap2d(GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points);
    void glMap1f(GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points);
    void glMap1d(GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points);
    void glPushAttrib(GLbitfield mask);
    void glPopAttrib();
    void glAccum(GLenum op, GLfloat value);
    void glIndexMask(GLuint mask);
    void glClearIndex(GLfloat c);
    void glClearAccum(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
    void glPushName(GLuint name);
    void glPopName();
    void glPassThrough(GLfloat token);
    void glLoadName(GLuint name);
    void glInitNames();
    GLint glRenderMode(GLenum mode);
    void glSelectBuffer(GLsizei size, GLuint *buffer);
    void glFeedbackBuffer(GLsizei size, GLenum type, GLfloat *buffer);
    void glTexGeniv(GLenum coord, GLenum pname, const GLint *params);
    void glTexGeni(GLenum coord, GLenum pname, GLint param);
    void glTexGenfv(GLenum coord, GLenum pname, const GLfloat *params);
    void glTexGenf(GLenum coord, GLenum pname, GLfloat param);
    void glTexGendv(GLenum coord, GLenum pname, const GLdouble *params);
    void glTexGend(GLenum coord, GLenum pname, GLdouble param);
    void glTexEnviv(GLenum target, GLenum pname, const GLint *params);
    void glTexEnvi(GLenum target, GLenum pname, GLint param);
    void glTexEnvfv(GLenum target, GLenum pname, const GLfloat *params);
    void glTexEnvf(GLenum target, GLenum pname, GLfloat param);
    void glShadeModel(GLenum mode);
    void glPolygonStipple(const GLubyte *mask);
    void glMaterialiv(GLenum face, GLenum pname, const GLint *params);
    void glMateriali(GLenum face, GLenum pname, GLint param);
    void glMaterialfv(GLenum face, GLenum pname, const GLfloat *params);
    void glMaterialf(GLenum face, GLenum pname, GLfloat param);
    void glLineStipple(GLint factor, GLushort pattern);
    void glLightModeliv(GLenum pname, const GLint *params);
    void glLightModeli(GLenum pname, GLint param);
    void glLightModelfv(GLenum pname, const GLfloat *params);
    void glLightModelf(GLenum pname, GLfloat param);
    void glLightiv(GLenum light, GLenum pname, const GLint *params);
    void glLighti(GLenum light, GLenum pname, GLint param);
    void glLightfv(GLenum light, GLenum pname, const GLfloat *params);
    void glLightf(GLenum light, GLenum pname, GLfloat param);
    void glFogiv(GLenum pname, const GLint *params);
    void glFogi(GLenum pname, GLint param);
    void glFogfv(GLenum pname, const GLfloat *params);
    void glFogf(GLenum pname, GLfloat param);
    void glColorMaterial(GLenum face, GLenum mode);
    void glClipPlane(GLenum plane, const GLdouble *equation);
    void glVertex4sv(const GLshort *v);
    void glVertex4s(GLshort x, GLshort y, GLshort z, GLshort w);
    void glVertex4iv(const GLint *v);
    void glVertex4i(GLint x, GLint y, GLint z, GLint w);
    void glVertex4fv(const GLfloat *v);
    void glVertex4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
    void glVertex4dv(const GLdouble *v);
    void glVertex4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w);
    void glVertex3sv(const GLshort *v);
    void glVertex3s(GLshort x, GLshort y, GLshort z);
    void glVertex3iv(const GLint *v);
    void glVertex3i(GLint x, GLint y, GLint z);
    void glVertex3fv(const GLfloat *v);
    void glVertex3f(GLfloat x, GLfloat y, GLfloat z);
    void glVertex3dv(const GLdouble *v);
    void glVertex3d(GLdouble x, GLdouble y, GLdouble z);
    void glVertex2sv(const GLshort *v);
    void glVertex2s(GLshort x, GLshort y);
    void glVertex2iv(const GLint *v);
    void glVertex2i(GLint x, GLint y);
    void glVertex2fv(const GLfloat *v);
    void glVertex2f(GLfloat x, GLfloat y);
    void glVertex2dv(const GLdouble *v);
    void glVertex2d(GLdouble x, GLdouble y);
    void glTexCoord4sv(const GLshort *v);
    void glTexCoord4s(GLshort s, GLshort t, GLshort r, GLshort q);
    void glTexCoord4iv(const GLint *v);
    void glTexCoord4i(GLint s, GLint t, GLint r, GLint q);
    void glTexCoord4fv(const GLfloat *v);
    void glTexCoord4f(GLfloat s, GLfloat t, GLfloat r, GLfloat q);
    void glTexCoord4dv(const GLdouble *v);
    void glTexCoord4d(GLdouble s, GLdouble t, GLdouble r, GLdouble q);
    void glTexCoord3sv(const GLshort *v);
    void glTexCoord3s(GLshort s, GLshort t, GLshort r);
    void glTexCoord3iv(const GLint *v);
    void glTexCoord3i(GLint s, GLint t, GLint r);
    void glTexCoord3fv(const GLfloat *v);
    void glTexCoord3f(GLfloat s, GLfloat t, GLfloat r);
    void glTexCoord3dv(const GLdouble *v);
    void glTexCoord3d(GLdouble s, GLdouble t, GLdouble r);
    void glTexCoord2sv(const GLshort *v);
    void glTexCoord2s(GLshort s, GLshort t);
    void glTexCoord2iv(const GLint *v);
    void glTexCoord2i(GLint s, GLint t);
    void glTexCoord2fv(const GLfloat *v);
    void glTexCoord2f(GLfloat s, GLfloat t);
    void glTexCoord2dv(const GLdouble *v);
    void glTexCoord2d(GLdouble s, GLdouble t);
    void glTexCoord1sv(const GLshort *v);
    void glTexCoord1s(GLshort s);
    void glTexCoord1iv(const GLint *v);
    void glTexCoord1i(GLint s);
    void glTexCoord1fv(const GLfloat *v);
    void glTexCoord1f(GLfloat s);
    void glTexCoord1dv(const GLdouble *v);
    void glTexCoord1d(GLdouble s);
    void glRectsv(const GLshort *v1, const GLshort *v2);
    void glRects(GLshort x1, GLshort y1, GLshort x2, GLshort y2);
    void glRectiv(const GLint *v1, const GLint *v2);
    void glRecti(GLint x1, GLint y1, GLint x2, GLint y2);
    void glRectfv(const GLfloat *v1, const GLfloat *v2);
    void glRectf(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);
    void glRectdv(const GLdouble *v1, const GLdouble *v2);
    void glRectd(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2);
    void glRasterPos4sv(const GLshort *v);
    void glRasterPos4s(GLshort x, GLshort y, GLshort z, GLshort w);
    void glRasterPos4iv(const GLint *v);
    void glRasterPos4i(GLint x, GLint y, GLint z, GLint w);
    void glRasterPos4fv(const GLfloat *v);
    void glRasterPos4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w);
    void glRasterPos4dv(const GLdouble *v);
    void glRasterPos4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w);
    void glRasterPos3sv(const GLshort *v);
    void glRasterPos3s(GLshort x, GLshort y, GLshort z);
    void glRasterPos3iv(const GLint *v);
    void glRasterPos3i(GLint x, GLint y, GLint z);
    void glRasterPos3fv(const GLfloat *v);
    void glRasterPos3f(GLfloat x, GLfloat y, GLfloat z);
    void glRasterPos3dv(const GLdouble *v);
    void glRasterPos3d(GLdouble x, GLdouble y, GLdouble z);
    void glRasterPos2sv(const GLshort *v);
    void glRasterPos2s(GLshort x, GLshort y);
    void glRasterPos2iv(const GLint *v);
    void glRasterPos2i(GLint x, GLint y);
    void glRasterPos2fv(const GLfloat *v);
    void glRasterPos2f(GLfloat x, GLfloat y);
    void glRasterPos2dv(const GLdouble *v);
    void glRasterPos2d(GLdouble x, GLdouble y);
    void glNormal3sv(const GLshort *v);
    void glNormal3s(GLshort nx, GLshort ny, GLshort nz);
    void glNormal3iv(const GLint *v);
    void glNormal3i(GLint nx, GLint ny, GLint nz);
    void glNormal3fv(const GLfloat *v);
    void glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz);
    void glNormal3dv(const GLdouble *v);
    void glNormal3d(GLdouble nx, GLdouble ny, GLdouble nz);
    void glNormal3bv(const GLbyte *v);
    void glNormal3b(GLbyte nx, GLbyte ny, GLbyte nz);
    void glIndexsv(const GLshort *c);
    void glIndexs(GLshort c);
    void glIndexiv(const GLint *c);
    void glIndexi(GLint c);
    void glIndexfv(const GLfloat *c);
    void glIndexf(GLfloat c);
    void glIndexdv(const GLdouble *c);
    void glIndexd(GLdouble c);
    void glEnd();
    void glEdgeFlagv(const GLboolean *flag);
    void glEdgeFlag(GLboolean flag);
    void glColor4usv(const GLushort *v);
    void glColor4us(GLushort red, GLushort green, GLushort blue, GLushort alpha);
    void glColor4uiv(const GLuint *v);
    void glColor4ui(GLuint red, GLuint green, GLuint blue, GLuint alpha);
    void glColor4ubv(const GLubyte *v);
    void glColor4ub(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
    void glColor4sv(const GLshort *v);
    void glColor4s(GLshort red, GLshort green, GLshort blue, GLshort alpha);
    void glColor4iv(const GLint *v);
    void glColor4i(GLint red, GLint green, GLint blue, GLint alpha);
    void glColor4fv(const GLfloat *v);
    void glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
    void glColor4dv(const GLdouble *v);
    void glColor4d(GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha);
    void glColor4bv(const GLbyte *v);
    void glColor4b(GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha);
    void glColor3usv(const GLushort *v);
    void glColor3us(GLushort red, GLushort green, GLushort blue);
    void glColor3uiv(const GLuint *v);
    void glColor3ui(GLuint red, GLuint green, GLuint blue);
    void glColor3ubv(const GLubyte *v);
    void glColor3ub(GLubyte red, GLubyte green, GLubyte blue);
    void glColor3sv(const GLshort *v);
    void glColor3s(GLshort red, GLshort green, GLshort blue);
    void glColor3iv(const GLint *v);
    void glColor3i(GLint red, GLint green, GLint blue);
    void glColor3fv(const GLfloat *v);
    void glColor3f(GLfloat red, GLfloat green, GLfloat blue);
    void glColor3dv(const GLdouble *v);
    void glColor3d(GLdouble red, GLdouble green, GLdouble blue);
    void glColor3bv(const GLbyte *v);
    void glColor3b(GLbyte red, GLbyte green, GLbyte blue);
    void glBitmap(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap);
    void glBegin(GLenum mode);
    void glListBase(GLuint base);
    GLuint glGenLists(GLsizei range);
    void glDeleteLists(GLuint list, GLsizei range);
    void glCallLists(GLsizei n, GLenum type, const void *lists);
    void glCallList(GLuint list);
    void glEndList();
    void glNewList(GLuint list, GLenum mode);

    // OpenGL 1.1 deprecated functions
    void glPushClientAttrib(GLbitfield mask);
    void glPopClientAttrib();
    void glIndexubv(const GLubyte *c);
    void glIndexub(GLubyte c);
    void glPrioritizeTextures(GLsizei n, const GLuint *textures, const GLfloat *priorities);
    GLboolean glAreTexturesResident(GLsizei n, const GLuint *textures, GLboolean *residences);
    void glVertexPointer(GLint size, GLenum type, GLsizei stride, const void *pointer);
    void glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const void *pointer);
    void glNormalPointer(GLenum type, GLsizei stride, const void *pointer);
    void glInterleavedArrays(GLenum format, GLsizei stride, const void *pointer);
    void glGetPointerv(GLenum pname, void * *params);
    void glIndexPointer(GLenum type, GLsizei stride, const void *pointer);
    void glEnableClientState(GLenum array);
    void glEdgeFlagPointer(GLsizei stride, const void *pointer);
    void glDisableClientState(GLenum array);
    void glColorPointer(GLint size, GLenum type, GLsizei stride, const void *pointer);
    void glArrayElement(GLint i);

    // OpenGL 1.2 deprecated functions
    void glColorTable(GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const void *table);
    void glColorTableParameterfv(GLenum target, GLenum pname, const GLfloat *params);
    void glColorTableParameteriv(GLenum target, GLenum pname, const GLint *params);
    void glCopyColorTable(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width);
    void glGetColorTable(GLenum target, GLenum format, GLenum type, void *table);
    void glGetColorTableParameterfv(GLenum target, GLenum pname, GLfloat *params);
    void glGetColorTableParameteriv(GLenum target, GLenum pname, GLint *params);
    void glColorSubTable(GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const void *data);
    void glCopyColorSubTable(GLenum target, GLsizei start, GLint x, GLint y, GLsizei width);
    void glConvolutionFilter1D(GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const void *image);
    void glConvolutionFilter2D(GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *image);
    void glConvolutionParameterf(GLenum target, GLenum pname, GLfloat params);
    void glConvolutionParameterfv(GLenum target, GLenum pname, const GLfloat *params);
    void glConvolutionParameteri(GLenum target, GLenum pname, GLint params);
    void glConvolutionParameteriv(GLenum target, GLenum pname, const GLint *params);
    void glCopyConvolutionFilter1D(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width);
    void glCopyConvolutionFilter2D(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height);
    void glGetConvolutionFilter(GLenum target, GLenum format, GLenum type, void *image);
    void glGetConvolutionParameterfv(GLenum target, GLenum pname, GLfloat *params);
    void glGetConvolutionParameteriv(GLenum target, GLenum pname, GLint *params);
    void glGetSeparableFilter(GLenum target, GLenum format, GLenum type, void *row, void *column, void *span);
    void glSeparableFilter2D(GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *row, const void *column);
    void glGetHistogram(GLenum target, GLboolean reset, GLenum format, GLenum type, void *values);
    void glGetHistogramParameterfv(GLenum target, GLenum pname, GLfloat *params);
    void glGetHistogramParameteriv(GLenum target, GLenum pname, GLint *params);
    void glGetMinmax(GLenum target, GLboolean reset, GLenum format, GLenum type, void *values);
    void glGetMinmaxParameterfv(GLenum target, GLenum pname, GLfloat *params);
    void glGetMinmaxParameteriv(GLenum target, GLenum pname, GLint *params);
    void glHistogram(GLenum target, GLsizei width, GLenum internalformat, GLboolean sink);
    void glMinmax(GLenum target, GLenum internalformat, GLboolean sink);
    void glResetHistogram(GLenum target);
    void glResetMinmax(GLenum target);

    // OpenGL 1.3 deprecated functions
    void glMultTransposeMatrixd(const GLdouble *m);
    void glMultTransposeMatrixf(const GLfloat *m);
    void glLoadTransposeMatrixd(const GLdouble *m);
    void glLoadTransposeMatrixf(const GLfloat *m);
    void glMultiTexCoord4sv(GLenum target, const GLshort *v);
    void glMultiTexCoord4s(GLenum target, GLshort s, GLshort t, GLshort r, GLshort q);
    void glMultiTexCoord4iv(GLenum target, const GLint *v);
    void glMultiTexCoord4i(GLenum target, GLint s, GLint t, GLint r, GLint q);
    void glMultiTexCoord4fv(GLenum target, const GLfloat *v);
    void glMultiTexCoord4f(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
    void glMultiTexCoord4dv(GLenum target, const GLdouble *v);
    void glMultiTexCoord4d(GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
    void glMultiTexCoord3sv(GLenum target, const GLshort *v);
    void glMultiTexCoord3s(GLenum target, GLshort s, GLshort t, GLshort r);
    void glMultiTexCoord3iv(GLenum target, const GLint *v);
    void glMultiTexCoord3i(GLenum target, GLint s, GLint t, GLint r);
    void glMultiTexCoord3fv(GLenum target, const GLfloat *v);
    void glMultiTexCoord3f(GLenum target, GLfloat s, GLfloat t, GLfloat r);
    void glMultiTexCoord3dv(GLenum target, const GLdouble *v);
    void glMultiTexCoord3d(GLenum target, GLdouble s, GLdouble t, GLdouble r);
    void glMultiTexCoord2sv(GLenum target, const GLshort *v);
    void glMultiTexCoord2s(GLenum target, GLshort s, GLshort t);
    void glMultiTexCoord2iv(GLenum target, const GLint *v);
    void glMultiTexCoord2i(GLenum target, GLint s, GLint t);
    void glMultiTexCoord2fv(GLenum target, const GLfloat *v);
    void glMultiTexCoord2f(GLenum target, GLfloat s, GLfloat t);
    void glMultiTexCoord2dv(GLenum target, const GLdouble *v);
    void glMultiTexCoord2d(GLenum target, GLdouble s, GLdouble t);
    void glMultiTexCoord1sv(GLenum target, const GLshort *v);
    void glMultiTexCoord1s(GLenum target, GLshort s);
    void glMultiTexCoord1iv(GLenum target, const GLint *v);
    void glMultiTexCoord1i(GLenum target, GLint s);
    void glMultiTexCoord1fv(GLenum target, const GLfloat *v);
    void glMultiTexCoord1f(GLenum target, GLfloat s);
    void glMultiTexCoord1dv(GLenum target, const GLdouble *v);
    void glMultiTexCoord1d(GLenum target, GLdouble s);
    void glClientActiveTexture(GLenum texture);

    // OpenGL 1.4 deprecated functions
    void glWindowPos3sv(const GLshort *v);
    void glWindowPos3s(GLshort x, GLshort y, GLshort z);
    void glWindowPos3iv(const GLint *v);
    void glWindowPos3i(GLint x, GLint y, GLint z);
    void glWindowPos3fv(const GLfloat *v);
    void glWindowPos3f(GLfloat x, GLfloat y, GLfloat z);
    void glWindowPos3dv(const GLdouble *v);
    void glWindowPos3d(GLdouble x, GLdouble y, GLdouble z);
    void glWindowPos2sv(const GLshort *v);
    void glWindowPos2s(GLshort x, GLshort y);
    void glWindowPos2iv(const GLint *v);
    void glWindowPos2i(GLint x, GLint y);
    void glWindowPos2fv(const GLfloat *v);
    void glWindowPos2f(GLfloat x, GLfloat y);
    void glWindowPos2dv(const GLdouble *v);
    void glWindowPos2d(GLdouble x, GLdouble y);
    void glSecondaryColorPointer(GLint size, GLenum type, GLsizei stride, const void *pointer);
    void glSecondaryColor3usv(const GLushort *v);
    void glSecondaryColor3us(GLushort red, GLushort green, GLushort blue);
    void glSecondaryColor3uiv(const GLuint *v);
    void glSecondaryColor3ui(GLuint red, GLuint green, GLuint blue);
    void glSecondaryColor3ubv(const GLubyte *v);
    void glSecondaryColor3ub(GLubyte red, GLubyte green, GLubyte blue);
    void glSecondaryColor3sv(const GLshort *v);
    void glSecondaryColor3s(GLshort red, GLshort green, GLshort blue);
    void glSecondaryColor3iv(const GLint *v);
    void glSecondaryColor3i(GLint red, GLint green, GLint blue);
    void glSecondaryColor3fv(const GLfloat *v);
    void glSecondaryColor3f(GLfloat red, GLfloat green, GLfloat blue);
    void glSecondaryColor3dv(const GLdouble *v);
    void glSecondaryColor3d(GLdouble red, GLdouble green, GLdouble blue);
    void glSecondaryColor3bv(const GLbyte *v);
    void glSecondaryColor3b(GLbyte red, GLbyte green, GLbyte blue);
    void glFogCoordPointer(GLenum type, GLsizei stride, const void *pointer);
    void glFogCoorddv(const GLdouble *coord);
    void glFogCoordd(GLdouble coord);
    void glFogCoordfv(const GLfloat *coord);
    void glFogCoordf(GLfloat coord);

    // OpenGL 1.5 deprecated functions

    // OpenGL 2.0 deprecated functions

    // OpenGL 2.1 deprecated functions

    // OpenGL 3.0 deprecated functions

    // OpenGL 3.1 deprecated functions

    // OpenGL 3.2 deprecated functions

    // OpenGL 3.3 deprecated functions
    void glSecondaryColorP3uiv(GLenum type, const GLuint *color);
    void glSecondaryColorP3ui(GLenum type, GLuint color);
    void glColorP4uiv(GLenum type, const GLuint *color);
    void glColorP4ui(GLenum type, GLuint color);
    void glColorP3uiv(GLenum type, const GLuint *color);
    void glColorP3ui(GLenum type, GLuint color);
    void glNormalP3uiv(GLenum type, const GLuint *coords);
    void glNormalP3ui(GLenum type, GLuint coords);
    void glMultiTexCoordP4uiv(GLenum texture, GLenum type, const GLuint *coords);
    void glMultiTexCoordP4ui(GLenum texture, GLenum type, GLuint coords);
    void glMultiTexCoordP3uiv(GLenum texture, GLenum type, const GLuint *coords);
    void glMultiTexCoordP3ui(GLenum texture, GLenum type, GLuint coords);
    void glMultiTexCoordP2uiv(GLenum texture, GLenum type, const GLuint *coords);
    void glMultiTexCoordP2ui(GLenum texture, GLenum type, GLuint coords);
    void glMultiTexCoordP1uiv(GLenum texture, GLenum type, const GLuint *coords);
    void glMultiTexCoordP1ui(GLenum texture, GLenum type, GLuint coords);
    void glTexCoordP4uiv(GLenum type, const GLuint *coords);
    void glTexCoordP4ui(GLenum type, GLuint coords);
    void glTexCoordP3uiv(GLenum type, const GLuint *coords);
    void glTexCoordP3ui(GLenum type, GLuint coords);
    void glTexCoordP2uiv(GLenum type, const GLuint *coords);
    void glTexCoordP2ui(GLenum type, GLuint coords);
    void glTexCoordP1uiv(GLenum type, const GLuint *coords);
    void glTexCoordP1ui(GLenum type, GLuint coords);
    void glVertexP4uiv(GLenum type, const GLuint *value);
    void glVertexP4ui(GLenum type, GLuint value);
    void glVertexP3uiv(GLenum type, const GLuint *value);
    void glVertexP3ui(GLenum type, GLuint value);
    void glVertexP2uiv(GLenum type, const GLuint *value);
    void glVertexP2ui(GLenum type, GLuint value);

    // OpenGL 4.0 deprecated functions

    // OpenGL 4.1 deprecated functions

    // OpenGL 4.2 deprecated functions

    // OpenGL 4.3 deprecated functions

    // OpenGL 4.4 deprecated functions

private:
    friend class QOpenGLContext;

    static bool isContextCompatible(QOpenGLContext *context);
    static QOpenGLVersionProfile versionProfile();

    QOpenGLFunctions_1_0_CoreBackend* d_1_0_Core;
    QOpenGLFunctions_1_1_CoreBackend* d_1_1_Core;
    QOpenGLFunctions_1_2_CoreBackend* d_1_2_Core;
    QOpenGLFunctions_1_3_CoreBackend* d_1_3_Core;
    QOpenGLFunctions_1_4_CoreBackend* d_1_4_Core;
    QOpenGLFunctions_1_5_CoreBackend* d_1_5_Core;
    QOpenGLFunctions_2_0_CoreBackend* d_2_0_Core;
    QOpenGLFunctions_2_1_CoreBackend* d_2_1_Core;
    QOpenGLFunctions_3_0_CoreBackend* d_3_0_Core;
    QOpenGLFunctions_3_1_CoreBackend* d_3_1_Core;
    QOpenGLFunctions_3_2_CoreBackend* d_3_2_Core;
    QOpenGLFunctions_3_3_CoreBackend* d_3_3_Core;
    QOpenGLFunctions_4_0_CoreBackend* d_4_0_Core;
    QOpenGLFunctions_4_1_CoreBackend* d_4_1_Core;
    QOpenGLFunctions_4_2_CoreBackend* d_4_2_Core;
    QOpenGLFunctions_4_3_CoreBackend* d_4_3_Core;
    QOpenGLFunctions_4_4_CoreBackend* d_4_4_Core;
    QOpenGLFunctions_1_0_DeprecatedBackend* d_1_0_Deprecated;
    QOpenGLFunctions_1_1_DeprecatedBackend* d_1_1_Deprecated;
    QOpenGLFunctions_1_2_DeprecatedBackend* d_1_2_Deprecated;
    QOpenGLFunctions_1_3_DeprecatedBackend* d_1_3_Deprecated;
    QOpenGLFunctions_1_4_DeprecatedBackend* d_1_4_Deprecated;
    QOpenGLFunctions_3_3_DeprecatedBackend* d_3_3_Deprecated;
};

// OpenGL 1.0 core functions
inline void QOpenGLFunctions_4_4_Compatibility::glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
    d_1_0_Core->Viewport(x, y, width, height);
}

inline void QOpenGLFunctions_4_4_Compatibility::glDepthRange(GLdouble nearVal, GLdouble farVal)
{
    d_1_0_Core->DepthRange(nearVal, farVal);
}

inline GLboolean QOpenGLFunctions_4_4_Compatibility::glIsEnabled(GLenum cap)
{
    return d_1_0_Core->IsEnabled(cap);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetTexLevelParameteriv(GLenum target, GLint level, GLenum pname, GLint *params)
{
    d_1_0_Core->GetTexLevelParameteriv(target, level, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetTexLevelParameterfv(GLenum target, GLint level, GLenum pname, GLfloat *params)
{
    d_1_0_Core->GetTexLevelParameterfv(target, level, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetTexParameteriv(GLenum target, GLenum pname, GLint *params)
{
    d_1_0_Core->GetTexParameteriv(target, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetTexParameterfv(GLenum target, GLenum pname, GLfloat *params)
{
    d_1_0_Core->GetTexParameterfv(target, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetTexImage(GLenum target, GLint level, GLenum format, GLenum type, void *pixels)
{
    d_1_0_Core->GetTexImage(target, level, format, type, pixels);
}

inline const GLubyte * QOpenGLFunctions_4_4_Compatibility::glGetString(GLenum name)
{
    return d_1_0_Core->GetString(name);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetIntegerv(GLenum pname, GLint *data)
{
    d_1_0_Core->GetIntegerv(pname, data);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetFloatv(GLenum pname, GLfloat *data)
{
    d_1_0_Core->GetFloatv(pname, data);
}

inline GLenum QOpenGLFunctions_4_4_Compatibility::glGetError()
{
    return d_1_0_Core->GetError();
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetDoublev(GLenum pname, GLdouble *data)
{
    d_1_0_Core->GetDoublev(pname, data);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetBooleanv(GLenum pname, GLboolean *data)
{
    d_1_0_Core->GetBooleanv(pname, data);
}

inline void QOpenGLFunctions_4_4_Compatibility::glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels)
{
    d_1_0_Core->ReadPixels(x, y, width, height, format, type, pixels);
}

inline void QOpenGLFunctions_4_4_Compatibility::glReadBuffer(GLenum src)
{
    d_1_0_Core->ReadBuffer(src);
}

inline void QOpenGLFunctions_4_4_Compatibility::glPixelStorei(GLenum pname, GLint param)
{
    d_1_0_Core->PixelStorei(pname, param);
}

inline void QOpenGLFunctions_4_4_Compatibility::glPixelStoref(GLenum pname, GLfloat param)
{
    d_1_0_Core->PixelStoref(pname, param);
}

inline void QOpenGLFunctions_4_4_Compatibility::glDepthFunc(GLenum func)
{
    d_1_0_Core->DepthFunc(func);
}

inline void QOpenGLFunctions_4_4_Compatibility::glStencilOp(GLenum fail, GLenum zfail, GLenum zpass)
{
    d_1_0_Core->StencilOp(fail, zfail, zpass);
}

inline void QOpenGLFunctions_4_4_Compatibility::glStencilFunc(GLenum func, GLint ref, GLuint mask)
{
    d_1_0_Core->StencilFunc(func, ref, mask);
}

inline void QOpenGLFunctions_4_4_Compatibility::glLogicOp(GLenum opcode)
{
    d_1_0_Core->LogicOp(opcode);
}

inline void QOpenGLFunctions_4_4_Compatibility::glBlendFunc(GLenum sfactor, GLenum dfactor)
{
    d_1_0_Core->BlendFunc(sfactor, dfactor);
}

inline void QOpenGLFunctions_4_4_Compatibility::glFlush()
{
    d_1_0_Core->Flush();
}

inline void QOpenGLFunctions_4_4_Compatibility::glFinish()
{
    d_1_0_Core->Finish();
}

inline void QOpenGLFunctions_4_4_Compatibility::glEnable(GLenum cap)
{
    d_1_0_Core->Enable(cap);
}

inline void QOpenGLFunctions_4_4_Compatibility::glDisable(GLenum cap)
{
    d_1_0_Core->Disable(cap);
}

inline void QOpenGLFunctions_4_4_Compatibility::glDepthMask(GLboolean flag)
{
    d_1_0_Core->DepthMask(flag);
}

inline void QOpenGLFunctions_4_4_Compatibility::glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
    d_1_0_Core->ColorMask(red, green, blue, alpha);
}

inline void QOpenGLFunctions_4_4_Compatibility::glStencilMask(GLuint mask)
{
    d_1_0_Core->StencilMask(mask);
}

inline void QOpenGLFunctions_4_4_Compatibility::glClearDepth(GLdouble depth)
{
    d_1_0_Core->ClearDepth(depth);
}

inline void QOpenGLFunctions_4_4_Compatibility::glClearStencil(GLint s)
{
    d_1_0_Core->ClearStencil(s);
}

inline void QOpenGLFunctions_4_4_Compatibility::glClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    d_1_0_Core->ClearColor(red, green, blue, alpha);
}

inline void QOpenGLFunctions_4_4_Compatibility::glClear(GLbitfield mask)
{
    d_1_0_Core->Clear(mask);
}

inline void QOpenGLFunctions_4_4_Compatibility::glDrawBuffer(GLenum buf)
{
    d_1_0_Core->DrawBuffer(buf);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels)
{
    d_1_0_Core->TexImage2D(target, level, internalformat, width, height, border, format, type, pixels);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexImage1D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const void *pixels)
{
    d_1_0_Core->TexImage1D(target, level, internalformat, width, border, format, type, pixels);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexParameteriv(GLenum target, GLenum pname, const GLint *params)
{
    d_1_0_Core->TexParameteriv(target, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexParameteri(GLenum target, GLenum pname, GLint param)
{
    d_1_0_Core->TexParameteri(target, pname, param);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexParameterfv(GLenum target, GLenum pname, const GLfloat *params)
{
    d_1_0_Core->TexParameterfv(target, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexParameterf(GLenum target, GLenum pname, GLfloat param)
{
    d_1_0_Core->TexParameterf(target, pname, param);
}

inline void QOpenGLFunctions_4_4_Compatibility::glScissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
    d_1_0_Core->Scissor(x, y, width, height);
}

inline void QOpenGLFunctions_4_4_Compatibility::glPolygonMode(GLenum face, GLenum mode)
{
    d_1_0_Core->PolygonMode(face, mode);
}

inline void QOpenGLFunctions_4_4_Compatibility::glPointSize(GLfloat size)
{
    d_1_0_Core->PointSize(size);
}

inline void QOpenGLFunctions_4_4_Compatibility::glLineWidth(GLfloat width)
{
    d_1_0_Core->LineWidth(width);
}

inline void QOpenGLFunctions_4_4_Compatibility::glHint(GLenum target, GLenum mode)
{
    d_1_0_Core->Hint(target, mode);
}

inline void QOpenGLFunctions_4_4_Compatibility::glFrontFace(GLenum mode)
{
    d_1_0_Core->FrontFace(mode);
}

inline void QOpenGLFunctions_4_4_Compatibility::glCullFace(GLenum mode)
{
    d_1_0_Core->CullFace(mode);
}


// OpenGL 1.1 core functions
inline GLboolean QOpenGLFunctions_4_4_Compatibility::glIsTexture(GLuint texture)
{
    return d_1_1_Core->IsTexture(texture);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGenTextures(GLsizei n, GLuint *textures)
{
    d_1_1_Core->GenTextures(n, textures);
}

inline void QOpenGLFunctions_4_4_Compatibility::glDeleteTextures(GLsizei n, const GLuint *textures)
{
    d_1_1_Core->DeleteTextures(n, textures);
}

inline void QOpenGLFunctions_4_4_Compatibility::glBindTexture(GLenum target, GLuint texture)
{
    d_1_1_Core->BindTexture(target, texture);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels)
{
    d_1_1_Core->TexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void *pixels)
{
    d_1_1_Core->TexSubImage1D(target, level, xoffset, width, format, type, pixels);
}

inline void QOpenGLFunctions_4_4_Compatibility::glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    d_1_1_Core->CopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
}

inline void QOpenGLFunctions_4_4_Compatibility::glCopyTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width)
{
    d_1_1_Core->CopyTexSubImage1D(target, level, xoffset, x, y, width);
}

inline void QOpenGLFunctions_4_4_Compatibility::glCopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
    d_1_1_Core->CopyTexImage2D(target, level, internalformat, x, y, width, height, border);
}

inline void QOpenGLFunctions_4_4_Compatibility::glCopyTexImage1D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border)
{
    d_1_1_Core->CopyTexImage1D(target, level, internalformat, x, y, width, border);
}

inline void QOpenGLFunctions_4_4_Compatibility::glPolygonOffset(GLfloat factor, GLfloat units)
{
    d_1_1_Core->PolygonOffset(factor, units);
}

inline void QOpenGLFunctions_4_4_Compatibility::glDrawElements(GLenum mode, GLsizei count, GLenum type, const void *indices)
{
    d_1_1_Core->DrawElements(mode, count, type, indices);
}

inline void QOpenGLFunctions_4_4_Compatibility::glDrawArrays(GLenum mode, GLint first, GLsizei count)
{
    d_1_1_Core->DrawArrays(mode, first, count);
}


// OpenGL 1.2 core functions
inline void QOpenGLFunctions_4_4_Compatibility::glBlendColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    d_1_2_Core->BlendColor(red, green, blue, alpha);
}

inline void QOpenGLFunctions_4_4_Compatibility::glBlendEquation(GLenum mode)
{
    d_1_2_Core->BlendEquation(mode);
}

inline void QOpenGLFunctions_4_4_Compatibility::glCopyTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    d_1_2_Core->CopyTexSubImage3D(target, level, xoffset, yoffset, zoffset, x, y, width, height);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels)
{
    d_1_2_Core->TexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexImage3D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void *pixels)
{
    d_1_2_Core->TexImage3D(target, level, internalformat, width, height, depth, border, format, type, pixels);
}

inline void QOpenGLFunctions_4_4_Compatibility::glDrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices)
{
    d_1_2_Core->DrawRangeElements(mode, start, end, count, type, indices);
}


// OpenGL 1.3 core functions
inline void QOpenGLFunctions_4_4_Compatibility::glGetCompressedTexImage(GLenum target, GLint level, void *img)
{
    d_1_3_Core->GetCompressedTexImage(target, level, img);
}

inline void QOpenGLFunctions_4_4_Compatibility::glCompressedTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void *data)
{
    d_1_3_Core->CompressedTexSubImage1D(target, level, xoffset, width, format, imageSize, data);
}

inline void QOpenGLFunctions_4_4_Compatibility::glCompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data)
{
    d_1_3_Core->CompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, imageSize, data);
}

inline void QOpenGLFunctions_4_4_Compatibility::glCompressedTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data)
{
    d_1_3_Core->CompressedTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data);
}

inline void QOpenGLFunctions_4_4_Compatibility::glCompressedTexImage1D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const void *data)
{
    d_1_3_Core->CompressedTexImage1D(target, level, internalformat, width, border, imageSize, data);
}

inline void QOpenGLFunctions_4_4_Compatibility::glCompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data)
{
    d_1_3_Core->CompressedTexImage2D(target, level, internalformat, width, height, border, imageSize, data);
}

inline void QOpenGLFunctions_4_4_Compatibility::glCompressedTexImage3D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void *data)
{
    d_1_3_Core->CompressedTexImage3D(target, level, internalformat, width, height, depth, border, imageSize, data);
}

inline void QOpenGLFunctions_4_4_Compatibility::glSampleCoverage(GLfloat value, GLboolean invert)
{
    d_1_3_Core->SampleCoverage(value, invert);
}

inline void QOpenGLFunctions_4_4_Compatibility::glActiveTexture(GLenum texture)
{
    d_1_3_Core->ActiveTexture(texture);
}


// OpenGL 1.4 core functions
inline void QOpenGLFunctions_4_4_Compatibility::glPointParameteriv(GLenum pname, const GLint *params)
{
    d_1_4_Core->PointParameteriv(pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glPointParameteri(GLenum pname, GLint param)
{
    d_1_4_Core->PointParameteri(pname, param);
}

inline void QOpenGLFunctions_4_4_Compatibility::glPointParameterfv(GLenum pname, const GLfloat *params)
{
    d_1_4_Core->PointParameterfv(pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glPointParameterf(GLenum pname, GLfloat param)
{
    d_1_4_Core->PointParameterf(pname, param);
}

inline void QOpenGLFunctions_4_4_Compatibility::glMultiDrawElements(GLenum mode, const GLsizei *count, GLenum type, const void *const *indices, GLsizei drawcount)
{
    d_1_4_Core->MultiDrawElements(mode, count, type, indices, drawcount);
}

inline void QOpenGLFunctions_4_4_Compatibility::glMultiDrawArrays(GLenum mode, const GLint *first, const GLsizei *count, GLsizei drawcount)
{
    d_1_4_Core->MultiDrawArrays(mode, first, count, drawcount);
}

inline void QOpenGLFunctions_4_4_Compatibility::glBlendFuncSeparate(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha)
{
    d_1_4_Core->BlendFuncSeparate(sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha);
}


// OpenGL 1.5 core functions
inline void QOpenGLFunctions_4_4_Compatibility::glGetBufferPointerv(GLenum target, GLenum pname, void * *params)
{
    d_1_5_Core->GetBufferPointerv(target, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetBufferParameteriv(GLenum target, GLenum pname, GLint *params)
{
    d_1_5_Core->GetBufferParameteriv(target, pname, params);
}

inline GLboolean QOpenGLFunctions_4_4_Compatibility::glUnmapBuffer(GLenum target)
{
    return d_1_5_Core->UnmapBuffer(target);
}

inline void * QOpenGLFunctions_4_4_Compatibility::glMapBuffer(GLenum target, GLenum access)
{
    return d_1_5_Core->MapBuffer(target, access);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, void *data)
{
    d_1_5_Core->GetBufferSubData(target, offset, size, data);
}

inline void QOpenGLFunctions_4_4_Compatibility::glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void *data)
{
    d_1_5_Core->BufferSubData(target, offset, size, data);
}

inline void QOpenGLFunctions_4_4_Compatibility::glBufferData(GLenum target, GLsizeiptr size, const void *data, GLenum usage)
{
    d_1_5_Core->BufferData(target, size, data, usage);
}

inline GLboolean QOpenGLFunctions_4_4_Compatibility::glIsBuffer(GLuint buffer)
{
    return d_1_5_Core->IsBuffer(buffer);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGenBuffers(GLsizei n, GLuint *buffers)
{
    d_1_5_Core->GenBuffers(n, buffers);
}

inline void QOpenGLFunctions_4_4_Compatibility::glDeleteBuffers(GLsizei n, const GLuint *buffers)
{
    d_1_5_Core->DeleteBuffers(n, buffers);
}

inline void QOpenGLFunctions_4_4_Compatibility::glBindBuffer(GLenum target, GLuint buffer)
{
    d_1_5_Core->BindBuffer(target, buffer);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetQueryObjectuiv(GLuint id, GLenum pname, GLuint *params)
{
    d_1_5_Core->GetQueryObjectuiv(id, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetQueryObjectiv(GLuint id, GLenum pname, GLint *params)
{
    d_1_5_Core->GetQueryObjectiv(id, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetQueryiv(GLenum target, GLenum pname, GLint *params)
{
    d_1_5_Core->GetQueryiv(target, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glEndQuery(GLenum target)
{
    d_1_5_Core->EndQuery(target);
}

inline void QOpenGLFunctions_4_4_Compatibility::glBeginQuery(GLenum target, GLuint id)
{
    d_1_5_Core->BeginQuery(target, id);
}

inline GLboolean QOpenGLFunctions_4_4_Compatibility::glIsQuery(GLuint id)
{
    return d_1_5_Core->IsQuery(id);
}

inline void QOpenGLFunctions_4_4_Compatibility::glDeleteQueries(GLsizei n, const GLuint *ids)
{
    d_1_5_Core->DeleteQueries(n, ids);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGenQueries(GLsizei n, GLuint *ids)
{
    d_1_5_Core->GenQueries(n, ids);
}


// OpenGL 2.0 core functions
inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer)
{
    d_2_0_Core->VertexAttribPointer(index, size, type, normalized, stride, pointer);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttrib4usv(GLuint index, const GLushort *v)
{
    d_2_0_Core->VertexAttrib4usv(index, v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttrib4uiv(GLuint index, const GLuint *v)
{
    d_2_0_Core->VertexAttrib4uiv(index, v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttrib4ubv(GLuint index, const GLubyte *v)
{
    d_2_0_Core->VertexAttrib4ubv(index, v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttrib4sv(GLuint index, const GLshort *v)
{
    d_2_0_Core->VertexAttrib4sv(index, v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttrib4s(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w)
{
    d_2_0_Core->VertexAttrib4s(index, x, y, z, w);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttrib4iv(GLuint index, const GLint *v)
{
    d_2_0_Core->VertexAttrib4iv(index, v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttrib4fv(GLuint index, const GLfloat *v)
{
    d_2_0_Core->VertexAttrib4fv(index, v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttrib4f(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    d_2_0_Core->VertexAttrib4f(index, x, y, z, w);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttrib4dv(GLuint index, const GLdouble *v)
{
    d_2_0_Core->VertexAttrib4dv(index, v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttrib4d(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    d_2_0_Core->VertexAttrib4d(index, x, y, z, w);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttrib4bv(GLuint index, const GLbyte *v)
{
    d_2_0_Core->VertexAttrib4bv(index, v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttrib4Nusv(GLuint index, const GLushort *v)
{
    d_2_0_Core->VertexAttrib4Nusv(index, v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttrib4Nuiv(GLuint index, const GLuint *v)
{
    d_2_0_Core->VertexAttrib4Nuiv(index, v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttrib4Nubv(GLuint index, const GLubyte *v)
{
    d_2_0_Core->VertexAttrib4Nubv(index, v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttrib4Nub(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w)
{
    d_2_0_Core->VertexAttrib4Nub(index, x, y, z, w);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttrib4Nsv(GLuint index, const GLshort *v)
{
    d_2_0_Core->VertexAttrib4Nsv(index, v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttrib4Niv(GLuint index, const GLint *v)
{
    d_2_0_Core->VertexAttrib4Niv(index, v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttrib4Nbv(GLuint index, const GLbyte *v)
{
    d_2_0_Core->VertexAttrib4Nbv(index, v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttrib3sv(GLuint index, const GLshort *v)
{
    d_2_0_Core->VertexAttrib3sv(index, v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttrib3s(GLuint index, GLshort x, GLshort y, GLshort z)
{
    d_2_0_Core->VertexAttrib3s(index, x, y, z);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttrib3fv(GLuint index, const GLfloat *v)
{
    d_2_0_Core->VertexAttrib3fv(index, v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttrib3f(GLuint index, GLfloat x, GLfloat y, GLfloat z)
{
    d_2_0_Core->VertexAttrib3f(index, x, y, z);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttrib3dv(GLuint index, const GLdouble *v)
{
    d_2_0_Core->VertexAttrib3dv(index, v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttrib3d(GLuint index, GLdouble x, GLdouble y, GLdouble z)
{
    d_2_0_Core->VertexAttrib3d(index, x, y, z);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttrib2sv(GLuint index, const GLshort *v)
{
    d_2_0_Core->VertexAttrib2sv(index, v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttrib2s(GLuint index, GLshort x, GLshort y)
{
    d_2_0_Core->VertexAttrib2s(index, x, y);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttrib2fv(GLuint index, const GLfloat *v)
{
    d_2_0_Core->VertexAttrib2fv(index, v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttrib2f(GLuint index, GLfloat x, GLfloat y)
{
    d_2_0_Core->VertexAttrib2f(index, x, y);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttrib2dv(GLuint index, const GLdouble *v)
{
    d_2_0_Core->VertexAttrib2dv(index, v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttrib2d(GLuint index, GLdouble x, GLdouble y)
{
    d_2_0_Core->VertexAttrib2d(index, x, y);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttrib1sv(GLuint index, const GLshort *v)
{
    d_2_0_Core->VertexAttrib1sv(index, v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttrib1s(GLuint index, GLshort x)
{
    d_2_0_Core->VertexAttrib1s(index, x);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttrib1fv(GLuint index, const GLfloat *v)
{
    d_2_0_Core->VertexAttrib1fv(index, v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttrib1f(GLuint index, GLfloat x)
{
    d_2_0_Core->VertexAttrib1f(index, x);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttrib1dv(GLuint index, const GLdouble *v)
{
    d_2_0_Core->VertexAttrib1dv(index, v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttrib1d(GLuint index, GLdouble x)
{
    d_2_0_Core->VertexAttrib1d(index, x);
}

inline void QOpenGLFunctions_4_4_Compatibility::glValidateProgram(GLuint program)
{
    d_2_0_Core->ValidateProgram(program);
}

inline void QOpenGLFunctions_4_4_Compatibility::glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    d_2_0_Core->UniformMatrix4fv(location, count, transpose, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    d_2_0_Core->UniformMatrix3fv(location, count, transpose, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    d_2_0_Core->UniformMatrix2fv(location, count, transpose, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glUniform4iv(GLint location, GLsizei count, const GLint *value)
{
    d_2_0_Core->Uniform4iv(location, count, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glUniform3iv(GLint location, GLsizei count, const GLint *value)
{
    d_2_0_Core->Uniform3iv(location, count, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glUniform2iv(GLint location, GLsizei count, const GLint *value)
{
    d_2_0_Core->Uniform2iv(location, count, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glUniform1iv(GLint location, GLsizei count, const GLint *value)
{
    d_2_0_Core->Uniform1iv(location, count, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glUniform4fv(GLint location, GLsizei count, const GLfloat *value)
{
    d_2_0_Core->Uniform4fv(location, count, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glUniform3fv(GLint location, GLsizei count, const GLfloat *value)
{
    d_2_0_Core->Uniform3fv(location, count, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glUniform2fv(GLint location, GLsizei count, const GLfloat *value)
{
    d_2_0_Core->Uniform2fv(location, count, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glUniform1fv(GLint location, GLsizei count, const GLfloat *value)
{
    d_2_0_Core->Uniform1fv(location, count, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glUniform4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3)
{
    d_2_0_Core->Uniform4i(location, v0, v1, v2, v3);
}

inline void QOpenGLFunctions_4_4_Compatibility::glUniform3i(GLint location, GLint v0, GLint v1, GLint v2)
{
    d_2_0_Core->Uniform3i(location, v0, v1, v2);
}

inline void QOpenGLFunctions_4_4_Compatibility::glUniform2i(GLint location, GLint v0, GLint v1)
{
    d_2_0_Core->Uniform2i(location, v0, v1);
}

inline void QOpenGLFunctions_4_4_Compatibility::glUniform1i(GLint location, GLint v0)
{
    d_2_0_Core->Uniform1i(location, v0);
}

inline void QOpenGLFunctions_4_4_Compatibility::glUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
    d_2_0_Core->Uniform4f(location, v0, v1, v2, v3);
}

inline void QOpenGLFunctions_4_4_Compatibility::glUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
{
    d_2_0_Core->Uniform3f(location, v0, v1, v2);
}

inline void QOpenGLFunctions_4_4_Compatibility::glUniform2f(GLint location, GLfloat v0, GLfloat v1)
{
    d_2_0_Core->Uniform2f(location, v0, v1);
}

inline void QOpenGLFunctions_4_4_Compatibility::glUniform1f(GLint location, GLfloat v0)
{
    d_2_0_Core->Uniform1f(location, v0);
}

inline void QOpenGLFunctions_4_4_Compatibility::glUseProgram(GLuint program)
{
    d_2_0_Core->UseProgram(program);
}

inline void QOpenGLFunctions_4_4_Compatibility::glShaderSource(GLuint shader, GLsizei count, const GLchar *const *string, const GLint *length)
{
    d_2_0_Core->ShaderSource(shader, count, string, length);
}

inline void QOpenGLFunctions_4_4_Compatibility::glLinkProgram(GLuint program)
{
    d_2_0_Core->LinkProgram(program);
}

inline GLboolean QOpenGLFunctions_4_4_Compatibility::glIsShader(GLuint shader)
{
    return d_2_0_Core->IsShader(shader);
}

inline GLboolean QOpenGLFunctions_4_4_Compatibility::glIsProgram(GLuint program)
{
    return d_2_0_Core->IsProgram(program);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetVertexAttribPointerv(GLuint index, GLenum pname, void * *pointer)
{
    d_2_0_Core->GetVertexAttribPointerv(index, pname, pointer);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetVertexAttribiv(GLuint index, GLenum pname, GLint *params)
{
    d_2_0_Core->GetVertexAttribiv(index, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetVertexAttribfv(GLuint index, GLenum pname, GLfloat *params)
{
    d_2_0_Core->GetVertexAttribfv(index, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetVertexAttribdv(GLuint index, GLenum pname, GLdouble *params)
{
    d_2_0_Core->GetVertexAttribdv(index, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetUniformiv(GLuint program, GLint location, GLint *params)
{
    d_2_0_Core->GetUniformiv(program, location, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetUniformfv(GLuint program, GLint location, GLfloat *params)
{
    d_2_0_Core->GetUniformfv(program, location, params);
}

inline GLint QOpenGLFunctions_4_4_Compatibility::glGetUniformLocation(GLuint program, const GLchar *name)
{
    return d_2_0_Core->GetUniformLocation(program, name);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetShaderSource(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source)
{
    d_2_0_Core->GetShaderSource(shader, bufSize, length, source);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog)
{
    d_2_0_Core->GetShaderInfoLog(shader, bufSize, length, infoLog);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetShaderiv(GLuint shader, GLenum pname, GLint *params)
{
    d_2_0_Core->GetShaderiv(shader, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog)
{
    d_2_0_Core->GetProgramInfoLog(program, bufSize, length, infoLog);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetProgramiv(GLuint program, GLenum pname, GLint *params)
{
    d_2_0_Core->GetProgramiv(program, pname, params);
}

inline GLint QOpenGLFunctions_4_4_Compatibility::glGetAttribLocation(GLuint program, const GLchar *name)
{
    return d_2_0_Core->GetAttribLocation(program, name);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetAttachedShaders(GLuint program, GLsizei maxCount, GLsizei *count, GLuint *shaders)
{
    d_2_0_Core->GetAttachedShaders(program, maxCount, count, shaders);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetActiveUniform(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name)
{
    d_2_0_Core->GetActiveUniform(program, index, bufSize, length, size, type, name);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetActiveAttrib(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name)
{
    d_2_0_Core->GetActiveAttrib(program, index, bufSize, length, size, type, name);
}

inline void QOpenGLFunctions_4_4_Compatibility::glEnableVertexAttribArray(GLuint index)
{
    d_2_0_Core->EnableVertexAttribArray(index);
}

inline void QOpenGLFunctions_4_4_Compatibility::glDisableVertexAttribArray(GLuint index)
{
    d_2_0_Core->DisableVertexAttribArray(index);
}

inline void QOpenGLFunctions_4_4_Compatibility::glDetachShader(GLuint program, GLuint shader)
{
    d_2_0_Core->DetachShader(program, shader);
}

inline void QOpenGLFunctions_4_4_Compatibility::glDeleteShader(GLuint shader)
{
    d_2_0_Core->DeleteShader(shader);
}

inline void QOpenGLFunctions_4_4_Compatibility::glDeleteProgram(GLuint program)
{
    d_2_0_Core->DeleteProgram(program);
}

inline GLuint QOpenGLFunctions_4_4_Compatibility::glCreateShader(GLenum type)
{
    return d_2_0_Core->CreateShader(type);
}

inline GLuint QOpenGLFunctions_4_4_Compatibility::glCreateProgram()
{
    return d_2_0_Core->CreateProgram();
}

inline void QOpenGLFunctions_4_4_Compatibility::glCompileShader(GLuint shader)
{
    d_2_0_Core->CompileShader(shader);
}

inline void QOpenGLFunctions_4_4_Compatibility::glBindAttribLocation(GLuint program, GLuint index, const GLchar *name)
{
    d_2_0_Core->BindAttribLocation(program, index, name);
}

inline void QOpenGLFunctions_4_4_Compatibility::glAttachShader(GLuint program, GLuint shader)
{
    d_2_0_Core->AttachShader(program, shader);
}

inline void QOpenGLFunctions_4_4_Compatibility::glStencilMaskSeparate(GLenum face, GLuint mask)
{
    d_2_0_Core->StencilMaskSeparate(face, mask);
}

inline void QOpenGLFunctions_4_4_Compatibility::glStencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask)
{
    d_2_0_Core->StencilFuncSeparate(face, func, ref, mask);
}

inline void QOpenGLFunctions_4_4_Compatibility::glStencilOpSeparate(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass)
{
    d_2_0_Core->StencilOpSeparate(face, sfail, dpfail, dppass);
}

inline void QOpenGLFunctions_4_4_Compatibility::glDrawBuffers(GLsizei n, const GLenum *bufs)
{
    d_2_0_Core->DrawBuffers(n, bufs);
}

inline void QOpenGLFunctions_4_4_Compatibility::glBlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha)
{
    d_2_0_Core->BlendEquationSeparate(modeRGB, modeAlpha);
}


// OpenGL 2.1 core functions
inline void QOpenGLFunctions_4_4_Compatibility::glUniformMatrix4x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    d_2_1_Core->UniformMatrix4x3fv(location, count, transpose, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glUniformMatrix3x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    d_2_1_Core->UniformMatrix3x4fv(location, count, transpose, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glUniformMatrix4x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    d_2_1_Core->UniformMatrix4x2fv(location, count, transpose, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glUniformMatrix2x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    d_2_1_Core->UniformMatrix2x4fv(location, count, transpose, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glUniformMatrix3x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    d_2_1_Core->UniformMatrix3x2fv(location, count, transpose, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glUniformMatrix2x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    d_2_1_Core->UniformMatrix2x3fv(location, count, transpose, value);
}


// OpenGL 3.0 core functions
inline GLboolean QOpenGLFunctions_4_4_Compatibility::glIsVertexArray(GLuint array)
{
    return d_3_0_Core->IsVertexArray(array);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGenVertexArrays(GLsizei n, GLuint *arrays)
{
    d_3_0_Core->GenVertexArrays(n, arrays);
}

inline void QOpenGLFunctions_4_4_Compatibility::glDeleteVertexArrays(GLsizei n, const GLuint *arrays)
{
    d_3_0_Core->DeleteVertexArrays(n, arrays);
}

inline void QOpenGLFunctions_4_4_Compatibility::glBindVertexArray(GLuint array)
{
    d_3_0_Core->BindVertexArray(array);
}

inline void QOpenGLFunctions_4_4_Compatibility::glFlushMappedBufferRange(GLenum target, GLintptr offset, GLsizeiptr length)
{
    d_3_0_Core->FlushMappedBufferRange(target, offset, length);
}

inline void * QOpenGLFunctions_4_4_Compatibility::glMapBufferRange(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access)
{
    return d_3_0_Core->MapBufferRange(target, offset, length, access);
}

inline void QOpenGLFunctions_4_4_Compatibility::glFramebufferTextureLayer(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer)
{
    d_3_0_Core->FramebufferTextureLayer(target, attachment, texture, level, layer);
}

inline void QOpenGLFunctions_4_4_Compatibility::glRenderbufferStorageMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
{
    d_3_0_Core->RenderbufferStorageMultisample(target, samples, internalformat, width, height);
}

inline void QOpenGLFunctions_4_4_Compatibility::glBlitFramebuffer(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)
{
    d_3_0_Core->BlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGenerateMipmap(GLenum target)
{
    d_3_0_Core->GenerateMipmap(target);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetFramebufferAttachmentParameteriv(GLenum target, GLenum attachment, GLenum pname, GLint *params)
{
    d_3_0_Core->GetFramebufferAttachmentParameteriv(target, attachment, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
    d_3_0_Core->FramebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer);
}

inline void QOpenGLFunctions_4_4_Compatibility::glFramebufferTexture3D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset)
{
    d_3_0_Core->FramebufferTexture3D(target, attachment, textarget, texture, level, zoffset);
}

inline void QOpenGLFunctions_4_4_Compatibility::glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
    d_3_0_Core->FramebufferTexture2D(target, attachment, textarget, texture, level);
}

inline void QOpenGLFunctions_4_4_Compatibility::glFramebufferTexture1D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
    d_3_0_Core->FramebufferTexture1D(target, attachment, textarget, texture, level);
}

inline GLenum QOpenGLFunctions_4_4_Compatibility::glCheckFramebufferStatus(GLenum target)
{
    return d_3_0_Core->CheckFramebufferStatus(target);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGenFramebuffers(GLsizei n, GLuint *framebuffers)
{
    d_3_0_Core->GenFramebuffers(n, framebuffers);
}

inline void QOpenGLFunctions_4_4_Compatibility::glDeleteFramebuffers(GLsizei n, const GLuint *framebuffers)
{
    d_3_0_Core->DeleteFramebuffers(n, framebuffers);
}

inline void QOpenGLFunctions_4_4_Compatibility::glBindFramebuffer(GLenum target, GLuint framebuffer)
{
    d_3_0_Core->BindFramebuffer(target, framebuffer);
}

inline GLboolean QOpenGLFunctions_4_4_Compatibility::glIsFramebuffer(GLuint framebuffer)
{
    return d_3_0_Core->IsFramebuffer(framebuffer);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetRenderbufferParameteriv(GLenum target, GLenum pname, GLint *params)
{
    d_3_0_Core->GetRenderbufferParameteriv(target, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
{
    d_3_0_Core->RenderbufferStorage(target, internalformat, width, height);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGenRenderbuffers(GLsizei n, GLuint *renderbuffers)
{
    d_3_0_Core->GenRenderbuffers(n, renderbuffers);
}

inline void QOpenGLFunctions_4_4_Compatibility::glDeleteRenderbuffers(GLsizei n, const GLuint *renderbuffers)
{
    d_3_0_Core->DeleteRenderbuffers(n, renderbuffers);
}

inline void QOpenGLFunctions_4_4_Compatibility::glBindRenderbuffer(GLenum target, GLuint renderbuffer)
{
    d_3_0_Core->BindRenderbuffer(target, renderbuffer);
}

inline GLboolean QOpenGLFunctions_4_4_Compatibility::glIsRenderbuffer(GLuint renderbuffer)
{
    return d_3_0_Core->IsRenderbuffer(renderbuffer);
}

inline const GLubyte * QOpenGLFunctions_4_4_Compatibility::glGetStringi(GLenum name, GLuint index)
{
    return d_3_0_Core->GetStringi(name, index);
}

inline void QOpenGLFunctions_4_4_Compatibility::glClearBufferfi(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil)
{
    d_3_0_Core->ClearBufferfi(buffer, drawbuffer, depth, stencil);
}

inline void QOpenGLFunctions_4_4_Compatibility::glClearBufferfv(GLenum buffer, GLint drawbuffer, const GLfloat *value)
{
    d_3_0_Core->ClearBufferfv(buffer, drawbuffer, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glClearBufferuiv(GLenum buffer, GLint drawbuffer, const GLuint *value)
{
    d_3_0_Core->ClearBufferuiv(buffer, drawbuffer, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glClearBufferiv(GLenum buffer, GLint drawbuffer, const GLint *value)
{
    d_3_0_Core->ClearBufferiv(buffer, drawbuffer, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetTexParameterIuiv(GLenum target, GLenum pname, GLuint *params)
{
    d_3_0_Core->GetTexParameterIuiv(target, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetTexParameterIiv(GLenum target, GLenum pname, GLint *params)
{
    d_3_0_Core->GetTexParameterIiv(target, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexParameterIuiv(GLenum target, GLenum pname, const GLuint *params)
{
    d_3_0_Core->TexParameterIuiv(target, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexParameterIiv(GLenum target, GLenum pname, const GLint *params)
{
    d_3_0_Core->TexParameterIiv(target, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glUniform4uiv(GLint location, GLsizei count, const GLuint *value)
{
    d_3_0_Core->Uniform4uiv(location, count, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glUniform3uiv(GLint location, GLsizei count, const GLuint *value)
{
    d_3_0_Core->Uniform3uiv(location, count, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glUniform2uiv(GLint location, GLsizei count, const GLuint *value)
{
    d_3_0_Core->Uniform2uiv(location, count, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glUniform1uiv(GLint location, GLsizei count, const GLuint *value)
{
    d_3_0_Core->Uniform1uiv(location, count, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glUniform4ui(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
    d_3_0_Core->Uniform4ui(location, v0, v1, v2, v3);
}

inline void QOpenGLFunctions_4_4_Compatibility::glUniform3ui(GLint location, GLuint v0, GLuint v1, GLuint v2)
{
    d_3_0_Core->Uniform3ui(location, v0, v1, v2);
}

inline void QOpenGLFunctions_4_4_Compatibility::glUniform2ui(GLint location, GLuint v0, GLuint v1)
{
    d_3_0_Core->Uniform2ui(location, v0, v1);
}

inline void QOpenGLFunctions_4_4_Compatibility::glUniform1ui(GLint location, GLuint v0)
{
    d_3_0_Core->Uniform1ui(location, v0);
}

inline GLint QOpenGLFunctions_4_4_Compatibility::glGetFragDataLocation(GLuint program, const GLchar *name)
{
    return d_3_0_Core->GetFragDataLocation(program, name);
}

inline void QOpenGLFunctions_4_4_Compatibility::glBindFragDataLocation(GLuint program, GLuint color, const GLchar *name)
{
    d_3_0_Core->BindFragDataLocation(program, color, name);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetUniformuiv(GLuint program, GLint location, GLuint *params)
{
    d_3_0_Core->GetUniformuiv(program, location, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttribI4usv(GLuint index, const GLushort *v)
{
    d_3_0_Core->VertexAttribI4usv(index, v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttribI4ubv(GLuint index, const GLubyte *v)
{
    d_3_0_Core->VertexAttribI4ubv(index, v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttribI4sv(GLuint index, const GLshort *v)
{
    d_3_0_Core->VertexAttribI4sv(index, v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttribI4bv(GLuint index, const GLbyte *v)
{
    d_3_0_Core->VertexAttribI4bv(index, v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttribI4uiv(GLuint index, const GLuint *v)
{
    d_3_0_Core->VertexAttribI4uiv(index, v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttribI3uiv(GLuint index, const GLuint *v)
{
    d_3_0_Core->VertexAttribI3uiv(index, v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttribI2uiv(GLuint index, const GLuint *v)
{
    d_3_0_Core->VertexAttribI2uiv(index, v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttribI1uiv(GLuint index, const GLuint *v)
{
    d_3_0_Core->VertexAttribI1uiv(index, v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttribI4iv(GLuint index, const GLint *v)
{
    d_3_0_Core->VertexAttribI4iv(index, v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttribI3iv(GLuint index, const GLint *v)
{
    d_3_0_Core->VertexAttribI3iv(index, v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttribI2iv(GLuint index, const GLint *v)
{
    d_3_0_Core->VertexAttribI2iv(index, v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttribI1iv(GLuint index, const GLint *v)
{
    d_3_0_Core->VertexAttribI1iv(index, v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttribI4ui(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w)
{
    d_3_0_Core->VertexAttribI4ui(index, x, y, z, w);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttribI3ui(GLuint index, GLuint x, GLuint y, GLuint z)
{
    d_3_0_Core->VertexAttribI3ui(index, x, y, z);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttribI2ui(GLuint index, GLuint x, GLuint y)
{
    d_3_0_Core->VertexAttribI2ui(index, x, y);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttribI1ui(GLuint index, GLuint x)
{
    d_3_0_Core->VertexAttribI1ui(index, x);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttribI4i(GLuint index, GLint x, GLint y, GLint z, GLint w)
{
    d_3_0_Core->VertexAttribI4i(index, x, y, z, w);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttribI3i(GLuint index, GLint x, GLint y, GLint z)
{
    d_3_0_Core->VertexAttribI3i(index, x, y, z);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttribI2i(GLuint index, GLint x, GLint y)
{
    d_3_0_Core->VertexAttribI2i(index, x, y);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttribI1i(GLuint index, GLint x)
{
    d_3_0_Core->VertexAttribI1i(index, x);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetVertexAttribIuiv(GLuint index, GLenum pname, GLuint *params)
{
    d_3_0_Core->GetVertexAttribIuiv(index, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetVertexAttribIiv(GLuint index, GLenum pname, GLint *params)
{
    d_3_0_Core->GetVertexAttribIiv(index, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer)
{
    d_3_0_Core->VertexAttribIPointer(index, size, type, stride, pointer);
}

inline void QOpenGLFunctions_4_4_Compatibility::glEndConditionalRender()
{
    d_3_0_Core->EndConditionalRender();
}

inline void QOpenGLFunctions_4_4_Compatibility::glBeginConditionalRender(GLuint id, GLenum mode)
{
    d_3_0_Core->BeginConditionalRender(id, mode);
}

inline void QOpenGLFunctions_4_4_Compatibility::glClampColor(GLenum target, GLenum clamp)
{
    d_3_0_Core->ClampColor(target, clamp);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetTransformFeedbackVarying(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLsizei *size, GLenum *type, GLchar *name)
{
    d_3_0_Core->GetTransformFeedbackVarying(program, index, bufSize, length, size, type, name);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTransformFeedbackVaryings(GLuint program, GLsizei count, const GLchar *const *varyings, GLenum bufferMode)
{
    d_3_0_Core->TransformFeedbackVaryings(program, count, varyings, bufferMode);
}

inline void QOpenGLFunctions_4_4_Compatibility::glBindBufferBase(GLenum target, GLuint index, GLuint buffer)
{
    d_3_0_Core->BindBufferBase(target, index, buffer);
}

inline void QOpenGLFunctions_4_4_Compatibility::glBindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)
{
    d_3_0_Core->BindBufferRange(target, index, buffer, offset, size);
}

inline void QOpenGLFunctions_4_4_Compatibility::glEndTransformFeedback()
{
    d_3_0_Core->EndTransformFeedback();
}

inline void QOpenGLFunctions_4_4_Compatibility::glBeginTransformFeedback(GLenum primitiveMode)
{
    d_3_0_Core->BeginTransformFeedback(primitiveMode);
}

inline GLboolean QOpenGLFunctions_4_4_Compatibility::glIsEnabledi(GLenum target, GLuint index)
{
    return d_3_0_Core->IsEnabledi(target, index);
}

inline void QOpenGLFunctions_4_4_Compatibility::glDisablei(GLenum target, GLuint index)
{
    d_3_0_Core->Disablei(target, index);
}

inline void QOpenGLFunctions_4_4_Compatibility::glEnablei(GLenum target, GLuint index)
{
    d_3_0_Core->Enablei(target, index);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetIntegeri_v(GLenum target, GLuint index, GLint *data)
{
    d_3_0_Core->GetIntegeri_v(target, index, data);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetBooleani_v(GLenum target, GLuint index, GLboolean *data)
{
    d_3_0_Core->GetBooleani_v(target, index, data);
}

inline void QOpenGLFunctions_4_4_Compatibility::glColorMaski(GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a)
{
    d_3_0_Core->ColorMaski(index, r, g, b, a);
}


// OpenGL 3.1 core functions
inline void QOpenGLFunctions_4_4_Compatibility::glUniformBlockBinding(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding)
{
    d_3_1_Core->UniformBlockBinding(program, uniformBlockIndex, uniformBlockBinding);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetActiveUniformBlockName(GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformBlockName)
{
    d_3_1_Core->GetActiveUniformBlockName(program, uniformBlockIndex, bufSize, length, uniformBlockName);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetActiveUniformBlockiv(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint *params)
{
    d_3_1_Core->GetActiveUniformBlockiv(program, uniformBlockIndex, pname, params);
}

inline GLuint QOpenGLFunctions_4_4_Compatibility::glGetUniformBlockIndex(GLuint program, const GLchar *uniformBlockName)
{
    return d_3_1_Core->GetUniformBlockIndex(program, uniformBlockName);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetActiveUniformName(GLuint program, GLuint uniformIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformName)
{
    d_3_1_Core->GetActiveUniformName(program, uniformIndex, bufSize, length, uniformName);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetActiveUniformsiv(GLuint program, GLsizei uniformCount, const GLuint *uniformIndices, GLenum pname, GLint *params)
{
    d_3_1_Core->GetActiveUniformsiv(program, uniformCount, uniformIndices, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetUniformIndices(GLuint program, GLsizei uniformCount, const GLchar *const *uniformNames, GLuint *uniformIndices)
{
    d_3_1_Core->GetUniformIndices(program, uniformCount, uniformNames, uniformIndices);
}

inline void QOpenGLFunctions_4_4_Compatibility::glCopyBufferSubData(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size)
{
    d_3_1_Core->CopyBufferSubData(readTarget, writeTarget, readOffset, writeOffset, size);
}

inline void QOpenGLFunctions_4_4_Compatibility::glPrimitiveRestartIndex(GLuint index)
{
    d_3_1_Core->PrimitiveRestartIndex(index);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexBuffer(GLenum target, GLenum internalformat, GLuint buffer)
{
    d_3_1_Core->TexBuffer(target, internalformat, buffer);
}

inline void QOpenGLFunctions_4_4_Compatibility::glDrawElementsInstanced(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount)
{
    d_3_1_Core->DrawElementsInstanced(mode, count, type, indices, instancecount);
}

inline void QOpenGLFunctions_4_4_Compatibility::glDrawArraysInstanced(GLenum mode, GLint first, GLsizei count, GLsizei instancecount)
{
    d_3_1_Core->DrawArraysInstanced(mode, first, count, instancecount);
}


// OpenGL 3.2 core functions
inline void QOpenGLFunctions_4_4_Compatibility::glSampleMaski(GLuint maskNumber, GLbitfield mask)
{
    d_3_2_Core->SampleMaski(maskNumber, mask);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetMultisamplefv(GLenum pname, GLuint index, GLfloat *val)
{
    d_3_2_Core->GetMultisamplefv(pname, index, val);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexImage3DMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations)
{
    d_3_2_Core->TexImage3DMultisample(target, samples, internalformat, width, height, depth, fixedsamplelocations);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexImage2DMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations)
{
    d_3_2_Core->TexImage2DMultisample(target, samples, internalformat, width, height, fixedsamplelocations);
}

inline void QOpenGLFunctions_4_4_Compatibility::glFramebufferTexture(GLenum target, GLenum attachment, GLuint texture, GLint level)
{
    d_3_2_Core->FramebufferTexture(target, attachment, texture, level);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetBufferParameteri64v(GLenum target, GLenum pname, GLint64 *params)
{
    d_3_2_Core->GetBufferParameteri64v(target, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetInteger64i_v(GLenum target, GLuint index, GLint64 *data)
{
    d_3_2_Core->GetInteger64i_v(target, index, data);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetSynciv(GLsync sync, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *values)
{
    d_3_2_Core->GetSynciv(sync, pname, bufSize, length, values);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetInteger64v(GLenum pname, GLint64 *data)
{
    d_3_2_Core->GetInteger64v(pname, data);
}

inline void QOpenGLFunctions_4_4_Compatibility::glWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout)
{
    d_3_2_Core->WaitSync(sync, flags, timeout);
}

inline GLenum QOpenGLFunctions_4_4_Compatibility::glClientWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout)
{
    return d_3_2_Core->ClientWaitSync(sync, flags, timeout);
}

inline void QOpenGLFunctions_4_4_Compatibility::glDeleteSync(GLsync sync)
{
    d_3_2_Core->DeleteSync(sync);
}

inline GLboolean QOpenGLFunctions_4_4_Compatibility::glIsSync(GLsync sync)
{
    return d_3_2_Core->IsSync(sync);
}

inline GLsync QOpenGLFunctions_4_4_Compatibility::glFenceSync(GLenum condition, GLbitfield flags)
{
    return d_3_2_Core->FenceSync(condition, flags);
}

inline void QOpenGLFunctions_4_4_Compatibility::glProvokingVertex(GLenum mode)
{
    d_3_2_Core->ProvokingVertex(mode);
}

inline void QOpenGLFunctions_4_4_Compatibility::glMultiDrawElementsBaseVertex(GLenum mode, const GLsizei *count, GLenum type, const void *const *indices, GLsizei drawcount, const GLint *basevertex)
{
    d_3_2_Core->MultiDrawElementsBaseVertex(mode, count, type, indices, drawcount, basevertex);
}

inline void QOpenGLFunctions_4_4_Compatibility::glDrawElementsInstancedBaseVertex(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex)
{
    d_3_2_Core->DrawElementsInstancedBaseVertex(mode, count, type, indices, instancecount, basevertex);
}

inline void QOpenGLFunctions_4_4_Compatibility::glDrawRangeElementsBaseVertex(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices, GLint basevertex)
{
    d_3_2_Core->DrawRangeElementsBaseVertex(mode, start, end, count, type, indices, basevertex);
}

inline void QOpenGLFunctions_4_4_Compatibility::glDrawElementsBaseVertex(GLenum mode, GLsizei count, GLenum type, const void *indices, GLint basevertex)
{
    d_3_2_Core->DrawElementsBaseVertex(mode, count, type, indices, basevertex);
}


// OpenGL 3.3 core functions
inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttribP4uiv(GLuint index, GLenum type, GLboolean normalized, const GLuint *value)
{
    d_3_3_Core->VertexAttribP4uiv(index, type, normalized, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttribP4ui(GLuint index, GLenum type, GLboolean normalized, GLuint value)
{
    d_3_3_Core->VertexAttribP4ui(index, type, normalized, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttribP3uiv(GLuint index, GLenum type, GLboolean normalized, const GLuint *value)
{
    d_3_3_Core->VertexAttribP3uiv(index, type, normalized, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttribP3ui(GLuint index, GLenum type, GLboolean normalized, GLuint value)
{
    d_3_3_Core->VertexAttribP3ui(index, type, normalized, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttribP2uiv(GLuint index, GLenum type, GLboolean normalized, const GLuint *value)
{
    d_3_3_Core->VertexAttribP2uiv(index, type, normalized, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttribP2ui(GLuint index, GLenum type, GLboolean normalized, GLuint value)
{
    d_3_3_Core->VertexAttribP2ui(index, type, normalized, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttribP1uiv(GLuint index, GLenum type, GLboolean normalized, const GLuint *value)
{
    d_3_3_Core->VertexAttribP1uiv(index, type, normalized, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttribP1ui(GLuint index, GLenum type, GLboolean normalized, GLuint value)
{
    d_3_3_Core->VertexAttribP1ui(index, type, normalized, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttribDivisor(GLuint index, GLuint divisor)
{
    d_3_3_Core->VertexAttribDivisor(index, divisor);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetQueryObjectui64v(GLuint id, GLenum pname, GLuint64 *params)
{
    d_3_3_Core->GetQueryObjectui64v(id, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetQueryObjecti64v(GLuint id, GLenum pname, GLint64 *params)
{
    d_3_3_Core->GetQueryObjecti64v(id, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glQueryCounter(GLuint id, GLenum target)
{
    d_3_3_Core->QueryCounter(id, target);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetSamplerParameterIuiv(GLuint sampler, GLenum pname, GLuint *params)
{
    d_3_3_Core->GetSamplerParameterIuiv(sampler, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetSamplerParameterfv(GLuint sampler, GLenum pname, GLfloat *params)
{
    d_3_3_Core->GetSamplerParameterfv(sampler, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetSamplerParameterIiv(GLuint sampler, GLenum pname, GLint *params)
{
    d_3_3_Core->GetSamplerParameterIiv(sampler, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetSamplerParameteriv(GLuint sampler, GLenum pname, GLint *params)
{
    d_3_3_Core->GetSamplerParameteriv(sampler, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glSamplerParameterIuiv(GLuint sampler, GLenum pname, const GLuint *param)
{
    d_3_3_Core->SamplerParameterIuiv(sampler, pname, param);
}

inline void QOpenGLFunctions_4_4_Compatibility::glSamplerParameterIiv(GLuint sampler, GLenum pname, const GLint *param)
{
    d_3_3_Core->SamplerParameterIiv(sampler, pname, param);
}

inline void QOpenGLFunctions_4_4_Compatibility::glSamplerParameterfv(GLuint sampler, GLenum pname, const GLfloat *param)
{
    d_3_3_Core->SamplerParameterfv(sampler, pname, param);
}

inline void QOpenGLFunctions_4_4_Compatibility::glSamplerParameterf(GLuint sampler, GLenum pname, GLfloat param)
{
    d_3_3_Core->SamplerParameterf(sampler, pname, param);
}

inline void QOpenGLFunctions_4_4_Compatibility::glSamplerParameteriv(GLuint sampler, GLenum pname, const GLint *param)
{
    d_3_3_Core->SamplerParameteriv(sampler, pname, param);
}

inline void QOpenGLFunctions_4_4_Compatibility::glSamplerParameteri(GLuint sampler, GLenum pname, GLint param)
{
    d_3_3_Core->SamplerParameteri(sampler, pname, param);
}

inline void QOpenGLFunctions_4_4_Compatibility::glBindSampler(GLuint unit, GLuint sampler)
{
    d_3_3_Core->BindSampler(unit, sampler);
}

inline GLboolean QOpenGLFunctions_4_4_Compatibility::glIsSampler(GLuint sampler)
{
    return d_3_3_Core->IsSampler(sampler);
}

inline void QOpenGLFunctions_4_4_Compatibility::glDeleteSamplers(GLsizei count, const GLuint *samplers)
{
    d_3_3_Core->DeleteSamplers(count, samplers);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGenSamplers(GLsizei count, GLuint *samplers)
{
    d_3_3_Core->GenSamplers(count, samplers);
}

inline GLint QOpenGLFunctions_4_4_Compatibility::glGetFragDataIndex(GLuint program, const GLchar *name)
{
    return d_3_3_Core->GetFragDataIndex(program, name);
}

inline void QOpenGLFunctions_4_4_Compatibility::glBindFragDataLocationIndexed(GLuint program, GLuint colorNumber, GLuint index, const GLchar *name)
{
    d_3_3_Core->BindFragDataLocationIndexed(program, colorNumber, index, name);
}


// OpenGL 4.0 core functions
inline void QOpenGLFunctions_4_4_Compatibility::glGetQueryIndexediv(GLenum target, GLuint index, GLenum pname, GLint *params)
{
    d_4_0_Core->GetQueryIndexediv(target, index, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glEndQueryIndexed(GLenum target, GLuint index)
{
    d_4_0_Core->EndQueryIndexed(target, index);
}

inline void QOpenGLFunctions_4_4_Compatibility::glBeginQueryIndexed(GLenum target, GLuint index, GLuint id)
{
    d_4_0_Core->BeginQueryIndexed(target, index, id);
}

inline void QOpenGLFunctions_4_4_Compatibility::glDrawTransformFeedbackStream(GLenum mode, GLuint id, GLuint stream)
{
    d_4_0_Core->DrawTransformFeedbackStream(mode, id, stream);
}

inline void QOpenGLFunctions_4_4_Compatibility::glDrawTransformFeedback(GLenum mode, GLuint id)
{
    d_4_0_Core->DrawTransformFeedback(mode, id);
}

inline void QOpenGLFunctions_4_4_Compatibility::glResumeTransformFeedback()
{
    d_4_0_Core->ResumeTransformFeedback();
}

inline void QOpenGLFunctions_4_4_Compatibility::glPauseTransformFeedback()
{
    d_4_0_Core->PauseTransformFeedback();
}

inline GLboolean QOpenGLFunctions_4_4_Compatibility::glIsTransformFeedback(GLuint id)
{
    return d_4_0_Core->IsTransformFeedback(id);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGenTransformFeedbacks(GLsizei n, GLuint *ids)
{
    d_4_0_Core->GenTransformFeedbacks(n, ids);
}

inline void QOpenGLFunctions_4_4_Compatibility::glDeleteTransformFeedbacks(GLsizei n, const GLuint *ids)
{
    d_4_0_Core->DeleteTransformFeedbacks(n, ids);
}

inline void QOpenGLFunctions_4_4_Compatibility::glBindTransformFeedback(GLenum target, GLuint id)
{
    d_4_0_Core->BindTransformFeedback(target, id);
}

inline void QOpenGLFunctions_4_4_Compatibility::glPatchParameterfv(GLenum pname, const GLfloat *values)
{
    d_4_0_Core->PatchParameterfv(pname, values);
}

inline void QOpenGLFunctions_4_4_Compatibility::glPatchParameteri(GLenum pname, GLint value)
{
    d_4_0_Core->PatchParameteri(pname, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetProgramStageiv(GLuint program, GLenum shadertype, GLenum pname, GLint *values)
{
    d_4_0_Core->GetProgramStageiv(program, shadertype, pname, values);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetUniformSubroutineuiv(GLenum shadertype, GLint location, GLuint *params)
{
    d_4_0_Core->GetUniformSubroutineuiv(shadertype, location, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glUniformSubroutinesuiv(GLenum shadertype, GLsizei count, const GLuint *indices)
{
    d_4_0_Core->UniformSubroutinesuiv(shadertype, count, indices);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetActiveSubroutineName(GLuint program, GLenum shadertype, GLuint index, GLsizei bufsize, GLsizei *length, GLchar *name)
{
    d_4_0_Core->GetActiveSubroutineName(program, shadertype, index, bufsize, length, name);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetActiveSubroutineUniformName(GLuint program, GLenum shadertype, GLuint index, GLsizei bufsize, GLsizei *length, GLchar *name)
{
    d_4_0_Core->GetActiveSubroutineUniformName(program, shadertype, index, bufsize, length, name);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetActiveSubroutineUniformiv(GLuint program, GLenum shadertype, GLuint index, GLenum pname, GLint *values)
{
    d_4_0_Core->GetActiveSubroutineUniformiv(program, shadertype, index, pname, values);
}

inline GLuint QOpenGLFunctions_4_4_Compatibility::glGetSubroutineIndex(GLuint program, GLenum shadertype, const GLchar *name)
{
    return d_4_0_Core->GetSubroutineIndex(program, shadertype, name);
}

inline GLint QOpenGLFunctions_4_4_Compatibility::glGetSubroutineUniformLocation(GLuint program, GLenum shadertype, const GLchar *name)
{
    return d_4_0_Core->GetSubroutineUniformLocation(program, shadertype, name);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetUniformdv(GLuint program, GLint location, GLdouble *params)
{
    d_4_0_Core->GetUniformdv(program, location, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glUniformMatrix4x3dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
    d_4_0_Core->UniformMatrix4x3dv(location, count, transpose, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glUniformMatrix4x2dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
    d_4_0_Core->UniformMatrix4x2dv(location, count, transpose, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glUniformMatrix3x4dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
    d_4_0_Core->UniformMatrix3x4dv(location, count, transpose, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glUniformMatrix3x2dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
    d_4_0_Core->UniformMatrix3x2dv(location, count, transpose, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glUniformMatrix2x4dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
    d_4_0_Core->UniformMatrix2x4dv(location, count, transpose, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glUniformMatrix2x3dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
    d_4_0_Core->UniformMatrix2x3dv(location, count, transpose, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glUniformMatrix4dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
    d_4_0_Core->UniformMatrix4dv(location, count, transpose, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glUniformMatrix3dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
    d_4_0_Core->UniformMatrix3dv(location, count, transpose, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glUniformMatrix2dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
    d_4_0_Core->UniformMatrix2dv(location, count, transpose, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glUniform4dv(GLint location, GLsizei count, const GLdouble *value)
{
    d_4_0_Core->Uniform4dv(location, count, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glUniform3dv(GLint location, GLsizei count, const GLdouble *value)
{
    d_4_0_Core->Uniform3dv(location, count, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glUniform2dv(GLint location, GLsizei count, const GLdouble *value)
{
    d_4_0_Core->Uniform2dv(location, count, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glUniform1dv(GLint location, GLsizei count, const GLdouble *value)
{
    d_4_0_Core->Uniform1dv(location, count, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glUniform4d(GLint location, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    d_4_0_Core->Uniform4d(location, x, y, z, w);
}

inline void QOpenGLFunctions_4_4_Compatibility::glUniform3d(GLint location, GLdouble x, GLdouble y, GLdouble z)
{
    d_4_0_Core->Uniform3d(location, x, y, z);
}

inline void QOpenGLFunctions_4_4_Compatibility::glUniform2d(GLint location, GLdouble x, GLdouble y)
{
    d_4_0_Core->Uniform2d(location, x, y);
}

inline void QOpenGLFunctions_4_4_Compatibility::glUniform1d(GLint location, GLdouble x)
{
    d_4_0_Core->Uniform1d(location, x);
}

inline void QOpenGLFunctions_4_4_Compatibility::glDrawElementsIndirect(GLenum mode, GLenum type, const void *indirect)
{
    d_4_0_Core->DrawElementsIndirect(mode, type, indirect);
}

inline void QOpenGLFunctions_4_4_Compatibility::glDrawArraysIndirect(GLenum mode, const void *indirect)
{
    d_4_0_Core->DrawArraysIndirect(mode, indirect);
}

inline void QOpenGLFunctions_4_4_Compatibility::glBlendFuncSeparatei(GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)
{
    d_4_0_Core->BlendFuncSeparatei(buf, srcRGB, dstRGB, srcAlpha, dstAlpha);
}

inline void QOpenGLFunctions_4_4_Compatibility::glBlendFunci(GLuint buf, GLenum src, GLenum dst)
{
    d_4_0_Core->BlendFunci(buf, src, dst);
}

inline void QOpenGLFunctions_4_4_Compatibility::glBlendEquationSeparatei(GLuint buf, GLenum modeRGB, GLenum modeAlpha)
{
    d_4_0_Core->BlendEquationSeparatei(buf, modeRGB, modeAlpha);
}

inline void QOpenGLFunctions_4_4_Compatibility::glBlendEquationi(GLuint buf, GLenum mode)
{
    d_4_0_Core->BlendEquationi(buf, mode);
}

inline void QOpenGLFunctions_4_4_Compatibility::glMinSampleShading(GLfloat value)
{
    d_4_0_Core->MinSampleShading(value);
}


// OpenGL 4.1 core functions
inline void QOpenGLFunctions_4_4_Compatibility::glGetDoublei_v(GLenum target, GLuint index, GLdouble *data)
{
    d_4_1_Core->GetDoublei_v(target, index, data);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetFloati_v(GLenum target, GLuint index, GLfloat *data)
{
    d_4_1_Core->GetFloati_v(target, index, data);
}

inline void QOpenGLFunctions_4_4_Compatibility::glDepthRangeIndexed(GLuint index, GLdouble n, GLdouble f)
{
    d_4_1_Core->DepthRangeIndexed(index, n, f);
}

inline void QOpenGLFunctions_4_4_Compatibility::glDepthRangeArrayv(GLuint first, GLsizei count, const GLdouble *v)
{
    d_4_1_Core->DepthRangeArrayv(first, count, v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glScissorIndexedv(GLuint index, const GLint *v)
{
    d_4_1_Core->ScissorIndexedv(index, v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glScissorIndexed(GLuint index, GLint left, GLint bottom, GLsizei width, GLsizei height)
{
    d_4_1_Core->ScissorIndexed(index, left, bottom, width, height);
}

inline void QOpenGLFunctions_4_4_Compatibility::glScissorArrayv(GLuint first, GLsizei count, const GLint *v)
{
    d_4_1_Core->ScissorArrayv(first, count, v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glViewportIndexedfv(GLuint index, const GLfloat *v)
{
    d_4_1_Core->ViewportIndexedfv(index, v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glViewportIndexedf(GLuint index, GLfloat x, GLfloat y, GLfloat w, GLfloat h)
{
    d_4_1_Core->ViewportIndexedf(index, x, y, w, h);
}

inline void QOpenGLFunctions_4_4_Compatibility::glViewportArrayv(GLuint first, GLsizei count, const GLfloat *v)
{
    d_4_1_Core->ViewportArrayv(first, count, v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetVertexAttribLdv(GLuint index, GLenum pname, GLdouble *params)
{
    d_4_1_Core->GetVertexAttribLdv(index, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttribLPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer)
{
    d_4_1_Core->VertexAttribLPointer(index, size, type, stride, pointer);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttribL4dv(GLuint index, const GLdouble *v)
{
    d_4_1_Core->VertexAttribL4dv(index, v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttribL3dv(GLuint index, const GLdouble *v)
{
    d_4_1_Core->VertexAttribL3dv(index, v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttribL2dv(GLuint index, const GLdouble *v)
{
    d_4_1_Core->VertexAttribL2dv(index, v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttribL1dv(GLuint index, const GLdouble *v)
{
    d_4_1_Core->VertexAttribL1dv(index, v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttribL4d(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    d_4_1_Core->VertexAttribL4d(index, x, y, z, w);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttribL3d(GLuint index, GLdouble x, GLdouble y, GLdouble z)
{
    d_4_1_Core->VertexAttribL3d(index, x, y, z);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttribL2d(GLuint index, GLdouble x, GLdouble y)
{
    d_4_1_Core->VertexAttribL2d(index, x, y);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttribL1d(GLuint index, GLdouble x)
{
    d_4_1_Core->VertexAttribL1d(index, x);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetProgramPipelineInfoLog(GLuint pipeline, GLsizei bufSize, GLsizei *length, GLchar *infoLog)
{
    d_4_1_Core->GetProgramPipelineInfoLog(pipeline, bufSize, length, infoLog);
}

inline void QOpenGLFunctions_4_4_Compatibility::glValidateProgramPipeline(GLuint pipeline)
{
    d_4_1_Core->ValidateProgramPipeline(pipeline);
}

inline void QOpenGLFunctions_4_4_Compatibility::glProgramUniformMatrix4x3dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
    d_4_1_Core->ProgramUniformMatrix4x3dv(program, location, count, transpose, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glProgramUniformMatrix3x4dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
    d_4_1_Core->ProgramUniformMatrix3x4dv(program, location, count, transpose, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glProgramUniformMatrix4x2dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
    d_4_1_Core->ProgramUniformMatrix4x2dv(program, location, count, transpose, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glProgramUniformMatrix2x4dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
    d_4_1_Core->ProgramUniformMatrix2x4dv(program, location, count, transpose, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glProgramUniformMatrix3x2dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
    d_4_1_Core->ProgramUniformMatrix3x2dv(program, location, count, transpose, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glProgramUniformMatrix2x3dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
    d_4_1_Core->ProgramUniformMatrix2x3dv(program, location, count, transpose, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glProgramUniformMatrix4x3fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    d_4_1_Core->ProgramUniformMatrix4x3fv(program, location, count, transpose, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glProgramUniformMatrix3x4fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    d_4_1_Core->ProgramUniformMatrix3x4fv(program, location, count, transpose, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glProgramUniformMatrix4x2fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    d_4_1_Core->ProgramUniformMatrix4x2fv(program, location, count, transpose, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glProgramUniformMatrix2x4fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    d_4_1_Core->ProgramUniformMatrix2x4fv(program, location, count, transpose, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glProgramUniformMatrix3x2fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    d_4_1_Core->ProgramUniformMatrix3x2fv(program, location, count, transpose, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glProgramUniformMatrix2x3fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    d_4_1_Core->ProgramUniformMatrix2x3fv(program, location, count, transpose, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glProgramUniformMatrix4dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
    d_4_1_Core->ProgramUniformMatrix4dv(program, location, count, transpose, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glProgramUniformMatrix3dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
    d_4_1_Core->ProgramUniformMatrix3dv(program, location, count, transpose, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glProgramUniformMatrix2dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
    d_4_1_Core->ProgramUniformMatrix2dv(program, location, count, transpose, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glProgramUniformMatrix4fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    d_4_1_Core->ProgramUniformMatrix4fv(program, location, count, transpose, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glProgramUniformMatrix3fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    d_4_1_Core->ProgramUniformMatrix3fv(program, location, count, transpose, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glProgramUniformMatrix2fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    d_4_1_Core->ProgramUniformMatrix2fv(program, location, count, transpose, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glProgramUniform4uiv(GLuint program, GLint location, GLsizei count, const GLuint *value)
{
    d_4_1_Core->ProgramUniform4uiv(program, location, count, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glProgramUniform4ui(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
    d_4_1_Core->ProgramUniform4ui(program, location, v0, v1, v2, v3);
}

inline void QOpenGLFunctions_4_4_Compatibility::glProgramUniform4dv(GLuint program, GLint location, GLsizei count, const GLdouble *value)
{
    d_4_1_Core->ProgramUniform4dv(program, location, count, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glProgramUniform4d(GLuint program, GLint location, GLdouble v0, GLdouble v1, GLdouble v2, GLdouble v3)
{
    d_4_1_Core->ProgramUniform4d(program, location, v0, v1, v2, v3);
}

inline void QOpenGLFunctions_4_4_Compatibility::glProgramUniform4fv(GLuint program, GLint location, GLsizei count, const GLfloat *value)
{
    d_4_1_Core->ProgramUniform4fv(program, location, count, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glProgramUniform4f(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
    d_4_1_Core->ProgramUniform4f(program, location, v0, v1, v2, v3);
}

inline void QOpenGLFunctions_4_4_Compatibility::glProgramUniform4iv(GLuint program, GLint location, GLsizei count, const GLint *value)
{
    d_4_1_Core->ProgramUniform4iv(program, location, count, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glProgramUniform4i(GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3)
{
    d_4_1_Core->ProgramUniform4i(program, location, v0, v1, v2, v3);
}

inline void QOpenGLFunctions_4_4_Compatibility::glProgramUniform3uiv(GLuint program, GLint location, GLsizei count, const GLuint *value)
{
    d_4_1_Core->ProgramUniform3uiv(program, location, count, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glProgramUniform3ui(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2)
{
    d_4_1_Core->ProgramUniform3ui(program, location, v0, v1, v2);
}

inline void QOpenGLFunctions_4_4_Compatibility::glProgramUniform3dv(GLuint program, GLint location, GLsizei count, const GLdouble *value)
{
    d_4_1_Core->ProgramUniform3dv(program, location, count, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glProgramUniform3d(GLuint program, GLint location, GLdouble v0, GLdouble v1, GLdouble v2)
{
    d_4_1_Core->ProgramUniform3d(program, location, v0, v1, v2);
}

inline void QOpenGLFunctions_4_4_Compatibility::glProgramUniform3fv(GLuint program, GLint location, GLsizei count, const GLfloat *value)
{
    d_4_1_Core->ProgramUniform3fv(program, location, count, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glProgramUniform3f(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
{
    d_4_1_Core->ProgramUniform3f(program, location, v0, v1, v2);
}

inline void QOpenGLFunctions_4_4_Compatibility::glProgramUniform3iv(GLuint program, GLint location, GLsizei count, const GLint *value)
{
    d_4_1_Core->ProgramUniform3iv(program, location, count, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glProgramUniform3i(GLuint program, GLint location, GLint v0, GLint v1, GLint v2)
{
    d_4_1_Core->ProgramUniform3i(program, location, v0, v1, v2);
}

inline void QOpenGLFunctions_4_4_Compatibility::glProgramUniform2uiv(GLuint program, GLint location, GLsizei count, const GLuint *value)
{
    d_4_1_Core->ProgramUniform2uiv(program, location, count, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glProgramUniform2ui(GLuint program, GLint location, GLuint v0, GLuint v1)
{
    d_4_1_Core->ProgramUniform2ui(program, location, v0, v1);
}

inline void QOpenGLFunctions_4_4_Compatibility::glProgramUniform2dv(GLuint program, GLint location, GLsizei count, const GLdouble *value)
{
    d_4_1_Core->ProgramUniform2dv(program, location, count, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glProgramUniform2d(GLuint program, GLint location, GLdouble v0, GLdouble v1)
{
    d_4_1_Core->ProgramUniform2d(program, location, v0, v1);
}

inline void QOpenGLFunctions_4_4_Compatibility::glProgramUniform2fv(GLuint program, GLint location, GLsizei count, const GLfloat *value)
{
    d_4_1_Core->ProgramUniform2fv(program, location, count, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glProgramUniform2f(GLuint program, GLint location, GLfloat v0, GLfloat v1)
{
    d_4_1_Core->ProgramUniform2f(program, location, v0, v1);
}

inline void QOpenGLFunctions_4_4_Compatibility::glProgramUniform2iv(GLuint program, GLint location, GLsizei count, const GLint *value)
{
    d_4_1_Core->ProgramUniform2iv(program, location, count, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glProgramUniform2i(GLuint program, GLint location, GLint v0, GLint v1)
{
    d_4_1_Core->ProgramUniform2i(program, location, v0, v1);
}

inline void QOpenGLFunctions_4_4_Compatibility::glProgramUniform1uiv(GLuint program, GLint location, GLsizei count, const GLuint *value)
{
    d_4_1_Core->ProgramUniform1uiv(program, location, count, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glProgramUniform1ui(GLuint program, GLint location, GLuint v0)
{
    d_4_1_Core->ProgramUniform1ui(program, location, v0);
}

inline void QOpenGLFunctions_4_4_Compatibility::glProgramUniform1dv(GLuint program, GLint location, GLsizei count, const GLdouble *value)
{
    d_4_1_Core->ProgramUniform1dv(program, location, count, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glProgramUniform1d(GLuint program, GLint location, GLdouble v0)
{
    d_4_1_Core->ProgramUniform1d(program, location, v0);
}

inline void QOpenGLFunctions_4_4_Compatibility::glProgramUniform1fv(GLuint program, GLint location, GLsizei count, const GLfloat *value)
{
    d_4_1_Core->ProgramUniform1fv(program, location, count, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glProgramUniform1f(GLuint program, GLint location, GLfloat v0)
{
    d_4_1_Core->ProgramUniform1f(program, location, v0);
}

inline void QOpenGLFunctions_4_4_Compatibility::glProgramUniform1iv(GLuint program, GLint location, GLsizei count, const GLint *value)
{
    d_4_1_Core->ProgramUniform1iv(program, location, count, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glProgramUniform1i(GLuint program, GLint location, GLint v0)
{
    d_4_1_Core->ProgramUniform1i(program, location, v0);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetProgramPipelineiv(GLuint pipeline, GLenum pname, GLint *params)
{
    d_4_1_Core->GetProgramPipelineiv(pipeline, pname, params);
}

inline GLboolean QOpenGLFunctions_4_4_Compatibility::glIsProgramPipeline(GLuint pipeline)
{
    return d_4_1_Core->IsProgramPipeline(pipeline);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGenProgramPipelines(GLsizei n, GLuint *pipelines)
{
    d_4_1_Core->GenProgramPipelines(n, pipelines);
}

inline void QOpenGLFunctions_4_4_Compatibility::glDeleteProgramPipelines(GLsizei n, const GLuint *pipelines)
{
    d_4_1_Core->DeleteProgramPipelines(n, pipelines);
}

inline void QOpenGLFunctions_4_4_Compatibility::glBindProgramPipeline(GLuint pipeline)
{
    d_4_1_Core->BindProgramPipeline(pipeline);
}

inline GLuint QOpenGLFunctions_4_4_Compatibility::glCreateShaderProgramv(GLenum type, GLsizei count, const GLchar *const *strings)
{
    return d_4_1_Core->CreateShaderProgramv(type, count, strings);
}

inline void QOpenGLFunctions_4_4_Compatibility::glActiveShaderProgram(GLuint pipeline, GLuint program)
{
    d_4_1_Core->ActiveShaderProgram(pipeline, program);
}

inline void QOpenGLFunctions_4_4_Compatibility::glUseProgramStages(GLuint pipeline, GLbitfield stages, GLuint program)
{
    d_4_1_Core->UseProgramStages(pipeline, stages, program);
}

inline void QOpenGLFunctions_4_4_Compatibility::glProgramParameteri(GLuint program, GLenum pname, GLint value)
{
    d_4_1_Core->ProgramParameteri(program, pname, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glProgramBinary(GLuint program, GLenum binaryFormat, const void *binary, GLsizei length)
{
    d_4_1_Core->ProgramBinary(program, binaryFormat, binary, length);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetProgramBinary(GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, void *binary)
{
    d_4_1_Core->GetProgramBinary(program, bufSize, length, binaryFormat, binary);
}

inline void QOpenGLFunctions_4_4_Compatibility::glClearDepthf(GLfloat dd)
{
    d_4_1_Core->ClearDepthf(dd);
}

inline void QOpenGLFunctions_4_4_Compatibility::glDepthRangef(GLfloat n, GLfloat f)
{
    d_4_1_Core->DepthRangef(n, f);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetShaderPrecisionFormat(GLenum shadertype, GLenum precisiontype, GLint *range, GLint *precision)
{
    d_4_1_Core->GetShaderPrecisionFormat(shadertype, precisiontype, range, precision);
}

inline void QOpenGLFunctions_4_4_Compatibility::glShaderBinary(GLsizei count, const GLuint *shaders, GLenum binaryformat, const void *binary, GLsizei length)
{
    d_4_1_Core->ShaderBinary(count, shaders, binaryformat, binary, length);
}

inline void QOpenGLFunctions_4_4_Compatibility::glReleaseShaderCompiler()
{
    d_4_1_Core->ReleaseShaderCompiler();
}


// OpenGL 4.2 core functions
inline void QOpenGLFunctions_4_4_Compatibility::glDrawTransformFeedbackStreamInstanced(GLenum mode, GLuint id, GLuint stream, GLsizei instancecount)
{
    d_4_2_Core->DrawTransformFeedbackStreamInstanced(mode, id, stream, instancecount);
}

inline void QOpenGLFunctions_4_4_Compatibility::glDrawTransformFeedbackInstanced(GLenum mode, GLuint id, GLsizei instancecount)
{
    d_4_2_Core->DrawTransformFeedbackInstanced(mode, id, instancecount);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexStorage3D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth)
{
    d_4_2_Core->TexStorage3D(target, levels, internalformat, width, height, depth);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexStorage2D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)
{
    d_4_2_Core->TexStorage2D(target, levels, internalformat, width, height);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexStorage1D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width)
{
    d_4_2_Core->TexStorage1D(target, levels, internalformat, width);
}

inline void QOpenGLFunctions_4_4_Compatibility::glMemoryBarrier(GLbitfield barriers)
{
    d_4_2_Core->MemoryBarrier(barriers);
}

inline void QOpenGLFunctions_4_4_Compatibility::glBindImageTexture(GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format)
{
    d_4_2_Core->BindImageTexture(unit, texture, level, layered, layer, access, format);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetActiveAtomicCounterBufferiv(GLuint program, GLuint bufferIndex, GLenum pname, GLint *params)
{
    d_4_2_Core->GetActiveAtomicCounterBufferiv(program, bufferIndex, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetInternalformativ(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint *params)
{
    d_4_2_Core->GetInternalformativ(target, internalformat, pname, bufSize, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glDrawElementsInstancedBaseVertexBaseInstance(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex, GLuint baseinstance)
{
    d_4_2_Core->DrawElementsInstancedBaseVertexBaseInstance(mode, count, type, indices, instancecount, basevertex, baseinstance);
}

inline void QOpenGLFunctions_4_4_Compatibility::glDrawElementsInstancedBaseInstance(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLuint baseinstance)
{
    d_4_2_Core->DrawElementsInstancedBaseInstance(mode, count, type, indices, instancecount, baseinstance);
}

inline void QOpenGLFunctions_4_4_Compatibility::glDrawArraysInstancedBaseInstance(GLenum mode, GLint first, GLsizei count, GLsizei instancecount, GLuint baseinstance)
{
    d_4_2_Core->DrawArraysInstancedBaseInstance(mode, first, count, instancecount, baseinstance);
}


// OpenGL 4.3 core functions
inline void QOpenGLFunctions_4_4_Compatibility::glGetObjectPtrLabel(const void *ptr, GLsizei bufSize, GLsizei *length, GLchar *label)
{
    d_4_3_Core->GetObjectPtrLabel(ptr, bufSize, length, label);
}

inline void QOpenGLFunctions_4_4_Compatibility::glObjectPtrLabel(const void *ptr, GLsizei length, const GLchar *label)
{
    d_4_3_Core->ObjectPtrLabel(ptr, length, label);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetObjectLabel(GLenum identifier, GLuint name, GLsizei bufSize, GLsizei *length, GLchar *label)
{
    d_4_3_Core->GetObjectLabel(identifier, name, bufSize, length, label);
}

inline void QOpenGLFunctions_4_4_Compatibility::glObjectLabel(GLenum identifier, GLuint name, GLsizei length, const GLchar *label)
{
    d_4_3_Core->ObjectLabel(identifier, name, length, label);
}

inline void QOpenGLFunctions_4_4_Compatibility::glPopDebugGroup()
{
    d_4_3_Core->PopDebugGroup();
}

inline void QOpenGLFunctions_4_4_Compatibility::glPushDebugGroup(GLenum source, GLuint id, GLsizei length, const GLchar *message)
{
    d_4_3_Core->PushDebugGroup(source, id, length, message);
}

inline GLuint QOpenGLFunctions_4_4_Compatibility::glGetDebugMessageLog(GLuint count, GLsizei bufSize, GLenum *sources, GLenum *types, GLuint *ids, GLenum *severities, GLsizei *lengths, GLchar *messageLog)
{
    return d_4_3_Core->GetDebugMessageLog(count, bufSize, sources, types, ids, severities, lengths, messageLog);
}

inline void QOpenGLFunctions_4_4_Compatibility::glDebugMessageCallback(GLDEBUGPROC callback, const void *userParam)
{
    d_4_3_Core->DebugMessageCallback(callback, userParam);
}

inline void QOpenGLFunctions_4_4_Compatibility::glDebugMessageInsert(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *buf)
{
    d_4_3_Core->DebugMessageInsert(source, type, id, severity, length, buf);
}

inline void QOpenGLFunctions_4_4_Compatibility::glDebugMessageControl(GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint *ids, GLboolean enabled)
{
    d_4_3_Core->DebugMessageControl(source, type, severity, count, ids, enabled);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexBindingDivisor(GLuint bindingindex, GLuint divisor)
{
    d_4_3_Core->VertexBindingDivisor(bindingindex, divisor);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttribBinding(GLuint attribindex, GLuint bindingindex)
{
    d_4_3_Core->VertexAttribBinding(attribindex, bindingindex);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttribLFormat(GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset)
{
    d_4_3_Core->VertexAttribLFormat(attribindex, size, type, relativeoffset);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttribIFormat(GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset)
{
    d_4_3_Core->VertexAttribIFormat(attribindex, size, type, relativeoffset);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexAttribFormat(GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset)
{
    d_4_3_Core->VertexAttribFormat(attribindex, size, type, normalized, relativeoffset);
}

inline void QOpenGLFunctions_4_4_Compatibility::glBindVertexBuffer(GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride)
{
    d_4_3_Core->BindVertexBuffer(bindingindex, buffer, offset, stride);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTextureView(GLuint texture, GLenum target, GLuint origtexture, GLenum internalformat, GLuint minlevel, GLuint numlevels, GLuint minlayer, GLuint numlayers)
{
    d_4_3_Core->TextureView(texture, target, origtexture, internalformat, minlevel, numlevels, minlayer, numlayers);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexStorage3DMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations)
{
    d_4_3_Core->TexStorage3DMultisample(target, samples, internalformat, width, height, depth, fixedsamplelocations);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexStorage2DMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations)
{
    d_4_3_Core->TexStorage2DMultisample(target, samples, internalformat, width, height, fixedsamplelocations);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexBufferRange(GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size)
{
    d_4_3_Core->TexBufferRange(target, internalformat, buffer, offset, size);
}

inline void QOpenGLFunctions_4_4_Compatibility::glShaderStorageBlockBinding(GLuint program, GLuint storageBlockIndex, GLuint storageBlockBinding)
{
    d_4_3_Core->ShaderStorageBlockBinding(program, storageBlockIndex, storageBlockBinding);
}

inline GLint QOpenGLFunctions_4_4_Compatibility::glGetProgramResourceLocationIndex(GLuint program, GLenum programInterface, const GLchar *name)
{
    return d_4_3_Core->GetProgramResourceLocationIndex(program, programInterface, name);
}

inline GLint QOpenGLFunctions_4_4_Compatibility::glGetProgramResourceLocation(GLuint program, GLenum programInterface, const GLchar *name)
{
    return d_4_3_Core->GetProgramResourceLocation(program, programInterface, name);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetProgramResourceiv(GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum *props, GLsizei bufSize, GLsizei *length, GLint *params)
{
    d_4_3_Core->GetProgramResourceiv(program, programInterface, index, propCount, props, bufSize, length, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetProgramResourceName(GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name)
{
    d_4_3_Core->GetProgramResourceName(program, programInterface, index, bufSize, length, name);
}

inline GLuint QOpenGLFunctions_4_4_Compatibility::glGetProgramResourceIndex(GLuint program, GLenum programInterface, const GLchar *name)
{
    return d_4_3_Core->GetProgramResourceIndex(program, programInterface, name);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetProgramInterfaceiv(GLuint program, GLenum programInterface, GLenum pname, GLint *params)
{
    d_4_3_Core->GetProgramInterfaceiv(program, programInterface, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glMultiDrawElementsIndirect(GLenum mode, GLenum type, const void *indirect, GLsizei drawcount, GLsizei stride)
{
    d_4_3_Core->MultiDrawElementsIndirect(mode, type, indirect, drawcount, stride);
}

inline void QOpenGLFunctions_4_4_Compatibility::glMultiDrawArraysIndirect(GLenum mode, const void *indirect, GLsizei drawcount, GLsizei stride)
{
    d_4_3_Core->MultiDrawArraysIndirect(mode, indirect, drawcount, stride);
}

inline void QOpenGLFunctions_4_4_Compatibility::glInvalidateSubFramebuffer(GLenum target, GLsizei numAttachments, const GLenum *attachments, GLint x, GLint y, GLsizei width, GLsizei height)
{
    d_4_3_Core->InvalidateSubFramebuffer(target, numAttachments, attachments, x, y, width, height);
}

inline void QOpenGLFunctions_4_4_Compatibility::glInvalidateFramebuffer(GLenum target, GLsizei numAttachments, const GLenum *attachments)
{
    d_4_3_Core->InvalidateFramebuffer(target, numAttachments, attachments);
}

inline void QOpenGLFunctions_4_4_Compatibility::glInvalidateBufferData(GLuint buffer)
{
    d_4_3_Core->InvalidateBufferData(buffer);
}

inline void QOpenGLFunctions_4_4_Compatibility::glInvalidateBufferSubData(GLuint buffer, GLintptr offset, GLsizeiptr length)
{
    d_4_3_Core->InvalidateBufferSubData(buffer, offset, length);
}

inline void QOpenGLFunctions_4_4_Compatibility::glInvalidateTexImage(GLuint texture, GLint level)
{
    d_4_3_Core->InvalidateTexImage(texture, level);
}

inline void QOpenGLFunctions_4_4_Compatibility::glInvalidateTexSubImage(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth)
{
    d_4_3_Core->InvalidateTexSubImage(texture, level, xoffset, yoffset, zoffset, width, height, depth);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetInternalformati64v(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint64 *params)
{
    d_4_3_Core->GetInternalformati64v(target, internalformat, pname, bufSize, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetFramebufferParameteriv(GLenum target, GLenum pname, GLint *params)
{
    d_4_3_Core->GetFramebufferParameteriv(target, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glFramebufferParameteri(GLenum target, GLenum pname, GLint param)
{
    d_4_3_Core->FramebufferParameteri(target, pname, param);
}

inline void QOpenGLFunctions_4_4_Compatibility::glCopyImageSubData(GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth)
{
    d_4_3_Core->CopyImageSubData(srcName, srcTarget, srcLevel, srcX, srcY, srcZ, dstName, dstTarget, dstLevel, dstX, dstY, dstZ, srcWidth, srcHeight, srcDepth);
}

inline void QOpenGLFunctions_4_4_Compatibility::glDispatchComputeIndirect(GLintptr indirect)
{
    d_4_3_Core->DispatchComputeIndirect(indirect);
}

inline void QOpenGLFunctions_4_4_Compatibility::glDispatchCompute(GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z)
{
    d_4_3_Core->DispatchCompute(num_groups_x, num_groups_y, num_groups_z);
}

inline void QOpenGLFunctions_4_4_Compatibility::glClearBufferSubData(GLenum target, GLenum internalformat, GLintptr offset, GLsizeiptr size, GLenum format, GLenum type, const void *data)
{
    d_4_3_Core->ClearBufferSubData(target, internalformat, offset, size, format, type, data);
}

inline void QOpenGLFunctions_4_4_Compatibility::glClearBufferData(GLenum target, GLenum internalformat, GLenum format, GLenum type, const void *data)
{
    d_4_3_Core->ClearBufferData(target, internalformat, format, type, data);
}


// OpenGL 4.4 core functions
inline void QOpenGLFunctions_4_4_Compatibility::glBindVertexBuffers(GLuint first, GLsizei count, const GLuint *buffers, const GLintptr *offsets, const GLsizei *strides)
{
    d_4_4_Core->BindVertexBuffers(first, count, buffers, offsets, strides);
}

inline void QOpenGLFunctions_4_4_Compatibility::glBindImageTextures(GLuint first, GLsizei count, const GLuint *textures)
{
    d_4_4_Core->BindImageTextures(first, count, textures);
}

inline void QOpenGLFunctions_4_4_Compatibility::glBindSamplers(GLuint first, GLsizei count, const GLuint *samplers)
{
    d_4_4_Core->BindSamplers(first, count, samplers);
}

inline void QOpenGLFunctions_4_4_Compatibility::glBindTextures(GLuint first, GLsizei count, const GLuint *textures)
{
    d_4_4_Core->BindTextures(first, count, textures);
}

inline void QOpenGLFunctions_4_4_Compatibility::glBindBuffersRange(GLenum target, GLuint first, GLsizei count, const GLuint *buffers, const GLintptr *offsets, const GLsizeiptr *sizes)
{
    d_4_4_Core->BindBuffersRange(target, first, count, buffers, offsets, sizes);
}

inline void QOpenGLFunctions_4_4_Compatibility::glBindBuffersBase(GLenum target, GLuint first, GLsizei count, const GLuint *buffers)
{
    d_4_4_Core->BindBuffersBase(target, first, count, buffers);
}

inline void QOpenGLFunctions_4_4_Compatibility::glClearTexSubImage(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *data)
{
    d_4_4_Core->ClearTexSubImage(texture, level, xoffset, yoffset, zoffset, width, height, depth, format, type, data);
}

inline void QOpenGLFunctions_4_4_Compatibility::glClearTexImage(GLuint texture, GLint level, GLenum format, GLenum type, const void *data)
{
    d_4_4_Core->ClearTexImage(texture, level, format, type, data);
}

inline void QOpenGLFunctions_4_4_Compatibility::glBufferStorage(GLenum target, GLsizeiptr size, const void *data, GLbitfield flags)
{
    d_4_4_Core->BufferStorage(target, size, data, flags);
}


// OpenGL 1.0 deprecated functions
inline void QOpenGLFunctions_4_4_Compatibility::glTranslatef(GLfloat x, GLfloat y, GLfloat z)
{
    d_1_0_Deprecated->Translatef(x, y, z);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTranslated(GLdouble x, GLdouble y, GLdouble z)
{
    d_1_0_Deprecated->Translated(x, y, z);
}

inline void QOpenGLFunctions_4_4_Compatibility::glScalef(GLfloat x, GLfloat y, GLfloat z)
{
    d_1_0_Deprecated->Scalef(x, y, z);
}

inline void QOpenGLFunctions_4_4_Compatibility::glScaled(GLdouble x, GLdouble y, GLdouble z)
{
    d_1_0_Deprecated->Scaled(x, y, z);
}

inline void QOpenGLFunctions_4_4_Compatibility::glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
    d_1_0_Deprecated->Rotatef(angle, x, y, z);
}

inline void QOpenGLFunctions_4_4_Compatibility::glRotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z)
{
    d_1_0_Deprecated->Rotated(angle, x, y, z);
}

inline void QOpenGLFunctions_4_4_Compatibility::glPushMatrix()
{
    d_1_0_Deprecated->PushMatrix();
}

inline void QOpenGLFunctions_4_4_Compatibility::glPopMatrix()
{
    d_1_0_Deprecated->PopMatrix();
}

inline void QOpenGLFunctions_4_4_Compatibility::glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)
{
    d_1_0_Deprecated->Ortho(left, right, bottom, top, zNear, zFar);
}

inline void QOpenGLFunctions_4_4_Compatibility::glMultMatrixd(const GLdouble *m)
{
    d_1_0_Deprecated->MultMatrixd(m);
}

inline void QOpenGLFunctions_4_4_Compatibility::glMultMatrixf(const GLfloat *m)
{
    d_1_0_Deprecated->MultMatrixf(m);
}

inline void QOpenGLFunctions_4_4_Compatibility::glMatrixMode(GLenum mode)
{
    d_1_0_Deprecated->MatrixMode(mode);
}

inline void QOpenGLFunctions_4_4_Compatibility::glLoadMatrixd(const GLdouble *m)
{
    d_1_0_Deprecated->LoadMatrixd(m);
}

inline void QOpenGLFunctions_4_4_Compatibility::glLoadMatrixf(const GLfloat *m)
{
    d_1_0_Deprecated->LoadMatrixf(m);
}

inline void QOpenGLFunctions_4_4_Compatibility::glLoadIdentity()
{
    d_1_0_Deprecated->LoadIdentity();
}

inline void QOpenGLFunctions_4_4_Compatibility::glFrustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)
{
    d_1_0_Deprecated->Frustum(left, right, bottom, top, zNear, zFar);
}

inline GLboolean QOpenGLFunctions_4_4_Compatibility::glIsList(GLuint list)
{
    return d_1_0_Deprecated->IsList(list);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetTexGeniv(GLenum coord, GLenum pname, GLint *params)
{
    d_1_0_Deprecated->GetTexGeniv(coord, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetTexGenfv(GLenum coord, GLenum pname, GLfloat *params)
{
    d_1_0_Deprecated->GetTexGenfv(coord, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetTexGendv(GLenum coord, GLenum pname, GLdouble *params)
{
    d_1_0_Deprecated->GetTexGendv(coord, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetTexEnviv(GLenum target, GLenum pname, GLint *params)
{
    d_1_0_Deprecated->GetTexEnviv(target, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetTexEnvfv(GLenum target, GLenum pname, GLfloat *params)
{
    d_1_0_Deprecated->GetTexEnvfv(target, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetPolygonStipple(GLubyte *mask)
{
    d_1_0_Deprecated->GetPolygonStipple(mask);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetPixelMapusv(GLenum map, GLushort *values)
{
    d_1_0_Deprecated->GetPixelMapusv(map, values);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetPixelMapuiv(GLenum map, GLuint *values)
{
    d_1_0_Deprecated->GetPixelMapuiv(map, values);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetPixelMapfv(GLenum map, GLfloat *values)
{
    d_1_0_Deprecated->GetPixelMapfv(map, values);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetMaterialiv(GLenum face, GLenum pname, GLint *params)
{
    d_1_0_Deprecated->GetMaterialiv(face, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetMaterialfv(GLenum face, GLenum pname, GLfloat *params)
{
    d_1_0_Deprecated->GetMaterialfv(face, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetMapiv(GLenum target, GLenum query, GLint *v)
{
    d_1_0_Deprecated->GetMapiv(target, query, v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetMapfv(GLenum target, GLenum query, GLfloat *v)
{
    d_1_0_Deprecated->GetMapfv(target, query, v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetMapdv(GLenum target, GLenum query, GLdouble *v)
{
    d_1_0_Deprecated->GetMapdv(target, query, v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetLightiv(GLenum light, GLenum pname, GLint *params)
{
    d_1_0_Deprecated->GetLightiv(light, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetLightfv(GLenum light, GLenum pname, GLfloat *params)
{
    d_1_0_Deprecated->GetLightfv(light, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetClipPlane(GLenum plane, GLdouble *equation)
{
    d_1_0_Deprecated->GetClipPlane(plane, equation);
}

inline void QOpenGLFunctions_4_4_Compatibility::glDrawPixels(GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels)
{
    d_1_0_Deprecated->DrawPixels(width, height, format, type, pixels);
}

inline void QOpenGLFunctions_4_4_Compatibility::glCopyPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type)
{
    d_1_0_Deprecated->CopyPixels(x, y, width, height, type);
}

inline void QOpenGLFunctions_4_4_Compatibility::glPixelMapusv(GLenum map, GLsizei mapsize, const GLushort *values)
{
    d_1_0_Deprecated->PixelMapusv(map, mapsize, values);
}

inline void QOpenGLFunctions_4_4_Compatibility::glPixelMapuiv(GLenum map, GLsizei mapsize, const GLuint *values)
{
    d_1_0_Deprecated->PixelMapuiv(map, mapsize, values);
}

inline void QOpenGLFunctions_4_4_Compatibility::glPixelMapfv(GLenum map, GLsizei mapsize, const GLfloat *values)
{
    d_1_0_Deprecated->PixelMapfv(map, mapsize, values);
}

inline void QOpenGLFunctions_4_4_Compatibility::glPixelTransferi(GLenum pname, GLint param)
{
    d_1_0_Deprecated->PixelTransferi(pname, param);
}

inline void QOpenGLFunctions_4_4_Compatibility::glPixelTransferf(GLenum pname, GLfloat param)
{
    d_1_0_Deprecated->PixelTransferf(pname, param);
}

inline void QOpenGLFunctions_4_4_Compatibility::glPixelZoom(GLfloat xfactor, GLfloat yfactor)
{
    d_1_0_Deprecated->PixelZoom(xfactor, yfactor);
}

inline void QOpenGLFunctions_4_4_Compatibility::glAlphaFunc(GLenum func, GLfloat ref)
{
    d_1_0_Deprecated->AlphaFunc(func, ref);
}

inline void QOpenGLFunctions_4_4_Compatibility::glEvalPoint2(GLint i, GLint j)
{
    d_1_0_Deprecated->EvalPoint2(i, j);
}

inline void QOpenGLFunctions_4_4_Compatibility::glEvalMesh2(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2)
{
    d_1_0_Deprecated->EvalMesh2(mode, i1, i2, j1, j2);
}

inline void QOpenGLFunctions_4_4_Compatibility::glEvalPoint1(GLint i)
{
    d_1_0_Deprecated->EvalPoint1(i);
}

inline void QOpenGLFunctions_4_4_Compatibility::glEvalMesh1(GLenum mode, GLint i1, GLint i2)
{
    d_1_0_Deprecated->EvalMesh1(mode, i1, i2);
}

inline void QOpenGLFunctions_4_4_Compatibility::glEvalCoord2fv(const GLfloat *u)
{
    d_1_0_Deprecated->EvalCoord2fv(u);
}

inline void QOpenGLFunctions_4_4_Compatibility::glEvalCoord2f(GLfloat u, GLfloat v)
{
    d_1_0_Deprecated->EvalCoord2f(u, v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glEvalCoord2dv(const GLdouble *u)
{
    d_1_0_Deprecated->EvalCoord2dv(u);
}

inline void QOpenGLFunctions_4_4_Compatibility::glEvalCoord2d(GLdouble u, GLdouble v)
{
    d_1_0_Deprecated->EvalCoord2d(u, v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glEvalCoord1fv(const GLfloat *u)
{
    d_1_0_Deprecated->EvalCoord1fv(u);
}

inline void QOpenGLFunctions_4_4_Compatibility::glEvalCoord1f(GLfloat u)
{
    d_1_0_Deprecated->EvalCoord1f(u);
}

inline void QOpenGLFunctions_4_4_Compatibility::glEvalCoord1dv(const GLdouble *u)
{
    d_1_0_Deprecated->EvalCoord1dv(u);
}

inline void QOpenGLFunctions_4_4_Compatibility::glEvalCoord1d(GLdouble u)
{
    d_1_0_Deprecated->EvalCoord1d(u);
}

inline void QOpenGLFunctions_4_4_Compatibility::glMapGrid2f(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2)
{
    d_1_0_Deprecated->MapGrid2f(un, u1, u2, vn, v1, v2);
}

inline void QOpenGLFunctions_4_4_Compatibility::glMapGrid2d(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2)
{
    d_1_0_Deprecated->MapGrid2d(un, u1, u2, vn, v1, v2);
}

inline void QOpenGLFunctions_4_4_Compatibility::glMapGrid1f(GLint un, GLfloat u1, GLfloat u2)
{
    d_1_0_Deprecated->MapGrid1f(un, u1, u2);
}

inline void QOpenGLFunctions_4_4_Compatibility::glMapGrid1d(GLint un, GLdouble u1, GLdouble u2)
{
    d_1_0_Deprecated->MapGrid1d(un, u1, u2);
}

inline void QOpenGLFunctions_4_4_Compatibility::glMap2f(GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points)
{
    d_1_0_Deprecated->Map2f(target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points);
}

inline void QOpenGLFunctions_4_4_Compatibility::glMap2d(GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points)
{
    d_1_0_Deprecated->Map2d(target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points);
}

inline void QOpenGLFunctions_4_4_Compatibility::glMap1f(GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points)
{
    d_1_0_Deprecated->Map1f(target, u1, u2, stride, order, points);
}

inline void QOpenGLFunctions_4_4_Compatibility::glMap1d(GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points)
{
    d_1_0_Deprecated->Map1d(target, u1, u2, stride, order, points);
}

inline void QOpenGLFunctions_4_4_Compatibility::glPushAttrib(GLbitfield mask)
{
    d_1_0_Deprecated->PushAttrib(mask);
}

inline void QOpenGLFunctions_4_4_Compatibility::glPopAttrib()
{
    d_1_0_Deprecated->PopAttrib();
}

inline void QOpenGLFunctions_4_4_Compatibility::glAccum(GLenum op, GLfloat value)
{
    d_1_0_Deprecated->Accum(op, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glIndexMask(GLuint mask)
{
    d_1_0_Deprecated->IndexMask(mask);
}

inline void QOpenGLFunctions_4_4_Compatibility::glClearIndex(GLfloat c)
{
    d_1_0_Deprecated->ClearIndex(c);
}

inline void QOpenGLFunctions_4_4_Compatibility::glClearAccum(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    d_1_0_Deprecated->ClearAccum(red, green, blue, alpha);
}

inline void QOpenGLFunctions_4_4_Compatibility::glPushName(GLuint name)
{
    d_1_0_Deprecated->PushName(name);
}

inline void QOpenGLFunctions_4_4_Compatibility::glPopName()
{
    d_1_0_Deprecated->PopName();
}

inline void QOpenGLFunctions_4_4_Compatibility::glPassThrough(GLfloat token)
{
    d_1_0_Deprecated->PassThrough(token);
}

inline void QOpenGLFunctions_4_4_Compatibility::glLoadName(GLuint name)
{
    d_1_0_Deprecated->LoadName(name);
}

inline void QOpenGLFunctions_4_4_Compatibility::glInitNames()
{
    d_1_0_Deprecated->InitNames();
}

inline GLint QOpenGLFunctions_4_4_Compatibility::glRenderMode(GLenum mode)
{
    return d_1_0_Deprecated->RenderMode(mode);
}

inline void QOpenGLFunctions_4_4_Compatibility::glSelectBuffer(GLsizei size, GLuint *buffer)
{
    d_1_0_Deprecated->SelectBuffer(size, buffer);
}

inline void QOpenGLFunctions_4_4_Compatibility::glFeedbackBuffer(GLsizei size, GLenum type, GLfloat *buffer)
{
    d_1_0_Deprecated->FeedbackBuffer(size, type, buffer);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexGeniv(GLenum coord, GLenum pname, const GLint *params)
{
    d_1_0_Deprecated->TexGeniv(coord, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexGeni(GLenum coord, GLenum pname, GLint param)
{
    d_1_0_Deprecated->TexGeni(coord, pname, param);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexGenfv(GLenum coord, GLenum pname, const GLfloat *params)
{
    d_1_0_Deprecated->TexGenfv(coord, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexGenf(GLenum coord, GLenum pname, GLfloat param)
{
    d_1_0_Deprecated->TexGenf(coord, pname, param);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexGendv(GLenum coord, GLenum pname, const GLdouble *params)
{
    d_1_0_Deprecated->TexGendv(coord, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexGend(GLenum coord, GLenum pname, GLdouble param)
{
    d_1_0_Deprecated->TexGend(coord, pname, param);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexEnviv(GLenum target, GLenum pname, const GLint *params)
{
    d_1_0_Deprecated->TexEnviv(target, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexEnvi(GLenum target, GLenum pname, GLint param)
{
    d_1_0_Deprecated->TexEnvi(target, pname, param);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexEnvfv(GLenum target, GLenum pname, const GLfloat *params)
{
    d_1_0_Deprecated->TexEnvfv(target, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexEnvf(GLenum target, GLenum pname, GLfloat param)
{
    d_1_0_Deprecated->TexEnvf(target, pname, param);
}

inline void QOpenGLFunctions_4_4_Compatibility::glShadeModel(GLenum mode)
{
    d_1_0_Deprecated->ShadeModel(mode);
}

inline void QOpenGLFunctions_4_4_Compatibility::glPolygonStipple(const GLubyte *mask)
{
    d_1_0_Deprecated->PolygonStipple(mask);
}

inline void QOpenGLFunctions_4_4_Compatibility::glMaterialiv(GLenum face, GLenum pname, const GLint *params)
{
    d_1_0_Deprecated->Materialiv(face, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glMateriali(GLenum face, GLenum pname, GLint param)
{
    d_1_0_Deprecated->Materiali(face, pname, param);
}

inline void QOpenGLFunctions_4_4_Compatibility::glMaterialfv(GLenum face, GLenum pname, const GLfloat *params)
{
    d_1_0_Deprecated->Materialfv(face, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glMaterialf(GLenum face, GLenum pname, GLfloat param)
{
    d_1_0_Deprecated->Materialf(face, pname, param);
}

inline void QOpenGLFunctions_4_4_Compatibility::glLineStipple(GLint factor, GLushort pattern)
{
    d_1_0_Deprecated->LineStipple(factor, pattern);
}

inline void QOpenGLFunctions_4_4_Compatibility::glLightModeliv(GLenum pname, const GLint *params)
{
    d_1_0_Deprecated->LightModeliv(pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glLightModeli(GLenum pname, GLint param)
{
    d_1_0_Deprecated->LightModeli(pname, param);
}

inline void QOpenGLFunctions_4_4_Compatibility::glLightModelfv(GLenum pname, const GLfloat *params)
{
    d_1_0_Deprecated->LightModelfv(pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glLightModelf(GLenum pname, GLfloat param)
{
    d_1_0_Deprecated->LightModelf(pname, param);
}

inline void QOpenGLFunctions_4_4_Compatibility::glLightiv(GLenum light, GLenum pname, const GLint *params)
{
    d_1_0_Deprecated->Lightiv(light, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glLighti(GLenum light, GLenum pname, GLint param)
{
    d_1_0_Deprecated->Lighti(light, pname, param);
}

inline void QOpenGLFunctions_4_4_Compatibility::glLightfv(GLenum light, GLenum pname, const GLfloat *params)
{
    d_1_0_Deprecated->Lightfv(light, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glLightf(GLenum light, GLenum pname, GLfloat param)
{
    d_1_0_Deprecated->Lightf(light, pname, param);
}

inline void QOpenGLFunctions_4_4_Compatibility::glFogiv(GLenum pname, const GLint *params)
{
    d_1_0_Deprecated->Fogiv(pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glFogi(GLenum pname, GLint param)
{
    d_1_0_Deprecated->Fogi(pname, param);
}

inline void QOpenGLFunctions_4_4_Compatibility::glFogfv(GLenum pname, const GLfloat *params)
{
    d_1_0_Deprecated->Fogfv(pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glFogf(GLenum pname, GLfloat param)
{
    d_1_0_Deprecated->Fogf(pname, param);
}

inline void QOpenGLFunctions_4_4_Compatibility::glColorMaterial(GLenum face, GLenum mode)
{
    d_1_0_Deprecated->ColorMaterial(face, mode);
}

inline void QOpenGLFunctions_4_4_Compatibility::glClipPlane(GLenum plane, const GLdouble *equation)
{
    d_1_0_Deprecated->ClipPlane(plane, equation);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertex4sv(const GLshort *v)
{
    d_1_0_Deprecated->Vertex4sv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertex4s(GLshort x, GLshort y, GLshort z, GLshort w)
{
    d_1_0_Deprecated->Vertex4s(x, y, z, w);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertex4iv(const GLint *v)
{
    d_1_0_Deprecated->Vertex4iv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertex4i(GLint x, GLint y, GLint z, GLint w)
{
    d_1_0_Deprecated->Vertex4i(x, y, z, w);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertex4fv(const GLfloat *v)
{
    d_1_0_Deprecated->Vertex4fv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertex4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    d_1_0_Deprecated->Vertex4f(x, y, z, w);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertex4dv(const GLdouble *v)
{
    d_1_0_Deprecated->Vertex4dv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertex4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    d_1_0_Deprecated->Vertex4d(x, y, z, w);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertex3sv(const GLshort *v)
{
    d_1_0_Deprecated->Vertex3sv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertex3s(GLshort x, GLshort y, GLshort z)
{
    d_1_0_Deprecated->Vertex3s(x, y, z);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertex3iv(const GLint *v)
{
    d_1_0_Deprecated->Vertex3iv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertex3i(GLint x, GLint y, GLint z)
{
    d_1_0_Deprecated->Vertex3i(x, y, z);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertex3fv(const GLfloat *v)
{
    d_1_0_Deprecated->Vertex3fv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertex3f(GLfloat x, GLfloat y, GLfloat z)
{
    d_1_0_Deprecated->Vertex3f(x, y, z);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertex3dv(const GLdouble *v)
{
    d_1_0_Deprecated->Vertex3dv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertex3d(GLdouble x, GLdouble y, GLdouble z)
{
    d_1_0_Deprecated->Vertex3d(x, y, z);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertex2sv(const GLshort *v)
{
    d_1_0_Deprecated->Vertex2sv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertex2s(GLshort x, GLshort y)
{
    d_1_0_Deprecated->Vertex2s(x, y);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertex2iv(const GLint *v)
{
    d_1_0_Deprecated->Vertex2iv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertex2i(GLint x, GLint y)
{
    d_1_0_Deprecated->Vertex2i(x, y);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertex2fv(const GLfloat *v)
{
    d_1_0_Deprecated->Vertex2fv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertex2f(GLfloat x, GLfloat y)
{
    d_1_0_Deprecated->Vertex2f(x, y);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertex2dv(const GLdouble *v)
{
    d_1_0_Deprecated->Vertex2dv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertex2d(GLdouble x, GLdouble y)
{
    d_1_0_Deprecated->Vertex2d(x, y);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexCoord4sv(const GLshort *v)
{
    d_1_0_Deprecated->TexCoord4sv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexCoord4s(GLshort s, GLshort t, GLshort r, GLshort q)
{
    d_1_0_Deprecated->TexCoord4s(s, t, r, q);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexCoord4iv(const GLint *v)
{
    d_1_0_Deprecated->TexCoord4iv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexCoord4i(GLint s, GLint t, GLint r, GLint q)
{
    d_1_0_Deprecated->TexCoord4i(s, t, r, q);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexCoord4fv(const GLfloat *v)
{
    d_1_0_Deprecated->TexCoord4fv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexCoord4f(GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
    d_1_0_Deprecated->TexCoord4f(s, t, r, q);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexCoord4dv(const GLdouble *v)
{
    d_1_0_Deprecated->TexCoord4dv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexCoord4d(GLdouble s, GLdouble t, GLdouble r, GLdouble q)
{
    d_1_0_Deprecated->TexCoord4d(s, t, r, q);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexCoord3sv(const GLshort *v)
{
    d_1_0_Deprecated->TexCoord3sv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexCoord3s(GLshort s, GLshort t, GLshort r)
{
    d_1_0_Deprecated->TexCoord3s(s, t, r);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexCoord3iv(const GLint *v)
{
    d_1_0_Deprecated->TexCoord3iv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexCoord3i(GLint s, GLint t, GLint r)
{
    d_1_0_Deprecated->TexCoord3i(s, t, r);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexCoord3fv(const GLfloat *v)
{
    d_1_0_Deprecated->TexCoord3fv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexCoord3f(GLfloat s, GLfloat t, GLfloat r)
{
    d_1_0_Deprecated->TexCoord3f(s, t, r);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexCoord3dv(const GLdouble *v)
{
    d_1_0_Deprecated->TexCoord3dv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexCoord3d(GLdouble s, GLdouble t, GLdouble r)
{
    d_1_0_Deprecated->TexCoord3d(s, t, r);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexCoord2sv(const GLshort *v)
{
    d_1_0_Deprecated->TexCoord2sv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexCoord2s(GLshort s, GLshort t)
{
    d_1_0_Deprecated->TexCoord2s(s, t);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexCoord2iv(const GLint *v)
{
    d_1_0_Deprecated->TexCoord2iv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexCoord2i(GLint s, GLint t)
{
    d_1_0_Deprecated->TexCoord2i(s, t);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexCoord2fv(const GLfloat *v)
{
    d_1_0_Deprecated->TexCoord2fv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexCoord2f(GLfloat s, GLfloat t)
{
    d_1_0_Deprecated->TexCoord2f(s, t);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexCoord2dv(const GLdouble *v)
{
    d_1_0_Deprecated->TexCoord2dv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexCoord2d(GLdouble s, GLdouble t)
{
    d_1_0_Deprecated->TexCoord2d(s, t);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexCoord1sv(const GLshort *v)
{
    d_1_0_Deprecated->TexCoord1sv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexCoord1s(GLshort s)
{
    d_1_0_Deprecated->TexCoord1s(s);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexCoord1iv(const GLint *v)
{
    d_1_0_Deprecated->TexCoord1iv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexCoord1i(GLint s)
{
    d_1_0_Deprecated->TexCoord1i(s);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexCoord1fv(const GLfloat *v)
{
    d_1_0_Deprecated->TexCoord1fv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexCoord1f(GLfloat s)
{
    d_1_0_Deprecated->TexCoord1f(s);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexCoord1dv(const GLdouble *v)
{
    d_1_0_Deprecated->TexCoord1dv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexCoord1d(GLdouble s)
{
    d_1_0_Deprecated->TexCoord1d(s);
}

inline void QOpenGLFunctions_4_4_Compatibility::glRectsv(const GLshort *v1, const GLshort *v2)
{
    d_1_0_Deprecated->Rectsv(v1, v2);
}

inline void QOpenGLFunctions_4_4_Compatibility::glRects(GLshort x1, GLshort y1, GLshort x2, GLshort y2)
{
    d_1_0_Deprecated->Rects(x1, y1, x2, y2);
}

inline void QOpenGLFunctions_4_4_Compatibility::glRectiv(const GLint *v1, const GLint *v2)
{
    d_1_0_Deprecated->Rectiv(v1, v2);
}

inline void QOpenGLFunctions_4_4_Compatibility::glRecti(GLint x1, GLint y1, GLint x2, GLint y2)
{
    d_1_0_Deprecated->Recti(x1, y1, x2, y2);
}

inline void QOpenGLFunctions_4_4_Compatibility::glRectfv(const GLfloat *v1, const GLfloat *v2)
{
    d_1_0_Deprecated->Rectfv(v1, v2);
}

inline void QOpenGLFunctions_4_4_Compatibility::glRectf(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2)
{
    d_1_0_Deprecated->Rectf(x1, y1, x2, y2);
}

inline void QOpenGLFunctions_4_4_Compatibility::glRectdv(const GLdouble *v1, const GLdouble *v2)
{
    d_1_0_Deprecated->Rectdv(v1, v2);
}

inline void QOpenGLFunctions_4_4_Compatibility::glRectd(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2)
{
    d_1_0_Deprecated->Rectd(x1, y1, x2, y2);
}

inline void QOpenGLFunctions_4_4_Compatibility::glRasterPos4sv(const GLshort *v)
{
    d_1_0_Deprecated->RasterPos4sv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glRasterPos4s(GLshort x, GLshort y, GLshort z, GLshort w)
{
    d_1_0_Deprecated->RasterPos4s(x, y, z, w);
}

inline void QOpenGLFunctions_4_4_Compatibility::glRasterPos4iv(const GLint *v)
{
    d_1_0_Deprecated->RasterPos4iv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glRasterPos4i(GLint x, GLint y, GLint z, GLint w)
{
    d_1_0_Deprecated->RasterPos4i(x, y, z, w);
}

inline void QOpenGLFunctions_4_4_Compatibility::glRasterPos4fv(const GLfloat *v)
{
    d_1_0_Deprecated->RasterPos4fv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glRasterPos4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    d_1_0_Deprecated->RasterPos4f(x, y, z, w);
}

inline void QOpenGLFunctions_4_4_Compatibility::glRasterPos4dv(const GLdouble *v)
{
    d_1_0_Deprecated->RasterPos4dv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glRasterPos4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    d_1_0_Deprecated->RasterPos4d(x, y, z, w);
}

inline void QOpenGLFunctions_4_4_Compatibility::glRasterPos3sv(const GLshort *v)
{
    d_1_0_Deprecated->RasterPos3sv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glRasterPos3s(GLshort x, GLshort y, GLshort z)
{
    d_1_0_Deprecated->RasterPos3s(x, y, z);
}

inline void QOpenGLFunctions_4_4_Compatibility::glRasterPos3iv(const GLint *v)
{
    d_1_0_Deprecated->RasterPos3iv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glRasterPos3i(GLint x, GLint y, GLint z)
{
    d_1_0_Deprecated->RasterPos3i(x, y, z);
}

inline void QOpenGLFunctions_4_4_Compatibility::glRasterPos3fv(const GLfloat *v)
{
    d_1_0_Deprecated->RasterPos3fv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glRasterPos3f(GLfloat x, GLfloat y, GLfloat z)
{
    d_1_0_Deprecated->RasterPos3f(x, y, z);
}

inline void QOpenGLFunctions_4_4_Compatibility::glRasterPos3dv(const GLdouble *v)
{
    d_1_0_Deprecated->RasterPos3dv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glRasterPos3d(GLdouble x, GLdouble y, GLdouble z)
{
    d_1_0_Deprecated->RasterPos3d(x, y, z);
}

inline void QOpenGLFunctions_4_4_Compatibility::glRasterPos2sv(const GLshort *v)
{
    d_1_0_Deprecated->RasterPos2sv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glRasterPos2s(GLshort x, GLshort y)
{
    d_1_0_Deprecated->RasterPos2s(x, y);
}

inline void QOpenGLFunctions_4_4_Compatibility::glRasterPos2iv(const GLint *v)
{
    d_1_0_Deprecated->RasterPos2iv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glRasterPos2i(GLint x, GLint y)
{
    d_1_0_Deprecated->RasterPos2i(x, y);
}

inline void QOpenGLFunctions_4_4_Compatibility::glRasterPos2fv(const GLfloat *v)
{
    d_1_0_Deprecated->RasterPos2fv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glRasterPos2f(GLfloat x, GLfloat y)
{
    d_1_0_Deprecated->RasterPos2f(x, y);
}

inline void QOpenGLFunctions_4_4_Compatibility::glRasterPos2dv(const GLdouble *v)
{
    d_1_0_Deprecated->RasterPos2dv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glRasterPos2d(GLdouble x, GLdouble y)
{
    d_1_0_Deprecated->RasterPos2d(x, y);
}

inline void QOpenGLFunctions_4_4_Compatibility::glNormal3sv(const GLshort *v)
{
    d_1_0_Deprecated->Normal3sv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glNormal3s(GLshort nx, GLshort ny, GLshort nz)
{
    d_1_0_Deprecated->Normal3s(nx, ny, nz);
}

inline void QOpenGLFunctions_4_4_Compatibility::glNormal3iv(const GLint *v)
{
    d_1_0_Deprecated->Normal3iv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glNormal3i(GLint nx, GLint ny, GLint nz)
{
    d_1_0_Deprecated->Normal3i(nx, ny, nz);
}

inline void QOpenGLFunctions_4_4_Compatibility::glNormal3fv(const GLfloat *v)
{
    d_1_0_Deprecated->Normal3fv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz)
{
    d_1_0_Deprecated->Normal3f(nx, ny, nz);
}

inline void QOpenGLFunctions_4_4_Compatibility::glNormal3dv(const GLdouble *v)
{
    d_1_0_Deprecated->Normal3dv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glNormal3d(GLdouble nx, GLdouble ny, GLdouble nz)
{
    d_1_0_Deprecated->Normal3d(nx, ny, nz);
}

inline void QOpenGLFunctions_4_4_Compatibility::glNormal3bv(const GLbyte *v)
{
    d_1_0_Deprecated->Normal3bv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glNormal3b(GLbyte nx, GLbyte ny, GLbyte nz)
{
    d_1_0_Deprecated->Normal3b(nx, ny, nz);
}

inline void QOpenGLFunctions_4_4_Compatibility::glIndexsv(const GLshort *c)
{
    d_1_0_Deprecated->Indexsv(c);
}

inline void QOpenGLFunctions_4_4_Compatibility::glIndexs(GLshort c)
{
    d_1_0_Deprecated->Indexs(c);
}

inline void QOpenGLFunctions_4_4_Compatibility::glIndexiv(const GLint *c)
{
    d_1_0_Deprecated->Indexiv(c);
}

inline void QOpenGLFunctions_4_4_Compatibility::glIndexi(GLint c)
{
    d_1_0_Deprecated->Indexi(c);
}

inline void QOpenGLFunctions_4_4_Compatibility::glIndexfv(const GLfloat *c)
{
    d_1_0_Deprecated->Indexfv(c);
}

inline void QOpenGLFunctions_4_4_Compatibility::glIndexf(GLfloat c)
{
    d_1_0_Deprecated->Indexf(c);
}

inline void QOpenGLFunctions_4_4_Compatibility::glIndexdv(const GLdouble *c)
{
    d_1_0_Deprecated->Indexdv(c);
}

inline void QOpenGLFunctions_4_4_Compatibility::glIndexd(GLdouble c)
{
    d_1_0_Deprecated->Indexd(c);
}

inline void QOpenGLFunctions_4_4_Compatibility::glEnd()
{
    d_1_0_Deprecated->End();
}

inline void QOpenGLFunctions_4_4_Compatibility::glEdgeFlagv(const GLboolean *flag)
{
    d_1_0_Deprecated->EdgeFlagv(flag);
}

inline void QOpenGLFunctions_4_4_Compatibility::glEdgeFlag(GLboolean flag)
{
    d_1_0_Deprecated->EdgeFlag(flag);
}

inline void QOpenGLFunctions_4_4_Compatibility::glColor4usv(const GLushort *v)
{
    d_1_0_Deprecated->Color4usv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glColor4us(GLushort red, GLushort green, GLushort blue, GLushort alpha)
{
    d_1_0_Deprecated->Color4us(red, green, blue, alpha);
}

inline void QOpenGLFunctions_4_4_Compatibility::glColor4uiv(const GLuint *v)
{
    d_1_0_Deprecated->Color4uiv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glColor4ui(GLuint red, GLuint green, GLuint blue, GLuint alpha)
{
    d_1_0_Deprecated->Color4ui(red, green, blue, alpha);
}

inline void QOpenGLFunctions_4_4_Compatibility::glColor4ubv(const GLubyte *v)
{
    d_1_0_Deprecated->Color4ubv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glColor4ub(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha)
{
    d_1_0_Deprecated->Color4ub(red, green, blue, alpha);
}

inline void QOpenGLFunctions_4_4_Compatibility::glColor4sv(const GLshort *v)
{
    d_1_0_Deprecated->Color4sv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glColor4s(GLshort red, GLshort green, GLshort blue, GLshort alpha)
{
    d_1_0_Deprecated->Color4s(red, green, blue, alpha);
}

inline void QOpenGLFunctions_4_4_Compatibility::glColor4iv(const GLint *v)
{
    d_1_0_Deprecated->Color4iv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glColor4i(GLint red, GLint green, GLint blue, GLint alpha)
{
    d_1_0_Deprecated->Color4i(red, green, blue, alpha);
}

inline void QOpenGLFunctions_4_4_Compatibility::glColor4fv(const GLfloat *v)
{
    d_1_0_Deprecated->Color4fv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    d_1_0_Deprecated->Color4f(red, green, blue, alpha);
}

inline void QOpenGLFunctions_4_4_Compatibility::glColor4dv(const GLdouble *v)
{
    d_1_0_Deprecated->Color4dv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glColor4d(GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha)
{
    d_1_0_Deprecated->Color4d(red, green, blue, alpha);
}

inline void QOpenGLFunctions_4_4_Compatibility::glColor4bv(const GLbyte *v)
{
    d_1_0_Deprecated->Color4bv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glColor4b(GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha)
{
    d_1_0_Deprecated->Color4b(red, green, blue, alpha);
}

inline void QOpenGLFunctions_4_4_Compatibility::glColor3usv(const GLushort *v)
{
    d_1_0_Deprecated->Color3usv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glColor3us(GLushort red, GLushort green, GLushort blue)
{
    d_1_0_Deprecated->Color3us(red, green, blue);
}

inline void QOpenGLFunctions_4_4_Compatibility::glColor3uiv(const GLuint *v)
{
    d_1_0_Deprecated->Color3uiv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glColor3ui(GLuint red, GLuint green, GLuint blue)
{
    d_1_0_Deprecated->Color3ui(red, green, blue);
}

inline void QOpenGLFunctions_4_4_Compatibility::glColor3ubv(const GLubyte *v)
{
    d_1_0_Deprecated->Color3ubv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glColor3ub(GLubyte red, GLubyte green, GLubyte blue)
{
    d_1_0_Deprecated->Color3ub(red, green, blue);
}

inline void QOpenGLFunctions_4_4_Compatibility::glColor3sv(const GLshort *v)
{
    d_1_0_Deprecated->Color3sv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glColor3s(GLshort red, GLshort green, GLshort blue)
{
    d_1_0_Deprecated->Color3s(red, green, blue);
}

inline void QOpenGLFunctions_4_4_Compatibility::glColor3iv(const GLint *v)
{
    d_1_0_Deprecated->Color3iv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glColor3i(GLint red, GLint green, GLint blue)
{
    d_1_0_Deprecated->Color3i(red, green, blue);
}

inline void QOpenGLFunctions_4_4_Compatibility::glColor3fv(const GLfloat *v)
{
    d_1_0_Deprecated->Color3fv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glColor3f(GLfloat red, GLfloat green, GLfloat blue)
{
    d_1_0_Deprecated->Color3f(red, green, blue);
}

inline void QOpenGLFunctions_4_4_Compatibility::glColor3dv(const GLdouble *v)
{
    d_1_0_Deprecated->Color3dv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glColor3d(GLdouble red, GLdouble green, GLdouble blue)
{
    d_1_0_Deprecated->Color3d(red, green, blue);
}

inline void QOpenGLFunctions_4_4_Compatibility::glColor3bv(const GLbyte *v)
{
    d_1_0_Deprecated->Color3bv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glColor3b(GLbyte red, GLbyte green, GLbyte blue)
{
    d_1_0_Deprecated->Color3b(red, green, blue);
}

inline void QOpenGLFunctions_4_4_Compatibility::glBitmap(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap)
{
    d_1_0_Deprecated->Bitmap(width, height, xorig, yorig, xmove, ymove, bitmap);
}

inline void QOpenGLFunctions_4_4_Compatibility::glBegin(GLenum mode)
{
    d_1_0_Deprecated->Begin(mode);
}

inline void QOpenGLFunctions_4_4_Compatibility::glListBase(GLuint base)
{
    d_1_0_Deprecated->ListBase(base);
}

inline GLuint QOpenGLFunctions_4_4_Compatibility::glGenLists(GLsizei range)
{
    return d_1_0_Deprecated->GenLists(range);
}

inline void QOpenGLFunctions_4_4_Compatibility::glDeleteLists(GLuint list, GLsizei range)
{
    d_1_0_Deprecated->DeleteLists(list, range);
}

inline void QOpenGLFunctions_4_4_Compatibility::glCallLists(GLsizei n, GLenum type, const void *lists)
{
    d_1_0_Deprecated->CallLists(n, type, lists);
}

inline void QOpenGLFunctions_4_4_Compatibility::glCallList(GLuint list)
{
    d_1_0_Deprecated->CallList(list);
}

inline void QOpenGLFunctions_4_4_Compatibility::glEndList()
{
    d_1_0_Deprecated->EndList();
}

inline void QOpenGLFunctions_4_4_Compatibility::glNewList(GLuint list, GLenum mode)
{
    d_1_0_Deprecated->NewList(list, mode);
}


// OpenGL 1.1 deprecated functions
inline void QOpenGLFunctions_4_4_Compatibility::glPushClientAttrib(GLbitfield mask)
{
    d_1_1_Deprecated->PushClientAttrib(mask);
}

inline void QOpenGLFunctions_4_4_Compatibility::glPopClientAttrib()
{
    d_1_1_Deprecated->PopClientAttrib();
}

inline void QOpenGLFunctions_4_4_Compatibility::glIndexubv(const GLubyte *c)
{
    d_1_1_Deprecated->Indexubv(c);
}

inline void QOpenGLFunctions_4_4_Compatibility::glIndexub(GLubyte c)
{
    d_1_1_Deprecated->Indexub(c);
}

inline void QOpenGLFunctions_4_4_Compatibility::glPrioritizeTextures(GLsizei n, const GLuint *textures, const GLfloat *priorities)
{
    d_1_1_Deprecated->PrioritizeTextures(n, textures, priorities);
}

inline GLboolean QOpenGLFunctions_4_4_Compatibility::glAreTexturesResident(GLsizei n, const GLuint *textures, GLboolean *residences)
{
    return d_1_1_Deprecated->AreTexturesResident(n, textures, residences);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexPointer(GLint size, GLenum type, GLsizei stride, const void *pointer)
{
    d_1_1_Deprecated->VertexPointer(size, type, stride, pointer);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const void *pointer)
{
    d_1_1_Deprecated->TexCoordPointer(size, type, stride, pointer);
}

inline void QOpenGLFunctions_4_4_Compatibility::glNormalPointer(GLenum type, GLsizei stride, const void *pointer)
{
    d_1_1_Deprecated->NormalPointer(type, stride, pointer);
}

inline void QOpenGLFunctions_4_4_Compatibility::glInterleavedArrays(GLenum format, GLsizei stride, const void *pointer)
{
    d_1_1_Deprecated->InterleavedArrays(format, stride, pointer);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetPointerv(GLenum pname, void * *params)
{
    d_1_1_Deprecated->GetPointerv(pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glIndexPointer(GLenum type, GLsizei stride, const void *pointer)
{
    d_1_1_Deprecated->IndexPointer(type, stride, pointer);
}

inline void QOpenGLFunctions_4_4_Compatibility::glEnableClientState(GLenum array)
{
    d_1_1_Deprecated->EnableClientState(array);
}

inline void QOpenGLFunctions_4_4_Compatibility::glEdgeFlagPointer(GLsizei stride, const void *pointer)
{
    d_1_1_Deprecated->EdgeFlagPointer(stride, pointer);
}

inline void QOpenGLFunctions_4_4_Compatibility::glDisableClientState(GLenum array)
{
    d_1_1_Deprecated->DisableClientState(array);
}

inline void QOpenGLFunctions_4_4_Compatibility::glColorPointer(GLint size, GLenum type, GLsizei stride, const void *pointer)
{
    d_1_1_Deprecated->ColorPointer(size, type, stride, pointer);
}

inline void QOpenGLFunctions_4_4_Compatibility::glArrayElement(GLint i)
{
    d_1_1_Deprecated->ArrayElement(i);
}


// OpenGL 1.2 deprecated functions
inline void QOpenGLFunctions_4_4_Compatibility::glColorTable(GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const void *table)
{
    d_1_2_Deprecated->ColorTable(target, internalformat, width, format, type, table);
}

inline void QOpenGLFunctions_4_4_Compatibility::glColorTableParameterfv(GLenum target, GLenum pname, const GLfloat *params)
{
    d_1_2_Deprecated->ColorTableParameterfv(target, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glColorTableParameteriv(GLenum target, GLenum pname, const GLint *params)
{
    d_1_2_Deprecated->ColorTableParameteriv(target, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glCopyColorTable(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width)
{
    d_1_2_Deprecated->CopyColorTable(target, internalformat, x, y, width);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetColorTable(GLenum target, GLenum format, GLenum type, void *table)
{
    d_1_2_Deprecated->GetColorTable(target, format, type, table);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetColorTableParameterfv(GLenum target, GLenum pname, GLfloat *params)
{
    d_1_2_Deprecated->GetColorTableParameterfv(target, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetColorTableParameteriv(GLenum target, GLenum pname, GLint *params)
{
    d_1_2_Deprecated->GetColorTableParameteriv(target, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glColorSubTable(GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const void *data)
{
    d_1_2_Deprecated->ColorSubTable(target, start, count, format, type, data);
}

inline void QOpenGLFunctions_4_4_Compatibility::glCopyColorSubTable(GLenum target, GLsizei start, GLint x, GLint y, GLsizei width)
{
    d_1_2_Deprecated->CopyColorSubTable(target, start, x, y, width);
}

inline void QOpenGLFunctions_4_4_Compatibility::glConvolutionFilter1D(GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const void *image)
{
    d_1_2_Deprecated->ConvolutionFilter1D(target, internalformat, width, format, type, image);
}

inline void QOpenGLFunctions_4_4_Compatibility::glConvolutionFilter2D(GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *image)
{
    d_1_2_Deprecated->ConvolutionFilter2D(target, internalformat, width, height, format, type, image);
}

inline void QOpenGLFunctions_4_4_Compatibility::glConvolutionParameterf(GLenum target, GLenum pname, GLfloat params)
{
    d_1_2_Deprecated->ConvolutionParameterf(target, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glConvolutionParameterfv(GLenum target, GLenum pname, const GLfloat *params)
{
    d_1_2_Deprecated->ConvolutionParameterfv(target, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glConvolutionParameteri(GLenum target, GLenum pname, GLint params)
{
    d_1_2_Deprecated->ConvolutionParameteri(target, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glConvolutionParameteriv(GLenum target, GLenum pname, const GLint *params)
{
    d_1_2_Deprecated->ConvolutionParameteriv(target, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glCopyConvolutionFilter1D(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width)
{
    d_1_2_Deprecated->CopyConvolutionFilter1D(target, internalformat, x, y, width);
}

inline void QOpenGLFunctions_4_4_Compatibility::glCopyConvolutionFilter2D(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height)
{
    d_1_2_Deprecated->CopyConvolutionFilter2D(target, internalformat, x, y, width, height);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetConvolutionFilter(GLenum target, GLenum format, GLenum type, void *image)
{
    d_1_2_Deprecated->GetConvolutionFilter(target, format, type, image);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetConvolutionParameterfv(GLenum target, GLenum pname, GLfloat *params)
{
    d_1_2_Deprecated->GetConvolutionParameterfv(target, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetConvolutionParameteriv(GLenum target, GLenum pname, GLint *params)
{
    d_1_2_Deprecated->GetConvolutionParameteriv(target, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetSeparableFilter(GLenum target, GLenum format, GLenum type, void *row, void *column, void *span)
{
    d_1_2_Deprecated->GetSeparableFilter(target, format, type, row, column, span);
}

inline void QOpenGLFunctions_4_4_Compatibility::glSeparableFilter2D(GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *row, const void *column)
{
    d_1_2_Deprecated->SeparableFilter2D(target, internalformat, width, height, format, type, row, column);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetHistogram(GLenum target, GLboolean reset, GLenum format, GLenum type, void *values)
{
    d_1_2_Deprecated->GetHistogram(target, reset, format, type, values);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetHistogramParameterfv(GLenum target, GLenum pname, GLfloat *params)
{
    d_1_2_Deprecated->GetHistogramParameterfv(target, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetHistogramParameteriv(GLenum target, GLenum pname, GLint *params)
{
    d_1_2_Deprecated->GetHistogramParameteriv(target, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetMinmax(GLenum target, GLboolean reset, GLenum format, GLenum type, void *values)
{
    d_1_2_Deprecated->GetMinmax(target, reset, format, type, values);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetMinmaxParameterfv(GLenum target, GLenum pname, GLfloat *params)
{
    d_1_2_Deprecated->GetMinmaxParameterfv(target, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glGetMinmaxParameteriv(GLenum target, GLenum pname, GLint *params)
{
    d_1_2_Deprecated->GetMinmaxParameteriv(target, pname, params);
}

inline void QOpenGLFunctions_4_4_Compatibility::glHistogram(GLenum target, GLsizei width, GLenum internalformat, GLboolean sink)
{
    d_1_2_Deprecated->Histogram(target, width, internalformat, sink);
}

inline void QOpenGLFunctions_4_4_Compatibility::glMinmax(GLenum target, GLenum internalformat, GLboolean sink)
{
    d_1_2_Deprecated->Minmax(target, internalformat, sink);
}

inline void QOpenGLFunctions_4_4_Compatibility::glResetHistogram(GLenum target)
{
    d_1_2_Deprecated->ResetHistogram(target);
}

inline void QOpenGLFunctions_4_4_Compatibility::glResetMinmax(GLenum target)
{
    d_1_2_Deprecated->ResetMinmax(target);
}


// OpenGL 1.3 deprecated functions
inline void QOpenGLFunctions_4_4_Compatibility::glMultTransposeMatrixd(const GLdouble *m)
{
    d_1_3_Deprecated->MultTransposeMatrixd(m);
}

inline void QOpenGLFunctions_4_4_Compatibility::glMultTransposeMatrixf(const GLfloat *m)
{
    d_1_3_Deprecated->MultTransposeMatrixf(m);
}

inline void QOpenGLFunctions_4_4_Compatibility::glLoadTransposeMatrixd(const GLdouble *m)
{
    d_1_3_Deprecated->LoadTransposeMatrixd(m);
}

inline void QOpenGLFunctions_4_4_Compatibility::glLoadTransposeMatrixf(const GLfloat *m)
{
    d_1_3_Deprecated->LoadTransposeMatrixf(m);
}

inline void QOpenGLFunctions_4_4_Compatibility::glMultiTexCoord4sv(GLenum target, const GLshort *v)
{
    d_1_3_Deprecated->MultiTexCoord4sv(target, v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glMultiTexCoord4s(GLenum target, GLshort s, GLshort t, GLshort r, GLshort q)
{
    d_1_3_Deprecated->MultiTexCoord4s(target, s, t, r, q);
}

inline void QOpenGLFunctions_4_4_Compatibility::glMultiTexCoord4iv(GLenum target, const GLint *v)
{
    d_1_3_Deprecated->MultiTexCoord4iv(target, v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glMultiTexCoord4i(GLenum target, GLint s, GLint t, GLint r, GLint q)
{
    d_1_3_Deprecated->MultiTexCoord4i(target, s, t, r, q);
}

inline void QOpenGLFunctions_4_4_Compatibility::glMultiTexCoord4fv(GLenum target, const GLfloat *v)
{
    d_1_3_Deprecated->MultiTexCoord4fv(target, v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glMultiTexCoord4f(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
    d_1_3_Deprecated->MultiTexCoord4f(target, s, t, r, q);
}

inline void QOpenGLFunctions_4_4_Compatibility::glMultiTexCoord4dv(GLenum target, const GLdouble *v)
{
    d_1_3_Deprecated->MultiTexCoord4dv(target, v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glMultiTexCoord4d(GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q)
{
    d_1_3_Deprecated->MultiTexCoord4d(target, s, t, r, q);
}

inline void QOpenGLFunctions_4_4_Compatibility::glMultiTexCoord3sv(GLenum target, const GLshort *v)
{
    d_1_3_Deprecated->MultiTexCoord3sv(target, v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glMultiTexCoord3s(GLenum target, GLshort s, GLshort t, GLshort r)
{
    d_1_3_Deprecated->MultiTexCoord3s(target, s, t, r);
}

inline void QOpenGLFunctions_4_4_Compatibility::glMultiTexCoord3iv(GLenum target, const GLint *v)
{
    d_1_3_Deprecated->MultiTexCoord3iv(target, v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glMultiTexCoord3i(GLenum target, GLint s, GLint t, GLint r)
{
    d_1_3_Deprecated->MultiTexCoord3i(target, s, t, r);
}

inline void QOpenGLFunctions_4_4_Compatibility::glMultiTexCoord3fv(GLenum target, const GLfloat *v)
{
    d_1_3_Deprecated->MultiTexCoord3fv(target, v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glMultiTexCoord3f(GLenum target, GLfloat s, GLfloat t, GLfloat r)
{
    d_1_3_Deprecated->MultiTexCoord3f(target, s, t, r);
}

inline void QOpenGLFunctions_4_4_Compatibility::glMultiTexCoord3dv(GLenum target, const GLdouble *v)
{
    d_1_3_Deprecated->MultiTexCoord3dv(target, v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glMultiTexCoord3d(GLenum target, GLdouble s, GLdouble t, GLdouble r)
{
    d_1_3_Deprecated->MultiTexCoord3d(target, s, t, r);
}

inline void QOpenGLFunctions_4_4_Compatibility::glMultiTexCoord2sv(GLenum target, const GLshort *v)
{
    d_1_3_Deprecated->MultiTexCoord2sv(target, v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glMultiTexCoord2s(GLenum target, GLshort s, GLshort t)
{
    d_1_3_Deprecated->MultiTexCoord2s(target, s, t);
}

inline void QOpenGLFunctions_4_4_Compatibility::glMultiTexCoord2iv(GLenum target, const GLint *v)
{
    d_1_3_Deprecated->MultiTexCoord2iv(target, v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glMultiTexCoord2i(GLenum target, GLint s, GLint t)
{
    d_1_3_Deprecated->MultiTexCoord2i(target, s, t);
}

inline void QOpenGLFunctions_4_4_Compatibility::glMultiTexCoord2fv(GLenum target, const GLfloat *v)
{
    d_1_3_Deprecated->MultiTexCoord2fv(target, v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glMultiTexCoord2f(GLenum target, GLfloat s, GLfloat t)
{
    d_1_3_Deprecated->MultiTexCoord2f(target, s, t);
}

inline void QOpenGLFunctions_4_4_Compatibility::glMultiTexCoord2dv(GLenum target, const GLdouble *v)
{
    d_1_3_Deprecated->MultiTexCoord2dv(target, v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glMultiTexCoord2d(GLenum target, GLdouble s, GLdouble t)
{
    d_1_3_Deprecated->MultiTexCoord2d(target, s, t);
}

inline void QOpenGLFunctions_4_4_Compatibility::glMultiTexCoord1sv(GLenum target, const GLshort *v)
{
    d_1_3_Deprecated->MultiTexCoord1sv(target, v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glMultiTexCoord1s(GLenum target, GLshort s)
{
    d_1_3_Deprecated->MultiTexCoord1s(target, s);
}

inline void QOpenGLFunctions_4_4_Compatibility::glMultiTexCoord1iv(GLenum target, const GLint *v)
{
    d_1_3_Deprecated->MultiTexCoord1iv(target, v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glMultiTexCoord1i(GLenum target, GLint s)
{
    d_1_3_Deprecated->MultiTexCoord1i(target, s);
}

inline void QOpenGLFunctions_4_4_Compatibility::glMultiTexCoord1fv(GLenum target, const GLfloat *v)
{
    d_1_3_Deprecated->MultiTexCoord1fv(target, v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glMultiTexCoord1f(GLenum target, GLfloat s)
{
    d_1_3_Deprecated->MultiTexCoord1f(target, s);
}

inline void QOpenGLFunctions_4_4_Compatibility::glMultiTexCoord1dv(GLenum target, const GLdouble *v)
{
    d_1_3_Deprecated->MultiTexCoord1dv(target, v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glMultiTexCoord1d(GLenum target, GLdouble s)
{
    d_1_3_Deprecated->MultiTexCoord1d(target, s);
}

inline void QOpenGLFunctions_4_4_Compatibility::glClientActiveTexture(GLenum texture)
{
    d_1_3_Deprecated->ClientActiveTexture(texture);
}


// OpenGL 1.4 deprecated functions
inline void QOpenGLFunctions_4_4_Compatibility::glWindowPos3sv(const GLshort *v)
{
    d_1_4_Deprecated->WindowPos3sv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glWindowPos3s(GLshort x, GLshort y, GLshort z)
{
    d_1_4_Deprecated->WindowPos3s(x, y, z);
}

inline void QOpenGLFunctions_4_4_Compatibility::glWindowPos3iv(const GLint *v)
{
    d_1_4_Deprecated->WindowPos3iv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glWindowPos3i(GLint x, GLint y, GLint z)
{
    d_1_4_Deprecated->WindowPos3i(x, y, z);
}

inline void QOpenGLFunctions_4_4_Compatibility::glWindowPos3fv(const GLfloat *v)
{
    d_1_4_Deprecated->WindowPos3fv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glWindowPos3f(GLfloat x, GLfloat y, GLfloat z)
{
    d_1_4_Deprecated->WindowPos3f(x, y, z);
}

inline void QOpenGLFunctions_4_4_Compatibility::glWindowPos3dv(const GLdouble *v)
{
    d_1_4_Deprecated->WindowPos3dv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glWindowPos3d(GLdouble x, GLdouble y, GLdouble z)
{
    d_1_4_Deprecated->WindowPos3d(x, y, z);
}

inline void QOpenGLFunctions_4_4_Compatibility::glWindowPos2sv(const GLshort *v)
{
    d_1_4_Deprecated->WindowPos2sv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glWindowPos2s(GLshort x, GLshort y)
{
    d_1_4_Deprecated->WindowPos2s(x, y);
}

inline void QOpenGLFunctions_4_4_Compatibility::glWindowPos2iv(const GLint *v)
{
    d_1_4_Deprecated->WindowPos2iv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glWindowPos2i(GLint x, GLint y)
{
    d_1_4_Deprecated->WindowPos2i(x, y);
}

inline void QOpenGLFunctions_4_4_Compatibility::glWindowPos2fv(const GLfloat *v)
{
    d_1_4_Deprecated->WindowPos2fv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glWindowPos2f(GLfloat x, GLfloat y)
{
    d_1_4_Deprecated->WindowPos2f(x, y);
}

inline void QOpenGLFunctions_4_4_Compatibility::glWindowPos2dv(const GLdouble *v)
{
    d_1_4_Deprecated->WindowPos2dv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glWindowPos2d(GLdouble x, GLdouble y)
{
    d_1_4_Deprecated->WindowPos2d(x, y);
}

inline void QOpenGLFunctions_4_4_Compatibility::glSecondaryColorPointer(GLint size, GLenum type, GLsizei stride, const void *pointer)
{
    d_1_4_Deprecated->SecondaryColorPointer(size, type, stride, pointer);
}

inline void QOpenGLFunctions_4_4_Compatibility::glSecondaryColor3usv(const GLushort *v)
{
    d_1_4_Deprecated->SecondaryColor3usv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glSecondaryColor3us(GLushort red, GLushort green, GLushort blue)
{
    d_1_4_Deprecated->SecondaryColor3us(red, green, blue);
}

inline void QOpenGLFunctions_4_4_Compatibility::glSecondaryColor3uiv(const GLuint *v)
{
    d_1_4_Deprecated->SecondaryColor3uiv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glSecondaryColor3ui(GLuint red, GLuint green, GLuint blue)
{
    d_1_4_Deprecated->SecondaryColor3ui(red, green, blue);
}

inline void QOpenGLFunctions_4_4_Compatibility::glSecondaryColor3ubv(const GLubyte *v)
{
    d_1_4_Deprecated->SecondaryColor3ubv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glSecondaryColor3ub(GLubyte red, GLubyte green, GLubyte blue)
{
    d_1_4_Deprecated->SecondaryColor3ub(red, green, blue);
}

inline void QOpenGLFunctions_4_4_Compatibility::glSecondaryColor3sv(const GLshort *v)
{
    d_1_4_Deprecated->SecondaryColor3sv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glSecondaryColor3s(GLshort red, GLshort green, GLshort blue)
{
    d_1_4_Deprecated->SecondaryColor3s(red, green, blue);
}

inline void QOpenGLFunctions_4_4_Compatibility::glSecondaryColor3iv(const GLint *v)
{
    d_1_4_Deprecated->SecondaryColor3iv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glSecondaryColor3i(GLint red, GLint green, GLint blue)
{
    d_1_4_Deprecated->SecondaryColor3i(red, green, blue);
}

inline void QOpenGLFunctions_4_4_Compatibility::glSecondaryColor3fv(const GLfloat *v)
{
    d_1_4_Deprecated->SecondaryColor3fv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glSecondaryColor3f(GLfloat red, GLfloat green, GLfloat blue)
{
    d_1_4_Deprecated->SecondaryColor3f(red, green, blue);
}

inline void QOpenGLFunctions_4_4_Compatibility::glSecondaryColor3dv(const GLdouble *v)
{
    d_1_4_Deprecated->SecondaryColor3dv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glSecondaryColor3d(GLdouble red, GLdouble green, GLdouble blue)
{
    d_1_4_Deprecated->SecondaryColor3d(red, green, blue);
}

inline void QOpenGLFunctions_4_4_Compatibility::glSecondaryColor3bv(const GLbyte *v)
{
    d_1_4_Deprecated->SecondaryColor3bv(v);
}

inline void QOpenGLFunctions_4_4_Compatibility::glSecondaryColor3b(GLbyte red, GLbyte green, GLbyte blue)
{
    d_1_4_Deprecated->SecondaryColor3b(red, green, blue);
}

inline void QOpenGLFunctions_4_4_Compatibility::glFogCoordPointer(GLenum type, GLsizei stride, const void *pointer)
{
    d_1_4_Deprecated->FogCoordPointer(type, stride, pointer);
}

inline void QOpenGLFunctions_4_4_Compatibility::glFogCoorddv(const GLdouble *coord)
{
    d_1_4_Deprecated->FogCoorddv(coord);
}

inline void QOpenGLFunctions_4_4_Compatibility::glFogCoordd(GLdouble coord)
{
    d_1_4_Deprecated->FogCoordd(coord);
}

inline void QOpenGLFunctions_4_4_Compatibility::glFogCoordfv(const GLfloat *coord)
{
    d_1_4_Deprecated->FogCoordfv(coord);
}

inline void QOpenGLFunctions_4_4_Compatibility::glFogCoordf(GLfloat coord)
{
    d_1_4_Deprecated->FogCoordf(coord);
}


// OpenGL 1.5 deprecated functions

// OpenGL 2.0 deprecated functions

// OpenGL 2.1 deprecated functions

// OpenGL 3.0 deprecated functions

// OpenGL 3.1 deprecated functions

// OpenGL 3.2 deprecated functions

// OpenGL 3.3 deprecated functions
inline void QOpenGLFunctions_4_4_Compatibility::glSecondaryColorP3uiv(GLenum type, const GLuint *color)
{
    d_3_3_Deprecated->SecondaryColorP3uiv(type, color);
}

inline void QOpenGLFunctions_4_4_Compatibility::glSecondaryColorP3ui(GLenum type, GLuint color)
{
    d_3_3_Deprecated->SecondaryColorP3ui(type, color);
}

inline void QOpenGLFunctions_4_4_Compatibility::glColorP4uiv(GLenum type, const GLuint *color)
{
    d_3_3_Deprecated->ColorP4uiv(type, color);
}

inline void QOpenGLFunctions_4_4_Compatibility::glColorP4ui(GLenum type, GLuint color)
{
    d_3_3_Deprecated->ColorP4ui(type, color);
}

inline void QOpenGLFunctions_4_4_Compatibility::glColorP3uiv(GLenum type, const GLuint *color)
{
    d_3_3_Deprecated->ColorP3uiv(type, color);
}

inline void QOpenGLFunctions_4_4_Compatibility::glColorP3ui(GLenum type, GLuint color)
{
    d_3_3_Deprecated->ColorP3ui(type, color);
}

inline void QOpenGLFunctions_4_4_Compatibility::glNormalP3uiv(GLenum type, const GLuint *coords)
{
    d_3_3_Deprecated->NormalP3uiv(type, coords);
}

inline void QOpenGLFunctions_4_4_Compatibility::glNormalP3ui(GLenum type, GLuint coords)
{
    d_3_3_Deprecated->NormalP3ui(type, coords);
}

inline void QOpenGLFunctions_4_4_Compatibility::glMultiTexCoordP4uiv(GLenum texture, GLenum type, const GLuint *coords)
{
    d_3_3_Deprecated->MultiTexCoordP4uiv(texture, type, coords);
}

inline void QOpenGLFunctions_4_4_Compatibility::glMultiTexCoordP4ui(GLenum texture, GLenum type, GLuint coords)
{
    d_3_3_Deprecated->MultiTexCoordP4ui(texture, type, coords);
}

inline void QOpenGLFunctions_4_4_Compatibility::glMultiTexCoordP3uiv(GLenum texture, GLenum type, const GLuint *coords)
{
    d_3_3_Deprecated->MultiTexCoordP3uiv(texture, type, coords);
}

inline void QOpenGLFunctions_4_4_Compatibility::glMultiTexCoordP3ui(GLenum texture, GLenum type, GLuint coords)
{
    d_3_3_Deprecated->MultiTexCoordP3ui(texture, type, coords);
}

inline void QOpenGLFunctions_4_4_Compatibility::glMultiTexCoordP2uiv(GLenum texture, GLenum type, const GLuint *coords)
{
    d_3_3_Deprecated->MultiTexCoordP2uiv(texture, type, coords);
}

inline void QOpenGLFunctions_4_4_Compatibility::glMultiTexCoordP2ui(GLenum texture, GLenum type, GLuint coords)
{
    d_3_3_Deprecated->MultiTexCoordP2ui(texture, type, coords);
}

inline void QOpenGLFunctions_4_4_Compatibility::glMultiTexCoordP1uiv(GLenum texture, GLenum type, const GLuint *coords)
{
    d_3_3_Deprecated->MultiTexCoordP1uiv(texture, type, coords);
}

inline void QOpenGLFunctions_4_4_Compatibility::glMultiTexCoordP1ui(GLenum texture, GLenum type, GLuint coords)
{
    d_3_3_Deprecated->MultiTexCoordP1ui(texture, type, coords);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexCoordP4uiv(GLenum type, const GLuint *coords)
{
    d_3_3_Deprecated->TexCoordP4uiv(type, coords);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexCoordP4ui(GLenum type, GLuint coords)
{
    d_3_3_Deprecated->TexCoordP4ui(type, coords);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexCoordP3uiv(GLenum type, const GLuint *coords)
{
    d_3_3_Deprecated->TexCoordP3uiv(type, coords);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexCoordP3ui(GLenum type, GLuint coords)
{
    d_3_3_Deprecated->TexCoordP3ui(type, coords);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexCoordP2uiv(GLenum type, const GLuint *coords)
{
    d_3_3_Deprecated->TexCoordP2uiv(type, coords);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexCoordP2ui(GLenum type, GLuint coords)
{
    d_3_3_Deprecated->TexCoordP2ui(type, coords);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexCoordP1uiv(GLenum type, const GLuint *coords)
{
    d_3_3_Deprecated->TexCoordP1uiv(type, coords);
}

inline void QOpenGLFunctions_4_4_Compatibility::glTexCoordP1ui(GLenum type, GLuint coords)
{
    d_3_3_Deprecated->TexCoordP1ui(type, coords);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexP4uiv(GLenum type, const GLuint *value)
{
    d_3_3_Deprecated->VertexP4uiv(type, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexP4ui(GLenum type, GLuint value)
{
    d_3_3_Deprecated->VertexP4ui(type, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexP3uiv(GLenum type, const GLuint *value)
{
    d_3_3_Deprecated->VertexP3uiv(type, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexP3ui(GLenum type, GLuint value)
{
    d_3_3_Deprecated->VertexP3ui(type, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexP2uiv(GLenum type, const GLuint *value)
{
    d_3_3_Deprecated->VertexP2uiv(type, value);
}

inline void QOpenGLFunctions_4_4_Compatibility::glVertexP2ui(GLenum type, GLuint value)
{
    d_3_3_Deprecated->VertexP2ui(type, value);
}


// OpenGL 4.0 deprecated functions

// OpenGL 4.1 deprecated functions

// OpenGL 4.2 deprecated functions

// OpenGL 4.3 deprecated functions

// OpenGL 4.4 deprecated functions


QT_END_NAMESPACE

#endif // QT_NO_OPENGL && !QT_OPENGL_ES_2

#endif
