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

#include <qplatform_screen.h>

#include <qdebug.h>
#include <qguiapplication.h>
#include <qplatform_cursor.h>
#include <qplatform_integration.h>
#include <qscreen.h>
#include <qwindow.h>

#include <qguiapplication_p.h>
#include <qplatform_screen_p.h>
#include <qhighdpiscaling_p.h>

QPlatformScreen::QPlatformScreen()
   : d_ptr(new QPlatformScreenPrivate)
{
   Q_D(QPlatformScreen);
   d->screen = nullptr;
}

QPlatformScreen::~QPlatformScreen()
{
   Q_D(QPlatformScreen);

   if (d->screen) {
      qWarning("QPlatformScreen Destructor was invoked manually, use QPlatformIntegration::destroyScreen instead");
      QGuiApplicationPrivate::platformIntegration()->removeScreen(d->screen);
      delete d->screen;
   }
}

QPixmap QPlatformScreen::grabWindow(WId window, int x, int y, int width, int height) const
{
   (void) window;
   (void) x;
   (void) y;
   (void) width;
   (void) height;

   return QPixmap();
}

QWindow *QPlatformScreen::topLevelWindowAt(const QPoint &pos) const
{
   QWindowList list = QGuiApplication::topLevelWindows();

   for (int i = list.size() - 1; i >= 0; --i) {
      QWindow *w = list[i];

      if (w->isVisible() && QHighDpi::toNativePixels(w->geometry(), w).contains(pos)) {
         return w;
      }
   }

   return nullptr;
}

const QPlatformScreen *QPlatformScreen::screenForPosition(const QPoint &point) const
{
   if (! geometry().contains(point)) {
      for (const QPlatformScreen *screen : virtualSiblings()) {
         if (screen->geometry().contains(point)) {
            return screen;
         }
      }
   }
   return this;
}

QList<QPlatformScreen *> QPlatformScreen::virtualSiblings() const
{
   QList<QPlatformScreen *> list;
   list << const_cast<QPlatformScreen *>(this);
   return list;
}

QScreen *QPlatformScreen::screen() const
{
   Q_D(const QPlatformScreen);
   return d->screen;
}

QSizeF QPlatformScreen::physicalSize() const
{
   static constexpr const int dpi = 100;

   return QSizeF(geometry().size()) / dpi * qreal(25.4);
}

QDpi QPlatformScreen::logicalDpi() const
{
   QSizeF ps = physicalSize();
   QSize s = geometry().size();

   return QDpi(25.4 * s.width() / ps.width(),
         25.4 * s.height() / ps.height());
}

qreal QPlatformScreen::devicePixelRatio() const
{
   return 1.0;
}

void *QPlatformScreen::nativeHandle()
{
   return nullptr;
}

qreal QPlatformScreen::pixelDensity()  const
{
   return 1.0;
}

qreal QPlatformScreen::refreshRate() const
{
   return 60;
}

Qt::ScreenOrientation QPlatformScreen::nativeOrientation() const
{
   return Qt::PrimaryOrientation;
}

Qt::ScreenOrientation QPlatformScreen::orientation() const
{
   return Qt::PrimaryOrientation;
}

void QPlatformScreen::setOrientationUpdateMask(Qt::ScreenOrientations mask)
{
   (void) mask;
}

QPlatformScreen *QPlatformScreen::platformScreenForWindow(const QWindow *window)
{
   // QTBUG 32681: It can happen during the transition between screens
   // when one screen is disconnected that the window doesn't have a screen.

   if (! window->screen()) {
      return nullptr;
   }

   return window->screen()->handle();
}

QPlatformCursor *QPlatformScreen::cursor() const
{
   return nullptr;
}

void QPlatformScreen::resizeMaximizedWindows()
{
   QList<QWindow *> windows = QGuiApplication::allWindows();

   // 'screen()' still has the old geometry info while 'this' has the new geometry info
   const QRect oldGeometry = screen()->geometry();
   const QRect oldAvailableGeometry = screen()->availableGeometry();
   const QRect newGeometry = deviceIndependentGeometry();
   const QRect newAvailableGeometry = QHighDpi::fromNative(availableGeometry(), QHighDpiScaling::factor(this), newGeometry.topLeft());

   // make sure maximized and fullscreen windows are updated
   for (int i = 0; i < windows.size(); ++i) {
      QWindow *w = windows.at(i);

      // Skip non-platform windows, e.g., offscreen windows.
      if (!w->handle()) {
         continue;
      }

      if (platformScreenForWindow(w) != this) {
         continue;
      }

      if (w->windowState() & Qt::WindowMaximized || w->geometry() == oldAvailableGeometry) {
         w->setGeometry(newAvailableGeometry);
      } else if (w->windowState() & Qt::WindowFullScreen || w->geometry() == oldGeometry) {
         w->setGeometry(newGeometry);
      }
   }
}

