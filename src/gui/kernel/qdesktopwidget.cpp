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

#include <qglobal.h>
#include <qdesktopwidget.h>
#include <qdesktopwidget_p.h>
#include <qscreen.h>
#include <qwidget_p.h>
#include <qwindow.h>

#include <qhighdpiscaling_p.h>
#include <qplatform_screen.h>

QDesktopScreenWidget::QDesktopScreenWidget(QScreen *screen, const QRect &geometry)
   : QWidget(nullptr, Qt::Desktop), m_screen(screen)
{
   setVisible(false);
   if (QWindow *winHandle = windowHandle()) {
      winHandle->setScreen(screen);
   }
   setScreenGeometry(geometry);
}

void QDesktopScreenWidget::setScreenGeometry(const QRect &geometry)
{
   m_geometry = geometry;
   setGeometry(geometry);
}

int QDesktopScreenWidget::screenNumber() const
{
   const QDesktopWidgetPrivate *desktopWidgetP
      = static_cast<const QDesktopWidgetPrivate *>(qt_widget_private(QApplication::desktop()));
   return desktopWidgetP->screens.indexOf(const_cast<QDesktopScreenWidget *>(this));
}

const QRect QDesktopWidget::screenGeometry(const QWidget *widget) const
{
   if (!widget) {
      qWarning("QDesktopWidget::screenGeometry() Unable to retrieve screen geometry for an invalid widget (nullptr)");
      return QRect();
   }

   QRect rect = QWidgetPrivate::screenGeometry(widget);
   if (rect.isNull()) {
      return screenGeometry(screenNumber(widget));
   } else {
      return rect;
   }
}

const QRect QDesktopWidget::availableGeometry(const QWidget *widget) const
{
   if (!widget) {
      qWarning("QDesktopWidget::availableGeometry() Unable to retrieve screen geometry for an invalid widget (nullptr)");

      return QRect();
   }

   QRect rect = QWidgetPrivate::screenGeometry(widget);

   if (rect.isNull()) {
      return availableGeometry(screenNumber(widget));
   } else {
      return rect;
   }
}

QDesktopScreenWidget *QDesktopWidgetPrivate::widgetForScreen(QScreen *qScreen) const
{
   for (QDesktopScreenWidget *widget : screens) {
      if (widget->screen() == qScreen) {
         return widget;
      }
   }

   return nullptr;
}

void QDesktopWidgetPrivate::_q_updateScreens()
{
   Q_Q(QDesktopWidget);

   const QList<QScreen *> screenList = QGuiApplication::screens();
   const int targetLength = screenList.length();
   bool screenCountChanged = false;

   // Re-build our screens list. This is the easiest way to later compute which signals to emit.
   // Create new screen widgets as necessary. While iterating, keep the old list in place so
   // that widgetForScreen works.
   // Furthermore, we note which screens have changed, and compute the overall virtual geometry.

   QList<QDesktopScreenWidget *> newScreens;
   QList<int> changedScreens;
   QRegion virtualGeometry;

   for (int i = 0; i < targetLength; ++i) {
      QScreen *qScreen = screenList.at(i);
      const QRect screenGeometry = qScreen->geometry();

      QDesktopScreenWidget *screenWidget = widgetForScreen(qScreen);

      if (screenWidget) {
         // an old screen. update geometry and remember the index in the *new* list
         if (screenGeometry != screenWidget->screenGeometry()) {
            screenWidget->setScreenGeometry(screenGeometry);
            changedScreens.push_back(i);
         }
      } else {
         // a new screen, create a widget and connect the signals.
         screenWidget = new QDesktopScreenWidget(qScreen, screenGeometry);

         QObject::connect(qScreen, &QScreen::geometryChanged, q, &QDesktopWidget::_q_updateScreens, Qt::QueuedConnection);
         QObject::connect(qScreen, &QScreen::availableGeometryChanged, q, &QDesktopWidget::_q_availableGeometryChanged, Qt::QueuedConnection);
         QObject::connect(qScreen, &QScreen::destroyed, q, &QDesktopWidget::_q_updateScreens, Qt::QueuedConnection);

         screenCountChanged = true;
      }

      // record all the screens and the overall geometry.
      newScreens.push_back(screenWidget);
      virtualGeometry += screenGeometry;
   }

   // Now we apply the accumulated updates.
   screens.swap(newScreens); // now [newScreens] is the old screen list
   Q_ASSERT(screens.size() == targetLength);

   q->setGeometry(virtualGeometry.boundingRect());

   // Delete the QDesktopScreenWidget that are not used any more.
   for (QDesktopScreenWidget *screen : newScreens) {
      if (!screens.contains(screen)) {
         delete screen;
         screenCountChanged = true;
      }
   }

   // Finally, emit the signals.
   if (screenCountChanged) {
      // Notice that we trigger screenCountChanged even if a screen was removed and another one added,
      // in which case the total number of screens did not change. This is the only way for applications
      // to notice that a screen was swapped out against another one.
      emit q->screenCountChanged(targetLength);
   }

   for (int changedScreen : changedScreens) {
      emit q->resized(changedScreen);
   }
}

