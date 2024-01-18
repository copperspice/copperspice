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

#include <qcompressedwhitespace_p.h>

#include <qstring.h>

using namespace QPatternist;

CompressedWhitespace::CharIdentifier CompressedWhitespace::toIdentifier(const QChar ch)
{
   switch (ch.unicode()) {
      case ' ':
         return Space;

      case '\n':
         return LF;

      case '\r':
         return CR;

      case '\t':
         return Tab;

      default: {
         Q_ASSERT_X(false, Q_FUNC_INFO, "Only whitespace should be passed.");
         return Tab;
      }
   }
}

bool CompressedWhitespace::isEven(const int number)
{
   Q_ASSERT(number >= 0);
   return number % 2 == 0;
}

quint8 CompressedWhitespace::toCompressedChar(const QChar ch, const int len)
{
   Q_ASSERT(len > 0);
   Q_ASSERT(len <= MaxCharCount);

   return len + toIdentifier(ch);
}

QChar CompressedWhitespace::toChar(const CharIdentifier id)
{
   switch (id) {
      case Space:
         return QLatin1Char(' ');
      case CR:
         return QLatin1Char('\r');
      case LF:
         return QLatin1Char('\n');
      case Tab:
         return QLatin1Char('\t');
      default: {
         Q_ASSERT_X(false, Q_FUNC_INFO, "Unexpected input");
         return QChar();
      }
   }
}

QString CompressedWhitespace::compress(QStringView input)
{
   Q_ASSERT(!isEven(1) && isEven(0) && isEven(2));
   Q_ASSERT(!input.isEmpty());

   QString result;
   const int len = input.length();

   /* The amount of compressed characters. For instance, if input is
    * four spaces followed by one tab, compressedChars will be 2, and the resulting
    * QString will have a length of 1, two compressedChars stored in one QChar. */
   int compressedChars = 0;

   for (int i = 0; i < len; ++i) {
      const QChar c(input.at(i));

      int start = i;

      while (true) {
         if (i + 1 == input.length() || input.at(i + 1) != c) {
            break;
         } else {
            ++i;
         }
      }

      /* The length of subsequent whitespace characters in the input. */
      int wsLen = (i - start) + 1;

      /* We might get a sequence of whitespace that is so long, that we can't
       * store it in one unit/byte. In that case we chop it into as many subsequent
       * ones that is needed. */
      while (true) {
         const int unitLength = qMin(wsLen, int(MaxCharCount));
         wsLen -= unitLength;

         ushort resultCP = toCompressedChar(c, unitLength);

         if (isEven(compressedChars)) {
            result += QChar(resultCP);
         } else {
            resultCP = resultCP << 8;
            resultCP |= result.at(result.size() - 1).unicode();

            result.chop(1);
            result.append(resultCP);
         }

         ++compressedChars;

         if (wsLen == 0) {
            break;
         }
      }
   }

   return result;
}

QString CompressedWhitespace::decompress(const QString &input)
{
   Q_ASSERT(!input.isEmpty());
   const int len = input.length() * 2;
   QString retval;

   for (int i = 0; i < len; ++i) {
      ushort cp = input.at(i / 2).unicode();

      if (isEven(i)) {
         cp &= Lower8Bits;
      } else {
         cp = cp >> 8;

         if (cp == 0) {
            return retval;
         }
      }

      const quint8 wsLen = cp & Lower6Bits;
      const quint8 id = cp & UpperTwoBits;

      /* Resize retval, and fill in on the top. */
      const int oldSize = retval.size();
      const int newSize = retval.size() + wsLen;
      retval.resize(newSize);

      const QChar ch(toChar(CharIdentifier(id)));

      for (int f = oldSize; f < newSize; ++f) {
         retval.replace(f, 1, ch);
      }
   }

   return retval;
}