// i must be power of two
static int log2(uint i)
{
   if (i == 0) {
      return -1;
   }

   int result = 0;
   while (!(i & 1)) {
      ++result;
      i >>= 1;
   }
   return result;
}

int QPlatformScreen::angleBetween(Qt::ScreenOrientation a, Qt::ScreenOrientation b)
{
   if (a == Qt::PrimaryOrientation || b == Qt::PrimaryOrientation) {
      qWarning("QPlatformScreen::angleBetween() Use angleBetween() in the QScreen class when passing Qt::PrimaryOrientation");
      return 0;
   }

   if (a == b) {
      return 0;
   }

   int ia = log2(uint(a));
   int ib = log2(uint(b));

   int delta = ia - ib;

   if (delta < 0) {
      delta = delta + 4;
   }

   int angles[] = { 0, 90, 180, 270 };
   return angles[delta];
}

QTransform QPlatformScreen::transformBetween(Qt::ScreenOrientation a, Qt::ScreenOrientation b, const QRect &target)
{
   if (a == Qt::PrimaryOrientation || b == Qt::PrimaryOrientation) {
      qWarning("QPlatformScreen::transformBetween() Use transformBetween() in the QScreen class when passing Qt::PrimaryOrientation");
      return QTransform();
   }

   if (a == b) {
      return QTransform();
   }

   int angle = angleBetween(a, b);

   QTransform result;
   switch (angle) {
      case 90:
         result.translate(target.width(), 0);
         break;
      case 180:
         result.translate(target.width(), target.height());
         break;
      case 270:
         result.translate(0, target.height());
         break;
      default:
         Q_ASSERT(false);
   }
   result.rotate(angle);

   return result;
}

QRect QPlatformScreen::mapBetween(Qt::ScreenOrientation a, Qt::ScreenOrientation b, const QRect &rect)
{
   if (a == Qt::PrimaryOrientation || b == Qt::PrimaryOrientation) {
      qWarning("QPlatformScreen::mapBetween() Use mapBetween() in the QScreen class when passing Qt::PrimaryOrientation");
      return rect;
   }

   if (a == b) {
      return rect;
   }

   if ((a == Qt::PortraitOrientation || a == Qt::InvertedPortraitOrientation)
      != (b == Qt::PortraitOrientation || b == Qt::InvertedPortraitOrientation)) {
      return QRect(rect.y(), rect.x(), rect.height(), rect.width());
   }

   return rect;
}

QRect QPlatformScreen::deviceIndependentGeometry() const
{
   qreal scaleFactor = QHighDpiScaling::factor(this);
   QRect nativeGeometry = geometry();

   return QRect(nativeGeometry.topLeft(), QHighDpi::fromNative(nativeGeometry.size(), scaleFactor));
}

QPlatformScreen::SubpixelAntialiasingType QPlatformScreen::subpixelAntialiasingTypeHint() const
{
   static int type = -1;

   if (type == -1) {
      QByteArray env = qgetenv("QT_SUBPIXEL_AA_TYPE");

      if (env == "RGB") {
         type = QPlatformScreen::Subpixel_RGB;
      }

      else if (env == "BGR") {
         type = QPlatformScreen::Subpixel_BGR;
      }

      else if (env == "VRGB") {
         type = QPlatformScreen::Subpixel_VRGB;
      }

      else if (env == "VBGR") {
         type = QPlatformScreen::Subpixel_VBGR;
      }

      else {
         type = QPlatformScreen::Subpixel_None;
      }

   }

   return static_cast<QPlatformScreen::SubpixelAntialiasingType>(type);
}

QPlatformScreen::PowerState QPlatformScreen::powerState() const
{
   return PowerStateOn;
}

void QPlatformScreen::setPowerState(PowerState state)
{
   (void) state;
}

