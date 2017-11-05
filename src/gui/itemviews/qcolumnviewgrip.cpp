/***********************************************************************
*
* Copyright (c) 2012-2017 Barbara Geller
* Copyright (c) 2012-2017 Ansel Sermersheim
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

#ifndef QT_NO_QCOLUMNVIEW

#include <qcolumnviewgrip_p.h>
#include <qstyleoption.h>
#include <qpainter.h>
#include <qbrush.h>
#include <qevent.h>
#include <qdebug.h>

QT_BEGIN_NAMESPACE

QColumnViewGrip::QColumnViewGrip(QWidget *parent)
   :  QWidget(*new QColumnViewGripPrivate, parent, 0)
{
#ifndef QT_NO_CURSOR
   setCursor(Qt::SplitHCursor);
#endif
}

/*!
  \internal
*/
QColumnViewGrip::QColumnViewGrip(QColumnViewGripPrivate &dd, QWidget *parent, Qt::WindowFlags f)
   :  QWidget(dd, parent, f)
{
}

/*!
  Destroys the view.
*/
QColumnViewGrip::~QColumnViewGrip()
{
}

/*!
    Attempt to resize the parent object by \a offset
    returns the amount of offset that it was actually able to resized
*/
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

/*!
    \reimp
*/
void QColumnViewGrip::paintEvent(QPaintEvent *event)
{
   QPainter painter(this);
   QStyleOption opt;
   opt.initFrom(this);
   style()->drawControl(QStyle::CE_ColumnViewGrip, &opt, &painter, this);
   event->accept();
}

/*!
    \reimp
    Resize the parent window to the sizeHint
*/
void QColumnViewGrip::mouseDoubleClickEvent(QMouseEvent *event)
{
   Q_UNUSED(event);
   QWidget *parentWidget = (QWidget *)parent();
   int offset = parentWidget->sizeHint().width() - parentWidget->width();
   if (isRightToLeft()) {
      offset *= -1;
   }
   moveGrip(offset);
   event->accept();
}

/*!
    \reimp
    Begin watching for mouse movements
*/
void QColumnViewGrip::mousePressEvent(QMouseEvent *event)
{
   Q_D(QColumnViewGrip);
   d->originalXLocation = event->globalX();
   event->accept();
}

/*!
    \reimp
    Calculate the movement of the grip and moveGrip() and emit gripMoved
*/
void QColumnViewGrip::mouseMoveEvent(QMouseEvent *event)
{
   Q_D(QColumnViewGrip);
   int offset = event->globalX() - d->originalXLocation;
   d->originalXLocation = moveGrip(offset) + d->originalXLocation;
   event->accept();
}

/*!
    \reimp
    Stop watching for mouse movements
*/
void QColumnViewGrip::mouseReleaseEvent(QMouseEvent *event)
{
   Q_D(QColumnViewGrip);
   d->originalXLocation = -1;
   event->accept();
}

/*
 * private object implementation
 */
QColumnViewGripPrivate::QColumnViewGripPrivate()
   :  QWidgetPrivate(),
      originalXLocation(-1)
{
}

QT_END_NAMESPACE

#endif // QT_NO_QCOLUMNVIEW
