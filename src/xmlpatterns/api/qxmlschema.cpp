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

#include "qxmlschema.h"
#include "qxmlschema_p.h"

#include <qiodevice.h>
#include <qurl.h>

QXmlSchema::QXmlSchema()
   : d(new QXmlSchemaPrivate(QXmlNamePool()))
{
}

QXmlSchema::~QXmlSchema()
{
}

bool QXmlSchema::load(const QUrl &source)
{
   d->load(source, QString());
   return d->isValid();
}

bool QXmlSchema::load(QIODevice *source, const QUrl &documentUri)
{
   d->load(source, documentUri, QString());
   return d->isValid();
}

bool QXmlSchema::load(const QByteArray &data, const QUrl &documentUri)
{
   d->load(data, documentUri, QString());
   return d->isValid();
}

bool QXmlSchema::isValid() const
{
   return d->isValid();
}

QXmlNamePool QXmlSchema::namePool() const
{
   return d->namePool();
}

QUrl QXmlSchema::documentUri() const
{
   return d->documentUri();
}

void QXmlSchema::setMessageHandler(QAbstractMessageHandler *handler)
{
   d->setMessageHandler(handler);
}

QAbstractMessageHandler *QXmlSchema::messageHandler() const
{
   return d->messageHandler();
}

void QXmlSchema::setUriResolver(const QAbstractUriResolver *resolver)
{
   d->setUriResolver(resolver);
}

const QAbstractUriResolver *QXmlSchema::uriResolver() const
{
   return d->uriResolver();
}

void QXmlSchema::setNetworkAccessManager(QNetworkAccessManager *manager)
{
   d->setNetworkAccessManager(manager);
}

QNetworkAccessManager *QXmlSchema::networkAccessManager() const
{
   return d->networkAccessManager();
}

