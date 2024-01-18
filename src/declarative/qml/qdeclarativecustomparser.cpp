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

#include "private/qdeclarativecustomparser_p.h"
#include "private/qdeclarativecustomparser_p_p.h"

#include "private/qdeclarativeparser_p.h"
#include "private/qdeclarativecompiler_p.h"

#include <QtCore/qdebug.h>

QT_BEGIN_NAMESPACE

using namespace QDeclarativeParser;

/*!
    \class QDeclarativeCustomParser
    \brief The QDeclarativeCustomParser class allows you to add new arbitrary types to QML.
    \internal

    By subclassing QDeclarativeCustomParser, you can add a parser for
    building a particular type.

    The subclass must implement compile() and setCustomData(), and register
    itself in the meta type system by calling the macro:

    \code
    QML_REGISTER_CUSTOM_TYPE(Module, MajorVersion, MinorVersion, Name, TypeClass, ParserClass)
    \endcode
*/

/*
    \fn QByteArray QDeclarativeCustomParser::compile(const QList<QDeclarativeCustomParserProperty> & properties)

    The custom parser processes \a properties, and returns
    a QByteArray containing data meaningful only to the
    custom parser; the type engine will pass this same data to
    setCustomData() when making an instance of the data.

    Errors must be reported via the error() functions.

    The QByteArray may be cached between executions of the system, so
    it must contain correctly-serialized data (not, for example,
    pointers to stack objects).
*/

/*
    \fn void QDeclarativeCustomParser::setCustomData(QObject *object, const QByteArray &data)

    This function sets \a object to have the properties defined
    by \a data, which is a block of data previously returned by a call
    to compile().

    Errors should be reported using qmlInfo(object).

    The \a object will be an instance of the TypeClass specified by QML_REGISTER_CUSTOM_TYPE.
*/

QDeclarativeCustomParserNode
QDeclarativeCustomParserNodePrivate::fromObject(QDeclarativeParser::Object *root)
{
   QDeclarativeCustomParserNode rootNode;
   rootNode.d->name = root->typeName;
   rootNode.d->location = root->location.start;

   for (QHash<QByteArray, Property *>::Iterator iter = root->properties.begin();
         iter != root->properties.end();
         ++iter) {

      Property *p = *iter;

      rootNode.d->properties << fromProperty(p);
   }

   if (root->defaultProperty) {
      rootNode.d->properties << fromProperty(root->defaultProperty);
   }

   return rootNode;
}

QDeclarativeCustomParserProperty
QDeclarativeCustomParserNodePrivate::fromProperty(QDeclarativeParser::Property *p)
{
   QDeclarativeCustomParserProperty prop;
   prop.d->name = p->name;
   prop.d->isList = (p->values.count() > 1);
   prop.d->location = p->location.start;

   if (p->value) {
      QDeclarativeCustomParserNode node = fromObject(p->value);
      QList<QDeclarativeCustomParserProperty> props = node.properties();
      for (int ii = 0; ii < props.count(); ++ii) {
         prop.d->values << QVariant::fromValue(props.at(ii));
      }
   } else {
      for (int ii = 0; ii < p->values.count(); ++ii) {
         QDeclarativeParser::Value *v = p->values.at(ii);
         v->type = QDeclarativeParser::Value::Literal;

         if (v->object) {
            QDeclarativeCustomParserNode node = fromObject(v->object);
            prop.d->values << QVariant::fromValue(node);
         } else {
            prop.d->values << QVariant::fromValue(v->value);
         }

      }
   }

   return prop;
}

QDeclarativeCustomParserNode::QDeclarativeCustomParserNode()
   : d(new QDeclarativeCustomParserNodePrivate)
{
}

QDeclarativeCustomParserNode::QDeclarativeCustomParserNode(const QDeclarativeCustomParserNode &other)
   : d(new QDeclarativeCustomParserNodePrivate)
{
   *this = other;
}

QDeclarativeCustomParserNode &QDeclarativeCustomParserNode::operator=(const QDeclarativeCustomParserNode &other)
{
   d->name = other.d->name;
   d->properties = other.d->properties;
   d->location = other.d->location;
   return *this;
}

