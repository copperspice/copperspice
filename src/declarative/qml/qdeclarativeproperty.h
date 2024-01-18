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

#ifndef QDECLARATIVEPROPERTY_H
#define QDECLARATIVEPROPERTY_H

#include <QtCore/qmetaobject.h>

QT_BEGIN_NAMESPACE

class QObject;
class QVariant;
class QDeclarativeContext;
class QDeclarativeEngine;
class QDeclarativePropertyPrivate;

class Q_DECLARATIVE_EXPORT QDeclarativeProperty
{
 public:
   enum PropertyTypeCategory {
      InvalidCategory,
      List,
      Object,
      Normal
   };

   enum Type {
      Invalid,
      Property,
      SignalProperty
   };

   QDeclarativeProperty();
   ~QDeclarativeProperty();

   QDeclarativeProperty(QObject *);
   QDeclarativeProperty(QObject *, QDeclarativeContext *);
   QDeclarativeProperty(QObject *, QDeclarativeEngine *);

   QDeclarativeProperty(QObject *, const QString &);
   QDeclarativeProperty(QObject *, const QString &, QDeclarativeContext *);
   QDeclarativeProperty(QObject *, const QString &, QDeclarativeEngine *);

   QDeclarativeProperty(const QDeclarativeProperty &);
   QDeclarativeProperty &operator=(const QDeclarativeProperty &);

   bool operator==(const QDeclarativeProperty &) const;

   Type type() const;
   bool isValid() const;
   bool isProperty() const;
   bool isSignalProperty() const;

   int propertyType() const;
   PropertyTypeCategory propertyTypeCategory() const;
   const char *propertyTypeName() const;

   QString name() const;

   QVariant read() const;
   static QVariant read(QObject *, const QString &);
   static QVariant read(QObject *, const QString &, QDeclarativeContext *);
   static QVariant read(QObject *, const QString &, QDeclarativeEngine *);

   bool write(const QVariant &) const;
   static bool write(QObject *, const QString &, const QVariant &);
   static bool write(QObject *, const QString &, const QVariant &, QDeclarativeContext *);
   static bool write(QObject *, const QString &, const QVariant &, QDeclarativeEngine *);

   bool reset() const;

   bool hasNotifySignal() const;
   bool needsNotifySignal() const;
   bool connectNotifySignal(QObject *dest, const char *slot) const;
   bool connectNotifySignal(QObject *dest, int method) const;

   bool isWritable() const;
   bool isDesignable() const;
   bool isResettable() const;
   QObject *object() const;

   int index() const;
   QMetaProperty property() const;
   QMetaMethod method() const;

 private:
   friend class QDeclarativePropertyPrivate;
   QDeclarativePropertyPrivate *d;
};
typedef QList<QDeclarativeProperty> QDeclarativeProperties;

inline uint qHash (const QDeclarativeProperty &key)
{
   return qHash(key.object()) + qHash(key.name());
}

QT_END_NAMESPACE

#endif // QDECLARATIVEPROPERTY_H
