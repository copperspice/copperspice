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

#include <qnamespace.h>
#include <qmetaobject.h>
#include <qtextcodec.h>
#include <set>

static const std::set<QString> elementSet = {
   "a",
   "address",
   "b",
   "big",
   "blockquote",
   "body",
   "br",
   "caption",
   "center",
   "cite",
   "code",
   "dd",
   "dfn",
   "div",
   "dl",
   "dt",
   "em",
   "font",
   "h1",
   "h2",
   "h3",
   "h4",
   "h5",
   "h6",
   "head",
   "hr",
   "html",
   "i",
   "img",
   "kbd",
   "li",
   "link",
   "meta",
   "nobr",
   "ol",
   "p",
   "pre",
   "qt",
   "s",
   "samp",
   "script",
   "small",
   "span",
   "strong",
   "style",
   "sub",
   "sup",
   "table",
   "tbody",
   "td",
   "tfoot",
   "th",
   "thead",
   "title",
   "tr",
   "tt",
   "u",
   "ul",
   "var",
};

static bool lookupElement_X(const QString &element)
{
   if (elementSet.count(element) == 0 )  {
      return false;

   } else {
      return true;

   }

}


// **
const char *Qt::cs_className()
{
   return "Qt";
}

const QMetaObject_T<Qt> &Qt::staticMetaObject()
{
   static const QMetaObject_T<Qt> metaInfo = QMetaObject_T<Qt>();
   return metaInfo;
}

const QMetaObject *Qt::metaObject() const
{
   return &staticMetaObject();
}

// internal
bool Qt::mightBeRichText(const QString &text)
{
   if (text.isEmpty()) {
      return false;
   }

   int start = 0;

   while (start < text.length() && text.at(start).isSpace()) {
      ++start;
   }

   // skip a leading <?xml ... ?> as for example with xhtml
   if (text.mid(start, 5) == QLatin1String("<?xml")) {
      while (start < text.length()) {
         if (text.at(start) == QLatin1Char('?') && start + 2 < text.length() && text.at(start + 1) == QLatin1Char('>')) {
            start += 2;
            break;
         }
         ++start;
      }

      while (start < text.length() && text.at(start).isSpace()) {
         ++start;
      }
   }

   if (text.mid(start, 5).toLower() == QLatin1String("<!doc")) {
      return true;
   }

   int open = start;
   while (open < text.length() && text.at(open) != QLatin1Char('<') && text.at(open) != QLatin1Char('\n')) {
      if (text.at(open) == QLatin1Char('&') &&  text.mid(open + 1, 3) == QLatin1String("lt;")) {
         return true;   // support desperate attempt of user to see <...>
      }
      ++open;
   }

   if (open < text.length() && text.at(open) == QLatin1Char('<')) {
      const int close = text.indexOf(QLatin1Char('>'), open);

      if (close > -1) {
         QString tag;

         for (int i = open + 1; i < close; ++i) {
            if (text[i].isDigit() || text[i].isLetter()) {
               tag += text[i];
            } else if (!tag.isEmpty() && text[i].isSpace()) {
               break;
            } else if (!tag.isEmpty() && text[i] == QLatin1Char('/') && i + 1 == close) {
               break;
            } else if (!text[i].isSpace() && (!tag.isEmpty() || text[i] != QLatin1Char('!'))) {
               return false;   // that's not a tag
            }
         }

#ifndef QT_NO_TEXTHTMLPARSER
         return lookupElement_X(tag.toLower());
#else
         return false;
#endif
      }
   }

   return false;
}

QString Qt::convertFromPlainText(const QString &plain, Qt::WhiteSpaceMode mode)
{
   int col = 0;
   QString rich;
   rich += QLatin1String("<p>");

   for (int i = 0; i < plain.length(); ++i) {
      if (plain[i] == QLatin1Char('\n')) {
         int c = 1;
         while (i + 1 < plain.length() && plain[i + 1] == QLatin1Char('\n')) {
            i++;
            c++;
         }

         if (c == 1) {
            rich += QLatin1String("<br>\n");
         } else {
            rich += QLatin1String("</p>\n");
            while (--c > 1) {
               rich += QLatin1String("<br>\n");
            }
            rich += QLatin1String("<p>");
         }
         col = 0;

      } else {
         if (mode == Qt::WhiteSpacePre && plain[i] == QLatin1Char('\t')) {
            rich += QChar(0x00a0U);
            ++col;

            while (col % 8) {
               rich += QChar(0x00a0U);
               ++col;
            }
         } else if (mode == Qt::WhiteSpacePre && plain[i].isSpace()) {
            rich += QChar(0x00a0U);
         } else if (plain[i] == QLatin1Char('<')) {
            rich += QLatin1String("&lt;");
         } else if (plain[i] == QLatin1Char('>')) {
            rich += QLatin1String("&gt;");
         } else if (plain[i] == QLatin1Char('&')) {
            rich += QLatin1String("&amp;");
         } else {
            rich += plain[i];
         }
         ++col;
      }
   }

   if (col != 0) {
      rich += QLatin1String("</p>");
   }
   return rich;
}

#ifndef QT_NO_TEXTCODEC
// internal
QTextCodec *Qt::codecForHtml(const QByteArray &ba)
{
   return QTextCodec::codecForHtml(ba);
}
#endif




