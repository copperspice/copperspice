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

#ifndef QDECLARATIVEPROPERTYMAP_H
#define QDECLARATIVEPROPERTYMAP_H

#include <QtCore/QObject>
#include <QtCore/QHash>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

QT_BEGIN_NAMESPACE

class QDeclarativePropertyMapPrivate;

class Q_DECLARATIVE_EXPORT QDeclarativePropertyMap : public QObject
{
   DECL_CS_OBJECT(QDeclarativePropertyMap)

 public:
   QDeclarativePropertyMap(QObject *parent = nullptr);
   virtual ~QDeclarativePropertyMap();

   QVariant value(const QString &key) const;
   void insert(const QString &key, const QVariant &value);
   void clear(const QString &key);

   Q_INVOKABLE QStringList keys() const;

   int count() const;
   int size() const;
   bool isEmpty() const;
   bool contains(const QString &key) const;

   QVariant &operator[](const QString &key);
   QVariant operator[](const QString &key) const;

 public:
   DECL_CS_SIGNAL_1(Public, void valueChanged(const QString &key, const QVariant &value))
   DECL_CS_SIGNAL_2(valueChanged, key, value)

 private:
   Q_DECLARE_PRIVATE(QDeclarativePropertyMap)
   Q_DISABLE_COPY(QDeclarativePropertyMap)
};

QT_END_NAMESPACE

#endif
