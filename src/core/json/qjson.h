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

#ifndef QJSON_H
#define QJSON_H

#include <qcontainerfwd.h>
#include <qflatmap.h>
#include <qjsonarray.h>
#include <qjsonobject.h>
#include <qjsonvalue.h>
#include <qstring.h>

class QJsonData
{
 public:
   virtual ~QJsonData()
   {
   }

   virtual std::unique_ptr<QJsonData> clone() const {
      return std::make_unique<QJsonData>();
   }

   virtual QJsonValue::Type type() const {
      return QJsonValue::Type::Undefined;
   }

   virtual bool toBool(bool defaultValue) const {
      return defaultValue;
   }

   virtual int toInt(int defaultValue) const {
      return defaultValue;
   }

   virtual double toDouble(double defaultValue) const {
      return defaultValue;
   }

   virtual QString toString(const QString &defaultValue) const {
      return defaultValue;
   }

   virtual QJsonArray toArray(const QJsonArray &defaultValue) const {
      return defaultValue;
   }

   virtual QJsonObject toObject(const QJsonObject &defaultValue) const {
      return defaultValue;
   }
};

class QJsonDataObject : public QJsonData
{
 public:
   QJsonDataObject() = default;

   QJsonDataObject(QFlatMap<QString, QJsonValue> value)
      : m_map(std::move(value))
   { }

   std::unique_ptr<QJsonData> clone() const override {
      return std::make_unique<QJsonDataObject>(*this);
   }

   QJsonObject toObject(const QJsonObject &) const override {
      return QJsonObject(m_map.begin(), m_map.end());
   }

   QJsonValue::Type type() const override {
      return QJsonValue::Type::Object;
   }

   // data
   QFlatMap<QString, QJsonValue> m_map;
};

class QJsonDataArray : public QJsonData
{
 public:
   QJsonDataArray() = default;

   QJsonDataArray(QVector<QJsonValue> value)
      : m_vector(std::move(value))
   { }

   std::unique_ptr<QJsonData> clone() const override {
      return std::make_unique<QJsonDataArray>(*this);
   }

   QJsonArray toArray(const QJsonArray &) const override {
      return QJsonArray(m_vector.begin(), m_vector.end());
   }

   QJsonValue::Type type() const override {
      return QJsonValue::Type::Array;
   }

   // data
   QVector<QJsonValue> m_vector;
};

class QJsonDataBool : public QJsonData
{
 public:
   QJsonDataBool(bool value)
      : m_data(value)
   { }

   std::unique_ptr<QJsonData> clone() const override {
      return std::make_unique<QJsonDataBool>(*this);
   }

   bool toBool(bool) const override {
      return m_data;
   }

   QJsonValue::Type type() const override {
      return QJsonValue::Type::Bool;
   }

   // data
   bool m_data;
};

class QJsonDataNull : public QJsonData
{
 public:
   QJsonDataNull()
   { }

   std::unique_ptr<QJsonData> clone() const override {
      return std::make_unique<QJsonDataNull>(*this);
   }

   int toInt(int defaultValue) const override {
      return defaultValue;
   }

   double toDouble(double defaultValue) const override {
      return defaultValue;
   }

   QJsonValue::Type type() const override {
      return QJsonValue::Type::Null;
   }
};

class QJsonDataNumber : public QJsonData
{
 public:
   QJsonDataNumber(double value)
      : m_data(value)
   { }

   std::unique_ptr<QJsonData> clone() const override {
      return std::make_unique<QJsonDataNumber>(*this);
   }

   int toInt(int) const override {
      return m_data;
   }

   double toDouble(double) const override {
      return m_data;
   }

   QJsonValue::Type type() const override {
      return QJsonValue::Type::Double;
   }

   // data
   double m_data;
};

class QJsonDataString : public QJsonData
{
 public:
   QJsonDataString(QString value)
      : m_data(std::move(value))
   { }

   std::unique_ptr<QJsonData> clone() const override {
      return std::make_unique<QJsonDataString>(*this);
   }

   QString toString(const QString &) const override {
      return m_data;
   }

   QJsonValue::Type type() const override {
      return QJsonValue::Type::String;
   }

   // data
   QString m_data;
};

#endif
