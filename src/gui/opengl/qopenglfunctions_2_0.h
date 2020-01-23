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

#ifndef QOPENGLVERSIONFUNCTIONS_2_0_H
#define QOPENGLVERSIONFUNCTIONS_2_0_H

#include <QtCore/qglobal.h>

#if !defined(QT_NO_OPENGL) && !defined(QT_OPENGL_ES_2)

#include <QtGui/QOpenGLVersionFunctions>
#include <QtGui/qopenglcontext.h>

QT_BEGIN_NAMESPACE

class Q_GUI_EXPORT QOpenGLFunctions_2_0 : public QAbstractOpenGLFunctions
{
public:
    QOpenGLFunctions_2_0();
    ~QOpenGLFunctions_2_0();

    bool initializeOpenGLFunctions() override;

    // OpenGL 1.0 core functions
    void glViewport(GLint x, GLint y, GLsizei width, GLsizei height);
    void glDepthRange(GLdouble nearVal, GLdouble farVal);
    GLboolean glIsEnabled(GLenum cap);
    void glGetTexLevelParameteriv(GLenum target, GLint level, GLenum pname, GLint *params);
    void glGetTexLevelParameterfv(GLenum target, GLint level, GLenum pname, GLfloat *params);
    void glGetTexParameteriv(GLenum target, GLenum pname, GLint *params);
    void glGetTexParameterfv(GLenum target, GLenum pname, GLfloat *params);
    void glGetTexImage(GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels);
    const GLubyte * glGetString(GLenum name);
    void glGetIntegerv(GLenum pname, GLint *params);
    void glGetFloatv(GLenum pname, GLfloat *params);
    GLenum glGetError();
    void glGetDoublev(GLenum pname, GLdouble *params);
    void glGetBooleanv(GLenum pname, GLboolean *params);
    void glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels);
    void glReadBuffer(GLenum mode);
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
    void glDrawBuffer(GLenum mode);
    void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
    void glTexImage1D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
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
    void glIndexubv(const GLubyte *c);
    void glIndexub(GLubyte c);
    GLboolean glIsTexture(GLuint texture);
    void glGenTextures(GLsizei n, GLuint *textures);
    void glDeleteTextures(GLsizei n, const GLuint *textures);
    void glBindTexture(GLenum target, GLuint texture);
    void glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
    void glTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels);
    void glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
    void glCopyTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width);
    void glCopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
    void glCopyTexImage1D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border);
    void glPolygonOffset(GLfloat factor, GLfloat units);
    void glGetPointerv(GLenum pname, GLvoid* *params);
    void glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
    void glDrawArrays(GLenum mode, GLint first, GLsizei count);

    // OpenGL 1.2 core functions
    void glCopyTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height);
    void glTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels);
    void glTexImage3D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
    void glDrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices);
    void glBlendEquation(GLenum mode);
    void glBlendColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);

    // OpenGL 1.3 core functions
    void glGetCompressedTexImage(GLenum target, GLint level, GLvoid *img);
    void glCompressedTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data);
    void glCompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data);
    void glCompressedTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *data);
    void glCompressedTexImage1D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data);
    void glCompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data);
    void glCompressedTexImage3D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *data);
    void glSampleCoverage(GLfloat value, GLboolean invert);
    void glActiveTexture(GLenum texture);

    // OpenGL 1.4 core functions
    void glPointParameteriv(GLenum pname, const GLint *params);
    void glPointParameteri(GLenum pname, GLint param);
    void glPointParameterfv(GLenum pname, const GLfloat *params);
    void glPointParameterf(GLenum pname, GLfloat param);
    void glMultiDrawElements(GLenum mode, const GLsizei *count, GLenum type, const GLvoid* const *indices, GLsizei drawcount);
    void glMultiDrawArrays(GLenum mode, const GLint *first, const GLsizei *count, GLsizei drawcount);
    void glBlendFuncSeparate(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);

    // OpenGL 1.5 core functions
    void glGetBufferPointerv(GLenum target, GLenum pname, GLvoid* *params);
    void glGetBufferParameteriv(GLenum target, GLenum pname, GLint *params);
    GLboolean glUnmapBuffer(GLenum target);
    GLvoid* glMapBuffer(GLenum target, GLenum access);
    void glGetBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, GLvoid *data);
    void glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid *data);
    void glBufferData(GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage);
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
    void glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer);
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
    void glGetVertexAttribPointerv(GLuint index, GLenum pname, GLvoid* *pointer);
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
    void glGetAttachedShaders(GLuint program, GLsizei maxCount, GLsizei *count, GLuint *obj);
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
    void glDrawPixels(GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels);
    void glCopyPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type);
    void glPixelMapusv(GLenum map, GLint mapsize, const GLushort *values);
    void glPixelMapuiv(GLenum map, GLint mapsize, const GLuint *values);
    void glPixelMapfv(GLenum map, GLint mapsize, const GLfloat *values);
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
    void glCallLists(GLsizei n, GLenum type, const GLvoid *lists);
    void glCallList(GLuint list);
    void glEndList();
    void glNewList(GLuint list, GLenum mode);

    // OpenGL 1.1 deprecated functions
    void glPushClientAttrib(GLbitfield mask);
    void glPopClientAttrib();
    void glPrioritizeTextures(GLsizei n, const GLuint *textures, const GLfloat *priorities);
    GLboolean glAreTexturesResident(GLsizei n, const GLuint *textures, GLboolean *residences);
    void glVertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
    void glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
    void glNormalPointer(GLenum type, GLsizei stride, const GLvoid *pointer);
    void glInterleavedArrays(GLenum format, GLsizei stride, const GLvoid *pointer);
    void glIndexPointer(GLenum type, GLsizei stride, const GLvoid *pointer);
    void glEnableClientState(GLenum array);
    void glEdgeFlagPointer(GLsizei stride, const GLvoid *pointer);
    void glDisableClientState(GLenum array);
    void glColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
    void glArrayElement(GLint i);

    // OpenGL 1.2 deprecated functions
    void glResetMinmax(GLenum target);
    void glResetHistogram(GLenum target);
    void glMinmax(GLenum target, GLenum internalformat, GLboolean sink);
    void glHistogram(GLenum target, GLsizei width, GLenum internalformat, GLboolean sink);
    void glGetMinmaxParameteriv(GLenum target, GLenum pname, GLint *params);
    void glGetMinmaxParameterfv(GLenum target, GLenum pname, GLfloat *params);
    void glGetMinmax(GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid *values);
    void glGetHistogramParameteriv(GLenum target, GLenum pname, GLint *params);
    void glGetHistogramParameterfv(GLenum target, GLenum pname, GLfloat *params);
    void glGetHistogram(GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid *values);
    void glSeparableFilter2D(GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *row, const GLvoid *column);
    void glGetSeparableFilter(GLenum target, GLenum format, GLenum type, GLvoid *row, GLvoid *column, GLvoid *span);
    void glGetConvolutionParameteriv(GLenum target, GLenum pname, GLint *params);
    void glGetConvolutionParameterfv(GLenum target, GLenum pname, GLfloat *params);
    void glGetConvolutionFilter(GLenum target, GLenum format, GLenum type, GLvoid *image);
    void glCopyConvolutionFilter2D(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height);
    void glCopyConvolutionFilter1D(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width);
    void glConvolutionParameteriv(GLenum target, GLenum pname, const GLint *params);
    void glConvolutionParameteri(GLenum target, GLenum pname, GLint params);
    void glConvolutionParameterfv(GLenum target, GLenum pname, const GLfloat *params);
    void glConvolutionParameterf(GLenum target, GLenum pname, GLfloat params);
    void glConvolutionFilter2D(GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *image);
    void glConvolutionFilter1D(GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *image);
    void glCopyColorSubTable(GLenum target, GLsizei start, GLint x, GLint y, GLsizei width);
    void glColorSubTable(GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const GLvoid *data);
    void glGetColorTableParameteriv(GLenum target, GLenum pname, GLint *params);
    void glGetColorTableParameterfv(GLenum target, GLenum pname, GLfloat *params);
    void glGetColorTable(GLenum target, GLenum format, GLenum type, GLvoid *table);
    void glCopyColorTable(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width);
    void glColorTableParameteriv(GLenum target, GLenum pname, const GLint *params);
    void glColorTableParameterfv(GLenum target, GLenum pname, const GLfloat *params);
    void glColorTable(GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *table);

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
    void glSecondaryColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
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
    void glFogCoordPointer(GLenum type, GLsizei stride, const GLvoid *pointer);
    void glFogCoorddv(const GLdouble *coord);
    void glFogCoordd(GLdouble coord);
    void glFogCoordfv(const GLfloat *coord);
    void glFogCoordf(GLfloat coord);

    // OpenGL 1.5 deprecated functions

    // OpenGL 2.0 deprecated functions
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
    QOpenGLFunctions_1_0_DeprecatedBackend* d_1_0_Deprecated;
    QOpenGLFunctions_1_1_DeprecatedBackend* d_1_1_Deprecated;
    QOpenGLFunctions_1_2_DeprecatedBackend* d_1_2_Deprecated;
    QOpenGLFunctions_1_3_DeprecatedBackend* d_1_3_Deprecated;
    QOpenGLFunctions_1_4_DeprecatedBackend* d_1_4_Deprecated;
    void *m_reserved_2_0_Deprecated; // To maintain BC
};

// OpenGL 1.0 core functions
inline void QOpenGLFunctions_2_0::glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
    d_1_0_Core->Viewport(x, y, width, height);
}

inline void QOpenGLFunctions_2_0::glDepthRange(GLdouble nearVal, GLdouble farVal)
{
    d_1_0_Core->DepthRange(nearVal, farVal);
}

inline GLboolean QOpenGLFunctions_2_0::glIsEnabled(GLenum cap)
{
    return d_1_0_Core->IsEnabled(cap);
}

inline void QOpenGLFunctions_2_0::glGetTexLevelParameteriv(GLenum target, GLint level, GLenum pname, GLint *params)
{
    d_1_0_Core->GetTexLevelParameteriv(target, level, pname, params);
}

inline void QOpenGLFunctions_2_0::glGetTexLevelParameterfv(GLenum target, GLint level, GLenum pname, GLfloat *params)
{
    d_1_0_Core->GetTexLevelParameterfv(target, level, pname, params);
}

inline void QOpenGLFunctions_2_0::glGetTexParameteriv(GLenum target, GLenum pname, GLint *params)
{
    d_1_0_Core->GetTexParameteriv(target, pname, params);
}

inline void QOpenGLFunctions_2_0::glGetTexParameterfv(GLenum target, GLenum pname, GLfloat *params)
{
    d_1_0_Core->GetTexParameterfv(target, pname, params);
}

inline void QOpenGLFunctions_2_0::glGetTexImage(GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels)
{
    d_1_0_Core->GetTexImage(target, level, format, type, pixels);
}

inline const GLubyte * QOpenGLFunctions_2_0::glGetString(GLenum name)
{
    return d_1_0_Core->GetString(name);
}

inline void QOpenGLFunctions_2_0::glGetIntegerv(GLenum pname, GLint *params)
{
    d_1_0_Core->GetIntegerv(pname, params);
}

inline void QOpenGLFunctions_2_0::glGetFloatv(GLenum pname, GLfloat *params)
{
    d_1_0_Core->GetFloatv(pname, params);
}

inline GLenum QOpenGLFunctions_2_0::glGetError()
{
    return d_1_0_Core->GetError();
}

inline void QOpenGLFunctions_2_0::glGetDoublev(GLenum pname, GLdouble *params)
{
    d_1_0_Core->GetDoublev(pname, params);
}

inline void QOpenGLFunctions_2_0::glGetBooleanv(GLenum pname, GLboolean *params)
{
    d_1_0_Core->GetBooleanv(pname, params);
}

inline void QOpenGLFunctions_2_0::glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels)
{
    d_1_0_Core->ReadPixels(x, y, width, height, format, type, pixels);
}

inline void QOpenGLFunctions_2_0::glReadBuffer(GLenum mode)
{
    d_1_0_Core->ReadBuffer(mode);
}

inline void QOpenGLFunctions_2_0::glPixelStorei(GLenum pname, GLint param)
{
    d_1_0_Core->PixelStorei(pname, param);
}

inline void QOpenGLFunctions_2_0::glPixelStoref(GLenum pname, GLfloat param)
{
    d_1_0_Core->PixelStoref(pname, param);
}

inline void QOpenGLFunctions_2_0::glDepthFunc(GLenum func)
{
    d_1_0_Core->DepthFunc(func);
}

inline void QOpenGLFunctions_2_0::glStencilOp(GLenum fail, GLenum zfail, GLenum zpass)
{
    d_1_0_Core->StencilOp(fail, zfail, zpass);
}

inline void QOpenGLFunctions_2_0::glStencilFunc(GLenum func, GLint ref, GLuint mask)
{
    d_1_0_Core->StencilFunc(func, ref, mask);
}

inline void QOpenGLFunctions_2_0::glLogicOp(GLenum opcode)
{
    d_1_0_Core->LogicOp(opcode);
}

inline void QOpenGLFunctions_2_0::glBlendFunc(GLenum sfactor, GLenum dfactor)
{
    d_1_0_Core->BlendFunc(sfactor, dfactor);
}

inline void QOpenGLFunctions_2_0::glFlush()
{
    d_1_0_Core->Flush();
}

inline void QOpenGLFunctions_2_0::glFinish()
{
    d_1_0_Core->Finish();
}

inline void QOpenGLFunctions_2_0::glEnable(GLenum cap)
{
    d_1_0_Core->Enable(cap);
}

inline void QOpenGLFunctions_2_0::glDisable(GLenum cap)
{
    d_1_0_Core->Disable(cap);
}

inline void QOpenGLFunctions_2_0::glDepthMask(GLboolean flag)
{
    d_1_0_Core->DepthMask(flag);
}

