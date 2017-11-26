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

#include "livesingleselectionmanipulator.h"
#include "../qdeclarativeviewinspector_p.h"

#include <QDebug>

namespace QmlJSDebugger {

LiveSingleSelectionManipulator::LiveSingleSelectionManipulator(QDeclarativeViewInspector *editorView)
    : m_editorView(editorView),
      m_isActive(false)
{
}


void LiveSingleSelectionManipulator::begin(const QPointF &beginPoint)
{
    m_beginPoint = beginPoint;
    m_isActive = true;
    m_oldSelectionList = QDeclarativeViewInspectorPrivate::get(m_editorView)->selectedItems();
}

void LiveSingleSelectionManipulator::update(const QPointF &/*updatePoint*/)
{
    m_oldSelectionList.clear();
}

void LiveSingleSelectionManipulator::clear()
{
    m_beginPoint = QPointF();
    m_oldSelectionList.clear();
}


void LiveSingleSelectionManipulator::end(const QPointF &/*updatePoint*/)
{
    m_oldSelectionList.clear();
    m_isActive = false;
}

void LiveSingleSelectionManipulator::select(SelectionType selectionType,
                                            const QList<QGraphicsItem*> &items,
                                            bool /*selectOnlyContentItems*/)
{
    QGraphicsItem *selectedItem = 0;

    foreach (QGraphicsItem* item, items)
    {
        //FormEditorItem *formEditorItem = FormEditorItem::fromQGraphicsItem(item);
        if (item
            /*&& !formEditorItem->qmlItemNode().isRootNode()
               && (formEditorItem->qmlItemNode().hasShowContent() || !selectOnlyContentItems)*/)
        {
            selectedItem = item;
            break;
        }
    }

    QList<QGraphicsItem*> resultList;

    switch (selectionType) {
    case AddToSelection: {
        resultList.append(m_oldSelectionList);
        if (selectedItem && !m_oldSelectionList.contains(selectedItem))
            resultList.append(selectedItem);
    }
        break;
    case ReplaceSelection: {
        if (selectedItem)
            resultList.append(selectedItem);
    }
        break;
    case RemoveFromSelection: {
        resultList.append(m_oldSelectionList);
        if (selectedItem)
            resultList.removeAll(selectedItem);
    }
        break;
    case InvertSelection: {
        if (selectedItem
                && !m_oldSelectionList.contains(selectedItem))
        {
            resultList.append(selectedItem);
        }
    }
    }

    m_editorView->setSelectedItems(resultList);
}

void LiveSingleSelectionManipulator::select(SelectionType selectionType, bool selectOnlyContentItems)
{
    QDeclarativeViewInspectorPrivate *inspectorPrivate =
            QDeclarativeViewInspectorPrivate::get(m_editorView);
    QList<QGraphicsItem*> itemList = inspectorPrivate->selectableItems(m_beginPoint);
    select(selectionType, itemList, selectOnlyContentItems);
}


bool LiveSingleSelectionManipulator::isActive() const
{
    return m_isActive;
}

QPointF LiveSingleSelectionManipulator::beginPoint() const
{
    return m_beginPoint;
}

} // namespace QmlJSDebugger
