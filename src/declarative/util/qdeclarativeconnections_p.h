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

#ifndef QDECLARATIVECONNECTIONS_P_H
#define QDECLARATIVECONNECTIONS_P_H

#include <qdeclarative.h>
#include <qdeclarativescriptstring.h>
#include <qdeclarativecustomparser_p.h>
#include <QtCore/qobject.h>
#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

class QDeclarativeBoundSignal;
class QDeclarativeContext;
class QDeclarativeConnectionsPrivate;

class QDeclarativeConnections : public QObject, public QDeclarativeParserStatus
{
   DECL_CS_OBJECT(QDeclarativeConnections)
   Q_DECLARE_PRIVATE(QDeclarativeConnections)

   CS_INTERFACES(QDeclarativeParserStatus)
   DECL_CS_PROPERTY_READ(*target, target)
   DECL_CS_PROPERTY_WRITE(*target, setTarget)
   DECL_CS_PROPERTY_NOTIFY(*target, targetChanged)
   DECL_CS_PROPERTY_READ(ignoreUnknownSignals, ignoreUnknownSignals)
   DECL_CS_PROPERTY_WRITE(ignoreUnknownSignals, setIgnoreUnknownSignals)

 public:
   QDeclarativeConnections(QObject *parent = nullptr);
   ~QDeclarativeConnections();

   QObject *target() const;
   void setTarget(QObject *);

   bool ignoreUnknownSignals() const;
   void setIgnoreUnknownSignals(bool ignore);

   DECL_CS_SIGNAL_1(Public, void targetChanged())
   DECL_CS_SIGNAL_2(targetChanged)

 private:
   void connectSignals();
   void classBegin();
   void componentComplete();
};

class QDeclarativeConnectionsParser : public QDeclarativeCustomParser
{
 public:
   virtual QByteArray compile(const QList<QDeclarativeCustomParserProperty> &);
   virtual void setCustomData(QObject *, const QByteArray &);
};


QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeConnections)

#endif
