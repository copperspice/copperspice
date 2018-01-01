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

#ifndef COLORPICKERTOOL_H
#define COLORPICKERTOOL_H

#include "abstractliveedittool.h"

#include <QtGui/QColor>

QT_FORWARD_DECLARE_CLASS(QPoint)

namespace QmlJSDebugger {

class ColorPickerTool : public AbstractLiveEditTool
{
    Q_OBJECT
public:
    explicit ColorPickerTool(QDeclarativeViewInspector *view);

    virtual ~ColorPickerTool();

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *) {}
    void mouseDoubleClickEvent(QMouseEvent *) {}

    void hoverMoveEvent(QMouseEvent *) {}

    void keyPressEvent(QKeyEvent *) {}
    void keyReleaseEvent(QKeyEvent *) {}

    void wheelEvent(QWheelEvent *) {}

    void itemsAboutToRemoved(const QList<QGraphicsItem*> &) {}

    void clear();

signals:
    void selectedColorChanged(const QColor &color);

protected:
    void selectedItemsChanged(const QList<QGraphicsItem*> &) {}

private:
    void pickColor(const QPoint &pos);

private:
    QColor m_selectedColor;
};

} // namespace QmlJSDebugger

#endif // COLORPICKERTOOL_H
