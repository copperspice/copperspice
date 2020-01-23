/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#include <qplatform_window.h>
#include <qplatform_window_p.h>

#include <qguiapplication_p.h>
#include <qplatform_screen.h>
#include <qwindowsysteminterface.h>
#include <qwindow.h>
#include <qscreen.h>

#include <qhighdpiscaling_p.h>
#include <qwindow_p.h>

QPlatformWindow::QPlatformWindow(QWindow *window)
   : QPlatformSurface(window), d_ptr(new QPlatformWindowPrivate)
{
   Q_D(QPlatformWindow);
   d->rect = window->geometry();
}

QPlatformWindow::~QPlatformWindow()
{
}

QWindow *QPlatformWindow::window() const
{
   return static_cast<QWindow *>(m_surface);
}

QPlatformWindow *QPlatformWindow::parent() const
{
   return window()->parent() ? window()->parent()->handle() : 0;
}

QPlatformScreen *QPlatformWindow::screen() const
{
   QScreen *scr = window()->screen();
   return scr ? scr->handle() : nullptr;
}

QSurfaceFormat QPlatformWindow::format() const
{
   return QSurfaceFormat();
}
void QPlatformWindow::setGeometry(const QRect &rect)
{
   Q_D(QPlatformWindow);
   d->rect = rect;
}

QRect QPlatformWindow::geometry() const
{
   Q_D(const QPlatformWindow);
   return d->rect;
}

QRect QPlatformWindow::normalGeometry() const
{
   return QRect();
}
QMargins QPlatformWindow::frameMargins() const
{
   return QMargins();
}
void QPlatformWindow::setVisible(bool visible)
{

   QRect rect(QPoint(), geometry().size());
   QWindowSystemInterface::handleExposeEvent(window(), rect);
   QWindowSystemInterface::flushWindowSystemEvents();
}

void QPlatformWindow::setWindowFlags(Qt::WindowFlags flags)
{
}
bool QPlatformWindow::isExposed() const
{
   return window()->isVisible();
}
bool QPlatformWindow::isActive() const
{
   return false;
}

bool QPlatformWindow::isEmbedded(const QPlatformWindow *parentWindow) const
{
   return false;
}

QPoint QPlatformWindow::mapToGlobal(const QPoint &pos) const
{
   const QPlatformWindow *p = this;
   QPoint result = pos;
   while (p) {
      result += p->geometry().topLeft();
      p = p->parent();
   }
   return result;
}

QPoint QPlatformWindow::mapFromGlobal(const QPoint &pos) const
{
   const QPlatformWindow *p = this;
   QPoint result = pos;
   while (p) {
      result -= p->geometry().topLeft();
      p = p->parent();
   }
   return result;
}

void QPlatformWindow::setWindowState(Qt::WindowState)
{
}

WId QPlatformWindow::winId() const
{
   // Return anything but 0. Returning 0 would cause havoc with QWidgets on
   // very basic platform plugins that do not reimplement this function,
   // because the top-level widget's internalWinId() would always be 0 which
   // would mean top-levels are never treated as native.
   return WId(1);
}

void QPlatformWindow::setParent(const QPlatformWindow *parent)
{
   qWarning("This plugin does not support setParent");
}

void QPlatformWindow::setWindowTitle(const QString &title)
{
}

void QPlatformWindow::setWindowFilePath(const QString &filePath)
{
}

void QPlatformWindow::setWindowIcon(const QIcon &icon)
{
}

void QPlatformWindow::raise()
{
   qWarning("This plugin does not support raise()");
}
void QPlatformWindow::lower()
{
   qWarning("This plugin does not support lower()");
}

void QPlatformWindow::propagateSizeHints()
{
   qWarning("This plugin does not support propagateSizeHints()");
}

void QPlatformWindow::setOpacity(qreal level)
{
   qWarning("This plugin does not support setting window opacity");
}

void QPlatformWindow::setMask(const QRegion &region)
{

   qWarning("This plugin does not support setting window masks");
}

