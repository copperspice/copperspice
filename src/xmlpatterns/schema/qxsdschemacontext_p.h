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
   /**
    * A smart pointer wrapping XsdSchemaContext instances.
    */
   typedef QExplicitlySharedDataPointer<XsdSchemaContext> Ptr;

   /**
    * Creates a new schema context object.
    *
    * @param namePool The name pool all names belong to.
    */
   XsdSchemaContext(const NamePool::Ptr &namePool);

   /**
    * Returns the name pool of the schema context.
    */
   NamePool::Ptr namePool() const override;

   /**
    * Sets the base URI for the main schema.
    *
    * The main schema is the one that includes resp. imports
    * all the other schema files.
    */
   virtual void setBaseURI(const QUrl &uri);

   /**
    * Returns the base URI of the main schema.
    */
   virtual QUrl baseURI() const;

   /**
    * Sets the network access manager that should be used
    * to access referenced schema definitions.
    */
   void setNetworkAccessManager(QNetworkAccessManager *accessManager);

   /**
    * Returns the network access manager that is used to
    * access referenced schema definitions.
    */
   virtual QNetworkAccessManager *networkAccessManager() const;

   /**
    * Sets the message @p handler used by the context for error reporting.
    */
   void setMessageHandler(QAbstractMessageHandler *handler);

   /**
    * Returns the message handler used by the context for
    * error reporting.
    */
   QAbstractMessageHandler *messageHandler() const override;

   /**
    * Always returns an empty source location.
    */
   QSourceLocation locationFor(const SourceLocationReflection *const reflection) const override;

   /**
    * Sets the uri @p resolver that is used for resolving URIs in the
    * schema parser.
    */
   void setUriResolver(const QAbstractUriResolver *resolver);

   /**
    * Returns the uri resolver that is used for resolving URIs in the
    * schema parser.
    */
   const QAbstractUriResolver *uriResolver() const override;

   /**
    * Returns the list of facets for the given simple @p type.
    */
   XsdFacet::Hash facetsForType(const AnySimpleType::Ptr &type) const;

   /**
    * Returns a schema type factory that contains some predefined schema types.
    */
   SchemaTypeFactory::Ptr schemaTypeFactory() const;

   /**
    * The following variables should not be accessed directly.
    */
   mutable SchemaTypeFactory::Ptr                 m_schemaTypeFactory;
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
