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

#ifndef QFONTSUBSET_P_H
#define QFONTSUBSET_P_H

#include <qfontengine_p.h>

#ifndef QT_NO_PRINTER

QT_BEGIN_NAMESPACE

class QFontSubset
{
 public:
   QFontSubset(QFontEngine *fe, int obj_id = 0)
      : object_id(obj_id), noEmbed(false), fontEngine(fe), downloaded_glyphs(0), standard_font(false) {
      fontEngine->ref.ref();
      addGlyph(0);
   }
   ~QFontSubset() {
      if (!fontEngine->ref.deref() && fontEngine->cache_count == 0) {
         delete fontEngine;
      }
   }

   QByteArray toTruetype() const;
   QByteArray toType1() const;
   QByteArray type1AddedGlyphs() const;
   QByteArray widthArray() const;
   QByteArray createToUnicodeMap() const;
   QVector<int> getReverseMap() const;
   QByteArray glyphName(unsigned int glyph, const QVector<int> reverseMap) const;

   static QByteArray glyphName(unsigned short unicode, bool symbol);

   int addGlyph(int index);
   const int object_id;
   bool noEmbed;
   QFontEngine *fontEngine;
   QList<int> glyph_indices;
   mutable int downloaded_glyphs;
   mutable bool standard_font;
   int nGlyphs() const {
      return glyph_indices.size();
   }
   mutable QFixed emSquare;
   mutable QVector<QFixed> widths;
};

QT_END_NAMESPACE

#endif // QT_NO_PRINTER

#endif // QFONTSUBSET_P_H