void QPlatformWindow::requestActivateWindow()
{
   QWindowSystemInterface::handleWindowActivated(window());
}

void QPlatformWindow::handleContentOrientationChange(Qt::ScreenOrientation orientation)
{
}
qreal QPlatformWindow::devicePixelRatio() const
{
   return 1.0;
}

bool QPlatformWindow::setKeyboardGrabEnabled(bool grab)
{
   qWarning("This plugin does not support grabbing the keyboard");
   return false;
}

bool QPlatformWindow::setMouseGrabEnabled(bool grab)
{
   qWarning("This plugin does not support grabbing the mouse");
   return false;
}

bool QPlatformWindow::setWindowModified(bool modified)
{
   return false;
}
void QPlatformWindow::windowEvent(QEvent *event)
{

}
bool QPlatformWindow::startSystemResize(const QPoint &pos, Qt::Corner corner)
{

   return false;
}
void QPlatformWindow::setFrameStrutEventsEnabled(bool enabled)
{
   Q_UNUSED(enabled) // Do not warn as widgets enable it by default causing warnings with XCB.
}
bool QPlatformWindow::frameStrutEventsEnabled() const
{
   return false;
}
QString QPlatformWindow::formatWindowTitle(const QString &title, const QString &separator)
{
   QString fullTitle = title;

   if (QGuiApplicationPrivate::displayName && !title.endsWith(*QGuiApplicationPrivate::displayName)) {
      if (!fullTitle.isEmpty()) {
         fullTitle += separator;
      }
      fullTitle += *QGuiApplicationPrivate::displayName;

   } else if (fullTitle.isEmpty()) {
      fullTitle = QCoreApplication::applicationName();
   }

   return fullTitle;
}

QPlatformScreen *QPlatformWindow::screenForGeometry(const QRect &newGeometry) const
{
   QPlatformScreen *currentScreen = screen();
   QPlatformScreen *fallback = currentScreen;
   QPoint center = newGeometry.isEmpty() ? newGeometry.topLeft() : newGeometry.center();

   if (window()->type() == Qt::ForeignWindow) {
      center = mapToGlobal(center - newGeometry.topLeft());
   }

   if (!parent() && currentScreen && !currentScreen->geometry().contains(center)) {
      for  (QPlatformScreen *screen : currentScreen->virtualSiblings()) {
         if (screen->geometry().contains(center)) {
            return screen;
         }

         if (screen->geometry().intersects(newGeometry)) {
            fallback = screen;
         }
      }
   }
   return fallback;
}
QSize QPlatformWindow::constrainWindowSize(const QSize &size)
{
   return size.expandedTo(QSize(0, 0)).boundedTo(QSize(QWINDOWSIZE_MAX, QWINDOWSIZE_MAX));
}
void QPlatformWindow::setAlertState(bool enable)
{
   Q_UNUSED(enable)
}
bool QPlatformWindow::isAlertState() const
{
   return false;
}
static inline const QScreen *effectiveScreen(const QWindow *window)
{
   if (!window) {
      return QGuiApplication::primaryScreen();
   }

   const QScreen *screen = window->screen();
   if (!screen) {
      return QGuiApplication::primaryScreen();
   }

   const QList<QScreen *> siblings = screen->virtualSiblings();

#ifndef QT_NO_CURSOR
   if (siblings.size() > 1) {
      const QPoint referencePoint = window->transientParent() ? window->transientParent()->geometry().center() : QCursor::pos();
      for (const QScreen *sibling : siblings)
         if (sibling->geometry().contains(referencePoint)) {
            return sibling;
         }
   }
#endif
   return screen;
}
void QPlatformWindow::invalidateSurface()
{
}
static QSize fixInitialSize(QSize size, const QWindow *w,
   int defaultWidth, int defaultHeight)
{
   if (size.width() == 0) {
      const int minWidth = w->minimumWidth();
      size.setWidth(minWidth > 0 ? minWidth : defaultWidth);
   }
   if (size.height() == 0) {
      const int minHeight = w->minimumHeight();
      size.setHeight(minHeight > 0 ? minHeight : defaultHeight);
   }
   return size;
}

