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

/*!
  \property QScreen::depth
  \brief the color depth of the screen
*/
int QScreen::depth() const
{
   Q_D(const QScreen);
   return d->platformScreen->depth();
}

/*!
  \property QScreen::size
  \brief the pixel resolution of the screen
*/
QSize QScreen::size() const
{
   Q_D(const QScreen);
   return d->geometry.size();
}

/*!
  \property QScreen::physicalDotsPerInchX
  \brief the number of physical dots or pixels per inch in the horizontal direction

  This value represents the actual horizontal pixel density on the screen's display.
  Depending on what information the underlying system provides the value might not be
  entirely accurate.

  \sa physicalDotsPerInchY()
*/
qreal QScreen::physicalDotsPerInchX() const
{
   return size().width() / physicalSize().width() * qreal(25.4);
}

/*!
  \property QScreen::physicalDotsPerInchY
  \brief the number of physical dots or pixels per inch in the vertical direction

  This value represents the actual vertical pixel density on the screen's display.
  Depending on what information the underlying system provides the value might not be
  entirely accurate.

  \sa physicalDotsPerInchX()
*/
qreal QScreen::physicalDotsPerInchY() const
{
   return size().height() / physicalSize().height() * qreal(25.4);
}

/*!
  \property QScreen::physicalDotsPerInch
  \brief the number of physical dots or pixels per inch

  This value represents the pixel density on the screen's display.
  Depending on what information the underlying system provides the value might not be
  entirely accurate.

  This is a convenience property that's simply the average of the physicalDotsPerInchX
  and physicalDotsPerInchY properties.

  \sa physicalDotsPerInchX()
  \sa physicalDotsPerInchY()
*/
qreal QScreen::physicalDotsPerInch() const
{
   QSize sz = size();
   QSizeF psz = physicalSize();
   return ((sz.height() / psz.height()) + (sz.width() / psz.width())) * qreal(25.4 * 0.5);
}

/*!
  \property QScreen::logicalDotsPerInchX
  \brief the number of logical dots or pixels per inch in the horizontal direction

  This value is used to convert font point sizes to pixel sizes.

  \sa logicalDotsPerInchY()
*/
qreal QScreen::logicalDotsPerInchX() const
{
   Q_D(const QScreen);
   if (QHighDpiScaling::isActive()) {
      return QHighDpiScaling::logicalDpi().first;
   }
   return d->logicalDpi.first;
}

/*!
  \property QScreen::logicalDotsPerInchY
  \brief the number of logical dots or pixels per inch in the vertical direction

  This value is used to convert font point sizes to pixel sizes.

  \sa logicalDotsPerInchX()
*/
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

/*!
  Get the screen's virtual siblings.

  The virtual siblings are the screen instances sharing the same virtual desktop.
  They share a common coordinate system, and windows can freely be moved or
  positioned across them without having to be re-created.
*/
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

/*!
    Returns the currently set orientation update mask.

    \sa setOrientationUpdateMask()
*/
Qt::ScreenOrientations QScreen::orientationUpdateMask() const
{
   Q_D(const QScreen);
   return d->orientationUpdateMask;
}

/*!
    \property QScreen::orientation
    \brief the screen orientation

    The screen orientation represents the physical orientation
    of the display. For example, the screen orientation of a mobile device
    will change based on how it is being held. A change to the orientation
    might or might not trigger a change to the primary orientation of the screen.

    Changes to this property will be filtered by orientationUpdateMask(),
    so in order to receive orientation updates the application must first
    call setOrientationUpdateMask() with a mask of the orientations it wants
    to receive.

    Qt::PrimaryOrientation is never returned.

    \sa primaryOrientation()
*/
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

