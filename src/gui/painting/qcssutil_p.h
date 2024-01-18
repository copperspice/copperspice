/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
*
* Copyright (c) 2015 The Qt Company Ltd.
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

#ifndef QCSSUTIL_P_H
#define QCSSUTIL_P_H

#include <qglobal.h>

#ifndef QT_NO_CSSPARSER

#include <qcssparser_p.h>
#include <qsize.h>



class QPainter;

extern void qDrawEdge(QPainter *p, qreal x1, qreal y1, qreal x2, qreal y2, qreal dw1, qreal dw2,
   QCss::Edge edge, QCss::BorderStyle style, QBrush c);

extern void qDrawRoundedCorners(QPainter *p, qreal x1, qreal y1, qreal x2, qreal y2,
   const QSizeF &r1, const QSizeF &r2,
   QCss::Edge edge, QCss::BorderStyle s, QBrush c);

extern void qDrawBorder(QPainter *p, const QRect &rect, const QCss::BorderStyle *styles,
   const int *borders, const QBrush *colors, const QSize *radii);

extern void qNormalizeRadii(const QRect &br, const QSize *radii,
   QSize *tlr, QSize *trr, QSize *blr, QSize *brr);


#endif //QT_NO_CSSPARSER

#endif // QCSSUTIL_P_H
