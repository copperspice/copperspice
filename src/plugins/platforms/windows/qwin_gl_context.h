/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
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

#ifndef QWINDOWSGLCONTEXT_H
#define QWINDOWSGLCONTEXT_H

#include <array.h>
#include <qwin_additional.h>
#include <qwin_opengl_context.h>
#include <qopenglcontext.h>

class QDebug;

enum QWindowsGLFormatFlags {
   QWindowsGLDirectRendering = 0x1,
   QWindowsGLOverlay = 0x2,
   QWindowsGLRenderToPixmap = 0x4,
   QWindowsGLAccumBuffer = 0x8
};

// Additional format information for Windows.
struct QWindowsOpenGLAdditionalFormat {
   QWindowsOpenGLAdditionalFormat(unsigned formatFlagsIn = 0, unsigned pixmapDepthIn = 0)
      : formatFlags(formatFlagsIn), pixmapDepth(pixmapDepthIn)
   { }

   unsigned formatFlags;    // QWindowsGLFormatFlags.
   unsigned pixmapDepth;    // for QWindowsGLRenderToPixmap
};

// Per-window data for active OpenGL contexts.
struct QOpenGLContextData {
   QOpenGLContextData(HGLRC r, HWND h, HDC d)
      : renderingContext(r), hwnd(h), hdc(d)
   {
   }

   QOpenGLContextData()
      : renderingContext(nullptr), hwnd(nullptr), hdc(nullptr)
   {
   }

   HGLRC renderingContext;
   HWND hwnd;
   HDC hdc;
};

class QOpenGLStaticContext;

struct QWindowsOpenGLContextFormat {
   QWindowsOpenGLContextFormat();

   static QWindowsOpenGLContextFormat current();
   void apply(QSurfaceFormat *format) const;

   QSurfaceFormat::OpenGLContextProfile profile;
   QSurfaceFormat::FormatOptions options;

   int m_version;
};

QDebug operator<<(QDebug debug, const PIXELFORMATDESCRIPTOR &);
QDebug operator<<(QDebug debug, const QWindowsOpenGLContextFormat &);
QDebug operator<<(QDebug debug, const QOpenGLStaticContext &s);

struct QWindowsOpengl32DLL {
   bool init(bool softwareRendering);
   void *moduleHandle() const {
      return m_lib;
   }
   bool moduleIsNotOpengl32() const {
      return m_nonOpengl32;
   }

   // Wrappers. Always use these instead of SwapBuffers/wglSwapBuffers/etc.
   BOOL swapBuffers(HDC dc);
   BOOL setPixelFormat(HDC dc, int pf, const PIXELFORMATDESCRIPTOR *pfd);

   // WGL
   HGLRC (WINAPI *wglCreateContext)(HDC dc);
   BOOL (WINAPI *wglDeleteContext)(HGLRC context);
   HGLRC (WINAPI *wglGetCurrentContext)();
   HDC (WINAPI *wglGetCurrentDC)();
   PROC (WINAPI *wglGetProcAddress)(LPCSTR name);
   BOOL (WINAPI *wglMakeCurrent)(HDC dc, HGLRC context);
   BOOL (WINAPI *wglShareLists)(HGLRC context1, HGLRC context2);

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

   // GL only
   void (APIENTRY *glClearDepth)(GLdouble depth);
   void (APIENTRY *glDepthRange)(GLdouble zNear, GLdouble zFar);

 private:
   void *resolve(const char *name);
   HMODULE m_lib;
   bool m_nonOpengl32;

   // For Mesa llvmpipe shipped with a name other than opengl32.dll
   BOOL (WINAPI *wglSwapBuffers)(HDC dc);
   BOOL (WINAPI *wglSetPixelFormat)(HDC dc, int pf, const PIXELFORMATDESCRIPTOR *pfd);
};

class QOpenGLStaticContext : public QWindowsStaticOpenGLContext
{
 public:
   enum Extensions {
      SampleBuffers = 0x1
   };

