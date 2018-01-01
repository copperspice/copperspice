/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QJSONPARSER_P_H
#define QJSONPARSER_P_H

#include <qjsondocument.h>
#include <qvarlengtharray.h>

#include <stdlib.h>

QT_BEGIN_NAMESPACE

namespace QJsonPrivate {

class Parser
{
 public:
   Parser(const char *json, int length);

   QJsonDocument parse(QJsonParseError *error);

   class ParsedObject
   {
    public:
      ParsedObject(Parser *p, int pos) : parser(p), objectPosition(pos) {}
      void insert(uint offset);

      Parser *parser;
      int objectPosition;
      QVarLengthArray<uint, 64> offsets;

      inline QJsonPrivate::Entry *entryAt(int i) const {
         return reinterpret_cast<QJsonPrivate::Entry *>(parser->data + objectPosition + offsets[i]);
      }
   };


 private:
   inline void eatBOM();
   inline bool eatSpace();
   inline char nextToken();

   bool parseObject();
   bool parseArray();
   bool parseMember(int baseOffset);
   bool parseString(bool *latin1);
   bool parseValue(QJsonPrivate::Value *val, int baseOffset);
   bool parseNumber(QJsonPrivate::Value *val, int baseOffset);
   const char *head;
   const char *json;
   const char *end;

   char *data;
   int dataLength;
   int current;
   int nestingLevel;
   QJsonParseError::ParseError lastError;

   inline int reserveSpace(int space) {
      if (current + space >= dataLength) {
         dataLength = 2 * dataLength + space;
         data = (char *)realloc(data, dataLength);
      }
      int pos = current;
      current += space;
      return pos;
   }
};

}

QT_END_NAMESPACE

#endif
