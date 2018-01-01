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

#ifndef LIVESELECTIONINDICATOR_H
#define LIVESELECTIONINDICATOR_H

#include <QtCore/QWeakPointer>
#include <QtCore/QHash>

QT_BEGIN_NAMESPACE
class QGraphicsObject;
class QGraphicsRectItem;
class QGraphicsItem;
class QPolygonF;
QT_END_NAMESPACE

namespace QmlJSDebugger {

class QDeclarativeViewInspector;

class LiveSelectionIndicator
{
public:
    LiveSelectionIndicator(QDeclarativeViewInspector *viewInspector, QGraphicsObject *layerItem);
    ~LiveSelectionIndicator();

    void show();
    void hide();

    void clear();

    void setItems(const QList<QWeakPointer<QGraphicsObject> > &itemList);

private:
    QHash<QGraphicsItem*, QGraphicsRectItem *> m_indicatorShapeHash;
    QWeakPointer<QGraphicsObject> m_layerItem;
    QDeclarativeViewInspector *m_view;
};

}

#endif // LIVESELECTIONINDICATOR_H
