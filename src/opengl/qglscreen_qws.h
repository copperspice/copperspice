/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QSCREENEGL_P_H
#define QSCREENEGL_P_H

#include <QtGui/QScreen>
#include <QtOpenGL/qgl.h>

#if defined(QT_OPENGL_ES_2)
#include <EGL/egl.h>
#else
#include <GLES/egl.h>
#endif

#if !defined(EGL_VERSION_1_3) && !defined(QEGL_NATIVE_TYPES_DEFINED)
#undef EGLNativeWindowType
#undef EGLNativePixmapType
#undef EGLNativeDisplayType
typedef NativeWindowType EGLNativeWindowType;
typedef NativePixmapType EGLNativePixmapType;
typedef NativeDisplayType EGLNativeDisplayType;
#define QEGL_NATIVE_TYPES_DEFINED 1
#endif

QT_BEGIN_NAMESPACE

class QGLScreenPrivate;

class Q_OPENGL_EXPORT QGLScreenSurfaceFunctions
{
 public:
   virtual bool createNativeWindow(QWidget *widget, EGLNativeWindowType *native);
   virtual bool createNativePixmap(QPixmap *pixmap, EGLNativePixmapType *native);
   virtual bool createNativeImage(QImage *image, EGLNativePixmapType *native);
};

class Q_OPENGL_EXPORT QGLScreen : public QScreen
{
   Q_DECLARE_PRIVATE(QGLScreen)

 public:
   QGLScreen(int displayId);
   virtual ~QGLScreen();

   enum Option {
      NoOptions       = 0,
      NativeWindows   = 1,
      NativePixmaps   = 2,
      NativeImages    = 4,
      Overlays        = 8
   };
   using Options = QFlags<Option>;

   QGLScreen::Options options() const;

   virtual bool chooseContext(QGLContext *context, const QGLContext *shareContext);
   virtual bool hasOpenGL() = 0;

   QGLScreenSurfaceFunctions *surfaceFunctions() const;

 protected:
   void setOptions(QGLScreen::Options value);
   void setSurfaceFunctions(QGLScreenSurfaceFunctions *functions);

 private:
   QGLScreenPrivate *d_ptr;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QGLScreen::Options)

QT_END_NAMESPACE

#endif // QSCREENEGL_P_H
