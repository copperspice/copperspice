/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QXMLSCHEMA_H
#define QXMLSCHEMA_H

#include <QtCore/QSharedDataPointer>
#include <QtCore/QUrl>
#include <QtXmlPatterns/QXmlNamePool>

QT_BEGIN_NAMESPACE

class QAbstractMessageHandler;
class QAbstractUriResolver;
class QIODevice;
class QNetworkAccessManager;
class QUrl;
class QXmlNamePool;
class QXmlSchemaPrivate;

class Q_XMLPATTERNS_EXPORT QXmlSchema
{
   friend class QXmlSchemaValidatorPrivate;

 public:
   QXmlSchema();
   QXmlSchema(const QXmlSchema &other);
   ~QXmlSchema();

   bool load(const QUrl &source);
   bool load(QIODevice *source, const QUrl &documentUri = QUrl());
   bool load(const QByteArray &data, const QUrl &documentUri = QUrl());

   bool isValid() const;

   QXmlNamePool namePool() const;
   QUrl documentUri() const;

   void setMessageHandler(QAbstractMessageHandler *handler);
   QAbstractMessageHandler *messageHandler() const;

   void setUriResolver(const QAbstractUriResolver *resolver);
   const QAbstractUriResolver *uriResolver() const;

   void setNetworkAccessManager(QNetworkAccessManager *networkmanager);
   QNetworkAccessManager *networkAccessManager() const;

 private:
   QSharedDataPointer<QXmlSchemaPrivate> d;
};

QT_END_NAMESPACE

#endif
