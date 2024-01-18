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

#ifndef QGLX_CONTEXT_H
#define QGLX_CONTEXT_H

#include <qxcb_window.h>
#include <qxcb_screen.h>
#include <qplatform_openglcontext.h>
#include <qplatform_offscreensurface.h>
#include <qsurfaceformat.h>
#include <qmutex.h>

#include <GL/glx.h>

class QGLXContext : public QPlatformOpenGLContext
{
 public:
   QGLXContext(QXcbScreen *screen, const QSurfaceFormat &format, QPlatformOpenGLContext *share,
      const QVariant &nativeHandle);
   ~QGLXContext();

   bool makeCurrent(QPlatformSurface *surface) override;
   void doneCurrent() override;
   void swapBuffers(QPlatformSurface *surface) override;
   FP_Void getProcAddress(const QByteArray &procName) override;

   QSurfaceFormat format() const override;
   bool isSharing() const override;
   bool isValid() const override;

   GLXContext glxContext() const {
      return m_context;
   }
   GLXFBConfig glxConfig() const {
      return m_config;
   }

   QVariant nativeHandle() const;

   static bool supportsThreading();
   static void queryDummyContext();

 private:
   void init(QXcbScreen *screen, QPlatformOpenGLContext *share);
   void init(QXcbScreen *screen, QPlatformOpenGLContext *share, const QVariant &nativeHandle);

   Display *m_display;
   GLXFBConfig m_config;
   GLXContext m_context;
   GLXContext m_shareContext;
   QSurfaceFormat m_format;
   bool m_isPBufferCurrent;
   int m_swapInterval;
   bool m_ownsContext;
   static bool m_queriedDummyContext;
   static bool m_supportsThreading;
};

class QGLXPbuffer : public QPlatformOffscreenSurface
{
 public:
   explicit QGLXPbuffer(QOffscreenSurface *offscreenSurface);
   ~QGLXPbuffer();

   QSurfaceFormat format() const override {
      return m_format;
   }
   bool isValid() const override {
      return m_pbuffer != 0;
   }

   GLXPbuffer pbuffer() const {
      return m_pbuffer;
   }

 private:
   QSurfaceFormat m_format;
   QXcbScreen *m_screen;
   GLXPbuffer m_pbuffer;
};

#endif
