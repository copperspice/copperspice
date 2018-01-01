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

#include "subcomponentmasklayeritem.h"

#include "../qmlinspectorconstants.h"
#include "../qdeclarativeviewinspector.h"

#include <QtGui/QPolygonF>

namespace QmlJSDebugger {

SubcomponentMaskLayerItem::SubcomponentMaskLayerItem(QDeclarativeViewInspector *inspector,
                                                     QGraphicsItem *parentItem) :
    QGraphicsPolygonItem(parentItem),
    m_inspector(inspector),
    m_currentItem(0),
    m_borderRect(new QGraphicsRectItem(this))
{
    m_borderRect->setRect(0,0,0,0);
    m_borderRect->setPen(QPen(QColor(60, 60, 60), 1));
    m_borderRect->setData(Constants::EditorItemDataKey, QVariant(true));

    setBrush(QBrush(QColor(160,160,160)));
    setPen(Qt::NoPen);
}

int SubcomponentMaskLayerItem::type() const
{
    return Constants::EditorItemType;
}

static QRectF resizeRect(const QRectF &newRect, const QRectF &oldRect)
{
    QRectF result = newRect;
    if (oldRect.left() < newRect.left())
        result.setLeft(oldRect.left());

    if (oldRect.top() < newRect.top())
        result.setTop(oldRect.top());

    if (oldRect.right() > newRect.right())
        result.setRight(oldRect.right());

    if (oldRect.bottom() > newRect.bottom())
        result.setBottom(oldRect.bottom());

    return result;
}

static QPolygonF regionToPolygon(const QRegion &region)
{
    QPainterPath path;
    foreach (const QRect &rect, region.rects())
        path.addRect(rect);
    return path.toFillPolygon();
}

void SubcomponentMaskLayerItem::setCurrentItem(QGraphicsItem *item)
{
    QGraphicsItem *prevItem = m_currentItem;
    m_currentItem = item;

    if (!m_currentItem)
        return;

    QRect viewRect = m_inspector->declarativeView()->rect();
    viewRect = m_inspector->declarativeView()->mapToScene(viewRect).boundingRect().toRect();

    QRectF itemRect = item->boundingRect() | item->childrenBoundingRect();
    itemRect = item->mapRectToScene(itemRect);

    // if updating the same item as before, resize the rectangle only bigger, not smaller.
    if (prevItem == item && prevItem != 0) {
        m_itemPolyRect = resizeRect(itemRect, m_itemPolyRect);
    } else {
        m_itemPolyRect = itemRect;
    }
    QRectF borderRect = m_itemPolyRect;
    borderRect.adjust(-1, -1, 1, 1);
    m_borderRect->setRect(borderRect);

    const QRegion externalRegion = QRegion(viewRect).subtracted(m_itemPolyRect.toRect());
    setPolygon(regionToPolygon(externalRegion));
}

QGraphicsItem *SubcomponentMaskLayerItem::currentItem() const
{
    return m_currentItem;
}

} // namespace QmlJSDebugger
