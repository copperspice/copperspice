/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#include "liveselectiontool.h"
#include "livelayeritem.h"

#include "../qdeclarativeviewinspector_p.h"

#include <QtGui/QApplication>
#include <QtGui/QWheelEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QClipboard>
#include <QtGui/QMenu>
#include <QtGui/QAction>
#include <QtGui/QGraphicsObject>

#include <QtDeclarative/QDeclarativeItem>
#include <QtDeclarative/QDeclarativeEngine>

#include <QtCore/QDebug>

namespace QmlJSDebugger {

LiveSelectionTool::LiveSelectionTool(QDeclarativeViewInspector *editorView) :
    AbstractLiveEditTool(editorView),
    m_rubberbandSelectionMode(false),
    m_rubberbandSelectionManipulator(
        QDeclarativeViewInspectorPrivate::get(editorView)->manipulatorLayer, editorView),
    m_singleSelectionManipulator(editorView),
    m_selectionIndicator(editorView,
        QDeclarativeViewInspectorPrivate::get(editorView)->manipulatorLayer),
    //m_resizeIndicator(editorView->manipulatorLayer()),
    m_selectOnlyContentItems(true)
{

}

LiveSelectionTool::~LiveSelectionTool()
{
}

void LiveSelectionTool::setRubberbandSelectionMode(bool value)
{
    m_rubberbandSelectionMode = value;
}

LiveSingleSelectionManipulator::SelectionType LiveSelectionTool::getSelectionType(Qt::KeyboardModifiers
                                                                          modifiers)
{
   LiveSingleSelectionManipulator::SelectionType selectionType
            = LiveSingleSelectionManipulator::ReplaceSelection;
    if (modifiers.testFlag(Qt::ControlModifier)) {
        selectionType = LiveSingleSelectionManipulator::RemoveFromSelection;
    } else if (modifiers.testFlag(Qt::ShiftModifier)) {
        selectionType = LiveSingleSelectionManipulator::AddToSelection;
    }
    return selectionType;
}

bool LiveSelectionTool::alreadySelected(const QList<QGraphicsItem*> &itemList) const
{
    QDeclarativeViewInspectorPrivate *inspectorPrivate
            = QDeclarativeViewInspectorPrivate::get(inspector());
    const QList<QGraphicsItem*> selectedItems = inspectorPrivate->selectedItems();

    if (selectedItems.isEmpty())
        return false;

    foreach (QGraphicsItem *item, itemList)
        if (selectedItems.contains(item))
            return true;

    return false;
}

void LiveSelectionTool::mousePressEvent(QMouseEvent *event)
{
    QDeclarativeViewInspectorPrivate *inspectorPrivate
            = QDeclarativeViewInspectorPrivate::get(inspector());
    QList<QGraphicsItem*> itemList = inspectorPrivate->selectableItems(event->pos());
    LiveSingleSelectionManipulator::SelectionType selectionType = getSelectionType(event->modifiers());

    if (event->buttons() & Qt::LeftButton) {
        m_mousePressTimer.start();

        if (m_rubberbandSelectionMode) {
            m_rubberbandSelectionManipulator.begin(event->pos());
        } else {
            m_singleSelectionManipulator.begin(event->pos());
            m_singleSelectionManipulator.select(selectionType, m_selectOnlyContentItems);
        }
    } else if (event->buttons() & Qt::RightButton) {
        createContextMenu(itemList, event->globalPos());
    }
}

void LiveSelectionTool::createContextMenu(const QList<QGraphicsItem*> &itemList, QPoint globalPos)
{
    QMenu contextMenu;
    connect(&contextMenu, SIGNAL(hovered(QAction*)),
            this, SLOT(contextMenuElementHovered(QAction*)));

    m_contextMenuItemList = itemList;

    contextMenu.addAction(tr("Items"));
    contextMenu.addSeparator();
    int shortcutKey = Qt::Key_1;
    int i = 0;

    foreach (QGraphicsItem * const item, itemList) {
        QString itemTitle = titleForItem(item);
        QAction *elementAction = contextMenu.addAction(itemTitle, this,
                                                       SLOT(contextMenuElementSelected()));

        if (inspector()->selectedItems().contains(item)) {
            QFont boldFont = elementAction->font();
            boldFont.setBold(true);
            elementAction->setFont(boldFont);
        }

        elementAction->setData(i);

        if (shortcutKey <= Qt::Key_9) {
            elementAction->setShortcut(QKeySequence(shortcutKey));
            shortcutKey++;
        }

        ++i;
    }
    // add root item separately
    //    QString itemTitle = QString(tr("%1")).arg(titleForItem(view()->currentRootItem()));
    //    contextMenu.addAction(itemTitle, this, SLOT(contextMenuElementSelected()));
    //    m_contextMenuItemList.append(view()->currentRootItem());

    contextMenu.exec(globalPos);
    m_contextMenuItemList.clear();
}

void LiveSelectionTool::contextMenuElementSelected()
{
    QAction *senderAction = static_cast<QAction*>(sender());
    int itemListIndex = senderAction->data().toInt();
    if (itemListIndex >= 0 && itemListIndex < m_contextMenuItemList.length()) {

        QPointF updatePt(0, 0);
        QGraphicsItem *item = m_contextMenuItemList.at(itemListIndex);
        m_singleSelectionManipulator.begin(updatePt);
        m_singleSelectionManipulator.select(LiveSingleSelectionManipulator::InvertSelection,
                                            QList<QGraphicsItem*>() << item,
                                            false);
        m_singleSelectionManipulator.end(updatePt);
    }
}

void LiveSelectionTool::contextMenuElementHovered(QAction *action)
{
    int itemListIndex = action->data().toInt();
    if (itemListIndex >= 0 && itemListIndex < m_contextMenuItemList.length()) {
        QGraphicsObject *item = m_contextMenuItemList.at(itemListIndex)->toGraphicsObject();
        QDeclarativeViewInspectorPrivate::get(inspector())->highlight(item);
    }
}

void LiveSelectionTool::mouseMoveEvent(QMouseEvent *event)
{
    if (m_singleSelectionManipulator.isActive()) {
        QPointF mouseMovementVector = m_singleSelectionManipulator.beginPoint() - event->pos();

        if ((mouseMovementVector.toPoint().manhattanLength() > Constants::DragStartDistance)
                && (m_mousePressTimer.elapsed() > Constants::DragStartTime))
        {
            m_singleSelectionManipulator.end(event->pos());
            //view()->changeToMoveTool(m_singleSelectionManipulator.beginPoint());
            return;
        }
    } else if (m_rubberbandSelectionManipulator.isActive()) {
        QPointF mouseMovementVector = m_rubberbandSelectionManipulator.beginPoint() - event->pos();

        if ((mouseMovementVector.toPoint().manhattanLength() > Constants::DragStartDistance)
                && (m_mousePressTimer.elapsed() > Constants::DragStartTime)) {
            m_rubberbandSelectionManipulator.update(event->pos());

            if (event->modifiers().testFlag(Qt::ControlModifier))
                m_rubberbandSelectionManipulator.select(
                            LiveRubberBandSelectionManipulator::RemoveFromSelection);
            else if (event->modifiers().testFlag(Qt::ShiftModifier))
                m_rubberbandSelectionManipulator.select(
                            LiveRubberBandSelectionManipulator::AddToSelection);
            else
                m_rubberbandSelectionManipulator.select(
                            LiveRubberBandSelectionManipulator::ReplaceSelection);
        }
    }
}

void LiveSelectionTool::hoverMoveEvent(QMouseEvent * event)
{
// ### commented out until move tool is re-enabled
//    QList<QGraphicsItem*> itemList = view()->items(event->pos());
//    if (!itemList.isEmpty() && !m_rubberbandSelectionMode) {
//
//        foreach (QGraphicsItem *item, itemList) {
//            if (item->type() == Constants::ResizeHandleItemType) {
//                ResizeHandleItem* resizeHandle = ResizeHandleItem::fromGraphicsItem(item);
//                if (resizeHandle)
//                    view()->changeTool(Constants::ResizeToolMode);
//                return;
//            }
//        }
//        if (topSelectedItemIsMovable(itemList))
//            view()->changeTool(Constants::MoveToolMode);
//    }
    QDeclarativeViewInspectorPrivate *inspectorPrivate
            = QDeclarativeViewInspectorPrivate::get(inspector());

    QList<QGraphicsItem*> selectableItemList = inspectorPrivate->selectableItems(event->pos());
    if (!selectableItemList.isEmpty()) {
        QGraphicsObject *item = selectableItemList.first()->toGraphicsObject();
        if (item)
            QDeclarativeViewInspectorPrivate::get(inspector())->highlight(item);

        return;
    }

    QDeclarativeViewInspectorPrivate::get(inspector())->clearHighlight();
}

void LiveSelectionTool::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_singleSelectionManipulator.isActive()) {
        m_singleSelectionManipulator.end(event->pos());
    }
    else if (m_rubberbandSelectionManipulator.isActive()) {

        QPointF mouseMovementVector = m_rubberbandSelectionManipulator.beginPoint() - event->pos();
        if (mouseMovementVector.toPoint().manhattanLength() < Constants::DragStartDistance) {
            m_singleSelectionManipulator.begin(event->pos());

            if (event->modifiers().testFlag(Qt::ControlModifier))
                m_singleSelectionManipulator.select(LiveSingleSelectionManipulator::RemoveFromSelection,
                                                    m_selectOnlyContentItems);
            else if (event->modifiers().testFlag(Qt::ShiftModifier))
                m_singleSelectionManipulator.select(LiveSingleSelectionManipulator::AddToSelection,
                                                    m_selectOnlyContentItems);
            else
                m_singleSelectionManipulator.select(LiveSingleSelectionManipulator::InvertSelection,
                                                    m_selectOnlyContentItems);

            m_singleSelectionManipulator.end(event->pos());
        } else {
            m_rubberbandSelectionManipulator.update(event->pos());

            if (event->modifiers().testFlag(Qt::ControlModifier))
                m_rubberbandSelectionManipulator.select(
                            LiveRubberBandSelectionManipulator::RemoveFromSelection);
            else if (event->modifiers().testFlag(Qt::ShiftModifier))
                m_rubberbandSelectionManipulator.select(
                            LiveRubberBandSelectionManipulator::AddToSelection);
            else
                m_rubberbandSelectionManipulator.select(
                            LiveRubberBandSelectionManipulator::ReplaceSelection);

            m_rubberbandSelectionManipulator.end();
        }
    }
}

