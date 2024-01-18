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

#ifndef QDECLARATIVEBIND_P_H
#define QDECLARATIVEBIND_P_H

#include <qdeclarative.h>
#include <QtCore/qobject.h>

QT_BEGIN_NAMESPACE

class QDeclarativeBindPrivate;

class QDeclarativeBind : public QObject, public QDeclarativeParserStatus
{
   DECL_CS_OBJECT(QDeclarativeBind)
   Q_DECLARE_PRIVATE(QDeclarativeBind)
   CS_INTERFACES(QDeclarativeParserStatus)
   DECL_CS_PROPERTY_READ(*target, object)
   DECL_CS_PROPERTY_WRITE(*target, setObject)
   DECL_CS_PROPERTY_READ(property, property)
   DECL_CS_PROPERTY_WRITE(property, setProperty)
   DECL_CS_PROPERTY_READ(value, value)
   DECL_CS_PROPERTY_WRITE(value, setValue)
   DECL_CS_PROPERTY_READ(when, when)
   DECL_CS_PROPERTY_WRITE(when, setWhen)

 public:
   QDeclarativeBind(QObject *parent = nullptr);
   ~QDeclarativeBind();

   bool when() const;
   void setWhen(bool);

   QObject *object();
   void setObject(QObject *);

   QString property() const;
   void setProperty(const QString &);

   QVariant value() const;
   void setValue(const QVariant &);

 protected:
   virtual void classBegin();
   virtual void componentComplete();

 private:
   void eval();
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeBind)

#endif
