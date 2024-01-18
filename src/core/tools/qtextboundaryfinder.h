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

#ifndef QTEXTBOUNDARYFINDER_H
#define QTEXTBOUNDARYFINDER_H

#include <qchar.h>
#include <qstring.h>

class QTextBoundaryFinderPrivate;

class Q_CORE_EXPORT QTextBoundaryFinder
{
 public:
   enum BoundaryType {
      Grapheme,
      Word,
      Sentence,
      Line
   };

   enum BoundaryReason {
      NotAtBoundary      = 0,
      BreakOpportunity   = 0x1f,
      StartOfItem        = 0x20,
      EndOfItem          = 0x40,
      MandatoryBreak     = 0x80,
      SoftHyphen         = 0x100
   };
   using BoundaryReasons = QFlags<BoundaryReason>;

   QTextBoundaryFinder();

   QTextBoundaryFinder(const QTextBoundaryFinder &other);
   QTextBoundaryFinder(QTextBoundaryFinder &&other);

   QTextBoundaryFinder(BoundaryType type, const QString &str);

   ~QTextBoundaryFinder();

   QTextBoundaryFinder &operator=(const QTextBoundaryFinder &other);
   QTextBoundaryFinder &operator=(QTextBoundaryFinder &&other);

   BoundaryType type() const {
      return m_type;
   }

   QString string() const;

   void toStart();
   void toEnd();
   int position() const;
   void setPosition(int position);

   int toNextBoundary();
   int toPreviousBoundary();

   bool isAtBoundary() const;
   BoundaryReasons boundaryReasons() const;

 private:
   BoundaryType m_type;

   QString m_str;
   QString::const_iterator iter_pos;

   bool m_valid = true;

   uint freePrivate : 1;

   QTextBoundaryFinderPrivate *d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QTextBoundaryFinder::BoundaryReasons)

#endif

