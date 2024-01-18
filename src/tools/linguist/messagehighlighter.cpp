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

#include <messagehighlighter.h>

#include <qtextstream.h>

MessageHighlighter::MessageHighlighter(QTextEdit *textEdit)
   : QSyntaxHighlighter(textEdit->document())
{
   QTextCharFormat entityFormat;
   entityFormat.setForeground(Qt::red);
   m_formats[Entity] = entityFormat;

   QTextCharFormat tagFormat;
   tagFormat.setForeground(Qt::darkMagenta);
   m_formats[Tag] = tagFormat;

   QTextCharFormat commentFormat;
   commentFormat.setForeground(Qt::gray);
   commentFormat.setFontItalic(true);
   m_formats[Comment] = commentFormat;

   QTextCharFormat attributeFormat;
   attributeFormat.setForeground(Qt::black);
   attributeFormat.setFontItalic(true);
   m_formats[Attribute] = attributeFormat;

   QTextCharFormat valueFormat;
   valueFormat.setForeground(Qt::blue);
   m_formats[Value] = valueFormat;

   QTextCharFormat acceleratorFormat;
   acceleratorFormat.setFontUnderline(true);
   m_formats[Accelerator] = acceleratorFormat;

   QTextCharFormat variableFormat;
   variableFormat.setForeground(Qt::blue);
   m_formats[Variable] = variableFormat;

   rehighlight();
}

void MessageHighlighter::highlightBlock(const QString &text)
{
   static const QChar tab            = '\t';
   static const QChar space          = ' ';
   static const QChar amp            = '&';
   static const QChar endTag         = '>';
   static const QChar quot           = '"';
   static const QChar apos           = '\'';
   static const QChar semicolon      = ';';
   static const QChar equals         = '=';
   static const QChar percent        = '%';

   static const QString startComment = "<!--";
   static const QString endComment   = "-->";
   static const QString endElement   = "/>";

   int state = previousBlockState();
   int len   = text.length();
   int start = 0;
   int pos   = 0;

   while (pos < len) {
      switch (state) {
         case NormalState:
         default:
            while (pos < len) {
               QChar ch = text.at(pos);

               if (ch == '<') {
                  if (text.mid(pos, 4) == startComment) {
                     state = InComment;

                  } else {
                     state = InTag;
                     start = pos;

                     while (pos < len && text.at(pos) != space && text.at(pos) != endTag &&
                              text.at(pos) != tab && text.mid(pos, 2) != endElement) {
                        ++pos;
                     }

                     if (text.mid(pos, 2) == endElement) {
                        ++pos;
                     }

                     setFormat(start, pos - start, m_formats[Tag]);
                     break;
                  }
                  break;

               } else if (ch == amp && pos + 1 < len) {
                  // Default is Accelerator
                  if (text.at(pos + 1).isLetterOrNumber()) {
                     setFormat(pos + 1, 1, m_formats[Accelerator]);
                  }

                  // When a semicolon follows assume an Entity
                  start = pos;
                  ch    = text.at(++pos);

                  while (pos + 1 < len && ch != semicolon && ch.isLetterOrNumber()) {
                     ch = text.at(++pos);
                  }

                  if (ch == semicolon) {
                     setFormat(start, pos - start + 1, m_formats[Entity]);
                  }

               } else if (ch == percent) {
                  start = pos;

                  // %[1-9]*
                  for (++pos; pos < len && text.at(pos).isDigit(); ++pos) {}

                  // %n
                  if (pos < len && pos == start + 1 && text.at(pos) == 'n') {
                     ++pos;
                  }

                  setFormat(start, pos - start, m_formats[Variable]);

               } else {
                  // No tag, comment or entity started, continue...
                  ++pos;
               }
            }
            break;

         case InComment:
            start = pos;

            while (pos < len) {
               if (text.mid(pos, 3) == endComment) {
                  pos += 3;
                  state = NormalState;
                  break;

               } else {
                  ++pos;
               }
            }

            setFormat(start, pos - start, m_formats[Comment]);
            break;

         case InTag:
            QChar quote = QChar::Null;

            while (pos < len) {
               QChar ch = text.at(pos);

               if (quote.isNull()) {
                  start = pos;
                  if (ch == apos || ch == quot) {
                     quote = ch;

                  } else if (ch == endTag) {
                     ++pos;
                     setFormat(start, pos - start, m_formats[Tag]);
                     state = NormalState;
                     break;

                  } else if (text.mid(pos, 2) == endElement) {
                     pos += 2;
                     setFormat(start, pos - start, m_formats[Tag]);
                     state = NormalState;
                     break;

                  } else if (ch != space && text.at(pos) != tab) {
                     // Tag not ending, not a quote and no whitespace, so
                     // we must be dealing with an attribute.
                     ++pos;

                     while (pos < len && text.at(pos) != space && text.at(pos) != tab && text.at(pos) != equals) {
                        ++pos;
                     }

                     setFormat(start, pos - start, m_formats[Attribute]);
                     start = pos;
                  }

               } else if (ch == quote) {
                  quote = QChar::Null;

                  // Anything quoted is a value
                  setFormat(start, pos - start, m_formats[Value]);
               }
               ++pos;
            }
            break;
      }
   }

   setCurrentBlockState(state);
}
