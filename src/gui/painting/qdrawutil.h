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

#ifndef QDRAWUTIL_H
#define QDRAWUTIL_H

#include <QtCore/qnamespace.h>
#include <QtCore/qstring.h>       // char*->QString conversion
#include <QtCore/qmargins.h>
#include <QtGui/qpixmap.h>

QT_BEGIN_NAMESPACE

class QPainter;
class QColorGroup;
class QPalette;
class QPoint;
class QColor;
class QBrush;
class QRect;

//
// Standard shade drawing
//

Q_GUI_EXPORT void qDrawShadeLine(QPainter *p, int x1, int y1, int x2, int y2,
                                 const QPalette &pal, bool sunken = true,
                                 int lineWidth = 1, int midLineWidth = 0);

Q_GUI_EXPORT void qDrawShadeLine(QPainter *p, const QPoint &p1, const QPoint &p2,
                                 const QPalette &pal, bool sunken = true,
                                 int lineWidth = 1, int midLineWidth = 0);

Q_GUI_EXPORT void qDrawShadeRect(QPainter *p, int x, int y, int w, int h,
                                 const QPalette &pal, bool sunken = false,
                                 int lineWidth = 1, int midLineWidth = 0,
                                 const QBrush *fill = 0);

Q_GUI_EXPORT void qDrawShadeRect(QPainter *p, const QRect &r,
                                 const QPalette &pal, bool sunken = false,
                                 int lineWidth = 1, int midLineWidth = 0,
                                 const QBrush *fill = 0);

Q_GUI_EXPORT void qDrawShadePanel(QPainter *p, int x, int y, int w, int h,
                                  const QPalette &pal, bool sunken = false,
                                  int lineWidth = 1, const QBrush *fill = 0);

Q_GUI_EXPORT void qDrawShadePanel(QPainter *p, const QRect &r,
                                  const QPalette &pal, bool sunken = false,
                                  int lineWidth = 1, const QBrush *fill = 0);

Q_GUI_EXPORT void qDrawWinButton(QPainter *p, int x, int y, int w, int h,
                                 const QPalette &pal, bool sunken = false,
                                 const QBrush *fill = 0);

Q_GUI_EXPORT void qDrawWinButton(QPainter *p, const QRect &r,
                                 const QPalette &pal, bool sunken = false,
                                 const QBrush *fill = 0);

Q_GUI_EXPORT void qDrawWinPanel(QPainter *p, int x, int y, int w, int h,
                                const QPalette &pal, bool sunken = false,
                                const QBrush *fill = 0);

Q_GUI_EXPORT void qDrawWinPanel(QPainter *p, const QRect &r,
                                const QPalette &pal, bool sunken = false,
                                const QBrush *fill = 0);

Q_GUI_EXPORT void qDrawPlainRect(QPainter *p, int x, int y, int w, int h, const QColor &,
                                 int lineWidth = 1, const QBrush *fill = 0);

Q_GUI_EXPORT void qDrawPlainRect(QPainter *p, const QRect &r, const QColor &,
                                 int lineWidth = 1, const QBrush *fill = 0);

struct QTileRules {
   inline QTileRules(Qt::TileRule horizontalRule, Qt::TileRule verticalRule)
      : horizontal(horizontalRule), vertical(verticalRule) {}

   inline QTileRules(Qt::TileRule rule = Qt::StretchTile)
      : horizontal(rule), vertical(rule) {}
   Qt::TileRule horizontal;
   Qt::TileRule vertical;
};


// For internal use only.
namespace QDrawBorderPixmap {
enum DrawingHint {
   OpaqueTopLeft = 0x0001,
   OpaqueTop = 0x0002,
   OpaqueTopRight = 0x0004,
   OpaqueLeft = 0x0008,
   OpaqueCenter = 0x0010,
   OpaqueRight = 0x0020,
   OpaqueBottomLeft = 0x0040,
   OpaqueBottom = 0x0080,
   OpaqueBottomRight = 0x0100,
   OpaqueCorners = OpaqueTopLeft | OpaqueTopRight | OpaqueBottomLeft | OpaqueBottomRight,
   OpaqueEdges = OpaqueTop | OpaqueLeft | OpaqueRight | OpaqueBottom,
   OpaqueFrame = OpaqueCorners | OpaqueEdges,
   OpaqueAll = OpaqueCenter | OpaqueFrame
};

using DrawingHints = QFlags<DrawingHint>;
}


Q_GUI_EXPORT void qDrawBorderPixmap(QPainter *painter,
                                    const QRect &targetRect,
                                    const QMargins &targetMargins,
                                    const QPixmap &pixmap,
                                    const QRect &sourceRect,
                                    const QMargins &sourceMargins,
                                    const QTileRules &rules = QTileRules()
                                          , QDrawBorderPixmap::DrawingHints hints = 0

                                   );

inline void qDrawBorderPixmap(QPainter *painter,
                              const QRect &target,
                              const QMargins &margins,
                              const QPixmap &pixmap)
{
   qDrawBorderPixmap(painter, target, margins, pixmap, pixmap.rect(), margins);
}

QT_END_NAMESPACE

#endif // QDRAWUTIL_H
