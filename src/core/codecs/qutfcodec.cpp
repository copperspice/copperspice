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

#include <qutfcodec_p.h>

#include <qendian.h>
#include <qlist.h>
#include <qstring.h>
#include <qstring16.h>

static constexpr const int Endian = 0;
static constexpr const int Data   = 1;

QByteArray QUtf8::convertFromUnicode(QStringView str, QTextCodec::ConverterState *state)
{
   QByteArray retval;

   if (state && ! (state->m_flags & QTextCodec::IgnoreHeader)) {
      retval.append(uchar(0xEF));
      retval.append(uchar(0xBB));
      retval.append(uchar(0xBF));
   }

   retval += QByteArray(str.charData(), str.size_storage());

   if (state) {
      state->remainingChars = 0;
      state->m_flags |= QTextCodec::IgnoreHeader;
   }

   return retval;
}

QString QUtf8::convertToUnicode(const char *chars, int len, QTextCodec::ConverterState *state)
{
   QString retval;

   bool headerDone   = false;
   QChar replacement = QChar::ReplacementCharacter;

   uint32_t data = 0;
   int bytesLeft = 0;
   int invalid   = 0;

   const char *src = chars;
   const char *end = src + len;

   if (state) {
      if (state->m_flags & QTextCodec::IgnoreHeader) {
         headerDone = true;
      }

      if (state->m_flags & QTextCodec::ConvertInvalidToNull) {
         replacement = QChar::Null;
      }

      if (state->remainingChars) {
         // handle incoming state

         bytesLeft = state->remainingChars;
         memcpy(&data, &state->state_data[0], 4);
      }
   }

   while (src != end) {
      uint32_t byte = uchar(*src);
      ++src;

      if (bytesLeft > 0 && (byte < 0x80 || byte > 0xBF)) {
         bytesLeft = 0;
         ++invalid;

         retval.append(replacement);
      }

      if (byte <= 0x7F) {
         // asc
         data = byte;

      } else if (byte <= 0xBF) {
         // continuation char

         data = (data << 6) | (byte & 0x3F);
         --bytesLeft;

      } else if (byte <= 0xDF) {
         // start of 2 byte seq
         data = (byte & 0x1F);

         if (data == 0) {
            ++invalid;
            retval.append(replacement);
            continue;
         }

         bytesLeft = 1;

      } else if (byte <= 0xEF) {
         // start of 3 byte seq
         data = (byte & 0x0F);

         if (data == 0) {
            ++invalid;
            retval.append(replacement);
            continue;
         }

         bytesLeft = 2;

      } else if (byte <= 0xF4) {
         // start of 4 byte seq
         data = (byte & 0x07);

         if (data == 0) {
            ++invalid;
            retval.append(replacement);
            continue;
         }

         bytesLeft = 3;

      } else {
         bytesLeft = 0;
         ++invalid;

         retval.append(replacement);
         continue;
      }

      if (bytesLeft == 0) {

         if (! headerDone) {
            headerDone = true;

            // skip the BOM
            if (data == 0xFEFF) {
               continue;
            }
         }

         if (data > 0x10FFFF) {
            ++invalid;
            retval.append(replacement);
            continue;
         }

         retval.append( QChar(char32_t(data)) );
      }
   }

   if (! state && bytesLeft != 0) {
      // unterminated UTF sequence
      retval.append(replacement);
   }

   if (state) {
      state->invalidChars += invalid;

      if (headerDone) {
         state->m_flags |= QTextCodec::IgnoreHeader;
      }

      if (bytesLeft != 0) {
         state->remainingChars = bytesLeft;
         memcpy(&state->state_data[0], &data, 4);

      } else {
         state->remainingChars = 0;

      }
   }

   return retval;
}

QByteArray QUtf16::convertFromUnicode(QStringView str, QTextCodec::ConverterState *state, DataEndianness e)
{
   QByteArray retval;

   DataEndianness endian = e;

   if (! state || (! (state->m_flags & QTextCodec::IgnoreHeader))) {
      endian = (QSysInfo::ByteOrder == QSysInfo::BigEndian) ? BigEndianness : LittleEndianness;
   }

   if (! state || ! (state->m_flags & QTextCodec::IgnoreHeader)) {
      if (endian == BigEndianness) {
         retval.append(uchar(0xFE));
         retval.append(uchar(0xFF));

      } else  {
         retval.append(uchar(0xFF));
         retval.append(uchar(0xFE));
      }
   }

   QString16 tmp(str.begin(), str.end());

   const char16_t *iter = tmp.data();
   const char16_t *end  = iter + tmp.size_storage();

   while (iter != end) {
      char16_t ch = *iter;
      ++iter;

      if (endian == BigEndianness) {
         retval.append(ch >> 8);
         retval.append(ch & 0xFF);

      } else  {
         retval.append(ch & 0xFF);
         retval.append(ch >> 8);
      }
   }

   if (state) {
      state->remainingChars = 0;
      state->m_flags |= QTextCodec::IgnoreHeader;
   }

   return retval;
}

