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

#ifndef QCOCOAGLCONTEXT_H
#define QCOCOAGLCONTEXT_H

#include <QPointer>
#include <qplatform_openglcontext.h>
#include <QOpenGLContext>
#include <QWindow>

#undef slots
#include <Cocoa/Cocoa.h>

class QCocoaGLContext : public QPlatformOpenGLContext
{
 public:
   QCocoaGLContext(const QSurfaceFormat &format, QPlatformOpenGLContext *share, const QVariant &nativeHandle);
   ~QCocoaGLContext();

   QSurfaceFormat format() const override;

   void swapBuffers(QPlatformSurface *surface) override;

   bool makeCurrent(QPlatformSurface *surface) override;
   void doneCurrent() override;

   void (*getProcAddress(const QByteArray &procName)) () override;

   void update();

   static NSOpenGLPixelFormat *createNSOpenGLPixelFormat(const QSurfaceFormat &format);
   NSOpenGLContext *nsOpenGLContext() const;

   bool isSharing() const override;
   bool isValid() const override;

   void windowWasHidden();

   QVariant nativeHandle() const;

 private:
   void setActiveWindow(QWindow *window);
   void updateSurfaceFormat();

   NSOpenGLContext *m_context;
   NSOpenGLContext *m_shareContext;
   QSurfaceFormat m_format;
   QPointer<QWindow> m_currentWindow;
};

#endif // QCOCOAGLCONTEXT_H
