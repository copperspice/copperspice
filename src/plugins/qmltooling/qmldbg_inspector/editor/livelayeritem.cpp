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

#include "livelayeritem.h"

#include "../qmlinspectorconstants.h"

#include <QGraphicsScene>

namespace QmlJSDebugger {

LiveLayerItem::LiveLayerItem(QGraphicsScene* scene)
    : QGraphicsObject()
{
    scene->addItem(this);
    setZValue(1);
    setFlag(QGraphicsItem::ItemIsMovable, false);
}

LiveLayerItem::~LiveLayerItem()
{
}

void LiveLayerItem::paint(QPainter * /*painter*/, const QStyleOptionGraphicsItem * /*option*/,
                          QWidget * /*widget*/)
{
}

int LiveLayerItem::type() const
{
    return Constants::EditorItemType;
}

QRectF LiveLayerItem::boundingRect() const
{
    return childrenBoundingRect();
}

QList<QGraphicsItem*> LiveLayerItem::findAllChildItems() const
{
    return findAllChildItems(this);
}

QList<QGraphicsItem*> LiveLayerItem::findAllChildItems(const QGraphicsItem *item) const
{
    QList<QGraphicsItem*> itemList(item->childItems());

    foreach (QGraphicsItem *childItem, item->childItems())
        itemList += findAllChildItems(childItem);

    return itemList;
}

} // namespace QmlJSDebugger