QString QUtf16::convertToUnicode(const char *chars, int len, QTextCodec::ConverterState *state, DataEndianness e)
{
   QString retval;

   DataEndianness endian = e;
   QChar replacement = QChar::ReplacementCharacter;

   uint32_t data    = 0;
   bool headerDone  = false;
   int  bytesRead   = 0;
   int invalid      = 0;

   const char *iter = chars;
   const char *end  = iter + len;

   if (state) {
      if (state->m_flags & QTextCodec::IgnoreHeader) {
         headerDone = true;
      }

      if (state->m_flags & QTextCodec::ConvertInvalidToNull) {
         replacement = QChar::Null;
      }

      if (state->remainingChars != 0) {
         bytesRead = state->remainingChars;
         memcpy(&data, &state->state_data[Data], 4);
      }

      if (endian == DetectEndianness) {
         endian = (DataEndianness)state->state_data[Endian];
      }
   }

   if (headerDone && endian == DetectEndianness) {
      endian = (QSysInfo::ByteOrder == QSysInfo::BigEndian) ? BigEndianness : LittleEndianness;
   }

   while (iter != end)   {
      uint8_t byte = *iter;
      ++iter;

      if (bytesRead == 1 || bytesRead == 3) {

         if (endian == LittleEndianness) {
            data = (data & 0xFFFF0000) | (byte << 8) | (data & 0xFF);

         } else {
            data = (data & 0xFFFF0000) | ((data & 0xFF) << 8) | byte;

         }

         ++bytesRead;

         if (! headerDone) {
            headerDone = true;

            if (endian == DetectEndianness) {

               if (data == 0xFFFE) {
                  endian = LittleEndianness;

                  bytesRead = 0;
                  continue;

               } else if (data == 0xFEFF) {
                  endian = BigEndianness;

                  bytesRead = 0;
                  continue;

               } else {
                  if (QSysInfo::ByteOrder == QSysInfo::LittleEndian) {
                     endian = LittleEndianness;
                     data = ((data & 0xFF) << 8) | (data >> 8);

                  } else {
                     endian = BigEndianness;

                  }
               }
            }
         }

         if (bytesRead == 2 && (data <= 0xD7FF || data >= 0xE000)) {
            bytesRead = 0;

            retval.append(char32_t(data));
            data = 0;

         } else if (bytesRead == 2 && (data >= 0xD800 && data <= 0xDBFF)) {
            data = data << 16;

         } else if (bytesRead == 4) {
            data = data - 0xD800DC00;
            data = (data >> 16) | (data & 0x3FF);

            if (data >= 0x10000 && data <= 0x10FFFF) {
               bytesRead = 0;

               retval.append(char32_t(data));
               data = 0;

            } else  {
               bytesRead = 0;
               ++invalid;

               retval.append(replacement);
               data = 0;
            }

         } else {
            bytesRead = 0;
            ++invalid;

            retval.append(replacement);
            data = 0;
         }

      } else {
         data = (data & 0xFFFF0000) | byte;
         ++bytesRead;
      }
   }

   if (state) {
      state->invalidChars += invalid;

      if (headerDone) {
         state->m_flags |= QTextCodec::IgnoreHeader;
      }

      state->state_data[Endian] = endian;

      if (bytesRead != 0) {
         state->remainingChars = bytesRead;
         memcpy(&state->state_data[Data], &data, 4);

      } else {
         state->remainingChars   = 0;
         state->state_data[Data] = 0;
      }
   }

   return retval;
}

QByteArray QUtf32::convertFromUnicode(QStringView str, QTextCodec::ConverterState *state, DataEndianness e)
{
   QByteArray retval;

   DataEndianness endian = e;

   if (! state || (! (state->m_flags & QTextCodec::IgnoreHeader))) {
      endian = (QSysInfo::ByteOrder == QSysInfo::BigEndian) ? BigEndianness : LittleEndianness;
   }

   if (! state || ! (state->m_flags & QTextCodec::IgnoreHeader)) {
      if (endian == BigEndianness) {
         retval.append('\x00');
         retval.append('\x00');
         retval.append(uchar(0xFE));
         retval.append(uchar(0xFF));

      } else  {
         retval.append(uchar(0xFF));
         retval.append(uchar(0xFE));
         retval.append('\x00');
         retval.append('\x00');
      }
   }

   for (auto ch : str) {

      if (endian == BigEndianness) {
         retval.append((ch.unicode() >> 24) & 0xFF);
         retval.append((ch.unicode() >> 16) & 0xFF);
         retval.append((ch.unicode() >> 8)  & 0xFF);
         retval.append(ch.unicode() & 0xFF);

      } else  {
         retval.append(ch.unicode() & 0xFF);
         retval.append((ch.unicode() >> 8)  & 0xFF);
         retval.append((ch.unicode() >> 16) & 0xFF);
         retval.append((ch.unicode() >> 24) & 0xFF);
      }
   }

   if (state) {
      state->remainingChars = 0;
      state->m_flags |= QTextCodec::IgnoreHeader;
   }

   return retval;
}

