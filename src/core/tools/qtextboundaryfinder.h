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

#ifndef QTEXTBOUNDARYFINDER_H
#define QTEXTBOUNDARYFINDER_H

#include <qstring.h>

class QTextBoundaryFinderPrivate;

class Q_CORE_EXPORT QTextBoundaryFinder
{
 public:
   enum BoundaryType {
      Grapheme,
      Word,
      Line,
      Sentence
   };

   enum BoundaryReason {
      NotAtBoundary = 0,
      StartWord     = 1,
      EndWord       = 2
      // Hyphen
   };
   using BoundaryReasons = QFlags<BoundaryReason>;

   QTextBoundaryFinder();
   QTextBoundaryFinder(const QTextBoundaryFinder &other);
   QTextBoundaryFinder(BoundaryType type, const QString &str);

   ~QTextBoundaryFinder();

   QTextBoundaryFinder &operator=(const QTextBoundaryFinder &other);

   inline bool isValid() const {
      return d;
   }

   inline BoundaryType type() const {
      return t;
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
   BoundaryType t;

   QString m_str;
   QString::const_iterator iter_pos;

   bool m_valid = true;

   uint freePrivate : 1;
   uint unused : 31;

   QTextBoundaryFinderPrivate *d;
};

#endif

