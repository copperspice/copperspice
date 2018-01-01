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

#ifndef ABSTRACTLIVEEDITTOOL_H
#define ABSTRACTLIVEEDITTOOL_H

#include <QtCore/QList>
#include "../abstracttool.h"

QT_BEGIN_NAMESPACE
class QMouseEvent;
class QGraphicsItem;
class QDeclarativeItem;
class QKeyEvent;
class QGraphicsScene;
class QGraphicsObject;
class QWheelEvent;
class QDeclarativeView;
QT_END_NAMESPACE

namespace QmlJSDebugger {

class QDeclarativeViewInspector;

class AbstractLiveEditTool : public AbstractTool
{
    Q_OBJECT
public:
    AbstractLiveEditTool(QDeclarativeViewInspector *inspector);

    virtual ~AbstractLiveEditTool();

    void leaveEvent(QEvent *) {}

    virtual void itemsAboutToRemoved(const QList<QGraphicsItem*> &itemList) = 0;

    virtual void clear() = 0;

    void updateSelectedItems();
    QList<QGraphicsItem*> items() const;

    bool topItemIsMovable(const QList<QGraphicsItem*> &itemList);
    bool topItemIsResizeHandle(const QList<QGraphicsItem*> &itemList);
    bool topSelectedItemIsMovable(const QList<QGraphicsItem*> &itemList);

    QString titleForItem(QGraphicsItem *item);

    static QList<QGraphicsObject*> toGraphicsObjectList(const QList<QGraphicsItem*> &itemList);
    static QGraphicsItem* topMovableGraphicsItem(const QList<QGraphicsItem*> &itemList);
    static QDeclarativeItem* topMovableDeclarativeItem(const QList<QGraphicsItem*> &itemList);
    static QDeclarativeItem *toQDeclarativeItem(QGraphicsItem *item);

protected:
    virtual void selectedItemsChanged(const QList<QGraphicsItem*> &objectList) = 0;

    QDeclarativeViewInspector *inspector() const;
    QDeclarativeView *view() const;
    QGraphicsScene *scene() const;

private:
    QList<QGraphicsItem*> m_itemList;
};

}

#endif // ABSTRACTLIVEEDITTOOL_H
