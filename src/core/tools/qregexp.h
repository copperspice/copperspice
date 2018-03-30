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

#ifndef QREGEXP_H
#define QREGEXP_H

#include <qstring.h>
class QStringList;

namespace QDeprecated {

struct QRegExpPrivate;

class Q_CORE_EXPORT QRegExp
{
 public:
   enum PatternSyntax {
      RegExp,
      Wildcard,
      FixedString,
      RegExp2,
      WildcardUnix,
      W3CXmlSchema11
   };
   enum CaretMode { CaretAtZero, CaretAtOffset, CaretWontMatch };

   QRegExp();
   explicit QRegExp(const QString &pattern, Qt::CaseSensitivity cs = Qt::CaseSensitive, PatternSyntax syntax = RegExp);
   QRegExp(const QRegExp &rx);

   ~QRegExp();

   QRegExp &operator=(const QRegExp &rx);

   inline QRegExp &operator=(QRegExp && other) {
      qSwap(priv, other.priv);
      return *this;
   }

   inline void swap(QRegExp &other) {
      qSwap(priv, other.priv);
   }

   bool operator==(const QRegExp &rx) const;
   inline bool operator!=(const QRegExp &rx) const {
      return !operator==(rx);
   }

   bool isEmpty() const;
   bool isValid() const;
   QString pattern() const;
   void setPattern(const QString &pattern);
   Qt::CaseSensitivity caseSensitivity() const;
   void setCaseSensitivity(Qt::CaseSensitivity cs);

   PatternSyntax patternSyntax() const;
   void setPatternSyntax(PatternSyntax syntax);

   bool isMinimal() const;
   void setMinimal(bool minimal);
   bool exactMatch(const QString &str) const;

   int indexIn(const QString &str, int offset = 0, CaretMode caretMode = CaretAtZero) const;
   int lastIndexIn(const QString &str, int offset = -1, CaretMode caretMode = CaretAtZero) const;

   int matchedLength() const;
   int numCaptures() const;

   int captureCount() const;
   QStringList capturedTexts() const;
   QStringList capturedTexts();
   QString cap(int nth = 0) const;
   QString cap(int nth = 0);
   int pos(int nth = 0) const;
   int pos(int nth = 0);
   QString errorString() const;
   QString errorString();

   static QString escape(const QString &str);

 private:
   QRegExpPrivate *priv;
};

Q_CORE_EXPORT QDataStream &operator<<(QDataStream &out, const QRegExp &regExp);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &in, QRegExp &regExp);

} // namespace

Q_DECLARE_TYPEINFO(QDeprecated::QRegExp, Q_MOVABLE_TYPE);

#endif // QREGEXP_H
