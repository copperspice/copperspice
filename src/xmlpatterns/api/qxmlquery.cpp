/***********************************************************************
*
* Copyright (c) 2012-2023 Barbara Geller
* Copyright (c) 2012-2023 Ansel Sermersheim
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

#include <qbuffer.h>
#include <qstringlist.h>
#include <qxmlformatter.h>

#include "qacceltreeresourceloader_p.h"
#include "qcommonvalues_p.h"
#include "qxmlresultitems.h"
#include "qxmlresultitems_p.h"
#include "qxmlserializer.h"
#include "qxpathhelper_p.h"

#include "qxmlquery.h"
#include "qxmlquery_p.h"

// ### Qt5/Merge constructor overloads

QXmlQuery::QXmlQuery() : d(new QXmlQueryPrivate())
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
   /* Keep this section in sync with QXmlQuery::QXmlQuery(const QXmlQuery &).
    */
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

/*!
    Returns the message handler that handles compile and runtime
    messages for this QXmlQuery.
 */
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

/*!
  \overload
  The behavior and requirements of this function are the same as for
  setQuery(QIODevice*, const QUrl&), after the XQuery has been read
  from the IO device into a string. Because \a sourceCode is already
  a Unicode string, detection of its encoding is unnecessary.
*/
void QXmlQuery::setQuery(const QString &sourceCode, const QUrl &documentURI)
{
   Q_ASSERT_X(documentURI.isEmpty() || documentURI.isValid(), Q_FUNC_INFO,
              "The document URI must be valid.");

   QByteArray query(sourceCode.toUtf8());
   QBuffer buffer(&query);
   buffer.open(QIODevice::ReadOnly);

   setQuery(&buffer, documentURI);
}

/*!
  Sets this QXmlQuery to the XQuery read from the \a queryURI.  Use
  isValid() after calling this function. If an error occurred reading
  \a queryURI, e.g., the query does not exist, cannot be read, or is
  invalid, isValid() will return \e false.

  The supported URI schemes are the same as those in the XQuery
  function \c{fn:doc}, except that queryURI can be the object of
  a variable binding.

  \a baseURI is the Base URI of the static context, as defined in the
  \l {http://www.w3.org/TR/xquery/}{XQuery language}. It is used
  internally to resolve relative URIs that appear in the query, and
  for message reporting. If \a baseURI is empty, \a queryURI is used.
  Otherwise, \a baseURI is used, and it is resolved against the \l
  {QCoreApplication::applicationFilePath()} {application file path} if
  it is relative.

  If \a queryURI is empty or invalid, or if \a baseURI is invalid,
  the behavior of this function is undefined.
 */
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

/*!
  Binds the variable \a name to the \a value so that $\a name can be
  used from within the query to refer to the \a value.

  \a name must not be \e null. \a {name}.isNull() must return false.
  If \a name has already been bound by a previous bindVariable() call,
  its previous binding will be overridden.

  If \a {value} is null so that \a {value}.isNull() returns true, and
  \a {name} already has a binding, the effect is to remove the
  existing binding for \a {name}.

  To bind a value of type QString or QUrl, wrap the value in a
  QVariant such that QXmlItem's QVariant constructor is called.

  All strings processed by the query must be valid XQuery strings,
  which means they must contain only XML 1.0 characters. However,
  this requirement is not checked. If the query processes an invalid
  string, the behavior is undefined.

  \sa QVariant::isValid(), {QtXDM}{How QVariant maps to XQuery's Data Model},
   QXmlItem::isNull()
 */
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

/*!
  Attempts to evaluate the query and returns the results in the
  \a target \l {QStringList} {string list}.

  If the query \l {isValid()} {is valid} and the evaluation succeeds,
  true is returned. Otherwise, false is returned and the contents of
  \a target are undefined.

  The query must evaluate to a sequence of \c{xs:string} values. If
  the query does not evaluate to a sequence of strings, the values can
  often be converted by adding a call to \c{string()} at the end of
  the XQuery.

  If \a target is null, the behavior is undefined.
 */
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

/*!
  Evaluates the query or stylesheet, and writes the output to \a target.

  QXmlSerializer is used to write the output to \a target. In a future
  release, it is expected that this function will be changed to
  respect serialization options set in the stylesheet.

  If an error occurs during the evaluation, error messages are sent to
  messageHandler() and \c false is returned.

  If \a target is \c null, or is not opened in at least
  QIODevice::WriteOnly mode, the behavior is undefined.  QXmlQuery
  does not take ownership of \a target.

  \since 4.5
  \overload
 */
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

