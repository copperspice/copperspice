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

#include <qcocoaglcontext.h>
#include <qcocoawindow.h>
#include <qcocoahelpers.h>
#include <qdebug.h>
#include <qcore_mac_p.h>
#include <qcocoanativecontext.h>

#include <cglconvenience_p.h>

#import <Cocoa/Cocoa.h>

static inline QByteArray getGlString(GLenum param)
{
   if (const GLubyte *s = glGetString(param)) {
      return QByteArray(reinterpret_cast<const char *>(s));
   }
   return QByteArray();
}

#if !defined(GL_CONTEXT_FLAGS)
#define GL_CONTEXT_FLAGS 0x821E
#endif

#if !defined(GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT)
#define GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT 0x0001
#endif

#if !defined(GL_CONTEXT_PROFILE_MASK)
#define GL_CONTEXT_PROFILE_MASK 0x9126
#endif

#if !defined(GL_CONTEXT_CORE_PROFILE_BIT)
#define GL_CONTEXT_CORE_PROFILE_BIT 0x00000001
#endif

#if !defined(GL_CONTEXT_COMPATIBILITY_PROFILE_BIT)
#define GL_CONTEXT_COMPATIBILITY_PROFILE_BIT 0x00000002
#endif

static void updateFormatFromContext(QSurfaceFormat *format)
{
   Q_ASSERT(format);

   // Update the version, profile, and context bit of the format
   int major = 0, minor = 0;
   QByteArray versionString(getGlString(GL_VERSION));
   if (QPlatformOpenGLContext::parseOpenGLVersion(versionString, major, minor)) {
      format->setMajorVersion(major);
      format->setMinorVersion(minor);
   }

   format->setProfile(QSurfaceFormat::NoProfile);

   Q_ASSERT(format->renderableType() == QSurfaceFormat::OpenGL);
   if (format->version() < qMakePair(3, 0)) {
      format->setOption(QSurfaceFormat::DeprecatedFunctions);
      return;
   }

   // Version 3.0 onwards - check if it includes deprecated functionality
   GLint value = 0;
   glGetIntegerv(GL_CONTEXT_FLAGS, &value);
   if (!(value & GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT)) {
      format->setOption(QSurfaceFormat::DeprecatedFunctions);
   }

   // Debug context option not supported on OS X

   if (format->version() < qMakePair(3, 2)) {
      return;
   }

   // Version 3.2 and newer have a profile
   value = 0;
   glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &value);

   if (value & GL_CONTEXT_CORE_PROFILE_BIT) {
      format->setProfile(QSurfaceFormat::CoreProfile);
   } else if (value & GL_CONTEXT_COMPATIBILITY_PROFILE_BIT) {
      format->setProfile(QSurfaceFormat::CompatibilityProfile);
   }
}

