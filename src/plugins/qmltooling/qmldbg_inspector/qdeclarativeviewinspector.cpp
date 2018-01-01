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

#include "qdeclarativeviewinspector.h"
#include "qdeclarativeviewinspector_p.h"

#include "editor/liveselectiontool.h"
#include "editor/zoomtool.h"
#include "editor/colorpickertool.h"
#include "editor/livelayeritem.h"
#include "editor/boundingrecthighlighter.h"

#include <QtDeclarative/QDeclarativeItem>
#include <QtGui/QWidget>
#include <QtGui/QMouseEvent>
#include <QtGui/QGraphicsObject>

namespace QmlJSDebugger {

QDeclarativeViewInspectorPrivate::QDeclarativeViewInspectorPrivate(QDeclarativeViewInspector *q) :
    q(q)
{
}

QDeclarativeViewInspectorPrivate::~QDeclarativeViewInspectorPrivate()
{
}

QDeclarativeViewInspector::QDeclarativeViewInspector(QDeclarativeView *view,
                                                   QObject *parent) :
    AbstractViewInspector(parent),
    data(new QDeclarativeViewInspectorPrivate(this))
{
    data->view = view;
    data->manipulatorLayer = new LiveLayerItem(view->scene());
    data->selectionTool = new LiveSelectionTool(this);
    data->zoomTool = new ZoomTool(this);
    data->colorPickerTool = new ColorPickerTool(this);
    data->boundingRectHighlighter = new BoundingRectHighlighter(this);
    setCurrentTool(data->selectionTool);

    // to capture ChildRemoved event when viewport changes
    data->view->installEventFilter(this);

    data->setViewport(data->view->viewport());

    connect(data->view, SIGNAL(statusChanged(QDeclarativeView::Status)),
            data.data(), SLOT(_q_onStatusChanged(QDeclarativeView::Status)));

    connect(data->colorPickerTool, SIGNAL(selectedColorChanged(QColor)),
            SIGNAL(selectedColorChanged(QColor)));
    connect(data->colorPickerTool, SIGNAL(selectedColorChanged(QColor)),
            this, SLOT(sendColorChanged(QColor)));

    changeTool(InspectorProtocol::SelectTool);
}

QDeclarativeViewInspector::~QDeclarativeViewInspector()
{
}

void QDeclarativeViewInspector::changeCurrentObjects(const QList<QObject*> &objects)
{
    QList<QGraphicsItem*> items;
    QList<QGraphicsObject*> gfxObjects;
    foreach (QObject *obj, objects) {
        if (QDeclarativeItem *declarativeItem = qobject_cast<QDeclarativeItem*>(obj)) {
            items << declarativeItem;
            gfxObjects << declarativeItem;
        }
    }
    if (designModeBehavior()) {
        data->setSelectedItemsForTools(items);
        data->clearHighlight();
        data->highlight(gfxObjects);
    }
}

void QDeclarativeViewInspector::reloadView()
{
    data->clearHighlight();
    emit reloadRequested();
}

void QDeclarativeViewInspector::changeTool(InspectorProtocol::Tool tool)
{
    switch (tool) {
    case InspectorProtocol::ColorPickerTool:
        data->changeToColorPickerTool();
        break;
    case InspectorProtocol::SelectMarqueeTool:
        data->changeToMarqueeSelectTool();
        break;
    case InspectorProtocol::SelectTool:
        data->changeToSingleSelectTool();
        break;
    case InspectorProtocol::ZoomTool:
        data->changeToZoomTool();
        break;
    }
}

AbstractLiveEditTool *QDeclarativeViewInspector::currentTool() const
{
    return static_cast<AbstractLiveEditTool*>(AbstractViewInspector::currentTool());
}

QDeclarativeEngine *QDeclarativeViewInspector::declarativeEngine() const
{
    return data->view->engine();
}

void QDeclarativeViewInspectorPrivate::setViewport(QWidget *widget)
{
    if (viewport.data() == widget)
        return;

    if (viewport)
        viewport.data()->removeEventFilter(q);

    viewport = widget;
    if (viewport) {
        // make sure we get mouse move events
        viewport.data()->setMouseTracking(true);
        viewport.data()->installEventFilter(q);
    }
}

void QDeclarativeViewInspectorPrivate::clearEditorItems()
{
    clearHighlight();
    setSelectedItems(QList<QGraphicsItem*>());
}

bool QDeclarativeViewInspector::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == data->view) {
        // Event from view
        if (event->type() == QEvent::ChildRemoved) {
            // Might mean that viewport has changed
            if (data->view->viewport() != data->viewport.data())
                data->setViewport(data->view->viewport());
        }
        return QObject::eventFilter(obj, event);
    }

    return AbstractViewInspector::eventFilter(obj, event);
}

