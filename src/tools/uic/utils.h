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

#ifndef UTILS_H
#define UTILS_H

#include <qlist.h>
#include <qset.h>
#include <qstring.h>
#include <ui4.h>

inline bool toBool(const QString &str)
{
   return str.toLower() == "true";
}

inline QString toString(const DomString *str)
{
   return str ? str->text() : QString();
}

inline QString fixString(const QString &str, const QString &indent)
{
   QString cursegment;
   QStringList result;

   const QByteArray utf8 = str.toUtf8();
   const int utf8Length  = utf8.length();

   for (int i = 0; i < utf8Length; ++i) {
      const uchar cbyte = utf8.at(i);

      if (cbyte >= 0x80) {
         cursegment += '\\';
         cursegment += QString::number(cbyte, 8);

      } else {
         switch (cbyte) {
            case '\\':
               cursegment += "\\\\";
               break;

            case '\"':
               cursegment += "\\\"";
               break;

            case '\r':
               break;

            case '\n':
               cursegment += "\\n\"\n\"";
               break;

            default:
               cursegment += cbyte;
         }
      }

      if (cursegment.length() > 1024) {
         result << cursegment;
         cursegment.clear();
      }
   }

   if (! cursegment.isEmpty()) {
      result << cursegment;
   }

   QString joinstr = "\"\n";
   joinstr += indent;
   joinstr += indent;
   joinstr += '"';

   QString rc('"');
   rc += result.join(joinstr);
   rc += '"';

   return rc;
}

inline QHash<QString, DomProperty *> propertyMap(const QList<DomProperty *> &properties)
{
   QHash<QString, DomProperty *> retval;

   for (auto item : properties) {
      retval.insert(item->attributeName(), item);
   }

   return retval;
}

inline QStringList unique(const QStringList &list)
{
   QSet<QString> retval;

   for (auto item : list) {
      retval.insert(item);
   }

   return retval.toList();
}

#endif