QCocoaGLContext::QCocoaGLContext(const QSurfaceFormat &format, QPlatformOpenGLContext *share,
   const QVariant &nativeHandle)
   : m_context(nil),
     m_shareContext(nil),
     m_format(format)
{
   if (!nativeHandle.isNull()) {
      if (!nativeHandle.canConvert<QCocoaNativeContext>()) {
         qWarning("QCocoaGLContext: Requires a QCocoaNativeContext");
         return;
      }
      QCocoaNativeContext handle = nativeHandle.value<QCocoaNativeContext>();
      NSOpenGLContext *context = handle.context();
      if (!context) {
         qWarning("QCocoaGLContext: No NSOpenGLContext given");
         return;
      }
      m_context = context;
      [m_context retain];
      m_shareContext = share ? static_cast<QCocoaGLContext *>(share)->nsOpenGLContext() : nil;
      // OpenGL surfaces can be ordered either above(default) or below the NSWindow.
      const GLint order = qt_mac_resolveOption(1, "QT_MAC_OPENGL_SURFACE_ORDER");
      [m_context setValues: &order forParameter: NSOpenGLCPSurfaceOrder];
      updateSurfaceFormat();
      return;
   }

   // we only support OpenGL contexts under Cocoa
   if (m_format.renderableType() == QSurfaceFormat::DefaultRenderableType) {
      m_format.setRenderableType(QSurfaceFormat::OpenGL);
   }
   if (m_format.renderableType() != QSurfaceFormat::OpenGL) {
      return;
   }

   QMacAutoReleasePool pool; // For the SG Canvas render thread

   // create native context for the requested pixel format and share
   NSOpenGLPixelFormat *pixelFormat = static_cast <NSOpenGLPixelFormat *>(qcgl_createNSOpenGLPixelFormat(m_format));
   m_shareContext = share ? static_cast<QCocoaGLContext *>(share)->nsOpenGLContext() : nil;
   m_context = [[NSOpenGLContext alloc] initWithFormat: pixelFormat shareContext: m_shareContext];

   // retry without sharing on context creation failure.
   if (!m_context && m_shareContext) {
      m_shareContext = nil;
      m_context = [[NSOpenGLContext alloc] initWithFormat: pixelFormat shareContext: nil];
      if (m_context) {
         qWarning("QCocoaGLContext: Falling back to unshared context.");
      }
   }

   // give up if we still did not get a native context
   [pixelFormat release];
   if (!m_context) {
      qWarning("QCocoaGLContext: Failed to create context.");
      return;
   }

   const GLint interval = format.swapInterval() >= 0 ? format.swapInterval() : 1;
   [m_context setValues: &interval forParameter: NSOpenGLCPSwapInterval];

   if (format.alphaBufferSize() > 0) {
      int zeroOpacity = 0;
      [m_context setValues: &zeroOpacity forParameter: NSOpenGLCPSurfaceOpacity];
   }


   // OpenGL surfaces can be ordered either above(default) or below the NSWindow.
   const GLint order = qt_mac_resolveOption(1, "QT_MAC_OPENGL_SURFACE_ORDER");
   [m_context setValues: &order forParameter: NSOpenGLCPSurfaceOrder];

   updateSurfaceFormat();
}

QCocoaGLContext::~QCocoaGLContext()
{
   if (m_currentWindow && m_currentWindow.data()->handle()) {
      static_cast<QCocoaWindow *>(m_currentWindow.data()->handle())->setCurrentContext(0);
   }

   [m_context release];
}

QVariant QCocoaGLContext::nativeHandle() const
{
   return QVariant::fromValue<QCocoaNativeContext>(QCocoaNativeContext(m_context));
}

// Match up with createNSOpenGLPixelFormat!
QSurfaceFormat QCocoaGLContext::format() const
{
   return m_format;
}

void QCocoaGLContext::windowWasHidden()
{
   // If the window is hidden, we need to unset the m_currentWindow
   // variable so that succeeding makeCurrent's will not abort prematurely
   // because of the optimization in setActiveWindow.
   // Doing a full doneCurrent here is not preferable, because the GL context
   // might be rendering in a different thread at this time.
   m_currentWindow.clear();
}

void QCocoaGLContext::swapBuffers(QPlatformSurface *surface)
{
   QWindow *window = static_cast<QCocoaWindow *>(surface)->window();
   setActiveWindow(window);

   [m_context flushBuffer];
}

bool QCocoaGLContext::makeCurrent(QPlatformSurface *surface)
{
   Q_ASSERT(surface->surface()->supportsOpenGL());

   QMacAutoReleasePool pool;

   QWindow *window = static_cast<QCocoaWindow *>(surface)->window();
   setActiveWindow(window);

   [m_context makeCurrentContext];
   update();
   return true;
}

void QCocoaGLContext::setActiveWindow(QWindow *window)
{
   if (window == m_currentWindow.data()) {
      return;
   }

   if (m_currentWindow && m_currentWindow.data()->handle()) {
      static_cast<QCocoaWindow *>(m_currentWindow.data()->handle())->setCurrentContext(0);
   }

   Q_ASSERT(window->handle());

   m_currentWindow = window;

   QCocoaWindow *cocoaWindow = static_cast<QCocoaWindow *>(window->handle());
   cocoaWindow->setCurrentContext(this);

   [(QNSView *) cocoaWindow->contentView() setQCocoaGLContext: this];
}

