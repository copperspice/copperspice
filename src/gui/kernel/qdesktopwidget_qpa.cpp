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

#include <qdesktopwidget.h>
#include <qapplication_p.h>
#include <qgraphicssystem_p.h>
#include <QWidget>
#include <qwidget_p.h>
#include <qdesktopwidget_qpa_p.h>
QT_BEGIN_NAMESPACE

QT_USE_NAMESPACE

void QDesktopWidgetPrivate::updateScreenList()
{
   Q_Q(QDesktopWidget);

   QList<QPlatformScreen *> screenList = QApplicationPrivate::platformIntegration()->screens();
   int targetLength = screenList.length();
   int currentLength = screens.length();

   // Add or remove screen widgets as necessary
   if (currentLength > targetLength) {
      QDesktopScreenWidget *screen;
      while (currentLength-- > targetLength) {
         screen = screens.takeLast();
         delete screen;
      }
   } else if (currentLength < targetLength) {
      QDesktopScreenWidget *screen;
      while (currentLength < targetLength) {
         screen = new QDesktopScreenWidget(currentLength++);
         screens.append(screen);
      }
   }

   QRegion virtualGeometry;

   // update the geometry of each screen widget
   for (int i = 0; i < screens.length(); i++) {
      QRect screenGeometry = screenList.at(i)->geometry();
      screens.at(i)->setGeometry(screenGeometry);
      virtualGeometry += screenGeometry;
   }

   q->setGeometry(virtualGeometry.boundingRect());
}

QDesktopWidget::QDesktopWidget()
   : QWidget(*new QDesktopWidgetPrivate, 0, Qt::Desktop)
{
   Q_D(QDesktopWidget);
   setObjectName(QLatin1String("desktop"));
   d->updateScreenList();
}

QDesktopWidget::~QDesktopWidget()
{
}

bool QDesktopWidget::isVirtualDesktop() const
{
   return QApplicationPrivate::platformIntegration()->isVirtualDesktop();
}

int QDesktopWidget::primaryScreen() const
{
   return 0;
}

int QDesktopWidget::numScreens() const
{
   QPlatformIntegration *pi = QApplicationPrivate::platformIntegration();
   return qMax(pi->screens().size(), 1);
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
   QPlatformIntegration *pi = QApplicationPrivate::platformIntegration();
   QList<QPlatformScreen *> screens = pi->screens();
   if (screenNo == -1) {
      screenNo = 0;
   }
   if (screenNo < 0 || screenNo >= screens.size()) {
      return QRect();
   } else {
      return screens[screenNo]->availableGeometry();
   }
}

const QRect QDesktopWidget::screenGeometry(int screenNo) const
{
   QPlatformIntegration *pi = QApplicationPrivate::platformIntegration();
   QList<QPlatformScreen *> screens = pi->screens();
   if (screenNo == -1) {
      screenNo = 0;
   }
   if (screenNo < 0 || screenNo >= screens.size()) {
      return QRect();
   } else {
      return screens[screenNo]->geometry();
   }
}

int QDesktopWidget::screenNumber(const QWidget *w) const
{
   if (!w) {
      return 0;
   }

   QRect frame = w->frameGeometry();
   if (!w->isWindow()) {
      frame.moveTopLeft(w->mapToGlobal(QPoint(0, 0)));
   }
   const QPoint midpoint = (frame.topLeft() + frame.bottomRight()) / 2;
   return screenNumber(midpoint);
}

int QDesktopWidget::screenNumber(const QPoint &p) const
{
   QPlatformIntegration *pi = QApplicationPrivate::platformIntegration();
   QList<QPlatformScreen *> screens = pi->screens();

   for (int i = 0; i < screens.size(); ++i)
      if (screens[i]->geometry().contains(p)) {
         return i;
      }

   return primaryScreen(); //even better would be closest screen
}

void QDesktopWidget::resizeEvent(QResizeEvent *)
{
}

QT_END_NAMESPACE
