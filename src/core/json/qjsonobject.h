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

#ifndef QJSONOBJECT_H
#define QJSONOBJECT_H

#include <qflatmap.h>
#include <qhash.h>
#include <qjsonvalue.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qvariant.h>

class QJsonData;
class QJsonDataObject;

using QVariantHash = QHash<QString, QVariant>;
using QVariantMap  = QMap<QString, QVariant>;

class Q_CORE_EXPORT QJsonObject
{
 public:
   using iterator       = QFlatMap<QString, QJsonValue>::iterator;
   using const_iterator = QFlatMap<QString, QJsonValue>::const_iterator;

   using size_type      = QFlatMap<QString, QJsonValue>::size_type;
   using key_type       = QString;
   using mapped_type    = QJsonValue;

   QJsonObject();
   QJsonObject(const_iterator iter_begin, const_iterator iter_end);
   QJsonObject(std::initializer_list<QPair<QString, QJsonValue>> list);
   QJsonObject(const QJsonObject &other);
   QJsonObject(QJsonObject &&other);

   QJsonObject &operator =(const QJsonObject &other);
   ~QJsonObject();

   // methods
   bool contains(const QString &key) const;

   int count() const {
      return size();
   }

   bool empty() const {
      return isEmpty();
   }

   iterator erase(const_iterator iter);
   iterator find(const QString &key);

   const_iterator find(const QString &key) const {
      return constFind(key);
   }

   const_iterator constFind(const QString &key) const;

   static QJsonObject fromVariantHash(const QVariantHash &hash);
   static QJsonObject fromVariantMap(const QVariantMap &map);

   bool isEmpty() const;

   iterator insert(const QString &key, QJsonValue value);
   QStringList keys() const;

   int length() const {
      return size();
   }

   void remove(const QString &key);

   size_type size() const;

   QJsonValue take(const QString &key);

   QVariantHash toVariantHash() const;
   QVariantMap toVariantMap() const;

   const QJsonValue &value(const QString &key) const;

   // operators
   bool operator==(const QJsonObject &other) const;
   bool operator!=(const QJsonObject &other) const;

   const QJsonValue &operator[] (const QString &key) const;
   QJsonValue &operator[] (const QString &key);

   // iterators
   iterator begin();
   const_iterator begin() const;
   const_iterator constBegin() const;

   iterator end();
   const_iterator end() const;
   const_iterator constEnd() const;

 private:
   friend class QJsonValue;
   std::shared_ptr<QJsonDataObject> m_object;
};

#endif