void QCocoaGLContext::updateSurfaceFormat()
{
   // need to populate the actual surface format from scratch

   QSurfaceFormat requestedFormat = m_format;
   m_format = QSurfaceFormat();
   m_format.setRenderableType(QSurfaceFormat::OpenGL);

   // CoreGL doesn't require a drawable to make the context current
   CGLContextObj oldContext = CGLGetCurrentContext();
   CGLContextObj ctx = static_cast<CGLContextObj>([m_context CGLContextObj]);
   CGLSetCurrentContext(ctx);

   // Get the data that OpenGL provides
   updateFormatFromContext(&m_format);

   // Get the data contained within the pixel format
   CGLPixelFormatObj cglPixelFormat = static_cast<CGLPixelFormatObj>(CGLGetPixelFormat(ctx));
   NSOpenGLPixelFormat *pixelFormat = [[NSOpenGLPixelFormat alloc] initWithCGLPixelFormatObj: cglPixelFormat];

   int colorSize = -1;
   [pixelFormat getValues: &colorSize forAttribute: NSOpenGLPFAColorSize forVirtualScreen: 0];
   if (colorSize > 0) {
      // This seems to return the total color buffer depth, including alpha
      m_format.setRedBufferSize(colorSize / 4);
      m_format.setGreenBufferSize(colorSize / 4);
      m_format.setBlueBufferSize(colorSize / 4);
   }

   // The pixel format always seems to return 8 for alpha. However, the framebuffer only
   // seems to have alpha enabled if we requested it explicitly. I can't find any other
   // attribute to check explicitly for this so we use our best guess for alpha.
   int alphaSize = -1;
   [pixelFormat getValues: &alphaSize forAttribute: NSOpenGLPFAAlphaSize forVirtualScreen: 0];
   if (alphaSize > 0 && requestedFormat.alphaBufferSize() > 0) {
      m_format.setAlphaBufferSize(alphaSize);
   }

   int depthSize = -1;
   [pixelFormat getValues: &depthSize forAttribute: NSOpenGLPFADepthSize forVirtualScreen: 0];
   if (depthSize > 0) {
      m_format.setDepthBufferSize(depthSize);
   }

   int stencilSize = -1;
   [pixelFormat getValues: &stencilSize forAttribute: NSOpenGLPFAStencilSize forVirtualScreen: 0];
   if (stencilSize > 0) {
      m_format.setStencilBufferSize(stencilSize);
   }

   int samples = -1;
   [pixelFormat getValues: &samples forAttribute: NSOpenGLPFASamples forVirtualScreen: 0];
   if (samples > 0) {
      m_format.setSamples(samples);
   }

   int doubleBuffered = -1;
   int tripleBuffered = -1;
   [pixelFormat getValues: &doubleBuffered forAttribute: NSOpenGLPFADoubleBuffer forVirtualScreen: 0];
   [pixelFormat getValues: &tripleBuffered forAttribute: NSOpenGLPFATripleBuffer forVirtualScreen: 0];

   if (tripleBuffered == 1) {
      m_format.setSwapBehavior(QSurfaceFormat::TripleBuffer);
   } else if (doubleBuffered == 1) {
      m_format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
   } else {
      m_format.setSwapBehavior(QSurfaceFormat::SingleBuffer);
   }

   int steroBuffers = -1;
   [pixelFormat getValues: &steroBuffers forAttribute: NSOpenGLPFAStereo forVirtualScreen: 0];
   if (steroBuffers == 1) {
      m_format.setOption(QSurfaceFormat::StereoBuffers);
   }

   [pixelFormat release];

   GLint swapInterval = -1;
   [m_context getValues: &swapInterval forParameter: NSOpenGLCPSwapInterval];
   if (swapInterval >= 0) {
      m_format.setSwapInterval(swapInterval);
   }

   // Restore the original context
   CGLSetCurrentContext(oldContext);
}

void QCocoaGLContext::doneCurrent()
{
   if (m_currentWindow && m_currentWindow.data()->handle()) {
      static_cast<QCocoaWindow *>(m_currentWindow.data()->handle())->setCurrentContext(0);
   }

   m_currentWindow.clear();

   [NSOpenGLContext clearCurrentContext];
}

void (*QCocoaGLContext::getProcAddress(const QByteArray &procName))()
{
   return qcgl_getProcAddress(procName);
}

void QCocoaGLContext::update()
{
   [m_context update];
}

NSOpenGLPixelFormat *QCocoaGLContext::createNSOpenGLPixelFormat(const QSurfaceFormat &format)
{
   return static_cast<NSOpenGLPixelFormat *>(qcgl_createNSOpenGLPixelFormat(format));
}

NSOpenGLContext *QCocoaGLContext::nsOpenGLContext() const
{
   return m_context;
}

bool QCocoaGLContext::isValid() const
{
   return m_context != nil;
}

bool QCocoaGLContext::isSharing() const
{
   return m_shareContext != nil;
}


