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

#ifndef QXMLSCHEMA_P_H
#define QXMLSCHEMA_P_H

#include <qabstractmessagehandler.h>
#include <qabstracturiresolver.h>
#include <qshareddata.h>
#include <qnetaccess_manager.h>

#include <qcoloringmessagehandler_p.h>
#include <qreferencecountedvalue_p.h>
#include <qxsdschemacontext_p.h>
#include <qxsdschemaparser_p.h>
#include <qxsdschemaparsercontext_p.h>

class QXmlSchemaPrivate : public QSharedData
{
 public:
   QXmlSchemaPrivate(const QXmlNamePool &namePool);
   QXmlSchemaPrivate(const QPatternist::XsdSchemaContext::Ptr &schemaContext);
   QXmlSchemaPrivate(const QXmlSchemaPrivate &other);

   void load(const QUrl &source, const QString &targetNamespace);
   void load(QIODevice *source, const QUrl &documentUri, const QString &targetNamespace);
   void load(const QByteArray &data, const QUrl &documentUri, const QString &targetNamespace);
   bool isValid() const;
   QXmlNamePool namePool() const;
   QUrl documentUri() const;
   void setMessageHandler(QAbstractMessageHandler *handler);
   QAbstractMessageHandler *messageHandler() const;
   void setUriResolver(const QAbstractUriResolver *resolver);
   const QAbstractUriResolver *uriResolver() const;
   void setNetworkAccessManager(QNetworkAccessManager *networkmanager);
   QNetworkAccessManager *networkAccessManager() const;

   QXmlNamePool                                                     m_namePool;
   QAbstractMessageHandler                                         *m_userMessageHandler;
   const QAbstractUriResolver                                      *m_uriResolver;
   QNetworkAccessManager                                           *m_userNetworkAccessManager;
   QPatternist::ReferenceCountedValue<QAbstractMessageHandler>::Ptr m_messageHandler;
   QPatternist::ReferenceCountedValue<QNetworkAccessManager>::Ptr   m_networkAccessManager;

   QPatternist::XsdSchemaContext::Ptr                               m_schemaContext;
   QPatternist::XsdSchemaParserContext::Ptr                         m_schemaParserContext;
   bool                                                             m_schemaIsValid;
   QUrl                                                             m_documentUri;
};

#endif
