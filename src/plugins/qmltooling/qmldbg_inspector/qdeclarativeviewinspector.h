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

#ifndef QDECLARATIVEVIEWINSPECTOR_H
#define QDECLARATIVEVIEWINSPECTOR_H

#include <qdeclarativeglobal_p.h>
#include <qmlinspectorconstants.h>
#include <abstractviewinspector.h>
#include <QtCore/QScopedPointer>
#include <QtDeclarative/QDeclarativeView>

namespace QmlJSDebugger {

class AbstractLiveEditTool;
class QDeclarativeViewInspectorPrivate;

class QDeclarativeViewInspector : public AbstractViewInspector
{
    Q_OBJECT

public:
    explicit QDeclarativeViewInspector(QDeclarativeView *view, QObject *parent = nullptr);
    ~QDeclarativeViewInspector();

    // AbstractViewInspector
    void changeCurrentObjects(const QList<QObject*> &objects);
    void reloadView();
    void reparentQmlObject(QObject *object, QObject *newParent);
    void changeTool(InspectorProtocol::Tool tool);
    QWidget *viewWidget() const { return declarativeView(); }
    QDeclarativeEngine *declarativeEngine() const;

    void setSelectedItems(QList<QGraphicsItem *> items);
    QList<QGraphicsItem *> selectedItems() const;

    QDeclarativeView *declarativeView() const;

    QRectF adjustToScreenBoundaries(const QRectF &boundingRectInSceneSpace);

protected:
    bool eventFilter(QObject *obj, QEvent *event);

    bool leaveEvent(QEvent *);
    bool mouseMoveEvent(QMouseEvent *event);

    AbstractLiveEditTool *currentTool() const;

private:
    Q_DISABLE_COPY(QDeclarativeViewInspector)

    inline QDeclarativeViewInspectorPrivate *d_func() { return data.data(); }
    QScopedPointer<QDeclarativeViewInspectorPrivate> data;
    friend class QDeclarativeViewInspectorPrivate;
    friend class AbstractLiveEditTool;
};

} // namespace QmlJSDebugger

#endif // QDECLARATIVEVIEWINSPECTOR_H
