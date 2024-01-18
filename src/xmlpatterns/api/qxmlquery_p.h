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

#ifndef QXMLQUERY_P_H
#define QXMLQUERY_P_H

#include <qabstractmessagehandler.h>
#include <qabstracturiresolver.h>
#include <qpointer.h>
#include <qsourcelocation.h>
#include <qurl.h>
#include <qvariant.h>
#include <qxmlname.h>
#include <qxmlnamepool.h>
#include <qxmlquery.h>

#include "qacceltreebuilder_p.h"
#include "qacceltreeresourceloader_p.h"
#include "qcoloringmessagehandler_p.h"
#include "qcommonsequencetypes_p.h"
#include "qexpressionfactory_p.h"
#include "qfocus_p.h"
#include "qfunctionfactorycollection_p.h"
#include "qgenericdynamiccontext_p.h"
#include "qgenericstaticcontext_p.h"
#include "qnamepool_p.h"
#include "qnetworkaccessdelegator_p.h"
#include "qreferencecountedvalue_p.h"
#include "qresourcedelegator_p.h"
#include "qstaticfocuscontext_p.h"
#include "quriloader_p.h"
#include "qvariableloader_p.h"

class QXmlQueryPrivate
{
 public:

   inline QXmlQueryPrivate(const QXmlNamePool &np = QXmlNamePool())
      : namePool(np), messageHandler(nullptr), uriResolver(nullptr), queryLanguage(QXmlQuery::XQuery10),
        m_networkAccessDelegator(new QPatternist::NetworkAccessDelegator(nullptr, nullptr))
   {
      m_networkAccessDelegator->m_variableURIManager = new QPatternist::URILoader(ownerObject(),
            namePool.d, variableLoader());
   }

   void detach() {
      if (m_variableLoader) {
         m_variableLoader = QPatternist::VariableLoader::Ptr(new QPatternist::VariableLoader(namePool.d, m_variableLoader));
      }

      delete m_networkAccessDelegator->m_variableURIManager;
      m_networkAccessDelegator->m_variableURIManager = new QPatternist::URILoader(ownerObject(), namePool.d, m_variableLoader);

      if (m_resourceLoader) {
         const QPatternist::AccelTreeResourceLoader::Ptr nev(new QPatternist::AccelTreeResourceLoader(namePool.d, m_networkAccessDelegator));

         m_resourceLoader = QPatternist::ResourceLoader::Ptr(new QPatternist::ResourceDelegator(m_resourceLoader->deviceURIs(),
               m_resourceLoader, nev));
      }
   }

   bool isValid() {
      return expression();
   }

   inline void recompileRequired() {
      m_expr.reset();
   }

   inline QPatternist::VariableLoader::Ptr variableLoader() {
      if (!m_variableLoader) {
         m_variableLoader = QPatternist::VariableLoader::Ptr(new QPatternist::VariableLoader(namePool.d));
      }

      return m_variableLoader;
   }

   inline QPatternist::GenericStaticContext::Ptr staticContext() {
      if (m_staticContext && m_expr) {
         return m_staticContext;
      }
      /* Else, re-create the staticContext. */

      if (!messageHandler) {
         messageHandler = new QPatternist::ColoringMessageHandler(ownerObject());
      }

      if (!m_functionFactory) {
         if (queryLanguage == QXmlQuery::XSLT20) {
            m_functionFactory = QPatternist::FunctionFactoryCollection::xslt20Factory(namePool.d);
         } else {
            m_functionFactory = QPatternist::FunctionFactoryCollection::xpath20Factory(namePool.d);
         }
      }

      const QPatternist::GenericStaticContext::Ptr genericStaticContext(new QPatternist::GenericStaticContext(namePool.d,
            messageHandler,
            queryURI,
            m_functionFactory,
            queryLanguage));
      genericStaticContext->setResourceLoader(resourceLoader());

      genericStaticContext->setExternalVariableLoader(variableLoader());

      m_staticContext = genericStaticContext;

      if (!contextItem.isNull()) {
         m_staticContext = QPatternist::StaticContext::Ptr(new QPatternist::StaticFocusContext(
                              QPatternist::AtomicValue::qtToXDMType(contextItem), m_staticContext));
      } else if (   queryLanguage == QXmlQuery::XmlSchema11IdentityConstraintField
                    || queryLanguage == QXmlQuery::XmlSchema11IdentityConstraintSelector
                    || queryLanguage == QXmlQuery::XPath20) {
         m_staticContext = QPatternist::StaticContext::Ptr(new QPatternist::StaticFocusContext(QPatternist::BuiltinTypes::node,
                           m_staticContext));
      }

      for (int i = 0; i < m_additionalNamespaceBindings.count(); ++i) {
         m_staticContext->namespaceBindings()->addBinding(m_additionalNamespaceBindings.at(i));
      }

      return m_staticContext;
   }

