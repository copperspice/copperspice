/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
*
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

#ifndef QWINDOWSEGLCONTEXT_H
#define QWINDOWSEGLCONTEXT_H

#include "qwindowsopenglcontext.h"
#include "qwindowsopengltester.h"

#include <EGL/egl.h>

struct QWindowsLibEGL {
   bool init();

   EGLint (EGLAPIENTRY *eglGetError)(void);
   EGLDisplay (EGLAPIENTRY *eglGetDisplay)(EGLNativeDisplayType display_id);
   EGLBoolean (EGLAPIENTRY *eglInitialize)(EGLDisplay dpy, EGLint *major, EGLint *minor);
   EGLBoolean (EGLAPIENTRY *eglTerminate)(EGLDisplay dpy);
   EGLBoolean (EGLAPIENTRY *eglChooseConfig)(EGLDisplay dpy, const EGLint *attrib_list,
      EGLConfig *configs, EGLint config_size,
      EGLint *num_config);
   EGLBoolean (EGLAPIENTRY *eglGetConfigAttrib)(EGLDisplay dpy, EGLConfig config,
      EGLint attribute, EGLint *value);
   EGLSurface (EGLAPIENTRY *eglCreateWindowSurface)(EGLDisplay dpy, EGLConfig config,
      EGLNativeWindowType win,
      const EGLint *attrib_list);
   EGLSurface (EGLAPIENTRY *eglCreatePbufferSurface)(EGLDisplay dpy, EGLConfig config,
      const EGLint *attrib_list);
   EGLBoolean (EGLAPIENTRY *eglDestroySurface)(EGLDisplay dpy, EGLSurface surface);
   EGLBoolean (EGLAPIENTRY *eglBindAPI)(EGLenum api);
   EGLBoolean (EGLAPIENTRY *eglSwapInterval)(EGLDisplay dpy, EGLint interval);
   EGLContext (EGLAPIENTRY *eglCreateContext)(EGLDisplay dpy, EGLConfig config,
      EGLContext share_context,
      const EGLint *attrib_list);
   EGLBoolean (EGLAPIENTRY *eglDestroyContext)(EGLDisplay dpy, EGLContext ctx);
   EGLBoolean (EGLAPIENTRY *eglMakeCurrent)(EGLDisplay dpy, EGLSurface draw,
      EGLSurface read, EGLContext ctx);
   EGLContext (EGLAPIENTRY *eglGetCurrentContext)(void);
   EGLSurface (EGLAPIENTRY *eglGetCurrentSurface)(EGLint readdraw);
   EGLDisplay (EGLAPIENTRY *eglGetCurrentDisplay)(void);
   EGLBoolean (EGLAPIENTRY *eglSwapBuffers)(EGLDisplay dpy, EGLSurface surface);
   QFunctionPointer (EGLAPIENTRY *eglGetProcAddress)(const char *procname);

   EGLDisplay (EGLAPIENTRY *eglGetPlatformDisplayEXT)(EGLenum platform, void *native_display, const EGLint *attrib_list);

 private:
#if ! defined(QT_STATIC) || defined(QT_OPENGL_DYNAMIC)
   void *resolve(const char *name);
   HMODULE m_lib;
#endif
};

struct QWindowsLibGLESv2 {
   bool init();

#if !defined(QT_STATIC) || defined(QT_OPENGL_DYNAMIC)
   void *moduleHandle() const {
      return m_lib;
   }
#else
   void *moduleHandle() const {
      return nullptr;
   }
#endif