inline void QOpenGLFunctions_2_0::glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
    d_1_0_Core->ColorMask(red, green, blue, alpha);
}

inline void QOpenGLFunctions_2_0::glStencilMask(GLuint mask)
{
    d_1_0_Core->StencilMask(mask);
}

inline void QOpenGLFunctions_2_0::glClearDepth(GLdouble depth)
{
    d_1_0_Core->ClearDepth(depth);
}

inline void QOpenGLFunctions_2_0::glClearStencil(GLint s)
{
    d_1_0_Core->ClearStencil(s);
}

inline void QOpenGLFunctions_2_0::glClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    d_1_0_Core->ClearColor(red, green, blue, alpha);
}

inline void QOpenGLFunctions_2_0::glClear(GLbitfield mask)
{
    d_1_0_Core->Clear(mask);
}

inline void QOpenGLFunctions_2_0::glDrawBuffer(GLenum mode)
{
    d_1_0_Core->DrawBuffer(mode);
}

inline void QOpenGLFunctions_2_0::glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
    d_1_0_Core->TexImage2D(target, level, internalformat, width, height, border, format, type, pixels);
}

inline void QOpenGLFunctions_2_0::glTexImage1D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
    d_1_0_Core->TexImage1D(target, level, internalformat, width, border, format, type, pixels);
}

inline void QOpenGLFunctions_2_0::glTexParameteriv(GLenum target, GLenum pname, const GLint *params)
{
    d_1_0_Core->TexParameteriv(target, pname, params);
}

inline void QOpenGLFunctions_2_0::glTexParameteri(GLenum target, GLenum pname, GLint param)
{
    d_1_0_Core->TexParameteri(target, pname, param);
}

inline void QOpenGLFunctions_2_0::glTexParameterfv(GLenum target, GLenum pname, const GLfloat *params)
{
    d_1_0_Core->TexParameterfv(target, pname, params);
}

inline void QOpenGLFunctions_2_0::glTexParameterf(GLenum target, GLenum pname, GLfloat param)
{
    d_1_0_Core->TexParameterf(target, pname, param);
}

inline void QOpenGLFunctions_2_0::glScissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
    d_1_0_Core->Scissor(x, y, width, height);
}

inline void QOpenGLFunctions_2_0::glPolygonMode(GLenum face, GLenum mode)
{
    d_1_0_Core->PolygonMode(face, mode);
}

inline void QOpenGLFunctions_2_0::glPointSize(GLfloat size)
{
    d_1_0_Core->PointSize(size);
}

inline void QOpenGLFunctions_2_0::glLineWidth(GLfloat width)
{
    d_1_0_Core->LineWidth(width);
}

inline void QOpenGLFunctions_2_0::glHint(GLenum target, GLenum mode)
{
    d_1_0_Core->Hint(target, mode);
}

inline void QOpenGLFunctions_2_0::glFrontFace(GLenum mode)
{
    d_1_0_Core->FrontFace(mode);
}

inline void QOpenGLFunctions_2_0::glCullFace(GLenum mode)
{
    d_1_0_Core->CullFace(mode);
}


// OpenGL 1.1 core functions
inline void QOpenGLFunctions_2_0::glIndexubv(const GLubyte *c)
{
    d_1_1_Deprecated->Indexubv(c);
}

inline void QOpenGLFunctions_2_0::glIndexub(GLubyte c)
{
    d_1_1_Deprecated->Indexub(c);
}

inline GLboolean QOpenGLFunctions_2_0::glIsTexture(GLuint texture)
{
    return d_1_1_Core->IsTexture(texture);
}

inline void QOpenGLFunctions_2_0::glGenTextures(GLsizei n, GLuint *textures)
{
    d_1_1_Core->GenTextures(n, textures);
}

inline void QOpenGLFunctions_2_0::glDeleteTextures(GLsizei n, const GLuint *textures)
{
    d_1_1_Core->DeleteTextures(n, textures);
}

inline void QOpenGLFunctions_2_0::glBindTexture(GLenum target, GLuint texture)
{
    d_1_1_Core->BindTexture(target, texture);
}

inline void QOpenGLFunctions_2_0::glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels)
{
    d_1_1_Core->TexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
}

inline void QOpenGLFunctions_2_0::glTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels)
{
    d_1_1_Core->TexSubImage1D(target, level, xoffset, width, format, type, pixels);
}

inline void QOpenGLFunctions_2_0::glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    d_1_1_Core->CopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);
}

inline void QOpenGLFunctions_2_0::glCopyTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width)
{
    d_1_1_Core->CopyTexSubImage1D(target, level, xoffset, x, y, width);
}

inline void QOpenGLFunctions_2_0::glCopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
    d_1_1_Core->CopyTexImage2D(target, level, internalformat, x, y, width, height, border);
}

inline void QOpenGLFunctions_2_0::glCopyTexImage1D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border)
{
    d_1_1_Core->CopyTexImage1D(target, level, internalformat, x, y, width, border);
}

inline void QOpenGLFunctions_2_0::glPolygonOffset(GLfloat factor, GLfloat units)
{
    d_1_1_Core->PolygonOffset(factor, units);
}

inline void QOpenGLFunctions_2_0::glGetPointerv(GLenum pname, GLvoid* *params)
{
    d_1_1_Deprecated->GetPointerv(pname, params);
}

inline void QOpenGLFunctions_2_0::glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices)
{
    d_1_1_Core->DrawElements(mode, count, type, indices);
}

inline void QOpenGLFunctions_2_0::glDrawArrays(GLenum mode, GLint first, GLsizei count)
{
    d_1_1_Core->DrawArrays(mode, first, count);
}


// OpenGL 1.2 core functions
inline void QOpenGLFunctions_2_0::glCopyTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    d_1_2_Core->CopyTexSubImage3D(target, level, xoffset, yoffset, zoffset, x, y, width, height);
}

inline void QOpenGLFunctions_2_0::glTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid *pixels)
{
    d_1_2_Core->TexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);
}

inline void QOpenGLFunctions_2_0::glTexImage3D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
    d_1_2_Core->TexImage3D(target, level, internalformat, width, height, depth, border, format, type, pixels);
}

inline void QOpenGLFunctions_2_0::glDrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid *indices)
{
    d_1_2_Core->DrawRangeElements(mode, start, end, count, type, indices);
}

inline void QOpenGLFunctions_2_0::glBlendEquation(GLenum mode)
{
    d_1_2_Core->BlendEquation(mode);
}

inline void QOpenGLFunctions_2_0::glBlendColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    d_1_2_Core->BlendColor(red, green, blue, alpha);
}


// OpenGL 1.3 core functions
inline void QOpenGLFunctions_2_0::glGetCompressedTexImage(GLenum target, GLint level, GLvoid *img)
{
    d_1_3_Core->GetCompressedTexImage(target, level, img);
}

inline void QOpenGLFunctions_2_0::glCompressedTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid *data)
{
    d_1_3_Core->CompressedTexSubImage1D(target, level, xoffset, width, format, imageSize, data);
}

inline void QOpenGLFunctions_2_0::glCompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data)
{
    d_1_3_Core->CompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, imageSize, data);
}

inline void QOpenGLFunctions_2_0::glCompressedTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid *data)
{
    d_1_3_Core->CompressedTexSubImage3D(target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data);
}

inline void QOpenGLFunctions_2_0::glCompressedTexImage1D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid *data)
{
    d_1_3_Core->CompressedTexImage1D(target, level, internalformat, width, border, imageSize, data);
}

inline void QOpenGLFunctions_2_0::glCompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data)
{
    d_1_3_Core->CompressedTexImage2D(target, level, internalformat, width, height, border, imageSize, data);
}

inline void QOpenGLFunctions_2_0::glCompressedTexImage3D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid *data)
{
    d_1_3_Core->CompressedTexImage3D(target, level, internalformat, width, height, depth, border, imageSize, data);
}

inline void QOpenGLFunctions_2_0::glSampleCoverage(GLfloat value, GLboolean invert)
{
    d_1_3_Core->SampleCoverage(value, invert);
}

inline void QOpenGLFunctions_2_0::glActiveTexture(GLenum texture)
{
    d_1_3_Core->ActiveTexture(texture);
}


// OpenGL 1.4 core functions
inline void QOpenGLFunctions_2_0::glPointParameteriv(GLenum pname, const GLint *params)
{
    d_1_4_Core->PointParameteriv(pname, params);
}

inline void QOpenGLFunctions_2_0::glPointParameteri(GLenum pname, GLint param)
{
    d_1_4_Core->PointParameteri(pname, param);
}

inline void QOpenGLFunctions_2_0::glPointParameterfv(GLenum pname, const GLfloat *params)
{
    d_1_4_Core->PointParameterfv(pname, params);
}

inline void QOpenGLFunctions_2_0::glPointParameterf(GLenum pname, GLfloat param)
{
    d_1_4_Core->PointParameterf(pname, param);
}

inline void QOpenGLFunctions_2_0::glMultiDrawElements(GLenum mode, const GLsizei *count, GLenum type, const GLvoid* const *indices, GLsizei drawcount)
{
    d_1_4_Core->MultiDrawElements(mode, count, type, indices, drawcount);
}

inline void QOpenGLFunctions_2_0::glMultiDrawArrays(GLenum mode, const GLint *first, const GLsizei *count, GLsizei drawcount)
{
    d_1_4_Core->MultiDrawArrays(mode, first, count, drawcount);
}

inline void QOpenGLFunctions_2_0::glBlendFuncSeparate(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha)
{
    d_1_4_Core->BlendFuncSeparate(sfactorRGB, dfactorRGB, sfactorAlpha, dfactorAlpha);
}


// OpenGL 1.5 core functions
inline void QOpenGLFunctions_2_0::glGetBufferPointerv(GLenum target, GLenum pname, GLvoid* *params)
{
    d_1_5_Core->GetBufferPointerv(target, pname, params);
}

inline void QOpenGLFunctions_2_0::glGetBufferParameteriv(GLenum target, GLenum pname, GLint *params)
{
    d_1_5_Core->GetBufferParameteriv(target, pname, params);
}

inline GLboolean QOpenGLFunctions_2_0::glUnmapBuffer(GLenum target)
{
    return d_1_5_Core->UnmapBuffer(target);
}

inline GLvoid* QOpenGLFunctions_2_0::glMapBuffer(GLenum target, GLenum access)
{
    return d_1_5_Core->MapBuffer(target, access);
}

inline void QOpenGLFunctions_2_0::glGetBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, GLvoid *data)
{
    d_1_5_Core->GetBufferSubData(target, offset, size, data);
}

inline void QOpenGLFunctions_2_0::glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid *data)
{
    d_1_5_Core->BufferSubData(target, offset, size, data);
}

inline void QOpenGLFunctions_2_0::glBufferData(GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage)
{
    d_1_5_Core->BufferData(target, size, data, usage);
}

inline GLboolean QOpenGLFunctions_2_0::glIsBuffer(GLuint buffer)
{
    return d_1_5_Core->IsBuffer(buffer);
}

inline void QOpenGLFunctions_2_0::glGenBuffers(GLsizei n, GLuint *buffers)
{
    d_1_5_Core->GenBuffers(n, buffers);
}

inline void QOpenGLFunctions_2_0::glDeleteBuffers(GLsizei n, const GLuint *buffers)
{
    d_1_5_Core->DeleteBuffers(n, buffers);
}

inline void QOpenGLFunctions_2_0::glBindBuffer(GLenum target, GLuint buffer)
{
    d_1_5_Core->BindBuffer(target, buffer);
}

inline void QOpenGLFunctions_2_0::glGetQueryObjectuiv(GLuint id, GLenum pname, GLuint *params)
{
    d_1_5_Core->GetQueryObjectuiv(id, pname, params);
}

inline void QOpenGLFunctions_2_0::glGetQueryObjectiv(GLuint id, GLenum pname, GLint *params)
{
    d_1_5_Core->GetQueryObjectiv(id, pname, params);
}

inline void QOpenGLFunctions_2_0::glGetQueryiv(GLenum target, GLenum pname, GLint *params)
{
    d_1_5_Core->GetQueryiv(target, pname, params);
}

inline void QOpenGLFunctions_2_0::glEndQuery(GLenum target)
{
    d_1_5_Core->EndQuery(target);
}

inline void QOpenGLFunctions_2_0::glBeginQuery(GLenum target, GLuint id)
{
    d_1_5_Core->BeginQuery(target, id);
}

inline GLboolean QOpenGLFunctions_2_0::glIsQuery(GLuint id)
{
    return d_1_5_Core->IsQuery(id);
}

inline void QOpenGLFunctions_2_0::glDeleteQueries(GLsizei n, const GLuint *ids)
{
    d_1_5_Core->DeleteQueries(n, ids);
}

inline void QOpenGLFunctions_2_0::glGenQueries(GLsizei n, GLuint *ids)
{
    d_1_5_Core->GenQueries(n, ids);
}


// OpenGL 2.0 core functions
inline void QOpenGLFunctions_2_0::glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer)
{
    d_2_0_Core->VertexAttribPointer(index, size, type, normalized, stride, pointer);
}

inline void QOpenGLFunctions_2_0::glValidateProgram(GLuint program)
{
    d_2_0_Core->ValidateProgram(program);
}

inline void QOpenGLFunctions_2_0::glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    d_2_0_Core->UniformMatrix4fv(location, count, transpose, value);
}

inline void QOpenGLFunctions_2_0::glUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    d_2_0_Core->UniformMatrix3fv(location, count, transpose, value);
}

inline void QOpenGLFunctions_2_0::glUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    d_2_0_Core->UniformMatrix2fv(location, count, transpose, value);
}

inline void QOpenGLFunctions_2_0::glUniform4iv(GLint location, GLsizei count, const GLint *value)
{
    d_2_0_Core->Uniform4iv(location, count, value);
}

