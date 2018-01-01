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

#ifndef QGLYPHRUN_P_H
#define QGLYPHRUN_P_H

#include <qglyphrun.h>
#include <qrawfont.h>
#include <qfont.h>

#if !defined(QT_NO_RAWFONT)

QT_BEGIN_NAMESPACE

class QGlyphRunPrivate: public QSharedData
{
 public:
   QGlyphRunPrivate()
      : overline(false)
      , underline(false)
      , strikeOut(false)
      , glyphIndexData(glyphIndexes.constData())
      , glyphIndexDataSize(0)
      , glyphPositionData(glyphPositions.constData())
      , glyphPositionDataSize(0) {
   }

   QGlyphRunPrivate(const QGlyphRunPrivate &other)
      : QSharedData(other)
      , glyphIndexes(other.glyphIndexes)
      , glyphPositions(other.glyphPositions)
      , rawFont(other.rawFont)
      , overline(other.overline)
      , underline(other.underline)
      , strikeOut(other.strikeOut)
      , glyphIndexData(other.glyphIndexData)
      , glyphIndexDataSize(other.glyphIndexDataSize)
      , glyphPositionData(other.glyphPositionData)
      , glyphPositionDataSize(other.glyphPositionDataSize) {
   }

   QVector<quint32> glyphIndexes;
   QVector<QPointF> glyphPositions;
   QRawFont rawFont;

   uint overline  : 1;
   uint underline : 1;
   uint strikeOut : 1;

   const quint32 *glyphIndexData;
   int glyphIndexDataSize;

   const QPointF *glyphPositionData;
   int glyphPositionDataSize;

   static QGlyphRunPrivate *get(const QGlyphRun &glyphRun) {
      return glyphRun.d.data();
   }
};

QT_END_NAMESPACE

#endif // QGLYPHS_P_H

#endif // QT_NO_RAWFONT
