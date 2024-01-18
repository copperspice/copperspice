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

#ifndef QJSONDOCUMENT_H
#define QJSONDOCUMENT_H

#include <qjson.h>
#include <qjsonarray.h>
#include <qjsonobject.h>
#include <qjsonvalue.h>
#include <qstring.h>

class QJsonParser;

struct Q_CORE_EXPORT QJsonParseError {
   enum ParseError {
      NoError = 0,
      UnterminatedObject,
      MissingNameSeparator,
      UnterminatedArray,
      MissingValueSeparator,
      IllegalValue,
      TerminationByNumber,
      IllegalNumber,
      IllegalEscapeSequence,
      IllegalUTF8String,
      UnterminatedString,
      MissingObject,
      DeepNesting,
      DocumentTooLarge
   };

   QString errorString() const;

   int offset;
   ParseError error;
};

class Q_CORE_EXPORT QJsonDocument
{
 public:
   enum DataValidation {
      Validate,
      BypassValidation
   };

   enum JsonFormat {
      Indented,
      Compact
   };

   QJsonDocument();
   explicit QJsonDocument(const QJsonObject &object);
   explicit QJsonDocument(const QJsonArray &array);

   QJsonDocument(const QJsonDocument &other);
   QJsonDocument(QJsonDocument &&other);

   ~QJsonDocument();

   // methods
   QJsonArray array() const;

   bool isEmpty() const;
   bool isArray() const;
   bool isObject() const;
   bool isNull() const;

   QJsonObject object() const;

   void setObject(const QJsonObject &object);
   void setArray(const QJsonArray &array);

   QVariant toVariant() const;

   QByteArray toJson(JsonFormat format = Indented) const {
      return toJsonByteArray(format);
   }

   QByteArray toJsonByteArray(JsonFormat format = Indented) const {
      return toJsonString(format).toUtf8();
   }

   QString toJsonString(JsonFormat format = Indented) const;

   //
   static QJsonDocument fromJson(QStringView json, QJsonParseError *error = nullptr);
   static QJsonDocument fromJson(const QByteArray &json, QJsonParseError *error = nullptr);
   static QJsonDocument fromVariant(const QVariant &variant);

   // operators
   QJsonDocument &operator =(const QJsonDocument &other);

   bool operator==(const QJsonDocument &other) const;

   bool operator!=(const QJsonDocument &other) const {
      return ! (*this == other);
   }

 private:
   std::shared_ptr<QJsonValue> m_data;
};

#endif
