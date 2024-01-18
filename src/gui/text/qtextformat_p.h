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

#ifndef QTEXTFORMAT_P_H
#define QTEXTFORMAT_P_H

#include <qtextformat.h>
#include <qmultihash.h>
#include <qvector.h>

class Q_GUI_EXPORT QTextFormatCollection
{
 public:
   QTextFormatCollection() {}
   ~QTextFormatCollection();

   QTextFormatCollection(const QTextFormatCollection &other);
   QTextFormatCollection &operator=(const QTextFormatCollection &other);

   QTextFormat objectFormat(int objectIndex) const {
      return format(objectFormatIndex(objectIndex));
   }

   void setObjectFormat(int objectIndex, const QTextFormat &format) {
      setObjectFormatIndex(objectIndex, indexForFormat(format));
   }

   int objectFormatIndex(int objectIndex) const;
   void setObjectFormatIndex(int objectIndex, int formatIndex);

   int createObjectIndex(const QTextFormat &f);

   int indexForFormat(const QTextFormat &f);
   bool hasFormatCached(const QTextFormat &format) const;

   QTextFormat format(int idx) const;

   QTextBlockFormat blockFormat(int index) const {
      return format(index).toBlockFormat();
   }

   QTextCharFormat charFormat(int index) const {
      return format(index).toCharFormat();
   }

   QTextListFormat listFormat(int index) const {
      return format(index).toListFormat();
   }

   QTextTableFormat tableFormat(int index) const {
      return format(index).toTableFormat();
   }

   QTextImageFormat imageFormat(int index) const {
      return format(index).toImageFormat();
   }

   int numFormats() const {
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

#endif // QTEXTFORMAT_P_H
