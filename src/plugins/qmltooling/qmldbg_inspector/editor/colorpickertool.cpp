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

#include "colorpickertool.h"

#include "../qdeclarativeviewinspector.h"

#include <QtGui/QMouseEvent>
#include <QtGui/QKeyEvent>
#include <QtCore/QRectF>
#include <QtGui/QRgb>
#include <QtGui/QImage>
#include <QtGui/QApplication>
#include <QtGui/QPalette>

namespace QmlJSDebugger {

ColorPickerTool::ColorPickerTool(QDeclarativeViewInspector *view) :
    AbstractLiveEditTool(view)
{
    m_selectedColor.setRgb(0,0,0);
}

ColorPickerTool::~ColorPickerTool()
{
}

void ColorPickerTool::mousePressEvent(QMouseEvent *event)
{
    pickColor(event->pos());
}

void ColorPickerTool::mouseMoveEvent(QMouseEvent *event)
{
    pickColor(event->pos());
}

void ColorPickerTool::clear()
{
#ifndef QT_NO_CURSOR
    view()->setCursor(Qt::CrossCursor);
#endif
}

void ColorPickerTool::pickColor(const QPoint &pos)
{
    QRgb fillColor = view()->backgroundBrush().color().rgb();
    if (view()->backgroundBrush().style() == Qt::NoBrush)
        fillColor = view()->palette().color(QPalette::Base).rgb();

    QRectF target(0,0, 1, 1);
    QRect source(pos.x(), pos.y(), 1, 1);
    QImage img(1, 1, QImage::Format_ARGB32);
    img.fill(fillColor);
    QPainter painter(&img);
    view()->render(&painter, target, source);
    m_selectedColor = QColor::fromRgb(img.pixel(0, 0));

    emit selectedColorChanged(m_selectedColor);
}

} // namespace QmlJSDebugger