   using WglGetPixelFormatAttribIVARB = bool  (APIENTRY *) (HDC hdc, int iPixelFormat, int iLayerPlane, uint nAttributes,
         const int *piAttributes, int *piValues);

   using WglChoosePixelFormatARB      = bool  (APIENTRY *)(HDC hdc, const int *piAttribList, const float *pfAttribFList,
         uint nMaxFormats, int *piFormats, UINT *nNumFormats);

   using WglCreateContextAttribsARB   = HGLRC (APIENTRY *)(HDC, HGLRC, const int *);
   using WglSwapInternalExt           = BOOL  (APIENTRY *)(int interval);
   using WglGetSwapInternalExt        = int   (APIENTRY *)(void);

   QOpenGLStaticContext(const QOpenGLStaticContext &) = delete;
   QOpenGLStaticContext &operator=(const QOpenGLStaticContext &) = delete;

   bool hasExtensions() const {
      return wglGetPixelFormatAttribIVARB && wglChoosePixelFormatARB && wglCreateContextAttribsARB;
   }

   static QOpenGLStaticContext *create(bool softwareRendering = false);
   static QByteArray getGlString(unsigned int which);

   QWindowsOpenGLContext *createContext(QOpenGLContext *context) override;

   void *moduleHandle() const override {
      return opengl32.moduleHandle();
   }

   QOpenGLContext::OpenGLModuleType moduleType() const override {
      return QOpenGLContext::LibGL;
   }

   // For a regular opengl32.dll report the ThreadedOpenGL capability.
   // For others, which are likely to be software-only, don't.
   bool supportsThreadedOpenGL() const override {
      return ! opengl32.moduleIsNotOpengl32();
   }

   const QByteArray vendor;
   const QByteArray renderer;
   const QByteArray extensionNames;
   unsigned extensions;
   const QWindowsOpenGLContextFormat m_defaultFormat;

   WglGetPixelFormatAttribIVARB wglGetPixelFormatAttribIVARB;
   WglChoosePixelFormatARB wglChoosePixelFormatARB;
   WglCreateContextAttribsARB wglCreateContextAttribsARB;
   WglSwapInternalExt wglSwapInternalExt;
   WglGetSwapInternalExt wglGetSwapInternalExt;

   static QWindowsOpengl32DLL opengl32;

 private:
   QOpenGLStaticContext();
};

class QWindowsGLContext : public QWindowsOpenGLContext
{
 public:
   explicit QWindowsGLContext(QOpenGLStaticContext *staticContext, QOpenGLContext *context);
   ~QWindowsGLContext();

   bool isSharing() const override {
      return m_context->shareHandle();
   }

   bool isValid() const override {
      return m_renderingContext && !m_lost;
   }

   QSurfaceFormat format() const override {
      return m_obtainedFormat;
   }

   void swapBuffers(QPlatformSurface *surface) override;

   bool makeCurrent(QPlatformSurface *surface) override;
   void doneCurrent() override;

   typedef void (*GL_Proc) ();

   FP_Void getProcAddress(const QByteArray &procName) override;

   HGLRC renderingContext() const {
      return m_renderingContext;
   }

   void *nativeContext() const override {
      return m_renderingContext;
   }

 private:
   inline void releaseDCs();
   bool updateObtainedParams(HDC hdc, int *obtainedSwapInterval = nullptr);

   QOpenGLStaticContext *m_staticContext;
   QOpenGLContext *m_context;
   QSurfaceFormat m_obtainedFormat;
   HGLRC m_renderingContext;

   Array<QOpenGLContextData> m_windowContexts;

   PIXELFORMATDESCRIPTOR m_obtainedPixelFormatDescriptor;
   int m_pixelFormat;
   bool m_extensionsUsed;
   int m_swapInterval;
   bool m_ownsContext;
   GLenum (APIENTRY *m_getGraphicsResetStatus)();
   bool m_lost;
};

#endif
