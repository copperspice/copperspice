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

#include "liverubberbandselectionmanipulator.h"

#include "../qdeclarativeviewinspector_p.h"

#include <QtGui/QGraphicsItem>

#include <QtCore/QDebug>

namespace QmlJSDebugger {

LiveRubberBandSelectionManipulator::LiveRubberBandSelectionManipulator(QGraphicsObject *layerItem,
                                                                       QDeclarativeViewInspector *editorView)
    : m_selectionRectangleElement(layerItem),
      m_editorView(editorView),
      m_beginFormEditorItem(0),
      m_isActive(false)
{
    m_selectionRectangleElement.hide();
}

void LiveRubberBandSelectionManipulator::clear()
{
    m_selectionRectangleElement.clear();
    m_isActive = false;
    m_beginPoint = QPointF();
    m_itemList.clear();
    m_oldSelectionList.clear();
}

QGraphicsItem *LiveRubberBandSelectionManipulator::topFormEditorItem(const QList<QGraphicsItem*>
                                                                     &itemList)
{
    if (itemList.isEmpty())
        return 0;

    return itemList.first();
}

void LiveRubberBandSelectionManipulator::begin(const QPointF &beginPoint)
{
    m_beginPoint = beginPoint;
    m_selectionRectangleElement.setRect(m_beginPoint, m_beginPoint);
    m_selectionRectangleElement.show();
    m_isActive = true;
    QDeclarativeViewInspectorPrivate *inspectorPrivate
            = QDeclarativeViewInspectorPrivate::get(m_editorView);
    m_beginFormEditorItem = topFormEditorItem(inspectorPrivate->selectableItems(beginPoint));
    m_oldSelectionList = m_editorView->selectedItems();
}

void LiveRubberBandSelectionManipulator::update(const QPointF &updatePoint)
{
    m_selectionRectangleElement.setRect(m_beginPoint, updatePoint);
}

void LiveRubberBandSelectionManipulator::end()
{
    m_oldSelectionList.clear();
    m_selectionRectangleElement.hide();
    m_isActive = false;
}

void LiveRubberBandSelectionManipulator::select(SelectionType selectionType)
{
    QDeclarativeViewInspectorPrivate *inspectorPrivate
            = QDeclarativeViewInspectorPrivate::get(m_editorView);
    QList<QGraphicsItem*> itemList
            = inspectorPrivate->selectableItems(m_selectionRectangleElement.rect(),
                                               Qt::IntersectsItemShape);
    QList<QGraphicsItem*> newSelectionList;

    foreach (QGraphicsItem* item, itemList) {
        if (item
                && item->parentItem()
                && !newSelectionList.contains(item)
                //&& m_beginFormEditorItem->childItems().contains(item) // TODO activate this test
                )
        {
            newSelectionList.append(item);
        }
    }

    if (newSelectionList.isEmpty() && m_beginFormEditorItem)
        newSelectionList.append(m_beginFormEditorItem);

    QList<QGraphicsItem*> resultList;

    switch (selectionType) {
    case AddToSelection: {
        resultList.append(m_oldSelectionList);
        resultList.append(newSelectionList);
    }
        break;
    case ReplaceSelection: {
        resultList.append(newSelectionList);
    }
        break;
    case RemoveFromSelection: {
        QSet<QGraphicsItem*> oldSelectionSet(m_oldSelectionList.toSet());
        QSet<QGraphicsItem*> newSelectionSet(newSelectionList.toSet());
        resultList.append(oldSelectionSet.subtract(newSelectionSet).toList());
    }
    }

    m_editorView->setSelectedItems(resultList);
}


void LiveRubberBandSelectionManipulator::setItems(const QList<QGraphicsItem*> &itemList)
{
    m_itemList = itemList;
}

QPointF LiveRubberBandSelectionManipulator::beginPoint() const
{
    return m_beginPoint;
}

bool LiveRubberBandSelectionManipulator::isActive() const
{
    return m_isActive;
}

} // namespace QmlJSDebugger
