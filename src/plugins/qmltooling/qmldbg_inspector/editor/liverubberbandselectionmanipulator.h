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

#ifndef RUBBERBANDSELECTIONMANIPULATOR_H
#define RUBBERBANDSELECTIONMANIPULATOR_H

#include "liveselectionrectangle.h"

#include <QtCore/QPointF>

QT_FORWARD_DECLARE_CLASS(QGraphicsItem)

namespace QmlJSDebugger {

class QDeclarativeViewInspector;

class LiveRubberBandSelectionManipulator
{
public:
    enum SelectionType {
        ReplaceSelection,
        AddToSelection,
        RemoveFromSelection
    };

    LiveRubberBandSelectionManipulator(QGraphicsObject *layerItem,
                                       QDeclarativeViewInspector *editorView);

    void setItems(const QList<QGraphicsItem*> &itemList);

    void begin(const QPointF& beginPoint);
    void update(const QPointF& updatePoint);
    void end();

    void clear();

    void select(SelectionType selectionType);

    QPointF beginPoint() const;

    bool isActive() const;

protected:
    QGraphicsItem *topFormEditorItem(const QList<QGraphicsItem*> &itemList);

private:
    QList<QGraphicsItem*> m_itemList;
    QList<QGraphicsItem*> m_oldSelectionList;
    LiveSelectionRectangle m_selectionRectangleElement;
    QPointF m_beginPoint;
    QDeclarativeViewInspector *m_editorView;
    QGraphicsItem *m_beginFormEditorItem;
    bool m_isActive;
};

}

#endif // RUBBERBANDSELECTIONMANIPULATOR_H
