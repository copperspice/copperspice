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

#include "qdeclarativegraphicswidget_p.h"
#include "private/qdeclarativeanchors_p.h"
#include "private/qdeclarativeitem_p.h"
#include "private/qdeclarativeanchors_p_p.h"

QT_BEGIN_NAMESPACE

class QDeclarativeGraphicsWidgetPrivate
{
   Q_DECLARE_PUBLIC(QDeclarativeGraphicsWidget)

 public :
   QDeclarativeGraphicsWidgetPrivate() :
      _anchors(0), _anchorLines(0) {
   }
   QDeclarativeItemPrivate::AnchorLines *anchorLines() const;
   QDeclarativeAnchors *_anchors;
   mutable QDeclarativeItemPrivate::AnchorLines *_anchorLines;
};

QDeclarativeGraphicsWidget::QDeclarativeGraphicsWidget(QObject *parent) :
   QObject(*new QDeclarativeGraphicsWidgetPrivate, parent)
{
}
QDeclarativeGraphicsWidget::~QDeclarativeGraphicsWidget()
{
   Q_D(QDeclarativeGraphicsWidget);
   delete d->_anchorLines;
   d->_anchorLines = 0;
   delete d->_anchors;
   d->_anchors = 0;
}

QDeclarativeAnchors *QDeclarativeGraphicsWidget::anchors()
{
   Q_D(QDeclarativeGraphicsWidget);
   if (!d->_anchors) {
      d->_anchors = new QDeclarativeAnchors(static_cast<QGraphicsObject *>(parent()));
   }
   return d->_anchors;
}

QDeclarativeItemPrivate::AnchorLines *QDeclarativeGraphicsWidgetPrivate::anchorLines() const
{
   Q_Q(const QDeclarativeGraphicsWidget);
   if (!_anchorLines) {
      _anchorLines = new QDeclarativeItemPrivate::AnchorLines(static_cast<QGraphicsObject *>(q->parent()));
   }
   return _anchorLines;
}

QDeclarativeAnchorLine QDeclarativeGraphicsWidget::left() const
{
   Q_D(const QDeclarativeGraphicsWidget);
   return d->anchorLines()->left;
}

QDeclarativeAnchorLine QDeclarativeGraphicsWidget::right() const
{
   Q_D(const QDeclarativeGraphicsWidget);
   return d->anchorLines()->right;
}

QDeclarativeAnchorLine QDeclarativeGraphicsWidget::horizontalCenter() const
{
   Q_D(const QDeclarativeGraphicsWidget);
   return d->anchorLines()->hCenter;
}

QDeclarativeAnchorLine QDeclarativeGraphicsWidget::top() const
{
   Q_D(const QDeclarativeGraphicsWidget);
   return d->anchorLines()->top;
}

QDeclarativeAnchorLine QDeclarativeGraphicsWidget::bottom() const
{
   Q_D(const QDeclarativeGraphicsWidget);
   return d->anchorLines()->bottom;
}

QDeclarativeAnchorLine QDeclarativeGraphicsWidget::verticalCenter() const
{
   Q_D(const QDeclarativeGraphicsWidget);
   return d->anchorLines()->vCenter;
}

QT_END_NAMESPACE
