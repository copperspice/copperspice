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

#include <qgraphics_gridlayoutengine_p.h>

#ifndef QT_NO_GRAPHICSVIEW

#include <qgraphicswidget.h>

#include <qgraphics_layoutitem_p.h>
#include <qgraphics_layout_p.h>
#include <qgraphics_widget_p.h>

bool QGraphicsGridLayoutEngineItem::isHidden() const
{
   if (QGraphicsItem *item = q_layoutItem->graphicsItem()) {
      return QGraphicsItemPrivate::get(item)->explicitlyHidden;
   }

   return false;
}

// TODO: Move to QGraphicsLayoutItem and make virtual
bool QGraphicsGridLayoutEngineItem::isIgnored() const
{
   return isHidden() && ! q_layoutItem->sizePolicy().retainSizeWhenHidden();
}

bool QGraphicsGridLayoutEngineItem::hasDynamicConstraint() const
{
   return QGraphicsLayoutItemPrivate::get(q_layoutItem)->hasHeightForWidth()
      || QGraphicsLayoutItemPrivate::get(q_layoutItem)->hasWidthForHeight();
}

Qt::Orientation QGraphicsGridLayoutEngineItem::dynamicConstraintOrientation() const
{
   if (QGraphicsLayoutItemPrivate::get(q_layoutItem)->hasHeightForWidth()) {
      return Qt::Vertical;

   } else {
      // if (QGraphicsLayoutItemPrivate::get(q_layoutItem)->hasWidthForHeight())
      return Qt::Horizontal;
   }
}

void QGraphicsGridLayoutEngine::setAlignment(QGraphicsLayoutItem *graphicsLayoutItem, Qt::Alignment alignment)
{
   if (QGraphicsGridLayoutEngineItem *gridEngineItem = findLayoutItem(graphicsLayoutItem)) {
      gridEngineItem->setAlignment(alignment);
      invalidate();
   }
}

Qt::Alignment QGraphicsGridLayoutEngine::alignment(QGraphicsLayoutItem *graphicsLayoutItem) const
{
   if (QGraphicsGridLayoutEngineItem *gridEngineItem = findLayoutItem(graphicsLayoutItem)) {
      return gridEngineItem->alignment();
   }

   return Qt::EmptyFlag;
}

void QGraphicsGridLayoutEngine::setStretchFactor(QGraphicsLayoutItem *layoutItem, int stretch,
   Qt::Orientation orientation)
{
   Q_ASSERT(stretch >= 0);

   if (QGraphicsGridLayoutEngineItem *item = findLayoutItem(layoutItem)) {
      item->setStretchFactor(stretch, orientation);
   }
}

int QGraphicsGridLayoutEngine::stretchFactor(QGraphicsLayoutItem *layoutItem, Qt::Orientation orientation) const
{
   if (QGraphicsGridLayoutEngineItem *item = findLayoutItem(layoutItem)) {
      return item->stretchFactor(orientation);
   }

   return 0;
}

#endif // QT_NO_GRAPHICSVIEW