inline void QOpenGLFunctions_2_0::glUniform3iv(GLint location, GLsizei count, const GLint *value)
{
    d_2_0_Core->Uniform3iv(location, count, value);
}

inline void QOpenGLFunctions_2_0::glUniform2iv(GLint location, GLsizei count, const GLint *value)
{
    d_2_0_Core->Uniform2iv(location, count, value);
}

inline void QOpenGLFunctions_2_0::glUniform1iv(GLint location, GLsizei count, const GLint *value)
{
    d_2_0_Core->Uniform1iv(location, count, value);
}

inline void QOpenGLFunctions_2_0::glUniform4fv(GLint location, GLsizei count, const GLfloat *value)
{
    d_2_0_Core->Uniform4fv(location, count, value);
}

inline void QOpenGLFunctions_2_0::glUniform3fv(GLint location, GLsizei count, const GLfloat *value)
{
    d_2_0_Core->Uniform3fv(location, count, value);
}

inline void QOpenGLFunctions_2_0::glUniform2fv(GLint location, GLsizei count, const GLfloat *value)
{
    d_2_0_Core->Uniform2fv(location, count, value);
}

inline void QOpenGLFunctions_2_0::glUniform1fv(GLint location, GLsizei count, const GLfloat *value)
{
    d_2_0_Core->Uniform1fv(location, count, value);
}

inline void QOpenGLFunctions_2_0::glUniform4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3)
{
    d_2_0_Core->Uniform4i(location, v0, v1, v2, v3);
}

inline void QOpenGLFunctions_2_0::glUniform3i(GLint location, GLint v0, GLint v1, GLint v2)
{
    d_2_0_Core->Uniform3i(location, v0, v1, v2);
}

inline void QOpenGLFunctions_2_0::glUniform2i(GLint location, GLint v0, GLint v1)
{
    d_2_0_Core->Uniform2i(location, v0, v1);
}

inline void QOpenGLFunctions_2_0::glUniform1i(GLint location, GLint v0)
{
    d_2_0_Core->Uniform1i(location, v0);
}

inline void QOpenGLFunctions_2_0::glUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
{
    d_2_0_Core->Uniform4f(location, v0, v1, v2, v3);
}

inline void QOpenGLFunctions_2_0::glUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2)
{
    d_2_0_Core->Uniform3f(location, v0, v1, v2);
}

inline void QOpenGLFunctions_2_0::glUniform2f(GLint location, GLfloat v0, GLfloat v1)
{
    d_2_0_Core->Uniform2f(location, v0, v1);
}

inline void QOpenGLFunctions_2_0::glUniform1f(GLint location, GLfloat v0)
{
    d_2_0_Core->Uniform1f(location, v0);
}

inline void QOpenGLFunctions_2_0::glUseProgram(GLuint program)
{
    d_2_0_Core->UseProgram(program);
}

inline void QOpenGLFunctions_2_0::glShaderSource(GLuint shader, GLsizei count, const GLchar* const *string, const GLint *length)
{
    d_2_0_Core->ShaderSource(shader, count, string, length);
}

inline void QOpenGLFunctions_2_0::glLinkProgram(GLuint program)
{
    d_2_0_Core->LinkProgram(program);
}

inline GLboolean QOpenGLFunctions_2_0::glIsShader(GLuint shader)
{
    return d_2_0_Core->IsShader(shader);
}

inline GLboolean QOpenGLFunctions_2_0::glIsProgram(GLuint program)
{
    return d_2_0_Core->IsProgram(program);
}

inline void QOpenGLFunctions_2_0::glGetVertexAttribPointerv(GLuint index, GLenum pname, GLvoid* *pointer)
{
    d_2_0_Core->GetVertexAttribPointerv(index, pname, pointer);
}

inline void QOpenGLFunctions_2_0::glGetVertexAttribiv(GLuint index, GLenum pname, GLint *params)
{
    d_2_0_Core->GetVertexAttribiv(index, pname, params);
}

inline void QOpenGLFunctions_2_0::glGetVertexAttribfv(GLuint index, GLenum pname, GLfloat *params)
{
    d_2_0_Core->GetVertexAttribfv(index, pname, params);
}

inline void QOpenGLFunctions_2_0::glGetVertexAttribdv(GLuint index, GLenum pname, GLdouble *params)
{
    d_2_0_Core->GetVertexAttribdv(index, pname, params);
}

inline void QOpenGLFunctions_2_0::glGetUniformiv(GLuint program, GLint location, GLint *params)
{
    d_2_0_Core->GetUniformiv(program, location, params);
}

inline void QOpenGLFunctions_2_0::glGetUniformfv(GLuint program, GLint location, GLfloat *params)
{
    d_2_0_Core->GetUniformfv(program, location, params);
}

inline GLint QOpenGLFunctions_2_0::glGetUniformLocation(GLuint program, const GLchar *name)
{
    return d_2_0_Core->GetUniformLocation(program, name);
}

inline void QOpenGLFunctions_2_0::glGetShaderSource(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source)
{
    d_2_0_Core->GetShaderSource(shader, bufSize, length, source);
}

inline void QOpenGLFunctions_2_0::glGetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog)
{
    d_2_0_Core->GetShaderInfoLog(shader, bufSize, length, infoLog);
}

inline void QOpenGLFunctions_2_0::glGetShaderiv(GLuint shader, GLenum pname, GLint *params)
{
    d_2_0_Core->GetShaderiv(shader, pname, params);
}

inline void QOpenGLFunctions_2_0::glGetProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog)
{
    d_2_0_Core->GetProgramInfoLog(program, bufSize, length, infoLog);
}

inline void QOpenGLFunctions_2_0::glGetProgramiv(GLuint program, GLenum pname, GLint *params)
{
    d_2_0_Core->GetProgramiv(program, pname, params);
}

inline GLint QOpenGLFunctions_2_0::glGetAttribLocation(GLuint program, const GLchar *name)
{
    return d_2_0_Core->GetAttribLocation(program, name);
}

inline void QOpenGLFunctions_2_0::glGetAttachedShaders(GLuint program, GLsizei maxCount, GLsizei *count, GLuint *obj)
{
    d_2_0_Core->GetAttachedShaders(program, maxCount, count, obj);
}

inline void QOpenGLFunctions_2_0::glGetActiveUniform(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name)
{
    d_2_0_Core->GetActiveUniform(program, index, bufSize, length, size, type, name);
}

inline void QOpenGLFunctions_2_0::glGetActiveAttrib(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name)
{
    d_2_0_Core->GetActiveAttrib(program, index, bufSize, length, size, type, name);
}

inline void QOpenGLFunctions_2_0::glEnableVertexAttribArray(GLuint index)
{
    d_2_0_Core->EnableVertexAttribArray(index);
}

inline void QOpenGLFunctions_2_0::glDisableVertexAttribArray(GLuint index)
{
    d_2_0_Core->DisableVertexAttribArray(index);
}

inline void QOpenGLFunctions_2_0::glDetachShader(GLuint program, GLuint shader)
{
    d_2_0_Core->DetachShader(program, shader);
}

inline void QOpenGLFunctions_2_0::glDeleteShader(GLuint shader)
{
    d_2_0_Core->DeleteShader(shader);
}

inline void QOpenGLFunctions_2_0::glDeleteProgram(GLuint program)
{
    d_2_0_Core->DeleteProgram(program);
}

inline GLuint QOpenGLFunctions_2_0::glCreateShader(GLenum type)
{
    return d_2_0_Core->CreateShader(type);
}

inline GLuint QOpenGLFunctions_2_0::glCreateProgram()
{
    return d_2_0_Core->CreateProgram();
}

inline void QOpenGLFunctions_2_0::glCompileShader(GLuint shader)
{
    d_2_0_Core->CompileShader(shader);
}

inline void QOpenGLFunctions_2_0::glBindAttribLocation(GLuint program, GLuint index, const GLchar *name)
{
    d_2_0_Core->BindAttribLocation(program, index, name);
}

inline void QOpenGLFunctions_2_0::glAttachShader(GLuint program, GLuint shader)
{
    d_2_0_Core->AttachShader(program, shader);
}

inline void QOpenGLFunctions_2_0::glStencilMaskSeparate(GLenum face, GLuint mask)
{
    d_2_0_Core->StencilMaskSeparate(face, mask);
}

inline void QOpenGLFunctions_2_0::glStencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask)
{
    d_2_0_Core->StencilFuncSeparate(face, func, ref, mask);
}

inline void QOpenGLFunctions_2_0::glStencilOpSeparate(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass)
{
    d_2_0_Core->StencilOpSeparate(face, sfail, dpfail, dppass);
}

inline void QOpenGLFunctions_2_0::glDrawBuffers(GLsizei n, const GLenum *bufs)
{
    d_2_0_Core->DrawBuffers(n, bufs);
}

inline void QOpenGLFunctions_2_0::glBlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha)
{
    d_2_0_Core->BlendEquationSeparate(modeRGB, modeAlpha);
}


// OpenGL 1.0 deprecated functions
inline void QOpenGLFunctions_2_0::glTranslatef(GLfloat x, GLfloat y, GLfloat z)
{
    d_1_0_Deprecated->Translatef(x, y, z);
}

inline void QOpenGLFunctions_2_0::glTranslated(GLdouble x, GLdouble y, GLdouble z)
{
    d_1_0_Deprecated->Translated(x, y, z);
}

inline void QOpenGLFunctions_2_0::glScalef(GLfloat x, GLfloat y, GLfloat z)
{
    d_1_0_Deprecated->Scalef(x, y, z);
}

inline void QOpenGLFunctions_2_0::glScaled(GLdouble x, GLdouble y, GLdouble z)
{
    d_1_0_Deprecated->Scaled(x, y, z);
}

inline void QOpenGLFunctions_2_0::glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
    d_1_0_Deprecated->Rotatef(angle, x, y, z);
}

inline void QOpenGLFunctions_2_0::glRotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z)
{
    d_1_0_Deprecated->Rotated(angle, x, y, z);
}

inline void QOpenGLFunctions_2_0::glPushMatrix()
{
    d_1_0_Deprecated->PushMatrix();
}

inline void QOpenGLFunctions_2_0::glPopMatrix()
{
    d_1_0_Deprecated->PopMatrix();
}

inline void QOpenGLFunctions_2_0::glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)
{
    d_1_0_Deprecated->Ortho(left, right, bottom, top, zNear, zFar);
}

inline void QOpenGLFunctions_2_0::glMultMatrixd(const GLdouble *m)
{
    d_1_0_Deprecated->MultMatrixd(m);
}

inline void QOpenGLFunctions_2_0::glMultMatrixf(const GLfloat *m)
{
    d_1_0_Deprecated->MultMatrixf(m);
}

inline void QOpenGLFunctions_2_0::glMatrixMode(GLenum mode)
{
    d_1_0_Deprecated->MatrixMode(mode);
}

inline void QOpenGLFunctions_2_0::glLoadMatrixd(const GLdouble *m)
{
    d_1_0_Deprecated->LoadMatrixd(m);
}

inline void QOpenGLFunctions_2_0::glLoadMatrixf(const GLfloat *m)
{
    d_1_0_Deprecated->LoadMatrixf(m);
}

inline void QOpenGLFunctions_2_0::glLoadIdentity()
{
    d_1_0_Deprecated->LoadIdentity();
}

inline void QOpenGLFunctions_2_0::glFrustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)
{
    d_1_0_Deprecated->Frustum(left, right, bottom, top, zNear, zFar);
}

inline GLboolean QOpenGLFunctions_2_0::glIsList(GLuint list)
{
    return d_1_0_Deprecated->IsList(list);
}

inline void QOpenGLFunctions_2_0::glGetTexGeniv(GLenum coord, GLenum pname, GLint *params)
{
    d_1_0_Deprecated->GetTexGeniv(coord, pname, params);
}

inline void QOpenGLFunctions_2_0::glGetTexGenfv(GLenum coord, GLenum pname, GLfloat *params)
{
    d_1_0_Deprecated->GetTexGenfv(coord, pname, params);
}

inline void QOpenGLFunctions_2_0::glGetTexGendv(GLenum coord, GLenum pname, GLdouble *params)
{
    d_1_0_Deprecated->GetTexGendv(coord, pname, params);
}

inline void QOpenGLFunctions_2_0::glGetTexEnviv(GLenum target, GLenum pname, GLint *params)
{
    d_1_0_Deprecated->GetTexEnviv(target, pname, params);
}

inline void QOpenGLFunctions_2_0::glGetTexEnvfv(GLenum target, GLenum pname, GLfloat *params)
{
    d_1_0_Deprecated->GetTexEnvfv(target, pname, params);
}

inline void QOpenGLFunctions_2_0::glGetPolygonStipple(GLubyte *mask)
{
    d_1_0_Deprecated->GetPolygonStipple(mask);
}

inline void QOpenGLFunctions_2_0::glGetPixelMapusv(GLenum map, GLushort *values)
{
    d_1_0_Deprecated->GetPixelMapusv(map, values);
}

inline void QOpenGLFunctions_2_0::glGetPixelMapuiv(GLenum map, GLuint *values)
{
    d_1_0_Deprecated->GetPixelMapuiv(map, values);
}

inline void QOpenGLFunctions_2_0::glGetPixelMapfv(GLenum map, GLfloat *values)
{
    d_1_0_Deprecated->GetPixelMapfv(map, values);
}

inline void QOpenGLFunctions_2_0::glGetMaterialiv(GLenum face, GLenum pname, GLint *params)
{
    d_1_0_Deprecated->GetMaterialiv(face, pname, params);
}

inline void QOpenGLFunctions_2_0::glGetMaterialfv(GLenum face, GLenum pname, GLfloat *params)
{
    d_1_0_Deprecated->GetMaterialfv(face, pname, params);
}

inline void QOpenGLFunctions_2_0::glGetMapiv(GLenum target, GLenum query, GLint *v)
{
    d_1_0_Deprecated->GetMapiv(target, query, v);
}

