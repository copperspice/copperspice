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

#ifndef QDECLARATIVECUSTOMPARSER_P_P_H
#define QDECLARATIVECUSTOMPARSER_P_P_H

#include <qdeclarativecustomparser_p.h>
#include <qdeclarativeparser_p.h>
#include <QtCore/qglobal.h>

QT_BEGIN_NAMESPACE

class QDeclarativeCustomParserNodePrivate
{
 public:
   QByteArray name;
   QList<QDeclarativeCustomParserProperty> properties;
   QDeclarativeParser::Location location;

   static QDeclarativeCustomParserNode fromObject(QDeclarativeParser::Object *);
   static QDeclarativeCustomParserProperty fromProperty(QDeclarativeParser::Property *);
};

class QDeclarativeCustomParserPropertyPrivate
{
 public:
   QDeclarativeCustomParserPropertyPrivate()
      : isList(false) {}

   QByteArray name;
   bool isList;
   QDeclarativeParser::Location location;
   QList<QVariant> values;
};

QT_END_NAMESPACE

#endif // QDECLARATIVECUSTOMPARSER_P_H
