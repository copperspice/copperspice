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

#ifndef QPLATFORM_OPENGLCONTEXT_H
#define QPLATFORM_OPENGLCONTEXT_H

#include <qnamespace.h>

#ifndef QT_NO_OPENGL
#include <qsurfaceformat.h>
#include <qwindow.h>
#include <qopengl.h>

class QPlatformOpenGLContextPrivate;

class Q_GUI_EXPORT QPlatformOpenGLContext
{
   Q_DECLARE_PRIVATE(QPlatformOpenGLContext)

 public:
   using FP_Void = void(*)();

   QPlatformOpenGLContext();

   QPlatformOpenGLContext(const QPlatformOpenGLContext &) = delete;
   QPlatformOpenGLContext &operator=(const QPlatformOpenGLContext &) = delete;

   virtual ~QPlatformOpenGLContext();

   virtual void initialize();
   virtual QSurfaceFormat format() const = 0;

   virtual void swapBuffers(QPlatformSurface *surface) = 0;

   virtual GLuint defaultFramebufferObject(QPlatformSurface *surface) const;
   virtual bool makeCurrent(QPlatformSurface *surface) = 0;
   virtual void doneCurrent() = 0;

   virtual bool isSharing() const {
      return false;
   }

   virtual bool isValid() const {
      return true;
   }

   virtual FP_Void getProcAddress(const QByteArray &procName) = 0;

   QOpenGLContext *context() const;
   static bool parseOpenGLVersion(const QByteArray &versionString, int &major, int &minor);

 protected:
   QScopedPointer<QPlatformOpenGLContextPrivate> d_ptr;

 private:
   friend class QOpenGLContext;

   void setContext(QOpenGLContext *context);
};

#endif  // no-opengl

#endif