/*!
  Starts the evaluation and makes it available in \a result.  If \a
  result is null, the behavior is undefined. The evaluation takes
  place incrementally (lazy evaluation), as the caller uses
  QXmlResultItems::next() to get the next result.

  \sa QXmlResultItems::next()
*/
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

/*!
  Evaluates the query, and serializes the output as XML to \a output.

  If an error occurs during the evaluation, error messages are sent to
  messageHandler(), the content of \a output is undefined and \c false is
  returned, otherwise \c true is returned.

  If \a output is \c null behavior is undefined. QXmlQuery
  does not take ownership of \a output.

  Internally, the class QXmlFormatter is used for this.
 \since 4.5
 */
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

/*!
  Returns true if this query is valid. Examples of invalid queries
  are ones that contain syntax errors or that have not had setQuery()
  called for them yet.
 */
bool QXmlQuery::isValid() const
{
   return d->isValid();
}

/*!
  Sets the URI resolver to \a resolver. QXmlQuery does not take
  ownership of \a resolver.

  \sa uriResolver()
 */
void QXmlQuery::setUriResolver(const QAbstractUriResolver *resolver)
{
   d->uriResolver = resolver;
}

/*!
  Returns the query's URI resolver. If no URI resolver has been set,
  QtXmlPatterns will use the URIs in queries as they are.

  The URI resolver provides a level of abstraction, or \e{polymorphic
  URIs}. A resolver can rewrite \e{logical} URIs to physical ones, or
  it can translate obsolete or invalid URIs to valid ones.

  QtXmlPatterns calls the URI resolver for all URIs it encounters,
  except for namespaces. Specifically, all builtin functions that deal
  with URIs (\c{fn:doc()}, and \c{fn:doc-available()}).

  In the case of \c{fn:doc()}, the absolute URI is the base URI in the
  static context (which most likely is the location of the query).
  Rather than use the URI the user specified, the return value of
  QAbstractUriResolver::resolve() will be used.

  When QtXmlPatterns calls QAbstractUriResolver::resolve() the
  absolute URI is the URI mandated by the XQuery language, and the
  relative URI is the URI specified by the user.

  \sa setUriResolver()
 */
const QAbstractUriResolver *QXmlQuery::uriResolver() const
{
   return d->uriResolver;
}

/*!
  Returns the name pool used by this QXmlQuery for constructing \l
  {QXmlName} {names}. There is no setter for the name pool, because
  mixing name pools causes errors due to name confusion.
 */
QXmlNamePool QXmlQuery::namePool() const
{
   return d->namePool;
}

/*!
  Sets the focus to \a item. The focus is the set of items that the
  context item expression and path expressions navigate from. For
  example, in the expression \e p/span, the element that \e p
  evaluates to is the focus for the following expression, \e span.

  The focus can be accessed using the context item expression, i.e.,
  dot (".").

  By default, the focus is not set and is undefined. It will
  therefore result in a dynamic error, \c XPDY0002, if the focus
  is attempted to be accessed. The focus must be set before the
  query is set with setQuery().

  There is no behavior defined for setting an item which is null.

 */
void QXmlQuery::setFocus(const QXmlItem &item)
{
   d->contextItem = item;
}

/**
 * This function should be a private member function of QXmlQuery,
 * but we don't dare that due to our weird compilers.
 * @internal
 * @relates QXmlQuery
 */
template<typename TInputType>
bool setFocusHelper(QXmlQuery *const queryInstance,
                    const TInputType &focusValue)
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

/*!
  \since 4.5
  \overload

  Sets the focus to be the document located at \a documentURI and
  returns true. If \a documentURI cannot be loaded, false is returned.
  It is undefined at what time the document may be loaded. When
  loading the document, the message handler and URI resolver set on
  this QXmlQuery are used.

  If \a documentURI is empty or is not a valid URI, the behavior of
  this function is undefined.
 */
bool QXmlQuery::setFocus(const QUrl &documentURI)
{
   Q_ASSERT_X(documentURI.isValid() && !documentURI.isEmpty(),
              Q_FUNC_INFO,
              "The URI passed must be valid.");

   return setFocusHelper(this, QVariant(documentURI));
}

