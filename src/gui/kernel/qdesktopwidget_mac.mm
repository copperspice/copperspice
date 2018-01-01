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

/***********************************************************************
** Copyright (C) 2007-2008, Apple, Inc.
***********************************************************************/

#import <Cocoa/Cocoa.h>

#include <qapplication.h>
#include <qdesktopwidget.h>
#include <qt_mac_p.h>
#include <qwidget_p.h>
#include <qt_cocoa_helpers_mac_p.h>
#include <qdesktopwidget_mac_p.h>

QT_BEGIN_NAMESPACE

QT_USE_NAMESPACE

Q_GLOBAL_STATIC(QDesktopWidgetImplementation, qdesktopWidgetImplementation)

QDesktopWidgetImplementation::QDesktopWidgetImplementation()
   : appScreen(0)
{
   onResize();
}

QDesktopWidgetImplementation::~QDesktopWidgetImplementation()
{
}

QDesktopWidgetImplementation *QDesktopWidgetImplementation::instance()
{
   return qdesktopWidgetImplementation();
}

QRect QDesktopWidgetImplementation::availableRect(int screenIndex) const
{
   if (screenIndex < 0 || screenIndex >= screenCount) {
      screenIndex = appScreen;
   }

   return availableRects[screenIndex].toRect();
}

QRect QDesktopWidgetImplementation::screenRect(int screenIndex) const
{
   if (screenIndex < 0 || screenIndex >= screenCount) {
      screenIndex = appScreen;
   }

   return screenRects[screenIndex].toRect();
}

void QDesktopWidgetImplementation::onResize()
{
   QMacCocoaAutoReleasePool pool;
   NSArray *displays = [NSScreen screens];
   screenCount = [displays count];

   screenRects.clear();
   availableRects.clear();
   NSRect primaryRect = [[displays objectAtIndex: 0] frame];
   for (int i = 0; i < screenCount; i++) {
      NSRect r = [[displays objectAtIndex: i] frame];
      int flippedY = - r.origin.y +                  // account for position offset and
                     primaryRect.size.height - r.size.height; // height difference.
      screenRects.append(QRectF(r.origin.x, flippedY,
                                r.size.width, r.size.height));

      r = [[displays objectAtIndex: i] visibleFrame];
      flippedY = - r.origin.y +                      // account for position offset and
                 primaryRect.size.height - r.size.height; // height difference.
      availableRects.append(QRectF(r.origin.x, flippedY,
                                   r.size.width, r.size.height));
   }
}



QDesktopWidget::QDesktopWidget()
   : QWidget(0, Qt::Desktop)
{
   setObjectName(QLatin1String("desktop"));
   setAttribute(Qt::WA_WState_Visible);
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
   return qdesktopWidgetImplementation()->appScreen;
}

int QDesktopWidget::numScreens() const
{
   return qdesktopWidgetImplementation()->screenCount;
}

QWidget *QDesktopWidget::screen(int)
{
   return this;
}

const QRect QDesktopWidget::availableGeometry(int screen) const
{
   return qdesktopWidgetImplementation()->availableRect(screen);
}

const QRect QDesktopWidget::screenGeometry(int screen) const
{
   return qdesktopWidgetImplementation()->screenRect(screen);
}

int QDesktopWidget::screenNumber(const QWidget *widget) const
{
   QDesktopWidgetImplementation *d = qdesktopWidgetImplementation();
   if (!widget) {
      return d->appScreen;
   }
   QRect frame = widget->frameGeometry();
   if (!widget->isWindow()) {
      frame.moveTopLeft(widget->mapToGlobal(QPoint(0, 0)));
   }
   int maxSize = -1, maxScreen = -1;
   for (int i = 0; i < d->screenCount; ++i) {
      QRect rr = d->screenRect(i);
      QRect sect = rr.intersected(frame);
      int size = sect.width() * sect.height();
      if (size > maxSize && sect.width() > 0 && sect.height() > 0) {
         maxSize = size;
         maxScreen = i;
      }
   }
   return maxScreen;
}

int QDesktopWidget::screenNumber(const QPoint &point) const
{
   QDesktopWidgetImplementation *d = qdesktopWidgetImplementation();
   int closestScreen = -1;
   int shortestDistance = INT_MAX;
   for (int i = 0; i < d->screenCount; ++i) {
      QRect rr = d->screenRect(i);
      int thisDistance = QWidgetPrivate::pointToRect(point, rr);
      if (thisDistance < shortestDistance) {
         shortestDistance = thisDistance;
         closestScreen = i;
      }
   }
   return closestScreen;
}

void QDesktopWidget::resizeEvent(QResizeEvent *)
{
   QDesktopWidgetImplementation *d = qdesktopWidgetImplementation();

   const int oldScreenCount = d->screenCount;
   const QVector<QRectF> oldRects(d->screenRects);
   const QVector<QRectF> oldWorks(d->availableRects);

   d->onResize();

   for (int i = 0; i < qMin(oldScreenCount, d->screenCount); ++i) {
      if (oldRects.at(i) != d->screenRects.at(i)) {
         emit resized(i);
      }
   }
   for (int i = 0; i < qMin(oldScreenCount, d->screenCount); ++i) {
      if (oldWorks.at(i) != d->availableRects.at(i)) {
         emit workAreaResized(i);
      }
   }

   if (oldScreenCount != d->screenCount) {
      emit screenCountChanged(d->screenCount);
   }
}

QT_END_NAMESPACE
