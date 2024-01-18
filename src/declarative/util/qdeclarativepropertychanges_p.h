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

#ifndef QDECLARATIVEPROPERTYCHANGES_P_H
#define QDECLARATIVEPROPERTYCHANGES_P_H

#include <qdeclarativestateoperations_p.h>
#include <qdeclarativecustomparser_p.h>

QT_BEGIN_NAMESPACE

class QDeclarativePropertyChangesPrivate;

class Q_DECLARATIVE_PRIVATE_EXPORT QDeclarativePropertyChanges : public QDeclarativeStateOperation
{
   DECL_CS_OBJECT(QDeclarativePropertyChanges)
   Q_DECLARE_PRIVATE(QDeclarativePropertyChanges)

   DECL_CS_PROPERTY_READ(*target, object)
   DECL_CS_PROPERTY_WRITE(*target, setObject)
   DECL_CS_PROPERTY_READ(restoreEntryValues, restoreEntryValues)
   DECL_CS_PROPERTY_WRITE(restoreEntryValues, setRestoreEntryValues)
   DECL_CS_PROPERTY_READ(explicit, isExplicit)
   DECL_CS_PROPERTY_WRITE(explicit, setIsExplicit)

 public:
   QDeclarativePropertyChanges();
   ~QDeclarativePropertyChanges();

   QObject *object() const;
   void setObject(QObject *);

   bool restoreEntryValues() const;
   void setRestoreEntryValues(bool);

   bool isExplicit() const;
   void setIsExplicit(bool);

   virtual ActionList actions();

   bool containsProperty(const QString &name) const;
   bool containsValue(const QString &name) const;
   bool containsExpression(const QString &name) const;
   void changeValue(const QString &name, const QVariant &value);
   void changeExpression(const QString &name, const QString &expression);
   void removeProperty(const QString &name);
   QVariant value(const QString &name) const;
   QString expression(const QString &name) const;

   void detachFromState();
   void attachToState();

   QVariant property(const QString &name) const;
};

class QDeclarativePropertyChangesParser : public QDeclarativeCustomParser
{
 public:
   QDeclarativePropertyChangesParser()
      : QDeclarativeCustomParser(AcceptsAttachedProperties) {}

   void compileList(QList<QPair<QByteArray, QVariant> > &list, const QByteArray &pre,
                    const QDeclarativeCustomParserProperty &prop);

   virtual QByteArray compile(const QList<QDeclarativeCustomParserProperty> &);
   virtual void setCustomData(QObject *, const QByteArray &);
};


QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativePropertyChanges)

#endif // QDECLARATIVEPROPERTYCHANGES_H
