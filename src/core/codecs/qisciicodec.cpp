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

#include <qisciicodec_p.h>

#include <qlist.h>

#ifndef QT_NO_CODECS

struct Codecs {
   const char name[10];
   ushort base;
};

static const Codecs codecs [] = {
   { "Iscii-Dev", 0x900 },
   { "Iscii-Bng", 0x980 },
   { "Iscii-Pnj", 0xa00 },
   { "Iscii-Gjr", 0xa80 },
   { "Iscii-Ori", 0xb00 },
   { "Iscii-Tml", 0xb80 },
   { "Iscii-Tlg", 0xc00 },
   { "Iscii-Knd", 0xc80 },
   { "Iscii-Mlm", 0xd00 }
};

QIsciiCodec::~QIsciiCodec()
{
}

QString QIsciiCodec::name() const
{
   return QString::fromLatin1(codecs[idx].name);
}

int QIsciiCodec::mibEnum() const
{
   /* There is no MIBEnum for Iscii */
   return -3000 - idx;
}

static constexpr const uchar inv = 0xFF;

/* iscii range from 0xa0 - 0xff */
static const uchar iscii_to_uni_table[0x60] = {
   0x00, 0x01, 0x02, 0x03,
   0x05, 0x06, 0x07, 0x08,
   0x09, 0x0a, 0x0b, 0x0e,
   0x0f, 0x20, 0x0d, 0x12,

   0x13, 0x14, 0x11, 0x15,
   0x16, 0x17, 0x18, 0x19,
   0x1a, 0x1b, 0x1c, 0x1d,
   0x1e, 0x1f, 0x20, 0x21,

   0x22, 0x23, 0x24, 0x25,
   0x26, 0x27, 0x28, 0x29,
   0x2a, 0x2b, 0x2c, 0x2d,
   0x2e, 0x2f, 0x5f, 0x30,

   0x31, 0x32, 0x33, 0x34,
   0x35, 0x36, 0x37, 0x38,
   0x39,  inv, 0x3e, 0x3f,
   0x40, 0x41, 0x42, 0x43,

   0x46, 0x47, 0x48, 0x45,
   0x4a, 0x4b, 0x4c, 0x49,
   0x4d, 0x3c, 0x64, 0x00,
   0x00, 0x00, 0x00, 0x00,

   0x00, 0x66, 0x67, 0x68,
   0x69, 0x6a, 0x6b, 0x6c,
   0x6d, 0x6e, 0x6f, 0x00,
   0x00, 0x00, 0x00, 0x00
};

static const uchar uni_to_iscii_table[0x80] = {
   0x00, 0xa1, 0xa2, 0xa3,
   0x00, 0xa4, 0xa5, 0xa6,
   0xa7, 0xa8, 0xa9, 0xaa,
   0x00, 0xae, 0xab, 0xac,

   0xad, 0xb2, 0xaf, 0xb0,
   0xb1, 0xb3, 0xb4, 0xb5,
   0xb6, 0xb7, 0xb8, 0xb9,
   0xba, 0xbb, 0xbc, 0xbd,

   0xbe, 0xbf, 0xc0, 0xc1,
   0xc2, 0xc3, 0xc4, 0xc5,
   0xc6, 0xc7, 0xc8, 0xc9,
   0xca, 0xcb, 0xcc, 0xcd,

   0xcf, 0xd0, 0xd1, 0xd2,
   0xd3, 0xd4, 0xd5, 0xd6,
   0xd7, 0xd8, 0x00, 0x00,
   0xe9, 0x00, 0xda, 0xdb,

   0xdc, 0xdd, 0xde, 0xdf,
   0x00, 0xe3, 0xe0, 0xe1,
   0xe2, 0xe7, 0xe4, 0xe5,
   0xe6, 0xe8, 0x00, 0x00,

   0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00,
   0x01, 0x02, 0x03, 0x04, // decomposable into the uc codes listed here + nukta
   0x05, 0x06, 0x07, 0xce,

   0x00, 0x00, 0x00, 0x00,
   0xea, 0x08, 0xf1, 0xf2,
   0xf3, 0xf4, 0xf5, 0xf6,
   0xf7, 0xf8, 0xf9, 0xfa,

   0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00
};

static const uchar uni_to_iscii_pairs[] = {
   0x00, 0x00,
   0x15, 0x3c, // 0x958
   0x16, 0x3c, // 0x959
   0x17, 0x3c, // 0x95a
   0x1c, 0x3c, // 0x95b
   0x21, 0x3c, // 0x95c
   0x22, 0x3c, // 0x95d
   0x2b, 0x3c, // 0x95e
   0x64, 0x64  // 0x965
};

QByteArray QIsciiCodec::convertFromUnicode(QStringView str, ConverterState *state) const
{
   char replacement = '?';
   bool halant      = false;

   if (state) {
      if (state->m_flags & ConvertInvalidToNull) {
         replacement = 0;
      }

      halant = state->state_data[0];
   }

   QByteArray retval;
   int invalid = 0;

   const int base = codecs[idx].base;

   for (auto c : str) {
      char32_t uc = c.unicode();

      // The low 7 bits of ISCII are plain ASCII
      // However, we go all the way up to 0xA0, so we are compatible with convertToUnicode()'s behavior.

      if (uc < 0xA0) {
         retval.append(uc & 0xff);
         continue;
      }

      const int pos = uc - base;

      if (pos > 0 && pos < 0x80) {
         uchar iscii = uni_to_iscii_table[pos];

         if (iscii > 0x80) {
            retval.append(iscii);

         } else if (iscii) {
            const uchar *pair = uni_to_iscii_pairs + 2 * iscii;
            char32_t tmp = (static_cast<char32_t>(pair[0]) - 0xD800) << 10 | (static_cast<char32_t>(pair[1]) - 0xDC00);
            retval.append(tmp);

         } else {
            retval.append(replacement);
            ++invalid;
         }

      } else {
         if (uc == 0x200c) {
            // ZWNJ

            if (halant) {
               // Consonant Halant ZWNJ -> Consonant Halant Halant
               retval.append(uchar(0xe8));
            }

         } else if (uc == 0x200d) {
            // ZWJ

            if (halant) {
               // Consonant Halant ZWJ -> Consonant Halant Nukta
               retval.append(uchar(0xe9));
            }

         } else {
            retval.append(replacement);
            ++invalid;
         }
      }

      halant = (pos == 0x4d);
   }

   if (state) {
      state->invalidChars += invalid;
      state->state_data[0] = halant;
   }

   return retval;
}

QString QIsciiCodec::convertToUnicode(const char *chars, int len, ConverterState *state) const
{
   bool halant = false;

   if (state) {
      halant = state->state_data[0];
   }

   QString result;
   const int base = codecs[idx].base;

   for (int i = 0; i < len; ++i) {
      ushort ch = (uchar) chars[i];

      if (ch < 0xa0) {
         result.append(chars[i]);

      } else {
         ushort c = iscii_to_uni_table[ch - 0xa0];

         if (halant && (c == inv || c == 0xe9)) {
            // Consonant Halant inv -> Consonant Halant ZWJ
            // Consonant Halant Nukta -> Consonant Halant ZWJ
            result.append(QChar(0x200d));

         } else if (halant && c == 0xe8) {
            // Consonant Halant Halant -> Consonant Halant ZWNJ
            result.append(QChar(0x200c));

         } else {
            result.append(QChar(c + base));
         }
      }

      halant = (ch == 0xe8);
   }

   if (state) {
      state->state_data[0] = halant;
   }

   return result;
}

#endif // QT_NO_CODECS
