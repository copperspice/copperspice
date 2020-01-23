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

#ifndef QOPENGLVERSIONFUNCTIONS_4_5_CORE_H
#define QOPENGLVERSIONFUNCTIONS_4_5_CORE_H

#include <QtCore/qglobal.h>

#if !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2)

#include <QtGui/QOpenGLVersionFunctions>
#include <QtGui/qopenglcontext.h>

QT_BEGIN_NAMESPACE

class Q_GUI_EXPORT QOpenGLFunctions_4_5_Core : public QAbstractOpenGLFunctions
{
public:
    QOpenGLFunctions_4_5_Core();
    ~QOpenGLFunctions_4_5_Core();

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

    // OpenGL 4.5 core functions
    void glTextureBarrier();
    void glReadnPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, void *data);
    void glGetnUniformuiv(GLuint program, GLint location, GLsizei bufSize, GLuint *params);
    void glGetnUniformiv(GLuint program, GLint location, GLsizei bufSize, GLint *params);
    void glGetnUniformfv(GLuint program, GLint location, GLsizei bufSize, GLfloat *params);
    void glGetnUniformdv(GLuint program, GLint location, GLsizei bufSize, GLdouble *params);
    void glGetnTexImage(GLenum target, GLint level, GLenum format, GLenum type, GLsizei bufSize, void *pixels);
    void glGetnCompressedTexImage(GLenum target, GLint lod, GLsizei bufSize, void *pixels);
    GLenum glGetGraphicsResetStatus();
    void glGetCompressedTextureSubImage(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLsizei bufSize, void *pixels);
    void glGetTextureSubImage(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, GLsizei bufSize, void *pixels);
    void glMemoryBarrierByRegion(GLbitfield barriers);
    void glCreateQueries(GLenum target, GLsizei n, GLuint *ids);
    void glCreateProgramPipelines(GLsizei n, GLuint *pipelines);
    void glCreateSamplers(GLsizei n, GLuint *samplers);
    void glGetVertexArrayIndexed64iv(GLuint vaobj, GLuint index, GLenum pname, GLint64 *param);
    void glGetVertexArrayIndexediv(GLuint vaobj, GLuint index, GLenum pname, GLint *param);
    void glGetVertexArrayiv(GLuint vaobj, GLenum pname, GLint *param);
    void glVertexArrayBindingDivisor(GLuint vaobj, GLuint bindingindex, GLuint divisor);
    void glVertexArrayAttribLFormat(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
    void glVertexArrayAttribIFormat(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset);
    void glVertexArrayAttribFormat(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset);
    void glVertexArrayAttribBinding(GLuint vaobj, GLuint attribindex, GLuint bindingindex);
    void glVertexArrayVertexBuffers(GLuint vaobj, GLuint first, GLsizei count, const GLuint *buffers, const GLintptr *offsets, const GLsizei *strides);
    void glVertexArrayVertexBuffer(GLuint vaobj, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride);
    void glVertexArrayElementBuffer(GLuint vaobj, GLuint buffer);
    void glEnableVertexArrayAttrib(GLuint vaobj, GLuint index);
    void glDisableVertexArrayAttrib(GLuint vaobj, GLuint index);
    void glCreateVertexArrays(GLsizei n, GLuint *arrays);
    void glGetTextureParameteriv(GLuint texture, GLenum pname, GLint *params);
    void glGetTextureParameterIuiv(GLuint texture, GLenum pname, GLuint *params);
    void glGetTextureParameterIiv(GLuint texture, GLenum pname, GLint *params);
    void glGetTextureParameterfv(GLuint texture, GLenum pname, GLfloat *params);
    void glGetTextureLevelParameteriv(GLuint texture, GLint level, GLenum pname, GLint *params);
    void glGetTextureLevelParameterfv(GLuint texture, GLint level, GLenum pname, GLfloat *params);
    void glGetCompressedTextureImage(GLuint texture, GLint level, GLsizei bufSize, void *pixels);
    void glGetTextureImage(GLuint texture, GLint level, GLenum format, GLenum type, GLsizei bufSize, void *pixels);
    void glBindTextureUnit(GLuint unit, GLuint texture);
    void glGenerateTextureMipmap(GLuint texture);
    void glTextureParameteriv(GLuint texture, GLenum pname, const GLint *param);
    void glTextureParameterIuiv(GLuint texture, GLenum pname, const GLuint *params);
    void glTextureParameterIiv(GLuint texture, GLenum pname, const GLint *params);
    void glTextureParameteri(GLuint texture, GLenum pname, GLint param);
    void glTextureParameterfv(GLuint texture, GLenum pname, const GLfloat *param);
    void glTextureParameterf(GLuint texture, GLenum pname, GLfloat param);
    void glCopyTextureSubImage3D(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
    void glCopyTextureSubImage2D(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
    void glCopyTextureSubImage1D(GLuint texture, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
    void glCompressedTextureSubImage3D(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data);
    void glCompressedTextureSubImage2D(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data);
    void glCompressedTextureSubImage1D(GLuint texture, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void *data);
    void glTextureSubImage3D(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels);
    void glTextureSubImage2D(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
    void glTextureSubImage1D(GLuint texture, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void *pixels);
    void glTextureStorage3DMultisample(GLuint texture, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations);
    void glTextureStorage2DMultisample(GLuint texture, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
    void glTextureStorage3D(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
    void glTextureStorage2D(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
    void glTextureStorage1D(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width);
    void glTextureBufferRange(GLuint texture, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizei size);
    void glTextureBuffer(GLuint texture, GLenum internalformat, GLuint buffer);
    void glCreateTextures(GLenum target, GLsizei n, GLuint *textures);
    void glGetNamedRenderbufferParameteriv(GLuint renderbuffer, GLenum pname, GLint *params);
    void glNamedRenderbufferStorageMultisample(GLuint renderbuffer, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
    void glNamedRenderbufferStorage(GLuint renderbuffer, GLenum internalformat, GLsizei width, GLsizei height);
    void glCreateRenderbuffers(GLsizei n, GLuint *renderbuffers);
    void glGetNamedFramebufferAttachmentParameteriv(GLuint framebuffer, GLenum attachment, GLenum pname, GLint *params);
    void glGetNamedFramebufferParameteriv(GLuint framebuffer, GLenum pname, GLint *param);
    GLenum glCheckNamedFramebufferStatus(GLuint framebuffer, GLenum target);
    void glBlitNamedFramebuffer(GLuint readFramebuffer, GLuint drawFramebuffer, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
    void glClearNamedFramebufferfi(GLuint framebuffer, GLenum buffer, GLfloat depth, GLint stencil);
    void glClearNamedFramebufferfv(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLfloat *value);
    void glClearNamedFramebufferuiv(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLuint *value);
    void glClearNamedFramebufferiv(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLint *value);
    void glInvalidateNamedFramebufferSubData(GLuint framebuffer, GLsizei numAttachments, const GLenum *attachments, GLint x, GLint y, GLsizei width, GLsizei height);
    void glInvalidateNamedFramebufferData(GLuint framebuffer, GLsizei numAttachments, const GLenum *attachments);
    void glNamedFramebufferReadBuffer(GLuint framebuffer, GLenum src);
    void glNamedFramebufferDrawBuffers(GLuint framebuffer, GLsizei n, const GLenum *bufs);
    void glNamedFramebufferDrawBuffer(GLuint framebuffer, GLenum buf);
    void glNamedFramebufferTextureLayer(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level, GLint layer);
    void glNamedFramebufferTexture(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level);
    void glNamedFramebufferParameteri(GLuint framebuffer, GLenum pname, GLint param);
    void glNamedFramebufferRenderbuffer(GLuint framebuffer, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
    void glCreateFramebuffers(GLsizei n, GLuint *framebuffers);
    void glGetNamedBufferSubData(GLuint buffer, GLintptr offset, GLsizei size, void *data);
    void glGetNamedBufferPointerv(GLuint buffer, GLenum pname, void * *params);
    void glGetNamedBufferParameteri64v(GLuint buffer, GLenum pname, GLint64 *params);
    void glGetNamedBufferParameteriv(GLuint buffer, GLenum pname, GLint *params);
    void glFlushMappedNamedBufferRange(GLuint buffer, GLintptr offset, GLsizei length);
    GLboolean glUnmapNamedBuffer(GLuint buffer);
    void * glMapNamedBufferRange(GLuint buffer, GLintptr offset, GLsizei length, GLbitfield access);
    void * glMapNamedBuffer(GLuint buffer, GLenum access);
    void glClearNamedBufferSubData(GLuint buffer, GLenum internalformat, GLintptr offset, GLsizei size, GLenum format, GLenum type, const void *data);
    void glClearNamedBufferData(GLuint buffer, GLenum internalformat, GLenum format, GLenum type, const void *data);
    void glCopyNamedBufferSubData(GLuint readBuffer, GLuint writeBuffer, GLintptr readOffset, GLintptr writeOffset, GLsizei size);
    void glNamedBufferSubData(GLuint buffer, GLintptr offset, GLsizei size, const void *data);
    void glNamedBufferData(GLuint buffer, GLsizei size, const void *data, GLenum usage);
    void glNamedBufferStorage(GLuint buffer, GLsizei size, const void *data, GLbitfield flags);
    void glCreateBuffers(GLsizei n, GLuint *buffers);
    void glGetTransformFeedbacki64_v(GLuint xfb, GLenum pname, GLuint index, GLint64 *param);
    void glGetTransformFeedbacki_v(GLuint xfb, GLenum pname, GLuint index, GLint *param);
    void glGetTransformFeedbackiv(GLuint xfb, GLenum pname, GLint *param);
    void glTransformFeedbackBufferRange(GLuint xfb, GLuint index, GLuint buffer, GLintptr offset, GLsizei size);
    void glTransformFeedbackBufferBase(GLuint xfb, GLuint index, GLuint buffer);
    void glCreateTransformFeedbacks(GLsizei n, GLuint *ids);
    void glClipControl(GLenum origin, GLenum depth);

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
    QOpenGLFunctions_4_5_CoreBackend* d_4_5_Core;
};

// OpenGL 1.0 core functions
inline void QOpenGLFunctions_4_5_Core::glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
    d_1_0_Core->Viewport(x, y, width, height);
}

inline void QOpenGLFunctions_4_5_Core::glDepthRange(GLdouble nearVal, GLdouble farVal)
{
    d_1_0_Core->DepthRange(nearVal, farVal);
}

inline GLboolean QOpenGLFunctions_4_5_Core::glIsEnabled(GLenum cap)
{
    return d_1_0_Core->IsEnabled(cap);
}

inline void QOpenGLFunctions_4_5_Core::glGetTexLevelParameteriv(GLenum target, GLint level, GLenum pname, GLint *params)
{
    d_1_0_Core->GetTexLevelParameteriv(target, level, pname, params);
}

inline void QOpenGLFunctions_4_5_Core::glGetTexLevelParameterfv(GLenum target, GLint level, GLenum pname, GLfloat *params)
{
    d_1_0_Core->GetTexLevelParameterfv(target, level, pname, params);
}

inline void QOpenGLFunctions_4_5_Core::glGetTexParameteriv(GLenum target, GLenum pname, GLint *params)
{
    d_1_0_Core->GetTexParameteriv(target, pname, params);
}

inline void QOpenGLFunctions_4_5_Core::glGetTexParameterfv(GLenum target, GLenum pname, GLfloat *params)
{
    d_1_0_Core->GetTexParameterfv(target, pname, params);
}

inline void QOpenGLFunctions_4_5_Core::glGetTexImage(GLenum target, GLint level, GLenum format, GLenum type, void *pixels)
{
    d_1_0_Core->GetTexImage(target, level, format, type, pixels);
}

inline const GLubyte * QOpenGLFunctions_4_5_Core::glGetString(GLenum name)
{
    return d_1_0_Core->GetString(name);
}

inline void QOpenGLFunctions_4_5_Core::glGetIntegerv(GLenum pname, GLint *data)
{
    d_1_0_Core->GetIntegerv(pname, data);
}

inline void QOpenGLFunctions_4_5_Core::glGetFloatv(GLenum pname, GLfloat *data)
{
    d_1_0_Core->GetFloatv(pname, data);
}

inline GLenum QOpenGLFunctions_4_5_Core::glGetError()
{
    return d_1_0_Core->GetError();
}

inline void QOpenGLFunctions_4_5_Core::glGetDoublev(GLenum pname, GLdouble *data)
{
    d_1_0_Core->GetDoublev(pname, data);
}

inline void QOpenGLFunctions_4_5_Core::glGetBooleanv(GLenum pname, GLboolean *data)
{
    d_1_0_Core->GetBooleanv(pname, data);
}

inline void QOpenGLFunctions_4_5_Core::glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels)
{
    d_1_0_Core->ReadPixels(x, y, width, height, format, type, pixels);
}

inline void QOpenGLFunctions_4_5_Core::glReadBuffer(GLenum src)
{
    d_1_0_Core->ReadBuffer(src);
}

inline void QOpenGLFunctions_4_5_Core::glPixelStorei(GLenum pname, GLint param)
{
    d_1_0_Core->PixelStorei(pname, param);
}

inline void QOpenGLFunctions_4_5_Core::glPixelStoref(GLenum pname, GLfloat param)
{
    d_1_0_Core->PixelStoref(pname, param);
}

inline void QOpenGLFunctions_4_5_Core::glDepthFunc(GLenum func)
{
    d_1_0_Core->DepthFunc(func);
}

inline void QOpenGLFunctions_4_5_Core::glStencilOp(GLenum fail, GLenum zfail, GLenum zpass)
{
    d_1_0_Core->StencilOp(fail, zfail, zpass);
}

inline void QOpenGLFunctions_4_5_Core::glStencilFunc(GLenum func, GLint ref, GLuint mask)
{
    d_1_0_Core->StencilFunc(func, ref, mask);
}

inline void QOpenGLFunctions_4_5_Core::glLogicOp(GLenum opcode)
{
    d_1_0_Core->LogicOp(opcode);
}

inline void QOpenGLFunctions_4_5_Core::glBlendFunc(GLenum sfactor, GLenum dfactor)
{
    d_1_0_Core->BlendFunc(sfactor, dfactor);
}

inline void QOpenGLFunctions_4_5_Core::glFlush()
{
    d_1_0_Core->Flush();
}

inline void QOpenGLFunctions_4_5_Core::glFinish()
{
    d_1_0_Core->Finish();
}

inline void QOpenGLFunctions_4_5_Core::glEnable(GLenum cap)
{
    d_1_0_Core->Enable(cap);
}

inline void QOpenGLFunctions_4_5_Core::glDisable(GLenum cap)
{
    d_1_0_Core->Disable(cap);
}

inline void QOpenGLFunctions_4_5_Core::glDepthMask(GLboolean flag)
{
    d_1_0_Core->DepthMask(flag);
}

inline void QOpenGLFunctions_4_5_Core::glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
    d_1_0_Core->ColorMask(red, green, blue, alpha);
}

inline void QOpenGLFunctions_4_5_Core::glStencilMask(GLuint mask)
{
    d_1_0_Core->StencilMask(mask);
}

inline void QOpenGLFunctions_4_5_Core::glClearDepth(GLdouble depth)
{
    d_1_0_Core->ClearDepth(depth);
}

inline void QOpenGLFunctions_4_5_Core::glClearStencil(GLint s)
{
    d_1_0_Core->ClearStencil(s);
}

inline void QOpenGLFunctions_4_5_Core::glClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    d_1_0_Core->ClearColor(red, green, blue, alpha);
}

inline void QOpenGLFunctions_4_5_Core::glClear(GLbitfield mask)
{
    d_1_0_Core->Clear(mask);
}

inline void QOpenGLFunctions_4_5_Core::glDrawBuffer(GLenum buf)
{
    d_1_0_Core->DrawBuffer(buf);
}

inline void QOpenGLFunctions_4_5_Core::glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels)
{
    d_1_0_Core->TexImage2D(target, level, internalformat, width, height, border, format, type, pixels);
}

inline void QOpenGLFunctions_4_5_Core::glTexImage1D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const void *pixels)
{
    d_1_0_Core->TexImage1D(target, level, internalformat, width, border, format, type, pixels);
}

inline void QOpenGLFunctions_4_5_Core::glTexParameteriv(GLenum target, GLenum pname, const GLint *params)
{
    d_1_0_Core->TexParameteriv(target, pname, params);
}

inline void QOpenGLFunctions_4_5_Core::glTexParameteri(GLenum target, GLenum pname, GLint param)
{
    d_1_0_Core->TexParameteri(target, pname, param);
}

inline void QOpenGLFunctions_4_5_Core::glTexParameterfv(GLenum target, GLenum pname, const GLfloat *params)
{
    d_1_0_Core->TexParameterfv(target, pname, params);
}

inline void QOpenGLFunctions_4_5_Core::glTexParameterf(GLenum target, GLenum pname, GLfloat param)
{
    d_1_0_Core->TexParameterf(target, pname, param);
}

inline void QOpenGLFunctions_4_5_Core::glScissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
    d_1_0_Core->Scissor(x, y, width, height);
}

inline void QOpenGLFunctions_4_5_Core::glPolygonMode(GLenum face, GLenum mode)
{
    d_1_0_Core->PolygonMode(face, mode);
}

inline void QOpenGLFunctions_4_5_Core::glPointSize(GLfloat size)
{
    d_1_0_Core->PointSize(size);
}

inline void QOpenGLFunctions_4_5_Core::glLineWidth(GLfloat width)
{
    d_1_0_Core->LineWidth(width);
}

inline void QOpenGLFunctions_4_5_Core::glHint(GLenum target, GLenum mode)
{
    d_1_0_Core->Hint(target, mode);
}

inline void QOpenGLFunctions_4_5_Core::glFrontFace(GLenum mode)
{
    d_1_0_Core->FrontFace(mode);
}

inline void QOpenGLFunctions_4_5_Core::glCullFace(GLenum mode)
{
    d_1_0_Core->CullFace(mode);
}


// OpenGL 1.1 core functions
inline GLboolean QOpenGLFunctions_4_5_Core::glIsTexture(GLuint texture)
{
    return d_1_1_Core->IsTexture(texture);
}

inline void QOpenGLFunctions_4_5_Core::glGenTextures(GLsizei n, GLuint *textures)
{
    d_1_1_Core->GenTextures(n, textures);
}

inline void QOpenGLFunctions_4_5_Core::glDeleteTextures(GLsizei n, const GLuint *textures)
{
    d_1_1_Core->DeleteTextures(n, textures);
}

inline void QOpenGLFunctions_4_5_Core::glBindTexture(GLenum target, GLuint texture)
{
    d_1_1_Core->BindTexture(target, texture);
}

inline void QOpenGLFunctions_4_5_Core::glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels)
{
    d_1_1_Core->TexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
}

inline void QOpenGLFunctions_4_5_Core::glTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void *pixels)
{
    d_1_1_Core->TexSubImage1D(target, level, xoffset, width, format, type, pixels);
}

inline void QOpenGLFunctions_4_5_Core::glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    d_1_1_Core->CopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
}

