/***********************************************************************
*
* Copyright (c) 2012-2015 Barbara Geller
* Copyright (c) 2012-2015 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QDECLARATIVEPACKAGE_P_H
#define QDECLARATIVEPACKAGE_P_H

#include <qdeclarative.h>

QT_BEGIN_NAMESPACE

class QDeclarativePackagePrivate;
class QDeclarativePackageAttached;

class QDeclarativePackage : public QObject
{
   DECL_CS_OBJECT(QDeclarativePackage)
   Q_DECLARE_PRIVATE(QDeclarativePackage)

   DECL_CS_CLASSINFO("DefaultProperty", "data")
   CS_PROPERTY_READ(data, data)
   CS_PROPERTY_SCRIPTABLE(data, false)

 public:
   QDeclarativePackage(QObject *parent = 0);
   virtual ~QDeclarativePackage();

   QDeclarativeListProperty<QObject> data();

   QObject *part(const QString & = QString());
   bool hasPart(const QString &);

   static QDeclarativePackageAttached *qmlAttachedProperties(QObject *);
};

class QDeclarativePackageAttached : public QObject
{
   DECL_CS_OBJECT(QDeclarativePackageAttached)
   CS_PROPERTY_READ(name, name)
   CS_PROPERTY_WRITE(name, setName)
 public:
   QDeclarativePackageAttached(QObject *parent);
   virtual ~QDeclarativePackageAttached();

   QString name() const;
   void setName(const QString &n);

   static QHash<QObject *, QDeclarativePackageAttached *> attached;
 private:
   QString _name;
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativePackage)
QML_DECLARE_TYPEINFO(QDeclarativePackage, QML_HAS_ATTACHED_PROPERTIES)

#endif // QDECLARATIVEPACKAGE_H