inline void QOpenGLFunctions_2_0::glGetMapfv(GLenum target, GLenum query, GLfloat *v)
{
    d_1_0_Deprecated->GetMapfv(target, query, v);
}

inline void QOpenGLFunctions_2_0::glGetMapdv(GLenum target, GLenum query, GLdouble *v)
{
    d_1_0_Deprecated->GetMapdv(target, query, v);
}

inline void QOpenGLFunctions_2_0::glGetLightiv(GLenum light, GLenum pname, GLint *params)
{
    d_1_0_Deprecated->GetLightiv(light, pname, params);
}

inline void QOpenGLFunctions_2_0::glGetLightfv(GLenum light, GLenum pname, GLfloat *params)
{
    d_1_0_Deprecated->GetLightfv(light, pname, params);
}

inline void QOpenGLFunctions_2_0::glGetClipPlane(GLenum plane, GLdouble *equation)
{
    d_1_0_Deprecated->GetClipPlane(plane, equation);
}

inline void QOpenGLFunctions_2_0::glDrawPixels(GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels)
{
    d_1_0_Deprecated->DrawPixels(width, height, format, type, pixels);
}

inline void QOpenGLFunctions_2_0::glCopyPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type)
{
    d_1_0_Deprecated->CopyPixels(x, y, width, height, type);
}

inline void QOpenGLFunctions_2_0::glPixelMapusv(GLenum map, GLint mapsize, const GLushort *values)
{
    d_1_0_Deprecated->PixelMapusv(map, mapsize, values);
}

inline void QOpenGLFunctions_2_0::glPixelMapuiv(GLenum map, GLint mapsize, const GLuint *values)
{
    d_1_0_Deprecated->PixelMapuiv(map, mapsize, values);
}

inline void QOpenGLFunctions_2_0::glPixelMapfv(GLenum map, GLint mapsize, const GLfloat *values)
{
    d_1_0_Deprecated->PixelMapfv(map, mapsize, values);
}

inline void QOpenGLFunctions_2_0::glPixelTransferi(GLenum pname, GLint param)
{
    d_1_0_Deprecated->PixelTransferi(pname, param);
}

inline void QOpenGLFunctions_2_0::glPixelTransferf(GLenum pname, GLfloat param)
{
    d_1_0_Deprecated->PixelTransferf(pname, param);
}

inline void QOpenGLFunctions_2_0::glPixelZoom(GLfloat xfactor, GLfloat yfactor)
{
    d_1_0_Deprecated->PixelZoom(xfactor, yfactor);
}

inline void QOpenGLFunctions_2_0::glAlphaFunc(GLenum func, GLfloat ref)
{
    d_1_0_Deprecated->AlphaFunc(func, ref);
}

inline void QOpenGLFunctions_2_0::glEvalPoint2(GLint i, GLint j)
{
    d_1_0_Deprecated->EvalPoint2(i, j);
}

inline void QOpenGLFunctions_2_0::glEvalMesh2(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2)
{
    d_1_0_Deprecated->EvalMesh2(mode, i1, i2, j1, j2);
}

inline void QOpenGLFunctions_2_0::glEvalPoint1(GLint i)
{
    d_1_0_Deprecated->EvalPoint1(i);
}

inline void QOpenGLFunctions_2_0::glEvalMesh1(GLenum mode, GLint i1, GLint i2)
{
    d_1_0_Deprecated->EvalMesh1(mode, i1, i2);
}

inline void QOpenGLFunctions_2_0::glEvalCoord2fv(const GLfloat *u)
{
    d_1_0_Deprecated->EvalCoord2fv(u);
}

inline void QOpenGLFunctions_2_0::glEvalCoord2f(GLfloat u, GLfloat v)
{
    d_1_0_Deprecated->EvalCoord2f(u, v);
}

inline void QOpenGLFunctions_2_0::glEvalCoord2dv(const GLdouble *u)
{
    d_1_0_Deprecated->EvalCoord2dv(u);
}

inline void QOpenGLFunctions_2_0::glEvalCoord2d(GLdouble u, GLdouble v)
{
    d_1_0_Deprecated->EvalCoord2d(u, v);
}

inline void QOpenGLFunctions_2_0::glEvalCoord1fv(const GLfloat *u)
{
    d_1_0_Deprecated->EvalCoord1fv(u);
}

inline void QOpenGLFunctions_2_0::glEvalCoord1f(GLfloat u)
{
    d_1_0_Deprecated->EvalCoord1f(u);
}

inline void QOpenGLFunctions_2_0::glEvalCoord1dv(const GLdouble *u)
{
    d_1_0_Deprecated->EvalCoord1dv(u);
}

inline void QOpenGLFunctions_2_0::glEvalCoord1d(GLdouble u)
{
    d_1_0_Deprecated->EvalCoord1d(u);
}

inline void QOpenGLFunctions_2_0::glMapGrid2f(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2)
{
    d_1_0_Deprecated->MapGrid2f(un, u1, u2, vn, v1, v2);
}

inline void QOpenGLFunctions_2_0::glMapGrid2d(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2)
{
    d_1_0_Deprecated->MapGrid2d(un, u1, u2, vn, v1, v2);
}

inline void QOpenGLFunctions_2_0::glMapGrid1f(GLint un, GLfloat u1, GLfloat u2)
{
    d_1_0_Deprecated->MapGrid1f(un, u1, u2);
}

inline void QOpenGLFunctions_2_0::glMapGrid1d(GLint un, GLdouble u1, GLdouble u2)
{
    d_1_0_Deprecated->MapGrid1d(un, u1, u2);
}

inline void QOpenGLFunctions_2_0::glMap2f(GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points)
{
    d_1_0_Deprecated->Map2f(target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points);
}

inline void QOpenGLFunctions_2_0::glMap2d(GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points)
{
    d_1_0_Deprecated->Map2d(target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points);
}

inline void QOpenGLFunctions_2_0::glMap1f(GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points)
{
    d_1_0_Deprecated->Map1f(target, u1, u2, stride, order, points);
}

inline void QOpenGLFunctions_2_0::glMap1d(GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points)
{
    d_1_0_Deprecated->Map1d(target, u1, u2, stride, order, points);
}

inline void QOpenGLFunctions_2_0::glPushAttrib(GLbitfield mask)
{
    d_1_0_Deprecated->PushAttrib(mask);
}

inline void QOpenGLFunctions_2_0::glPopAttrib()
{
    d_1_0_Deprecated->PopAttrib();
}

inline void QOpenGLFunctions_2_0::glAccum(GLenum op, GLfloat value)
{
    d_1_0_Deprecated->Accum(op, value);
}

inline void QOpenGLFunctions_2_0::glIndexMask(GLuint mask)
{
    d_1_0_Deprecated->IndexMask(mask);
}

inline void QOpenGLFunctions_2_0::glClearIndex(GLfloat c)
{
    d_1_0_Deprecated->ClearIndex(c);
}

inline void QOpenGLFunctions_2_0::glClearAccum(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    d_1_0_Deprecated->ClearAccum(red, green, blue, alpha);
}

inline void QOpenGLFunctions_2_0::glPushName(GLuint name)
{
    d_1_0_Deprecated->PushName(name);
}

inline void QOpenGLFunctions_2_0::glPopName()
{
    d_1_0_Deprecated->PopName();
}

inline void QOpenGLFunctions_2_0::glPassThrough(GLfloat token)
{
    d_1_0_Deprecated->PassThrough(token);
}

inline void QOpenGLFunctions_2_0::glLoadName(GLuint name)
{
    d_1_0_Deprecated->LoadName(name);
}

inline void QOpenGLFunctions_2_0::glInitNames()
{
    d_1_0_Deprecated->InitNames();
}

inline GLint QOpenGLFunctions_2_0::glRenderMode(GLenum mode)
{
    return d_1_0_Deprecated->RenderMode(mode);
}

inline void QOpenGLFunctions_2_0::glSelectBuffer(GLsizei size, GLuint *buffer)
{
    d_1_0_Deprecated->SelectBuffer(size, buffer);
}

inline void QOpenGLFunctions_2_0::glFeedbackBuffer(GLsizei size, GLenum type, GLfloat *buffer)
{
    d_1_0_Deprecated->FeedbackBuffer(size, type, buffer);
}

inline void QOpenGLFunctions_2_0::glTexGeniv(GLenum coord, GLenum pname, const GLint *params)
{
    d_1_0_Deprecated->TexGeniv(coord, pname, params);
}

inline void QOpenGLFunctions_2_0::glTexGeni(GLenum coord, GLenum pname, GLint param)
{
    d_1_0_Deprecated->TexGeni(coord, pname, param);
}

inline void QOpenGLFunctions_2_0::glTexGenfv(GLenum coord, GLenum pname, const GLfloat *params)
{
    d_1_0_Deprecated->TexGenfv(coord, pname, params);
}

inline void QOpenGLFunctions_2_0::glTexGenf(GLenum coord, GLenum pname, GLfloat param)
{
    d_1_0_Deprecated->TexGenf(coord, pname, param);
}

inline void QOpenGLFunctions_2_0::glTexGendv(GLenum coord, GLenum pname, const GLdouble *params)
{
    d_1_0_Deprecated->TexGendv(coord, pname, params);
}

inline void QOpenGLFunctions_2_0::glTexGend(GLenum coord, GLenum pname, GLdouble param)
{
    d_1_0_Deprecated->TexGend(coord, pname, param);
}

inline void QOpenGLFunctions_2_0::glTexEnviv(GLenum target, GLenum pname, const GLint *params)
{
    d_1_0_Deprecated->TexEnviv(target, pname, params);
}

inline void QOpenGLFunctions_2_0::glTexEnvi(GLenum target, GLenum pname, GLint param)
{
    d_1_0_Deprecated->TexEnvi(target, pname, param);
}

inline void QOpenGLFunctions_2_0::glTexEnvfv(GLenum target, GLenum pname, const GLfloat *params)
{
    d_1_0_Deprecated->TexEnvfv(target, pname, params);
}

inline void QOpenGLFunctions_2_0::glTexEnvf(GLenum target, GLenum pname, GLfloat param)
{
    d_1_0_Deprecated->TexEnvf(target, pname, param);
}

inline void QOpenGLFunctions_2_0::glShadeModel(GLenum mode)
{
    d_1_0_Deprecated->ShadeModel(mode);
}

inline void QOpenGLFunctions_2_0::glPolygonStipple(const GLubyte *mask)
{
    d_1_0_Deprecated->PolygonStipple(mask);
}

inline void QOpenGLFunctions_2_0::glMaterialiv(GLenum face, GLenum pname, const GLint *params)
{
    d_1_0_Deprecated->Materialiv(face, pname, params);
}

inline void QOpenGLFunctions_2_0::glMateriali(GLenum face, GLenum pname, GLint param)
{
    d_1_0_Deprecated->Materiali(face, pname, param);
}

inline void QOpenGLFunctions_2_0::glMaterialfv(GLenum face, GLenum pname, const GLfloat *params)
{
    d_1_0_Deprecated->Materialfv(face, pname, params);
}

inline void QOpenGLFunctions_2_0::glMaterialf(GLenum face, GLenum pname, GLfloat param)
{
    d_1_0_Deprecated->Materialf(face, pname, param);
}

inline void QOpenGLFunctions_2_0::glLineStipple(GLint factor, GLushort pattern)
{
    d_1_0_Deprecated->LineStipple(factor, pattern);
}

inline void QOpenGLFunctions_2_0::glLightModeliv(GLenum pname, const GLint *params)
{
    d_1_0_Deprecated->LightModeliv(pname, params);
}

inline void QOpenGLFunctions_2_0::glLightModeli(GLenum pname, GLint param)
{
    d_1_0_Deprecated->LightModeli(pname, param);
}

inline void QOpenGLFunctions_2_0::glLightModelfv(GLenum pname, const GLfloat *params)
{
    d_1_0_Deprecated->LightModelfv(pname, params);
}

inline void QOpenGLFunctions_2_0::glLightModelf(GLenum pname, GLfloat param)
{
    d_1_0_Deprecated->LightModelf(pname, param);
}

inline void QOpenGLFunctions_2_0::glLightiv(GLenum light, GLenum pname, const GLint *params)
{
    d_1_0_Deprecated->Lightiv(light, pname, params);
}

inline void QOpenGLFunctions_2_0::glLighti(GLenum light, GLenum pname, GLint param)
{
    d_1_0_Deprecated->Lighti(light, pname, param);
}

inline void QOpenGLFunctions_2_0::glLightfv(GLenum light, GLenum pname, const GLfloat *params)
{
    d_1_0_Deprecated->Lightfv(light, pname, params);
}

inline void QOpenGLFunctions_2_0::glLightf(GLenum light, GLenum pname, GLfloat param)
{
    d_1_0_Deprecated->Lightf(light, pname, param);
}

inline void QOpenGLFunctions_2_0::glFogiv(GLenum pname, const GLint *params)
{
    d_1_0_Deprecated->Fogiv(pname, params);
}

inline void QOpenGLFunctions_2_0::glFogi(GLenum pname, GLint param)
{
    d_1_0_Deprecated->Fogi(pname, param);
}

inline void QOpenGLFunctions_2_0::glFogfv(GLenum pname, const GLfloat *params)
{
    d_1_0_Deprecated->Fogfv(pname, params);
}

inline void QOpenGLFunctions_2_0::glFogf(GLenum pname, GLfloat param)
{
    d_1_0_Deprecated->Fogf(pname, param);
}

inline void QOpenGLFunctions_2_0::glColorMaterial(GLenum face, GLenum mode)
{
    d_1_0_Deprecated->ColorMaterial(face, mode);
}

inline void QOpenGLFunctions_2_0::glClipPlane(GLenum plane, const GLdouble *equation)
{
    d_1_0_Deprecated->ClipPlane(plane, equation);
}

