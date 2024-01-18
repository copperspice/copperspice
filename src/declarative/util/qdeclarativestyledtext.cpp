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

#include <QStack>
#include <QVector>
#include <QPainter>
#include <QTextLayout>
#include <QDebug>
#include <qmath.h>
#include "private/qdeclarativestyledtext_p.h"

/*
    QDeclarativeStyledText supports few tags:

    <b></b> - bold
    <i></i> - italic
    <br> - new line
    <font color="color_name" size="1-7"></font>

    The opening and closing tags must be correctly nested.
*/

QT_BEGIN_NAMESPACE

class QDeclarativeStyledTextPrivate
{
 public:
   QDeclarativeStyledTextPrivate(const QString &t, QTextLayout &l) : text(t), layout(l), baseFont(layout.font()) {}

   void parse();
   bool parseTag(const QChar *&ch, const QString &textIn, QString &textOut, QTextCharFormat &format);
   bool parseCloseTag(const QChar *&ch, const QString &textIn);
   void parseEntity(const QChar *&ch, const QString &textIn, QString &textOut);
   bool parseFontAttributes(const QChar *&ch, const QString &textIn, QTextCharFormat &format);
   QPair<QStringView, QStringView> parseAttribute(const QChar *&ch, const QString &textIn);
   QStringView parseValue(const QChar *&ch, const QString &textIn);

   inline void skipSpace(const QChar *&ch) {
      while (ch->isSpace() && !ch->isNull()) {
         ++ch;
      }
   }

   QString text;
   QTextLayout &layout;
   QFont baseFont;

   static const QChar lessThan;
   static const QChar greaterThan;
   static const QChar equals;
   static const QChar singleQuote;
   static const QChar doubleQuote;
   static const QChar slash;
   static const QChar ampersand;
};

const QChar QDeclarativeStyledTextPrivate::lessThan(QLatin1Char('<'));
const QChar QDeclarativeStyledTextPrivate::greaterThan(QLatin1Char('>'));
const QChar QDeclarativeStyledTextPrivate::equals(QLatin1Char('='));
const QChar QDeclarativeStyledTextPrivate::singleQuote(QLatin1Char('\''));
const QChar QDeclarativeStyledTextPrivate::doubleQuote(QLatin1Char('\"'));
const QChar QDeclarativeStyledTextPrivate::slash(QLatin1Char('/'));
const QChar QDeclarativeStyledTextPrivate::ampersand(QLatin1Char('&'));

QDeclarativeStyledText::QDeclarativeStyledText(const QString &string, QTextLayout &layout)
   : d(new QDeclarativeStyledTextPrivate(string, layout))
{
}

QDeclarativeStyledText::~QDeclarativeStyledText()
{
   delete d;
}

void QDeclarativeStyledText::parse(const QString &string, QTextLayout &layout)
{
   if (string.isEmpty()) {
      return;
   }
   QDeclarativeStyledText styledText(string, layout);
   styledText.d->parse();
}

void QDeclarativeStyledTextPrivate::parse()
{
   QList<QTextLayout::FormatRange> ranges;
   QStack<QTextCharFormat> formatStack;

   QString drawText;
   drawText.reserve(text.count());

   int textStart = 0;
   int textLength = 0;
   int rangeStart = 0;
   const QChar *ch = text.constData();
   while (!ch->isNull()) {
      if (*ch == lessThan) {
         if (textLength) {
            drawText.append(QStringView(&text, textStart, textLength));
         }
         if (rangeStart != drawText.length() && formatStack.count()) {
            QTextLayout::FormatRange formatRange;
            formatRange.format = formatStack.top();
            formatRange.start = rangeStart;
            formatRange.length = drawText.length() - rangeStart;
            ranges.append(formatRange);
         }
         rangeStart = drawText.length();
         ++ch;
         if (*ch == slash) {
            ++ch;
            if (parseCloseTag(ch, text)) {
               if (formatStack.count()) {
                  formatStack.pop();
               }
            }
         } else {
            QTextCharFormat format;
            if (formatStack.count()) {
               format = formatStack.top();
            }
            if (parseTag(ch, text, drawText, format)) {
               formatStack.push(format);
            }
         }
         textStart = ch - text.constData() + 1;
         textLength = 0;
      } else if (*ch == ampersand) {
         ++ch;
         drawText.append(QStringView(&text, textStart, textLength));
         parseEntity(ch, text, drawText);
         textStart = ch - text.constData() + 1;
         textLength = 0;
      } else {
         ++textLength;
      }
      if (!ch->isNull()) {
         ++ch;
      }
   }
   if (textLength) {
      drawText.append(QStringView(&text, textStart, textLength));
   }
   if (rangeStart != drawText.length() && formatStack.count()) {
      QTextLayout::FormatRange formatRange;
      formatRange.format = formatStack.top();
      formatRange.start = rangeStart;
      formatRange.length = drawText.length() - rangeStart;
      ranges.append(formatRange);
   }

   layout.setText(drawText);
   layout.setAdditionalFormats(ranges);
}