void LiveSelectionTool::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Left:
    case Qt::Key_Right:
    case Qt::Key_Up:
    case Qt::Key_Down:
        // disabled for now, cannot move stuff yet.
        //view()->changeTool(Constants::MoveToolMode);
        //view()->currentTool()->keyPressEvent(event);
        break;
    }
}

void LiveSelectionTool::wheelEvent(QWheelEvent *event)
{
    if (event->orientation() == Qt::Horizontal || m_rubberbandSelectionMode)
        return;

    QDeclarativeViewInspectorPrivate *inspectorPrivate
            = QDeclarativeViewInspectorPrivate::get(inspector());
    QList<QGraphicsItem*> itemList = inspectorPrivate->selectableItems(event->pos());

    if (itemList.isEmpty())
        return;

    int selectedIdx = 0;
    if (!inspector()->selectedItems().isEmpty()) {
        selectedIdx = itemList.indexOf(inspector()->selectedItems().first());
        if (selectedIdx >= 0) {
            if (event->delta() > 0) {
                selectedIdx++;
                if (selectedIdx == itemList.length())
                    selectedIdx = 0;
            } else if (event->delta() < 0) {
                selectedIdx--;
                if (selectedIdx == -1)
                    selectedIdx = itemList.length() - 1;
            }
        } else {
            selectedIdx = 0;
        }
    }

    QPointF updatePt(0, 0);
    m_singleSelectionManipulator.begin(updatePt);
    m_singleSelectionManipulator.select(LiveSingleSelectionManipulator::ReplaceSelection,
                                        QList<QGraphicsItem*>() << itemList.at(selectedIdx),
                                        false);
    m_singleSelectionManipulator.end(updatePt);

}

