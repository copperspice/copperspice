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

#include <qtextlist.h>
#include <qtextobject_p.h>
#include <qtextcursor.h>
#include <qtextdocument_p.h>
#include <qdebug.h>

class QTextListPrivate : public QTextBlockGroupPrivate
{
 public:
   QTextListPrivate(QTextDocument *doc)
      : QTextBlockGroupPrivate(doc) {
   }
};

QTextList::QTextList(QTextDocument *doc)
   : QTextBlockGroup(*new QTextListPrivate(doc), doc)
{
}


QTextList::~QTextList()
{
}


int QTextList::count() const
{
   Q_D(const QTextList);
   return d->blocks.count();
}


QTextBlock QTextList::item(int i) const
{
   Q_D(const QTextList);

   if (i < 0 || i >= d->blocks.size()) {
      return QTextBlock();
   }

   return d->blocks.at(i);
}

int QTextList::itemNumber(const QTextBlock &blockIt) const
{
   Q_D(const QTextList);
   return d->blocks.indexOf(blockIt);
}


QString QTextList::itemText(const QTextBlock &blockIt) const
{
   Q_D(const QTextList);
   int item = d->blocks.indexOf(blockIt) + 1;
   if (item <= 0) {
      return QString();
   }

   QTextBlock block = d->blocks.at(item - 1);
   QTextBlockFormat blockFormat = block.blockFormat();

   QString result;

   const int style = format().style();
   QString numberPrefix;
   QString numberSuffix = QLatin1String(".");

   if (format().hasProperty(QTextFormat::ListNumberPrefix)) {
      numberPrefix = format().numberPrefix();
   }
   if (format().hasProperty(QTextFormat::ListNumberSuffix)) {
      numberSuffix = format().numberSuffix();
   }

   switch (style) {
      case QTextListFormat::ListDecimal:
         result = QString::number(item);
         break;
      // from the old richtext
      case QTextListFormat::ListLowerAlpha:
      case QTextListFormat::ListUpperAlpha: {
         const char baseChar = style == QTextListFormat::ListUpperAlpha ? 'A' : 'a';

         int c = item;
         while (c > 0) {
            c--;
            result.prepend(QChar(baseChar + (c % 26)));
            c /= 26;
         }
      }
      break;
      case QTextListFormat::ListLowerRoman:
      case QTextListFormat::ListUpperRoman: {
         if (item < 5000) {
            QByteArray romanNumeral;

            // works for up to 4999 items
            static const char romanSymbolsLower[] = "iiivixxxlxcccdcmmmm";
            static const char romanSymbolsUpper[] = "IIIVIXXXLXCCCDCMMMM";
            QByteArray romanSymbols; // wrap to have "mid"
            if (style == QTextListFormat::ListLowerRoman) {
               romanSymbols = QByteArray::fromRawData(romanSymbolsLower, sizeof(romanSymbolsLower));
            } else {
               romanSymbols = QByteArray::fromRawData(romanSymbolsUpper, sizeof(romanSymbolsUpper));
            }

            int c[] = { 1, 4, 5, 9, 10, 40, 50, 90, 100, 400, 500, 900, 1000 };
            int n = item;
            for (int i = 12; i >= 0; n %= c[i], i--) {
               int q = n / c[i];
               if (q > 0) {
                  int startDigit = i + (i + 3) / 4;
                  int numDigits;
                  if (i % 4) {
                     // c[i] == 4|5|9|40|50|90|400|500|900
                     if ((i - 2) % 4) {
                        // c[i] == 4|9|40|90|400|900 => with subtraction (IV, IX, XL, XC, ...)
                        numDigits = 2;
                     } else {
                        // c[i] == 5|50|500 (V, L, D)
                        numDigits = 1;
                     }
                  } else {
                     // c[i] == 1|10|100|1000 (I, II, III, X, XX, ...)
                     numDigits = q;
                  }

                  romanNumeral.append(romanSymbols.mid(startDigit, numDigits));
               }
            }
            result = QString::fromLatin1(romanNumeral);
         } else {
            result = QLatin1String("?");
         }

      }
      break;
      default:
         Q_ASSERT(false);
   }
   if (blockIt.textDirection() == Qt::RightToLeft) {
      return numberSuffix + result + numberPrefix;
   } else {
      return numberPrefix + result + numberSuffix;
   }
}


void QTextList::removeItem(int i)
{
   Q_D(QTextList);
   if (i < 0 || i >= d->blocks.size()) {
      return;
   }

   QTextBlock block = d->blocks.at(i);
   remove(block);
}


void QTextList::remove(const QTextBlock &block)
{
   QTextBlockFormat fmt = block.blockFormat();
   fmt.setIndent(fmt.indent() + format().indent());
   fmt.setObjectIndex(-1);
   block.docHandle()->setBlockFormat(block, block, fmt, QTextDocumentPrivate::SetFormat);
}


void QTextList::add(const QTextBlock &block)
{
   QTextBlockFormat fmt = block.blockFormat();
   fmt.setObjectIndex(objectIndex());
   block.docHandle()->setBlockFormat(block, block, fmt, QTextDocumentPrivate::SetFormat);
}

