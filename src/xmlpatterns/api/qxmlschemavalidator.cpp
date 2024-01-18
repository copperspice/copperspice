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

#include <qxmlschemavalidator.h>
#include <qxmlschemavalidator_p.h>

#include <qbuffer.h>
#include <qiodevice.h>
#include <qurl.h>
#include <qxmlschema.h>

#include <qacceltreeresourceloader_p.h>
#include <qxmlschema_p.h>
#include <qxsdvalidatinginstancereader_p.h>

QXmlSchemaValidator::QXmlSchemaValidator()
   : d(new QXmlSchemaValidatorPrivate(QXmlSchema()))
{
}

QXmlSchemaValidator::QXmlSchemaValidator(const QXmlSchema &schema)
   : d(new QXmlSchemaValidatorPrivate(schema))
{
}

QXmlSchemaValidator::~QXmlSchemaValidator()
{
   delete d;
}

void QXmlSchemaValidator::setSchema(const QXmlSchema &schema)
{
   d->setSchema(schema);
}

bool QXmlSchemaValidator::validate(const QByteArray &data, const QUrl &documentUri) const
{
   QByteArray localData(data);

   QBuffer buffer(&localData);
   buffer.open(QIODevice::ReadOnly);

   return validate(&buffer, documentUri);
}

bool QXmlSchemaValidator::validate(const QUrl &source) const
{
   d->m_context->setMessageHandler(messageHandler());
   d->m_context->setUriResolver(uriResolver());
   d->m_context->setNetworkAccessManager(networkAccessManager());

   const std::unique_ptr<QNetworkReply> reply(QPatternist::AccelTreeResourceLoader::load(source,
         d->m_context->networkAccessManager(),
         d->m_context, QPatternist::AccelTreeResourceLoader::ContinueOnError));

   if (reply) {
      return validate(reply.get(), source);
   } else {
      return false;
   }
}

bool QXmlSchemaValidator::validate(QIODevice *source, const QUrl &documentUri) const
{
   if (!source) {
      qWarning("A null QIODevice pointer cannot be passed.");
      return false;
   }

   if (!source->isReadable()) {
      qWarning("The device must be readable.");
      return false;
   }

   const QUrl normalizedUri = QPatternist::XPathHelper::normalizeQueryURI(documentUri);

   d->m_context->setMessageHandler(messageHandler());
   d->m_context->setUriResolver(uriResolver());
   d->m_context->setNetworkAccessManager(networkAccessManager());

   QPatternist::NetworkAccessDelegator::Ptr delegator(new QPatternist::NetworkAccessDelegator(
            d->m_context->networkAccessManager(),
            d->m_context->networkAccessManager()));

   QPatternist::AccelTreeResourceLoader loader(d->m_context->namePool(), delegator,
         QPatternist::AccelTreeBuilder<true>::SourceLocationsFeature);

   QPatternist::Item item;
   try {
      item = loader.openDocument(source, normalizedUri, d->m_context);
   } catch (QPatternist::Exception exception) {
      (void) exception;
      return false;
   }

   const QAbstractXmlNodeModel *model = item.asNode().model();

   QPatternist::XsdValidatedXmlNodeModel *validatedModel = new QPatternist::XsdValidatedXmlNodeModel(model);

   QPatternist::XsdValidatingInstanceReader reader(validatedModel, normalizedUri, d->m_context);
   if (d->m_schema) {
      reader.addSchema(d->m_schema, d->m_schemaDocumentUri);
   }
   try {
      reader.read();
   } catch (QPatternist::Exception exception) {
      (void) exception;
      return false;
   }

   return true;
}

QXmlNamePool QXmlSchemaValidator::namePool() const
{
   return d->m_namePool;
}

QXmlSchema QXmlSchemaValidator::schema() const
{
   return d->m_originalSchema;
}

void QXmlSchemaValidator::setMessageHandler(QAbstractMessageHandler *handler)
{
   d->m_userMessageHandler = handler;
}

QAbstractMessageHandler *QXmlSchemaValidator::messageHandler() const
{
   if (d->m_userMessageHandler) {
      return d->m_userMessageHandler;
   }

   return d->m_messageHandler.data()->value;
}

void QXmlSchemaValidator::setUriResolver(const QAbstractUriResolver *resolver)
{
   d->m_uriResolver = resolver;
}

const QAbstractUriResolver *QXmlSchemaValidator::uriResolver() const
{
   return d->m_uriResolver;
}

void QXmlSchemaValidator::setNetworkAccessManager(QNetworkAccessManager *manager)
{
   d->m_userNetworkAccessManager = manager;
}

QNetworkAccessManager *QXmlSchemaValidator::networkAccessManager() const
{
   if (d->m_userNetworkAccessManager) {
      return d->m_userNetworkAccessManager;
   }

   return d->m_networkAccessManager.data()->value;
}
