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

#include <qscriptclass.h>
#include <qscriptstring.h>

class QScriptClassPrivate
{
   Q_DECLARE_PUBLIC(QScriptClass)

 public:
   QScriptClassPrivate() {}
   virtual ~QScriptClassPrivate() {}

   QScriptEngine *engine;

   QScriptClass *q_ptr;
};

/*!
  Constructs a QScriptClass object to be used in the given \a engine.

  The engine does not take ownership of the QScriptClass object.
*/
QScriptClass::QScriptClass(QScriptEngine *engine)
   : d_ptr(new QScriptClassPrivate)
{
   d_ptr->q_ptr = this;
   d_ptr->engine = engine;
}

/*!
  \internal
*/
QScriptClass::QScriptClass(QScriptEngine *engine, QScriptClassPrivate &dd)
   : d_ptr(&dd)
{
   d_ptr->q_ptr = this;
   d_ptr->engine = engine;
}

/*!
  Destroys the QScriptClass object.

  If a QScriptClass object is deleted before the associated engine(),
  any Qt Script objects using the QScriptClass will be "demoted" to
  normal Qt Script objects.
*/
QScriptClass::~QScriptClass()
{
}

/*!
  Returns the engine that this QScriptClass is associated with.
*/
QScriptEngine *QScriptClass::engine() const
{
   Q_D(const QScriptClass);
   return d->engine;
}

/*!
  Returns the object to be used as the prototype of new instances
  of this class (created with QScriptEngine::newObject()).

  The default implementation returns an invalid QScriptValue, meaning
  that the standard Object prototype will be used.  Reimplement this
  function to provide your own custom prototype.

  Typically you initialize your prototype object in the constructor of
  your class, then return it in this function.

  See the "Making Use of Prototype-Based Inheritance" section in the
  QtScript documentation for more information on how prototypes are
  used.
*/
QScriptValue QScriptClass::prototype() const
{
   return QScriptValue();
}

/*!
  Returns the name of the script class.

  Qt Script uses this name to generate a default string representation
  of objects in case you do not provide a toString function.

  The default implementation returns a null string.
*/
QString QScriptClass::name() const
{
   return QString();
}

/*!
  Queries this script class for how access to the property with the
  given \a name of the given \a object should be handled. The given \a
  flags specify the aspects of interest. This function should return a
  subset of \a flags to indicate which aspects of property access
  should be further handled by the script class.

  For example, if the \a flags contain HandlesReadAccess, and you
  would like your class to handle the reading of the property (through
  the property() function), the returned flags should include
  HandlesReadAccess. If the returned flags do not contain
  HandlesReadAccess, the property will be handled as a normal script
  object property.

  You can optionally use the \a id argument to store a value that will
  subsequently be passed on to functions such as property() and
  setProperty().

  The default implementation of this function returns 0.

  Note: This function is only called if the given property isn't
  already a normal property of the object. For example, say you
  advertise that you want to handle read access to property \c{foo},
  but not write access; if \c{foo} is then assigned a value, it will
  become a normal script object property, and subsequently you will no
  longer be queried regarding read access to \c{foo}.

  \sa property()
*/
QScriptClass::QueryFlags QScriptClass::queryProperty(const QScriptValue &object,
            const QScriptString &name, QueryFlags flags, uint *id)
{
   Q_UNUSED(object);
   Q_UNUSED(name);
   Q_UNUSED(flags);
   Q_UNUSED(id);

   return Qt::EmptyFlag;
}

/*!
  Returns the value of the property with the given \a name of the given
  \a object.

  The \a id argument is only useful if you assigned a value to it in
  queryProperty().

  The default implementation does nothing and returns an invalid QScriptValue.

  \sa setProperty(), propertyFlags()
*/
QScriptValue QScriptClass::property(const QScriptValue &object, const QScriptString &name, uint id)
{
   Q_UNUSED(object);
   Q_UNUSED(name);
   Q_UNUSED(id);
   return QScriptValue();
}

/*!
  Returns the flags of the property with the given \a name of the given
  \a object.

  The \a id argument is only useful if you assigned a value to it in
  queryProperty().

  The default implementation returns 0.

  \sa property()
*/
QScriptValue::PropertyFlags QScriptClass::propertyFlags(
   const QScriptValue &object, const QScriptString &name, uint id)
{
   Q_UNUSED(object);
   Q_UNUSED(name);
   Q_UNUSED(id);

   return Qt::EmptyFlag;
}

void QScriptClass::setProperty(QScriptValue &object, const QScriptString &name,
   uint id, const QScriptValue &value)
{
   Q_UNUSED(object);
   Q_UNUSED(name);
   Q_UNUSED(id);
   Q_UNUSED(value);
}

QScriptClassPropertyIterator *QScriptClass::newIterator(const QScriptValue &object)
{
   Q_UNUSED(object);
   return nullptr;
}

bool QScriptClass::supportsExtension(Extension extension) const
{
   Q_UNUSED(extension);
   return false;
}

QVariant QScriptClass::extension(Extension extension, const QVariant &argument)
{
   Q_UNUSED(extension);
   Q_UNUSED(argument);
   return QVariant();
}

