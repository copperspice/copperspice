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

#ifndef QDRAWUTIL_H
#define QDRAWUTIL_H

#include <qnamespace.h>
#include <qmargins.h>
#include <qpixmap.h>

class QPainter;
class QColorGroup;
class QPalette;
class QPoint;
class QColor;
class QBrush;
class QRect;

// Standard shade drawing

Q_GUI_EXPORT void qDrawShadeLine(QPainter *painter, int x1, int y1, int x2, int y2,
   const QPalette &palette, bool sunken = true, int lineWidth = 1, int midLineWidth = 0);

Q_GUI_EXPORT void qDrawShadeLine(QPainter *painter, const QPoint &p1, const QPoint &p2,
   const QPalette &palette, bool sunken = true, int lineWidth = 1, int midLineWidth = 0);

Q_GUI_EXPORT void qDrawShadeRect(QPainter *painter, int x, int y, int width, int height,
   const QPalette &palette, bool sunken = false, int lineWidth = 1, int midLineWidth = 0,
   const QBrush *fill = nullptr);

Q_GUI_EXPORT void qDrawShadeRect(QPainter *painter, const QRect &rect,
   const QPalette &palette, bool sunken = false, int lineWidth = 1, int midLineWidth = 0,
   const QBrush *fill = nullptr);

Q_GUI_EXPORT void qDrawShadePanel(QPainter *painter, int x, int y, int width, int height,
   const QPalette &palette, bool sunken = false, int lineWidth = 1, const QBrush *fill = nullptr);

Q_GUI_EXPORT void qDrawShadePanel(QPainter *painter, const QRect &rect,
   const QPalette &palette, bool sunken = false, int lineWidth = 1, const QBrush *fill = nullptr);

Q_GUI_EXPORT void qDrawWinButton(QPainter *painter, int x, int y, int width, int height,
   const QPalette &palette, bool sunken = false, const QBrush *fill = nullptr);

Q_GUI_EXPORT void qDrawWinButton(QPainter *painter, const QRect &rect,
   const QPalette &palette, bool sunken = false, const QBrush *fill = nullptr);

Q_GUI_EXPORT void qDrawWinPanel(QPainter *painter, int x, int y, int width, int height,
   const QPalette &palette, bool sunken = false, const QBrush *fill = nullptr);

Q_GUI_EXPORT void qDrawWinPanel(QPainter *painter, const QRect &rect,
   const QPalette &palette, bool sunken = false, const QBrush *fill = nullptr);

Q_GUI_EXPORT void qDrawPlainRect(QPainter *painter, int x, int y, int width, int height, const QColor &color,
   int lineWidth = 1, const QBrush *fill = nullptr);

Q_GUI_EXPORT void qDrawPlainRect(QPainter *painter, const QRect &rect, const QColor &color,
   int lineWidth = 1, const QBrush *fill = nullptr);

struct QTileRules {
   inline QTileRules(Qt::TileRule horizontalRule, Qt::TileRule verticalRule)
      : horizontal(horizontalRule), vertical(verticalRule)
   {
   }

   inline QTileRules(Qt::TileRule rule = Qt::StretchTile)
      : horizontal(rule), vertical(rule)
   {
   }

   Qt::TileRule horizontal;
   Qt::TileRule vertical;
};


// For internal use only
namespace QDrawBorderPixmap {

enum DrawingHint {
   OpaqueTopLeft     = 0x0001,
   OpaqueTop         = 0x0002,
   OpaqueTopRight    = 0x0004,
   OpaqueLeft        = 0x0008,
   OpaqueCenter      = 0x0010,
   OpaqueRight       = 0x0020,
   OpaqueBottomLeft  = 0x0040,
   OpaqueBottom      = 0x0080,
   OpaqueBottomRight = 0x0100,
   OpaqueCorners     = OpaqueTopLeft | OpaqueTopRight | OpaqueBottomLeft | OpaqueBottomRight,
   OpaqueEdges       = OpaqueTop | OpaqueLeft | OpaqueRight | OpaqueBottom,
   OpaqueFrame       = OpaqueCorners | OpaqueEdges,
   OpaqueAll         = OpaqueCenter | OpaqueFrame
};

using DrawingHints = QFlags<DrawingHint>;
}

Q_GUI_EXPORT void qDrawBorderPixmap(QPainter *painter,
   const QRect &targetRect,
   const QMargins &targetMargins,
   const QPixmap &pixmap,
   const QRect &sourceRect,
   const QMargins &sourceMargins,
   const QTileRules &rules = QTileRules(), QDrawBorderPixmap::DrawingHints hints = QDrawBorderPixmap::DrawingHints()

);

inline void qDrawBorderPixmap(QPainter *painter,
   const QRect &target,
   const QMargins &margins,
   const QPixmap &pixmap)
{
   qDrawBorderPixmap(painter, target, margins, pixmap, pixmap.rect(), margins);
}

#endif