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

#ifndef QXMLSCHEMAVALIDATOR_P_H
#define QXMLSCHEMAVALIDATOR_P_H

#include "qabstractmessagehandler.h"
#include "qabstracturiresolver.h"
#include "qcoloringmessagehandler_p.h"
#include "qxmlschema.h"
#include "qxmlschema_p.h"
#include "qxsdschemacontext_p.h"
#include "qxsdschema_p.h"
#include <qnetaccess_manager.h>

class QXmlSchemaValidatorPrivate
{
 public:
   QXmlSchemaValidatorPrivate(const QXmlSchema &schema)
      : m_namePool(schema.namePool()), m_userMessageHandler(nullptr), m_uriResolver(nullptr),
        m_userNetworkAccessManager(nullptr)
   {
      setSchema(schema);

      const QXmlSchemaPrivate *p = schema.d;

      // initialize the environment properties with the ones from the schema

      if (p->m_userNetworkAccessManager) { // schema has user defined network access manager
         m_userNetworkAccessManager = p->m_userNetworkAccessManager;
      } else {
         m_networkAccessManager = p->m_networkAccessManager;
      }

      if (p->m_userMessageHandler) { // schema has user defined message handler
         m_userMessageHandler = p->m_userMessageHandler;
      } else {
         m_messageHandler = p->m_messageHandler;
      }

      m_uriResolver = p->m_uriResolver;
   }

   void setSchema(const QXmlSchema &schema) {
      // use same name pool as the schema
      m_namePool = schema.namePool();
      m_schema = schema.d->m_schemaParserContext->schema();
      m_schemaDocumentUri = schema.documentUri();

      // create a new schema context
      m_context = QPatternist::XsdSchemaContext::Ptr(new QPatternist::XsdSchemaContext(m_namePool.d));
      m_context->m_schemaTypeFactory = schema.d->m_schemaContext->m_schemaTypeFactory;
      m_context->m_builtinTypesFacetList = schema.d->m_schemaContext->m_builtinTypesFacetList;

      m_originalSchema = schema;
   }

   QXmlNamePool                                                     m_namePool;
   QAbstractMessageHandler                                         *m_userMessageHandler;
   const QAbstractUriResolver                                      *m_uriResolver;
   QNetworkAccessManager                                           *m_userNetworkAccessManager;
   QPatternist::ReferenceCountedValue<QAbstractMessageHandler>::Ptr m_messageHandler;
   QPatternist::ReferenceCountedValue<QNetworkAccessManager>::Ptr   m_networkAccessManager;

   QXmlSchema                                                       m_originalSchema;
   QPatternist::XsdSchemaContext::Ptr                               m_context;
   QPatternist::XsdSchema::Ptr                                      m_schema;
   QUrl                                                             m_schemaDocumentUri;
};

#endif