inline void QOpenGLFunctions_2_0::glVertex4sv(const GLshort *v)
{
    d_1_0_Deprecated->Vertex4sv(v);
}

inline void QOpenGLFunctions_2_0::glVertex4s(GLshort x, GLshort y, GLshort z, GLshort w)
{
    d_1_0_Deprecated->Vertex4s(x, y, z, w);
}

inline void QOpenGLFunctions_2_0::glVertex4iv(const GLint *v)
{
    d_1_0_Deprecated->Vertex4iv(v);
}

inline void QOpenGLFunctions_2_0::glVertex4i(GLint x, GLint y, GLint z, GLint w)
{
    d_1_0_Deprecated->Vertex4i(x, y, z, w);
}

inline void QOpenGLFunctions_2_0::glVertex4fv(const GLfloat *v)
{
    d_1_0_Deprecated->Vertex4fv(v);
}

inline void QOpenGLFunctions_2_0::glVertex4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    d_1_0_Deprecated->Vertex4f(x, y, z, w);
}

inline void QOpenGLFunctions_2_0::glVertex4dv(const GLdouble *v)
{
    d_1_0_Deprecated->Vertex4dv(v);
}

inline void QOpenGLFunctions_2_0::glVertex4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    d_1_0_Deprecated->Vertex4d(x, y, z, w);
}

inline void QOpenGLFunctions_2_0::glVertex3sv(const GLshort *v)
{
    d_1_0_Deprecated->Vertex3sv(v);
}

inline void QOpenGLFunctions_2_0::glVertex3s(GLshort x, GLshort y, GLshort z)
{
    d_1_0_Deprecated->Vertex3s(x, y, z);
}

inline void QOpenGLFunctions_2_0::glVertex3iv(const GLint *v)
{
    d_1_0_Deprecated->Vertex3iv(v);
}

inline void QOpenGLFunctions_2_0::glVertex3i(GLint x, GLint y, GLint z)
{
    d_1_0_Deprecated->Vertex3i(x, y, z);
}

inline void QOpenGLFunctions_2_0::glVertex3fv(const GLfloat *v)
{
    d_1_0_Deprecated->Vertex3fv(v);
}

inline void QOpenGLFunctions_2_0::glVertex3f(GLfloat x, GLfloat y, GLfloat z)
{
    d_1_0_Deprecated->Vertex3f(x, y, z);
}

inline void QOpenGLFunctions_2_0::glVertex3dv(const GLdouble *v)
{
    d_1_0_Deprecated->Vertex3dv(v);
}

inline void QOpenGLFunctions_2_0::glVertex3d(GLdouble x, GLdouble y, GLdouble z)
{
    d_1_0_Deprecated->Vertex3d(x, y, z);
}

inline void QOpenGLFunctions_2_0::glVertex2sv(const GLshort *v)
{
    d_1_0_Deprecated->Vertex2sv(v);
}

inline void QOpenGLFunctions_2_0::glVertex2s(GLshort x, GLshort y)
{
    d_1_0_Deprecated->Vertex2s(x, y);
}

inline void QOpenGLFunctions_2_0::glVertex2iv(const GLint *v)
{
    d_1_0_Deprecated->Vertex2iv(v);
}

inline void QOpenGLFunctions_2_0::glVertex2i(GLint x, GLint y)
{
    d_1_0_Deprecated->Vertex2i(x, y);
}

inline void QOpenGLFunctions_2_0::glVertex2fv(const GLfloat *v)
{
    d_1_0_Deprecated->Vertex2fv(v);
}

inline void QOpenGLFunctions_2_0::glVertex2f(GLfloat x, GLfloat y)
{
    d_1_0_Deprecated->Vertex2f(x, y);
}

inline void QOpenGLFunctions_2_0::glVertex2dv(const GLdouble *v)
{
    d_1_0_Deprecated->Vertex2dv(v);
}

inline void QOpenGLFunctions_2_0::glVertex2d(GLdouble x, GLdouble y)
{
    d_1_0_Deprecated->Vertex2d(x, y);
}

inline void QOpenGLFunctions_2_0::glTexCoord4sv(const GLshort *v)
{
    d_1_0_Deprecated->TexCoord4sv(v);
}

inline void QOpenGLFunctions_2_0::glTexCoord4s(GLshort s, GLshort t, GLshort r, GLshort q)
{
    d_1_0_Deprecated->TexCoord4s(s, t, r, q);
}

inline void QOpenGLFunctions_2_0::glTexCoord4iv(const GLint *v)
{
    d_1_0_Deprecated->TexCoord4iv(v);
}

inline void QOpenGLFunctions_2_0::glTexCoord4i(GLint s, GLint t, GLint r, GLint q)
{
    d_1_0_Deprecated->TexCoord4i(s, t, r, q);
}

inline void QOpenGLFunctions_2_0::glTexCoord4fv(const GLfloat *v)
{
    d_1_0_Deprecated->TexCoord4fv(v);
}

inline void QOpenGLFunctions_2_0::glTexCoord4f(GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
    d_1_0_Deprecated->TexCoord4f(s, t, r, q);
}

inline void QOpenGLFunctions_2_0::glTexCoord4dv(const GLdouble *v)
{
    d_1_0_Deprecated->TexCoord4dv(v);
}

inline void QOpenGLFunctions_2_0::glTexCoord4d(GLdouble s, GLdouble t, GLdouble r, GLdouble q)
{
    d_1_0_Deprecated->TexCoord4d(s, t, r, q);
}

inline void QOpenGLFunctions_2_0::glTexCoord3sv(const GLshort *v)
{
    d_1_0_Deprecated->TexCoord3sv(v);
}

inline void QOpenGLFunctions_2_0::glTexCoord3s(GLshort s, GLshort t, GLshort r)
{
    d_1_0_Deprecated->TexCoord3s(s, t, r);
}

inline void QOpenGLFunctions_2_0::glTexCoord3iv(const GLint *v)
{
    d_1_0_Deprecated->TexCoord3iv(v);
}

inline void QOpenGLFunctions_2_0::glTexCoord3i(GLint s, GLint t, GLint r)
{
    d_1_0_Deprecated->TexCoord3i(s, t, r);
}

inline void QOpenGLFunctions_2_0::glTexCoord3fv(const GLfloat *v)
{
    d_1_0_Deprecated->TexCoord3fv(v);
}

inline void QOpenGLFunctions_2_0::glTexCoord3f(GLfloat s, GLfloat t, GLfloat r)
{
    d_1_0_Deprecated->TexCoord3f(s, t, r);
}

inline void QOpenGLFunctions_2_0::glTexCoord3dv(const GLdouble *v)
{
    d_1_0_Deprecated->TexCoord3dv(v);
}

inline void QOpenGLFunctions_2_0::glTexCoord3d(GLdouble s, GLdouble t, GLdouble r)
{
    d_1_0_Deprecated->TexCoord3d(s, t, r);
}

inline void QOpenGLFunctions_2_0::glTexCoord2sv(const GLshort *v)
{
    d_1_0_Deprecated->TexCoord2sv(v);
}

inline void QOpenGLFunctions_2_0::glTexCoord2s(GLshort s, GLshort t)
{
    d_1_0_Deprecated->TexCoord2s(s, t);
}

inline void QOpenGLFunctions_2_0::glTexCoord2iv(const GLint *v)
{
    d_1_0_Deprecated->TexCoord2iv(v);
}

inline void QOpenGLFunctions_2_0::glTexCoord2i(GLint s, GLint t)
{
    d_1_0_Deprecated->TexCoord2i(s, t);
}

inline void QOpenGLFunctions_2_0::glTexCoord2fv(const GLfloat *v)
{
    d_1_0_Deprecated->TexCoord2fv(v);
}

inline void QOpenGLFunctions_2_0::glTexCoord2f(GLfloat s, GLfloat t)
{
    d_1_0_Deprecated->TexCoord2f(s, t);
}

inline void QOpenGLFunctions_2_0::glTexCoord2dv(const GLdouble *v)
{
    d_1_0_Deprecated->TexCoord2dv(v);
}

inline void QOpenGLFunctions_2_0::glTexCoord2d(GLdouble s, GLdouble t)
{
    d_1_0_Deprecated->TexCoord2d(s, t);
}

inline void QOpenGLFunctions_2_0::glTexCoord1sv(const GLshort *v)
{
    d_1_0_Deprecated->TexCoord1sv(v);
}

inline void QOpenGLFunctions_2_0::glTexCoord1s(GLshort s)
{
    d_1_0_Deprecated->TexCoord1s(s);
}

inline void QOpenGLFunctions_2_0::glTexCoord1iv(const GLint *v)
{
    d_1_0_Deprecated->TexCoord1iv(v);
}

inline void QOpenGLFunctions_2_0::glTexCoord1i(GLint s)
{
    d_1_0_Deprecated->TexCoord1i(s);
}

inline void QOpenGLFunctions_2_0::glTexCoord1fv(const GLfloat *v)
{
    d_1_0_Deprecated->TexCoord1fv(v);
}

inline void QOpenGLFunctions_2_0::glTexCoord1f(GLfloat s)
{
    d_1_0_Deprecated->TexCoord1f(s);
}

inline void QOpenGLFunctions_2_0::glTexCoord1dv(const GLdouble *v)
{
    d_1_0_Deprecated->TexCoord1dv(v);
}

inline void QOpenGLFunctions_2_0::glTexCoord1d(GLdouble s)
{
    d_1_0_Deprecated->TexCoord1d(s);
}

inline void QOpenGLFunctions_2_0::glRectsv(const GLshort *v1, const GLshort *v2)
{
    d_1_0_Deprecated->Rectsv(v1, v2);
}

inline void QOpenGLFunctions_2_0::glRects(GLshort x1, GLshort y1, GLshort x2, GLshort y2)
{
    d_1_0_Deprecated->Rects(x1, y1, x2, y2);
}

inline void QOpenGLFunctions_2_0::glRectiv(const GLint *v1, const GLint *v2)
{
    d_1_0_Deprecated->Rectiv(v1, v2);
}

inline void QOpenGLFunctions_2_0::glRecti(GLint x1, GLint y1, GLint x2, GLint y2)
{
    d_1_0_Deprecated->Recti(x1, y1, x2, y2);
}

inline void QOpenGLFunctions_2_0::glRectfv(const GLfloat *v1, const GLfloat *v2)
{
    d_1_0_Deprecated->Rectfv(v1, v2);
}

inline void QOpenGLFunctions_2_0::glRectf(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2)
{
    d_1_0_Deprecated->Rectf(x1, y1, x2, y2);
}

inline void QOpenGLFunctions_2_0::glRectdv(const GLdouble *v1, const GLdouble *v2)
{
    d_1_0_Deprecated->Rectdv(v1, v2);
}

inline void QOpenGLFunctions_2_0::glRectd(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2)
{
    d_1_0_Deprecated->Rectd(x1, y1, x2, y2);
}

inline void QOpenGLFunctions_2_0::glRasterPos4sv(const GLshort *v)
{
    d_1_0_Deprecated->RasterPos4sv(v);
}

inline void QOpenGLFunctions_2_0::glRasterPos4s(GLshort x, GLshort y, GLshort z, GLshort w)
{
    d_1_0_Deprecated->RasterPos4s(x, y, z, w);
}

inline void QOpenGLFunctions_2_0::glRasterPos4iv(const GLint *v)
{
    d_1_0_Deprecated->RasterPos4iv(v);
}

inline void QOpenGLFunctions_2_0::glRasterPos4i(GLint x, GLint y, GLint z, GLint w)
{
    d_1_0_Deprecated->RasterPos4i(x, y, z, w);
}

inline void QOpenGLFunctions_2_0::glRasterPos4fv(const GLfloat *v)
{
    d_1_0_Deprecated->RasterPos4fv(v);
}

inline void QOpenGLFunctions_2_0::glRasterPos4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    d_1_0_Deprecated->RasterPos4f(x, y, z, w);
}

inline void QOpenGLFunctions_2_0::glRasterPos4dv(const GLdouble *v)
{
    d_1_0_Deprecated->RasterPos4dv(v);
}

inline void QOpenGLFunctions_2_0::glRasterPos4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    d_1_0_Deprecated->RasterPos4d(x, y, z, w);
}

inline void QOpenGLFunctions_2_0::glRasterPos3sv(const GLshort *v)
{
    d_1_0_Deprecated->RasterPos3sv(v);
}

inline void QOpenGLFunctions_2_0::glRasterPos3s(GLshort x, GLshort y, GLshort z)
{
    d_1_0_Deprecated->RasterPos3s(x, y, z);
}

inline void QOpenGLFunctions_2_0::glRasterPos3iv(const GLint *v)
{
    d_1_0_Deprecated->RasterPos3iv(v);
}

inline void QOpenGLFunctions_2_0::glRasterPos3i(GLint x, GLint y, GLint z)
{
    d_1_0_Deprecated->RasterPos3i(x, y, z);
}

inline void QOpenGLFunctions_2_0::glRasterPos3fv(const GLfloat *v)
{
    d_1_0_Deprecated->RasterPos3fv(v);
}

inline void QOpenGLFunctions_2_0::glRasterPos3f(GLfloat x, GLfloat y, GLfloat z)
{
    d_1_0_Deprecated->RasterPos3f(x, y, z);
}

inline void QOpenGLFunctions_2_0::glRasterPos3dv(const GLdouble *v)
{
    d_1_0_Deprecated->RasterPos3dv(v);
}

inline void QOpenGLFunctions_2_0::glRasterPos3d(GLdouble x, GLdouble y, GLdouble z)
{
    d_1_0_Deprecated->RasterPos3d(x, y, z);
}

inline void QOpenGLFunctions_2_0::glRasterPos2sv(const GLshort *v)
{
    d_1_0_Deprecated->RasterPos2sv(v);
}

