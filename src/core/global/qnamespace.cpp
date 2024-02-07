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

// ** simulate cs_gadget
const char *Qt::cs_className()
{
   static const char *retval("Qt");
   return retval;
}

const QMetaObject_T<Qt> &Qt::staticMetaObject()
{
   static std::atomic<bool> isCreated(false);
   static std::atomic<QMetaObject_T<Qt> *> createdObj(nullptr);

   if (isCreated) {
      return *createdObj;
   }

   std::lock_guard<std::recursive_mutex> lock(QObject::m_metaObjectMutex());

   if (createdObj != nullptr) {
      return *createdObj;
   }

   QMap<std::type_index, QMetaObject *> &map = QObject::m_metaObjectsAll();
   auto index = map.find(typeid(Qt));

   QMetaObject_T<Qt> *metaInfo;

   if (index == map.end()) {
      metaInfo = new QMetaObject_T<Qt>;
      map.insert(typeid(Qt), metaInfo);
      metaInfo->postConstruct();
      return *metaInfo;

   } else {                                        \
      metaInfo = dynamic_cast<QMetaObject_T<Qt> *> (index.value());
      createdObj.store(metaInfo);
      isCreated = true;
      return *metaInfo;
   }
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
   if (text.mid(start, 5) == "<?xml") {

      while (start < text.length()) {
         if (text.at(start) == '?' && start + 2 < text.length() && text.at(start + 1) == '>') {
            start += 2;
            break;
         }
         ++start;
      }

      while (start < text.length() && text.at(start).isSpace()) {
         ++start;
      }
   }

   if (text.mid(start, 5).toLower() == "<!doc") {
      return true;
   }

   int open = start;
   while (open < text.length() && text.at(open) != '<' && text.at(open) != '\n') {
      if (text.at(open) == '&' &&  text.mid(open + 1, 3) == "lt;") {
         return true;   // support desperate attempt of user to see <...>
      }
      ++open;
   }

   if (open < text.length() && text.at(open) == '<') {
      const int close = text.indexOf('>', open);

      if (close > -1) {
         QString tag;

         for (int i = open + 1; i < close; ++i) {
            if (text[i].isDigit() || text[i].isLetter()) {
               tag += text[i];

            } else if (!tag.isEmpty() && text[i].isSpace()) {
               break;

            } else if (!tag.isEmpty() && text[i] == '/' && i + 1 == close) {
               break;

            } else if (!text[i].isSpace() && (!tag.isEmpty() || text[i] != '!')) {
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
   QString rich = "<p>";

   for (int i = 0; i < plain.length(); ++i) {
      if (plain[i] == '\n') {
         int c = 1;

         while (i + 1 < plain.length() && plain[i + 1] == '\n') {
            i++;
            c++;
         }

         if (c == 1) {
            rich += "<br>\n";

         } else {
            rich += "</p>\n";

            while (--c > 1) {
               rich += "<br>\n";
            }
            rich += "<p>";
         }

         col = 0;

      } else {
         char32_t nbsp = 0x00A0;

         if (mode == Qt::WhiteSpacePre && plain[i] == '\t') {
            rich += QChar32(nbsp);
            ++col;

            while (col % 8) {
               rich += QChar32(nbsp);
               ++col;
            }

         } else if (mode == Qt::WhiteSpacePre && plain[i].isSpace()) {
            rich += QChar32(nbsp);

         } else if (plain[i] == '<') {
            rich += "&lt;";

         } else if (plain[i] == '>') {
            rich += "&gt;";

         } else if (plain[i] == '&') {
            rich += "&amp;";

         } else {
            rich += plain[i];
         }

         ++col;
      }
   }

   if (col != 0) {
      rich += "</p>";
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




