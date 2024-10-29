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

#include <qscreen.h>
#include <qscreen_p.h>
#include <qdebug.h>
#include <qpixmap.h>
#include <qplatform_screen.h>

#include <qapplication_p.h>
#include <qhighdpiscaling_p.h>
#include <qplatform_screen_p.h>

QScreen::QScreen(QPlatformScreen *screen)
   : d_ptr(new QScreenPrivate)
{
   d_ptr->q_ptr = this;
   Q_D(QScreen);

   d->setPlatformScreen(screen);
}

void QScreenPrivate::setPlatformScreen(QPlatformScreen *screen)
{
   Q_Q(QScreen);
   platformScreen = screen;
   platformScreen->d_func()->screen = q;
   orientation = platformScreen->orientation();
   geometry = platformScreen->deviceIndependentGeometry();
   availableGeometry = QHighDpi::fromNative(platformScreen->availableGeometry(), QHighDpiScaling::factor(platformScreen),
         geometry.topLeft());
   logicalDpi = platformScreen->logicalDpi();
   refreshRate = platformScreen->refreshRate();

   // safeguard ourselves against buggy platform behavior...
   if (refreshRate < 1.0) {
      refreshRate = 60.0;
   }

   updatePrimaryOrientation();

   filteredOrientation = orientation;
   if (filteredOrientation == Qt::PrimaryOrientation) {
      filteredOrientation = primaryOrientation;
   }

   updateHighDpi();
}

QScreen::~QScreen()
{
   if (! qApp) {
      return;
   }

   // Allow clients to manage windows that are affected by the screen going
   // away, before we fall back to moving them to the primary screen.

   emit qApp->screenRemoved(this);

   if (QGuiApplication::closingDown()) {
      return;
   }

   QScreen *primaryScreen = QGuiApplication::primaryScreen();
   if (this == primaryScreen) {
      return;
   }

   bool movingFromVirtualSibling = primaryScreen && primaryScreen->handle()->virtualSiblings().contains(handle());

   // Move any leftover windows to the primary screen
   for (QWindow *window : QGuiApplication::allWindows()) {
      if (!window->isTopLevel() || window->screen() != this) {
         continue;
      }

      const bool wasVisible = window->isVisible();
      window->setScreen(primaryScreen);

      // Re-show window if moved from a virtual sibling screen. Otherwise
      // leave it up to the application developer to show the window.
      if (movingFromVirtualSibling) {
         window->setVisible(wasVisible);
      }
   }
}

QPlatformScreen *QScreen::handle() const
{
   Q_D(const QScreen);
   return d->platformScreen;
}

QString QScreen::name() const
{
   Q_D(const QScreen);
   return d->platformScreen->name();
}

int QScreen::depth() const
{
   Q_D(const QScreen);
   return d->platformScreen->depth();
}

QSize QScreen::size() const
{
   Q_D(const QScreen);
   return d->geometry.size();
}

qreal QScreen::physicalDotsPerInchX() const
{
   return size().width() / physicalSize().width() * qreal(25.4);
}

qreal QScreen::physicalDotsPerInchY() const
{
   return size().height() / physicalSize().height() * qreal(25.4);
}

qreal QScreen::physicalDotsPerInch() const
{
   QSize sz = size();
   QSizeF psz = physicalSize();
   return ((sz.height() / psz.height()) + (sz.width() / psz.width())) * qreal(25.4 * 0.5);
}

qreal QScreen::logicalDotsPerInchX() const
{
   Q_D(const QScreen);
   if (QHighDpiScaling::isActive()) {
      return QHighDpiScaling::logicalDpi().first;
   }
   return d->logicalDpi.first;
}

qreal QScreen::logicalDotsPerInchY() const
{
   Q_D(const QScreen);
   if (QHighDpiScaling::isActive()) {
      return QHighDpiScaling::logicalDpi().second;
   }
   return d->logicalDpi.second;
}

qreal QScreen::logicalDotsPerInch() const
{
   Q_D(const QScreen);
   QDpi dpi = QHighDpiScaling::isActive() ? QHighDpiScaling::logicalDpi() : d->logicalDpi;
   return (dpi.first + dpi.second) * qreal(0.5);
}

qreal QScreen::devicePixelRatio() const
{
   Q_D(const QScreen);
   return d->platformScreen->devicePixelRatio() * QHighDpiScaling::factor(this);
}


QSizeF QScreen::physicalSize() const
{
   Q_D(const QScreen);
   return d->platformScreen->physicalSize();
}

QSize QScreen::availableSize() const
{
   Q_D(const QScreen);
   return d->availableGeometry.size();
}

QRect QScreen::geometry() const
{
   Q_D(const QScreen);
   return d->geometry;
}

QRect QScreen::availableGeometry() const
{
   Q_D(const QScreen);
   return d->availableGeometry;
}

QList<QScreen *> QScreen::virtualSiblings() const
{

   Q_D(const QScreen);
   QList<QPlatformScreen *> platformScreens = d->platformScreen->virtualSiblings();
   QList<QScreen *> screens;

   for (QPlatformScreen *platformScreen : platformScreens) {
      screens << platformScreen->screen();
   }
   return screens;
}

QSize QScreen::virtualSize() const
{
   return virtualGeometry().size();
}

QRect QScreen::virtualGeometry() const
{
   QRect result;
   for (QScreen *screen : virtualSiblings()) {
      result |= screen->geometry();
   }
   return result;
}