void LiveSelectionTool::setSelectOnlyContentItems(bool selectOnlyContentItems)
{
    m_selectOnlyContentItems = selectOnlyContentItems;
}

void LiveSelectionTool::clear()
{
#ifndef QT_NO_CURSOR
    view()->setCursor(Qt::ArrowCursor);
#endif
    m_rubberbandSelectionManipulator.clear(),
            m_singleSelectionManipulator.clear();
    m_selectionIndicator.clear();
    //m_resizeIndicator.clear();
}

void LiveSelectionTool::selectedItemsChanged(const QList<QGraphicsItem*> &itemList)
{
    foreach (const QWeakPointer<QGraphicsObject> &obj, m_selectedItemList) {
        if (!obj.isNull()) {
            disconnect(obj.data(), SIGNAL(xChanged()), this, SLOT(repaintBoundingRects()));
            disconnect(obj.data(), SIGNAL(yChanged()), this, SLOT(repaintBoundingRects()));
            disconnect(obj.data(), SIGNAL(widthChanged()), this, SLOT(repaintBoundingRects()));
            disconnect(obj.data(), SIGNAL(heightChanged()), this, SLOT(repaintBoundingRects()));
            disconnect(obj.data(), SIGNAL(rotationChanged()), this, SLOT(repaintBoundingRects()));
        }
    }

    QList<QGraphicsObject*> objects = toGraphicsObjectList(itemList);
    m_selectedItemList.clear();

    foreach (QGraphicsObject *obj, objects) {
        m_selectedItemList.append(obj);
        connect(obj, SIGNAL(xChanged()), this, SLOT(repaintBoundingRects()));
        connect(obj, SIGNAL(yChanged()), this, SLOT(repaintBoundingRects()));
        connect(obj, SIGNAL(widthChanged()), this, SLOT(repaintBoundingRects()));
        connect(obj, SIGNAL(heightChanged()), this, SLOT(repaintBoundingRects()));
        connect(obj, SIGNAL(rotationChanged()), this, SLOT(repaintBoundingRects()));
    }

    m_selectionIndicator.setItems(m_selectedItemList);
    //m_resizeIndicator.setItems(toGraphicsObjectList(itemList));
}

void LiveSelectionTool::repaintBoundingRects()
{
    m_selectionIndicator.setItems(m_selectedItemList);
}

void LiveSelectionTool::selectUnderPoint(QMouseEvent *event)
{
    m_singleSelectionManipulator.begin(event->pos());

    if (event->modifiers().testFlag(Qt::ControlModifier))
        m_singleSelectionManipulator.select(LiveSingleSelectionManipulator::RemoveFromSelection,
                                            m_selectOnlyContentItems);
    else if (event->modifiers().testFlag(Qt::ShiftModifier))
        m_singleSelectionManipulator.select(LiveSingleSelectionManipulator::AddToSelection,
                                            m_selectOnlyContentItems);
    else
        m_singleSelectionManipulator.select(LiveSingleSelectionManipulator::InvertSelection,
                                            m_selectOnlyContentItems);

    m_singleSelectionManipulator.end(event->pos());
}

} // namespace QmlJSDebugger