inline void QOpenGLFunctions_2_0::glRasterPos2s(GLshort x, GLshort y)
{
    d_1_0_Deprecated->RasterPos2s(x, y);
}

inline void QOpenGLFunctions_2_0::glRasterPos2iv(const GLint *v)
{
    d_1_0_Deprecated->RasterPos2iv(v);
}

inline void QOpenGLFunctions_2_0::glRasterPos2i(GLint x, GLint y)
{
    d_1_0_Deprecated->RasterPos2i(x, y);
}

inline void QOpenGLFunctions_2_0::glRasterPos2fv(const GLfloat *v)
{
    d_1_0_Deprecated->RasterPos2fv(v);
}

inline void QOpenGLFunctions_2_0::glRasterPos2f(GLfloat x, GLfloat y)
{
    d_1_0_Deprecated->RasterPos2f(x, y);
}

inline void QOpenGLFunctions_2_0::glRasterPos2dv(const GLdouble *v)
{
    d_1_0_Deprecated->RasterPos2dv(v);
}

inline void QOpenGLFunctions_2_0::glRasterPos2d(GLdouble x, GLdouble y)
{
    d_1_0_Deprecated->RasterPos2d(x, y);
}

inline void QOpenGLFunctions_2_0::glNormal3sv(const GLshort *v)
{
    d_1_0_Deprecated->Normal3sv(v);
}

inline void QOpenGLFunctions_2_0::glNormal3s(GLshort nx, GLshort ny, GLshort nz)
{
    d_1_0_Deprecated->Normal3s(nx, ny, nz);
}

inline void QOpenGLFunctions_2_0::glNormal3iv(const GLint *v)
{
    d_1_0_Deprecated->Normal3iv(v);
}

inline void QOpenGLFunctions_2_0::glNormal3i(GLint nx, GLint ny, GLint nz)
{
    d_1_0_Deprecated->Normal3i(nx, ny, nz);
}

inline void QOpenGLFunctions_2_0::glNormal3fv(const GLfloat *v)
{
    d_1_0_Deprecated->Normal3fv(v);
}

inline void QOpenGLFunctions_2_0::glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz)
{
    d_1_0_Deprecated->Normal3f(nx, ny, nz);
}

inline void QOpenGLFunctions_2_0::glNormal3dv(const GLdouble *v)
{
    d_1_0_Deprecated->Normal3dv(v);
}

inline void QOpenGLFunctions_2_0::glNormal3d(GLdouble nx, GLdouble ny, GLdouble nz)
{
    d_1_0_Deprecated->Normal3d(nx, ny, nz);
}

inline void QOpenGLFunctions_2_0::glNormal3bv(const GLbyte *v)
{
    d_1_0_Deprecated->Normal3bv(v);
}

inline void QOpenGLFunctions_2_0::glNormal3b(GLbyte nx, GLbyte ny, GLbyte nz)
{
    d_1_0_Deprecated->Normal3b(nx, ny, nz);
}

inline void QOpenGLFunctions_2_0::glIndexsv(const GLshort *c)
{
    d_1_0_Deprecated->Indexsv(c);
}

inline void QOpenGLFunctions_2_0::glIndexs(GLshort c)
{
    d_1_0_Deprecated->Indexs(c);
}

inline void QOpenGLFunctions_2_0::glIndexiv(const GLint *c)
{
    d_1_0_Deprecated->Indexiv(c);
}

inline void QOpenGLFunctions_2_0::glIndexi(GLint c)
{
    d_1_0_Deprecated->Indexi(c);
}

inline void QOpenGLFunctions_2_0::glIndexfv(const GLfloat *c)
{
    d_1_0_Deprecated->Indexfv(c);
}

inline void QOpenGLFunctions_2_0::glIndexf(GLfloat c)
{
    d_1_0_Deprecated->Indexf(c);
}

inline void QOpenGLFunctions_2_0::glIndexdv(const GLdouble *c)
{
    d_1_0_Deprecated->Indexdv(c);
}

inline void QOpenGLFunctions_2_0::glIndexd(GLdouble c)
{
    d_1_0_Deprecated->Indexd(c);
}

inline void QOpenGLFunctions_2_0::glEnd()
{
    d_1_0_Deprecated->End();
}

inline void QOpenGLFunctions_2_0::glEdgeFlagv(const GLboolean *flag)
{
    d_1_0_Deprecated->EdgeFlagv(flag);
}

inline void QOpenGLFunctions_2_0::glEdgeFlag(GLboolean flag)
{
    d_1_0_Deprecated->EdgeFlag(flag);
}

inline void QOpenGLFunctions_2_0::glColor4usv(const GLushort *v)
{
    d_1_0_Deprecated->Color4usv(v);
}

inline void QOpenGLFunctions_2_0::glColor4us(GLushort red, GLushort green, GLushort blue, GLushort alpha)
{
    d_1_0_Deprecated->Color4us(red, green, blue, alpha);
}

inline void QOpenGLFunctions_2_0::glColor4uiv(const GLuint *v)
{
    d_1_0_Deprecated->Color4uiv(v);
}

inline void QOpenGLFunctions_2_0::glColor4ui(GLuint red, GLuint green, GLuint blue, GLuint alpha)
{
    d_1_0_Deprecated->Color4ui(red, green, blue, alpha);
}

inline void QOpenGLFunctions_2_0::glColor4ubv(const GLubyte *v)
{
    d_1_0_Deprecated->Color4ubv(v);
}

inline void QOpenGLFunctions_2_0::glColor4ub(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha)
{
    d_1_0_Deprecated->Color4ub(red, green, blue, alpha);
}

inline void QOpenGLFunctions_2_0::glColor4sv(const GLshort *v)
{
    d_1_0_Deprecated->Color4sv(v);
}

inline void QOpenGLFunctions_2_0::glColor4s(GLshort red, GLshort green, GLshort blue, GLshort alpha)
{
    d_1_0_Deprecated->Color4s(red, green, blue, alpha);
}

inline void QOpenGLFunctions_2_0::glColor4iv(const GLint *v)
{
    d_1_0_Deprecated->Color4iv(v);
}

inline void QOpenGLFunctions_2_0::glColor4i(GLint red, GLint green, GLint blue, GLint alpha)
{
    d_1_0_Deprecated->Color4i(red, green, blue, alpha);
}

inline void QOpenGLFunctions_2_0::glColor4fv(const GLfloat *v)
{
    d_1_0_Deprecated->Color4fv(v);
}

inline void QOpenGLFunctions_2_0::glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    d_1_0_Deprecated->Color4f(red, green, blue, alpha);
}

inline void QOpenGLFunctions_2_0::glColor4dv(const GLdouble *v)
{
    d_1_0_Deprecated->Color4dv(v);
}

inline void QOpenGLFunctions_2_0::glColor4d(GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha)
{
    d_1_0_Deprecated->Color4d(red, green, blue, alpha);
}

inline void QOpenGLFunctions_2_0::glColor4bv(const GLbyte *v)
{
    d_1_0_Deprecated->Color4bv(v);
}

inline void QOpenGLFunctions_2_0::glColor4b(GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha)
{
    d_1_0_Deprecated->Color4b(red, green, blue, alpha);
}

inline void QOpenGLFunctions_2_0::glColor3usv(const GLushort *v)
{
    d_1_0_Deprecated->Color3usv(v);
}

inline void QOpenGLFunctions_2_0::glColor3us(GLushort red, GLushort green, GLushort blue)
{
    d_1_0_Deprecated->Color3us(red, green, blue);
}

inline void QOpenGLFunctions_2_0::glColor3uiv(const GLuint *v)
{
    d_1_0_Deprecated->Color3uiv(v);
}

inline void QOpenGLFunctions_2_0::glColor3ui(GLuint red, GLuint green, GLuint blue)
{
    d_1_0_Deprecated->Color3ui(red, green, blue);
}

inline void QOpenGLFunctions_2_0::glColor3ubv(const GLubyte *v)
{
    d_1_0_Deprecated->Color3ubv(v);
}

inline void QOpenGLFunctions_2_0::glColor3ub(GLubyte red, GLubyte green, GLubyte blue)
{
    d_1_0_Deprecated->Color3ub(red, green, blue);
}

inline void QOpenGLFunctions_2_0::glColor3sv(const GLshort *v)
{
    d_1_0_Deprecated->Color3sv(v);
}

inline void QOpenGLFunctions_2_0::glColor3s(GLshort red, GLshort green, GLshort blue)
{
    d_1_0_Deprecated->Color3s(red, green, blue);
}

inline void QOpenGLFunctions_2_0::glColor3iv(const GLint *v)
{
    d_1_0_Deprecated->Color3iv(v);
}

inline void QOpenGLFunctions_2_0::glColor3i(GLint red, GLint green, GLint blue)
{
    d_1_0_Deprecated->Color3i(red, green, blue);
}

inline void QOpenGLFunctions_2_0::glColor3fv(const GLfloat *v)
{
    d_1_0_Deprecated->Color3fv(v);
}

inline void QOpenGLFunctions_2_0::glColor3f(GLfloat red, GLfloat green, GLfloat blue)
{
    d_1_0_Deprecated->Color3f(red, green, blue);
}

inline void QOpenGLFunctions_2_0::glColor3dv(const GLdouble *v)
{
    d_1_0_Deprecated->Color3dv(v);
}

inline void QOpenGLFunctions_2_0::glColor3d(GLdouble red, GLdouble green, GLdouble blue)
{
    d_1_0_Deprecated->Color3d(red, green, blue);
}

inline void QOpenGLFunctions_2_0::glColor3bv(const GLbyte *v)
{
    d_1_0_Deprecated->Color3bv(v);
}

inline void QOpenGLFunctions_2_0::glColor3b(GLbyte red, GLbyte green, GLbyte blue)
{
    d_1_0_Deprecated->Color3b(red, green, blue);
}

inline void QOpenGLFunctions_2_0::glBitmap(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap)
{
    d_1_0_Deprecated->Bitmap(width, height, xorig, yorig, xmove, ymove, bitmap);
}

inline void QOpenGLFunctions_2_0::glBegin(GLenum mode)
{
    d_1_0_Deprecated->Begin(mode);
}

inline void QOpenGLFunctions_2_0::glListBase(GLuint base)
{
    d_1_0_Deprecated->ListBase(base);
}

inline GLuint QOpenGLFunctions_2_0::glGenLists(GLsizei range)
{
    return d_1_0_Deprecated->GenLists(range);
}

inline void QOpenGLFunctions_2_0::glDeleteLists(GLuint list, GLsizei range)
{
    d_1_0_Deprecated->DeleteLists(list, range);
}

inline void QOpenGLFunctions_2_0::glCallLists(GLsizei n, GLenum type, const GLvoid *lists)
{
    d_1_0_Deprecated->CallLists(n, type, lists);
}

inline void QOpenGLFunctions_2_0::glCallList(GLuint list)
{
    d_1_0_Deprecated->CallList(list);
}

inline void QOpenGLFunctions_2_0::glEndList()
{
    d_1_0_Deprecated->EndList();
}

inline void QOpenGLFunctions_2_0::glNewList(GLuint list, GLenum mode)
{
    d_1_0_Deprecated->NewList(list, mode);
}


// OpenGL 1.1 deprecated functions
inline void QOpenGLFunctions_2_0::glPushClientAttrib(GLbitfield mask)
{
    d_1_1_Deprecated->PushClientAttrib(mask);
}

inline void QOpenGLFunctions_2_0::glPopClientAttrib()
{
    d_1_1_Deprecated->PopClientAttrib();
}

inline void QOpenGLFunctions_2_0::glPrioritizeTextures(GLsizei n, const GLuint *textures, const GLfloat *priorities)
{
    d_1_1_Deprecated->PrioritizeTextures(n, textures, priorities);
}

inline GLboolean QOpenGLFunctions_2_0::glAreTexturesResident(GLsizei n, const GLuint *textures, GLboolean *residences)
{
    return d_1_1_Deprecated->AreTexturesResident(n, textures, residences);
}

inline void QOpenGLFunctions_2_0::glVertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
    d_1_1_Deprecated->VertexPointer(size, type, stride, pointer);
}

inline void QOpenGLFunctions_2_0::glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
    d_1_1_Deprecated->TexCoordPointer(size, type, stride, pointer);
}

inline void QOpenGLFunctions_2_0::glNormalPointer(GLenum type, GLsizei stride, const GLvoid *pointer)
{
    d_1_1_Deprecated->NormalPointer(type, stride, pointer);
}

inline void QOpenGLFunctions_2_0::glInterleavedArrays(GLenum format, GLsizei stride, const GLvoid *pointer)
{
    d_1_1_Deprecated->InterleavedArrays(format, stride, pointer);
}

inline void QOpenGLFunctions_2_0::glIndexPointer(GLenum type, GLsizei stride, const GLvoid *pointer)
{
    d_1_1_Deprecated->IndexPointer(type, stride, pointer);
}

inline void QOpenGLFunctions_2_0::glEnableClientState(GLenum array)
{
    d_1_1_Deprecated->EnableClientState(array);
}

inline void QOpenGLFunctions_2_0::glEdgeFlagPointer(GLsizei stride, const GLvoid *pointer)
{
    d_1_1_Deprecated->EdgeFlagPointer(stride, pointer);
}

inline void QOpenGLFunctions_2_0::glDisableClientState(GLenum array)
{
    d_1_1_Deprecated->DisableClientState(array);
}

inline void QOpenGLFunctions_2_0::glColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
    d_1_1_Deprecated->ColorPointer(size, type, stride, pointer);
}

inline void QOpenGLFunctions_2_0::glArrayElement(GLint i)
{
    d_1_1_Deprecated->ArrayElement(i);
}


// OpenGL 1.2 deprecated functions
inline void QOpenGLFunctions_2_0::glResetMinmax(GLenum target)
{
    d_1_2_Deprecated->ResetMinmax(target);
}

