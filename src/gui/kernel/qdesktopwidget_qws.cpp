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
#include <qscreen_qws.h>
#include <qapplication_p.h>

QT_BEGIN_NAMESPACE

QT_USE_NAMESPACE

QDesktopWidget::QDesktopWidget()
   : QWidget(0, Qt::Desktop)
{
   setObjectName(QLatin1String("desktop"));
}

QDesktopWidget::~QDesktopWidget()
{
}

bool QDesktopWidget::isVirtualDesktop() const
{
   return true;
}

int QDesktopWidget::primaryScreen() const
{
   return 0;
}

int QDesktopWidget::numScreens() const
{
   QScreen *screen = QScreen::instance();
   if (!screen) {
      return 0;
   }

   const QList<QScreen *> subScreens = screen->subScreens();
   return qMax(subScreens.size(), 1);
}

QWidget *QDesktopWidget::screen(int)
{
   return this;
}

const QRect QDesktopWidget::availableGeometry(int screenNo) const
{
   const QScreen *screen = QScreen::instance();
   if (screenNo == -1) {
      screenNo = 0;
   }
   if (!screen || screenNo < 0) {
      return QRect();
   }

   const QList<QScreen *> subScreens = screen->subScreens();
   if (!subScreens.isEmpty()) {
      if (screenNo >= subScreens.size()) {
         return QRect();
      }
      screen = subScreens.at(screenNo);
   }

   QApplicationPrivate *ap = QApplicationPrivate::instance();
   const QRect r = ap->maxWindowRect(screen);
   if (!r.isEmpty()) {
      return r;
   }

   return screen->region().boundingRect();
}

const QRect QDesktopWidget::screenGeometry(int screenNo) const
{
   const QScreen *screen = QScreen::instance();
   if (screenNo == -1) {
      screenNo = 0;
   }
   if (!screen || screenNo < 0) {
      return QRect();
   }

   const QList<QScreen *> subScreens = screen->subScreens();
   if (subScreens.size() == 0 && screenNo == 0) {
      return screen->region().boundingRect();
   }

   if (screenNo >= subScreens.size()) {
      return QRect();
   }

   return subScreens.at(screenNo)->region().boundingRect();
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
   const QScreen *screen = QScreen::instance();
   if (!screen || !screen->region().contains(p)) {
      return -1;
   }

   const QList<QScreen *> subScreens = screen->subScreens();
   if (subScreens.size() == 0) {
      return 0;
   }

   for (int i = 0; i < subScreens.size(); ++i)
      if (subScreens.at(i)->region().contains(p)) {
         return i;
      }

   return -1;
}

void QDesktopWidget::resizeEvent(QResizeEvent *)
{
}

QT_END_NAMESPACE
