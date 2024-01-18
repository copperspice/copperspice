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

#ifndef QGLYPHRUN_P_H
#define QGLYPHRUN_P_H

#include <qglyphrun.h>
#include <qrawfont.h>
#include <qfont.h>

class QGlyphRunPrivate: public QSharedData
{
 public:
   QGlyphRunPrivate()
      : flags(Qt::EmptyFlag), glyphIndexData(glyphIndexes.constData()), glyphIndexDataSize(0),
        glyphPositionData(glyphPositions.constData()), glyphPositionDataSize(0),
        textRangeStart(-1), textRangeEnd(-1)
   {
   }

   QGlyphRunPrivate(const QGlyphRunPrivate &other)
      : QSharedData(other)
      , glyphIndexes(other.glyphIndexes)
      , glyphPositions(other.glyphPositions)
      , rawFont(other.rawFont)
      , boundingRect(other.boundingRect)
      , flags(other.flags)
      , glyphIndexData(other.glyphIndexData)
      , glyphIndexDataSize(other.glyphIndexDataSize)
      , glyphPositionData(other.glyphPositionData)
      , glyphPositionDataSize(other.glyphPositionDataSize)
      , textRangeStart(other.textRangeStart)
      , textRangeEnd(other.textRangeEnd)
   { }

   QVector<quint32> glyphIndexes;
   QVector<QPointF> glyphPositions;
   QRawFont rawFont;
   QRectF boundingRect;

   QGlyphRun::GlyphRunFlags flags;

   const quint32 *glyphIndexData;
   int glyphIndexDataSize;

   const QPointF *glyphPositionData;
   int glyphPositionDataSize;

   int textRangeStart;
   int textRangeEnd;

   static QGlyphRunPrivate *get(const QGlyphRun &glyphRun) {
      return glyphRun.d.data();
   }
};

#endif
