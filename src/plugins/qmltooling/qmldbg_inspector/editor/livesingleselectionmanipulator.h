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

#ifndef LIVESINGLESELECTIONMANIPULATOR_H
#define LIVESINGLESELECTIONMANIPULATOR_H

#include <QtCore/QPointF>
#include <QtCore/QList>

QT_FORWARD_DECLARE_CLASS(QGraphicsItem)

namespace QmlJSDebugger {

class QDeclarativeViewInspector;

class LiveSingleSelectionManipulator
{
public:
    LiveSingleSelectionManipulator(QDeclarativeViewInspector *editorView);

    enum SelectionType {
        ReplaceSelection,
        AddToSelection,
        RemoveFromSelection,
        InvertSelection
    };

    void begin(const QPointF& beginPoint);
    void update(const QPointF& updatePoint);
    void end(const QPointF& updatePoint);

    void select(SelectionType selectionType, const QList<QGraphicsItem*> &items,
                bool selectOnlyContentItems);
    void select(SelectionType selectionType, bool selectOnlyContentItems);

    void clear();

    QPointF beginPoint() const;

    bool isActive() const;

private:
    QList<QGraphicsItem*> m_oldSelectionList;
    QPointF m_beginPoint;
    QDeclarativeViewInspector *m_editorView;
    bool m_isActive;
};

} // namespace QmlJSDebugger

#endif // LIVESINGLESELECTIONMANIPULATOR_H