QDeclarativeCustomParserNode::~QDeclarativeCustomParserNode()
{
   delete d;
   d = 0;
}

QByteArray QDeclarativeCustomParserNode::name() const
{
   return d->name;
}

QList<QDeclarativeCustomParserProperty> QDeclarativeCustomParserNode::properties() const
{
   return d->properties;
}

QDeclarativeParser::Location QDeclarativeCustomParserNode::location() const
{
   return d->location;
}

QDeclarativeCustomParserProperty::QDeclarativeCustomParserProperty()
   : d(new QDeclarativeCustomParserPropertyPrivate)
{
}

QDeclarativeCustomParserProperty::QDeclarativeCustomParserProperty(const QDeclarativeCustomParserProperty &other)
   : d(new QDeclarativeCustomParserPropertyPrivate)
{
   *this = other;
}

QDeclarativeCustomParserProperty &QDeclarativeCustomParserProperty::operator=(const QDeclarativeCustomParserProperty
      &other)
{
   d->name = other.d->name;
   d->isList = other.d->isList;
   d->values = other.d->values;
   d->location = other.d->location;
   return *this;
}

QDeclarativeCustomParserProperty::~QDeclarativeCustomParserProperty()
{
   delete d;
   d = 0;
}

QByteArray QDeclarativeCustomParserProperty::name() const
{
   return d->name;
}

bool QDeclarativeCustomParserProperty::isList() const
{
   return d->isList;
}

QDeclarativeParser::Location QDeclarativeCustomParserProperty::location() const
{
   return d->location;
}

QList<QVariant> QDeclarativeCustomParserProperty::assignedValues() const
{
   return d->values;
}

void QDeclarativeCustomParser::clearErrors()
{
   exceptions.clear();
}

/*!
    Reports an error with the given \a description.

    This can only be used during the compile() step. For errors during setCustomData(), use qmlInfo().

    An error is generated referring to the position of the element in the source file.
*/
void QDeclarativeCustomParser::error(const QString &description)
{
   Q_ASSERT(object);
   QDeclarativeError error;
   QString exceptionDescription;
   error.setLine(object->location.start.line);
   error.setColumn(object->location.start.column);
   error.setDescription(description);
   exceptions << error;
}

/*!
    Reports an error in parsing \a prop, with the given \a description.

    An error is generated referring to the position of \a node in the source file.
*/
void QDeclarativeCustomParser::error(const QDeclarativeCustomParserProperty &prop, const QString &description)
{
   QDeclarativeError error;
   QString exceptionDescription;
   error.setLine(prop.location().line);
   error.setColumn(prop.location().column);
   error.setDescription(description);
   exceptions << error;
}

/*!
    Reports an error in parsing \a node, with the given \a description.

    An error is generated referring to the position of \a node in the source file.
*/
void QDeclarativeCustomParser::error(const QDeclarativeCustomParserNode &node, const QString &description)
{
   QDeclarativeError error;
   QString exceptionDescription;
   error.setLine(node.location().line);
   error.setColumn(node.location().column);
   error.setDescription(description);
   exceptions << error;
}

/*!
    If \a script is a simply enum expression (eg. Text.AlignLeft),
    returns the integer equivalent (eg. 1).

    Otherwise, returns -1.
*/
int QDeclarativeCustomParser::evaluateEnum(const QByteArray &script) const
{
   return compiler->evaluateEnum(script);
}

/*!
    Resolves \a name to a type, or 0 if it is not a type. This can be used
    to type-check object nodes.
*/
const QMetaObject *QDeclarativeCustomParser::resolveType(const QByteArray &name) const
{
   return compiler->resolveType(name);
}

/*!
    Rewrites \a expression and returns an identifier that can be
    used to construct the binding later. \a name
    is used as the name of the rewritten function.
*/
QDeclarativeBinding::Identifier QDeclarativeCustomParser::rewriteBinding(const QString &expression,
      const QByteArray &name)
{
   return compiler->rewriteBinding(expression, name);
}

QT_END_NAMESPACE