inline void QOpenGLFunctions_4_5_Core::glCopyTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width)
{
    d_1_1_Core->CopyTexSubImage1D(target, level, xoffset, x, y, width);
}

inline void QOpenGLFunctions_4_5_Core::glCopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
    d_1_1_Core->CopyTexImage2D(target, level, internalformat, x, y, width, height, border);
}

inline void QOpenGLFunctions_4_5_Core::glCopyTexImage1D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border)
{
    d_1_1_Core->CopyTexImage1D(target, level, internalformat, x, y, width, border);
}

inline void QOpenGLFunctions_4_5_Core::glPolygonOffset(GLfloat factor, GLfloat units)
{
    d_1_1_Core->PolygonOffset(factor, units);
}

inline void QOpenGLFunctions_4_5_Core::glDrawElements(GLenum mode, GLsizei count, GLenum type, const void *indices)
{
    d_1_1_Core->DrawElements(mode, count, type, indices);
}

inline void QOpenGLFunctions_4_5_Core::glDrawArrays(GLenum mode, GLint first, GLsizei count)
{
    d_1_1_Core->DrawArrays(mode, first, count);
}


// OpenGL 1.2 core functions
inline void QOpenGLFunctions_4_5_Core::glBlendColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    d_1_2_Core->BlendColor(red, green, blue, alpha);
}

inline void QOpenGLFunctions_4_5_Core::glBlendEquation(GLenum mode)
{
    d_1_2_Core->BlendEquation(mode);
}

inline void QOpenGLFunctions_4_5_Core::glCopyTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    d_1_2_Core->CopyTexSubImage3D(target, level, xoffset, yoffset, zoffset, x, y, width, height);
}

inline void QOpenGLFunctions_4_5_Core::glTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels)
{
    d_1_2_Core->TexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
}

inline void QOpenGLFunctions_4_5_Core::glTexImage3D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void *pixels)
{
    d_1_2_Core->TexImage3D(target, level, internalformat, width, height, depth, border, format, type, pixels);
}

inline void QOpenGLFunctions_4_5_Core::glDrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices)
{
    d_1_2_Core->DrawRangeElements(mode, start, end, count, type, indices);
}


// OpenGL 1.3 core functions
inline void QOpenGLFunctions_4_5_Core::glGetCompressedTexImage(GLenum target, GLint level, void *img)
{
    d_1_3_Core->GetCompressedTexImage(target, level, img);
}

inline void QOpenGLFunctions_4_5_Core::glCompressedTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void *data)
{
    d_1_3_Core->CompressedTexSubImage1D(target, level, xoffset, width, format, imageSize, data);
}

inline void QOpenGLFunctions_4_5_Core::glCompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data)
{
    d_1_3_Core->CompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, imageSize, data);
}

inline void QOpenGLFunctions_4_5_Core::glCompressedTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data)
{
    d_1_3_Core->CompressedTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data);
}

inline void QOpenGLFunctions_4_5_Core::glCompressedTexImage1D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const void *data)
{
    d_1_3_Core->CompressedTexImage1D(target, level, internalformat, width, border, imageSize, data);
}

inline void QOpenGLFunctions_4_5_Core::glCompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data)
{
    d_1_3_Core->CompressedTexImage2D(target, level, internalformat, width, height, border, imageSize, data);
}

inline void QOpenGLFunctions_4_5_Core::glCompressedTexImage3D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void *data)
{
    d_1_3_Core->CompressedTexImage3D(target, level, internalformat, width, height, depth, border, imageSize, data);
}

inline void QOpenGLFunctions_4_5_Core::glSampleCoverage(GLfloat value, GLboolean invert)
{
    d_1_3_Core->SampleCoverage(value, invert);
}

inline void QOpenGLFunctions_4_5_Core::glActiveTexture(GLenum texture)
{
    d_1_3_Core->ActiveTexture(texture);
}


// OpenGL 1.4 core functions
inline void QOpenGLFunctions_4_5_Core::glPointParameteriv(GLenum pname, const GLint *params)
{
    d_1_4_Core->PointParameteriv(pname, params);
}

inline void QOpenGLFunctions_4_5_Core::glPointParameteri(GLenum pname, GLint param)
{
    d_1_4_Core->PointParameteri(pname, param);
}

inline void QOpenGLFunctions_4_5_Core::glPointParameterfv(GLenum pname, const GLfloat *params)
{
    d_1_4_Core->PointParameterfv(pname, params);
}

inline void QOpenGLFunctions_4_5_Core::glPointParameterf(GLenum pname, GLfloat param)
{
    d_1_4_Core->PointParameterf(pname, param);
}

inline void QOpenGLFunctions_4_5_Core::glMultiDrawElements(GLenum mode, const GLsizei *count, GLenum type, const void *const *indices, GLsizei drawcount)
{
    d_1_4_Core->MultiDrawElements(mode, count, type, indices, drawcount);
}

inline void QOpenGLFunctions_4_5_Core::glMultiDrawArrays(GLenum mode, const GLint *first, const GLsizei *count, GLsizei drawcount)
{
    d_1_4_Core->MultiDrawArrays(mode, first, count, drawcount);
}

inline void QOpenGLFunctions_4_5_Core::glBlendFuncSeparate(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha)
{
    d_1_4_Core->BlendFuncSeparate(sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha);
}


// OpenGL 1.5 core functions
inline void QOpenGLFunctions_4_5_Core::glGetBufferPointerv(GLenum target, GLenum pname, void * *params)
{
    d_1_5_Core->GetBufferPointerv(target, pname, params);
}

inline void QOpenGLFunctions_4_5_Core::glGetBufferParameteriv(GLenum target, GLenum pname, GLint *params)
{
    d_1_5_Core->GetBufferParameteriv(target, pname, params);
}

inline GLboolean QOpenGLFunctions_4_5_Core::glUnmapBuffer(GLenum target)
{
    return d_1_5_Core->UnmapBuffer(target);
}

inline void * QOpenGLFunctions_4_5_Core::glMapBuffer(GLenum target, GLenum access)
{
    return d_1_5_Core->MapBuffer(target, access);
}

inline void QOpenGLFunctions_4_5_Core::glGetBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, void *data)
{
    d_1_5_Core->GetBufferSubData(target, offset, size, data);
}

inline void QOpenGLFunctions_4_5_Core::glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void *data)
{
    d_1_5_Core->BufferSubData(target, offset, size, data);
}

inline void QOpenGLFunctions_4_5_Core::glBufferData(GLenum target, GLsizeiptr size, const void *data, GLenum usage)
{
    d_1_5_Core->BufferData(target, size, data, usage);
}

inline GLboolean QOpenGLFunctions_4_5_Core::glIsBuffer(GLuint buffer)
{
    return d_1_5_Core->IsBuffer(buffer);
}

inline void QOpenGLFunctions_4_5_Core::glGenBuffers(GLsizei n, GLuint *buffers)
{
    d_1_5_Core->GenBuffers(n, buffers);
}

inline void QOpenGLFunctions_4_5_Core::glDeleteBuffers(GLsizei n, const GLuint *buffers)
{
    d_1_5_Core->DeleteBuffers(n, buffers);
}

inline void QOpenGLFunctions_4_5_Core::glBindBuffer(GLenum target, GLuint buffer)
{
    d_1_5_Core->BindBuffer(target, buffer);
}

inline void QOpenGLFunctions_4_5_Core::glGetQueryObjectuiv(GLuint id, GLenum pname, GLuint *params)
{
    d_1_5_Core->GetQueryObjectuiv(id, pname, params);
}

inline void QOpenGLFunctions_4_5_Core::glGetQueryObjectiv(GLuint id, GLenum pname, GLint *params)
{
    d_1_5_Core->GetQueryObjectiv(id, pname, params);
}

inline void QOpenGLFunctions_4_5_Core::glGetQueryiv(GLenum target, GLenum pname, GLint *params)
{
    d_1_5_Core->GetQueryiv(target, pname, params);
}

inline void QOpenGLFunctions_4_5_Core::glEndQuery(GLenum target)
{
    d_1_5_Core->EndQuery(target);
}

inline void QOpenGLFunctions_4_5_Core::glBeginQuery(GLenum target, GLuint id)
{
    d_1_5_Core->BeginQuery(target, id);
}

inline GLboolean QOpenGLFunctions_4_5_Core::glIsQuery(GLuint id)
{
    return d_1_5_Core->IsQuery(id);
}

inline void QOpenGLFunctions_4_5_Core::glDeleteQueries(GLsizei n, const GLuint *ids)
{
    d_1_5_Core->DeleteQueries(n, ids);
}

inline void QOpenGLFunctions_4_5_Core::glGenQueries(GLsizei n, GLuint *ids)
{
    d_1_5_Core->GenQueries(n, ids);
}