/*!
    Convenience function to compute a transform that maps from the coordinate system
    defined by orientation \a a into the coordinate system defined by orientation
    \a b and target dimensions \a target.

    Example, \a a is Qt::Landscape, \a b is Qt::Portrait, and \a target is QRect(0, 0, w, h)
    the resulting transform will be such that the point QPoint(0, 0) is mapped to QPoint(0, w),
    and QPoint(h, w) is mapped to QPoint(0, h). Thus, the landscape coordinate system QRect(0, 0, h, w)
    is mapped (with a 90 degree rotation) into the portrait coordinate system QRect(0, 0, w, h).

    Qt::PrimaryOrientation is interpreted as the screen's primaryOrientation().
*/
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

/*!
    Maps the rect between two screen orientations.

    This will flip the x and y dimensions of the rectangle \a{rect} if the orientation \a{a} is
    Qt::PortraitOrientation or Qt::InvertedPortraitOrientation and orientation \a{b} is
    Qt::LandscapeOrientation or Qt::InvertedLandscapeOrientation, or vice versa.

    Qt::PrimaryOrientation is interpreted as the screen's primaryOrientation().
*/
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

/*!
    Convenience function that returns \c true if \a o is either portrait or inverted portrait;
    otherwise returns \c false.

    Qt::PrimaryOrientation is interpreted as the screen's primaryOrientation().
*/
bool QScreen::isPortrait(Qt::ScreenOrientation o) const
{
   return o == Qt::PortraitOrientation || o == Qt::InvertedPortraitOrientation
      || (o == Qt::PrimaryOrientation && primaryOrientation() == Qt::PortraitOrientation);
}

/*!
    Convenience function that returns \c true if \a o is either landscape or inverted landscape;
    otherwise returns \c false.

    Qt::PrimaryOrientation is interpreted as the screen's primaryOrientation().
*/
bool QScreen::isLandscape(Qt::ScreenOrientation o) const
{
   return o == Qt::LandscapeOrientation || o == Qt::InvertedLandscapeOrientation
      || (o == Qt::PrimaryOrientation && primaryOrientation() == Qt::LandscapeOrientation);
}

/*!
    \fn void QScreen::orientationChanged(Qt::ScreenOrientation orientation)

    This signal is emitted when the orientation of the screen
    changes with \a orientation as an argument.

    \sa orientation()
*/

/*!
    \fn void QScreen::primaryOrientationChanged(Qt::ScreenOrientation orientation)

    This signal is emitted when the primary orientation of the screen
    changes with \a orientation as an argument.

    \sa primaryOrientation()
*/

void QScreenPrivate::updatePrimaryOrientation()
{
   primaryOrientation = geometry.width() >= geometry.height() ? Qt::LandscapeOrientation : Qt::PortraitOrientation;
}

/*!
    Creates and returns a pixmap constructed by grabbing the contents
    of the given \a window restricted by QRect(\a x, \a y, \a width,
    \a height).

    The arguments (\a{x}, \a{y}) specify the offset in the window,
    whereas (\a{width}, \a{height}) specify the area to be copied.  If
    \a width is negative, the function copies everything to the right
    border of the window. If \a height is negative, the function
    copies everything to the bottom of the window.

    The window system identifier (\c WId) can be retrieved using the
    QWidget::winId() function. The rationale for using a window
    identifier and not a QWidget, is to enable grabbing of windows
    that are not part of the application, window system frames, and so
    on.

    The grabWindow() function grabs pixels from the screen, not from
    the window, i.e. if there is another window partially or entirely
    over the one you grab, you get pixels from the overlying window,
    too. The mouse cursor is generally not grabbed.

    Note on X11 that if the given \a window doesn't have the same depth
    as the root window, and another window partially or entirely
    obscures the one you grab, you will \e not get pixels from the
    overlying window.  The contents of the obscured areas in the
    pixmap will be undefined and uninitialized.

    On Windows Vista and above grabbing a layered window, which is
    created by setting the Qt::WA_TranslucentBackground attribute, will
    not work. Instead grabbing the desktop widget should work.

    \warning In general, grabbing an area outside the screen is not
    safe. This depends on the underlying window system.
*/

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



