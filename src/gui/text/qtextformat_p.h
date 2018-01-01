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

#ifndef QTEXTFORMAT_P_H
#define QTEXTFORMAT_P_H

#include <qtextformat.h>
#include <qmultihash.h>
#include <qvector.h>

QT_BEGIN_NAMESPACE

class Q_GUI_EXPORT QTextFormatCollection
{
 public:
   QTextFormatCollection() {}
   ~QTextFormatCollection();

   QTextFormatCollection(const QTextFormatCollection &rhs);
   QTextFormatCollection &operator=(const QTextFormatCollection &rhs);

   QTextFormat objectFormat(int objectIndex) const;
   void setObjectFormat(int objectIndex, const QTextFormat &format);

   int objectFormatIndex(int objectIndex) const;
   void setObjectFormatIndex(int objectIndex, int formatIndex);

   int createObjectIndex(const QTextFormat &f);

   int indexForFormat(const QTextFormat &f);
   bool hasFormatCached(const QTextFormat &format) const;

   QTextFormat format(int idx) const;
   inline QTextBlockFormat blockFormat(int index) const {
      return format(index).toBlockFormat();
   }
   inline QTextCharFormat charFormat(int index) const {
      return format(index).toCharFormat();
   }
   inline QTextListFormat listFormat(int index) const {
      return format(index).toListFormat();
   }
   inline QTextTableFormat tableFormat(int index) const {
      return format(index).toTableFormat();
   }
   inline QTextImageFormat imageFormat(int index) const {
      return format(index).toImageFormat();
   }

   inline int numFormats() const {
      return formats.count();
   }

   typedef QVector<QTextFormat> FormatVector;

   FormatVector formats;
   QVector<qint32> objFormats;
   QMultiHash<uint, int> hashes;

   inline QFont defaultFont() const {
      return defaultFnt;
   }
   void setDefaultFont(const QFont &f);

 private:
   QFont defaultFnt;
};

QT_END_NAMESPACE

#endif // QTEXTFORMAT_P_H
