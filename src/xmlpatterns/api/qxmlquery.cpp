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

#include <qxmlquery.h>
#include <qxmlquery_p.h>

#include <qbuffer.h>
#include <qstringlist.h>
#include <qxmlformatter.h>

#include <qacceltreeresourceloader_p.h>
#include <qcommonvalues_p.h>
#include <qxmlresultitems.h>
#include <qxmlresultitems_p.h>
#include <qxmlserializer.h>
#include <qxpathhelper_p.h>

// ### Merge constructor overloads

QXmlQuery::QXmlQuery()
   : d(new QXmlQueryPrivate())
{
}

QXmlQuery::QXmlQuery(const QXmlQuery &other) : d(new QXmlQueryPrivate(*other.d))
{
   /* First we have invoked QXmlQueryPrivate's synthesized copy constructor.
    * Keep this section in sync with QXmlQuery::operator=(). */
   d->detach();
}

QXmlQuery::QXmlQuery(const QXmlNamePool &np) : d(new QXmlQueryPrivate(np))
{
}

QXmlQuery::QXmlQuery(QueryLanguage queryLanguage, const QXmlNamePool &np) : d(new QXmlQueryPrivate(np))
{
   d->queryLanguage = queryLanguage;
}

QXmlQuery::~QXmlQuery()
{
   delete d;
}

QXmlQuery &QXmlQuery::operator=(const QXmlQuery &other)
{
   // Keep this section in sync with QXmlQuery::QXmlQuery(const QXmlQuery &).

   if (d != other.d) {
      *d = *other.d;
      d->detach();
   }

   return *this;
}

void QXmlQuery::setMessageHandler(QAbstractMessageHandler *aMessageHandler)
{
   d->messageHandler = aMessageHandler;
}

QAbstractMessageHandler *QXmlQuery::messageHandler() const
{
   return d->messageHandler;
}

void QXmlQuery::setQuery(QIODevice *sourceCode, const QUrl &documentURI)
{
   if (!sourceCode) {
      qWarning("A null QIODevice pointer cannot be passed.");
      return;
   }

   if (!sourceCode->isReadable()) {
      qWarning("The device must be readable.");
      return;
   }

   d->queryURI = QPatternist::XPathHelper::normalizeQueryURI(documentURI);
   d->expression(sourceCode);
}

void QXmlQuery::setQuery(const QString &sourceCode, const QUrl &documentURI)
{
   Q_ASSERT_X(documentURI.isEmpty() || documentURI.isValid(), Q_FUNC_INFO,
              "The document URI must be valid.");

   QByteArray query(sourceCode.toUtf8());
   QBuffer buffer(&query);
   buffer.open(QIODevice::ReadOnly);

   setQuery(&buffer, documentURI);
}

void QXmlQuery::setQuery(const QUrl &queryURI, const QUrl &baseURI)
{
   Q_ASSERT_X(queryURI.isValid(), Q_FUNC_INFO, "The passed URI must be valid.");

   const QUrl canonicalURI(QPatternist::XPathHelper::normalizeQueryURI(queryURI));
   Q_ASSERT(canonicalURI.isValid());
   Q_ASSERT(!canonicalURI.isRelative());
   Q_ASSERT(baseURI.isValid() || baseURI.isEmpty());

   d->queryURI = QPatternist::XPathHelper::normalizeQueryURI(baseURI.isEmpty() ? queryURI : baseURI);

   std::unique_ptr<QIODevice> result;

   try {
      result.reset(QPatternist::AccelTreeResourceLoader::load(canonicalURI, d->m_networkAccessDelegator,
                   d->staticContext()));

   } catch (const QPatternist::Exception) {
      /* We do nothing, result will be 0. */
   }

   if (result) {
      setQuery(result.get(), d->queryURI);
      result->close();

   } else {
      d->recompileRequired();
   }
}

void QXmlQuery::bindVariable(const QXmlName &name, const QXmlItem &value)
{
   if (name.isNull()) {
      qWarning("The variable name cannot be null.");
      return;
   }

   const QPatternist::VariableLoader::Ptr vl(d->variableLoader());
   const QVariant variant(QVariant::fromValue(value));

   /* If the type of the variable changed(as opposed to only the value),
    * we will have to recompile. */
   if (vl->invalidationRequired(name, variant) || value.isNull()) {
      d->recompileRequired();
   }

   vl->addBinding(name, variant);
}

void QXmlQuery::bindVariable(const QString &localName, const QXmlItem &value)
{
   bindVariable(QXmlName(d->namePool, localName), value);
}