/*!

  Sets the focus to be the \a document read from the QIODevice and
  returns true. If \a document cannot be loaded, false is returned.

  QXmlQuery does not take ownership of \a document. The user
  guarantees that a document is available from the \a document device
  and that the document is not empty. The device must be opened in at
  least read-only mode. \a document must stay in scope as long as the
  current query is active.

 \since 4.5
 \overload
 */
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

/*!
  This function behaves identically to calling the setFocus() overload with a
  QIODevice whose content is \a focus encoded as UTF-8. That is, \a focus is
  treated as if it contained an XML document.

  Returns the same result as the overload.

  \overload
  \since 4.6
 */
bool QXmlQuery::setFocus(const QString &focus)
{
   QBuffer device;
   device.setData(focus.toUtf8());
   device.open(QIODevice::ReadOnly);

   return setFocusHelper(this, &device);
}

/*!
  Returns a value indicating what this QXmlQuery is being used for.
  The default is QXmlQuery::XQuery10, which means the QXmlQuery is
  being used for running XQuery and XPath queries. QXmlQuery::XSLT20
  can also be returned, which indicates the QXmlQuery is for running
  XSL-T spreadsheets.

 \since 4.5
 */
QXmlQuery::QueryLanguage QXmlQuery::queryLanguage() const
{
   return d->queryLanguage;
}

/*!
  Sets the \a name of the initial template. The initial template is
  the one the processor calls first, instead of attempting to match a
  template to the context node (if any). If an initial template is not
  set, the standard order of template invocation will be used.

  This function only applies when using QXmlQuery to process XSL-T
  stylesheets. The name becomes part of the compiled stylesheet.
  Therefore, this function must be called before calling setQuery().

  If the stylesheet has no template named \a name, the processor will
  use the standard order of template invocation.

  \since 4.5
  \sa initialTemplateName()
 */
void QXmlQuery::setInitialTemplateName(const QXmlName &name)
{
   d->initialTemplateName = name;
}

/*!
  \overload

  Sets the name of the initial template to \a localName, which must be
  a valid \l{QXmlName::localName()} {local name}. The initial template
  is the one the processor calls first, instead of attempting to match
  a template to the context node (if any). If an initial template is
  not set, the standard order of template invocation will be used.

  This function only applies when using QXmlQuery to process XSL-T
  stylesheets. The name becomes part of the compiled stylesheet.
  Therefore, this function must be called before calling setQuery().

  If \a localName is not a valid \l{QXmlName::localName()} {local
  name}, the effect is undefined. If the stylesheet has no template
  named \a localName, the processor will use the standard order of
  template invocation.

  \since 4.5
  \sa initialTemplateName()
 */
void QXmlQuery::setInitialTemplateName(const QString &localName)
{
   Q_ASSERT_X(QXmlName::isNCName(localName),
              Q_FUNC_INFO,
              "The name passed must be a valid NCName.");
   setInitialTemplateName(QXmlName(d->namePool, localName));
}

/*!
  Returns the name of the XSL-T stylesheet template that the processor
  will call first when running an XSL-T stylesheet. This function only
  applies when using QXmlQuery to process XSL-T stylesheets. By
  default, no initial template is set. In that case, a default
  constructed QXmlName is returned.

  \since 4.5
 */
QXmlName QXmlQuery::initialTemplateName() const
{
   return d->initialTemplateName;
}

/*!
  Sets the network manager to \a newManager.
  QXmlQuery does not take ownership of \a newManager.

  \sa networkAccessManager()
  \since 4.5
 */
void QXmlQuery::setNetworkAccessManager(QNetworkAccessManager *newManager)
{
   d->m_networkAccessDelegator->m_genericManager = newManager;
}

/*!
  Returns the network manager, or 0 if it has not been set.

  \sa setNetworkAccessManager()
  \since 4.5
 */
QNetworkAccessManager *QXmlQuery::networkAccessManager() const
{
   return d->m_networkAccessDelegator->m_genericManager;
}

/*!
  Binds the result of the query \a query, to a variable by name \a name.

  Evaluation of \a query will be commenced when this function is called.

  If \a query is invalid, behavior is undefined. \a query will be copied.

  \since 4.5
  \sa isValid()
 */
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

/*!
 \overload

 Has the same behavior and effects as the function being overloaded, but takes
 the variable name \a localName as a QString. \a query is used as in the
 overloaded function.

  \since 4.5
 */
void QXmlQuery::bindVariable(const QString &localName, const QXmlQuery &query)
{
   return bindVariable(QXmlName(d->namePool, localName), query);
}
