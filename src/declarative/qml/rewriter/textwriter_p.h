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

#ifndef TEXTWRITER_H
#define TEXTWRITER_H

#include <qdeclarativejsglobal_p.h>
#include <QtCore/QString>
#include <QtCore/QList>
#include <QtGui/QTextCursor>

QT_QML_BEGIN_NAMESPACE

namespace QDeclarativeJS {

class TextWriter
{
   QString *string;
   QTextCursor *cursor;

   struct Replace {
      int pos;
      int length;
      QString replacement;
   };

   QList<Replace> replaceList;

   struct Move {
      int pos;
      int length;
      int to;
   };

   QList<Move> moveList;

   bool hasOverlap(int pos, int length);
   bool hasMoveInto(int pos, int length);

   void doReplace(const Replace &replace);
   void doMove(const Move &move);

   void write_helper();

 public:
   TextWriter();

   void replace(int pos, int length, const QString &replacement);
   void move(int pos, int length, int to);

   void write(QString *s);
   void write(QTextCursor *textCursor);

};

} // end of namespace QDeclarativeJS

QT_QML_END_NAMESPACE

#endif // TEXTWRITER_H
