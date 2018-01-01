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

#ifndef MESSAGEHIGHLIGHTER_H
#define MESSAGEHIGHLIGHTER_H

#include <QtGui/QSyntaxHighlighter>

QT_BEGIN_NAMESPACE

/* Message highlighter based on HtmlSyntaxHighlighter from designer */
class MessageHighlighter : public QSyntaxHighlighter
{
   Q_OBJECT

 public:
   MessageHighlighter(QTextEdit *textEdit);

 protected:
   void highlightBlock(const QString &text);

 private:
   enum Construct {
      Entity,
      Tag,
      Comment,
      Attribute,
      Value,
      Accelerator, // "Open &File"
      Variable,    // "Opening %1"
      LastConstruct = Variable
   };

   enum State {
      NormalState = -1,
      InComment,
      InTag
   };

   QTextCharFormat m_formats[LastConstruct + 1];
};

QT_END_NAMESPACE

#endif // MESSAGEHIGHLIGHTER_H
