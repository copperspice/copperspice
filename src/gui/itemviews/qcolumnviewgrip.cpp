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

#ifndef QT_NO_QCOLUMNVIEW

#include <qcolumnviewgrip_p.h>

#include <qbrush.h>
#include <qdebug.h>
#include <qevent.h>
#include <qpainter.h>
#include <qstyleoption.h>

QColumnViewGrip::QColumnViewGrip(QWidget *parent)
   :  QWidget(*new QColumnViewGripPrivate, parent, Qt::EmptyFlag)
{
#ifndef QT_NO_CURSOR
   setCursor(Qt::SplitHCursor);
#endif
}

// internal
QColumnViewGrip::QColumnViewGrip(QColumnViewGripPrivate &dd, QWidget *parent, Qt::WindowFlags flags)
   :  QWidget(dd, parent, flags)
{
}

QColumnViewGrip::~QColumnViewGrip()
{
}

int QColumnViewGrip::moveGrip(int offset)
{
   QWidget *parentWidget = (QWidget *)parent();

   // first resize the parent
   int oldWidth = parentWidget->width();
   int newWidth = oldWidth;

   if (isRightToLeft()) {
      newWidth -= offset;
   } else {
      newWidth += offset;
   }
   newWidth = qMax(parentWidget->minimumWidth(), newWidth);
   parentWidget->resize(newWidth, parentWidget->height());

   // Then have the view move the widget
   int realOffset = parentWidget->width() - oldWidth;
   int oldX = parentWidget->x();
   if (realOffset != 0) {
      emit gripMoved(realOffset);
   }

   if (isRightToLeft()) {
      realOffset = -1 * (oldX - parentWidget->x());
   }

   return realOffset;
}

void QColumnViewGrip::paintEvent(QPaintEvent *event)
{
   QPainter painter(this);
   QStyleOption opt;
   opt.initFrom(this);
   style()->drawControl(QStyle::CE_ColumnViewGrip, &opt, &painter, this);
   event->accept();
}

void QColumnViewGrip::mouseDoubleClickEvent(QMouseEvent *event)
{
   (void) event;
   QWidget *parentWidget = (QWidget *)parent();
   int offset = parentWidget->sizeHint().width() - parentWidget->width();
   if (isRightToLeft()) {
      offset *= -1;
   }
   moveGrip(offset);
   event->accept();
}

void QColumnViewGrip::mousePressEvent(QMouseEvent *event)
{
   Q_D(QColumnViewGrip);
   d->originalXLocation = event->globalX();
   event->accept();
}

void QColumnViewGrip::mouseMoveEvent(QMouseEvent *event)
{
   Q_D(QColumnViewGrip);
   int offset = event->globalX() - d->originalXLocation;
   d->originalXLocation = moveGrip(offset) + d->originalXLocation;
   event->accept();
}

void QColumnViewGrip::mouseReleaseEvent(QMouseEvent *event)
{
   Q_D(QColumnViewGrip);
   d->originalXLocation = -1;
   event->accept();
}
QColumnViewGripPrivate::QColumnViewGripPrivate()
   :  QWidgetPrivate(),
      originalXLocation(-1)
{
}

#endif // QT_NO_QCOLUMNVIEW