bool QDeclarativeViewInspector::leaveEvent(QEvent *event)
{
    data->clearHighlight();
    return AbstractViewInspector::leaveEvent(event);
}

bool QDeclarativeViewInspector::mouseMoveEvent(QMouseEvent *event)
{
    QList<QGraphicsItem*> selItems = data->selectableItems(event->pos());
    if (!selItems.isEmpty()) {
        declarativeView()->setToolTip(currentTool()->titleForItem(selItems.first()));
    } else {
        declarativeView()->setToolTip(QString());
    }

    return AbstractViewInspector::mouseMoveEvent(event);
}

void QDeclarativeViewInspector::reparentQmlObject(QObject *object, QObject *newParent)
{
    if (!newParent)
        return;

    object->setParent(newParent);
    QDeclarativeItem *newParentItem = qobject_cast<QDeclarativeItem*>(newParent);
    QDeclarativeItem *item    = qobject_cast<QDeclarativeItem*>(object);
    if (newParentItem && item)
        item->setParentItem(newParentItem);
}

void QDeclarativeViewInspectorPrivate::_q_removeFromSelection(QObject *obj)
{
    QList<QGraphicsItem*> items = selectedItems();
    if (QGraphicsItem *item = qobject_cast<QGraphicsObject*>(obj))
        items.removeOne(item);
    setSelectedItems(items);
}

void QDeclarativeViewInspectorPrivate::setSelectedItemsForTools(const QList<QGraphicsItem *> &items)
{
    foreach (const QWeakPointer<QGraphicsObject> &obj, currentSelection) {
        if (QGraphicsItem *item = obj.data()) {
            if (!items.contains(item)) {
                QObject::disconnect(obj.data(), SIGNAL(destroyed(QObject*)),
                                    this, SLOT(_q_removeFromSelection(QObject*)));
                currentSelection.removeOne(obj);
            }
        }
    }

    foreach (QGraphicsItem *item, items) {
        if (QGraphicsObject *obj = item->toGraphicsObject()) {
            if (!currentSelection.contains(obj)) {
                QObject::connect(obj, SIGNAL(destroyed(QObject*)),
                                 this, SLOT(_q_removeFromSelection(QObject*)));
                currentSelection.append(obj);
            }
        }
    }

    q->currentTool()->updateSelectedItems();
}

void QDeclarativeViewInspectorPrivate::setSelectedItems(const QList<QGraphicsItem *> &items)
{
    QList<QWeakPointer<QGraphicsObject> > oldList = currentSelection;
    setSelectedItemsForTools(items);
    if (oldList != currentSelection) {
        QList<QObject*> objectList;
        foreach (const QWeakPointer<QGraphicsObject> &graphicsObject, currentSelection) {
            if (graphicsObject)
                objectList << graphicsObject.data();
        }

        q->sendCurrentObjects(objectList);
    }
}

QList<QGraphicsItem *> QDeclarativeViewInspectorPrivate::selectedItems() const
{
    QList<QGraphicsItem *> selection;
    foreach (const QWeakPointer<QGraphicsObject> &selectedObject, currentSelection) {
        if (selectedObject.data())
            selection << selectedObject.data();
    }

    return selection;
}

void QDeclarativeViewInspector::setSelectedItems(QList<QGraphicsItem *> items)
{
    data->setSelectedItems(items);
}

QList<QGraphicsItem *> QDeclarativeViewInspector::selectedItems() const
{
    return data->selectedItems();
}

QDeclarativeView *QDeclarativeViewInspector::declarativeView() const
{
    return data->view;
}

void QDeclarativeViewInspectorPrivate::clearHighlight()
{
    boundingRectHighlighter->clear();
}