// OpenGL 2.0 core functions
inline void QOpenGLFunctions_4_5_Core::glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer)
{
    d_2_0_Core->VertexAttribPointer(index, size, type, normalized, stride, pointer);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttrib4usv(GLuint index, const GLushort *v)
{
    d_2_0_Core->VertexAttrib4usv(index, v);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttrib4uiv(GLuint index, const GLuint *v)
{
    d_2_0_Core->VertexAttrib4uiv(index, v);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttrib4ubv(GLuint index, const GLubyte *v)
{
    d_2_0_Core->VertexAttrib4ubv(index, v);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttrib4sv(GLuint index, const GLshort *v)
{
    d_2_0_Core->VertexAttrib4sv(index, v);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttrib4s(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w)
{
    d_2_0_Core->VertexAttrib4s(index, x, y, z, w);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttrib4iv(GLuint index, const GLint *v)
{
    d_2_0_Core->VertexAttrib4iv(index, v);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttrib4fv(GLuint index, const GLfloat *v)
{
    d_2_0_Core->VertexAttrib4fv(index, v);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttrib4f(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    d_2_0_Core->VertexAttrib4f(index, x, y, z, w);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttrib4dv(GLuint index, const GLdouble *v)
{
    d_2_0_Core->VertexAttrib4dv(index, v);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttrib4d(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    d_2_0_Core->VertexAttrib4d(index, x, y, z, w);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttrib4bv(GLuint index, const GLbyte *v)
{
    d_2_0_Core->VertexAttrib4bv(index, v);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttrib4Nusv(GLuint index, const GLushort *v)
{
    d_2_0_Core->VertexAttrib4Nusv(index, v);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttrib4Nuiv(GLuint index, const GLuint *v)
{
    d_2_0_Core->VertexAttrib4Nuiv(index, v);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttrib4Nubv(GLuint index, const GLubyte *v)
{
    d_2_0_Core->VertexAttrib4Nubv(index, v);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttrib4Nub(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w)
{
    d_2_0_Core->VertexAttrib4Nub(index, x, y, z, w);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttrib4Nsv(GLuint index, const GLshort *v)
{
    d_2_0_Core->VertexAttrib4Nsv(index, v);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttrib4Niv(GLuint index, const GLint *v)
{
    d_2_0_Core->VertexAttrib4Niv(index, v);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttrib4Nbv(GLuint index, const GLbyte *v)
{
    d_2_0_Core->VertexAttrib4Nbv(index, v);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttrib3sv(GLuint index, const GLshort *v)
{
    d_2_0_Core->VertexAttrib3sv(index, v);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttrib3s(GLuint index, GLshort x, GLshort y, GLshort z)
{
    d_2_0_Core->VertexAttrib3s(index, x, y, z);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttrib3fv(GLuint index, const GLfloat *v)
{
    d_2_0_Core->VertexAttrib3fv(index, v);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttrib3f(GLuint index, GLfloat x, GLfloat y, GLfloat z)
{
    d_2_0_Core->VertexAttrib3f(index, x, y, z);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttrib3dv(GLuint index, const GLdouble *v)
{
    d_2_0_Core->VertexAttrib3dv(index, v);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttrib3d(GLuint index, GLdouble x, GLdouble y, GLdouble z)
{
    d_2_0_Core->VertexAttrib3d(index, x, y, z);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttrib2sv(GLuint index, const GLshort *v)
{
    d_2_0_Core->VertexAttrib2sv(index, v);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttrib2s(GLuint index, GLshort x, GLshort y)
{
    d_2_0_Core->VertexAttrib2s(index, x, y);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttrib2fv(GLuint index, const GLfloat *v)
{
    d_2_0_Core->VertexAttrib2fv(index, v);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttrib2f(GLuint index, GLfloat x, GLfloat y)
{
    d_2_0_Core->VertexAttrib2f(index, x, y);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttrib2dv(GLuint index, const GLdouble *v)
{
    d_2_0_Core->VertexAttrib2dv(index, v);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttrib2d(GLuint index, GLdouble x, GLdouble y)
{
    d_2_0_Core->VertexAttrib2d(index, x, y);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttrib1sv(GLuint index, const GLshort *v)
{
    d_2_0_Core->VertexAttrib1sv(index, v);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttrib1s(GLuint index, GLshort x)
{
    d_2_0_Core->VertexAttrib1s(index, x);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttrib1fv(GLuint index, const GLfloat *v)
{
    d_2_0_Core->VertexAttrib1fv(index, v);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttrib1f(GLuint index, GLfloat x)
{
    d_2_0_Core->VertexAttrib1f(index, x);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttrib1dv(GLuint index, const GLdouble *v)
{
    d_2_0_Core->VertexAttrib1dv(index, v);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttrib1d(GLuint index, GLdouble x)
{
    d_2_0_Core->VertexAttrib1d(index, x);
}

inline void QOpenGLFunctions_4_5_Core::glValidateProgram(GLuint program)
{
    d_2_0_Core->ValidateProgram(program);
}

inline void QOpenGLFunctions_4_5_Core::glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    d_2_0_Core->UniformMatrix4fv(location, count, transpose, value);
}

inline void QOpenGLFunctions_4_5_Core::glUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    d_2_0_Core->UniformMatrix3fv(location, count, transpose, value);
}

inline void QOpenGLFunctions_4_5_Core::glUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    d_2_0_Core->UniformMatrix2fv(location, count, transpose, value);
}

inline void QOpenGLFunctions_4_5_Core::glUniform4iv(GLint location, GLsizei count, const GLint *value)
{
    d_2_0_Core->Uniform4iv(location, count, value);
}

inline void QOpenGLFunctions_4_5_Core::glUniform3iv(GLint location, GLsizei count, const GLint *value)
{
    d_2_0_Core->Uniform3iv(location, count, value);
}

inline void QOpenGLFunctions_4_5_Core::glUniform2iv(GLint location, GLsizei count, const GLint *value)
{
    d_2_0_Core->Uniform2iv(location, count, value);
}

inline void QOpenGLFunctions_4_5_Core::glUniform1iv(GLint location, GLsizei count, const GLint *value)
{
    d_2_0_Core->Uniform1iv(location, count, value);
}

inline void QOpenGLFunctions_4_5_Core::glUniform4fv(GLint location, GLsizei count, const GLfloat *value)
{
    d_2_0_Core->Uniform4fv(location, count, value);
}

inline void QOpenGLFunctions_4_5_Core::glUniform3fv(GLint location, GLsizei count, const GLfloat *value)
{
    d_2_0_Core->Uniform3fv(location, count, value);
}

inline void QOpenGLFunctions_4_5_Core::glUniform2fv(GLint location, GLsizei count, const GLfloat *value)
{
    d_2_0_Core->Uniform2fv(location, count, value);
}

inline void QOpenGLFunctions_4_5_Core::glUniform1fv(GLint location, GLsizei count, const GLfloat *value)
{
    d_2_0_Core->Uniform1fv(location, count, value);
}

inline void QOpenGLFunctions_4_5_Core::glUniform4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3)
{
    d_2_0_Core->Uniform4i(location, v0, v1, v2, v3);
}

inline void QOpenGLFunctions_4_5_Core::glUniform3i(GLint location, GLint v0, GLint v1, GLint v2)
{
    d_2_0_Core->Uniform3i(location, v0, v1, v2);
}

inline void QOpenGLFunctions_4_5_Core::glUniform2i(GLint location, GLint v0, GLint v1)
{
    d_2_0_Core->Uniform2i(location, v0, v1);
}

inline void QOpenGLFunctions_4_5_Core::glUniform1i(GLint location, GLint v0)
{
    d_2_0_Core->Uniform1i(location, v0);
}

inline void QOpenGLFunctions_4_5_Core::glUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
    d_2_0_Core->Uniform4f(location, v0, v1, v2, v3);
}

inline void QOpenGLFunctions_4_5_Core::glUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
{
    d_2_0_Core->Uniform3f(location, v0, v1, v2);
}

inline void QOpenGLFunctions_4_5_Core::glUniform2f(GLint location, GLfloat v0, GLfloat v1)
{
    d_2_0_Core->Uniform2f(location, v0, v1);
}

inline void QOpenGLFunctions_4_5_Core::glUniform1f(GLint location, GLfloat v0)
{
    d_2_0_Core->Uniform1f(location, v0);
}

inline void QOpenGLFunctions_4_5_Core::glUseProgram(GLuint program)
{
    d_2_0_Core->UseProgram(program);
}

inline void QOpenGLFunctions_4_5_Core::glShaderSource(GLuint shader, GLsizei count, const GLchar *const *string, const GLint *length)
{
    d_2_0_Core->ShaderSource(shader, count, string, length);
}

inline void QOpenGLFunctions_4_5_Core::glLinkProgram(GLuint program)
{
    d_2_0_Core->LinkProgram(program);
}

inline GLboolean QOpenGLFunctions_4_5_Core::glIsShader(GLuint shader)
{
    return d_2_0_Core->IsShader(shader);
}

inline GLboolean QOpenGLFunctions_4_5_Core::glIsProgram(GLuint program)
{
    return d_2_0_Core->IsProgram(program);
}

inline void QOpenGLFunctions_4_5_Core::glGetVertexAttribPointerv(GLuint index, GLenum pname, void * *pointer)
{
    d_2_0_Core->GetVertexAttribPointerv(index, pname, pointer);
}

inline void QOpenGLFunctions_4_5_Core::glGetVertexAttribiv(GLuint index, GLenum pname, GLint *params)
{
    d_2_0_Core->GetVertexAttribiv(index, pname, params);
}

inline void QOpenGLFunctions_4_5_Core::glGetVertexAttribfv(GLuint index, GLenum pname, GLfloat *params)
{
    d_2_0_Core->GetVertexAttribfv(index, pname, params);
}

inline void QOpenGLFunctions_4_5_Core::glGetVertexAttribdv(GLuint index, GLenum pname, GLdouble *params)
{
    d_2_0_Core->GetVertexAttribdv(index, pname, params);
}

inline void QOpenGLFunctions_4_5_Core::glGetUniformiv(GLuint program, GLint location, GLint *params)
{
    d_2_0_Core->GetUniformiv(program, location, params);
}

inline void QOpenGLFunctions_4_5_Core::glGetUniformfv(GLuint program, GLint location, GLfloat *params)
{
    d_2_0_Core->GetUniformfv(program, location, params);
}

inline GLint QOpenGLFunctions_4_5_Core::glGetUniformLocation(GLuint program, const GLchar *name)
{
    return d_2_0_Core->GetUniformLocation(program, name);
}

inline void QOpenGLFunctions_4_5_Core::glGetShaderSource(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source)
{
    d_2_0_Core->GetShaderSource(shader, bufSize, length, source);
}

inline void QOpenGLFunctions_4_5_Core::glGetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog)
{
    d_2_0_Core->GetShaderInfoLog(shader, bufSize, length, infoLog);
}

inline void QOpenGLFunctions_4_5_Core::glGetShaderiv(GLuint shader, GLenum pname, GLint *params)
{
    d_2_0_Core->GetShaderiv(shader, pname, params);
}

inline void QOpenGLFunctions_4_5_Core::glGetProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog)
{
    d_2_0_Core->GetProgramInfoLog(program, bufSize, length, infoLog);
}

inline void QOpenGLFunctions_4_5_Core::glGetProgramiv(GLuint program, GLenum pname, GLint *params)
{
    d_2_0_Core->GetProgramiv(program, pname, params);
}

inline GLint QOpenGLFunctions_4_5_Core::glGetAttribLocation(GLuint program, const GLchar *name)
{
    return d_2_0_Core->GetAttribLocation(program, name);
}

inline void QOpenGLFunctions_4_5_Core::glGetAttachedShaders(GLuint program, GLsizei maxCount, GLsizei *count, GLuint *shaders)
{
    d_2_0_Core->GetAttachedShaders(program, maxCount, count, shaders);
}

inline void QOpenGLFunctions_4_5_Core::glGetActiveUniform(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name)
{
    d_2_0_Core->GetActiveUniform(program, index, bufSize, length, size, type, name);
}

inline void QOpenGLFunctions_4_5_Core::glGetActiveAttrib(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name)
{
    d_2_0_Core->GetActiveAttrib(program, index, bufSize, length, size, type, name);
}

inline void QOpenGLFunctions_4_5_Core::glEnableVertexAttribArray(GLuint index)
{
    d_2_0_Core->EnableVertexAttribArray(index);
}

inline void QOpenGLFunctions_4_5_Core::glDisableVertexAttribArray(GLuint index)
{
    d_2_0_Core->DisableVertexAttribArray(index);
}

inline void QOpenGLFunctions_4_5_Core::glDetachShader(GLuint program, GLuint shader)
{
    d_2_0_Core->DetachShader(program, shader);
}

inline void QOpenGLFunctions_4_5_Core::glDeleteShader(GLuint shader)
{
    d_2_0_Core->DeleteShader(shader);
}

inline void QOpenGLFunctions_4_5_Core::glDeleteProgram(GLuint program)
{
    d_2_0_Core->DeleteProgram(program);
}

inline GLuint QOpenGLFunctions_4_5_Core::glCreateShader(GLenum type)
{
    return d_2_0_Core->CreateShader(type);
}

inline GLuint QOpenGLFunctions_4_5_Core::glCreateProgram()
{
    return d_2_0_Core->CreateProgram();
}

inline void QOpenGLFunctions_4_5_Core::glCompileShader(GLuint shader)
{
    d_2_0_Core->CompileShader(shader);
}

inline void QOpenGLFunctions_4_5_Core::glBindAttribLocation(GLuint program, GLuint index, const GLchar *name)
{
    d_2_0_Core->BindAttribLocation(program, index, name);
}

inline void QOpenGLFunctions_4_5_Core::glAttachShader(GLuint program, GLuint shader)
{
    d_2_0_Core->AttachShader(program, shader);
}

inline void QOpenGLFunctions_4_5_Core::glStencilMaskSeparate(GLenum face, GLuint mask)
{
    d_2_0_Core->StencilMaskSeparate(face, mask);
}

inline void QOpenGLFunctions_4_5_Core::glStencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask)
{
    d_2_0_Core->StencilFuncSeparate(face, func, ref, mask);
}

inline void QOpenGLFunctions_4_5_Core::glStencilOpSeparate(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass)
{
    d_2_0_Core->StencilOpSeparate(face, sfail, dpfail, dppass);
}

inline void QOpenGLFunctions_4_5_Core::glDrawBuffers(GLsizei n, const GLenum *bufs)
{
    d_2_0_Core->DrawBuffers(n, bufs);
}

inline void QOpenGLFunctions_4_5_Core::glBlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha)
{
    d_2_0_Core->BlendEquationSeparate(modeRGB, modeAlpha);
}


// OpenGL 2.1 core functions
inline void QOpenGLFunctions_4_5_Core::glUniformMatrix4x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    d_2_1_Core->UniformMatrix4x3fv(location, count, transpose, value);
}

inline void QOpenGLFunctions_4_5_Core::glUniformMatrix3x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    d_2_1_Core->UniformMatrix3x4fv(location, count, transpose, value);
}

inline void QOpenGLFunctions_4_5_Core::glUniformMatrix4x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    d_2_1_Core->UniformMatrix4x2fv(location, count, transpose, value);
}

inline void QOpenGLFunctions_4_5_Core::glUniformMatrix2x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    d_2_1_Core->UniformMatrix2x4fv(location, count, transpose, value);
}

inline void QOpenGLFunctions_4_5_Core::glUniformMatrix3x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    d_2_1_Core->UniformMatrix3x2fv(location, count, transpose, value);
}

inline void QOpenGLFunctions_4_5_Core::glUniformMatrix2x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    d_2_1_Core->UniformMatrix2x3fv(location, count, transpose, value);
}


// OpenGL 3.0 core functions
inline GLboolean QOpenGLFunctions_4_5_Core::glIsVertexArray(GLuint array)
{
    return d_3_0_Core->IsVertexArray(array);
}

inline void QOpenGLFunctions_4_5_Core::glGenVertexArrays(GLsizei n, GLuint *arrays)
{
    d_3_0_Core->GenVertexArrays(n, arrays);
}

inline void QOpenGLFunctions_4_5_Core::glDeleteVertexArrays(GLsizei n, const GLuint *arrays)
{
    d_3_0_Core->DeleteVertexArrays(n, arrays);
}

inline void QOpenGLFunctions_4_5_Core::glBindVertexArray(GLuint array)
{
    d_3_0_Core->BindVertexArray(array);
}

inline void QOpenGLFunctions_4_5_Core::glFlushMappedBufferRange(GLenum target, GLintptr offset, GLsizeiptr length)
{
    d_3_0_Core->FlushMappedBufferRange(target, offset, length);
}

inline void * QOpenGLFunctions_4_5_Core::glMapBufferRange(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access)
{
    return d_3_0_Core->MapBufferRange(target, offset, length, access);
}

inline void QOpenGLFunctions_4_5_Core::glFramebufferTextureLayer(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer)
{
    d_3_0_Core->FramebufferTextureLayer(target, attachment, texture, level, layer);
}

inline void QOpenGLFunctions_4_5_Core::glRenderbufferStorageMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
{
    d_3_0_Core->RenderbufferStorageMultisample(target, samples, internalformat, width, height);
}

inline void QOpenGLFunctions_4_5_Core::glBlitFramebuffer(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)
{
    d_3_0_Core->BlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
}

inline void QOpenGLFunctions_4_5_Core::glGenerateMipmap(GLenum target)
{
    d_3_0_Core->GenerateMipmap(target);
}

inline void QOpenGLFunctions_4_5_Core::glGetFramebufferAttachmentParameteriv(GLenum target, GLenum attachment, GLenum pname, GLint *params)
{
    d_3_0_Core->GetFramebufferAttachmentParameteriv(target, attachment, pname, params);
}

inline void QOpenGLFunctions_4_5_Core::glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
    d_3_0_Core->FramebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer);
}

inline void QOpenGLFunctions_4_5_Core::glFramebufferTexture3D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset)
{
    d_3_0_Core->FramebufferTexture3D(target, attachment, textarget, texture, level, zoffset);
}

inline void QOpenGLFunctions_4_5_Core::glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
    d_3_0_Core->FramebufferTexture2D(target, attachment, textarget, texture, level);
}

inline void QOpenGLFunctions_4_5_Core::glFramebufferTexture1D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
    d_3_0_Core->FramebufferTexture1D(target, attachment, textarget, texture, level);
}

inline GLenum QOpenGLFunctions_4_5_Core::glCheckFramebufferStatus(GLenum target)
{
    return d_3_0_Core->CheckFramebufferStatus(target);
}

inline void QOpenGLFunctions_4_5_Core::glGenFramebuffers(GLsizei n, GLuint *framebuffers)
{
    d_3_0_Core->GenFramebuffers(n, framebuffers);
}

inline void QOpenGLFunctions_4_5_Core::glDeleteFramebuffers(GLsizei n, const GLuint *framebuffers)
{
    d_3_0_Core->DeleteFramebuffers(n, framebuffers);
}

inline void QOpenGLFunctions_4_5_Core::glBindFramebuffer(GLenum target, GLuint framebuffer)
{
    d_3_0_Core->BindFramebuffer(target, framebuffer);
}

inline GLboolean QOpenGLFunctions_4_5_Core::glIsFramebuffer(GLuint framebuffer)
{
    return d_3_0_Core->IsFramebuffer(framebuffer);
}

inline void QOpenGLFunctions_4_5_Core::glGetRenderbufferParameteriv(GLenum target, GLenum pname, GLint *params)
{
    d_3_0_Core->GetRenderbufferParameteriv(target, pname, params);
}

inline void QOpenGLFunctions_4_5_Core::glRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
{
    d_3_0_Core->RenderbufferStorage(target, internalformat, width, height);
}

inline void QOpenGLFunctions_4_5_Core::glGenRenderbuffers(GLsizei n, GLuint *renderbuffers)
{
    d_3_0_Core->GenRenderbuffers(n, renderbuffers);
}

inline void QOpenGLFunctions_4_5_Core::glDeleteRenderbuffers(GLsizei n, const GLuint *renderbuffers)
{
    d_3_0_Core->DeleteRenderbuffers(n, renderbuffers);
}

inline void QOpenGLFunctions_4_5_Core::glBindRenderbuffer(GLenum target, GLuint renderbuffer)
{
    d_3_0_Core->BindRenderbuffer(target, renderbuffer);
}

inline GLboolean QOpenGLFunctions_4_5_Core::glIsRenderbuffer(GLuint renderbuffer)
{
    return d_3_0_Core->IsRenderbuffer(renderbuffer);
}

inline const GLubyte * QOpenGLFunctions_4_5_Core::glGetStringi(GLenum name, GLuint index)
{
    return d_3_0_Core->GetStringi(name, index);
}

inline void QOpenGLFunctions_4_5_Core::glClearBufferfi(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil)
{
    d_3_0_Core->ClearBufferfi(buffer, drawbuffer, depth, stencil);
}

inline void QOpenGLFunctions_4_5_Core::glClearBufferfv(GLenum buffer, GLint drawbuffer, const GLfloat *value)
{
    d_3_0_Core->ClearBufferfv(buffer, drawbuffer, value);
}

inline void QOpenGLFunctions_4_5_Core::glClearBufferuiv(GLenum buffer, GLint drawbuffer, const GLuint *value)
{
    d_3_0_Core->ClearBufferuiv(buffer, drawbuffer, value);
}

inline void QOpenGLFunctions_4_5_Core::glClearBufferiv(GLenum buffer, GLint drawbuffer, const GLint *value)
{
    d_3_0_Core->ClearBufferiv(buffer, drawbuffer, value);
}

inline void QOpenGLFunctions_4_5_Core::glGetTexParameterIuiv(GLenum target, GLenum pname, GLuint *params)
{
    d_3_0_Core->GetTexParameterIuiv(target, pname, params);
}

inline void QOpenGLFunctions_4_5_Core::glGetTexParameterIiv(GLenum target, GLenum pname, GLint *params)
{
    d_3_0_Core->GetTexParameterIiv(target, pname, params);
}

inline void QOpenGLFunctions_4_5_Core::glTexParameterIuiv(GLenum target, GLenum pname, const GLuint *params)
{
    d_3_0_Core->TexParameterIuiv(target, pname, params);
}

inline void QOpenGLFunctions_4_5_Core::glTexParameterIiv(GLenum target, GLenum pname, const GLint *params)
{
    d_3_0_Core->TexParameterIiv(target, pname, params);
}

inline void QOpenGLFunctions_4_5_Core::glUniform4uiv(GLint location, GLsizei count, const GLuint *value)
{
    d_3_0_Core->Uniform4uiv(location, count, value);
}

inline void QOpenGLFunctions_4_5_Core::glUniform3uiv(GLint location, GLsizei count, const GLuint *value)
{
    d_3_0_Core->Uniform3uiv(location, count, value);
}

inline void QOpenGLFunctions_4_5_Core::glUniform2uiv(GLint location, GLsizei count, const GLuint *value)
{
    d_3_0_Core->Uniform2uiv(location, count, value);
}

inline void QOpenGLFunctions_4_5_Core::glUniform1uiv(GLint location, GLsizei count, const GLuint *value)
{
    d_3_0_Core->Uniform1uiv(location, count, value);
}

inline void QOpenGLFunctions_4_5_Core::glUniform4ui(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
    d_3_0_Core->Uniform4ui(location, v0, v1, v2, v3);
}

inline void QOpenGLFunctions_4_5_Core::glUniform3ui(GLint location, GLuint v0, GLuint v1, GLuint v2)
{
    d_3_0_Core->Uniform3ui(location, v0, v1, v2);
}

inline void QOpenGLFunctions_4_5_Core::glUniform2ui(GLint location, GLuint v0, GLuint v1)
{
    d_3_0_Core->Uniform2ui(location, v0, v1);
}

inline void QOpenGLFunctions_4_5_Core::glUniform1ui(GLint location, GLuint v0)
{
    d_3_0_Core->Uniform1ui(location, v0);
}

inline GLint QOpenGLFunctions_4_5_Core::glGetFragDataLocation(GLuint program, const GLchar *name)
{
    return d_3_0_Core->GetFragDataLocation(program, name);
}

inline void QOpenGLFunctions_4_5_Core::glBindFragDataLocation(GLuint program, GLuint color, const GLchar *name)
{
    d_3_0_Core->BindFragDataLocation(program, color, name);
}

inline void QOpenGLFunctions_4_5_Core::glGetUniformuiv(GLuint program, GLint location, GLuint *params)
{
    d_3_0_Core->GetUniformuiv(program, location, params);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttribI4usv(GLuint index, const GLushort *v)
{
    d_3_0_Core->VertexAttribI4usv(index, v);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttribI4ubv(GLuint index, const GLubyte *v)
{
    d_3_0_Core->VertexAttribI4ubv(index, v);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttribI4sv(GLuint index, const GLshort *v)
{
    d_3_0_Core->VertexAttribI4sv(index, v);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttribI4bv(GLuint index, const GLbyte *v)
{
    d_3_0_Core->VertexAttribI4bv(index, v);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttribI4uiv(GLuint index, const GLuint *v)
{
    d_3_0_Core->VertexAttribI4uiv(index, v);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttribI3uiv(GLuint index, const GLuint *v)
{
    d_3_0_Core->VertexAttribI3uiv(index, v);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttribI2uiv(GLuint index, const GLuint *v)
{
    d_3_0_Core->VertexAttribI2uiv(index, v);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttribI1uiv(GLuint index, const GLuint *v)
{
    d_3_0_Core->VertexAttribI1uiv(index, v);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttribI4iv(GLuint index, const GLint *v)
{
    d_3_0_Core->VertexAttribI4iv(index, v);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttribI3iv(GLuint index, const GLint *v)
{
    d_3_0_Core->VertexAttribI3iv(index, v);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttribI2iv(GLuint index, const GLint *v)
{
    d_3_0_Core->VertexAttribI2iv(index, v);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttribI1iv(GLuint index, const GLint *v)
{
    d_3_0_Core->VertexAttribI1iv(index, v);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttribI4ui(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w)
{
    d_3_0_Core->VertexAttribI4ui(index, x, y, z, w);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttribI3ui(GLuint index, GLuint x, GLuint y, GLuint z)
{
    d_3_0_Core->VertexAttribI3ui(index, x, y, z);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttribI2ui(GLuint index, GLuint x, GLuint y)
{
    d_3_0_Core->VertexAttribI2ui(index, x, y);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttribI1ui(GLuint index, GLuint x)
{
    d_3_0_Core->VertexAttribI1ui(index, x);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttribI4i(GLuint index, GLint x, GLint y, GLint z, GLint w)
{
    d_3_0_Core->VertexAttribI4i(index, x, y, z, w);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttribI3i(GLuint index, GLint x, GLint y, GLint z)
{
    d_3_0_Core->VertexAttribI3i(index, x, y, z);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttribI2i(GLuint index, GLint x, GLint y)
{
    d_3_0_Core->VertexAttribI2i(index, x, y);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttribI1i(GLuint index, GLint x)
{
    d_3_0_Core->VertexAttribI1i(index, x);
}

inline void QOpenGLFunctions_4_5_Core::glGetVertexAttribIuiv(GLuint index, GLenum pname, GLuint *params)
{
    d_3_0_Core->GetVertexAttribIuiv(index, pname, params);
}

inline void QOpenGLFunctions_4_5_Core::glGetVertexAttribIiv(GLuint index, GLenum pname, GLint *params)
{
    d_3_0_Core->GetVertexAttribIiv(index, pname, params);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer)
{
    d_3_0_Core->VertexAttribIPointer(index, size, type, stride, pointer);
}

inline void QOpenGLFunctions_4_5_Core::glEndConditionalRender()
{
    d_3_0_Core->EndConditionalRender();
}

inline void QOpenGLFunctions_4_5_Core::glBeginConditionalRender(GLuint id, GLenum mode)
{
    d_3_0_Core->BeginConditionalRender(id, mode);
}

inline void QOpenGLFunctions_4_5_Core::glClampColor(GLenum target, GLenum clamp)
{
    d_3_0_Core->ClampColor(target, clamp);
}

inline void QOpenGLFunctions_4_5_Core::glGetTransformFeedbackVarying(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLsizei *size, GLenum *type, GLchar *name)
{
    d_3_0_Core->GetTransformFeedbackVarying(program, index, bufSize, length, size, type, name);
}

inline void QOpenGLFunctions_4_5_Core::glTransformFeedbackVaryings(GLuint program, GLsizei count, const GLchar *const *varyings, GLenum bufferMode)
{
    d_3_0_Core->TransformFeedbackVaryings(program, count, varyings, bufferMode);
}

inline void QOpenGLFunctions_4_5_Core::glBindBufferBase(GLenum target, GLuint index, GLuint buffer)
{
    d_3_0_Core->BindBufferBase(target, index, buffer);
}

inline void QOpenGLFunctions_4_5_Core::glBindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)
{
    d_3_0_Core->BindBufferRange(target, index, buffer, offset, size);
}

inline void QOpenGLFunctions_4_5_Core::glEndTransformFeedback()
{
    d_3_0_Core->EndTransformFeedback();
}

inline void QOpenGLFunctions_4_5_Core::glBeginTransformFeedback(GLenum primitiveMode)
{
    d_3_0_Core->BeginTransformFeedback(primitiveMode);
}

inline GLboolean QOpenGLFunctions_4_5_Core::glIsEnabledi(GLenum target, GLuint index)
{
    return d_3_0_Core->IsEnabledi(target, index);
}

inline void QOpenGLFunctions_4_5_Core::glDisablei(GLenum target, GLuint index)
{
    d_3_0_Core->Disablei(target, index);
}

inline void QOpenGLFunctions_4_5_Core::glEnablei(GLenum target, GLuint index)
{
    d_3_0_Core->Enablei(target, index);
}

inline void QOpenGLFunctions_4_5_Core::glGetIntegeri_v(GLenum target, GLuint index, GLint *data)
{
    d_3_0_Core->GetIntegeri_v(target, index, data);
}

inline void QOpenGLFunctions_4_5_Core::glGetBooleani_v(GLenum target, GLuint index, GLboolean *data)
{
    d_3_0_Core->GetBooleani_v(target, index, data);
}

inline void QOpenGLFunctions_4_5_Core::glColorMaski(GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a)
{
    d_3_0_Core->ColorMaski(index, r, g, b, a);
}


// OpenGL 3.1 core functions
inline void QOpenGLFunctions_4_5_Core::glUniformBlockBinding(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding)
{
    d_3_1_Core->UniformBlockBinding(program, uniformBlockIndex, uniformBlockBinding);
}

inline void QOpenGLFunctions_4_5_Core::glGetActiveUniformBlockName(GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformBlockName)
{
    d_3_1_Core->GetActiveUniformBlockName(program, uniformBlockIndex, bufSize, length, uniformBlockName);
}

inline void QOpenGLFunctions_4_5_Core::glGetActiveUniformBlockiv(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint *params)
{
    d_3_1_Core->GetActiveUniformBlockiv(program, uniformBlockIndex, pname, params);
}

inline GLuint QOpenGLFunctions_4_5_Core::glGetUniformBlockIndex(GLuint program, const GLchar *uniformBlockName)
{
    return d_3_1_Core->GetUniformBlockIndex(program, uniformBlockName);
}

inline void QOpenGLFunctions_4_5_Core::glGetActiveUniformName(GLuint program, GLuint uniformIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformName)
{
    d_3_1_Core->GetActiveUniformName(program, uniformIndex, bufSize, length, uniformName);
}

inline void QOpenGLFunctions_4_5_Core::glGetActiveUniformsiv(GLuint program, GLsizei uniformCount, const GLuint *uniformIndices, GLenum pname, GLint *params)
{
    d_3_1_Core->GetActiveUniformsiv(program, uniformCount, uniformIndices, pname, params);
}

inline void QOpenGLFunctions_4_5_Core::glGetUniformIndices(GLuint program, GLsizei uniformCount, const GLchar *const *uniformNames, GLuint *uniformIndices)
{
    d_3_1_Core->GetUniformIndices(program, uniformCount, uniformNames, uniformIndices);
}

inline void QOpenGLFunctions_4_5_Core::glCopyBufferSubData(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size)
{
    d_3_1_Core->CopyBufferSubData(readTarget, writeTarget, readOffset, writeOffset, size);
}

inline void QOpenGLFunctions_4_5_Core::glPrimitiveRestartIndex(GLuint index)
{
    d_3_1_Core->PrimitiveRestartIndex(index);
}

inline void QOpenGLFunctions_4_5_Core::glTexBuffer(GLenum target, GLenum internalformat, GLuint buffer)
{
    d_3_1_Core->TexBuffer(target, internalformat, buffer);
}

inline void QOpenGLFunctions_4_5_Core::glDrawElementsInstanced(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount)
{
    d_3_1_Core->DrawElementsInstanced(mode, count, type, indices, instancecount);
}

inline void QOpenGLFunctions_4_5_Core::glDrawArraysInstanced(GLenum mode, GLint first, GLsizei count, GLsizei instancecount)
{
    d_3_1_Core->DrawArraysInstanced(mode, first, count, instancecount);
}


// OpenGL 3.2 core functions
inline void QOpenGLFunctions_4_5_Core::glSampleMaski(GLuint maskNumber, GLbitfield mask)
{
    d_3_2_Core->SampleMaski(maskNumber, mask);
}

inline void QOpenGLFunctions_4_5_Core::glGetMultisamplefv(GLenum pname, GLuint index, GLfloat *val)
{
    d_3_2_Core->GetMultisamplefv(pname, index, val);
}

inline void QOpenGLFunctions_4_5_Core::glTexImage3DMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations)
{
    d_3_2_Core->TexImage3DMultisample(target, samples, internalformat, width, height, depth, fixedsamplelocations);
}

inline void QOpenGLFunctions_4_5_Core::glTexImage2DMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations)
{
    d_3_2_Core->TexImage2DMultisample(target, samples, internalformat, width, height, fixedsamplelocations);
}

inline void QOpenGLFunctions_4_5_Core::glFramebufferTexture(GLenum target, GLenum attachment, GLuint texture, GLint level)
{
    d_3_2_Core->FramebufferTexture(target, attachment, texture, level);
}

inline void QOpenGLFunctions_4_5_Core::glGetBufferParameteri64v(GLenum target, GLenum pname, GLint64 *params)
{
    d_3_2_Core->GetBufferParameteri64v(target, pname, params);
}

inline void QOpenGLFunctions_4_5_Core::glGetInteger64i_v(GLenum target, GLuint index, GLint64 *data)
{
    d_3_2_Core->GetInteger64i_v(target, index, data);
}

inline void QOpenGLFunctions_4_5_Core::glGetSynciv(GLsync sync, GLenum pname, GLsizei bufSize, GLsizei *length, GLint *values)
{
    d_3_2_Core->GetSynciv(sync, pname, bufSize, length, values);
}

inline void QOpenGLFunctions_4_5_Core::glGetInteger64v(GLenum pname, GLint64 *data)
{
    d_3_2_Core->GetInteger64v(pname, data);
}

inline void QOpenGLFunctions_4_5_Core::glWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout)
{
    d_3_2_Core->WaitSync(sync, flags, timeout);
}

inline GLenum QOpenGLFunctions_4_5_Core::glClientWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout)
{
    return d_3_2_Core->ClientWaitSync(sync, flags, timeout);
}

inline void QOpenGLFunctions_4_5_Core::glDeleteSync(GLsync sync)
{
    d_3_2_Core->DeleteSync(sync);
}

inline GLboolean QOpenGLFunctions_4_5_Core::glIsSync(GLsync sync)
{
    return d_3_2_Core->IsSync(sync);
}

inline GLsync QOpenGLFunctions_4_5_Core::glFenceSync(GLenum condition, GLbitfield flags)
{
    return d_3_2_Core->FenceSync(condition, flags);
}

inline void QOpenGLFunctions_4_5_Core::glProvokingVertex(GLenum mode)
{
    d_3_2_Core->ProvokingVertex(mode);
}

inline void QOpenGLFunctions_4_5_Core::glMultiDrawElementsBaseVertex(GLenum mode, const GLsizei *count, GLenum type, const void *const *indices, GLsizei drawcount, const GLint *basevertex)
{
    d_3_2_Core->MultiDrawElementsBaseVertex(mode, count, type, indices, drawcount, basevertex);
}

inline void QOpenGLFunctions_4_5_Core::glDrawElementsInstancedBaseVertex(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex)
{
    d_3_2_Core->DrawElementsInstancedBaseVertex(mode, count, type, indices, instancecount, basevertex);
}

inline void QOpenGLFunctions_4_5_Core::glDrawRangeElementsBaseVertex(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices, GLint basevertex)
{
    d_3_2_Core->DrawRangeElementsBaseVertex(mode, start, end, count, type, indices, basevertex);
}

inline void QOpenGLFunctions_4_5_Core::glDrawElementsBaseVertex(GLenum mode, GLsizei count, GLenum type, const void *indices, GLint basevertex)
{
    d_3_2_Core->DrawElementsBaseVertex(mode, count, type, indices, basevertex);
}


// OpenGL 3.3 core functions
inline void QOpenGLFunctions_4_5_Core::glVertexAttribP4uiv(GLuint index, GLenum type, GLboolean normalized, const GLuint *value)
{
    d_3_3_Core->VertexAttribP4uiv(index, type, normalized, value);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttribP4ui(GLuint index, GLenum type, GLboolean normalized, GLuint value)
{
    d_3_3_Core->VertexAttribP4ui(index, type, normalized, value);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttribP3uiv(GLuint index, GLenum type, GLboolean normalized, const GLuint *value)
{
    d_3_3_Core->VertexAttribP3uiv(index, type, normalized, value);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttribP3ui(GLuint index, GLenum type, GLboolean normalized, GLuint value)
{
    d_3_3_Core->VertexAttribP3ui(index, type, normalized, value);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttribP2uiv(GLuint index, GLenum type, GLboolean normalized, const GLuint *value)
{
    d_3_3_Core->VertexAttribP2uiv(index, type, normalized, value);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttribP2ui(GLuint index, GLenum type, GLboolean normalized, GLuint value)
{
    d_3_3_Core->VertexAttribP2ui(index, type, normalized, value);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttribP1uiv(GLuint index, GLenum type, GLboolean normalized, const GLuint *value)
{
    d_3_3_Core->VertexAttribP1uiv(index, type, normalized, value);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttribP1ui(GLuint index, GLenum type, GLboolean normalized, GLuint value)
{
    d_3_3_Core->VertexAttribP1ui(index, type, normalized, value);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttribDivisor(GLuint index, GLuint divisor)
{
    d_3_3_Core->VertexAttribDivisor(index, divisor);
}

inline void QOpenGLFunctions_4_5_Core::glGetQueryObjectui64v(GLuint id, GLenum pname, GLuint64 *params)
{
    d_3_3_Core->GetQueryObjectui64v(id, pname, params);
}

inline void QOpenGLFunctions_4_5_Core::glGetQueryObjecti64v(GLuint id, GLenum pname, GLint64 *params)
{
    d_3_3_Core->GetQueryObjecti64v(id, pname, params);
}

inline void QOpenGLFunctions_4_5_Core::glQueryCounter(GLuint id, GLenum target)
{
    d_3_3_Core->QueryCounter(id, target);
}

inline void QOpenGLFunctions_4_5_Core::glGetSamplerParameterIuiv(GLuint sampler, GLenum pname, GLuint *params)
{
    d_3_3_Core->GetSamplerParameterIuiv(sampler, pname, params);
}

inline void QOpenGLFunctions_4_5_Core::glGetSamplerParameterfv(GLuint sampler, GLenum pname, GLfloat *params)
{
    d_3_3_Core->GetSamplerParameterfv(sampler, pname, params);
}

inline void QOpenGLFunctions_4_5_Core::glGetSamplerParameterIiv(GLuint sampler, GLenum pname, GLint *params)
{
    d_3_3_Core->GetSamplerParameterIiv(sampler, pname, params);
}

inline void QOpenGLFunctions_4_5_Core::glGetSamplerParameteriv(GLuint sampler, GLenum pname, GLint *params)
{
    d_3_3_Core->GetSamplerParameteriv(sampler, pname, params);
}

inline void QOpenGLFunctions_4_5_Core::glSamplerParameterIuiv(GLuint sampler, GLenum pname, const GLuint *param)
{
    d_3_3_Core->SamplerParameterIuiv(sampler, pname, param);
}

inline void QOpenGLFunctions_4_5_Core::glSamplerParameterIiv(GLuint sampler, GLenum pname, const GLint *param)
{
    d_3_3_Core->SamplerParameterIiv(sampler, pname, param);
}

inline void QOpenGLFunctions_4_5_Core::glSamplerParameterfv(GLuint sampler, GLenum pname, const GLfloat *param)
{
    d_3_3_Core->SamplerParameterfv(sampler, pname, param);
}

inline void QOpenGLFunctions_4_5_Core::glSamplerParameterf(GLuint sampler, GLenum pname, GLfloat param)
{
    d_3_3_Core->SamplerParameterf(sampler, pname, param);
}

inline void QOpenGLFunctions_4_5_Core::glSamplerParameteriv(GLuint sampler, GLenum pname, const GLint *param)
{
    d_3_3_Core->SamplerParameteriv(sampler, pname, param);
}

inline void QOpenGLFunctions_4_5_Core::glSamplerParameteri(GLuint sampler, GLenum pname, GLint param)
{
    d_3_3_Core->SamplerParameteri(sampler, pname, param);
}

inline void QOpenGLFunctions_4_5_Core::glBindSampler(GLuint unit, GLuint sampler)
{
    d_3_3_Core->BindSampler(unit, sampler);
}

inline GLboolean QOpenGLFunctions_4_5_Core::glIsSampler(GLuint sampler)
{
    return d_3_3_Core->IsSampler(sampler);
}

inline void QOpenGLFunctions_4_5_Core::glDeleteSamplers(GLsizei count, const GLuint *samplers)
{
    d_3_3_Core->DeleteSamplers(count, samplers);
}

inline void QOpenGLFunctions_4_5_Core::glGenSamplers(GLsizei count, GLuint *samplers)
{
    d_3_3_Core->GenSamplers(count, samplers);
}

inline GLint QOpenGLFunctions_4_5_Core::glGetFragDataIndex(GLuint program, const GLchar *name)
{
    return d_3_3_Core->GetFragDataIndex(program, name);
}

inline void QOpenGLFunctions_4_5_Core::glBindFragDataLocationIndexed(GLuint program, GLuint colorNumber, GLuint index, const GLchar *name)
{
    d_3_3_Core->BindFragDataLocationIndexed(program, colorNumber, index, name);
}


// OpenGL 4.0 core functions
inline void QOpenGLFunctions_4_5_Core::glGetQueryIndexediv(GLenum target, GLuint index, GLenum pname, GLint *params)
{
    d_4_0_Core->GetQueryIndexediv(target, index, pname, params);
}

inline void QOpenGLFunctions_4_5_Core::glEndQueryIndexed(GLenum target, GLuint index)
{
    d_4_0_Core->EndQueryIndexed(target, index);
}

inline void QOpenGLFunctions_4_5_Core::glBeginQueryIndexed(GLenum target, GLuint index, GLuint id)
{
    d_4_0_Core->BeginQueryIndexed(target, index, id);
}

inline void QOpenGLFunctions_4_5_Core::glDrawTransformFeedbackStream(GLenum mode, GLuint id, GLuint stream)
{
    d_4_0_Core->DrawTransformFeedbackStream(mode, id, stream);
}

inline void QOpenGLFunctions_4_5_Core::glDrawTransformFeedback(GLenum mode, GLuint id)
{
    d_4_0_Core->DrawTransformFeedback(mode, id);
}

inline void QOpenGLFunctions_4_5_Core::glResumeTransformFeedback()
{
    d_4_0_Core->ResumeTransformFeedback();
}

inline void QOpenGLFunctions_4_5_Core::glPauseTransformFeedback()
{
    d_4_0_Core->PauseTransformFeedback();
}

inline GLboolean QOpenGLFunctions_4_5_Core::glIsTransformFeedback(GLuint id)
{
    return d_4_0_Core->IsTransformFeedback(id);
}

inline void QOpenGLFunctions_4_5_Core::glGenTransformFeedbacks(GLsizei n, GLuint *ids)
{
    d_4_0_Core->GenTransformFeedbacks(n, ids);
}

inline void QOpenGLFunctions_4_5_Core::glDeleteTransformFeedbacks(GLsizei n, const GLuint *ids)
{
    d_4_0_Core->DeleteTransformFeedbacks(n, ids);
}

inline void QOpenGLFunctions_4_5_Core::glBindTransformFeedback(GLenum target, GLuint id)
{
    d_4_0_Core->BindTransformFeedback(target, id);
}

inline void QOpenGLFunctions_4_5_Core::glPatchParameterfv(GLenum pname, const GLfloat *values)
{
    d_4_0_Core->PatchParameterfv(pname, values);
}

inline void QOpenGLFunctions_4_5_Core::glPatchParameteri(GLenum pname, GLint value)
{
    d_4_0_Core->PatchParameteri(pname, value);
}

inline void QOpenGLFunctions_4_5_Core::glGetProgramStageiv(GLuint program, GLenum shadertype, GLenum pname, GLint *values)
{
    d_4_0_Core->GetProgramStageiv(program, shadertype, pname, values);
}

inline void QOpenGLFunctions_4_5_Core::glGetUniformSubroutineuiv(GLenum shadertype, GLint location, GLuint *params)
{
    d_4_0_Core->GetUniformSubroutineuiv(shadertype, location, params);
}

inline void QOpenGLFunctions_4_5_Core::glUniformSubroutinesuiv(GLenum shadertype, GLsizei count, const GLuint *indices)
{
    d_4_0_Core->UniformSubroutinesuiv(shadertype, count, indices);
}

inline void QOpenGLFunctions_4_5_Core::glGetActiveSubroutineName(GLuint program, GLenum shadertype, GLuint index, GLsizei bufsize, GLsizei *length, GLchar *name)
{
    d_4_0_Core->GetActiveSubroutineName(program, shadertype, index, bufsize, length, name);
}

inline void QOpenGLFunctions_4_5_Core::glGetActiveSubroutineUniformName(GLuint program, GLenum shadertype, GLuint index, GLsizei bufsize, GLsizei *length, GLchar *name)
{
    d_4_0_Core->GetActiveSubroutineUniformName(program, shadertype, index, bufsize, length, name);
}

inline void QOpenGLFunctions_4_5_Core::glGetActiveSubroutineUniformiv(GLuint program, GLenum shadertype, GLuint index, GLenum pname, GLint *values)
{
    d_4_0_Core->GetActiveSubroutineUniformiv(program, shadertype, index, pname, values);
}

inline GLuint QOpenGLFunctions_4_5_Core::glGetSubroutineIndex(GLuint program, GLenum shadertype, const GLchar *name)
{
    return d_4_0_Core->GetSubroutineIndex(program, shadertype, name);
}

inline GLint QOpenGLFunctions_4_5_Core::glGetSubroutineUniformLocation(GLuint program, GLenum shadertype, const GLchar *name)
{
    return d_4_0_Core->GetSubroutineUniformLocation(program, shadertype, name);
}

inline void QOpenGLFunctions_4_5_Core::glGetUniformdv(GLuint program, GLint location, GLdouble *params)
{
    d_4_0_Core->GetUniformdv(program, location, params);
}

inline void QOpenGLFunctions_4_5_Core::glUniformMatrix4x3dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
    d_4_0_Core->UniformMatrix4x3dv(location, count, transpose, value);
}

inline void QOpenGLFunctions_4_5_Core::glUniformMatrix4x2dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
    d_4_0_Core->UniformMatrix4x2dv(location, count, transpose, value);
}

inline void QOpenGLFunctions_4_5_Core::glUniformMatrix3x4dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
    d_4_0_Core->UniformMatrix3x4dv(location, count, transpose, value);
}

inline void QOpenGLFunctions_4_5_Core::glUniformMatrix3x2dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
    d_4_0_Core->UniformMatrix3x2dv(location, count, transpose, value);
}

inline void QOpenGLFunctions_4_5_Core::glUniformMatrix2x4dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
    d_4_0_Core->UniformMatrix2x4dv(location, count, transpose, value);
}

inline void QOpenGLFunctions_4_5_Core::glUniformMatrix2x3dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
    d_4_0_Core->UniformMatrix2x3dv(location, count, transpose, value);
}

inline void QOpenGLFunctions_4_5_Core::glUniformMatrix4dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
    d_4_0_Core->UniformMatrix4dv(location, count, transpose, value);
}

inline void QOpenGLFunctions_4_5_Core::glUniformMatrix3dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
    d_4_0_Core->UniformMatrix3dv(location, count, transpose, value);
}

inline void QOpenGLFunctions_4_5_Core::glUniformMatrix2dv(GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
    d_4_0_Core->UniformMatrix2dv(location, count, transpose, value);
}

inline void QOpenGLFunctions_4_5_Core::glUniform4dv(GLint location, GLsizei count, const GLdouble *value)
{
    d_4_0_Core->Uniform4dv(location, count, value);
}

inline void QOpenGLFunctions_4_5_Core::glUniform3dv(GLint location, GLsizei count, const GLdouble *value)
{
    d_4_0_Core->Uniform3dv(location, count, value);
}

inline void QOpenGLFunctions_4_5_Core::glUniform2dv(GLint location, GLsizei count, const GLdouble *value)
{
    d_4_0_Core->Uniform2dv(location, count, value);
}

inline void QOpenGLFunctions_4_5_Core::glUniform1dv(GLint location, GLsizei count, const GLdouble *value)
{
    d_4_0_Core->Uniform1dv(location, count, value);
}

inline void QOpenGLFunctions_4_5_Core::glUniform4d(GLint location, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    d_4_0_Core->Uniform4d(location, x, y, z, w);
}

inline void QOpenGLFunctions_4_5_Core::glUniform3d(GLint location, GLdouble x, GLdouble y, GLdouble z)
{
    d_4_0_Core->Uniform3d(location, x, y, z);
}

inline void QOpenGLFunctions_4_5_Core::glUniform2d(GLint location, GLdouble x, GLdouble y)
{
    d_4_0_Core->Uniform2d(location, x, y);
}

inline void QOpenGLFunctions_4_5_Core::glUniform1d(GLint location, GLdouble x)
{
    d_4_0_Core->Uniform1d(location, x);
}

inline void QOpenGLFunctions_4_5_Core::glDrawElementsIndirect(GLenum mode, GLenum type, const void *indirect)
{
    d_4_0_Core->DrawElementsIndirect(mode, type, indirect);
}

inline void QOpenGLFunctions_4_5_Core::glDrawArraysIndirect(GLenum mode, const void *indirect)
{
    d_4_0_Core->DrawArraysIndirect(mode, indirect);
}

inline void QOpenGLFunctions_4_5_Core::glBlendFuncSeparatei(GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)
{
    d_4_0_Core->BlendFuncSeparatei(buf, srcRGB, dstRGB, srcAlpha, dstAlpha);
}

inline void QOpenGLFunctions_4_5_Core::glBlendFunci(GLuint buf, GLenum src, GLenum dst)
{
    d_4_0_Core->BlendFunci(buf, src, dst);
}

inline void QOpenGLFunctions_4_5_Core::glBlendEquationSeparatei(GLuint buf, GLenum modeRGB, GLenum modeAlpha)
{
    d_4_0_Core->BlendEquationSeparatei(buf, modeRGB, modeAlpha);
}

inline void QOpenGLFunctions_4_5_Core::glBlendEquationi(GLuint buf, GLenum mode)
{
    d_4_0_Core->BlendEquationi(buf, mode);
}

inline void QOpenGLFunctions_4_5_Core::glMinSampleShading(GLfloat value)
{
    d_4_0_Core->MinSampleShading(value);
}


// OpenGL 4.1 core functions
inline void QOpenGLFunctions_4_5_Core::glGetDoublei_v(GLenum target, GLuint index, GLdouble *data)
{
    d_4_1_Core->GetDoublei_v(target, index, data);
}

inline void QOpenGLFunctions_4_5_Core::glGetFloati_v(GLenum target, GLuint index, GLfloat *data)
{
    d_4_1_Core->GetFloati_v(target, index, data);
}

inline void QOpenGLFunctions_4_5_Core::glDepthRangeIndexed(GLuint index, GLdouble n, GLdouble f)
{
    d_4_1_Core->DepthRangeIndexed(index, n, f);
}

inline void QOpenGLFunctions_4_5_Core::glDepthRangeArrayv(GLuint first, GLsizei count, const GLdouble *v)
{
    d_4_1_Core->DepthRangeArrayv(first, count, v);
}

inline void QOpenGLFunctions_4_5_Core::glScissorIndexedv(GLuint index, const GLint *v)
{
    d_4_1_Core->ScissorIndexedv(index, v);
}

inline void QOpenGLFunctions_4_5_Core::glScissorIndexed(GLuint index, GLint left, GLint bottom, GLsizei width, GLsizei height)
{
    d_4_1_Core->ScissorIndexed(index, left, bottom, width, height);
}

inline void QOpenGLFunctions_4_5_Core::glScissorArrayv(GLuint first, GLsizei count, const GLint *v)
{
    d_4_1_Core->ScissorArrayv(first, count, v);
}

inline void QOpenGLFunctions_4_5_Core::glViewportIndexedfv(GLuint index, const GLfloat *v)
{
    d_4_1_Core->ViewportIndexedfv(index, v);
}

inline void QOpenGLFunctions_4_5_Core::glViewportIndexedf(GLuint index, GLfloat x, GLfloat y, GLfloat w, GLfloat h)
{
    d_4_1_Core->ViewportIndexedf(index, x, y, w, h);
}

inline void QOpenGLFunctions_4_5_Core::glViewportArrayv(GLuint first, GLsizei count, const GLfloat *v)
{
    d_4_1_Core->ViewportArrayv(first, count, v);
}

inline void QOpenGLFunctions_4_5_Core::glGetVertexAttribLdv(GLuint index, GLenum pname, GLdouble *params)
{
    d_4_1_Core->GetVertexAttribLdv(index, pname, params);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttribLPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer)
{
    d_4_1_Core->VertexAttribLPointer(index, size, type, stride, pointer);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttribL4dv(GLuint index, const GLdouble *v)
{
    d_4_1_Core->VertexAttribL4dv(index, v);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttribL3dv(GLuint index, const GLdouble *v)
{
    d_4_1_Core->VertexAttribL3dv(index, v);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttribL2dv(GLuint index, const GLdouble *v)
{
    d_4_1_Core->VertexAttribL2dv(index, v);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttribL1dv(GLuint index, const GLdouble *v)
{
    d_4_1_Core->VertexAttribL1dv(index, v);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttribL4d(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    d_4_1_Core->VertexAttribL4d(index, x, y, z, w);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttribL3d(GLuint index, GLdouble x, GLdouble y, GLdouble z)
{
    d_4_1_Core->VertexAttribL3d(index, x, y, z);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttribL2d(GLuint index, GLdouble x, GLdouble y)
{
    d_4_1_Core->VertexAttribL2d(index, x, y);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttribL1d(GLuint index, GLdouble x)
{
    d_4_1_Core->VertexAttribL1d(index, x);
}

inline void QOpenGLFunctions_4_5_Core::glGetProgramPipelineInfoLog(GLuint pipeline, GLsizei bufSize, GLsizei *length, GLchar *infoLog)
{
    d_4_1_Core->GetProgramPipelineInfoLog(pipeline, bufSize, length, infoLog);
}

inline void QOpenGLFunctions_4_5_Core::glValidateProgramPipeline(GLuint pipeline)
{
    d_4_1_Core->ValidateProgramPipeline(pipeline);
}

inline void QOpenGLFunctions_4_5_Core::glProgramUniformMatrix4x3dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
    d_4_1_Core->ProgramUniformMatrix4x3dv(program, location, count, transpose, value);
}

inline void QOpenGLFunctions_4_5_Core::glProgramUniformMatrix3x4dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
    d_4_1_Core->ProgramUniformMatrix3x4dv(program, location, count, transpose, value);
}

inline void QOpenGLFunctions_4_5_Core::glProgramUniformMatrix4x2dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
    d_4_1_Core->ProgramUniformMatrix4x2dv(program, location, count, transpose, value);
}

inline void QOpenGLFunctions_4_5_Core::glProgramUniformMatrix2x4dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
    d_4_1_Core->ProgramUniformMatrix2x4dv(program, location, count, transpose, value);
}

inline void QOpenGLFunctions_4_5_Core::glProgramUniformMatrix3x2dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
    d_4_1_Core->ProgramUniformMatrix3x2dv(program, location, count, transpose, value);
}

inline void QOpenGLFunctions_4_5_Core::glProgramUniformMatrix2x3dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
    d_4_1_Core->ProgramUniformMatrix2x3dv(program, location, count, transpose, value);
}

inline void QOpenGLFunctions_4_5_Core::glProgramUniformMatrix4x3fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    d_4_1_Core->ProgramUniformMatrix4x3fv(program, location, count, transpose, value);
}

inline void QOpenGLFunctions_4_5_Core::glProgramUniformMatrix3x4fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    d_4_1_Core->ProgramUniformMatrix3x4fv(program, location, count, transpose, value);
}

inline void QOpenGLFunctions_4_5_Core::glProgramUniformMatrix4x2fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    d_4_1_Core->ProgramUniformMatrix4x2fv(program, location, count, transpose, value);
}

inline void QOpenGLFunctions_4_5_Core::glProgramUniformMatrix2x4fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    d_4_1_Core->ProgramUniformMatrix2x4fv(program, location, count, transpose, value);
}

inline void QOpenGLFunctions_4_5_Core::glProgramUniformMatrix3x2fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    d_4_1_Core->ProgramUniformMatrix3x2fv(program, location, count, transpose, value);
}

inline void QOpenGLFunctions_4_5_Core::glProgramUniformMatrix2x3fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    d_4_1_Core->ProgramUniformMatrix2x3fv(program, location, count, transpose, value);
}

inline void QOpenGLFunctions_4_5_Core::glProgramUniformMatrix4dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
    d_4_1_Core->ProgramUniformMatrix4dv(program, location, count, transpose, value);
}

inline void QOpenGLFunctions_4_5_Core::glProgramUniformMatrix3dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
    d_4_1_Core->ProgramUniformMatrix3dv(program, location, count, transpose, value);
}

inline void QOpenGLFunctions_4_5_Core::glProgramUniformMatrix2dv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)
{
    d_4_1_Core->ProgramUniformMatrix2dv(program, location, count, transpose, value);
}

inline void QOpenGLFunctions_4_5_Core::glProgramUniformMatrix4fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    d_4_1_Core->ProgramUniformMatrix4fv(program, location, count, transpose, value);
}

inline void QOpenGLFunctions_4_5_Core::glProgramUniformMatrix3fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    d_4_1_Core->ProgramUniformMatrix3fv(program, location, count, transpose, value);
}

inline void QOpenGLFunctions_4_5_Core::glProgramUniformMatrix2fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    d_4_1_Core->ProgramUniformMatrix2fv(program, location, count, transpose, value);
}

inline void QOpenGLFunctions_4_5_Core::glProgramUniform4uiv(GLuint program, GLint location, GLsizei count, const GLuint *value)
{
    d_4_1_Core->ProgramUniform4uiv(program, location, count, value);
}

inline void QOpenGLFunctions_4_5_Core::glProgramUniform4ui(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
    d_4_1_Core->ProgramUniform4ui(program, location, v0, v1, v2, v3);
}

inline void QOpenGLFunctions_4_5_Core::glProgramUniform4dv(GLuint program, GLint location, GLsizei count, const GLdouble *value)
{
    d_4_1_Core->ProgramUniform4dv(program, location, count, value);
}

inline void QOpenGLFunctions_4_5_Core::glProgramUniform4d(GLuint program, GLint location, GLdouble v0, GLdouble v1, GLdouble v2, GLdouble v3)
{
    d_4_1_Core->ProgramUniform4d(program, location, v0, v1, v2, v3);
}

inline void QOpenGLFunctions_4_5_Core::glProgramUniform4fv(GLuint program, GLint location, GLsizei count, const GLfloat *value)
{
    d_4_1_Core->ProgramUniform4fv(program, location, count, value);
}

inline void QOpenGLFunctions_4_5_Core::glProgramUniform4f(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
    d_4_1_Core->ProgramUniform4f(program, location, v0, v1, v2, v3);
}

inline void QOpenGLFunctions_4_5_Core::glProgramUniform4iv(GLuint program, GLint location, GLsizei count, const GLint *value)
{
    d_4_1_Core->ProgramUniform4iv(program, location, count, value);
}

inline void QOpenGLFunctions_4_5_Core::glProgramUniform4i(GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3)
{
    d_4_1_Core->ProgramUniform4i(program, location, v0, v1, v2, v3);
}

inline void QOpenGLFunctions_4_5_Core::glProgramUniform3uiv(GLuint program, GLint location, GLsizei count, const GLuint *value)
{
    d_4_1_Core->ProgramUniform3uiv(program, location, count, value);
}

inline void QOpenGLFunctions_4_5_Core::glProgramUniform3ui(GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2)
{
    d_4_1_Core->ProgramUniform3ui(program, location, v0, v1, v2);
}

inline void QOpenGLFunctions_4_5_Core::glProgramUniform3dv(GLuint program, GLint location, GLsizei count, const GLdouble *value)
{
    d_4_1_Core->ProgramUniform3dv(program, location, count, value);
}

inline void QOpenGLFunctions_4_5_Core::glProgramUniform3d(GLuint program, GLint location, GLdouble v0, GLdouble v1, GLdouble v2)
{
    d_4_1_Core->ProgramUniform3d(program, location, v0, v1, v2);
}

inline void QOpenGLFunctions_4_5_Core::glProgramUniform3fv(GLuint program, GLint location, GLsizei count, const GLfloat *value)
{
    d_4_1_Core->ProgramUniform3fv(program, location, count, value);
}

inline void QOpenGLFunctions_4_5_Core::glProgramUniform3f(GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
{
    d_4_1_Core->ProgramUniform3f(program, location, v0, v1, v2);
}

inline void QOpenGLFunctions_4_5_Core::glProgramUniform3iv(GLuint program, GLint location, GLsizei count, const GLint *value)
{
    d_4_1_Core->ProgramUniform3iv(program, location, count, value);
}

inline void QOpenGLFunctions_4_5_Core::glProgramUniform3i(GLuint program, GLint location, GLint v0, GLint v1, GLint v2)
{
    d_4_1_Core->ProgramUniform3i(program, location, v0, v1, v2);
}

inline void QOpenGLFunctions_4_5_Core::glProgramUniform2uiv(GLuint program, GLint location, GLsizei count, const GLuint *value)
{
    d_4_1_Core->ProgramUniform2uiv(program, location, count, value);
}

inline void QOpenGLFunctions_4_5_Core::glProgramUniform2ui(GLuint program, GLint location, GLuint v0, GLuint v1)
{
    d_4_1_Core->ProgramUniform2ui(program, location, v0, v1);
}

inline void QOpenGLFunctions_4_5_Core::glProgramUniform2dv(GLuint program, GLint location, GLsizei count, const GLdouble *value)
{
    d_4_1_Core->ProgramUniform2dv(program, location, count, value);
}

inline void QOpenGLFunctions_4_5_Core::glProgramUniform2d(GLuint program, GLint location, GLdouble v0, GLdouble v1)
{
    d_4_1_Core->ProgramUniform2d(program, location, v0, v1);
}

inline void QOpenGLFunctions_4_5_Core::glProgramUniform2fv(GLuint program, GLint location, GLsizei count, const GLfloat *value)
{
    d_4_1_Core->ProgramUniform2fv(program, location, count, value);
}

inline void QOpenGLFunctions_4_5_Core::glProgramUniform2f(GLuint program, GLint location, GLfloat v0, GLfloat v1)
{
    d_4_1_Core->ProgramUniform2f(program, location, v0, v1);
}

inline void QOpenGLFunctions_4_5_Core::glProgramUniform2iv(GLuint program, GLint location, GLsizei count, const GLint *value)
{
    d_4_1_Core->ProgramUniform2iv(program, location, count, value);
}

inline void QOpenGLFunctions_4_5_Core::glProgramUniform2i(GLuint program, GLint location, GLint v0, GLint v1)
{
    d_4_1_Core->ProgramUniform2i(program, location, v0, v1);
}

inline void QOpenGLFunctions_4_5_Core::glProgramUniform1uiv(GLuint program, GLint location, GLsizei count, const GLuint *value)
{
    d_4_1_Core->ProgramUniform1uiv(program, location, count, value);
}

inline void QOpenGLFunctions_4_5_Core::glProgramUniform1ui(GLuint program, GLint location, GLuint v0)
{
    d_4_1_Core->ProgramUniform1ui(program, location, v0);
}

inline void QOpenGLFunctions_4_5_Core::glProgramUniform1dv(GLuint program, GLint location, GLsizei count, const GLdouble *value)
{
    d_4_1_Core->ProgramUniform1dv(program, location, count, value);
}

inline void QOpenGLFunctions_4_5_Core::glProgramUniform1d(GLuint program, GLint location, GLdouble v0)
{
    d_4_1_Core->ProgramUniform1d(program, location, v0);
}

inline void QOpenGLFunctions_4_5_Core::glProgramUniform1fv(GLuint program, GLint location, GLsizei count, const GLfloat *value)
{
    d_4_1_Core->ProgramUniform1fv(program, location, count, value);
}

inline void QOpenGLFunctions_4_5_Core::glProgramUniform1f(GLuint program, GLint location, GLfloat v0)
{
    d_4_1_Core->ProgramUniform1f(program, location, v0);
}

inline void QOpenGLFunctions_4_5_Core::glProgramUniform1iv(GLuint program, GLint location, GLsizei count, const GLint *value)
{
    d_4_1_Core->ProgramUniform1iv(program, location, count, value);
}

inline void QOpenGLFunctions_4_5_Core::glProgramUniform1i(GLuint program, GLint location, GLint v0)
{
    d_4_1_Core->ProgramUniform1i(program, location, v0);
}

inline void QOpenGLFunctions_4_5_Core::glGetProgramPipelineiv(GLuint pipeline, GLenum pname, GLint *params)
{
    d_4_1_Core->GetProgramPipelineiv(pipeline, pname, params);
}

inline GLboolean QOpenGLFunctions_4_5_Core::glIsProgramPipeline(GLuint pipeline)
{
    return d_4_1_Core->IsProgramPipeline(pipeline);
}

inline void QOpenGLFunctions_4_5_Core::glGenProgramPipelines(GLsizei n, GLuint *pipelines)
{
    d_4_1_Core->GenProgramPipelines(n, pipelines);
}

inline void QOpenGLFunctions_4_5_Core::glDeleteProgramPipelines(GLsizei n, const GLuint *pipelines)
{
    d_4_1_Core->DeleteProgramPipelines(n, pipelines);
}

inline void QOpenGLFunctions_4_5_Core::glBindProgramPipeline(GLuint pipeline)
{
    d_4_1_Core->BindProgramPipeline(pipeline);
}

inline GLuint QOpenGLFunctions_4_5_Core::glCreateShaderProgramv(GLenum type, GLsizei count, const GLchar *const *strings)
{
    return d_4_1_Core->CreateShaderProgramv(type, count, strings);
}

inline void QOpenGLFunctions_4_5_Core::glActiveShaderProgram(GLuint pipeline, GLuint program)
{
    d_4_1_Core->ActiveShaderProgram(pipeline, program);
}

inline void QOpenGLFunctions_4_5_Core::glUseProgramStages(GLuint pipeline, GLbitfield stages, GLuint program)
{
    d_4_1_Core->UseProgramStages(pipeline, stages, program);
}

inline void QOpenGLFunctions_4_5_Core::glProgramParameteri(GLuint program, GLenum pname, GLint value)
{
    d_4_1_Core->ProgramParameteri(program, pname, value);
}

inline void QOpenGLFunctions_4_5_Core::glProgramBinary(GLuint program, GLenum binaryFormat, const void *binary, GLsizei length)
{
    d_4_1_Core->ProgramBinary(program, binaryFormat, binary, length);
}

inline void QOpenGLFunctions_4_5_Core::glGetProgramBinary(GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, void *binary)
{
    d_4_1_Core->GetProgramBinary(program, bufSize, length, binaryFormat, binary);
}

inline void QOpenGLFunctions_4_5_Core::glClearDepthf(GLfloat dd)
{
    d_4_1_Core->ClearDepthf(dd);
}

inline void QOpenGLFunctions_4_5_Core::glDepthRangef(GLfloat n, GLfloat f)
{
    d_4_1_Core->DepthRangef(n, f);
}

inline void QOpenGLFunctions_4_5_Core::glGetShaderPrecisionFormat(GLenum shadertype, GLenum precisiontype, GLint *range, GLint *precision)
{
    d_4_1_Core->GetShaderPrecisionFormat(shadertype, precisiontype, range, precision);
}

inline void QOpenGLFunctions_4_5_Core::glShaderBinary(GLsizei count, const GLuint *shaders, GLenum binaryformat, const void *binary, GLsizei length)
{
    d_4_1_Core->ShaderBinary(count, shaders, binaryformat, binary, length);
}

inline void QOpenGLFunctions_4_5_Core::glReleaseShaderCompiler()
{
    d_4_1_Core->ReleaseShaderCompiler();
}


// OpenGL 4.2 core functions
inline void QOpenGLFunctions_4_5_Core::glDrawTransformFeedbackStreamInstanced(GLenum mode, GLuint id, GLuint stream, GLsizei instancecount)
{
    d_4_2_Core->DrawTransformFeedbackStreamInstanced(mode, id, stream, instancecount);
}

inline void QOpenGLFunctions_4_5_Core::glDrawTransformFeedbackInstanced(GLenum mode, GLuint id, GLsizei instancecount)
{
    d_4_2_Core->DrawTransformFeedbackInstanced(mode, id, instancecount);
}

inline void QOpenGLFunctions_4_5_Core::glTexStorage3D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth)
{
    d_4_2_Core->TexStorage3D(target, levels, internalformat, width, height, depth);
}

inline void QOpenGLFunctions_4_5_Core::glTexStorage2D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)
{
    d_4_2_Core->TexStorage2D(target, levels, internalformat, width, height);
}

inline void QOpenGLFunctions_4_5_Core::glTexStorage1D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width)
{
    d_4_2_Core->TexStorage1D(target, levels, internalformat, width);
}

inline void QOpenGLFunctions_4_5_Core::glMemoryBarrier(GLbitfield barriers)
{
    d_4_2_Core->MemoryBarrier(barriers);
}

inline void QOpenGLFunctions_4_5_Core::glBindImageTexture(GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format)
{
    d_4_2_Core->BindImageTexture(unit, texture, level, layered, layer, access, format);
}

inline void QOpenGLFunctions_4_5_Core::glGetActiveAtomicCounterBufferiv(GLuint program, GLuint bufferIndex, GLenum pname, GLint *params)
{
    d_4_2_Core->GetActiveAtomicCounterBufferiv(program, bufferIndex, pname, params);
}

inline void QOpenGLFunctions_4_5_Core::glGetInternalformativ(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint *params)
{
    d_4_2_Core->GetInternalformativ(target, internalformat, pname, bufSize, params);
}

inline void QOpenGLFunctions_4_5_Core::glDrawElementsInstancedBaseVertexBaseInstance(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex, GLuint baseinstance)
{
    d_4_2_Core->DrawElementsInstancedBaseVertexBaseInstance(mode, count, type, indices, instancecount, basevertex, baseinstance);
}

inline void QOpenGLFunctions_4_5_Core::glDrawElementsInstancedBaseInstance(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLuint baseinstance)
{
    d_4_2_Core->DrawElementsInstancedBaseInstance(mode, count, type, indices, instancecount, baseinstance);
}

inline void QOpenGLFunctions_4_5_Core::glDrawArraysInstancedBaseInstance(GLenum mode, GLint first, GLsizei count, GLsizei instancecount, GLuint baseinstance)
{
    d_4_2_Core->DrawArraysInstancedBaseInstance(mode, first, count, instancecount, baseinstance);
}


// OpenGL 4.3 core functions
inline void QOpenGLFunctions_4_5_Core::glGetObjectPtrLabel(const void *ptr, GLsizei bufSize, GLsizei *length, GLchar *label)
{
    d_4_3_Core->GetObjectPtrLabel(ptr, bufSize, length, label);
}

inline void QOpenGLFunctions_4_5_Core::glObjectPtrLabel(const void *ptr, GLsizei length, const GLchar *label)
{
    d_4_3_Core->ObjectPtrLabel(ptr, length, label);
}

inline void QOpenGLFunctions_4_5_Core::glGetObjectLabel(GLenum identifier, GLuint name, GLsizei bufSize, GLsizei *length, GLchar *label)
{
    d_4_3_Core->GetObjectLabel(identifier, name, bufSize, length, label);
}

inline void QOpenGLFunctions_4_5_Core::glObjectLabel(GLenum identifier, GLuint name, GLsizei length, const GLchar *label)
{
    d_4_3_Core->ObjectLabel(identifier, name, length, label);
}

inline void QOpenGLFunctions_4_5_Core::glPopDebugGroup()
{
    d_4_3_Core->PopDebugGroup();
}

inline void QOpenGLFunctions_4_5_Core::glPushDebugGroup(GLenum source, GLuint id, GLsizei length, const GLchar *message)
{
    d_4_3_Core->PushDebugGroup(source, id, length, message);
}

inline GLuint QOpenGLFunctions_4_5_Core::glGetDebugMessageLog(GLuint count, GLsizei bufSize, GLenum *sources, GLenum *types, GLuint *ids, GLenum *severities, GLsizei *lengths, GLchar *messageLog)
{
    return d_4_3_Core->GetDebugMessageLog(count, bufSize, sources, types, ids, severities, lengths, messageLog);
}

inline void QOpenGLFunctions_4_5_Core::glDebugMessageCallback(GLDEBUGPROC callback, const void *userParam)
{
    d_4_3_Core->DebugMessageCallback(callback, userParam);
}

inline void QOpenGLFunctions_4_5_Core::glDebugMessageInsert(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *buf)
{
    d_4_3_Core->DebugMessageInsert(source, type, id, severity, length, buf);
}

inline void QOpenGLFunctions_4_5_Core::glDebugMessageControl(GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint *ids, GLboolean enabled)
{
    d_4_3_Core->DebugMessageControl(source, type, severity, count, ids, enabled);
}

inline void QOpenGLFunctions_4_5_Core::glVertexBindingDivisor(GLuint bindingindex, GLuint divisor)
{
    d_4_3_Core->VertexBindingDivisor(bindingindex, divisor);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttribBinding(GLuint attribindex, GLuint bindingindex)
{
    d_4_3_Core->VertexAttribBinding(attribindex, bindingindex);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttribLFormat(GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset)
{
    d_4_3_Core->VertexAttribLFormat(attribindex, size, type, relativeoffset);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttribIFormat(GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset)
{
    d_4_3_Core->VertexAttribIFormat(attribindex, size, type, relativeoffset);
}

inline void QOpenGLFunctions_4_5_Core::glVertexAttribFormat(GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset)
{
    d_4_3_Core->VertexAttribFormat(attribindex, size, type, normalized, relativeoffset);
}

inline void QOpenGLFunctions_4_5_Core::glBindVertexBuffer(GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride)
{
    d_4_3_Core->BindVertexBuffer(bindingindex, buffer, offset, stride);
}

inline void QOpenGLFunctions_4_5_Core::glTextureView(GLuint texture, GLenum target, GLuint origtexture, GLenum internalformat, GLuint minlevel, GLuint numlevels, GLuint minlayer, GLuint numlayers)
{
    d_4_3_Core->TextureView(texture, target, origtexture, internalformat, minlevel, numlevels, minlayer, numlayers);
}

inline void QOpenGLFunctions_4_5_Core::glTexStorage3DMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations)
{
    d_4_3_Core->TexStorage3DMultisample(target, samples, internalformat, width, height, depth, fixedsamplelocations);
}

inline void QOpenGLFunctions_4_5_Core::glTexStorage2DMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations)
{
    d_4_3_Core->TexStorage2DMultisample(target, samples, internalformat, width, height, fixedsamplelocations);
}

inline void QOpenGLFunctions_4_5_Core::glTexBufferRange(GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size)
{
    d_4_3_Core->TexBufferRange(target, internalformat, buffer, offset, size);
}

inline void QOpenGLFunctions_4_5_Core::glShaderStorageBlockBinding(GLuint program, GLuint storageBlockIndex, GLuint storageBlockBinding)
{
    d_4_3_Core->ShaderStorageBlockBinding(program, storageBlockIndex, storageBlockBinding);
}

inline GLint QOpenGLFunctions_4_5_Core::glGetProgramResourceLocationIndex(GLuint program, GLenum programInterface, const GLchar *name)
{
    return d_4_3_Core->GetProgramResourceLocationIndex(program, programInterface, name);
}

inline GLint QOpenGLFunctions_4_5_Core::glGetProgramResourceLocation(GLuint program, GLenum programInterface, const GLchar *name)
{
    return d_4_3_Core->GetProgramResourceLocation(program, programInterface, name);
}

inline void QOpenGLFunctions_4_5_Core::glGetProgramResourceiv(GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum *props, GLsizei bufSize, GLsizei *length, GLint *params)
{
    d_4_3_Core->GetProgramResourceiv(program, programInterface, index, propCount, props, bufSize, length, params);
}

inline void QOpenGLFunctions_4_5_Core::glGetProgramResourceName(GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name)
{
    d_4_3_Core->GetProgramResourceName(program, programInterface, index, bufSize, length, name);
}

inline GLuint QOpenGLFunctions_4_5_Core::glGetProgramResourceIndex(GLuint program, GLenum programInterface, const GLchar *name)
{
    return d_4_3_Core->GetProgramResourceIndex(program, programInterface, name);
}

inline void QOpenGLFunctions_4_5_Core::glGetProgramInterfaceiv(GLuint program, GLenum programInterface, GLenum pname, GLint *params)
{
    d_4_3_Core->GetProgramInterfaceiv(program, programInterface, pname, params);
}

inline void QOpenGLFunctions_4_5_Core::glMultiDrawElementsIndirect(GLenum mode, GLenum type, const void *indirect, GLsizei drawcount, GLsizei stride)
{
    d_4_3_Core->MultiDrawElementsIndirect(mode, type, indirect, drawcount, stride);
}

inline void QOpenGLFunctions_4_5_Core::glMultiDrawArraysIndirect(GLenum mode, const void *indirect, GLsizei drawcount, GLsizei stride)
{
    d_4_3_Core->MultiDrawArraysIndirect(mode, indirect, drawcount, stride);
}

inline void QOpenGLFunctions_4_5_Core::glInvalidateSubFramebuffer(GLenum target, GLsizei numAttachments, const GLenum *attachments, GLint x, GLint y, GLsizei width, GLsizei height)
{
    d_4_3_Core->InvalidateSubFramebuffer(target, numAttachments, attachments, x, y, width, height);
}

inline void QOpenGLFunctions_4_5_Core::glInvalidateFramebuffer(GLenum target, GLsizei numAttachments, const GLenum *attachments)
{
    d_4_3_Core->InvalidateFramebuffer(target, numAttachments, attachments);
}

inline void QOpenGLFunctions_4_5_Core::glInvalidateBufferData(GLuint buffer)
{
    d_4_3_Core->InvalidateBufferData(buffer);
}

inline void QOpenGLFunctions_4_5_Core::glInvalidateBufferSubData(GLuint buffer, GLintptr offset, GLsizeiptr length)
{
    d_4_3_Core->InvalidateBufferSubData(buffer, offset, length);
}

inline void QOpenGLFunctions_4_5_Core::glInvalidateTexImage(GLuint texture, GLint level)
{
    d_4_3_Core->InvalidateTexImage(texture, level);
}

inline void QOpenGLFunctions_4_5_Core::glInvalidateTexSubImage(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth)
{
    d_4_3_Core->InvalidateTexSubImage(texture, level, xoffset, yoffset, zoffset, width, height, depth);
}

inline void QOpenGLFunctions_4_5_Core::glGetInternalformati64v(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint64 *params)
{
    d_4_3_Core->GetInternalformati64v(target, internalformat, pname, bufSize, params);
}

inline void QOpenGLFunctions_4_5_Core::glGetFramebufferParameteriv(GLenum target, GLenum pname, GLint *params)
{
    d_4_3_Core->GetFramebufferParameteriv(target, pname, params);
}

inline void QOpenGLFunctions_4_5_Core::glFramebufferParameteri(GLenum target, GLenum pname, GLint param)
{
    d_4_3_Core->FramebufferParameteri(target, pname, param);
}

inline void QOpenGLFunctions_4_5_Core::glCopyImageSubData(GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth)
{
    d_4_3_Core->CopyImageSubData(srcName, srcTarget, srcLevel, srcX, srcY, srcZ, dstName, dstTarget, dstLevel, dstX, dstY, dstZ, srcWidth, srcHeight, srcDepth);
}

inline void QOpenGLFunctions_4_5_Core::glDispatchComputeIndirect(GLintptr indirect)
{
    d_4_3_Core->DispatchComputeIndirect(indirect);
}

inline void QOpenGLFunctions_4_5_Core::glDispatchCompute(GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z)
{
    d_4_3_Core->DispatchCompute(num_groups_x, num_groups_y, num_groups_z);
}

inline void QOpenGLFunctions_4_5_Core::glClearBufferSubData(GLenum target, GLenum internalformat, GLintptr offset, GLsizeiptr size, GLenum format, GLenum type, const void *data)
{
    d_4_3_Core->ClearBufferSubData(target, internalformat, offset, size, format, type, data);
}

inline void QOpenGLFunctions_4_5_Core::glClearBufferData(GLenum target, GLenum internalformat, GLenum format, GLenum type, const void *data)
{
    d_4_3_Core->ClearBufferData(target, internalformat, format, type, data);
}


// OpenGL 4.4 core functions
inline void QOpenGLFunctions_4_5_Core::glBindVertexBuffers(GLuint first, GLsizei count, const GLuint *buffers, const GLintptr *offsets, const GLsizei *strides)
{
    d_4_4_Core->BindVertexBuffers(first, count, buffers, offsets, strides);
}

inline void QOpenGLFunctions_4_5_Core::glBindImageTextures(GLuint first, GLsizei count, const GLuint *textures)
{
    d_4_4_Core->BindImageTextures(first, count, textures);
}

inline void QOpenGLFunctions_4_5_Core::glBindSamplers(GLuint first, GLsizei count, const GLuint *samplers)
{
    d_4_4_Core->BindSamplers(first, count, samplers);
}

inline void QOpenGLFunctions_4_5_Core::glBindTextures(GLuint first, GLsizei count, const GLuint *textures)
{
    d_4_4_Core->BindTextures(first, count, textures);
}

inline void QOpenGLFunctions_4_5_Core::glBindBuffersRange(GLenum target, GLuint first, GLsizei count, const GLuint *buffers, const GLintptr *offsets, const GLsizeiptr *sizes)
{
    d_4_4_Core->BindBuffersRange(target, first, count, buffers, offsets, sizes);
}

inline void QOpenGLFunctions_4_5_Core::glBindBuffersBase(GLenum target, GLuint first, GLsizei count, const GLuint *buffers)
{
    d_4_4_Core->BindBuffersBase(target, first, count, buffers);
}

inline void QOpenGLFunctions_4_5_Core::glClearTexSubImage(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *data)
{
    d_4_4_Core->ClearTexSubImage(texture, level, xoffset, yoffset, zoffset, width, height, depth, format, type, data);
}

inline void QOpenGLFunctions_4_5_Core::glClearTexImage(GLuint texture, GLint level, GLenum format, GLenum type, const void *data)
{
    d_4_4_Core->ClearTexImage(texture, level, format, type, data);
}

inline void QOpenGLFunctions_4_5_Core::glBufferStorage(GLenum target, GLsizeiptr size, const void *data, GLbitfield flags)
{
    d_4_4_Core->BufferStorage(target, size, data, flags);
}


// OpenGL 4.5 core functions
inline void QOpenGLFunctions_4_5_Core::glTextureBarrier()
{
    d_4_5_Core->TextureBarrier();
}

inline void QOpenGLFunctions_4_5_Core::glReadnPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, void *data)
{
    d_4_5_Core->ReadnPixels(x, y, width, height, format, type, bufSize, data);
}

inline void QOpenGLFunctions_4_5_Core::glGetnUniformuiv(GLuint program, GLint location, GLsizei bufSize, GLuint *params)
{
    d_4_5_Core->GetnUniformuiv(program, location, bufSize, params);
}

inline void QOpenGLFunctions_4_5_Core::glGetnUniformiv(GLuint program, GLint location, GLsizei bufSize, GLint *params)
{
    d_4_5_Core->GetnUniformiv(program, location, bufSize, params);
}

inline void QOpenGLFunctions_4_5_Core::glGetnUniformfv(GLuint program, GLint location, GLsizei bufSize, GLfloat *params)
{
    d_4_5_Core->GetnUniformfv(program, location, bufSize, params);
}

inline void QOpenGLFunctions_4_5_Core::glGetnUniformdv(GLuint program, GLint location, GLsizei bufSize, GLdouble *params)
{
    d_4_5_Core->GetnUniformdv(program, location, bufSize, params);
}

inline void QOpenGLFunctions_4_5_Core::glGetnTexImage(GLenum target, GLint level, GLenum format, GLenum type, GLsizei bufSize, void *pixels)
{
    d_4_5_Core->GetnTexImage(target, level, format, type, bufSize, pixels);
}

inline void QOpenGLFunctions_4_5_Core::glGetnCompressedTexImage(GLenum target, GLint lod, GLsizei bufSize, void *pixels)
{
    d_4_5_Core->GetnCompressedTexImage(target, lod, bufSize, pixels);
}

inline GLenum QOpenGLFunctions_4_5_Core::glGetGraphicsResetStatus()
{
    return d_4_5_Core->GetGraphicsResetStatus();
}

inline void QOpenGLFunctions_4_5_Core::glGetCompressedTextureSubImage(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLsizei bufSize, void *pixels)
{
    d_4_5_Core->GetCompressedTextureSubImage(texture, level, xoffset, yoffset, zoffset, width, height, depth, bufSize, pixels);
}

inline void QOpenGLFunctions_4_5_Core::glGetTextureSubImage(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, GLsizei bufSize, void *pixels)
{
    d_4_5_Core->GetTextureSubImage(texture, level, xoffset, yoffset, zoffset, width, height, depth, format, type, bufSize, pixels);
}

inline void QOpenGLFunctions_4_5_Core::glMemoryBarrierByRegion(GLbitfield barriers)
{
    d_4_5_Core->MemoryBarrierByRegion(barriers);
}

inline void QOpenGLFunctions_4_5_Core::glCreateQueries(GLenum target, GLsizei n, GLuint *ids)
{
    d_4_5_Core->CreateQueries(target, n, ids);
}

inline void QOpenGLFunctions_4_5_Core::glCreateProgramPipelines(GLsizei n, GLuint *pipelines)
{
    d_4_5_Core->CreateProgramPipelines(n, pipelines);
}

inline void QOpenGLFunctions_4_5_Core::glCreateSamplers(GLsizei n, GLuint *samplers)
{
    d_4_5_Core->CreateSamplers(n, samplers);
}

inline void QOpenGLFunctions_4_5_Core::glGetVertexArrayIndexed64iv(GLuint vaobj, GLuint index, GLenum pname, GLint64 *param)
{
    d_4_5_Core->GetVertexArrayIndexed64iv(vaobj, index, pname, param);
}

inline void QOpenGLFunctions_4_5_Core::glGetVertexArrayIndexediv(GLuint vaobj, GLuint index, GLenum pname, GLint *param)
{
    d_4_5_Core->GetVertexArrayIndexediv(vaobj, index, pname, param);
}

inline void QOpenGLFunctions_4_5_Core::glGetVertexArrayiv(GLuint vaobj, GLenum pname, GLint *param)
{
    d_4_5_Core->GetVertexArrayiv(vaobj, pname, param);
}

inline void QOpenGLFunctions_4_5_Core::glVertexArrayBindingDivisor(GLuint vaobj, GLuint bindingindex, GLuint divisor)
{
    d_4_5_Core->VertexArrayBindingDivisor(vaobj, bindingindex, divisor);
}

inline void QOpenGLFunctions_4_5_Core::glVertexArrayAttribLFormat(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset)
{
    d_4_5_Core->VertexArrayAttribLFormat(vaobj, attribindex, size, type, relativeoffset);
}

inline void QOpenGLFunctions_4_5_Core::glVertexArrayAttribIFormat(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset)
{
    d_4_5_Core->VertexArrayAttribIFormat(vaobj, attribindex, size, type, relativeoffset);
}

inline void QOpenGLFunctions_4_5_Core::glVertexArrayAttribFormat(GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset)
{
    d_4_5_Core->VertexArrayAttribFormat(vaobj, attribindex, size, type, normalized, relativeoffset);
}

inline void QOpenGLFunctions_4_5_Core::glVertexArrayAttribBinding(GLuint vaobj, GLuint attribindex, GLuint bindingindex)
{
    d_4_5_Core->VertexArrayAttribBinding(vaobj, attribindex, bindingindex);
}

inline void QOpenGLFunctions_4_5_Core::glVertexArrayVertexBuffers(GLuint vaobj, GLuint first, GLsizei count, const GLuint *buffers, const GLintptr *offsets, const GLsizei *strides)
{
    d_4_5_Core->VertexArrayVertexBuffers(vaobj, first, count, buffers, offsets, strides);
}

inline void QOpenGLFunctions_4_5_Core::glVertexArrayVertexBuffer(GLuint vaobj, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride)
{
    d_4_5_Core->VertexArrayVertexBuffer(vaobj, bindingindex, buffer, offset, stride);
}

inline void QOpenGLFunctions_4_5_Core::glVertexArrayElementBuffer(GLuint vaobj, GLuint buffer)
{
    d_4_5_Core->VertexArrayElementBuffer(vaobj, buffer);
}

inline void QOpenGLFunctions_4_5_Core::glEnableVertexArrayAttrib(GLuint vaobj, GLuint index)
{
    d_4_5_Core->EnableVertexArrayAttrib(vaobj, index);
}

inline void QOpenGLFunctions_4_5_Core::glDisableVertexArrayAttrib(GLuint vaobj, GLuint index)
{
    d_4_5_Core->DisableVertexArrayAttrib(vaobj, index);
}

inline void QOpenGLFunctions_4_5_Core::glCreateVertexArrays(GLsizei n, GLuint *arrays)
{
    d_4_5_Core->CreateVertexArrays(n, arrays);
}

inline void QOpenGLFunctions_4_5_Core::glGetTextureParameteriv(GLuint texture, GLenum pname, GLint *params)
{
    d_4_5_Core->GetTextureParameteriv(texture, pname, params);
}

inline void QOpenGLFunctions_4_5_Core::glGetTextureParameterIuiv(GLuint texture, GLenum pname, GLuint *params)
{
    d_4_5_Core->GetTextureParameterIuiv(texture, pname, params);
}

inline void QOpenGLFunctions_4_5_Core::glGetTextureParameterIiv(GLuint texture, GLenum pname, GLint *params)
{
    d_4_5_Core->GetTextureParameterIiv(texture, pname, params);
}

inline void QOpenGLFunctions_4_5_Core::glGetTextureParameterfv(GLuint texture, GLenum pname, GLfloat *params)
{
    d_4_5_Core->GetTextureParameterfv(texture, pname, params);
}

inline void QOpenGLFunctions_4_5_Core::glGetTextureLevelParameteriv(GLuint texture, GLint level, GLenum pname, GLint *params)
{
    d_4_5_Core->GetTextureLevelParameteriv(texture, level, pname, params);
}

inline void QOpenGLFunctions_4_5_Core::glGetTextureLevelParameterfv(GLuint texture, GLint level, GLenum pname, GLfloat *params)
{
    d_4_5_Core->GetTextureLevelParameterfv(texture, level, pname, params);
}

inline void QOpenGLFunctions_4_5_Core::glGetCompressedTextureImage(GLuint texture, GLint level, GLsizei bufSize, void *pixels)
{
    d_4_5_Core->GetCompressedTextureImage(texture, level, bufSize, pixels);
}

inline void QOpenGLFunctions_4_5_Core::glGetTextureImage(GLuint texture, GLint level, GLenum format, GLenum type, GLsizei bufSize, void *pixels)
{
    d_4_5_Core->GetTextureImage(texture, level, format, type, bufSize, pixels);
}

inline void QOpenGLFunctions_4_5_Core::glBindTextureUnit(GLuint unit, GLuint texture)
{
    d_4_5_Core->BindTextureUnit(unit, texture);
}

inline void QOpenGLFunctions_4_5_Core::glGenerateTextureMipmap(GLuint texture)
{
    d_4_5_Core->GenerateTextureMipmap(texture);
}

inline void QOpenGLFunctions_4_5_Core::glTextureParameteriv(GLuint texture, GLenum pname, const GLint *param)
{
    d_4_5_Core->TextureParameteriv(texture, pname, param);
}

inline void QOpenGLFunctions_4_5_Core::glTextureParameterIuiv(GLuint texture, GLenum pname, const GLuint *params)
{
    d_4_5_Core->TextureParameterIuiv(texture, pname, params);
}

inline void QOpenGLFunctions_4_5_Core::glTextureParameterIiv(GLuint texture, GLenum pname, const GLint *params)
{
    d_4_5_Core->TextureParameterIiv(texture, pname, params);
}

inline void QOpenGLFunctions_4_5_Core::glTextureParameteri(GLuint texture, GLenum pname, GLint param)
{
    d_4_5_Core->TextureParameteri(texture, pname, param);
}

inline void QOpenGLFunctions_4_5_Core::glTextureParameterfv(GLuint texture, GLenum pname, const GLfloat *param)
{
    d_4_5_Core->TextureParameterfv(texture, pname, param);
}

inline void QOpenGLFunctions_4_5_Core::glTextureParameterf(GLuint texture, GLenum pname, GLfloat param)
{
    d_4_5_Core->TextureParameterf(texture, pname, param);
}

inline void QOpenGLFunctions_4_5_Core::glCopyTextureSubImage3D(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    d_4_5_Core->CopyTextureSubImage3D(texture, level, xoffset, yoffset, zoffset, x, y, width, height);
}

inline void QOpenGLFunctions_4_5_Core::glCopyTextureSubImage2D(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    d_4_5_Core->CopyTextureSubImage2D(texture, level, xoffset, yoffset, x, y, width, height);
}

inline void QOpenGLFunctions_4_5_Core::glCopyTextureSubImage1D(GLuint texture, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width)
{
    d_4_5_Core->CopyTextureSubImage1D(texture, level, xoffset, x, y, width);
}

inline void QOpenGLFunctions_4_5_Core::glCompressedTextureSubImage3D(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data)
{
    d_4_5_Core->CompressedTextureSubImage3D(texture, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data);
}

inline void QOpenGLFunctions_4_5_Core::glCompressedTextureSubImage2D(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data)
{
    d_4_5_Core->CompressedTextureSubImage2D(texture, level, xoffset, yoffset, width, height, format, imageSize, data);
}

inline void QOpenGLFunctions_4_5_Core::glCompressedTextureSubImage1D(GLuint texture, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void *data)
{
    d_4_5_Core->CompressedTextureSubImage1D(texture, level, xoffset, width, format, imageSize, data);
}

inline void QOpenGLFunctions_4_5_Core::glTextureSubImage3D(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels)
{
    d_4_5_Core->TextureSubImage3D(texture, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
}

inline void QOpenGLFunctions_4_5_Core::glTextureSubImage2D(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels)
{
    d_4_5_Core->TextureSubImage2D(texture, level, xoffset, yoffset, width, height, format, type, pixels);
}

inline void QOpenGLFunctions_4_5_Core::glTextureSubImage1D(GLuint texture, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void *pixels)
{
    d_4_5_Core->TextureSubImage1D(texture, level, xoffset, width, format, type, pixels);
}

inline void QOpenGLFunctions_4_5_Core::glTextureStorage3DMultisample(GLuint texture, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations)
{
    d_4_5_Core->TextureStorage3DMultisample(texture, samples, internalformat, width, height, depth, fixedsamplelocations);
}

inline void QOpenGLFunctions_4_5_Core::glTextureStorage2DMultisample(GLuint texture, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations)
{
    d_4_5_Core->TextureStorage2DMultisample(texture, samples, internalformat, width, height, fixedsamplelocations);
}

inline void QOpenGLFunctions_4_5_Core::glTextureStorage3D(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth)
{
    d_4_5_Core->TextureStorage3D(texture, levels, internalformat, width, height, depth);
}

inline void QOpenGLFunctions_4_5_Core::glTextureStorage2D(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)
{
    d_4_5_Core->TextureStorage2D(texture, levels, internalformat, width, height);
}

inline void QOpenGLFunctions_4_5_Core::glTextureStorage1D(GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width)
{
    d_4_5_Core->TextureStorage1D(texture, levels, internalformat, width);
}

inline void QOpenGLFunctions_4_5_Core::glTextureBufferRange(GLuint texture, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizei size)
{
    d_4_5_Core->TextureBufferRange(texture, internalformat, buffer, offset, size);
}

inline void QOpenGLFunctions_4_5_Core::glTextureBuffer(GLuint texture, GLenum internalformat, GLuint buffer)
{
    d_4_5_Core->TextureBuffer(texture, internalformat, buffer);
}

inline void QOpenGLFunctions_4_5_Core::glCreateTextures(GLenum target, GLsizei n, GLuint *textures)
{
    d_4_5_Core->CreateTextures(target, n, textures);
}

inline void QOpenGLFunctions_4_5_Core::glGetNamedRenderbufferParameteriv(GLuint renderbuffer, GLenum pname, GLint *params)
{
    d_4_5_Core->GetNamedRenderbufferParameteriv(renderbuffer, pname, params);
}

inline void QOpenGLFunctions_4_5_Core::glNamedRenderbufferStorageMultisample(GLuint renderbuffer, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
{
    d_4_5_Core->NamedRenderbufferStorageMultisample(renderbuffer, samples, internalformat, width, height);
}

inline void QOpenGLFunctions_4_5_Core::glNamedRenderbufferStorage(GLuint renderbuffer, GLenum internalformat, GLsizei width, GLsizei height)
{
    d_4_5_Core->NamedRenderbufferStorage(renderbuffer, internalformat, width, height);
}

inline void QOpenGLFunctions_4_5_Core::glCreateRenderbuffers(GLsizei n, GLuint *renderbuffers)
{
    d_4_5_Core->CreateRenderbuffers(n, renderbuffers);
}

inline void QOpenGLFunctions_4_5_Core::glGetNamedFramebufferAttachmentParameteriv(GLuint framebuffer, GLenum attachment, GLenum pname, GLint *params)
{
    d_4_5_Core->GetNamedFramebufferAttachmentParameteriv(framebuffer, attachment, pname, params);
}

inline void QOpenGLFunctions_4_5_Core::glGetNamedFramebufferParameteriv(GLuint framebuffer, GLenum pname, GLint *param)
{
    d_4_5_Core->GetNamedFramebufferParameteriv(framebuffer, pname, param);
}

inline GLenum QOpenGLFunctions_4_5_Core::glCheckNamedFramebufferStatus(GLuint framebuffer, GLenum target)
{
    return d_4_5_Core->CheckNamedFramebufferStatus(framebuffer, target);
}

inline void QOpenGLFunctions_4_5_Core::glBlitNamedFramebuffer(GLuint readFramebuffer, GLuint drawFramebuffer, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)
{
    d_4_5_Core->BlitNamedFramebuffer(readFramebuffer, drawFramebuffer, srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
}

inline void QOpenGLFunctions_4_5_Core::glClearNamedFramebufferfi(GLuint framebuffer, GLenum buffer, GLfloat depth, GLint stencil)
{
    d_4_5_Core->ClearNamedFramebufferfi(framebuffer, buffer, depth, stencil);
}

inline void QOpenGLFunctions_4_5_Core::glClearNamedFramebufferfv(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLfloat *value)
{
    d_4_5_Core->ClearNamedFramebufferfv(framebuffer, buffer, drawbuffer, value);
}

inline void QOpenGLFunctions_4_5_Core::glClearNamedFramebufferuiv(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLuint *value)
{
    d_4_5_Core->ClearNamedFramebufferuiv(framebuffer, buffer, drawbuffer, value);
}

inline void QOpenGLFunctions_4_5_Core::glClearNamedFramebufferiv(GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLint *value)
{
    d_4_5_Core->ClearNamedFramebufferiv(framebuffer, buffer, drawbuffer, value);
}

inline void QOpenGLFunctions_4_5_Core::glInvalidateNamedFramebufferSubData(GLuint framebuffer, GLsizei numAttachments, const GLenum *attachments, GLint x, GLint y, GLsizei width, GLsizei height)
{
    d_4_5_Core->InvalidateNamedFramebufferSubData(framebuffer, numAttachments, attachments, x, y, width, height);
}

inline void QOpenGLFunctions_4_5_Core::glInvalidateNamedFramebufferData(GLuint framebuffer, GLsizei numAttachments, const GLenum *attachments)
{
    d_4_5_Core->InvalidateNamedFramebufferData(framebuffer, numAttachments, attachments);
}

inline void QOpenGLFunctions_4_5_Core::glNamedFramebufferReadBuffer(GLuint framebuffer, GLenum src)
{
    d_4_5_Core->NamedFramebufferReadBuffer(framebuffer, src);
}

inline void QOpenGLFunctions_4_5_Core::glNamedFramebufferDrawBuffers(GLuint framebuffer, GLsizei n, const GLenum *bufs)
{
    d_4_5_Core->NamedFramebufferDrawBuffers(framebuffer, n, bufs);
}

inline void QOpenGLFunctions_4_5_Core::glNamedFramebufferDrawBuffer(GLuint framebuffer, GLenum buf)
{
    d_4_5_Core->NamedFramebufferDrawBuffer(framebuffer, buf);
}

inline void QOpenGLFunctions_4_5_Core::glNamedFramebufferTextureLayer(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level, GLint layer)
{
    d_4_5_Core->NamedFramebufferTextureLayer(framebuffer, attachment, texture, level, layer);
}

inline void QOpenGLFunctions_4_5_Core::glNamedFramebufferTexture(GLuint framebuffer, GLenum attachment, GLuint texture, GLint level)
{
    d_4_5_Core->NamedFramebufferTexture(framebuffer, attachment, texture, level);
}

inline void QOpenGLFunctions_4_5_Core::glNamedFramebufferParameteri(GLuint framebuffer, GLenum pname, GLint param)
{
    d_4_5_Core->NamedFramebufferParameteri(framebuffer, pname, param);
}

inline void QOpenGLFunctions_4_5_Core::glNamedFramebufferRenderbuffer(GLuint framebuffer, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
    d_4_5_Core->NamedFramebufferRenderbuffer(framebuffer, attachment, renderbuffertarget, renderbuffer);
}

inline void QOpenGLFunctions_4_5_Core::glCreateFramebuffers(GLsizei n, GLuint *framebuffers)
{
    d_4_5_Core->CreateFramebuffers(n, framebuffers);
}

inline void QOpenGLFunctions_4_5_Core::glGetNamedBufferSubData(GLuint buffer, GLintptr offset, GLsizei size, void *data)
{
    d_4_5_Core->GetNamedBufferSubData(buffer, offset, size, data);
}

inline void QOpenGLFunctions_4_5_Core::glGetNamedBufferPointerv(GLuint buffer, GLenum pname, void * *params)
{
    d_4_5_Core->GetNamedBufferPointerv(buffer, pname, params);
}

inline void QOpenGLFunctions_4_5_Core::glGetNamedBufferParameteri64v(GLuint buffer, GLenum pname, GLint64 *params)
{
    d_4_5_Core->GetNamedBufferParameteri64v(buffer, pname, params);
}

inline void QOpenGLFunctions_4_5_Core::glGetNamedBufferParameteriv(GLuint buffer, GLenum pname, GLint *params)
{
    d_4_5_Core->GetNamedBufferParameteriv(buffer, pname, params);
}

inline void QOpenGLFunctions_4_5_Core::glFlushMappedNamedBufferRange(GLuint buffer, GLintptr offset, GLsizei length)
{
    d_4_5_Core->FlushMappedNamedBufferRange(buffer, offset, length);
}

inline GLboolean QOpenGLFunctions_4_5_Core::glUnmapNamedBuffer(GLuint buffer)
{
    return d_4_5_Core->UnmapNamedBuffer(buffer);
}

inline void * QOpenGLFunctions_4_5_Core::glMapNamedBufferRange(GLuint buffer, GLintptr offset, GLsizei length, GLbitfield access)
{
    return d_4_5_Core->MapNamedBufferRange(buffer, offset, length, access);
}

inline void * QOpenGLFunctions_4_5_Core::glMapNamedBuffer(GLuint buffer, GLenum access)
{
    return d_4_5_Core->MapNamedBuffer(buffer, access);
}

inline void QOpenGLFunctions_4_5_Core::glClearNamedBufferSubData(GLuint buffer, GLenum internalformat, GLintptr offset, GLsizei size, GLenum format, GLenum type, const void *data)
{
    d_4_5_Core->ClearNamedBufferSubData(buffer, internalformat, offset, size, format, type, data);
}

inline void QOpenGLFunctions_4_5_Core::glClearNamedBufferData(GLuint buffer, GLenum internalformat, GLenum format, GLenum type, const void *data)
{
    d_4_5_Core->ClearNamedBufferData(buffer, internalformat, format, type, data);
}

inline void QOpenGLFunctions_4_5_Core::glCopyNamedBufferSubData(GLuint readBuffer, GLuint writeBuffer, GLintptr readOffset, GLintptr writeOffset, GLsizei size)
{
    d_4_5_Core->CopyNamedBufferSubData(readBuffer, writeBuffer, readOffset, writeOffset, size);
}

inline void QOpenGLFunctions_4_5_Core::glNamedBufferSubData(GLuint buffer, GLintptr offset, GLsizei size, const void *data)
{
    d_4_5_Core->NamedBufferSubData(buffer, offset, size, data);
}

inline void QOpenGLFunctions_4_5_Core::glNamedBufferData(GLuint buffer, GLsizei size, const void *data, GLenum usage)
{
    d_4_5_Core->NamedBufferData(buffer, size, data, usage);
}

inline void QOpenGLFunctions_4_5_Core::glNamedBufferStorage(GLuint buffer, GLsizei size, const void *data, GLbitfield flags)
{
    d_4_5_Core->NamedBufferStorage(buffer, size, data, flags);
}

inline void QOpenGLFunctions_4_5_Core::glCreateBuffers(GLsizei n, GLuint *buffers)
{
    d_4_5_Core->CreateBuffers(n, buffers);
}

inline void QOpenGLFunctions_4_5_Core::glGetTransformFeedbacki64_v(GLuint xfb, GLenum pname, GLuint index, GLint64 *param)
{
    d_4_5_Core->GetTransformFeedbacki64_v(xfb, pname, index, param);
}

inline void QOpenGLFunctions_4_5_Core::glGetTransformFeedbacki_v(GLuint xfb, GLenum pname, GLuint index, GLint *param)
{
    d_4_5_Core->GetTransformFeedbacki_v(xfb, pname, index, param);
}

inline void QOpenGLFunctions_4_5_Core::glGetTransformFeedbackiv(GLuint xfb, GLenum pname, GLint *param)
{
    d_4_5_Core->GetTransformFeedbackiv(xfb, pname, param);
}

inline void QOpenGLFunctions_4_5_Core::glTransformFeedbackBufferRange(GLuint xfb, GLuint index, GLuint buffer, GLintptr offset, GLsizei size)
{
    d_4_5_Core->TransformFeedbackBufferRange(xfb, index, buffer, offset, size);
}

inline void QOpenGLFunctions_4_5_Core::glTransformFeedbackBufferBase(GLuint xfb, GLuint index, GLuint buffer)
{
    d_4_5_Core->TransformFeedbackBufferBase(xfb, index, buffer);
}

inline void QOpenGLFunctions_4_5_Core::glCreateTransformFeedbacks(GLsizei n, GLuint *ids)
{
    d_4_5_Core->CreateTransformFeedbacks(n, ids);
}

inline void QOpenGLFunctions_4_5_Core::glClipControl(GLenum origin, GLenum depth)
{
    d_4_5_Core->ClipControl(origin, depth);
}



QT_END_NAMESPACE

#endif // QT_NO_OPENGL && !QT_OPENGL_ES_2

#endif