QString QUtf32::convertToUnicode(const char *chars, int len, QTextCodec::ConverterState *state, DataEndianness e)
{
   QString retval;

   DataEndianness endian = e;

   uint32_t data    = 0;
   bool headerDone  = false;
   int  bytesRead   = 0;

   const char *iter = chars;
   const char *end  = iter + len;

   if (state) {
      headerDone = state->m_flags & QTextCodec::IgnoreHeader;

      if (endian == DetectEndianness) {
         endian = (DataEndianness)state->state_data[Endian];
      }

      bytesRead = state->remainingChars;
      memcpy(&data, &state->state_data[Data], 4);
   }

   if (headerDone && endian == DetectEndianness) {
      endian = (QSysInfo::ByteOrder == QSysInfo::BigEndian) ? BigEndianness : LittleEndianness;
   }

   while (iter != end)   {
      data = (data << 8) | *iter;

      ++iter;
      ++bytesRead;

      if (bytesRead == 4) {

         if (! headerDone) {

            if (endian == DetectEndianness) {
               if (data == 0x0000FEFF) {
                  endian = BigEndianness;
                  bytesRead = 0;
                  continue;

               } else if (data == 0xFFFE0000)  {
                  endian = LittleEndianness;
                  bytesRead = 0;
                  continue;

               } else if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
                  endian = BigEndianness;

               } else {
                  endian = LittleEndianness;
               }

            } else if (endian == BigEndianness && data == 0x0000FEFF) {
               bytesRead = 0;
               continue;

            } else if (endian == LittleEndianness && data == 0xFFFE0000) {
               bytesRead = 0;
               continue;
            }
         }

         if (endian == BigEndianness) {
            retval.append(char32_t(data));

         } else {
            data = (data & 0x0000FFFF) << 16 | (data & 0xFFFF0000) >> 16;
            char32_t tmp = (data & 0x00FF00FF) << 8 | (data & 0xFF00FF00) >> 8;
            retval.append(tmp);
         }

         bytesRead = 0;
      }
   }

   if (state) {
      if (headerDone) {
         state->m_flags |= QTextCodec::IgnoreHeader;
      }

      state->state_data[Endian] = endian;
      state->remainingChars     = bytesRead;
      memcpy(&state->state_data[Data], &data, 4);
   }

   return retval;
}

#ifndef QT_NO_TEXTCODEC

QUtf8Codec::~QUtf8Codec()
{
}

QByteArray QUtf8Codec::convertFromUnicode(QStringView str, ConverterState *state) const
{
   return QUtf8::convertFromUnicode(str, state);
}

void QUtf8Codec::convertToUnicode(QString *target, const char *chars, int len, ConverterState *state) const
{
   *target += QUtf8::convertToUnicode(chars, len, state);
}

QString QUtf8Codec::convertToUnicode(const char *chars, int len, ConverterState *state) const
{
   return QUtf8::convertToUnicode(chars, len, state);
}

QString QUtf8Codec::name() const
{
   return QString("UTF-8");
}

int QUtf8Codec::mibEnum() const
{
   return 106;
}

QUtf16Codec::~QUtf16Codec()
{
}

QByteArray QUtf16Codec::convertFromUnicode(QStringView str, ConverterState *state) const
{
   return QUtf16::convertFromUnicode(str, state, e);
}

QString QUtf16Codec::convertToUnicode(const char *chars, int len, ConverterState *state) const
{
   return QUtf16::convertToUnicode(chars, len, state, e);
}

int QUtf16Codec::mibEnum() const
{
   return 1015;
}

QString QUtf16Codec::name() const
{
   return QString("UTF-16");
}

QStringList QUtf16Codec::aliases() const
{
   return QStringList();
}

int QUtf16BECodec::mibEnum() const
{
   return 1013;
}

QString QUtf16BECodec::name() const
{
   return QString("UTF-16BE");
}

QStringList QUtf16BECodec::aliases() const
{
   QStringList list;
   return list;
}

int QUtf16LECodec::mibEnum() const
{
   return 1014;
}

QString QUtf16LECodec::name() const
{
   return QString("UTF-16LE");
}

QStringList QUtf16LECodec::aliases() const
{
   QStringList list;
   return list;
}

QUtf32Codec::~QUtf32Codec()
{
}

QByteArray QUtf32Codec::convertFromUnicode(QStringView str, ConverterState *state) const
{
   return QUtf32::convertFromUnicode(str, state, e);
}

QString QUtf32Codec::convertToUnicode(const char *chars, int len, ConverterState *state) const
{
   return QUtf32::convertToUnicode(chars, len, state, e);
}

int QUtf32Codec::mibEnum() const
{
   return 1017;
}

QString QUtf32Codec::name() const
{
   return QString("UTF-32");
}

QStringList QUtf32Codec::aliases() const
{
   return QStringList();
}

int QUtf32BECodec::mibEnum() const
{
   return 1018;
}

QString QUtf32BECodec::name() const
{
   return QString("UTF-32BE");
}

QStringList QUtf32BECodec::aliases() const
{
   QStringList list;
   return list;
}

int QUtf32LECodec::mibEnum() const
{
   return 1019;
}

QString QUtf32LECodec::name() const
{
   return QString("UTF-32LE");
}

QStringList QUtf32LECodec::aliases() const
{
   QStringList list;
   return list;
}

#endif