inline void QOpenGLFunctions_2_0::glResetHistogram(GLenum target)
{
    d_1_2_Deprecated->ResetHistogram(target);
}

inline void QOpenGLFunctions_2_0::glMinmax(GLenum target, GLenum internalformat, GLboolean sink)
{
    d_1_2_Deprecated->Minmax(target, internalformat, sink);
}

inline void QOpenGLFunctions_2_0::glHistogram(GLenum target, GLsizei width, GLenum internalformat, GLboolean sink)
{
    d_1_2_Deprecated->Histogram(target, width, internalformat, sink);
}

inline void QOpenGLFunctions_2_0::glGetMinmaxParameteriv(GLenum target, GLenum pname, GLint *params)
{
    d_1_2_Deprecated->GetMinmaxParameteriv(target, pname, params);
}

inline void QOpenGLFunctions_2_0::glGetMinmaxParameterfv(GLenum target, GLenum pname, GLfloat *params)
{
    d_1_2_Deprecated->GetMinmaxParameterfv(target, pname, params);
}

inline void QOpenGLFunctions_2_0::glGetMinmax(GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid *values)
{
    d_1_2_Deprecated->GetMinmax(target, reset, format, type, values);
}

inline void QOpenGLFunctions_2_0::glGetHistogramParameteriv(GLenum target, GLenum pname, GLint *params)
{
    d_1_2_Deprecated->GetHistogramParameteriv(target, pname, params);
}

inline void QOpenGLFunctions_2_0::glGetHistogramParameterfv(GLenum target, GLenum pname, GLfloat *params)
{
    d_1_2_Deprecated->GetHistogramParameterfv(target, pname, params);
}

inline void QOpenGLFunctions_2_0::glGetHistogram(GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid *values)
{
    d_1_2_Deprecated->GetHistogram(target, reset, format, type, values);
}

inline void QOpenGLFunctions_2_0::glSeparableFilter2D(GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *row, const GLvoid *column)
{
    d_1_2_Deprecated->SeparableFilter2D(target, internalformat, width, height, format, type, row, column);
}

inline void QOpenGLFunctions_2_0::glGetSeparableFilter(GLenum target, GLenum format, GLenum type, GLvoid *row, GLvoid *column, GLvoid *span)
{
    d_1_2_Deprecated->GetSeparableFilter(target, format, type, row, column, span);
}

inline void QOpenGLFunctions_2_0::glGetConvolutionParameteriv(GLenum target, GLenum pname, GLint *params)
{
    d_1_2_Deprecated->GetConvolutionParameteriv(target, pname, params);
}

inline void QOpenGLFunctions_2_0::glGetConvolutionParameterfv(GLenum target, GLenum pname, GLfloat *params)
{
    d_1_2_Deprecated->GetConvolutionParameterfv(target, pname, params);
}

inline void QOpenGLFunctions_2_0::glGetConvolutionFilter(GLenum target, GLenum format, GLenum type, GLvoid *image)
{
    d_1_2_Deprecated->GetConvolutionFilter(target, format, type, image);
}

inline void QOpenGLFunctions_2_0::glCopyConvolutionFilter2D(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height)
{
    d_1_2_Deprecated->CopyConvolutionFilter2D(target, internalformat, x, y, width, height);
}

inline void QOpenGLFunctions_2_0::glCopyConvolutionFilter1D(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width)
{
    d_1_2_Deprecated->CopyConvolutionFilter1D(target, internalformat, x, y, width);
}

inline void QOpenGLFunctions_2_0::glConvolutionParameteriv(GLenum target, GLenum pname, const GLint *params)
{
    d_1_2_Deprecated->ConvolutionParameteriv(target, pname, params);
}

inline void QOpenGLFunctions_2_0::glConvolutionParameteri(GLenum target, GLenum pname, GLint params)
{
    d_1_2_Deprecated->ConvolutionParameteri(target, pname, params);
}

inline void QOpenGLFunctions_2_0::glConvolutionParameterfv(GLenum target, GLenum pname, const GLfloat *params)
{
    d_1_2_Deprecated->ConvolutionParameterfv(target, pname, params);
}

inline void QOpenGLFunctions_2_0::glConvolutionParameterf(GLenum target, GLenum pname, GLfloat params)
{
    d_1_2_Deprecated->ConvolutionParameterf(target, pname, params);
}

inline void QOpenGLFunctions_2_0::glConvolutionFilter2D(GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *image)
{
    d_1_2_Deprecated->ConvolutionFilter2D(target, internalformat, width, height, format, type, image);
}

inline void QOpenGLFunctions_2_0::glConvolutionFilter1D(GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *image)
{
    d_1_2_Deprecated->ConvolutionFilter1D(target, internalformat, width, format, type, image);
}

inline void QOpenGLFunctions_2_0::glCopyColorSubTable(GLenum target, GLsizei start, GLint x, GLint y, GLsizei width)
{
    d_1_2_Deprecated->CopyColorSubTable(target, start, x, y, width);
}

inline void QOpenGLFunctions_2_0::glColorSubTable(GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const GLvoid *data)
{
    d_1_2_Deprecated->ColorSubTable(target, start, count, format, type, data);
}

inline void QOpenGLFunctions_2_0::glGetColorTableParameteriv(GLenum target, GLenum pname, GLint *params)
{
    d_1_2_Deprecated->GetColorTableParameteriv(target, pname, params);
}

inline void QOpenGLFunctions_2_0::glGetColorTableParameterfv(GLenum target, GLenum pname, GLfloat *params)
{
    d_1_2_Deprecated->GetColorTableParameterfv(target, pname, params);
}

inline void QOpenGLFunctions_2_0::glGetColorTable(GLenum target, GLenum format, GLenum type, GLvoid *table)
{
    d_1_2_Deprecated->GetColorTable(target, format, type, table);
}

inline void QOpenGLFunctions_2_0::glCopyColorTable(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width)
{
    d_1_2_Deprecated->CopyColorTable(target, internalformat, x, y, width);
}

inline void QOpenGLFunctions_2_0::glColorTableParameteriv(GLenum target, GLenum pname, const GLint *params)
{
    d_1_2_Deprecated->ColorTableParameteriv(target, pname, params);
}

inline void QOpenGLFunctions_2_0::glColorTableParameterfv(GLenum target, GLenum pname, const GLfloat *params)
{
    d_1_2_Deprecated->ColorTableParameterfv(target, pname, params);
}

inline void QOpenGLFunctions_2_0::glColorTable(GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid *table)
{
    d_1_2_Deprecated->ColorTable(target, internalformat, width, format, type, table);
}


// OpenGL 1.3 deprecated functions
inline void QOpenGLFunctions_2_0::glMultTransposeMatrixd(const GLdouble *m)
{
    d_1_3_Deprecated->MultTransposeMatrixd(m);
}

inline void QOpenGLFunctions_2_0::glMultTransposeMatrixf(const GLfloat *m)
{
    d_1_3_Deprecated->MultTransposeMatrixf(m);
}

inline void QOpenGLFunctions_2_0::glLoadTransposeMatrixd(const GLdouble *m)
{
    d_1_3_Deprecated->LoadTransposeMatrixd(m);
}

inline void QOpenGLFunctions_2_0::glLoadTransposeMatrixf(const GLfloat *m)
{
    d_1_3_Deprecated->LoadTransposeMatrixf(m);
}

inline void QOpenGLFunctions_2_0::glMultiTexCoord4sv(GLenum target, const GLshort *v)
{
    d_1_3_Deprecated->MultiTexCoord4sv(target, v);
}

inline void QOpenGLFunctions_2_0::glMultiTexCoord4s(GLenum target, GLshort s, GLshort t, GLshort r, GLshort q)
{
    d_1_3_Deprecated->MultiTexCoord4s(target, s, t, r, q);
}

inline void QOpenGLFunctions_2_0::glMultiTexCoord4iv(GLenum target, const GLint *v)
{
    d_1_3_Deprecated->MultiTexCoord4iv(target, v);
}

inline void QOpenGLFunctions_2_0::glMultiTexCoord4i(GLenum target, GLint s, GLint t, GLint r, GLint q)
{
    d_1_3_Deprecated->MultiTexCoord4i(target, s, t, r, q);
}

inline void QOpenGLFunctions_2_0::glMultiTexCoord4fv(GLenum target, const GLfloat *v)
{
    d_1_3_Deprecated->MultiTexCoord4fv(target, v);
}

inline void QOpenGLFunctions_2_0::glMultiTexCoord4f(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
    d_1_3_Deprecated->MultiTexCoord4f(target, s, t, r, q);
}

inline void QOpenGLFunctions_2_0::glMultiTexCoord4dv(GLenum target, const GLdouble *v)
{
    d_1_3_Deprecated->MultiTexCoord4dv(target, v);
}

inline void QOpenGLFunctions_2_0::glMultiTexCoord4d(GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q)
{
    d_1_3_Deprecated->MultiTexCoord4d(target, s, t, r, q);
}

inline void QOpenGLFunctions_2_0::glMultiTexCoord3sv(GLenum target, const GLshort *v)
{
    d_1_3_Deprecated->MultiTexCoord3sv(target, v);
}

inline void QOpenGLFunctions_2_0::glMultiTexCoord3s(GLenum target, GLshort s, GLshort t, GLshort r)
{
    d_1_3_Deprecated->MultiTexCoord3s(target, s, t, r);
}

inline void QOpenGLFunctions_2_0::glMultiTexCoord3iv(GLenum target, const GLint *v)
{
    d_1_3_Deprecated->MultiTexCoord3iv(target, v);
}

inline void QOpenGLFunctions_2_0::glMultiTexCoord3i(GLenum target, GLint s, GLint t, GLint r)
{
    d_1_3_Deprecated->MultiTexCoord3i(target, s, t, r);
}

inline void QOpenGLFunctions_2_0::glMultiTexCoord3fv(GLenum target, const GLfloat *v)
{
    d_1_3_Deprecated->MultiTexCoord3fv(target, v);
}

inline void QOpenGLFunctions_2_0::glMultiTexCoord3f(GLenum target, GLfloat s, GLfloat t, GLfloat r)
{
    d_1_3_Deprecated->MultiTexCoord3f(target, s, t, r);
}

inline void QOpenGLFunctions_2_0::glMultiTexCoord3dv(GLenum target, const GLdouble *v)
{
    d_1_3_Deprecated->MultiTexCoord3dv(target, v);
}

inline void QOpenGLFunctions_2_0::glMultiTexCoord3d(GLenum target, GLdouble s, GLdouble t, GLdouble r)
{
    d_1_3_Deprecated->MultiTexCoord3d(target, s, t, r);
}

inline void QOpenGLFunctions_2_0::glMultiTexCoord2sv(GLenum target, const GLshort *v)
{
    d_1_3_Deprecated->MultiTexCoord2sv(target, v);
}

inline void QOpenGLFunctions_2_0::glMultiTexCoord2s(GLenum target, GLshort s, GLshort t)
{
    d_1_3_Deprecated->MultiTexCoord2s(target, s, t);
}

inline void QOpenGLFunctions_2_0::glMultiTexCoord2iv(GLenum target, const GLint *v)
{
    d_1_3_Deprecated->MultiTexCoord2iv(target, v);
}

inline void QOpenGLFunctions_2_0::glMultiTexCoord2i(GLenum target, GLint s, GLint t)
{
    d_1_3_Deprecated->MultiTexCoord2i(target, s, t);
}

inline void QOpenGLFunctions_2_0::glMultiTexCoord2fv(GLenum target, const GLfloat *v)
{
    d_1_3_Deprecated->MultiTexCoord2fv(target, v);
}

inline void QOpenGLFunctions_2_0::glMultiTexCoord2f(GLenum target, GLfloat s, GLfloat t)
{
    d_1_3_Deprecated->MultiTexCoord2f(target, s, t);
}

inline void QOpenGLFunctions_2_0::glMultiTexCoord2dv(GLenum target, const GLdouble *v)
{
    d_1_3_Deprecated->MultiTexCoord2dv(target, v);
}

inline void QOpenGLFunctions_2_0::glMultiTexCoord2d(GLenum target, GLdouble s, GLdouble t)
{
    d_1_3_Deprecated->MultiTexCoord2d(target, s, t);
}

inline void QOpenGLFunctions_2_0::glMultiTexCoord1sv(GLenum target, const GLshort *v)
{
    d_1_3_Deprecated->MultiTexCoord1sv(target, v);
}

inline void QOpenGLFunctions_2_0::glMultiTexCoord1s(GLenum target, GLshort s)
{
    d_1_3_Deprecated->MultiTexCoord1s(target, s);
}

inline void QOpenGLFunctions_2_0::glMultiTexCoord1iv(GLenum target, const GLint *v)
{
    d_1_3_Deprecated->MultiTexCoord1iv(target, v);
}

inline void QOpenGLFunctions_2_0::glMultiTexCoord1i(GLenum target, GLint s)
{
    d_1_3_Deprecated->MultiTexCoord1i(target, s);
}

inline void QOpenGLFunctions_2_0::glMultiTexCoord1fv(GLenum target, const GLfloat *v)
{
    d_1_3_Deprecated->MultiTexCoord1fv(target, v);
}

inline void QOpenGLFunctions_2_0::glMultiTexCoord1f(GLenum target, GLfloat s)
{
    d_1_3_Deprecated->MultiTexCoord1f(target, s);
}

inline void QOpenGLFunctions_2_0::glMultiTexCoord1dv(GLenum target, const GLdouble *v)
{
    d_1_3_Deprecated->MultiTexCoord1dv(target, v);
}

inline void QOpenGLFunctions_2_0::glMultiTexCoord1d(GLenum target, GLdouble s)
{
    d_1_3_Deprecated->MultiTexCoord1d(target, s);
}

inline void QOpenGLFunctions_2_0::glClientActiveTexture(GLenum texture)
{
    d_1_3_Deprecated->ClientActiveTexture(texture);
}


