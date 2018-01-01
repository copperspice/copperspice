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
#include <qeglcontext_p.h>

#if !defined(QT_NO_EGL)

#include <qgraphicssystem_p.h>
#include <qapplication_p.h>
#include <qdesktopwidget.h>

QT_BEGIN_NAMESPACE

EGLNativeDisplayType QEgl::nativeDisplay()
{
   return EGLNativeDisplayType(EGL_DEFAULT_DISPLAY);
}

EGLNativeWindowType QEgl::nativeWindow(QWidget *widget)
{
   return (EGLNativeWindowType)(widget->winId());
}

EGLNativePixmapType QEgl::nativePixmap(QPixmap *pixmap)
{
   Q_UNUSED(pixmap);
   return 0;
}

//EGLDisplay QEglContext::display()
//{
//    return eglGetDisplay(EGLNativeDisplayType(EGL_DEFAULT_DISPLAY));
//}

static QPlatformScreen *screenForDevice(QPaintDevice *device)
{
   QPlatformIntegration *pi = QApplicationPrivate::platformIntegration();

   QList<QPlatformScreen *> screens = pi->screens();

   int screenNumber;
   if (device && device->devType() == QInternal::Widget) {
      screenNumber = qApp->desktop()->screenNumber(static_cast<QWidget *>(device));
   } else {
      screenNumber = 0;
   }
   if (screenNumber < 0 || screenNumber >= screens.size()) {
      return 0;
   }
   return screens[screenNumber];
}

// Set pixel format and other properties based on a paint device.
void QEglProperties::setPaintDeviceFormat(QPaintDevice *dev)
{
   if (!dev) {
      return;
   }

   // Find the QGLScreen for this paint device.
   QPlatformScreen *screen = screenForDevice(dev);
   if (!screen) {
      return;
   }
   int devType = dev->devType();
   if (devType == QInternal::Image) {
      setPixelFormat(static_cast<QImage *>(dev)->format());
   } else {
      setPixelFormat(screen->format());
   }
}

QT_END_NAMESPACE

#endif // !QT_NO_EGL
