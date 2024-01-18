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

#ifndef QXMLQUERY_H
#define QXMLQUERY_H

#include <qurl.h>
#include <qabstractxmlnodemodel.h>
#include <qabstractxmlreceiver.h>
#include <qxmlnamepool.h>

class QAbstractMessageHandler;
class QAbstractUriResolver;
class QIODevice;
class QNetworkAccessManager;
class QXmlName;
class QXmlNodeIndex;
class QXmlQueryPrivate;
class QXmlResultItems;
class QXmlSerializer;

/* The members in the namespace QPatternistSDK are internal, not part of the public API, and
 * unsupported. Using them leads to undefined behavior. */
namespace QPatternistSDK {
class TestCase;
}

namespace QPatternist {
class XsdSchemaParser;
class XsdValidatingInstanceReader;
class VariableLoader;
}

class Q_XMLPATTERNS_EXPORT QXmlQuery
{
 public:
   enum QueryLanguage {
      XQuery10                                = 1,
      XSLT20                                  = 2,
      XmlSchema11IdentityConstraintSelector   = 1024,
      XmlSchema11IdentityConstraintField      = 2048,
      XPath20                                 = 4096
   };

   QXmlQuery();
   QXmlQuery(const QXmlQuery &other);
   QXmlQuery(const QXmlNamePool &np);
   QXmlQuery(QueryLanguage queryLanguage,
             const QXmlNamePool &np = QXmlNamePool());
   ~QXmlQuery();
   QXmlQuery &operator=(const QXmlQuery &other);

   void setMessageHandler(QAbstractMessageHandler *msgHandler);
   QAbstractMessageHandler *messageHandler() const;

   void setQuery(const QString &sourceCode, const QUrl &documentURI = QUrl());
   void setQuery(QIODevice *sourceCode, const QUrl &documentURI = QUrl());
   void setQuery(const QUrl &queryURI, const QUrl &baseURI = QUrl());

   QXmlNamePool namePool() const;

   void bindVariable(const QXmlName &name, const QXmlItem &value);
   void bindVariable(const QString &localName, const QXmlItem &value);

   void bindVariable(const QXmlName &name, QIODevice *device);
   void bindVariable(const QString &localName, QIODevice *device);
   void bindVariable(const QXmlName &name, const QXmlQuery &query);
   void bindVariable(const QString &localName, const QXmlQuery &query);

   bool isValid() const;

   void evaluateTo(QXmlResultItems *result) const;
   bool evaluateTo(QAbstractXmlReceiver *callback) const;
   bool evaluateTo(QStringList *target) const;
   bool evaluateTo(QIODevice *target) const;
   bool evaluateTo(QString *output) const;

   void setUriResolver(const QAbstractUriResolver *resolver);
   const QAbstractUriResolver *uriResolver() const;

   void setFocus(const QXmlItem &item);
   bool setFocus(const QUrl &documentURI);
   bool setFocus(QIODevice *document);
   bool setFocus(const QString &focusText);

   void setInitialTemplateName(const QXmlName &name);
   void setInitialTemplateName(const QString &name);
   QXmlName initialTemplateName() const;

   void setNetworkAccessManager(QNetworkAccessManager *newManager);
   QNetworkAccessManager *networkAccessManager() const;

   QueryLanguage queryLanguage() const;

 private:
   friend class QXmlName;
   friend class QXmlSerializer;
   friend class QPatternistSDK::TestCase;
   friend class QPatternist::XsdSchemaParser;
   friend class QPatternist::XsdValidatingInstanceReader;
   friend class QPatternist::VariableLoader;

   template<typename TInputType> friend bool setFocusHelper(QXmlQuery *const queryInstance, const TInputType &focusValue);
   QXmlQueryPrivate *d;
};

#endif
