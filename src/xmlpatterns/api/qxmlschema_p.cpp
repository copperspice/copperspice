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

#include "qacceltreeresourceloader_p.h"
#include "qxmlschema.h"
#include "qxmlschema_p.h"

#include <QBuffer>
#include <QIODevice>
#include <QUrl>

QXmlSchemaPrivate::QXmlSchemaPrivate(const QXmlNamePool &namePool)
   : m_namePool(namePool), m_userMessageHandler(nullptr), m_uriResolver(nullptr), m_userNetworkAccessManager(nullptr),
     m_schemaContext(new QPatternist::XsdSchemaContext(m_namePool.d)),
     m_schemaParserContext(new QPatternist::XsdSchemaParserContext(m_namePool.d, m_schemaContext)),
     m_schemaIsValid(false)
{
   m_networkAccessManager = new QPatternist::ReferenceCountedValue<QNetworkAccessManager>(new QNetworkAccessManager());
   m_messageHandler = new QPatternist::ReferenceCountedValue<QAbstractMessageHandler>(new
         QPatternist::ColoringMessageHandler());
}

QXmlSchemaPrivate::QXmlSchemaPrivate(const QPatternist::XsdSchemaContext::Ptr &schemaContext)
   : m_namePool(QXmlNamePool(schemaContext->namePool().data())), m_userMessageHandler(nullptr),
     m_uriResolver(nullptr), m_userNetworkAccessManager(nullptr), m_schemaContext(schemaContext),
     m_schemaParserContext(new QPatternist::XsdSchemaParserContext(m_namePool.d, m_schemaContext)),
     m_schemaIsValid(false)
{
   m_networkAccessManager = new QPatternist::ReferenceCountedValue<QNetworkAccessManager>(new QNetworkAccessManager());
   m_messageHandler = new QPatternist::ReferenceCountedValue<QAbstractMessageHandler>(new
         QPatternist::ColoringMessageHandler());
}

QXmlSchemaPrivate::QXmlSchemaPrivate(const QXmlSchemaPrivate &other)
   : QSharedData(other)
{
   m_namePool = other.m_namePool;
   m_userMessageHandler = other.m_userMessageHandler;
   m_uriResolver = other.m_uriResolver;
   m_userNetworkAccessManager = other.m_userNetworkAccessManager;
   m_messageHandler = other.m_messageHandler;
   m_networkAccessManager = other.m_networkAccessManager;

   m_schemaContext = other.m_schemaContext;
   m_schemaParserContext = other.m_schemaParserContext;
   m_schemaIsValid = other.m_schemaIsValid;
   m_documentUri = other.m_documentUri;
}

void QXmlSchemaPrivate::load(const QUrl &source, const QString &targetNamespace)
{
   m_documentUri = QPatternist::XPathHelper::normalizeQueryURI(source);

   m_schemaContext->setMessageHandler(messageHandler());
   m_schemaContext->setUriResolver(uriResolver());
   m_schemaContext->setNetworkAccessManager(networkAccessManager());

   const std::unique_ptr<QNetworkReply> reply(QPatternist::AccelTreeResourceLoader::load(source,
         m_schemaContext->networkAccessManager(),
         m_schemaContext, QPatternist::AccelTreeResourceLoader::ContinueOnError));

   if (reply) {
      load(reply.get(), source, targetNamespace);
   }
}

void QXmlSchemaPrivate::load(const QByteArray &data, const QUrl &documentUri, const QString &targetNamespace)
{
   QByteArray localData(data);

   QBuffer buffer(&localData);
   buffer.open(QIODevice::ReadOnly);

   load(&buffer, documentUri, targetNamespace);
}

void QXmlSchemaPrivate::load(QIODevice *source, const QUrl &documentUri, const QString &targetNamespace)
{
   m_schemaParserContext = QPatternist::XsdSchemaParserContext::Ptr(new QPatternist::XsdSchemaParserContext(m_namePool.d,
                           m_schemaContext));
   m_schemaIsValid = false;

   if (!source) {
      qWarning("A null QIODevice pointer cannot be passed.");
      return;
   }

   if (!source->isReadable()) {
      qWarning("The device must be readable.");
      return;
   }

   m_documentUri = QPatternist::XPathHelper::normalizeQueryURI(documentUri);
   m_schemaContext->setMessageHandler(messageHandler());
   m_schemaContext->setUriResolver(uriResolver());
   m_schemaContext->setNetworkAccessManager(networkAccessManager());

   QPatternist::XsdSchemaParser parser(m_schemaContext, m_schemaParserContext, source);
   parser.setDocumentURI(documentUri);
   parser.setTargetNamespace(targetNamespace);

   try {
      parser.parse();
      m_schemaParserContext->resolver()->resolve();

      m_schemaIsValid = true;
   } catch (QPatternist::Exception exception) {
      (void) exception;
      m_schemaIsValid = false;
   }
}

bool QXmlSchemaPrivate::isValid() const
{
   return m_schemaIsValid;
}

QXmlNamePool QXmlSchemaPrivate::namePool() const
{
   return m_namePool;
}

QUrl QXmlSchemaPrivate::documentUri() const
{
   return m_documentUri;
}

void QXmlSchemaPrivate::setMessageHandler(QAbstractMessageHandler *handler)
{
   m_userMessageHandler = handler;
}

QAbstractMessageHandler *QXmlSchemaPrivate::messageHandler() const
{
   if (m_userMessageHandler) {
      return m_userMessageHandler;
   }

   return m_messageHandler.data()->value;
}

void QXmlSchemaPrivate::setUriResolver(const QAbstractUriResolver *resolver)
{
   m_uriResolver = resolver;
}

const QAbstractUriResolver *QXmlSchemaPrivate::uriResolver() const
{
   return m_uriResolver;
}

void QXmlSchemaPrivate::setNetworkAccessManager(QNetworkAccessManager *networkmanager)
{
   m_userNetworkAccessManager = networkmanager;
}

QNetworkAccessManager *QXmlSchemaPrivate::networkAccessManager() const
{
   if (m_userNetworkAccessManager) {
      return m_userNetworkAccessManager;
   }

   return m_networkAccessManager.data()->value;
}