bool QDeclarativeStyledTextPrivate::parseTag(const QChar *&ch, const QString &textIn, QString &textOut,
      QTextCharFormat &format)
{
   skipSpace(ch);

   int tagStart = ch - textIn.constData();
   int tagLength = 0;
   while (!ch->isNull()) {
      if (*ch == greaterThan) {
         QStringView tag(&textIn, tagStart, tagLength);
         const QChar char0 = tag.at(0);
         if (char0 == QLatin1Char('b')) {
            if (tagLength == 1) {
               format.setFontWeight(QFont::Bold);
            } else if (tagLength == 2 && tag.at(1) == QLatin1Char('r')) {
               textOut.append(QChar(QChar::LineSeparator));
               return false;
            }
         } else if (char0 == QLatin1Char('i')) {
            if (tagLength == 1) {
               format.setFontItalic(true);
            }
         }
         return true;
      } else if (ch->isSpace()) {
         // may have params.
         QStringView tag(&textIn, tagStart, tagLength);
         if (tag == QLatin1String("font")) {
            return parseFontAttributes(ch, textIn, format);
         }
         if (*ch == greaterThan || ch->isNull()) {
            continue;
         }
      } else if (*ch != slash) {
         tagLength++;
      }
      ++ch;
   }

   return false;
}

bool QDeclarativeStyledTextPrivate::parseCloseTag(const QChar *&ch, const QString &textIn)
{
   skipSpace(ch);

   int tagStart = ch - textIn.constData();
   int tagLength = 0;
   while (!ch->isNull()) {
      if (*ch == greaterThan) {
         QStringView tag(&textIn, tagStart, tagLength);
         const QChar char0 = tag.at(0);
         if (char0 == QLatin1Char('b')) {
            if (tagLength == 1) {
               return true;
            } else if (tag.at(1) == QLatin1Char('r') && tagLength == 2) {
               return true;
            }
         } else if (char0 == QLatin1Char('i')) {
            if (tagLength == 1) {
               return true;
            }
         } else if (tag == QLatin1String("font")) {
            return true;
         }
         return false;
      } else if (!ch->isSpace()) {
         tagLength++;
      }
      ++ch;
   }

   return false;
}

void QDeclarativeStyledTextPrivate::parseEntity(const QChar *&ch, const QString &textIn, QString &textOut)
{
   int entityStart = ch - textIn.constData();
   int entityLength = 0;
   while (!ch->isNull()) {
      if (*ch == QLatin1Char(';')) {
         QStringView entity(&textIn, entityStart, entityLength);
         if (entity == QLatin1String("gt")) {
            textOut += QChar(62);
         } else if (entity == QLatin1String("lt")) {
            textOut += QChar(60);
         } else if (entity == QLatin1String("amp")) {
            textOut += QChar(38);
         }
         return;
      }
      ++entityLength;
      ++ch;
   }
}

bool QDeclarativeStyledTextPrivate::parseFontAttributes(const QChar *&ch, const QString &textIn,
      QTextCharFormat &format)
{
   bool valid = false;
   QPair<QStringView, QStringView> attr;
   do {
      attr = parseAttribute(ch, textIn);
      if (attr.first == QLatin1String("color")) {
         valid = true;
         format.setForeground(QColor(attr.second.toString()));
      } else if (attr.first == QLatin1String("size")) {
         valid = true;
         int size = attr.second.toString().toInt();
         if (attr.second.at(0) == QLatin1Char('-') || attr.second.at(0) == QLatin1Char('+')) {
            size += 3;
         }
         if (size >= 1 && size <= 7) {
            static const qreal scaling[] = { 0.7, 0.8, 1.0, 1.2, 1.5, 2.0, 2.4 };
            format.setFontPointSize(baseFont.pointSize() * scaling[size - 1]);
         }
      }
   } while (!ch->isNull() && !attr.first.isEmpty());

   return valid;
}

QPair<QStringView, QStringView> QDeclarativeStyledTextPrivate::parseAttribute(const QChar *&ch, const QString &textIn)
{
   skipSpace(ch);

   int attrStart = ch - textIn.constData();
   int attrLength = 0;
   while (!ch->isNull()) {
      if (*ch == greaterThan) {
         break;
      } else if (*ch == equals) {
         ++ch;
         if (*ch != singleQuote && *ch != doubleQuote) {
            while (*ch != greaterThan && !ch->isNull()) {
               ++ch;
            }
            break;
         }
         ++ch;
         if (!attrLength) {
            break;
         }
         QStringView attr(&textIn, attrStart, attrLength);
         QStringView val = parseValue(ch, textIn);
         if (!val.isEmpty()) {
            return QPair<QStringView, QStringView>(attr, val);
         }
         break;
      } else {
         ++attrLength;
      }
      ++ch;
   }

   return QPair<QStringView, QStringView>();
}

QStringView QDeclarativeStyledTextPrivate::parseValue(const QChar *&ch, const QString &textIn)
{
   int valStart = ch - textIn.constData();
   int valLength = 0;
   while (*ch != singleQuote && *ch != doubleQuote && !ch->isNull()) {
      ++valLength;
      ++ch;
   }
   if (ch->isNull()) {
      return QStringView();
   }
   ++ch; // skip quote

   return QStringView(&textIn, valStart, valLength);
}

QT_END_NAMESPACE