   inline QPatternist::DynamicContext::Ptr dynamicContext(QAbstractXmlReceiver *const callback = nullptr) {
      const QPatternist::StaticContext::Ptr statContext(staticContext());
      Q_ASSERT(statContext);

      QPatternist::GenericDynamicContext::Ptr dynContext(new QPatternist::GenericDynamicContext(namePool.d,
            statContext->messageHandler(),
            statContext->sourceLocations()));

      std::unique_ptr<QPatternist::NodeBuilder> nodeBuilder(new QPatternist::AccelTreeBuilder<false>(QUrl(), QUrl(),
            namePool.d, dynContext.data()));

      dynContext->setNodeBuilder(nodeBuilder);

      dynContext->setResourceLoader(statContext->resourceLoader());
      dynContext->setExternalVariableLoader(statContext->externalVariableLoader());
      dynContext->setUriResolver(uriResolver);

      if (callback) {
         dynContext->setOutputReceiver(callback);
      }

      if (contextItem.isNull()) {
         return dynContext;
      } else {
         QPatternist::DynamicContext::Ptr focus(new QPatternist::Focus(dynContext));
         QPatternist::Item::Iterator::Ptr it(QPatternist::makeSingletonIterator(QPatternist::Item::fromPublic(contextItem)));
         it->next();
         focus->setFocusIterator(it);
         return focus;
      }
   }

   inline QPatternist::AccelTreeResourceLoader::Ptr resourceLoader() {
      if (!m_resourceLoader) {
         m_resourceLoader = (new QPatternist::AccelTreeResourceLoader(namePool.d, m_networkAccessDelegator));
      }

      return m_resourceLoader;
   }

   void setRequiredType(const QPatternist::SequenceType::Ptr &seqType) {
      Q_ASSERT(seqType);
      if (!m_requiredType || m_requiredType->is(seqType)) {
         return;
      }

      m_requiredType = seqType;
      m_staticContext.reset();
   }

   QPatternist::SequenceType::Ptr requiredType() {
      if (m_requiredType) {
         return m_requiredType;
      } else {
         m_requiredType = QPatternist::CommonSequenceTypes::ZeroOrMoreItems;
         return m_requiredType;
      }
   }

   QPatternist::Expression::Ptr expression(QIODevice *const queryDevice = nullptr) {
      if (m_expr && !queryDevice) {
         return m_expr;
      }

      /* If we need to update, but we don't have any source code, we can
       * never create an Expression. */
      if (!queryDevice) {
         return QPatternist::Expression::Ptr();
      }

      try {
         /* The static context has source locations, and they need to be
          * updated to the new query. */
         m_staticContext.reset();

         if (!m_expressionFactory) {
            m_expressionFactory = QPatternist::ExpressionFactory::Ptr(new QPatternist::ExpressionFactory());
         }

         m_expr = m_expressionFactory->createExpression(queryDevice, staticContext(),
                  queryLanguage,
                  requiredType(),
                  queryURI,
                  initialTemplateName);
      } catch (const QPatternist::Exception) {
         m_expr.reset();

         /* We don't call m_staticContext.reset() because it shouldn't be
          * necessary, since m_staticContext is changed when the expression
          * is changed. */
      }

      return m_expr;
   }

   inline void addAdditionalNamespaceBinding(const QXmlName &binding) {
      m_additionalNamespaceBindings.append(binding);
   }

   QXmlNamePool                                namePool;
   QPointer<QAbstractMessageHandler>           messageHandler;
   /**
    * Must be absolute and valid.
    */
   QUrl                                        queryURI;
   const QAbstractUriResolver                 *uriResolver;
   QXmlItem                                    contextItem;
   QXmlName                                    initialTemplateName;

   inline void setExpressionFactory(const QPatternist::ExpressionFactory::Ptr &expr) {
      m_expressionFactory = expr;
   }

   QXmlQuery::QueryLanguage                    queryLanguage;
   QPointer<QNetworkAccessManager>             userNetworkManager;

   inline QObject *ownerObject() {
      if (!m_owner) {
         m_owner = new QPatternist::ReferenceCountedValue<QObject>(new QObject());
      }

      return m_owner->value;
   }

   QPatternist::ExpressionFactory::Ptr         m_expressionFactory;
   QPatternist::StaticContext::Ptr             m_staticContext;
   QPatternist::VariableLoader::Ptr            m_variableLoader;
   QPatternist::DeviceResourceLoader::Ptr      m_resourceLoader;
   /**
    * This is the AST for the query.
    */
   QPatternist::Expression::Ptr                m_expr;
   QPatternist::ReferenceCountedValue<QObject>::Ptr m_owner;

   /**
    * This is our effective network manager, that we end up using. The one the
    * user sets is userNetworkManager.
    */
   QPatternist::SequenceType::Ptr              m_requiredType;
   QPatternist::FunctionFactory::Ptr           m_functionFactory;
   QPatternist::NetworkAccessDelegator::Ptr    m_networkAccessDelegator;

   QList<QXmlName>                             m_additionalNamespaceBindings;
};

#endif
