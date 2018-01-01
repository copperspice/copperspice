/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include "qxmlschemavalidator.h"
#include "qxmlschemavalidator_p.h"

#include "qacceltreeresourceloader_p.h"
#include "qxmlschema.h"
#include "qxmlschema_p.h"
#include "qxsdvalidatinginstancereader_p.h"

#include <QtCore/QBuffer>
#include <QtCore/QIODevice>
#include <QtCore/QUrl>

QT_BEGIN_NAMESPACE

/*!
  \class QXmlSchemaValidator

  \brief The QXmlSchemaValidator class validates XML instance documents against a W3C XML Schema.

  \reentrant
  \since 4.6
  \ingroup xml-tools

  The QXmlSchemaValidator class loads, parses an XML instance document and validates it
  against a W3C XML Schema that has been compiled with \l{QXmlSchema}.

  The following example shows how to load a XML Schema from a local
  file, check whether it is a valid schema document and use it for validation
  of an XML instance document:

  \snippet doc/src/snippets/qxmlschemavalidator/main.cpp 3

  \section1 XML Schema Version

  This class implements schema validation according to the \l{XML Schema} 1.0
  specification.

  \sa QXmlSchema, {xmlpatterns/schema}{XML Schema Validation Example}
*/

/*!
  Constructs a schema validator.
  The schema used for validation must be referenced in the XML instance document
  via the \c xsi:schemaLocation or \c xsi:noNamespaceSchemaLocation attribute.
 */
QXmlSchemaValidator::QXmlSchemaValidator()
   : d(new QXmlSchemaValidatorPrivate(QXmlSchema()))
{
}

/*!
  Constructs a schema validator that will use \a schema for validation.
  If an empty \l {QXmlSchema} schema is passed to the validator, the schema used
  for validation must be referenced in the XML instance document
  via the \c xsi:schemaLocation or \c xsi:noNamespaceSchemaLocation attribute.
 */
QXmlSchemaValidator::QXmlSchemaValidator(const QXmlSchema &schema)
   : d(new QXmlSchemaValidatorPrivate(schema))
{
}

/*!
  Destroys this QXmlSchemaValidator.
 */
QXmlSchemaValidator::~QXmlSchemaValidator()
{
   delete d;
}

/*!
  Sets the \a schema that shall be used for further validation.
  If the schema is empty, the schema used for validation must be referenced
  in the XML instance document via the \c xsi:schemaLocation or
  \c xsi:noNamespaceSchemaLocation attribute.
 */
void QXmlSchemaValidator::setSchema(const QXmlSchema &schema)
{
   d->setSchema(schema);
}

/*!
  Validates the XML instance document read from \a data with the
  given \a documentUri against the schema.

  Returns \c true if the XML instance document is valid according to the
  schema, \c false otherwise.

  Example:

  \snippet doc/src/snippets/qxmlschemavalidator/main.cpp 2
 */
bool QXmlSchemaValidator::validate(const QByteArray &data, const QUrl &documentUri) const
{
   QByteArray localData(data);

   QBuffer buffer(&localData);
   buffer.open(QIODevice::ReadOnly);

   return validate(&buffer, documentUri);
}

/*!
  Validates the XML instance document read from \a source against the schema.

  Returns \c true if the XML instance document is valid according to the
  schema, \c false otherwise.

  Example:

  \snippet doc/src/snippets/qxmlschemavalidator/main.cpp 0
 */
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

/*!
  Validates the XML instance document read from \a source with the
  given \a documentUri against the schema.

  Returns \c true if the XML instance document is valid according to the
  schema, \c false otherwise.

  Example:

  \snippet doc/src/snippets/qxmlschemavalidator/main.cpp 1
 */
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
      Q_UNUSED(exception);
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
      Q_UNUSED(exception);
      return false;
   }

   return true;
}

/*!
  Returns the name pool used by this QXmlSchemaValidator for constructing \l
  {QXmlName} {names}. There is no setter for the name pool, because
  mixing name pools causes errors due to name confusion.
 */
QXmlNamePool QXmlSchemaValidator::namePool() const
{
   return d->m_namePool;
}

/*!
  Returns the schema that is used for validation.
 */
QXmlSchema QXmlSchemaValidator::schema() const
{
   return d->m_originalSchema;
}

/*!
  Changes the \l {QAbstractMessageHandler}{message handler} for this
  QXmlSchemaValidator to \a handler. The schema validator sends all parsing and
  validation messages to this message handler. QXmlSchemaValidator does not take
  ownership of \a handler.

  Normally, the default message handler is sufficient. It writes
  compile and validation messages to \e stderr. The default message
  handler includes color codes if \e stderr can render colors.

  When QXmlSchemaValidator calls QAbstractMessageHandler::message(),
  the arguments are as follows:

  \table
  \header
    \o message() argument
    \o Semantics
  \row
    \o QtMsgType type
    \o Only QtWarningMsg and QtFatalMsg are used. The former
       identifies a warning, while the latter identifies an error.
  \row
    \o const QString & description
    \o An XHTML document which is the actual message. It is translated
       into the current language.
  \row
    \o const QUrl &identifier
    \o Identifies the error with a URI, where the fragment is
       the error code, and the rest of the URI is the error namespace.
  \row
    \o const QSourceLocation & sourceLocation
    \o Identifies where the error occurred.
  \endtable

 */
void QXmlSchemaValidator::setMessageHandler(QAbstractMessageHandler *handler)
{
   d->m_userMessageHandler = handler;
}

/*!
    Returns the message handler that handles parsing and validation
    messages for this QXmlSchemaValidator.
 */
QAbstractMessageHandler *QXmlSchemaValidator::messageHandler() const
{
   if (d->m_userMessageHandler) {
      return d->m_userMessageHandler;
   }

   return d->m_messageHandler.data()->value;
}

/*!
  Sets the URI resolver to \a resolver. QXmlSchemaValidator does not take
  ownership of \a resolver.

  \sa uriResolver()
 */
void QXmlSchemaValidator::setUriResolver(const QAbstractUriResolver *resolver)
{
   d->m_uriResolver = resolver;
}

/*!
  Returns the schema's URI resolver. If no URI resolver has been set,
  QtXmlPatterns will use the URIs in instance documents as they are.

  The URI resolver provides a level of abstraction, or \e{polymorphic
  URIs}. A resolver can rewrite \e{logical} URIs to physical ones, or
  it can translate obsolete or invalid URIs to valid ones.

  When QtXmlPatterns calls QAbstractUriResolver::resolve() the
  absolute URI is the URI mandated by the schema specification, and the
  relative URI is the URI specified by the user.

  \sa setUriResolver()
 */
const QAbstractUriResolver *QXmlSchemaValidator::uriResolver() const
{
   return d->m_uriResolver;
}

/*!
  Sets the network manager to \a manager.
  QXmlSchemaValidator does not take ownership of \a manager.

  \sa networkAccessManager()
 */
void QXmlSchemaValidator::setNetworkAccessManager(QNetworkAccessManager *manager)
{
   d->m_userNetworkAccessManager = manager;
}

/*!
  Returns the network manager, or 0 if it has not been set.

  \sa setNetworkAccessManager()
 */
QNetworkAccessManager *QXmlSchemaValidator::networkAccessManager() const
{
   if (d->m_userNetworkAccessManager) {
      return d->m_userNetworkAccessManager;
   }

   return d->m_networkAccessManager.data()->value;
}

QT_END_NAMESPACE
