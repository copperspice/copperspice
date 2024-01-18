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

#ifndef QJSONPARSER_P_H
#define QJSONPARSER_P_H

#include <qjsondocument.h>
#include <qjsonvalue.h>
#include <qstring.h>

class QJsonParser
{
 public:
   enum TokenType {
      Null           = 0x00,
      Space          = 0x20,
      Tab            = 0x09,
      LineFeed       = 0x0a,
      Return         = 0x0d,
      BeginArray     = 0x5b,
      BeginObject    = 0x7b,
      EndArray       = 0x5d,
      EndObject      = 0x7d,
      NameSeparator  = 0x3a,
      ValueSeparator = 0x2c,
      Quote          = 0x22
   };

   QJsonParser(QStringView data);
   QJsonDocument parse(QJsonParseError *error);

 private:
   void eatBOM();
   bool eatWhiteSpace();
   TokenType nextToken();

   bool parseArray(QJsonArray &array);
   bool parseObject(QJsonObject &object);
   bool parseMember(QJsonObject &object);
   bool parseString(QString &str);

   bool parseValue(QJsonValue &value);
   bool parseNumber(QJsonValue &value);

   QStringView m_data;
   QString::const_iterator m_position;

   int nestingLevel;
   QJsonParseError::ParseError lastError;
};

#endif
