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

#ifndef QDECLARATIVEVIEWINSPECTOR_P_H
#define QDECLARATIVEVIEWINSPECTOR_P_H

#include <qdeclarativeviewinspector.h>
#include <QtCore/QWeakPointer>
#include <QtCore/QPointF>
#include <qdeclarativeinspectorservice_p.h>

namespace QmlJSDebugger {

class QDeclarativeViewInspector;
class LiveSelectionTool;
class ZoomTool;
class ColorPickerTool;
class LiveLayerItem;
class BoundingRectHighlighter;
class AbstractLiveEditTool;

class QDeclarativeViewInspectorPrivate : public QObject
{
    Q_OBJECT
public:
    QDeclarativeViewInspectorPrivate(QDeclarativeViewInspector *);
    ~QDeclarativeViewInspectorPrivate();

    QDeclarativeView *view;
    QDeclarativeViewInspector *q;
    QWeakPointer<QWidget> viewport;

    QList<QWeakPointer<QGraphicsObject> > currentSelection;

    LiveSelectionTool *selectionTool;
    ZoomTool *zoomTool;
    ColorPickerTool *colorPickerTool;
    LiveLayerItem *manipulatorLayer;

    BoundingRectHighlighter *boundingRectHighlighter;

    void setViewport(QWidget *widget);

    void clearEditorItems();
    void changeToSelectTool();
    QList<QGraphicsItem*> filterForSelection(QList<QGraphicsItem*> &itemlist) const;

    QList<QGraphicsItem*> selectableItems(const QPoint &pos) const;
    QList<QGraphicsItem*> selectableItems(const QPointF &scenePos) const;
    QList<QGraphicsItem*> selectableItems(const QRectF &sceneRect, Qt::ItemSelectionMode selectionMode) const;

    void setSelectedItemsForTools(const QList<QGraphicsItem *> &items);
    void setSelectedItems(const QList<QGraphicsItem *> &items);
    QList<QGraphicsItem *> selectedItems() const;

    void clearHighlight();
    void highlight(const QList<QGraphicsObject *> &item);
    inline void highlight(QGraphicsObject *item)
    { highlight(QList<QGraphicsObject*>() << item); }

    void changeToSingleSelectTool();
    void changeToMarqueeSelectTool();
    void changeToZoomTool();
    void changeToColorPickerTool();

public slots:
    void _q_onStatusChanged(QDeclarativeView::Status status);

    void _q_removeFromSelection(QObject *);

public:
    static QDeclarativeViewInspectorPrivate *get(QDeclarativeViewInspector *v) { return v->d_func(); }
};

} // namespace QmlJSDebugger

#endif // QDECLARATIVEVIEWINSPECTOR_P_H
