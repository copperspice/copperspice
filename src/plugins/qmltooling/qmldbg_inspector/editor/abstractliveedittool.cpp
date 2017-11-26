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

#include "abstractliveedittool.h"
#include "../qdeclarativeviewinspector_p.h"

#include <QDeclarativeEngine>
#include <QDebug>
#include <QGraphicsItem>
#include <QDeclarativeItem>

namespace QmlJSDebugger {

AbstractLiveEditTool::AbstractLiveEditTool(QDeclarativeViewInspector *editorView)
    : AbstractTool(editorView)
{
}


AbstractLiveEditTool::~AbstractLiveEditTool()
{
}

QDeclarativeViewInspector *AbstractLiveEditTool::inspector() const
{
    return static_cast<QDeclarativeViewInspector*>(AbstractTool::inspector());
}

QDeclarativeView *AbstractLiveEditTool::view() const
{
    return inspector()->declarativeView();
}

QGraphicsScene* AbstractLiveEditTool::scene() const
{
    return view()->scene();
}

void AbstractLiveEditTool::updateSelectedItems()
{
    selectedItemsChanged(items());
}

QList<QGraphicsItem*> AbstractLiveEditTool::items() const
{
    return inspector()->selectedItems();
}

bool AbstractLiveEditTool::topItemIsMovable(const QList<QGraphicsItem*> & itemList)
{
    QGraphicsItem *firstSelectableItem = topMovableGraphicsItem(itemList);
    if (firstSelectableItem == 0)
        return false;
    if (toQDeclarativeItem(firstSelectableItem) != 0)
        return true;

    return false;

}

bool AbstractLiveEditTool::topSelectedItemIsMovable(const QList<QGraphicsItem*> &itemList)
{
    QList<QGraphicsItem*> selectedItems = inspector()->selectedItems();

    foreach (QGraphicsItem *item, itemList) {
        QDeclarativeItem *declarativeItem = toQDeclarativeItem(item);
        if (declarativeItem
                && selectedItems.contains(declarativeItem)
                /*&& (declarativeItem->qmlItemNode().hasShowContent() || selectNonContentItems)*/)
            return true;
    }

    return false;

}

bool AbstractLiveEditTool::topItemIsResizeHandle(const QList<QGraphicsItem*> &/*itemList*/)
{
    return false;
}

QDeclarativeItem *AbstractLiveEditTool::toQDeclarativeItem(QGraphicsItem *item)
{
    return qobject_cast<QDeclarativeItem*>(item->toGraphicsObject());
}

QGraphicsItem *AbstractLiveEditTool::topMovableGraphicsItem(const QList<QGraphicsItem*> &itemList)
{
    foreach (QGraphicsItem *item, itemList) {
        if (item->flags().testFlag(QGraphicsItem::ItemIsMovable))
            return item;
    }
    return 0;
}

QDeclarativeItem *AbstractLiveEditTool::topMovableDeclarativeItem(const QList<QGraphicsItem*>
                                                                  &itemList)
{
    foreach (QGraphicsItem *item, itemList) {
        QDeclarativeItem *declarativeItem = toQDeclarativeItem(item);
        if (declarativeItem /*&& (declarativeItem->qmlItemNode().hasShowContent())*/)
            return declarativeItem;
    }

    return 0;
}

QList<QGraphicsObject*> AbstractLiveEditTool::toGraphicsObjectList(const QList<QGraphicsItem*>
                                                                   &itemList)
{
    QList<QGraphicsObject*> gfxObjects;
    foreach (QGraphicsItem *item, itemList) {
        QGraphicsObject *obj = item->toGraphicsObject();
        if (obj)
            gfxObjects << obj;
    }

    return gfxObjects;
}

QString AbstractLiveEditTool::titleForItem(QGraphicsItem *item)
{
    QString className(QLatin1String("QGraphicsItem"));
    QString objectStringId;

    QString constructedName;

    QGraphicsObject *gfxObject = item->toGraphicsObject();
    if (gfxObject) {
        className = QLatin1String(gfxObject->metaObject()->className());

        className.remove(QRegExp(QLatin1String("_QMLTYPE_\\d+")));
        className.remove(QRegExp(QLatin1String("_QML_\\d+")));
        if (className.startsWith(QLatin1String("QDeclarative")))
            className = className.remove(QLatin1String("QDeclarative"));

        QDeclarativeItem *declarativeItem = qobject_cast<QDeclarativeItem*>(gfxObject);
        if (declarativeItem) {
            objectStringId = inspector()->idStringForObject(declarativeItem);
        }

        if (!objectStringId.isEmpty()) {
            constructedName = objectStringId + QLatin1String(" (") + className + QLatin1Char(')');
        } else {
            if (!gfxObject->objectName().isEmpty()) {
                constructedName = gfxObject->objectName() + QLatin1String(" (") + className + QLatin1Char(')');
            } else {
                constructedName = className;
            }
        }
    }

    return constructedName;
}


} // namespace QmlJSDebugger
