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

#include <qjsonwriter_p.h>

#include <qjson.h>
#include <qstringparser.h>

#include <cmath>

static void cs_internal_objectToStr(const QJsonObject &data, QString &retval, int indent, bool compact);
static void cs_internal_arrayToStr(const QJsonArray &data,   QString &retval, int indent, bool compact);

static uchar hexdig(uint u)
{
   return (u < 0xa ? '0' + u : 'a' + u - 0xa);
}

static QString escapedString(const QString &s)
{
   QString retval;
   const uchar replacement = '?';

   for (QChar c : s) {
      char32_t uc = c.unicode();

      if (uc >= 0xD800 && uc <= 0xDFFF) {
         retval.append(replacement);
         continue;
      }

      if (uc < 0x80) {

         if (uc < 0x20 || uc == 0x22 || uc == 0x5c) {
            retval.append('\\');

            switch (uc) {
               case 0x22:
                  retval.append('"');
                  break;

               case 0x5c:
                  retval.append('\\');
                  break;

               case 0x8:
                  retval.append('b');
                  break;

               case 0xc:
                  retval.append('f');
                  break;

               case 0xa:
                  retval.append('n');
                  break;

               case 0xd:
                  retval.append('r');
                  break;

               case 0x9:
                  retval.append('t');
                  break;

               default:
                  retval.append("u00");

                  retval.append(hexdig(uc >> 4));
                  retval.append(hexdig(uc & 0xf));
            }

         } else {
            retval.append(uc);
         }

      } else {
         if (uc < 0x0800) {
            retval.append(0xc0 | (uc >> 6));

         } else {

            if (uc >= 0x10000) {
               retval.append(0xf0 | (uc >> 18));
               retval.append(0x80 | ((uc >> 12) & 0x3f));

            } else {
               retval.append(0xe0 | ((uc >> 12) & 0x3f));

            }

            retval.append(0x80 | ((uc >> 6) & 0x3f));
         }

         retval.append(0x80 | (uc & 0x3f));
      }

   }

   return retval;
}

static void valueToJson(const QJsonValue &value, QString &retval, int indent, bool compact)
{
   QJsonValue::Type type = value.type();

   switch (type) {

      case QJsonValue::Bool:
         retval += value.toBool() ? QString("true") : QString("false");
         break;

      case QJsonValue::Double: {
         const double d = value.toDouble();

         if (std::isfinite(d)) {
            retval += QString::number(d, 'g', std::numeric_limits<double>::digits10 + 2);

         } else {
            retval += "null";
         }

         break;
      }

      case QJsonValue::String:
         retval += '"' + escapedString(value.toString()) + '"';
         break;

      case QJsonValue::Array:
         retval += compact ? "[" : "[\n";
         cs_internal_arrayToStr(value.toArray(), retval, indent + (compact ? 0 : 1), compact);
         retval += QString(4 * indent, ' ') + "]";
         break;

      case QJsonValue::Object:
         retval += compact ? "{" : "{\n";
         cs_internal_objectToStr(value.toObject(), retval, indent + (compact ? 0 : 1), compact);
         retval += QString(4 * indent, ' ') + "}";
         break;

      case QJsonValue::Null:
      default:
         retval += "null";
   }
}

static void cs_internal_arrayToStr(const QJsonArray &data, QString &retval, int indent, bool compact)
{
   if (data.isEmpty()) {
      return;
   }

   QString indentString(4 * indent, ' ');
   uint i = 0;

   while (true) {
      retval += indentString;
      valueToJson(data.at(i), retval, indent, compact);

      if (++i == data.size()) {
         if (! compact) {
            retval += '\n';
         }

         break;
      }

      retval += compact ? "," : ",\n";
   }
}

static void cs_internal_objectToStr(const QJsonObject &data, QString  &retval, int indent, bool compact)
{
   if (data.isEmpty()) {
      return;
   }

   QString indentString(4 * indent, ' ');
   QJsonObject::const_iterator iter = data.begin();

   while (iter != data.end()) {
      retval += indentString + '"' + escapedString(iter.key()) + "\": ";

      valueToJson(iter.value(), retval, indent, compact);

      if (++iter == data.end()) {
         if (! compact) {
            retval += '\n';
         }

         break;
      }

      retval += compact ? "," : ",\n";
   }
}

QString QJsonWriter::arrayToString(const QJsonArray &data, int indent, QJsonDocument::JsonFormat format)
{
   bool compact = (format == QJsonDocument::Compact);

   QString retval;
   retval += compact ? QString("[") : QString("[\n");

   cs_internal_arrayToStr(data, retval, indent + (compact ? 0 : 1), compact);

   retval += QString(4 * indent, ' ');
   retval += compact ? QString("]") : QString("]\n");

   return retval;
}

QString QJsonWriter::objectToString(const QJsonObject &data,  int indent, QJsonDocument::JsonFormat format)
{
   bool compact = (format == QJsonDocument::Compact);

   QString retval;
   retval += compact ? QString("{") : QString("{\n");

   cs_internal_objectToStr(data, retval, indent + (compact ? 0 : 1), compact);

   retval += QString(4 * indent, ' ');
   retval += compact ? QString("}") : QString("}\n");

   return retval;
}
