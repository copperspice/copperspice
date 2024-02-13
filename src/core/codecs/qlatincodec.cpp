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

#include <qlatincodec_p.h>
#include <qlist.h>

#ifndef QT_NO_TEXTCODEC

QLatin1Codec::~QLatin1Codec()
{
}

QString QLatin1Codec::convertToUnicode(const char *chars, int len, ConverterState *) const
{
   if (chars == nullptr) {
      return QString();
   }

   return QString::fromLatin1(chars, len);
}

QByteArray QLatin1Codec::convertFromUnicode(QStringView str, ConverterState *state) const
{
   const char replacement = (state && state->m_flags & ConvertInvalidToNull) ? 0 : '?';

   QByteArray retval;

   int invalid = 0;

   for (auto c : str) {

      if (c > 0xff) {
         retval.append(replacement);
         ++invalid;

      } else {
         retval.append(c.unicode() & 0xFF);
      }
   }

   if (state) {
      state->invalidChars += invalid;
   }

   return retval;
}

QString QLatin1Codec::name() const
{
   return QString("ISO-8859-1");
}

QStringList QLatin1Codec::aliases() const
{
   QStringList list;

   list << "latin1"
        << "CP819"
        << "IBM819"
        << "iso-ir-100"
        << "csISOLatin1";
   return list;
}

int QLatin1Codec::mibEnum() const
{
   return 4;
}

QLatin15Codec::~QLatin15Codec()
{
}

QString QLatin15Codec::convertToUnicode(const char *chars, int len, ConverterState *) const
{
   if (chars == nullptr) {
      return QString();
   }

   QString retval;
   const char *c = chars;

   while (len--) {

      switch (static_cast<uchar>(*c)) {

         case 0xa4: {
            char32_t tmp = U'\u20ac';
            retval.append(tmp);
         }
         break;

         case 0xa6: {
            char32_t tmp = U'\u0160';
            retval.append(tmp);
         }
         break;

         case 0xa8: {
            char32_t tmp = U'\u0161';
            retval.append(tmp);
         }
         break;

         case 0xb4: {
            char32_t tmp = U'\u017d';
            retval.append(tmp);
         }
         break;

         case 0xb8: {
            char32_t tmp = U'\u017e';
            retval.append(tmp);
         }
         break;

         case 0xbc: {
            char32_t tmp = U'\u0152';
            retval.append(tmp);
         }
         break;

         case 0xbd: {
            char32_t tmp = U'\u0153';
            retval.append(tmp);
         }
         break;

         case 0xbe: {
            char32_t tmp = U'\u0178';
            retval.append(tmp);
         }
         break;

         default:
            retval.append(c);
            break;
      }

      ++c;
   }

   return retval;
}

QByteArray QLatin15Codec::convertFromUnicode(QStringView str, ConverterState *state) const
{
   const char replacement = (state && state->m_flags & ConvertInvalidToNull) ? 0 : '?';

   QByteArray retval;
   int invalid = 0;

   for (auto c : str) {
      char32_t uc = c.unicode();
      uchar tmp   = uc & 0xFF;

      if (uc < 0x0100) {

         if (uc > 0xa3) {
            switch (uc) {
               case 0xa4:
               case 0xa6:
               case 0xa8:
               case 0xb4:
               case 0xb8:
               case 0xbc:
               case 0xbd:
               case 0xbe:
                  tmp = replacement;
                  ++invalid;
                  break;

               default:
                  break;
            }
         }

      } else {
         if (uc == 0x20ac) {
            tmp = 0xa4;

         } else if ((uc & 0xff00) == 0x0100) {

            switch (uc) {
               case 0x0160:
                  tmp = 0xa6;
                  break;

               case 0x0161:
                  tmp = 0xa8;
                  break;

               case 0x017d:
                  tmp = 0xb4;
                  break;

               case 0x017e:
                  tmp = 0xb8;
                  break;

               case 0x0152:
                  tmp = 0xbc;
                  break;

               case 0x0153:
                  tmp = 0xbd;
                  break;

               case 0x0178:
                  tmp = 0xbe;
                  break;

               default:
                  tmp = replacement;
                  ++invalid;
            }

         } else {
            tmp = replacement;
            ++invalid;
         }
      }

      retval.append(tmp);
   }

   if (state) {
      state->remainingChars = 0;
      state->invalidChars += invalid;
   }

   return retval;
}

QString QLatin15Codec::name() const
{
   return QString("ISO-8859-15");
}

QStringList QLatin15Codec::aliases() const
{
   QStringList list;
   list << "latin9";

   return list;
}

int QLatin15Codec::mibEnum() const
{
   return 111;
}

#endif // QT_NO_TEXTCODEC