QSize QScreen::availableVirtualSize() const
{
   return availableVirtualGeometry().size();
}

QRect QScreen::availableVirtualGeometry() const
{
   QRect result;
   for (QScreen *screen : virtualSiblings()) {
      result |= screen->availableGeometry();
   }
   return result;
}

void QScreen::setOrientationUpdateMask(Qt::ScreenOrientations mask)
{
   Q_D(QScreen);

   d->orientationUpdateMask = mask;
   d->platformScreen->setOrientationUpdateMask(mask);
   QGuiApplicationPrivate::updateFilteredScreenOrientation(this);
}

Qt::ScreenOrientations QScreen::orientationUpdateMask() const
{
   Q_D(const QScreen);
   return d->orientationUpdateMask;
}

Qt::ScreenOrientation QScreen::orientation() const
{
   Q_D(const QScreen);
   return d->filteredOrientation;
}

qreal QScreen::refreshRate() const
{
   Q_D(const QScreen);
   return d->refreshRate;
}

Qt::ScreenOrientation QScreen::primaryOrientation() const
{
   Q_D(const QScreen);
   return d->primaryOrientation;
}

Qt::ScreenOrientation QScreen::nativeOrientation() const
{
   Q_D(const QScreen);
   return d->platformScreen->nativeOrientation();
}

int QScreen::angleBetween(Qt::ScreenOrientation a, Qt::ScreenOrientation b) const
{
   if (a == Qt::PrimaryOrientation) {
      a = primaryOrientation();
   }

   if (b == Qt::PrimaryOrientation) {
      b = primaryOrientation();
   }

   return QPlatformScreen::angleBetween(a, b);
}

QTransform QScreen::transformBetween(Qt::ScreenOrientation a, Qt::ScreenOrientation b, const QRect &target) const
{
   if (a == Qt::PrimaryOrientation) {
      a = primaryOrientation();
   }

   if (b == Qt::PrimaryOrientation) {
      b = primaryOrientation();
   }

   return QPlatformScreen::transformBetween(a, b, target);
}

QRect QScreen::mapBetween(Qt::ScreenOrientation a, Qt::ScreenOrientation b, const QRect &rect) const
{
   if (a == Qt::PrimaryOrientation) {
      a = primaryOrientation();
   }

   if (b == Qt::PrimaryOrientation) {
      b = primaryOrientation();
   }

   return QPlatformScreen::mapBetween(a, b, rect);
}

bool QScreen::isPortrait(Qt::ScreenOrientation o) const
{
   return o == Qt::PortraitOrientation || o == Qt::InvertedPortraitOrientation
      || (o == Qt::PrimaryOrientation && primaryOrientation() == Qt::PortraitOrientation);
}

bool QScreen::isLandscape(Qt::ScreenOrientation o) const
{
   return o == Qt::LandscapeOrientation || o == Qt::InvertedLandscapeOrientation
      || (o == Qt::PrimaryOrientation && primaryOrientation() == Qt::LandscapeOrientation);
}

void QScreenPrivate::updatePrimaryOrientation()
{
   primaryOrientation = geometry.width() >= geometry.height() ? Qt::LandscapeOrientation : Qt::PortraitOrientation;
}

QPixmap QScreen::grabWindow(WId window, int x, int y, int width, int height)
{
   const QPlatformScreen *platformScreen = handle();
   if (! platformScreen) {
      qWarning("QScreen::grabWindow() Called with an invalid handle (0)");
      return QPixmap();
   }

   const qreal factor = QHighDpiScaling::factor(this);
   if (qFuzzyCompare(factor, 1)) {
      return platformScreen->grabWindow(window, x, y, width, height);
   }

   const QPoint nativePos = QHighDpi::toNative(QPoint(x, y), factor);
   QSize nativeSize(width, height);

   if (nativeSize.isValid()) {
      nativeSize = QHighDpi::toNative(nativeSize, factor);
   }

   QPixmap result = platformScreen->grabWindow(window, nativePos.x(), nativePos.y(),
         nativeSize.width(), nativeSize.height());

   result.setDevicePixelRatio(factor);

   return result;
}

static inline void formatRect(QDebug &debug, const QRect r)
{
   debug << r.width() << 'x' << r.height()
      << forcesign << r.x() << r.y() << noforcesign;
}

Q_GUI_EXPORT QDebug operator<<(QDebug debug, const QScreen *screen)
{
   const QDebugStateSaver saver(debug);
   debug.nospace();

   debug << "QScreen(" << (const void *)screen;

   if (screen) {
      debug << ", name=" << screen->name();

      if (screen == QGuiApplication::primaryScreen()) {
         debug << ", primary";
      }

      debug << ", geometry=";

      formatRect(debug, screen->geometry());
      debug << ", available=";

      formatRect(debug, screen->availableGeometry());
      debug << ", logical DPI=" << screen->logicalDotsPerInchX()
         << ',' << screen->logicalDotsPerInchY()
         << ", physical DPI=" << screen->physicalDotsPerInchX()
         << ',' << screen->physicalDotsPerInchY()
         << ", devicePixelRatio=" << screen->devicePixelRatio()
         << ", orientation=" << screen->orientation()
         << ", physical size=" << screen->physicalSize().width()
         << 'x' << screen->physicalSize().height() << "mm";

   }

   debug << ')';
   return debug;
}