void QXmlQuery::bindVariable(const QXmlName &name, QIODevice *device)
{
   if (device && ! device->isReadable()) {
      qWarning("A null, or readable QIODevice must be passed.");
      return;
   }

   if (name.isNull()) {
      qWarning("The variable name can not be null.");
      return;
   }

   const QPatternist::VariableLoader::Ptr vl(d->variableLoader());

   if (device) {
      const QVariant variant(QVariant::fromValue(device));

      if (vl->invalidationRequired(name, variant)) {
         d->recompileRequired();
      }

      vl->addBinding(name, variant);

      /* We need to tell the resource loader to discard its document, because
       * the underlying QIODevice has changed, but the variable name is the
       * same which means that the URI is the same, and hence the resource
       * loader will return the document for the old QIODevice.
       */

      d->resourceLoader()->clear(QUrl("tag:copperspice.com,2007:QtXmlPatterns:QIODeviceVariable:" +
                                      d->namePool.d->stringForLocalName(name.localName())));

   } else {
      vl->removeBinding(name);
      d->recompileRequired();
   }
}

void QXmlQuery::bindVariable(const QString &localName, QIODevice *device)
{
   bindVariable(QXmlName(d->namePool, localName), device);
}

bool QXmlQuery::evaluateTo(QAbstractXmlReceiver *callback) const
{
   if (!callback) {
      qWarning("A non-null callback must be passed.");
      return false;
   }

   if (isValid()) {
      try {
         /*
          * This order is significant. expression() might cause
          * query recompilation, and as part of that it recreates
          * the static context. However, if we create the dynamic
          * context before the query recompilation has been
          * triggered, it will use the old static context, and
          * hence old source locations.
          */
         const QPatternist::Expression::Ptr expr(d->expression());
         const QPatternist::DynamicContext::Ptr dynContext(d->dynamicContext(callback));

         callback->startOfSequence();
         expr->evaluateToSequenceReceiver(dynContext);

         callback->endOfSequence();
         return true;

      } catch (const QPatternist::Exception) {
         return false;
      }

   } else {
      return false;
   }
}

bool QXmlQuery::evaluateTo(QStringList *target) const
{
   if (!target) {
      qWarning("A non-null callback must be passed.");
      return false;
   }

   if (isValid()) {
      try {
         /*
          * This order is significant. expression() might cause
          * query recompilation, and as part of that it recreates
          * the static context. However, if we create the dynamic
          * context before the query recompilation has been
          * triggered, it will use the old static context, and
          * hence old source locations.
          */
         const QPatternist::Expression::Ptr expr(d->expression());
         if (!expr) {
            return false;
         }

         QPatternist::DynamicContext::Ptr dynContext(d->dynamicContext());

         if (!QPatternist::BuiltinTypes::xsString->xdtTypeMatches(expr->staticType()->itemType())) {
            return false;
         }

         const QPatternist::Item::Iterator::Ptr it(expr->evaluateSequence(dynContext));
         QPatternist::Item next(it->next());

         while (!next.isNull()) {
            target->append(next.stringValue());
            next = it->next();
         }

         return true;
      } catch (const QPatternist::Exception) {
         return false;
      }
   } else {
      return false;
   }
}

bool QXmlQuery::evaluateTo(QIODevice *target) const
{
   if (!target) {
      qWarning("The pointer to the device cannot be null.");
      return false;
   }

   if (!target->isWritable()) {
      qWarning("The device must be writable.");
      return false;
   }

   QXmlSerializer serializer(*this, target);
   return evaluateTo(&serializer);
}

void QXmlQuery::evaluateTo(QXmlResultItems *result) const
{
   if (!result) {
      qWarning("A null pointer cannot be passed.");
      return;
   }

   if (isValid()) {
      try {
         /*
          * We don't have the d->expression() calls and
          * d->dynamicContext() calls in the same order as seen in
          * QXmlQuery::evaluateTo(), and the reason to why
          * that isn't a problem, is that we call isValid().
          */
         const QPatternist::DynamicContext::Ptr dynContext(d->dynamicContext());
         result->d_ptr->setDynamicContext(dynContext);
         result->d_ptr->iterator = d->expression()->evaluateSequence(dynContext);
      } catch (const QPatternist::Exception) {
         result->d_ptr->iterator = QPatternist::CommonValues::emptyIterator;
         result->d_ptr->hasError = true;
      }
   } else {
      result->d_ptr->iterator = QPatternist::CommonValues::emptyIterator;
      result->d_ptr->hasError = true;
   }
}

bool QXmlQuery::evaluateTo(QString *output) const
{
   Q_ASSERT_X(output, Q_FUNC_INFO,
              "The input cannot be null");

   QBuffer outputDevice;
   outputDevice.open(QIODevice::ReadWrite);

   QXmlFormatter formatter(*this, &outputDevice);
   const bool success = evaluateTo(&formatter);

   outputDevice.close();
   *output = QString::fromUtf8(outputDevice.data().constData());

   return success;
}