QRect QPlatformWindow::initialGeometry(const QWindow *w,
   const QRect &initialGeometry, int defaultWidth, int defaultHeight)
{
   if (!w->isTopLevel()) {
      const qreal factor = QHighDpiScaling::factor(w);
      const QSize size = fixInitialSize(QHighDpi::fromNative(initialGeometry.size(), factor),
            w, defaultWidth, defaultHeight);
      return QRect(initialGeometry.topLeft(), QHighDpi::toNative(size, factor));
   }
   const QScreen *screen = effectiveScreen(w);
   if (!screen) {
      return initialGeometry;
   }
   QRect rect(QHighDpi::fromNativePixels(initialGeometry, w));
   rect.setSize(fixInitialSize(rect.size(), w, defaultWidth, defaultHeight));
   if (qt_window_private(const_cast<QWindow *>(w))->positionAutomatic
      && w->type() != Qt::Popup) {
      const QRect availableGeometry = screen->availableGeometry();
      if (rect.height() < (availableGeometry.height() * 8) / 9
         && rect.width() < (availableGeometry.width() * 8) / 9) {
         const QWindow *tp = w->transientParent();
         if (tp) {
            rect.moveCenter(tp->geometry().center());
         } else {
            rect.moveCenter(availableGeometry.center());
         }
      }
   }
   return QHighDpi::toNativePixels(rect, screen);
}

void QPlatformWindow::requestUpdate()
{
   static int timeout = -1;

   if (timeout == -1) {
      bool ok = false;
      timeout = qgetenv("QT_QPA_UPDATE_IDLE_TIME").toInt(&ok);

      if (! ok) {
         timeout = 5;
      }
   }

   QWindow *obj = window();

   // emerald - obj->startTimer(timeout, Qt::PreciseTimer);

   int updateTimer = obj->startTimer(timeout);
   obj->cs_internal_updateTimer(updateTimer);
}

QSize QPlatformWindow::windowMinimumSize() const
{
   return constrainWindowSize(QHighDpi::toNativePixels(window()->minimumSize(), window()));
}

QSize QPlatformWindow::windowMaximumSize() const
{
   return constrainWindowSize(QHighDpi::toNativePixels(window()->maximumSize(), window()));
}

QSize QPlatformWindow::windowBaseSize() const
{
   return QHighDpi::toNativePixels(window()->baseSize(), window());
}

QSize QPlatformWindow::windowSizeIncrement() const
{
   QSize increment = window()->sizeIncrement();
   if (!QHighDpiScaling::isActive()) {
      return increment;
   }
   if (increment.isEmpty()) {
      increment = QSize(1, 1);
   }
   return QHighDpi::toNativePixels(increment, window());
}

QRect QPlatformWindow::windowGeometry() const
{
   return QHighDpi::toNativePixels(window()->geometry(), window());
}
QRect QPlatformWindow::windowFrameGeometry() const
{
   return QHighDpi::toNativePixels(window()->frameGeometry(), window());
}
QRectF QPlatformWindow::closestAcceptableGeometry(const QWindow *qWindow, const QRectF &nativeRect)
{
   const QRectF rectF = QHighDpi::fromNativePixels(nativeRect, qWindow);
   const QRectF correctedGeometryF = qt_window_private(const_cast<QWindow *>(qWindow))->closestAcceptableGeometry(rectF);
   return !correctedGeometryF.isEmpty() && rectF != correctedGeometryF
      ? QHighDpi::toNativePixels(correctedGeometryF, qWindow) : nativeRect;
}
QRectF QPlatformWindow::windowClosestAcceptableGeometry(const QRectF &nativeRect) const
{
   return QPlatformWindow::closestAcceptableGeometry(window(), nativeRect);
}



