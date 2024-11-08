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

#ifndef QXsdSchemaContext_P_H
#define QXsdSchemaContext_P_H

#include <qnamedschemacomponent_p.h>
#include <qreportcontext_p.h>
#include <qschematypefactory_p.h>
#include <qxsdschematoken_p.h>
#include <qxsdschema_p.h>
#include <qxsdschemachecker_p.h>
#include <qxsdschemaresolver_p.h>
#include <qurl.h>
#include <qnetaccess_manager.h>
#include <qabstractmessagehandler.h>

namespace QPatternist {

class XsdSchemaContext : public ReportContext
{
 public:
   typedef QExplicitlySharedDataPointer<XsdSchemaContext> Ptr;

   XsdSchemaContext(const NamePool::Ptr &namePool);

   NamePool::Ptr namePool() const override;

   virtual void setBaseURI(const QUrl &uri);
   virtual QUrl baseURI() const;

   void setNetworkAccessManager(QNetworkAccessManager *accessManager);

   virtual QNetworkAccessManager *networkAccessManager() const;

   void setMessageHandler(QAbstractMessageHandler *handler);
   QAbstractMessageHandler *messageHandler() const override;

   QSourceLocation locationFor(const SourceLocationReflection *const reflection) const override;

   void setUriResolver(const QAbstractUriResolver *resolver);
   const QAbstractUriResolver *uriResolver() const override;

   XsdFacet::Hash facetsForType(const AnySimpleType::Ptr &type) const;

   SchemaTypeFactory::Ptr schemaTypeFactory() const;

   mutable SchemaTypeFactory::Ptr m_schemaTypeFactory;
   mutable QHash<SchemaType::Ptr, XsdFacet::Hash> m_builtinTypesFacetList;

 private:
   QHash<SchemaType::Ptr, XsdFacet::Hash> setupBuiltinTypesFacetList() const;

   NamePool::Ptr                                 m_namePool;
   QNetworkAccessManager                        *m_networkAccessManager;
   QUrl                                          m_baseURI;
   const QAbstractUriResolver                   *m_uriResolver;
   QAbstractMessageHandler                      *m_messageHandler;
};
}

#endif
