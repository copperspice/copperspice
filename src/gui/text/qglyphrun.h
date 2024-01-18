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

#ifndef QGLYPHRUN_H
#define QGLYPHRUN_H

#include <qsharedpointer.h>
#include <qvector.h>
#include <qpoint.h>
#include <qrawfont.h>

class QGlyphRunPrivate;

class Q_GUI_EXPORT QGlyphRun
{

 public:
   enum GlyphRunFlag {
      Overline        = 0x01,
      Underline       = 0x02,
      StrikeOut       = 0x04,
      RightToLeft     = 0x08,
      SplitLigature   = 0x10
   };

   using GlyphRunFlags = QFlags<GlyphRunFlag>;

   QGlyphRun();
   QGlyphRun(const QGlyphRun &other);

   ~QGlyphRun();


   QRawFont rawFont() const;
   void setRawFont(const QRawFont &rawFont);

   void setRawData(const quint32 *glyphIndexArray, const QPointF *glyphPositionArray, int size);

   QVector<quint32> glyphIndexes() const;
   void setGlyphIndexes(const QVector<quint32> &glyphIndexes);

   QVector<QPointF> positions() const;
   void setPositions(const QVector<QPointF> &positions);

   void clear();

   void swap(QGlyphRun &other) {
      qSwap(d, other.d);
   }

   QGlyphRun &operator=(const QGlyphRun &other);

   QGlyphRun &operator=(QGlyphRun &&other) {
      swap(other);
      return *this;
   }

   bool operator==(const QGlyphRun &other) const;
   inline bool operator!=(const QGlyphRun &other) const {
      return !operator==(other);
   }

   void setOverline(bool overline);
   bool overline() const;

   void setUnderline(bool underline);
   bool underline() const;

   void setStrikeOut(bool strikeOut);
   bool strikeOut() const;

   void setRightToLeft(bool rightToLeft);
   bool isRightToLeft() const;

   void setFlag(GlyphRunFlag flag, bool enabled = true);
   void setFlags(GlyphRunFlags flags);
   GlyphRunFlags flags() const;

   void setBoundingRect(const QRectF &boundingRect);
   QRectF boundingRect() const;

   bool isEmpty() const;

 private:
   friend class QGlyphRunPrivate;
   friend class QTextLine;

   QGlyphRun operator+(const QGlyphRun &other) const;
   QGlyphRun &operator+=(const QGlyphRun &other);

   void detach();
   QExplicitlySharedDataPointer<QGlyphRunPrivate> d;
};

#endif