bool QXmlQuery::isValid() const
{
   return d->isValid();
}

void QXmlQuery::setUriResolver(const QAbstractUriResolver *resolver)
{
   d->uriResolver = resolver;
}

const QAbstractUriResolver *QXmlQuery::uriResolver() const
{
   return d->uriResolver;
}

QXmlNamePool QXmlQuery::namePool() const
{
   return d->namePool;
}

void QXmlQuery::setFocus(const QXmlItem &item)
{
   d->contextItem = item;
}

// function should be a private member function of QXmlQuery
template<typename TInputType>
bool setFocusHelper(QXmlQuery *const queryInstance, const TInputType &focusValue)
{
   /* We call resourceLoader(), so we have ensured that we have a resourceLoader
    * that we will share in our copy. */
   queryInstance->d->resourceLoader();

   QXmlQuery focusQuery(*queryInstance);

   /* Now we use the same, so we own the loaded document. */
   focusQuery.d->m_resourceLoader = queryInstance->d->m_resourceLoader;

   /* The copy constructor doesn't allow us to copy an existing QXmlQuery and
    * changing the language at the same time so we need to use private API. */
   focusQuery.d->queryLanguage = QXmlQuery::XQuery10;

   Q_ASSERT(focusQuery.queryLanguage() == QXmlQuery::XQuery10);
   focusQuery.bindVariable(QChar::fromLatin1('u'), focusValue);
   focusQuery.setQuery(QLatin1String("doc($u)"));
   Q_ASSERT(focusQuery.isValid());

   QXmlResultItems focusResult;

   queryInstance->d->m_resourceLoader = focusQuery.d->m_resourceLoader;

   focusQuery.evaluateTo(&focusResult);
   const QXmlItem focusItem(focusResult.next());

   if (focusItem.isNull() || focusResult.hasError()) {
      /* The previous focus must be cleared in error situations.
       * Otherwise the query may be left in an inconsistent state. */
      queryInstance->setFocus(QXmlItem());
      return false;
   } else {
      queryInstance->setFocus(focusItem);
      return true;
   }
}

bool QXmlQuery::setFocus(const QUrl &documentURI)
{
   Q_ASSERT_X(documentURI.isValid() && !documentURI.isEmpty(),
              Q_FUNC_INFO,
              "The URI passed must be valid.");

   return setFocusHelper(this, QVariant(documentURI));
}

bool QXmlQuery::setFocus(QIODevice *document)
{
   if (!document) {
      qWarning("A null QIODevice pointer cannot be passed.");
      return false;
   }

   if (!document->isReadable()) {
      qWarning("The device must be readable.");
      return false;
   }

   return setFocusHelper(this, document);
}

bool QXmlQuery::setFocus(const QString &focus)
{
   QBuffer device;
   device.setData(focus.toUtf8());
   device.open(QIODevice::ReadOnly);

   return setFocusHelper(this, &device);
}

QXmlQuery::QueryLanguage QXmlQuery::queryLanguage() const
{
   return d->queryLanguage;
}

void QXmlQuery::setInitialTemplateName(const QXmlName &name)
{
   d->initialTemplateName = name;
}

void QXmlQuery::setInitialTemplateName(const QString &localName)
{
   Q_ASSERT_X(QXmlName::isNCName(localName),
              Q_FUNC_INFO,
              "The name passed must be a valid NCName.");
   setInitialTemplateName(QXmlName(d->namePool, localName));
}

QXmlName QXmlQuery::initialTemplateName() const
{
   return d->initialTemplateName;
}

void QXmlQuery::setNetworkAccessManager(QNetworkAccessManager *newManager)
{
   d->m_networkAccessDelegator->m_genericManager = newManager;
}

QNetworkAccessManager *QXmlQuery::networkAccessManager() const
{
   return d->m_networkAccessDelegator->m_genericManager;
}

void QXmlQuery::bindVariable(const QXmlName &name, const QXmlQuery &query)
{
   Q_ASSERT_X(query.isValid(), Q_FUNC_INFO, "The query being bound must be valid.");

   const QPatternist::VariableLoader::Ptr vl(d->variableLoader());
   const QVariant variant(QVariant::fromValue(query));

   if (vl->invalidationRequired(name, variant)) {
      d->recompileRequired();
   }

   vl->addBinding(name, variant);
}

void QXmlQuery::bindVariable(const QString &localName, const QXmlQuery &query)
{
   return bindVariable(QXmlName(d->namePool, localName), query);
}