void QDesktopWidgetPrivate::_q_availableGeometryChanged()
{
   Q_Q(QDesktopWidget);

   if (QScreen *screen = dynamic_cast<QScreen *>(q->sender())) {
      emit q->workAreaResized(QGuiApplication::screens().indexOf(screen));
   }
}

QDesktopWidget::QDesktopWidget()
   : QWidget(*new QDesktopWidgetPrivate, nullptr, Qt::Desktop)
{
   Q_D(QDesktopWidget);

   setObjectName(QLatin1String("desktop"));
   d->_q_updateScreens();

   connect(qApp, &QApplication::screenAdded,          this, &QDesktopWidget::_q_updateScreens);
   connect(qApp, &QApplication::primaryScreenChanged, this, &QDesktopWidget::primaryScreenChanged);
}

QDesktopWidget::~QDesktopWidget()
{
}

bool QDesktopWidget::isVirtualDesktop() const
{
   return QGuiApplication::primaryScreen()->virtualSiblings().size() > 1;
}

int QDesktopWidget::primaryScreen() const
{
   return 0;
}

int QDesktopWidget::numScreens() const
{
   return qMax(QGuiApplication::screens().size(), 1);
}

QWidget *QDesktopWidget::screen(int screen)
{
   Q_D(QDesktopWidget);
   if (screen < 0 || screen >= d->screens.length()) {
      return d->screens.at(0);
   }
   return d->screens.at(screen);
}
const QRect QDesktopWidget::availableGeometry(int screenNo) const
{
   QList<QScreen *> screens = QGuiApplication::screens();
   if (screenNo == -1) {
      screenNo = 0;
   }
   if (screenNo < 0 || screenNo >= screens.size()) {
      return QRect();
   } else {
      return screens.at(screenNo)->availableGeometry();
   }
}

const QRect QDesktopWidget::screenGeometry(int screenNo) const
{
   QList<QScreen *> screens = QGuiApplication::screens();
   if (screenNo == -1) {
      screenNo = 0;
   }
   if (screenNo < 0 || screenNo >= screens.size()) {
      return QRect();
   } else {
      return screens.at(screenNo)->geometry();
   }
}
int QDesktopWidget::screenNumber(const QWidget *w) const
{
   if (!w) {
      return primaryScreen();
   }

   const QList<QScreen *> allScreens = QGuiApplication::screens();
   QList<QScreen *> screens = allScreens;
   if (screens.isEmpty()) { // This should never happen
      return primaryScreen();
   }

   const QWindow *winHandle = w->windowHandle();
   if (!winHandle) {
      if (const QWidget *nativeParent = w->nativeParentWidget()) {
         winHandle = nativeParent->windowHandle();
      }
   }

   // If there is more than one virtual desktop
   if (screens.count() != screens.constFirst()->virtualSiblings().count()) {
      // Find the root widget, get a QScreen from it and use the
      // virtual siblings for checking the window position.
      if (winHandle) {
         if (const QScreen *winScreen = winHandle->screen()) {
            screens = winScreen->virtualSiblings();
         }
      }
   }

   // Get the screen number from window position using screen geometry
   // and proper screens.
   QRect frame = w->frameGeometry();
   if (!w->isWindow()) {
      frame.moveTopLeft(w->mapToGlobal(QPoint(0, 0)));
   }

   QScreen *widgetScreen = nullptr;
   int largestArea = 0;

   for (QScreen *screen : screens) {
      const QRect deviceIndependentScreenGeometry = QHighDpi::fromNativePixels(screen->handle()->geometry(), screen);
      const QRect intersected = deviceIndependentScreenGeometry.intersected(frame);

      int area = intersected.width() * intersected.height();
      if (largestArea < area) {
         widgetScreen = screen;
         largestArea = area;
      }
   }

   return allScreens.indexOf(widgetScreen);
}

int QDesktopWidget::screenNumber(const QPoint &p) const
{
   const QList<QScreen *> screens = QGuiApplication::screens();
   if (! screens.isEmpty()) {
      const QList<QScreen *> primaryScreens = screens.first()->virtualSiblings();

      // Find the screen index on the primary virtual desktop first
      for (QScreen *screen : primaryScreens) {
         if (screen->geometry().contains(p)) {
            return screens.indexOf(screen);
         }
      }

      // If the screen index is not found on primary virtual desktop, find
      // the screen index on all screens except the first which was for
      // sure in the previous loop. Some other screens may repeat. Find
      // only when there is more than one virtual desktop.
      if (screens.count() != primaryScreens.count()) {
         for (int i = 1; i < screens.size(); ++i) {
            if (screens[i]->geometry().contains(p)) {
               return i;
            }
         }
      }
   }
   return primaryScreen(); //even better would be closest screen
}

void QDesktopWidget::resizeEvent(QResizeEvent *)
{
}

void QDesktopWidget::_q_updateScreens()
{
   Q_D(QDesktopWidget);
   d->_q_updateScreens();
}

void QDesktopWidget::_q_availableGeometryChanged()
{
   Q_D(QDesktopWidget);
   d->_q_availableGeometryChanged();
}
