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

#ifndef QPLATFORM_SCREEN_H
#define QPLATFORM_SCREEN_H

#include <qnamespace.h>
#include <qcoreevent.h>
#include <qvariant.h>
#include <qrect.h>
#include <qobject.h>
#include <qcursor.h>
#include <qimage.h>
#include <qwindowdefs.h>
#include <qplatform_pixmap.h>

class QScreen;
class QSurfaceFormat;
class QPlatformCursor;
class QPlatformBackingStore;
class QPlatformOpenGLContext;
class QPlatformScreenPrivate;
class QPlatformWindow;

using QDpi = QPair<qreal, qreal>;

class Q_GUI_EXPORT QPlatformScreen
{
 public:
   enum SubpixelAntialiasingType {
      // copied from qfontengine_p.h since we can't include private headers
      Subpixel_None,
      Subpixel_RGB,
      Subpixel_BGR,
      Subpixel_VRGB,
      Subpixel_VBGR
   };

   enum PowerState {
      PowerStateOn,
      PowerStateStandby,
      PowerStateSuspend,
      PowerStateOff
   };

   QPlatformScreen();

   QPlatformScreen(const QPlatformScreen &) = delete;
   QPlatformScreen &operator=(const QPlatformScreen &) = delete;

   virtual ~QPlatformScreen();

   virtual QPixmap grabWindow(WId window, int x, int y, int width, int height) const;

   virtual QRect geometry() const = 0;
   virtual QRect availableGeometry() const {
      return geometry();
   }

   virtual int depth() const = 0;
   virtual QImage::Format format() const = 0;

   virtual QSizeF physicalSize() const;
   virtual QDpi logicalDpi() const;
   virtual qreal devicePixelRatio() const;
   virtual qreal pixelDensity()  const;

   virtual qreal refreshRate() const;

   virtual Qt::ScreenOrientation nativeOrientation() const;
   virtual Qt::ScreenOrientation orientation() const;
   virtual void setOrientationUpdateMask(Qt::ScreenOrientations mask);

   virtual QWindow *topLevelWindowAt(const QPoint &point) const;
   virtual QList<QPlatformScreen *> virtualSiblings() const;
   const QPlatformScreen *screenForPosition(const QPoint &point) const;

   QScreen *screen() const;

   // should this function be in QPlatformIntegration
   // maybe screenForWindow is a better name?

   static QPlatformScreen *platformScreenForWindow(const QWindow *window);

   virtual QString name() const {
      return QString();
   }

   virtual QPlatformCursor *cursor() const;
   virtual SubpixelAntialiasingType subpixelAntialiasingTypeHint() const;

   virtual PowerState powerState() const;
   virtual void setPowerState(PowerState state);

   static int angleBetween(Qt::ScreenOrientation a, Qt::ScreenOrientation b);
   static QTransform transformBetween(Qt::ScreenOrientation a, Qt::ScreenOrientation b, const QRect &target);
   static QRect mapBetween(Qt::ScreenOrientation a, Qt::ScreenOrientation b, const QRect &rect);

   // The platform screen's geometry in device independent coordinates
   QRect deviceIndependentGeometry() const;

   virtual void * nativeHandle();

 protected:
   void resizeMaximizedWindows();

   QScopedPointer<QPlatformScreenPrivate> d_ptr;

 private:
   Q_DECLARE_PRIVATE(QPlatformScreen)
   friend class QScreenPrivate;
};

#endif
