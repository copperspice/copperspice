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

#include <QHash>

#include "qxmlname.h"
#include "qnamepool_p.h"
#include "qabstractxmlpullprovider_p.h"

QT_BEGIN_NAMESPACE

using namespace QPatternist;

// TODO have example where query selects, and the events for the result are indented

/*!
  \internal
  \class AbstractXmlPullProvider
  \brief The AbstractXmlPullProvider class provides a pull-based stream interface for the XPath Data Model.
  \reentrant
  \ingroup xml-tools

  AbstractXmlPullProvider allows a stream of items from the XPath Data Model -- essentially XML --
  to be iterated over. The subclass of AbstractXmlPullProvider provides the events, and the
  user calling next() and so on, consumes them. AbstractXmlPullProvider can be considered
  a forward-only, non-reversible iterator.

  Note that the content the events describes, are not necessarily a well-formed XML document, but
  rather an instance of the XPath Data model, to be specific. For instance, maybe a pull provider
  returns two atomic values, followed by an element tree, and at the end two document nodes.

  If you are subclassing AbstractXmlPullProvider, be careful to correctly implement
  the behaviors, as described for the individual members and events.

  \sa AbstractXmlPullProvider::Event
 */

/*!
  \enum AbstractXmlPullProvider::Event
  \value StartOfInput The value AbstractXmlPullProvider::current() returns before the first call to next().
  \value AtomicValue an atomic value such as an \c xs:integer, \c xs:hexBinary, or \c xs:dateTime. Atomic values
         can only be top level items.
  \value StartDocument Signals the start of a document node. Note that a AbstractXmlPullProvider can provide
         a sequence of document nodes.
  \value EndDocument Signals the end of a document node. StartDocument and EndDocument are always balanced
         and always top-level events. For instance, StartDocument can never appear after any StartElement
         events that hasn't been balanced by the corresponding amount of EndElement events.
  \value StartElement Signals an element start tag.
  \value EndElement Signals the end of an element. StartElement and EndElement events are always balanced.
  \value Text Signals a text node. Adjacent text nodes cannot occur.
  \value ProcessingInstruction A processing instruction. Its name is returned from name(), and its value in stringValue().
  \value Comment a comment node. Its value can be retrieved with stingValue().
  \value Attribute Signals an attribute node. Attribute events can only appear after Namespace events, or
         if no such are sent, after the StartElement. In addition they must appear sequentially,
         and each name must be unique. The ordering of attribute events is undefined and insignificant.
  \value Namespace Signals a namespace binding. They occur very infrequently and are not needed for attributes
         and elements. Namespace events can only appear after the StartElement event. The
         ordering of namespace events is undefined and insignificant.
  \value EndOfInput When next() is called after the last event, EndOfInput is returned.

  \sa AbstractXmlPullProvider::current()
 */

/*!
  Constucts a AbstractXmlPullProvider instance.
 */
AbstractXmlPullProvider::AbstractXmlPullProvider()
{
}

/*!
  Destructs this AbstractXmlPullProvider.
 */
AbstractXmlPullProvider::~AbstractXmlPullProvider()
{
}

/*!
  \fn Event AbstractXmlPullProvider::next() = 0;
  Advances this AbstractXmlPullProvider, and returns the new event.

  \sa current()
 */

/*!
  \fn Event AbstractXmlPullProvider::current() const = 0;
  Returns the event that next() returned the last time it was called. It doesn't
  alter this AbstractXmlPullProvider.

  current() may not modify this AbstractXmlPullProvider's state. Subsequent calls to current()
  must return the same value.

  \sa AbstractXmlPullProvider::Event
 */

/*!
  \fn QName AbstractXmlPullProvider::name() const = 0;
  If the current event is StartElement,
  EndElement, ProcessingInstruction, Attribute, or Namespace, the node's name is returned.

  If the current event is ProcessingInstruction,
  the processing instruction target is in in the local name.

  If the current event is Namespace, the name's namespace URI is the namespace, and
  the local name is the prefix the name is binding to.

  In all other cases, an invalid QName is returned.
 */

/*!
  \fn QVariant AbstractXmlPullProvider::atomicValue() const = 0;

  If current() event is AtomicValue, the atomic value is returned as a QVariant.
  In all other cases, this function returns a null QVariant.
 */

/*!
 \fn QString AbstractXmlPullProvider::stringValue() const = 0;

  If current() is Text, the text node's value is returned.

  If the current() event is Comment, its value is returned. The subclasser guarantees
  it does not contain the string "-->".

  If the current() event is ProcessingInstruction, its data is returned. The subclasser
  guarantees the data does not contain the string "?>".

  In other cases, it returns a default constructed string.
  */

/*!
 \fn QHash<QXmlName, QString> AbstractXmlPullProvider::attributes() = 0;

  If the current() is Element, the attributes of the element are returned,
  an empty list of attributes otherwise.
 */

QT_END_NAMESPACE

