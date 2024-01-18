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

#ifndef QDeclarativeRectangle_P_P_H
#define QDeclarativeRectangle_P_P_H

#include <qdeclarativeitem_p.h>

QT_BEGIN_NAMESPACE

class QDeclarativeGradient;
class QDeclarativeRectangle;
class QDeclarativeRectanglePrivate : public QDeclarativeItemPrivate
{
   Q_DECLARE_PUBLIC(QDeclarativeRectangle)

 public:
   QDeclarativeRectanglePrivate() :
      color(Qt::white), gradient(0), pen(0), radius(0), paintmargin(0) {
      QGraphicsItemPrivate::flags = QGraphicsItemPrivate::flags & ~QGraphicsItem::ItemHasNoContents;
   }

   ~QDeclarativeRectanglePrivate() {
      delete pen;
   }

   QColor color;
   QDeclarativeGradient *gradient;
   QDeclarativePen *pen;
   qreal radius;
   qreal paintmargin;
   QPixmap rectImage;
   static int doUpdateSlotIdx;

   QDeclarativePen *getPen() {
      if (!pen) {
         Q_Q(QDeclarativeRectangle);
         pen = new QDeclarativePen;
         static int penChangedSignalIdx = -1;
         if (penChangedSignalIdx < 0) {
            penChangedSignalIdx = QDeclarativePen::staticMetaObject.indexOfSignal("penChanged()");
         }
         if (doUpdateSlotIdx < 0) {
            doUpdateSlotIdx = QDeclarativeRectangle::staticMetaObject.indexOfSlot("doUpdate()");
         }
         QMetaObject::connect(pen, penChangedSignalIdx, q, doUpdateSlotIdx);
      }
      return pen;
   }

   void setPaintMargin(qreal margin) {
      Q_Q(QDeclarativeRectangle);
      if (margin == paintmargin) {
         return;
      }
      q->prepareGeometryChange();
      paintmargin = margin;
   }
};

QT_END_NAMESPACE

#endif // QDECLARATIVERECT_P_H
