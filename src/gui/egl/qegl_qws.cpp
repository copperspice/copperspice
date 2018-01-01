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

#include <QtGui/qpaintdevice.h>
#include <QtGui/qpixmap.h>
#include <QtGui/qwidget.h>

#include <qegl_p.h>
#include <qeglcontext_p.h>

#if !defined(QT_NO_EGL)

#include <qscreen_qws.h>
#include <qscreenproxy_qws.h>
#include <qapplication.h>
#include <qdesktopwidget.h>

QT_BEGIN_NAMESPACE

static QScreen *screenForDevice(QPaintDevice *device)
{
   QScreen *screen = qt_screen;
   if (!screen) {
      return 0;
   }
   if (screen->classId() == QScreen::MultiClass) {
      int screenNumber;
      if (device && device->devType() == QInternal::Widget) {
         screenNumber = qApp->desktop()->screenNumber(static_cast<QWidget *>(device));
      } else {
         screenNumber = 0;
      }
      screen = screen->subScreens()[screenNumber];
   }
   while (screen->classId() == QScreen::ProxyClass ||
          screen->classId() == QScreen::TransformedClass) {
      screen = static_cast<QProxyScreen *>(screen)->screen();
   }
   return screen;
}

// Set pixel format and other properties based on a paint device.
void QEglProperties::setPaintDeviceFormat(QPaintDevice *dev)
{
   if (!dev) {
      return;
   }

   // Find the QGLScreen for this paint device.
   QScreen *screen = screenForDevice(dev);
   if (!screen) {
      return;
   }
   int devType = dev->devType();
   if (devType == QInternal::Image) {
      setPixelFormat(static_cast<QImage *>(dev)->format());
   } else {
      setPixelFormat(screen->pixelFormat());
   }
}

EGLNativeDisplayType QEgl::nativeDisplay()
{
   return  EGLNativeDisplayType(EGL_DEFAULT_DISPLAY);
}

EGLNativeWindowType QEgl::nativeWindow(QWidget *widget)
{
   return (EGLNativeWindowType)(widget->winId()); // Might work
}

EGLNativePixmapType QEgl::nativePixmap(QPixmap *)
{
   qWarning("QEgl: EGL pixmap surfaces not supported on QWS");
   return (EGLNativePixmapType)0;
}


QT_END_NAMESPACE

#endif // !QT_NO_EGL
