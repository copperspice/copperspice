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

#include <QtCore/qglobal.h>
#include <QtCore/qpoint.h>
#include <QtCore/qstring.h>
#include <QtGui/qpolygon.h>

#ifndef QSTYLEHELPER_P_H
#define QSTYLEHELPER_P_H

#include <qhexstring_p.h>

class QPainter;
class QPixmap;
class QStyleOptionSlider;
class QStyleOption;

namespace QStyleHelper {
QString uniqueName(const QString &key, const QStyleOption *option, const QSize &size);
qreal dpiScaled(qreal value);

#ifndef QT_NO_DIAL
qreal angle(const QPointF &p1, const QPointF &p2);
QPolygonF calcLines(const QStyleOptionSlider *dial);
int calcBigLineSize(int radius);
void drawDial(const QStyleOptionSlider *dial, QPainter *painter);
#endif

void drawBorderPixmap(const QPixmap &pixmap, QPainter *painter, const QRect &rect, int left = 0, int top = 0, int right = 0, int bottom = 0);
}


#endif // QSTYLEHELPER_P_H
