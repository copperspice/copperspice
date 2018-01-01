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

#include "qxmlschema.h"
#include "qxmlschema_p.h"

#include <QtCore/QIODevice>
#include <QtCore/QUrl>

QT_BEGIN_NAMESPACE

/*!
  \class QXmlSchema

  \brief The QXmlSchema class provides loading and validation of a W3C XML Schema.

  \reentrant
  \since 4.6
  \ingroup xml-tools

  The QXmlSchema class loads, compiles and validates W3C XML Schema files
  that can be used further for validation of XML instance documents via
  \l{QXmlSchemaValidator}.

  The following example shows how to load a XML Schema file from the network
  and test whether it is a valid schema document:

  \snippet doc/src/snippets/qxmlschema/main.cpp 0

  \section1 XML Schema Version

  This class is used to represent schemas that conform to the \l{XML Schema} 1.0
  specification.

  \sa QXmlSchemaValidator, {xmlpatterns/schema}{XML Schema Validation Example}
*/

/*!
  Constructs an invalid, empty schema that cannot be used until
  load() is called.
 */
QXmlSchema::QXmlSchema()
   : d(new QXmlSchemaPrivate(QXmlNamePool()))
{
}

/*!
  Constructs a QXmlSchema that is a copy of \a other. The new
  instance will share resources with the existing schema
  to the extent possible.
 */
QXmlSchema::QXmlSchema(const QXmlSchema &other)
   : d(other.d)
{
}

/*!
  Destroys this QXmlSchema.
 */
QXmlSchema::~QXmlSchema()
{
}

/*!
  Sets this QXmlSchema to a schema loaded from the \a source
  URI.

  If the schema \l {isValid()} {is invalid}, \c{false} is returned
  and the behavior is undefined.

  Example:

  \snippet doc/src/snippets/qxmlschema/main.cpp 0

  \sa isValid()
 */
bool QXmlSchema::load(const QUrl &source)
{
   d->load(source, QString());
   return d->isValid();
}

/*!
  Sets this QXmlSchema to a schema read from the \a source
  device. The device must have been opened with at least
  QIODevice::ReadOnly.

  \a documentUri represents the schema obtained from the \a source
  device. It is the base URI of the schema, that is used
  internally to resolve relative URIs that appear in the schema, and
  for message reporting.

  If \a source is \c null or not readable, or if \a documentUri is not
  a valid URI, behavior is undefined.

  If the schema \l {isValid()} {is invalid}, \c{false} is returned
  and the behavior is undefined.

  Example:

  \snippet doc/src/snippets/qxmlschema/main.cpp 1

  \sa isValid()
 */
bool QXmlSchema::load(QIODevice *source, const QUrl &documentUri)
{
   d->load(source, documentUri, QString());
   return d->isValid();
}

/*!
  Sets this QXmlSchema to a schema read from the \a data

  \a documentUri represents the schema obtained from the \a data.
  It is the base URI of the schema, that is used internally to
  resolve relative URIs that appear in the schema, and
  for message reporting.

  If \a documentUri is not a valid URI, behavior is undefined.
  \sa isValid()

  If the schema \l {isValid()} {is invalid}, \c{false} is returned
  and the behavior is undefined.

  Example:

  \snippet doc/src/snippets/qxmlschema/main.cpp 2

  \sa isValid()
 */
bool QXmlSchema::load(const QByteArray &data, const QUrl &documentUri)
{
   d->load(data, documentUri, QString());
   return d->isValid();
}

/*!
  Returns true if this schema is valid. Examples of invalid schemas
  are ones that contain syntax errors or that do not conform the
  W3C XML Schema specification.
 */
bool QXmlSchema::isValid() const
{
   return d->isValid();
}

/*!
  Returns the name pool used by this QXmlSchema for constructing \l
  {QXmlName} {names}. There is no setter for the name pool, because
  mixing name pools causes errors due to name confusion.
 */
QXmlNamePool QXmlSchema::namePool() const
{
   return d->namePool();
}

/*!
  Returns the document URI of the schema or an empty URI if no
  schema has been set.
 */
QUrl QXmlSchema::documentUri() const
{
   return d->documentUri();
}

/*!
  Changes the \l {QAbstractMessageHandler}{message handler} for this
  QXmlSchema to \a handler. The schema sends all compile and
  validation messages to this message handler. QXmlSchema does not take
  ownership of \a handler.

  Normally, the default message handler is sufficient. It writes
  compile and validation messages to \e stderr. The default message
  handler includes color codes if \e stderr can render colors.

  When QXmlSchema calls QAbstractMessageHandler::message(),
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
void QXmlSchema::setMessageHandler(QAbstractMessageHandler *handler)
{
   d->setMessageHandler(handler);
}

/*!
    Returns the message handler that handles compile and validation
    messages for this QXmlSchema.
 */
QAbstractMessageHandler *QXmlSchema::messageHandler() const
{
   return d->messageHandler();
}

/*!
  Sets the URI resolver to \a resolver. QXmlSchema does not take
  ownership of \a resolver.

  \sa uriResolver()
 */
void QXmlSchema::setUriResolver(const QAbstractUriResolver *resolver)
{
   d->setUriResolver(resolver);
}

/*!
  Returns the schema's URI resolver. If no URI resolver has been set,
  QtXmlPatterns will use the URIs in schemas as they are.

  The URI resolver provides a level of abstraction, or \e{polymorphic
  URIs}. A resolver can rewrite \e{logical} URIs to physical ones, or
  it can translate obsolete or invalid URIs to valid ones.

  When QtXmlPatterns calls QAbstractUriResolver::resolve() the
  absolute URI is the URI mandated by the schema specification, and the
  relative URI is the URI specified by the user.

  \sa setUriResolver()
 */
const QAbstractUriResolver *QXmlSchema::uriResolver() const
{
   return d->uriResolver();
}

/*!
  Sets the network manager to \a manager.
  QXmlSchema does not take ownership of \a manager.

  \sa networkAccessManager()
 */
void QXmlSchema::setNetworkAccessManager(QNetworkAccessManager *manager)
{
   d->setNetworkAccessManager(manager);
}

/*!
  Returns the network manager, or 0 if it has not been set.

  \sa setNetworkAccessManager()
 */
QNetworkAccessManager *QXmlSchema::networkAccessManager() const
{
   return d->networkAccessManager();
}

QT_END_NAMESPACE