// OpenGL 1.4 deprecated functions
inline void QOpenGLFunctions_2_0::glWindowPos3sv(const GLshort *v)
{
    d_1_4_Deprecated->WindowPos3sv(v);
}

inline void QOpenGLFunctions_2_0::glWindowPos3s(GLshort x, GLshort y, GLshort z)
{
    d_1_4_Deprecated->WindowPos3s(x, y, z);
}

inline void QOpenGLFunctions_2_0::glWindowPos3iv(const GLint *v)
{
    d_1_4_Deprecated->WindowPos3iv(v);
}

inline void QOpenGLFunctions_2_0::glWindowPos3i(GLint x, GLint y, GLint z)
{
    d_1_4_Deprecated->WindowPos3i(x, y, z);
}

inline void QOpenGLFunctions_2_0::glWindowPos3fv(const GLfloat *v)
{
    d_1_4_Deprecated->WindowPos3fv(v);
}

inline void QOpenGLFunctions_2_0::glWindowPos3f(GLfloat x, GLfloat y, GLfloat z)
{
    d_1_4_Deprecated->WindowPos3f(x, y, z);
}

inline void QOpenGLFunctions_2_0::glWindowPos3dv(const GLdouble *v)
{
    d_1_4_Deprecated->WindowPos3dv(v);
}

inline void QOpenGLFunctions_2_0::glWindowPos3d(GLdouble x, GLdouble y, GLdouble z)
{
    d_1_4_Deprecated->WindowPos3d(x, y, z);
}

inline void QOpenGLFunctions_2_0::glWindowPos2sv(const GLshort *v)
{
    d_1_4_Deprecated->WindowPos2sv(v);
}

inline void QOpenGLFunctions_2_0::glWindowPos2s(GLshort x, GLshort y)
{
    d_1_4_Deprecated->WindowPos2s(x, y);
}

inline void QOpenGLFunctions_2_0::glWindowPos2iv(const GLint *v)
{
    d_1_4_Deprecated->WindowPos2iv(v);
}

inline void QOpenGLFunctions_2_0::glWindowPos2i(GLint x, GLint y)
{
    d_1_4_Deprecated->WindowPos2i(x, y);
}

inline void QOpenGLFunctions_2_0::glWindowPos2fv(const GLfloat *v)
{
    d_1_4_Deprecated->WindowPos2fv(v);
}

inline void QOpenGLFunctions_2_0::glWindowPos2f(GLfloat x, GLfloat y)
{
    d_1_4_Deprecated->WindowPos2f(x, y);
}

inline void QOpenGLFunctions_2_0::glWindowPos2dv(const GLdouble *v)
{
    d_1_4_Deprecated->WindowPos2dv(v);
}

inline void QOpenGLFunctions_2_0::glWindowPos2d(GLdouble x, GLdouble y)
{
    d_1_4_Deprecated->WindowPos2d(x, y);
}

inline void QOpenGLFunctions_2_0::glSecondaryColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
    d_1_4_Deprecated->SecondaryColorPointer(size, type, stride, pointer);
}

inline void QOpenGLFunctions_2_0::glSecondaryColor3usv(const GLushort *v)
{
    d_1_4_Deprecated->SecondaryColor3usv(v);
}

inline void QOpenGLFunctions_2_0::glSecondaryColor3us(GLushort red, GLushort green, GLushort blue)
{
    d_1_4_Deprecated->SecondaryColor3us(red, green, blue);
}

inline void QOpenGLFunctions_2_0::glSecondaryColor3uiv(const GLuint *v)
{
    d_1_4_Deprecated->SecondaryColor3uiv(v);
}

inline void QOpenGLFunctions_2_0::glSecondaryColor3ui(GLuint red, GLuint green, GLuint blue)
{
    d_1_4_Deprecated->SecondaryColor3ui(red, green, blue);
}

inline void QOpenGLFunctions_2_0::glSecondaryColor3ubv(const GLubyte *v)
{
    d_1_4_Deprecated->SecondaryColor3ubv(v);
}

inline void QOpenGLFunctions_2_0::glSecondaryColor3ub(GLubyte red, GLubyte green, GLubyte blue)
{
    d_1_4_Deprecated->SecondaryColor3ub(red, green, blue);
}

inline void QOpenGLFunctions_2_0::glSecondaryColor3sv(const GLshort *v)
{
    d_1_4_Deprecated->SecondaryColor3sv(v);
}

inline void QOpenGLFunctions_2_0::glSecondaryColor3s(GLshort red, GLshort green, GLshort blue)
{
    d_1_4_Deprecated->SecondaryColor3s(red, green, blue);
}

inline void QOpenGLFunctions_2_0::glSecondaryColor3iv(const GLint *v)
{
    d_1_4_Deprecated->SecondaryColor3iv(v);
}

inline void QOpenGLFunctions_2_0::glSecondaryColor3i(GLint red, GLint green, GLint blue)
{
    d_1_4_Deprecated->SecondaryColor3i(red, green, blue);
}

inline void QOpenGLFunctions_2_0::glSecondaryColor3fv(const GLfloat *v)
{
    d_1_4_Deprecated->SecondaryColor3fv(v);
}

inline void QOpenGLFunctions_2_0::glSecondaryColor3f(GLfloat red, GLfloat green, GLfloat blue)
{
    d_1_4_Deprecated->SecondaryColor3f(red, green, blue);
}

inline void QOpenGLFunctions_2_0::glSecondaryColor3dv(const GLdouble *v)
{
    d_1_4_Deprecated->SecondaryColor3dv(v);
}

inline void QOpenGLFunctions_2_0::glSecondaryColor3d(GLdouble red, GLdouble green, GLdouble blue)
{
    d_1_4_Deprecated->SecondaryColor3d(red, green, blue);
}

inline void QOpenGLFunctions_2_0::glSecondaryColor3bv(const GLbyte *v)
{
    d_1_4_Deprecated->SecondaryColor3bv(v);
}

inline void QOpenGLFunctions_2_0::glSecondaryColor3b(GLbyte red, GLbyte green, GLbyte blue)
{
    d_1_4_Deprecated->SecondaryColor3b(red, green, blue);
}

inline void QOpenGLFunctions_2_0::glFogCoordPointer(GLenum type, GLsizei stride, const GLvoid *pointer)
{
    d_1_4_Deprecated->FogCoordPointer(type, stride, pointer);
}

inline void QOpenGLFunctions_2_0::glFogCoorddv(const GLdouble *coord)
{
    d_1_4_Deprecated->FogCoorddv(coord);
}

inline void QOpenGLFunctions_2_0::glFogCoordd(GLdouble coord)
{
    d_1_4_Deprecated->FogCoordd(coord);
}

inline void QOpenGLFunctions_2_0::glFogCoordfv(const GLfloat *coord)
{
    d_1_4_Deprecated->FogCoordfv(coord);
}

inline void QOpenGLFunctions_2_0::glFogCoordf(GLfloat coord)
{
    d_1_4_Deprecated->FogCoordf(coord);
}


// OpenGL 1.5 deprecated functions

// OpenGL 2.0 deprecated functions
inline void QOpenGLFunctions_2_0::glVertexAttrib4usv(GLuint index, const GLushort *v)
{
    d_2_0_Core->VertexAttrib4usv(index, v);
}

inline void QOpenGLFunctions_2_0::glVertexAttrib4uiv(GLuint index, const GLuint *v)
{
    d_2_0_Core->VertexAttrib4uiv(index, v);
}

inline void QOpenGLFunctions_2_0::glVertexAttrib4ubv(GLuint index, const GLubyte *v)
{
    d_2_0_Core->VertexAttrib4ubv(index, v);
}

inline void QOpenGLFunctions_2_0::glVertexAttrib4sv(GLuint index, const GLshort *v)
{
    d_2_0_Core->VertexAttrib4sv(index, v);
}

inline void QOpenGLFunctions_2_0::glVertexAttrib4s(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w)
{
    d_2_0_Core->VertexAttrib4s(index, x, y, z, w);
}

inline void QOpenGLFunctions_2_0::glVertexAttrib4iv(GLuint index, const GLint *v)
{
    d_2_0_Core->VertexAttrib4iv(index, v);
}

inline void QOpenGLFunctions_2_0::glVertexAttrib4fv(GLuint index, const GLfloat *v)
{
    d_2_0_Core->VertexAttrib4fv(index, v);
}

inline void QOpenGLFunctions_2_0::glVertexAttrib4f(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    d_2_0_Core->VertexAttrib4f(index, x, y, z, w);
}

inline void QOpenGLFunctions_2_0::glVertexAttrib4dv(GLuint index, const GLdouble *v)
{
    d_2_0_Core->VertexAttrib4dv(index, v);
}

inline void QOpenGLFunctions_2_0::glVertexAttrib4d(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    d_2_0_Core->VertexAttrib4d(index, x, y, z, w);
}

inline void QOpenGLFunctions_2_0::glVertexAttrib4bv(GLuint index, const GLbyte *v)
{
    d_2_0_Core->VertexAttrib4bv(index, v);
}

inline void QOpenGLFunctions_2_0::glVertexAttrib4Nusv(GLuint index, const GLushort *v)
{
    d_2_0_Core->VertexAttrib4Nusv(index, v);
}

inline void QOpenGLFunctions_2_0::glVertexAttrib4Nuiv(GLuint index, const GLuint *v)
{
    d_2_0_Core->VertexAttrib4Nuiv(index, v);
}

inline void QOpenGLFunctions_2_0::glVertexAttrib4Nubv(GLuint index, const GLubyte *v)
{
    d_2_0_Core->VertexAttrib4Nubv(index, v);
}

inline void QOpenGLFunctions_2_0::glVertexAttrib4Nub(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w)
{
    d_2_0_Core->VertexAttrib4Nub(index, x, y, z, w);
}

inline void QOpenGLFunctions_2_0::glVertexAttrib4Nsv(GLuint index, const GLshort *v)
{
    d_2_0_Core->VertexAttrib4Nsv(index, v);
}

inline void QOpenGLFunctions_2_0::glVertexAttrib4Niv(GLuint index, const GLint *v)
{
    d_2_0_Core->VertexAttrib4Niv(index, v);
}

inline void QOpenGLFunctions_2_0::glVertexAttrib4Nbv(GLuint index, const GLbyte *v)
{
    d_2_0_Core->VertexAttrib4Nbv(index, v);
}

inline void QOpenGLFunctions_2_0::glVertexAttrib3sv(GLuint index, const GLshort *v)
{
    d_2_0_Core->VertexAttrib3sv(index, v);
}

inline void QOpenGLFunctions_2_0::glVertexAttrib3s(GLuint index, GLshort x, GLshort y, GLshort z)
{
    d_2_0_Core->VertexAttrib3s(index, x, y, z);
}

inline void QOpenGLFunctions_2_0::glVertexAttrib3fv(GLuint index, const GLfloat *v)
{
    d_2_0_Core->VertexAttrib3fv(index, v);
}

inline void QOpenGLFunctions_2_0::glVertexAttrib3f(GLuint index, GLfloat x, GLfloat y, GLfloat z)
{
    d_2_0_Core->VertexAttrib3f(index, x, y, z);
}

inline void QOpenGLFunctions_2_0::glVertexAttrib3dv(GLuint index, const GLdouble *v)
{
    d_2_0_Core->VertexAttrib3dv(index, v);
}

inline void QOpenGLFunctions_2_0::glVertexAttrib3d(GLuint index, GLdouble x, GLdouble y, GLdouble z)
{
    d_2_0_Core->VertexAttrib3d(index, x, y, z);
}

inline void QOpenGLFunctions_2_0::glVertexAttrib2sv(GLuint index, const GLshort *v)
{
    d_2_0_Core->VertexAttrib2sv(index, v);
}

inline void QOpenGLFunctions_2_0::glVertexAttrib2s(GLuint index, GLshort x, GLshort y)
{
    d_2_0_Core->VertexAttrib2s(index, x, y);
}

inline void QOpenGLFunctions_2_0::glVertexAttrib2fv(GLuint index, const GLfloat *v)
{
    d_2_0_Core->VertexAttrib2fv(index, v);
}

inline void QOpenGLFunctions_2_0::glVertexAttrib2f(GLuint index, GLfloat x, GLfloat y)
{
    d_2_0_Core->VertexAttrib2f(index, x, y);
}

inline void QOpenGLFunctions_2_0::glVertexAttrib2dv(GLuint index, const GLdouble *v)
{
    d_2_0_Core->VertexAttrib2dv(index, v);
}

inline void QOpenGLFunctions_2_0::glVertexAttrib2d(GLuint index, GLdouble x, GLdouble y)
{
    d_2_0_Core->VertexAttrib2d(index, x, y);
}

inline void QOpenGLFunctions_2_0::glVertexAttrib1sv(GLuint index, const GLshort *v)
{
    d_2_0_Core->VertexAttrib1sv(index, v);
}

inline void QOpenGLFunctions_2_0::glVertexAttrib1s(GLuint index, GLshort x)
{
    d_2_0_Core->VertexAttrib1s(index, x);
}

inline void QOpenGLFunctions_2_0::glVertexAttrib1fv(GLuint index, const GLfloat *v)
{
    d_2_0_Core->VertexAttrib1fv(index, v);
}

inline void QOpenGLFunctions_2_0::glVertexAttrib1f(GLuint index, GLfloat x)
{
    d_2_0_Core->VertexAttrib1f(index, x);
}

inline void QOpenGLFunctions_2_0::glVertexAttrib1dv(GLuint index, const GLdouble *v)
{
    d_2_0_Core->VertexAttrib1dv(index, v);
}

inline void QOpenGLFunctions_2_0::glVertexAttrib1d(GLuint index, GLdouble x)
{
    d_2_0_Core->VertexAttrib1d(index, x);
}



QT_END_NAMESPACE

#endif // QT_NO_OPENGL && !QT_OPENGL_ES_2

#endif
