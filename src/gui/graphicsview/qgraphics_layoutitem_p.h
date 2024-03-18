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

#ifndef QGRAPHICS_LAYOUTITEM_P_H
#define QGRAPHICS_LAYOUTITEM_P_H

#include <qsizef.h>
#include <qsizepolicy.h>

class QGraphicsLayoutItem;
class QGraphicsLayoutItemPrivate
{
   Q_DECLARE_PUBLIC(QGraphicsLayoutItem)

 public:
   enum SizeComponent {
      Width,
      Height
   };

   virtual ~QGraphicsLayoutItemPrivate();

   QGraphicsLayoutItemPrivate(QGraphicsLayoutItem *parent, bool isLayout);
   static QGraphicsLayoutItemPrivate *get(QGraphicsLayoutItem *q) {
      return q->d_func();
   }

   static const QGraphicsLayoutItemPrivate *get(const QGraphicsLayoutItem *q) {
      return q->d_func();
   }

   void init();
   QSizeF *effectiveSizeHints(const QSizeF &constraint) const;
   QGraphicsItem *parentItem() const;
   void ensureUserSizeHints();
   void setSize(Qt::SizeHint which, const QSizeF &size);

   void setSizeComponent(Qt::SizeHint which, SizeComponent component, qreal value);

   bool hasHeightForWidth() const;
   bool hasWidthForHeight() const;

   QSizePolicy sizePolicy;
   QGraphicsLayoutItem *parent;

   QSizeF *userSizeHints;
   mutable QSizeF cachedSizeHints[Qt::NSizeHints];
   mutable QSizeF cachedConstraint;
   mutable QSizeF cachedSizeHintsWithConstraints[Qt::NSizeHints];

   mutable quint32 sizeHintCacheDirty : 1;
   mutable quint32 sizeHintWithConstraintCacheDirty : 1;
   quint32 isLayout : 1;
   quint32 ownedByLayout : 1;

   QGraphicsLayoutItem *q_ptr;
   QRectF geom;
   QGraphicsItem *graphicsItem;
};

#endif

