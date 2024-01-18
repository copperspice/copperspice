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

#ifndef QPICTURE_P_H
#define QPICTURE_P_H

#include <qatomic.h>
#include <qbuffer.h>
#include <qobject.h>
#include <qvector.h>
#include <qpicture.h>
#include <qpixmap.h>
#include <qpen.h>
#include <qbrush.h>
#include <qrect.h>



class QPaintEngine;

extern const char  *qt_mfhdr_tag;

class QPicturePrivate
{
   friend class QPicturePaintEngine;
   friend Q_GUI_EXPORT QDataStream &operator<<(QDataStream &stream, const QPicture &picture);
   friend Q_GUI_EXPORT QDataStream &operator>>(QDataStream &stream, QPicture &picture);

 public:
   enum PaintCommand {
      PdcNOP = 0, //  <void>
      PdcDrawPoint = 1, // point
      PdcDrawFirst = PdcDrawPoint,
      PdcMoveTo = 2, // point
      PdcLineTo = 3, // point
      PdcDrawLine = 4, // point,point
      PdcDrawRect = 5, // rect
      PdcDrawRoundRect = 6, // rect,ival,ival
      PdcDrawEllipse = 7, // rect
      PdcDrawArc = 8, // rect,ival,ival
      PdcDrawPie = 9, // rect,ival,ival
      PdcDrawChord = 10, // rect,ival,ival
      PdcDrawLineSegments = 11, // ptarr
      PdcDrawPolyline = 12, // ptarr
      PdcDrawPolygon = 13, // ptarr,ival
      PdcDrawCubicBezier = 14, // ptarr
      PdcDrawText = 15, // point,str
      PdcDrawTextFormatted = 16, // rect,ival,str
      PdcDrawPixmap = 17, // rect,pixmap
      PdcDrawImage = 18, // rect,image
      PdcDrawText2 = 19, // point,str
      PdcDrawText2Formatted = 20, // rect,ival,str
      PdcDrawTextItem = 21, // pos,text,font,flags
      PdcDrawLast = PdcDrawTextItem,
      PdcDrawPoints = 22, // ptarr,ival,ival
      PdcDrawWinFocusRect = 23, // rect,color
      PdcDrawTiledPixmap = 24, // rect,pixmap,point
      PdcDrawPath = 25, // path

      // no painting commands below PdcDrawLast.

      PdcBegin = 30, //  <void>
      PdcEnd = 31, //  <void>
      PdcSave = 32, //  <void>
      PdcRestore = 33, //  <void>
      PdcSetdev = 34, // device - PRIVATE
      PdcSetBkColor = 40, // color
      PdcSetBkMode = 41, // ival
      PdcSetROP = 42, // ival
      PdcSetBrushOrigin = 43, // point
      PdcSetFont = 45, // font
      PdcSetPen = 46, // pen
      PdcSetBrush = 47, // brush
      PdcSetTabStops = 48, // ival
      PdcSetTabArray = 49, // ival,ivec
      PdcSetUnit = 50, // ival
      PdcSetVXform = 51, // ival
      PdcSetWindow = 52, // rect
      PdcSetViewport = 53, // rect
      PdcSetWXform = 54, // ival
      PdcSetWMatrix = 55, // matrix,ival
      PdcSaveWMatrix = 56,
      PdcRestoreWMatrix = 57,
      PdcSetClip = 60, // ival
      PdcSetClipRegion = 61, // rgn
      PdcSetClipPath = 62, // path
      PdcSetRenderHint = 63, // ival
      PdcSetCompositionMode = 64, // ival
      PdcSetClipEnabled = 65, // bool
      PdcSetOpacity = 66, // qreal

      PdcReservedStart = 0, // codes 0-199 are reserved
      PdcReservedStop = 199 //   for Qt
   };

   QPicturePrivate();
   QPicturePrivate(const QPicturePrivate &other);
   QAtomicInt ref;

   bool checkFormat();
   void resetFormat();

   QBuffer pictb;
   int trecs;
   bool formatOk;
   int formatMajor;
   int formatMinor;
   QRect brect;
   QRect override_rect;
   QScopedPointer<QPaintEngine> paintEngine;
   bool in_memory_only;

   QVector<QImage> image_list;
   QVector<QPixmap> pixmap_list;
   QList<QBrush> brush_list;
   QList<QPen> pen_list;
};



#endif
