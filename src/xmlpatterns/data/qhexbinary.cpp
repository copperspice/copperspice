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

#include <qglobal.h>

#include "qhexbinary_p.h"
#include "qbase64binary_p.h"
#include "qbuiltintypes_p.h"
#include "qpatternistlocale_p.h"
#include "qvalidationerror_p.h"

using namespace QPatternist;

HexBinary::HexBinary(const QByteArray &val) : Base64Binary(val)
{
}

qint8 HexBinary::fromHex(const QChar &c)
{
   if (c.unicode() > 'f') {
      return -1;
   }

   const char *const range = "0123456789ABCDEFabcdef";

   const char *const in = strchr(range, c.unicode());

   if (!in) {
      return -1;
   }

   /* Pointer arithmetic. */
   int digit = in - range;

   if (digit > 15) {
      digit -= 6;
   }

   return digit;
}

AtomicValue::Ptr HexBinary::fromLexical(const NamePool::Ptr &np, const QString &str)
{
   const QString lexical(str.trimmed());
   const int len = lexical.length();

   if (len == 0) {
      return AtomicValue::Ptr(new HexBinary(QByteArray()));
   }

   if ((len & 1) != 0) {
      /* Catch a common case. */
      return ValidationError::createError(QtXmlPatterns::tr("A value of type %1 must contain an even number of "
                  "digits. The value %2 does not.")
                  .formatArgs(formatType(np, BuiltinTypes::xsHexBinary), formatData(QString::number(len))));
   }

   QByteArray val;
   val.resize(len / 2);

   for (int i = 0; i < len / 2; ++i) {
      qint8 p1 = fromHex(lexical[i * 2]);
      qint8 p2 = fromHex(lexical[i * 2 + 1]);

      if (p1 == -1 || p2 == -1) {
         const QString hex(QString("%1%2").formatArgs(lexical[i * 2], lexical[i * 2 + 1]));

         return ValidationError::createError(QtXmlPatterns::tr("%1 is not valid as a value of type %2.")
                  .formatArgs(formatData(hex), formatType(np, BuiltinTypes::xsHexBinary)));
      }

      val[i] = static_cast<char>(p1 * 16 + p2);
   }

   Q_ASSERT(!val.isEmpty());

   return AtomicValue::Ptr(new HexBinary(val));
}

HexBinary::Ptr HexBinary::fromValue(const QByteArray &data)
{
   return HexBinary::Ptr(new HexBinary(data));
}

QString HexBinary::stringValue() const
{
   static const char s_toHex[] = "0123456789ABCDEF";
   const int len = m_value.count();

   QString result;

   for (int i = 0; i < len; ++i) {
      // This cast is significant.
      const unsigned char val = static_cast<unsigned char>(m_value.at(i));
      result += QLatin1Char(s_toHex[val >> 4]);
      result += QLatin1Char(s_toHex[val & 0x0F]);
   }

   return result;
}

ItemType::Ptr HexBinary::type() const
{
   return BuiltinTypes::xsHexBinary;
}