void QDeclarativeViewInspectorPrivate::highlight(const QList<QGraphicsObject *> &items)
{
    if (items.isEmpty())
        return;

    QList<QGraphicsObject*> objectList;
    foreach (QGraphicsItem *item, items) {
        QGraphicsItem *child = item;

        if (child) {
            QGraphicsObject *childObject = child->toGraphicsObject();
            if (childObject)
                objectList << childObject;
        }
    }

    boundingRectHighlighter->highlight(objectList);
}

QList<QGraphicsItem*> QDeclarativeViewInspectorPrivate::selectableItems(
    const QPointF &scenePos) const
{
    QList<QGraphicsItem*> itemlist = view->scene()->items(scenePos);
    return filterForSelection(itemlist);
}

QList<QGraphicsItem*> QDeclarativeViewInspectorPrivate::selectableItems(const QPoint &pos) const
{
    QList<QGraphicsItem*> itemlist = view->items(pos);
    return filterForSelection(itemlist);
}

QList<QGraphicsItem*> QDeclarativeViewInspectorPrivate::selectableItems(
    const QRectF &sceneRect, Qt::ItemSelectionMode selectionMode) const
{
    QList<QGraphicsItem*> itemlist = view->scene()->items(sceneRect, selectionMode);
    return filterForSelection(itemlist);
}

void QDeclarativeViewInspectorPrivate::changeToSingleSelectTool()
{
    selectionTool->setRubberbandSelectionMode(false);

    changeToSelectTool();

    emit q->selectToolActivated();
    q->sendCurrentTool(Constants::SelectionToolMode);
}

void QDeclarativeViewInspectorPrivate::changeToSelectTool()
{
    if (q->currentTool() == selectionTool)
        return;

    q->currentTool()->clear();
    q->setCurrentTool(selectionTool);
    q->currentTool()->clear();
    q->currentTool()->updateSelectedItems();
}

void QDeclarativeViewInspectorPrivate::changeToMarqueeSelectTool()
{
    changeToSelectTool();
    selectionTool->setRubberbandSelectionMode(true);

    emit q->marqueeSelectToolActivated();
    q->sendCurrentTool(Constants::MarqueeSelectionToolMode);
}

void QDeclarativeViewInspectorPrivate::changeToZoomTool()
{
    q->currentTool()->clear();
    q->setCurrentTool(zoomTool);
    q->currentTool()->clear();

    emit q->zoomToolActivated();
    q->sendCurrentTool(Constants::ZoomMode);
}

void QDeclarativeViewInspectorPrivate::changeToColorPickerTool()
{
    if (q->currentTool() == colorPickerTool)
        return;

    q->currentTool()->clear();
    q->setCurrentTool(colorPickerTool);
    q->currentTool()->clear();

    emit q->colorPickerActivated();
    q->sendCurrentTool(Constants::ColorPickerMode);
}


static bool isEditorItem(QGraphicsItem *item)
{
    return (item->type() == Constants::EditorItemType
            || item->type() == Constants::ResizeHandleItemType
            || item->data(Constants::EditorItemDataKey).toBool());
}

QList<QGraphicsItem*> QDeclarativeViewInspectorPrivate::filterForSelection(
    QList<QGraphicsItem*> &itemlist) const
{
    foreach (QGraphicsItem *item, itemlist) {
        if (isEditorItem(item))
            itemlist.removeOne(item);
    }

    return itemlist;
}

void QDeclarativeViewInspectorPrivate::_q_onStatusChanged(QDeclarativeView::Status status)
{
    if (status == QDeclarativeView::Ready)
        q->sendReloaded();
}

// adjusts bounding boxes on edges of screen to be visible
QRectF QDeclarativeViewInspector::adjustToScreenBoundaries(const QRectF &boundingRectInSceneSpace)
{
    int marginFromEdge = 1;
    QRectF boundingRect(boundingRectInSceneSpace);
    if (qAbs(boundingRect.left()) - 1 < 2)
        boundingRect.setLeft(marginFromEdge);

    QRect rect = data->view->rect();

    if (boundingRect.right() >= rect.right())
        boundingRect.setRight(rect.right() - marginFromEdge);

    if (qAbs(boundingRect.top()) - 1 < 2)
        boundingRect.setTop(marginFromEdge);

    if (boundingRect.bottom() >= rect.bottom())
        boundingRect.setBottom(rect.bottom() - marginFromEdge);

    return boundingRect;
}

} // namespace QmlJSDebugger