   // GL1+GLES2 common
   void (APIENTRY *glBindTexture)(GLenum target, GLuint texture);
   void (APIENTRY *glBlendFunc)(GLenum sfactor, GLenum dfactor);
   void (APIENTRY *glClear)(GLbitfield mask);
   void (APIENTRY *glClearColor)(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
   void (APIENTRY *glClearStencil)(GLint s);
   void (APIENTRY *glColorMask)(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
   void (APIENTRY *glCopyTexImage2D)(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height,
      GLint border);
   void (APIENTRY *glCopyTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width,
      GLsizei height);
   void (APIENTRY *glCullFace)(GLenum mode);
   void (APIENTRY *glDeleteTextures)(GLsizei n, const GLuint *textures);
   void (APIENTRY *glDepthFunc)(GLenum func);
   void (APIENTRY *glDepthMask)(GLboolean flag);
   void (APIENTRY *glDisable)(GLenum cap);
   void (APIENTRY *glDrawArrays)(GLenum mode, GLint first, GLsizei count);
   void (APIENTRY *glDrawElements)(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices);
   void (APIENTRY *glEnable)(GLenum cap);
   void (APIENTRY *glFinish)();
   void (APIENTRY *glFlush)();
   void (APIENTRY *glFrontFace)(GLenum mode);
   void (APIENTRY *glGenTextures)(GLsizei n, GLuint *textures);
   void (APIENTRY *glGetBooleanv)(GLenum pname, GLboolean *params);
   GLenum (APIENTRY *glGetError)();
   void (APIENTRY *glGetFloatv)(GLenum pname, GLfloat *params);
   void (APIENTRY *glGetIntegerv)(GLenum pname, GLint *params);
   const GLubyte *(APIENTRY *glGetString)(GLenum name);
   void (APIENTRY *glGetTexParameterfv)(GLenum target, GLenum pname, GLfloat *params);
   void (APIENTRY *glGetTexParameteriv)(GLenum target, GLenum pname, GLint *params);
   void (APIENTRY *glHint)(GLenum target, GLenum mode);
   GLboolean (APIENTRY *glIsEnabled)(GLenum cap);
   GLboolean (APIENTRY *glIsTexture)(GLuint texture);
   void (APIENTRY *glLineWidth)(GLfloat width);
   void (APIENTRY *glPixelStorei)(GLenum pname, GLint param);
   void (APIENTRY *glPolygonOffset)(GLfloat factor, GLfloat units);
   void (APIENTRY *glReadPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels);
   void (APIENTRY *glScissor)(GLint x, GLint y, GLsizei width, GLsizei height);
   void (APIENTRY *glStencilFunc)(GLenum func, GLint ref, GLuint mask);
   void (APIENTRY *glStencilMask)(GLuint mask);
   void (APIENTRY *glStencilOp)(GLenum fail, GLenum zfail, GLenum zpass);
   void (APIENTRY *glTexImage2D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border,
      GLenum format, GLenum type, const GLvoid *pixels);
   void (APIENTRY *glTexParameterf)(GLenum target, GLenum pname, GLfloat param);
   void (APIENTRY *glTexParameterfv)(GLenum target, GLenum pname, const GLfloat *params);
   void (APIENTRY *glTexParameteri)(GLenum target, GLenum pname, GLint param);
   void (APIENTRY *glTexParameteriv)(GLenum target, GLenum pname, const GLint *params);
   void (APIENTRY *glTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height,
      GLenum format, GLenum type, const GLvoid *pixels);
   void (APIENTRY *glViewport)(GLint x, GLint y, GLsizei width, GLsizei height);

   // GLES2
   void (APIENTRY *glActiveTexture)(GLenum texture);
   void (APIENTRY *glAttachShader)(GLuint program, GLuint shader);
   void (APIENTRY *glBindAttribLocation)(GLuint program, GLuint index, const char *name);
   void (APIENTRY *glBindBuffer)(GLenum target, GLuint buffer);
   void (APIENTRY *glBindFramebuffer)(GLenum target, GLuint framebuffer);
   void (APIENTRY *glBindRenderbuffer)(GLenum target, GLuint renderbuffer);
   void (APIENTRY *glBlendColor)(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
   void (APIENTRY *glBlendEquation)(GLenum mode);
   void (APIENTRY *glBlendEquationSeparate)(GLenum modeRGB, GLenum modeAlpha);
   void (APIENTRY *glBlendFuncSeparate)(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);
   void (APIENTRY *glBufferData)(GLenum target, qopengl_GLsizeiptr size, const void *data, GLenum usage);
   void (APIENTRY *glBufferSubData)(GLenum target, qopengl_GLintptr offset, qopengl_GLsizeiptr size, const void *data);
   GLenum (APIENTRY *glCheckFramebufferStatus)(GLenum target);
   void (APIENTRY *glCompileShader)(GLuint shader);
   void (APIENTRY *glCompressedTexImage2D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border,
      GLsizei imageSize, const void *data);
   void (APIENTRY *glCompressedTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height,
      GLenum format, GLsizei imageSize, const void *data);
   GLuint (APIENTRY *glCreateProgram)();
   GLuint (APIENTRY *glCreateShader)(GLenum type);
   void (APIENTRY *glDeleteBuffers)(GLsizei n, const GLuint *buffers);
   void (APIENTRY *glDeleteFramebuffers)(GLsizei n, const GLuint *framebuffers);
   void (APIENTRY *glDeleteProgram)(GLuint program);
   void (APIENTRY *glDeleteRenderbuffers)(GLsizei n, const GLuint *renderbuffers);
   void (APIENTRY *glDeleteShader)(GLuint shader);
   void (APIENTRY *glDetachShader)(GLuint program, GLuint shader);
   void (APIENTRY *glDisableVertexAttribArray)(GLuint index);
   void (APIENTRY *glEnableVertexAttribArray)(GLuint index);
   void (APIENTRY *glFramebufferRenderbuffer)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
   void (APIENTRY *glFramebufferTexture2D)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
   void (APIENTRY *glGenBuffers)(GLsizei n, GLuint *buffers);
   void (APIENTRY *glGenerateMipmap)(GLenum target);
   void (APIENTRY *glGenFramebuffers)(GLsizei n, GLuint *framebuffers);
   void (APIENTRY *glGenRenderbuffers)(GLsizei n, GLuint *renderbuffers);
   void (APIENTRY *glGetActiveAttrib)(GLuint program, GLuint index, GLsizei bufsize, GLsizei *length, GLint *size, GLenum *type,
      char *name);
   void (APIENTRY *glGetActiveUniform)(GLuint program, GLuint index, GLsizei bufsize, GLsizei *length, GLint *size, GLenum *type,
      char *name);
   void (APIENTRY *glGetAttachedShaders)(GLuint program, GLsizei maxcount, GLsizei *count, GLuint *shaders);
   GLint (APIENTRY *glGetAttribLocation)(GLuint program, const char *name);
   void (APIENTRY *glGetBufferParameteriv)(GLenum target, GLenum pname, GLint *params);
   void (APIENTRY *glGetFramebufferAttachmentParameteriv)(GLenum target, GLenum attachment, GLenum pname, GLint *params);
   void (APIENTRY *glGetProgramiv)(GLuint program, GLenum pname, GLint *params);
   void (APIENTRY *glGetProgramInfoLog)(GLuint program, GLsizei bufsize, GLsizei *length, char *infolog);
   void (APIENTRY *glGetRenderbufferParameteriv)(GLenum target, GLenum pname, GLint *params);
   void (APIENTRY *glGetShaderiv)(GLuint shader, GLenum pname, GLint *params);
   void (APIENTRY *glGetShaderInfoLog)(GLuint shader, GLsizei bufsize, GLsizei *length, char *infolog);
   void (APIENTRY *glGetShaderPrecisionFormat)(GLenum shadertype, GLenum precisiontype, GLint *range, GLint *precision);
   void (APIENTRY *glGetShaderSource)(GLuint shader, GLsizei bufsize, GLsizei *length, char *source);
   void (APIENTRY *glGetUniformfv)(GLuint program, GLint location, GLfloat *params);
   void (APIENTRY *glGetUniformiv)(GLuint program, GLint location, GLint *params);
   GLint (APIENTRY *glGetUniformLocation)(GLuint program, const char *name);
   void (APIENTRY *glGetVertexAttribfv)(GLuint index, GLenum pname, GLfloat *params);
   void (APIENTRY *glGetVertexAttribiv)(GLuint index, GLenum pname, GLint *params);
   void (APIENTRY *glGetVertexAttribPointerv)(GLuint index, GLenum pname, void **pointer);
   GLboolean (APIENTRY *glIsBuffer)(GLuint buffer);
   GLboolean (APIENTRY *glIsFramebuffer)(GLuint framebuffer);
   GLboolean (APIENTRY *glIsProgram)(GLuint program);
   GLboolean (APIENTRY *glIsRenderbuffer)(GLuint renderbuffer);
   GLboolean (APIENTRY *glIsShader)(GLuint shader);
   void (APIENTRY *glLinkProgram)(GLuint program);
   void (APIENTRY *glReleaseShaderCompiler)();
   void (APIENTRY *glRenderbufferStorage)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
   void (APIENTRY *glSampleCoverage)(GLclampf value, GLboolean invert);
   void (APIENTRY *glShaderBinary)(GLint n, const GLuint *shaders, GLenum binaryformat, const void *binary, GLint length);
   void (APIENTRY *glShaderSource)(GLuint shader, GLsizei count, const char **string, const GLint *length);
   void (APIENTRY *glStencilFuncSeparate)(GLenum face, GLenum func, GLint ref, GLuint mask);
   void (APIENTRY *glStencilMaskSeparate)(GLenum face, GLuint mask);
   void (APIENTRY *glStencilOpSeparate)(GLenum face, GLenum fail, GLenum zfail, GLenum zpass);
   void (APIENTRY *glUniform1f)(GLint location, GLfloat x);
   void (APIENTRY *glUniform1fv)(GLint location, GLsizei count, const GLfloat *v);
   void (APIENTRY *glUniform1i)(GLint location, GLint x);
   void (APIENTRY *glUniform1iv)(GLint location, GLsizei count, const GLint *v);
   void (APIENTRY *glUniform2f)(GLint location, GLfloat x, GLfloat y);
   void (APIENTRY *glUniform2fv)(GLint location, GLsizei count, const GLfloat *v);
   void (APIENTRY *glUniform2i)(GLint location, GLint x, GLint y);
   void (APIENTRY *glUniform2iv)(GLint location, GLsizei count, const GLint *v);
   void (APIENTRY *glUniform3f)(GLint location, GLfloat x, GLfloat y, GLfloat z);
   void (APIENTRY *glUniform3fv)(GLint location, GLsizei count, const GLfloat *v);
   void (APIENTRY *glUniform3i)(GLint location, GLint x, GLint y, GLint z);
   void (APIENTRY *glUniform3iv)(GLint location, GLsizei count, const GLint *v);
   void (APIENTRY *glUniform4f)(GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
   void (APIENTRY *glUniform4fv)(GLint location, GLsizei count, const GLfloat *v);
   void (APIENTRY *glUniform4i)(GLint location, GLint x, GLint y, GLint z, GLint w);
   void (APIENTRY *glUniform4iv)(GLint location, GLsizei count, const GLint *v);
   void (APIENTRY *glUniformMatrix2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
   void (APIENTRY *glUniformMatrix3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
   void (APIENTRY *glUniformMatrix4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
   void (APIENTRY *glUseProgram)(GLuint program);
   void (APIENTRY *glValidateProgram)(GLuint program);
   void (APIENTRY *glVertexAttrib1f)(GLuint indx, GLfloat x);
   void (APIENTRY *glVertexAttrib1fv)(GLuint indx, const GLfloat *values);
   void (APIENTRY *glVertexAttrib2f)(GLuint indx, GLfloat x, GLfloat y);
   void (APIENTRY *glVertexAttrib2fv)(GLuint indx, const GLfloat *values);
   void (APIENTRY *glVertexAttrib3f)(GLuint indx, GLfloat x, GLfloat y, GLfloat z);
   void (APIENTRY *glVertexAttrib3fv)(GLuint indx, const GLfloat *values);
   void (APIENTRY *glVertexAttrib4f)(GLuint indx, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
   void (APIENTRY *glVertexAttrib4fv)(GLuint indx, const GLfloat *values);
   void (APIENTRY *glVertexAttribPointer)(GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *ptr);

   // ES only
   void (APIENTRY *glClearDepthf)(GLclampf depth);
   void (APIENTRY *glDepthRangef)(GLclampf nearVal, GLclampf farVal);

 private:
#if !defined(QT_STATIC) || defined(QT_OPENGL_DYNAMIC)
   void *resolve(const char *name);
   HMODULE m_lib;
#endif
};

class QWindowsEGLStaticContext : public QWindowsStaticOpenGLContext
{
   Q_DISABLE_COPY(QWindowsEGLStaticContext)

 public:
   static QWindowsEGLStaticContext *create(QWindowsOpenGLTester::Renderers preferredType);
   ~QWindowsEGLStaticContext();

   EGLDisplay display() const {
      return m_display;
   }

   QWindowsOpenGLContext *createContext(QOpenGLContext *context) override;
   void *moduleHandle() const override {
      return libGLESv2.moduleHandle();
   }
   QOpenGLContext::OpenGLModuleType moduleType() const override {
      return QOpenGLContext::LibGLES;
   }

   void *createWindowSurface(void *nativeWindow, void *nativeConfig, int *err) override;
   void destroyWindowSurface(void *nativeSurface) override;

   QSurfaceFormat formatFromConfig(EGLDisplay display, EGLConfig config, const QSurfaceFormat &referenceFormat);

   static QWindowsLibEGL libEGL;
   static QWindowsLibGLESv2 libGLESv2;

 private:
   explicit QWindowsEGLStaticContext(EGLDisplay display);
   static bool initializeAngle(QWindowsOpenGLTester::Renderers preferredType, HDC dc,
      EGLDisplay *display, EGLint *major, EGLint *minor);

   const EGLDisplay m_display;
};

class QWindowsEGLContext : public QWindowsOpenGLContext
{
 public:
   QWindowsEGLContext(QWindowsEGLStaticContext *staticContext, const QSurfaceFormat &format,
      QPlatformOpenGLContext *share);

   ~QWindowsEGLContext();

   bool makeCurrent(QPlatformSurface *surface) override;
   void doneCurrent() override;
   void swapBuffers(QPlatformSurface *surface) override;
   QFunctionPointer getProcAddress(const QByteArray &procName) override;

   QSurfaceFormat format() const override {
      return m_format;
   }
   bool isSharing() const override {
      return m_shareContext != EGL_NO_CONTEXT;
   }
   bool isValid() const override {
      return m_eglContext != EGL_NO_CONTEXT;
   }

   void *nativeContext() const override {
      return m_eglContext;
   }
   void *nativeDisplay() const override {
      return m_eglDisplay;
   }
   void *nativeConfig() const override {
      return m_eglConfig;
   }

 private:
   EGLConfig chooseConfig(const QSurfaceFormat &format);

   QWindowsEGLStaticContext *m_staticContext;
   EGLContext m_eglContext;
   EGLContext m_shareContext;
   EGLDisplay m_eglDisplay;
   EGLConfig m_eglConfig;
   QSurfaceFormat m_format;
   EGLenum m_api;
   int m_swapInterval;
};

#endif
