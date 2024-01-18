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

#include "private/qdeclarativedom_p.h"
#include "private/qdeclarativedom_p_p.h"

#include "private/qdeclarativecompiler_p.h"
#include "private/qdeclarativeengine_p.h"
#include "private/qdeclarativescriptparser_p.h"
#include "private/qdeclarativeglobal_p.h"

#include <QtCore/QByteArray>
#include <QtCore/QDebug>
#include <QtCore/QString>

QT_BEGIN_NAMESPACE

QDeclarativeDomDocumentPrivate::QDeclarativeDomDocumentPrivate()
   : root(0)
{
}

QDeclarativeDomDocumentPrivate::~QDeclarativeDomDocumentPrivate()
{
   if (root) {
      root->release();
   }
}

/*!
    \class QDeclarativeDomDocument
    \internal
    \brief The QDeclarativeDomDocument class represents the root of a QML document

    A QML document is a self-contained snippet of QML, usually contained in a
    single file. Each document has a root object, accessible through
    QDeclarativeDomDocument::rootObject().

    The QDeclarativeDomDocument class allows the programmer to inspect a QML document by
    calling QDeclarativeDomDocument::load().

    The following example loads a QML file from disk, and prints out its root
    object type and the properties assigned in the root object.
    \code
    QFile file(inputFileName);
    file.open(QIODevice::ReadOnly);
    QByteArray xmlData = file.readAll();

    QDeclarativeDomDocument document;
    document.load(qmlengine, xmlData);

    QDeclarativeDomObject rootObject = document.rootObject();
    qDebug() << rootObject.objectType();
    foreach(QDeclarativeDomProperty property, rootObject.properties())
        qDebug() << property.propertyName();
    \endcode
*/

/*!
    Construct an empty QDeclarativeDomDocument.
*/
QDeclarativeDomDocument::QDeclarativeDomDocument()
   : d(new QDeclarativeDomDocumentPrivate)
{
}

/*!
    Create a copy of \a other QDeclarativeDomDocument.
*/
QDeclarativeDomDocument::QDeclarativeDomDocument(const QDeclarativeDomDocument &other)
   : d(other.d)
{
}

/*!
    Destroy the QDeclarativeDomDocument
*/
QDeclarativeDomDocument::~QDeclarativeDomDocument()
{
}

/*!
    Assign \a other to this QDeclarativeDomDocument.
*/
QDeclarativeDomDocument &QDeclarativeDomDocument::operator=(const QDeclarativeDomDocument &other)
{
   d = other.d;
   return *this;
}

/*!
    Returns all import statements in qml.
*/
QList<QDeclarativeDomImport> QDeclarativeDomDocument::imports() const
{
   return d->imports;
}

/*!
    Loads a QDeclarativeDomDocument from \a data.  \a data should be valid QML
    data.  On success, true is returned.  If the \a data is malformed, false
    is returned and QDeclarativeDomDocument::errors() contains an error description.

    \sa QDeclarativeDomDocument::loadError()
*/
bool QDeclarativeDomDocument::load(QDeclarativeEngine *engine, const QByteArray &data, const QUrl &url)
{
   d->errors.clear();
   d->imports.clear();

   QDeclarativeEnginePrivate *ep = QDeclarativeEnginePrivate::get(engine);
   QDeclarativeTypeData *td = ep->typeLoader.get(data, url, QDeclarativeTypeLoader::PreserveParser);

   if (td->isError()) {
      d->errors = td->errors();
      td->release();
      return false;
   } else if (!td->isCompleteOrError()) {
      QDeclarativeError error;
      error.setDescription(QLatin1String("QDeclarativeDomDocument supports local types only"));
      d->errors << error;
      td->release();
      return false;
   }

   for (int i = 0; i < td->parser().imports().size(); ++i) {
      QDeclarativeScriptParser::Import parserImport = td->parser().imports().at(i);
      QDeclarativeDomImport domImport;
      domImport.d->type = static_cast<QDeclarativeDomImportPrivate::Type>(parserImport.type);
      domImport.d->uri = parserImport.uri;
      domImport.d->qualifier = parserImport.qualifier;
      domImport.d->version = parserImport.version;
      d->imports += domImport;
   }

   if (td->parser().tree()) {
      d->root = td->parser().tree();
      d->root->addref();
   }

   td->release();
   return true;
}

/*!
    Returns the last load errors.  The load errors will be reset after a
    successful call to load().

    \sa load()
*/
QList<QDeclarativeError> QDeclarativeDomDocument::errors() const
{
   return d->errors;
}

/*!
    Returns the document's root object, or an invalid QDeclarativeDomObject if the
    document has no root.

    In the sample QML below, the root object will be the QDeclarativeItem type.
    \qml
Item {
    Text {
        text: "Hello World"
    }
}
    \endqml
*/
QDeclarativeDomObject QDeclarativeDomDocument::rootObject() const
{
   QDeclarativeDomObject rv;
   rv.d->object = d->root;
   if (rv.d->object) {
      rv.d->object->addref();
   }
   return rv;
}

QDeclarativeDomPropertyPrivate::QDeclarativeDomPropertyPrivate()
   : property(0)
{
}

QDeclarativeDomPropertyPrivate::~QDeclarativeDomPropertyPrivate()
{
   if (property) {
      property->release();
   }
}

QDeclarativeDomDynamicPropertyPrivate::QDeclarativeDomDynamicPropertyPrivate():
   valid(false)
{
}

QDeclarativeDomDynamicPropertyPrivate::~QDeclarativeDomDynamicPropertyPrivate()
{
   if (valid && property.defaultValue) {
      property.defaultValue->release();
   }
}

/*!
    \class QDeclarativeDomProperty
    \internal
    \brief The QDeclarativeDomProperty class represents one property assignment in the
    QML DOM tree

    Properties in QML can be assigned QML \l {QDeclarativeDomValue}{values}.

    \sa QDeclarativeDomObject
*/

/*!
    Construct an invalid QDeclarativeDomProperty.
*/
QDeclarativeDomProperty::QDeclarativeDomProperty()
   : d(new QDeclarativeDomPropertyPrivate)
{
}

/*!
    Create a copy of \a other QDeclarativeDomProperty.
*/
QDeclarativeDomProperty::QDeclarativeDomProperty(const QDeclarativeDomProperty &other)
   : d(other.d)
{
}

/*!
    Destroy the QDeclarativeDomProperty.
*/
QDeclarativeDomProperty::~QDeclarativeDomProperty()
{
}

/*!
    Assign \a other to this QDeclarativeDomProperty.
*/
QDeclarativeDomProperty &QDeclarativeDomProperty::operator=(const QDeclarativeDomProperty &other)
{
   d = other.d;
   return *this;
}

/*!
    Returns true if this is a valid QDeclarativeDomProperty, false otherwise.
*/
bool QDeclarativeDomProperty::isValid() const
{
   return d->property != 0;
}


/*!
    Return the name of this property.

    \qml
Text {
    x: 10
    y: 10
    font.bold: true
}
    \endqml

    As illustrated above, a property name can be a simple string, such as "x" or
    "y", or a more complex "dot property", such as "font.bold".  In both cases
    the full name is returned ("x", "y" and "font.bold") by this method.

    For dot properties, a split version of the name can be accessed by calling
    QDeclarativeDomProperty::propertyNameParts().

    \sa QDeclarativeDomProperty::propertyNameParts()
*/
QByteArray QDeclarativeDomProperty::propertyName() const
{
   return d->propertyName;
}

/*!
    Return the name of this property, split into multiple parts in the case
    of dot properties.

    \qml
Text {
    x: 10
    y: 10
    font.bold: true
}
    \endqml

    For each of the properties shown above, this method would return ("x"),
    ("y") and ("font", "bold").

    \sa QDeclarativeDomProperty::propertyName()
*/
QList<QByteArray> QDeclarativeDomProperty::propertyNameParts() const
{
   if (d->propertyName.isEmpty()) {
      return QList<QByteArray>();
   } else {
      return d->propertyName.split('.');
   }
}

/*!
    Return true if this property is used as a default property in the QML
    document.

    \code
<Text text="hello"/>
<Text>hello</Text>
    \endcode

    The above two examples return the same DOM tree, except that the second has
    the default property flag set on the text property.  Observe that whether
    or not a property has isDefaultProperty set is determined by how the
    property is used, and not only by whether the property is the types default
    property.
*/
bool QDeclarativeDomProperty::isDefaultProperty() const
{
   return d->property && d->property->isDefault;
}

/*!
    Returns the QDeclarativeDomValue that is assigned to this property, or an invalid
    QDeclarativeDomValue if no value is assigned.
*/
QDeclarativeDomValue QDeclarativeDomProperty::value() const
{
   QDeclarativeDomValue rv;
   if (d->property) {
      rv.d->property = d->property;
      if (d->property->values.count()) {
         rv.d->value = d->property->values.at(0);
      } else {
         rv.d->value = d->property->onValues.at(0);
      }
      rv.d->property->addref();
      rv.d->value->addref();
   }
   return rv;
}

/*!
    Returns the position in the input data where the property ID startd, or -1 if
 the property is invalid.
*/
int QDeclarativeDomProperty::position() const
{
   if (d && d->property) {
      return d->property->location.range.offset;
   } else {
      return -1;
   }
}

/*!
    Returns the length in the input data from where the property ID started upto
 the end of it, or -1 if the property is invalid.
*/
int QDeclarativeDomProperty::length() const
{
   if (d && d->property) {
      return d->property->location.range.length;
   } else {
      return -1;
   }
}

/*!
    Construct an invalid QDeclarativeDomDynamicProperty.
*/
QDeclarativeDomDynamicProperty::QDeclarativeDomDynamicProperty():
   d(new QDeclarativeDomDynamicPropertyPrivate)
{
}

/*!
    Create a copy of \a other QDeclarativeDomDynamicProperty.
*/
QDeclarativeDomDynamicProperty::QDeclarativeDomDynamicProperty(const QDeclarativeDomDynamicProperty &other):
   d(other.d)
{
}

/*!
    Destroy the QDeclarativeDomDynamicProperty.
*/
QDeclarativeDomDynamicProperty::~QDeclarativeDomDynamicProperty()
{
}

/*!
    Assign \a other to this QDeclarativeDomDynamicProperty.
*/
QDeclarativeDomDynamicProperty &QDeclarativeDomDynamicProperty::operator=(const QDeclarativeDomDynamicProperty &other)
{
   d = other.d;
   return *this;
}

bool QDeclarativeDomDynamicProperty::isValid() const
{
   return d && d->valid;
}

/*!
    Return the name of this dynamic property.

    \qml
Item {
    property int count: 10;
}
    \endqml

    As illustrated above, a dynamic property name can have a name and a
    default value ("10").
*/
QByteArray QDeclarativeDomDynamicProperty::propertyName() const
{
   if (isValid()) {
      return d->property.name;
   } else {
      return QByteArray();
   }
}

/*!
   Returns the type of the dynamic property. Note that when the property is an
   alias property, this will return -1. Use QDeclarativeDomProperty::isAlias() to check
   if the property is an alias.
*/
int QDeclarativeDomDynamicProperty::propertyType() const
{
   if (isValid()) {
      switch (d->property.type) {
         case QDeclarativeParser::Object::DynamicProperty::Bool:
            return QMetaType::type("bool");

         case QDeclarativeParser::Object::DynamicProperty::Color:
            return QMetaType::type("QColor");

         case QDeclarativeParser::Object::DynamicProperty::Time:
            return QMetaType::type("QTime");

         case QDeclarativeParser::Object::DynamicProperty::Date:
            return QMetaType::type("QDate");

         case QDeclarativeParser::Object::DynamicProperty::DateTime:
            return QMetaType::type("QDateTime");

         case QDeclarativeParser::Object::DynamicProperty::Int:
            return QMetaType::type("int");

         case QDeclarativeParser::Object::DynamicProperty::Real:
            return sizeof(qreal) == sizeof(double) ? QMetaType::type("double") : QMetaType::type("float");

         case QDeclarativeParser::Object::DynamicProperty::String:
            return QMetaType::type("QString");

         case QDeclarativeParser::Object::DynamicProperty::Url:
            return QMetaType::type("QUrl");

         case QDeclarativeParser::Object::DynamicProperty::Variant:
            return QMetaType::type("QVariant");

         default:
            break;
      }
   }

   return -1;
}

QByteArray QDeclarativeDomDynamicProperty::propertyTypeName() const
{
   if (isValid()) {
      return d->property.customType;
   }

   return QByteArray();
}

/*!
    Return true if this property is used as a default property in the QML
    document.

    \code
<Text text="hello"/>
<Text>hello</Text>
    \endcode

    The above two examples return the same DOM tree, except that the second has
    the default property flag set on the text property.  Observe that whether
    or not a property has isDefaultProperty set is determined by how the
    property is used, and not only by whether the property is the types default
    property.
*/
bool QDeclarativeDomDynamicProperty::isDefaultProperty() const
{
   if (isValid()) {
      return d->property.isDefaultProperty;
   } else {
      return false;
   }
}

/*!
    Returns the default value as a QDeclarativeDomProperty.
*/
QDeclarativeDomProperty QDeclarativeDomDynamicProperty::defaultValue() const
{
   QDeclarativeDomProperty rp;

   if (isValid() && d->property.defaultValue) {
      rp.d->property = d->property.defaultValue;
      rp.d->propertyName = propertyName();
      rp.d->property->addref();
   }

   return rp;
}

/*!
    Returns true if this dynamic property is an alias for another property,
    false otherwise.
*/
bool QDeclarativeDomDynamicProperty::isAlias() const
{
   if (isValid()) {
      return d->property.type == QDeclarativeParser::Object::DynamicProperty::Alias;
   } else {
      return false;
   }
}

/*!
    Returns the position in the input data where the property ID startd, or 0 if
 the property is invalid.
*/
int QDeclarativeDomDynamicProperty::position() const
{
   if (isValid()) {
      return d->property.location.range.offset;
   } else {
      return -1;
   }
}

/*!
    Returns the length in the input data from where the property ID started upto
 the end of it, or 0 if the property is invalid.
*/
int QDeclarativeDomDynamicProperty::length() const
{
   if (isValid()) {
      return d->property.location.range.length;
   } else {
      return -1;
   }
}

QDeclarativeDomObjectPrivate::QDeclarativeDomObjectPrivate()
   : object(0)
{
}

QDeclarativeDomObjectPrivate::~QDeclarativeDomObjectPrivate()
{
   if (object) {
      object->release();
   }
}

QDeclarativeDomObjectPrivate::Properties
QDeclarativeDomObjectPrivate::properties() const
{
   Properties rv;

   for (QHash<QByteArray, QDeclarativeParser::Property *>::ConstIterator iter =
            object->properties.begin();
         iter != object->properties.end();
         ++iter) {

      rv << properties(*iter);

   }
   return rv;
}

QDeclarativeDomObjectPrivate::Properties
QDeclarativeDomObjectPrivate::properties(QDeclarativeParser::Property *property) const
{
   Properties rv;

   if (property->value) {

      for (QHash<QByteArray, QDeclarativeParser::Property *>::ConstIterator iter =
               property->value->properties.begin();
            iter != property->value->properties.end();
            ++iter) {

         rv << properties(*iter);

      }

      QByteArray name(property->name + '.');
      for (Properties::Iterator iter = rv.begin(); iter != rv.end(); ++iter) {
         iter->second.prepend(name);
      }

   } else {
      rv << qMakePair(property, property->name);
   }

   return rv;
}

/*!
    \class QDeclarativeDomObject
    \internal
    \brief The QDeclarativeDomObject class represents an object instantiation.

    Each object instantiated in a QML file has a corresponding QDeclarativeDomObject
    node in the QML DOM.

    In addition to the type information that determines the object to
    instantiate, QDeclarativeDomObject's also have a set of associated QDeclarativeDomProperty's.
    Each QDeclarativeDomProperty represents a QML property assignment on the instantiated
    object.  For example,

    \qml
QGraphicsWidget {
    opacity: 0.5
    size: "100x100"
}
    \endqml

    describes a single QDeclarativeDomObject - "QGraphicsWidget" - with two properties,
    "opacity" and "size".  Obviously QGraphicsWidget has many more properties than just
    these two, but the QML DOM representation only contains those assigned
    values (or bindings) in the QML file.
*/

/*!
    Construct an invalid QDeclarativeDomObject.
*/
QDeclarativeDomObject::QDeclarativeDomObject()
   : d(new QDeclarativeDomObjectPrivate)
{
}

/*!
    Create a copy of \a other QDeclarativeDomObject.
*/
QDeclarativeDomObject::QDeclarativeDomObject(const QDeclarativeDomObject &other)
   : d(other.d)
{
}

/*!
    Destroy the QDeclarativeDomObject.
*/
QDeclarativeDomObject::~QDeclarativeDomObject()
{
}

/*!
    Assign \a other to this QDeclarativeDomObject.
*/
QDeclarativeDomObject &QDeclarativeDomObject::operator=(const QDeclarativeDomObject &other)
{
   d = other.d;
   return *this;
}

/*!
    Returns true if this is a valid QDeclarativeDomObject, false otherwise.
*/
bool QDeclarativeDomObject::isValid() const
{
   return d->object != 0;
}

/*!
    Returns the fully-qualified type name of this object.

    For example, the type of this object would be "Qt/4.6/Rectangle".
    \qml
Rectangle { }
    \endqml
*/
QByteArray QDeclarativeDomObject::objectType() const
{
   if (d->object) {
      return d->object->typeName;
   } else {
      return QByteArray();
   }
}

/*!
    Returns the type name as referenced in the qml file.

    For example, the type of this object would be "Rectangle".
    \qml
Rectangle { }
    \endqml
*/
QByteArray QDeclarativeDomObject::objectClassName() const
{
   if (d->object) {
      return d->object->className;
   } else {
      return QByteArray();
   }
}

int QDeclarativeDomObject::objectTypeMajorVersion() const
{
   if (d->object) {
      return d->object->majorVersion;
   } else {
      return -1;
   }
}

int QDeclarativeDomObject::objectTypeMinorVersion() const
{
   if (d->object) {
      return d->object->minorVersion;
   } else {
      return -1;
   }
}

/*!
    Returns the QML id assigned to this object, or an empty QByteArray if no id
    has been assigned.

    For example, the object id of this object would be "MyText".
    \qml
Text { id: myText }
    \endqml
*/
QString QDeclarativeDomObject::objectId() const
{
   if (d->object) {
      return d->object->id;
   } else {
      return QString();
   }
}

/*!
    Returns the list of assigned properties on this object.

    In the following example, "text" and "x" properties would be returned.
    \qml
Text {
    text: "Hello world!"
    x: 100
}
    \endqml
*/
QList<QDeclarativeDomProperty> QDeclarativeDomObject::properties() const
{
   QList<QDeclarativeDomProperty> rv;

   if (!d->object || isComponent()) {
      return rv;
   }

   QDeclarativeDomObjectPrivate::Properties properties = d->properties();
   for (int ii = 0; ii < properties.count(); ++ii) {

      QDeclarativeDomProperty domProperty;
      domProperty.d->property = properties.at(ii).first;
      domProperty.d->property->addref();
      domProperty.d->propertyName = properties.at(ii).second;
      rv << domProperty;

   }

   if (d->object->defaultProperty) {
      QDeclarativeDomProperty domProperty;
      domProperty.d->property = d->object->defaultProperty;
      domProperty.d->property->addref();
      domProperty.d->propertyName = d->object->defaultProperty->name;
      rv << domProperty;
   }

   return rv;
}

/*!
    Returns the object's \a name property if a value has been assigned to
    it, or an invalid QDeclarativeDomProperty otherwise.

    In the example below, \c {object.property("source")} would return a valid
    QDeclarativeDomProperty, and \c {object.property("tile")} an invalid QDeclarativeDomProperty.

    \qml
Image { source: "sample.jpg" }
    \endqml
*/
QDeclarativeDomProperty QDeclarativeDomObject::property(const QByteArray &name) const
{
   QList<QDeclarativeDomProperty> props = properties();
   for (int ii = 0; ii < props.count(); ++ii)
      if (props.at(ii).propertyName() == name) {
         return props.at(ii);
      }
   return QDeclarativeDomProperty();
}

QList<QDeclarativeDomDynamicProperty> QDeclarativeDomObject::dynamicProperties() const
{
   QList<QDeclarativeDomDynamicProperty> properties;

   for (int i = 0; i < d->object->dynamicProperties.size(); ++i) {
      QDeclarativeDomDynamicProperty p;
      p.d = new QDeclarativeDomDynamicPropertyPrivate;
      p.d->property = d->object->dynamicProperties.at(i);
      p.d->valid = true;

      if (p.d->property.defaultValue) {
         p.d->property.defaultValue->addref();
      }

      properties.append(p);
   }

   return properties;
}

QDeclarativeDomDynamicProperty QDeclarativeDomObject::dynamicProperty(const QByteArray &name) const
{
   QDeclarativeDomDynamicProperty p;

   if (!isValid()) {
      return p;
   }

   for (int i = 0; i < d->object->dynamicProperties.size(); ++i) {
      if (d->object->dynamicProperties.at(i).name == name) {
         p.d = new QDeclarativeDomDynamicPropertyPrivate;
         p.d->property = d->object->dynamicProperties.at(i);
         if (p.d->property.defaultValue) {
            p.d->property.defaultValue->addref();
         }
         p.d->valid = true;
      }
   }

   return p;
}

/*!
    Returns true if this object is a custom type.  Custom types are special
    types that allow embeddeding non-QML data, such as SVG or HTML data,
    directly into QML files.

    \note Currently this method will always return false, and is a placekeeper
    for future functionality.

    \sa QDeclarativeDomObject::customTypeData()
*/
bool QDeclarativeDomObject::isCustomType() const
{
   return false;
}

/*!
    If this object represents a custom type, returns the data associated with
    the custom type, otherwise returns an empty QByteArray().
    QDeclarativeDomObject::isCustomType() can be used to check if this object represents
    a custom type.
*/
QByteArray QDeclarativeDomObject::customTypeData() const
{
   return QByteArray();
}

/*!
    Returns true if this object is a sub-component object.  Sub-component
    objects can be converted into QDeclarativeDomComponent instances by calling
    QDeclarativeDomObject::toComponent().

    \sa QDeclarativeDomObject::toComponent()
*/
bool QDeclarativeDomObject::isComponent() const
{
   return (d->object && (d->object->typeName == "Qt/Component" || d->object->typeName == "QtQuick/Component"));
}

/*!
    Returns a QDeclarativeDomComponent for this object if it is a sub-component, or
    an invalid QDeclarativeDomComponent if not.  QDeclarativeDomObject::isComponent() can be used
    to check if this object represents a sub-component.

    \sa QDeclarativeDomObject::isComponent()
*/
QDeclarativeDomComponent QDeclarativeDomObject::toComponent() const
{
   QDeclarativeDomComponent rv;
   if (isComponent()) {
      rv.d = d;
   }
   return rv;
}

/*!
    Returns the position in the input data where the property assignment started
, or -1 if the property is invalid.
*/
int QDeclarativeDomObject::position() const
{
   if (d && d->object) {
      return d->object->location.range.offset;
   } else {
      return -1;
   }
}

/*!
    Returns the length in the input data from where the property assignment star
ted upto the end of it, or -1 if the property is invalid.
*/
int QDeclarativeDomObject::length() const
{
   if (d && d->object) {
      return d->object->location.range.length;
   } else {
      return -1;
   }
}

// Returns the URL of the type, if it is an external type, or an empty URL if
// not
QUrl QDeclarativeDomObject::url() const
{
   if (d && d->object) {
      return d->object->url;
   } else {
      return QUrl();
   }
}

QDeclarativeDomBasicValuePrivate::QDeclarativeDomBasicValuePrivate()
   : value(0)
{
}

QDeclarativeDomBasicValuePrivate::~QDeclarativeDomBasicValuePrivate()
{
   if (value) {
      value->release();
   }
}

/*!
    \class QDeclarativeDomValueLiteral
    \internal
    \brief The QDeclarativeDomValueLiteral class represents a literal value.

    A literal value is a simple value, written inline with the QML.  In the
    example below, the "x", "y" and "color" properties are being assigned
    literal values.

    \qml
Rectangle {
    x: 10
    y: 10
    color: "red"
}
    \endqml
*/

/*!
    Construct an empty QDeclarativeDomValueLiteral.
*/
QDeclarativeDomValueLiteral::QDeclarativeDomValueLiteral():
   d(new QDeclarativeDomBasicValuePrivate)
{
}

/*!
    Create a copy of \a other QDeclarativeDomValueLiteral.
*/
QDeclarativeDomValueLiteral::QDeclarativeDomValueLiteral(const QDeclarativeDomValueLiteral &other)
   : d(other.d)
{
}

/*!
    Destroy the QDeclarativeDomValueLiteral.
*/
QDeclarativeDomValueLiteral::~QDeclarativeDomValueLiteral()
{
}

/*!
    Assign \a other to this QDeclarativeDomValueLiteral.
*/
QDeclarativeDomValueLiteral &QDeclarativeDomValueLiteral::operator=(const QDeclarativeDomValueLiteral &other)
{
   d = other.d;
   return *this;
}

/*!
    Return the literal value.

    In the example below, the literal value will be the string "10".
    \qml
Rectangle { x: 10 }
    \endqml
*/
QString QDeclarativeDomValueLiteral::literal() const
{
   if (d->value) {
      return d->value->primitive();
   } else {
      return QString();
   }
}

/*!
    \class QDeclarativeDomValueBinding
    \internal
    \brief The QDeclarativeDomValueBinding class represents a property binding.

    A property binding is an ECMAScript expression assigned to a property.  In
    the example below, the "x" property is being assigned a property binding.

    \qml
Rectangle { x: Other.x }
    \endqml
*/

/*!
    Construct an empty QDeclarativeDomValueBinding.
*/
QDeclarativeDomValueBinding::QDeclarativeDomValueBinding():
   d(new QDeclarativeDomBasicValuePrivate)
{
}

/*!
    Create a copy of \a other QDeclarativeDomValueBinding.
*/
QDeclarativeDomValueBinding::QDeclarativeDomValueBinding(const QDeclarativeDomValueBinding &other)
   : d(other.d)
{
}

/*!
    Destroy the QDeclarativeDomValueBinding.
*/
QDeclarativeDomValueBinding::~QDeclarativeDomValueBinding()
{
}

/*!
    Assign \a other to this QDeclarativeDomValueBinding.
*/
QDeclarativeDomValueBinding &QDeclarativeDomValueBinding::operator=(const QDeclarativeDomValueBinding &other)
{
   d = other.d;
   return *this;
}

/*!
    Return the binding expression.

    In the example below, the string "Other.x" will be returned.
    \qml
Rectangle { x: Other.x }
    \endqml
*/
QString QDeclarativeDomValueBinding::binding() const
{
   if (d->value) {
      return d->value->value.asScript();
   } else {
      return QString();
   }
}

/*!
    \class QDeclarativeDomValueValueSource
    \internal
    \brief The QDeclarativeDomValueValueSource class represents a value source assignment value.

    In QML, value sources are special value generating types that may be
    assigned to properties.  Value sources inherit the QDeclarativePropertyValueSource
    class.  In the example below, the "x" property is being assigned the
    NumberAnimation value source.

    \qml
Rectangle {
    x: NumberAnimation {
        from: 0
        to: 100
        loops: Animation.Infinite
    }
}
    \endqml
*/

/*!
    Construct an empty QDeclarativeDomValueValueSource.
*/
QDeclarativeDomValueValueSource::QDeclarativeDomValueValueSource():
   d(new QDeclarativeDomBasicValuePrivate)
{
}

/*!
    Create a copy of \a other QDeclarativeDomValueValueSource.
*/
QDeclarativeDomValueValueSource::QDeclarativeDomValueValueSource(const QDeclarativeDomValueValueSource &other)
   : d(other.d)
{
}

/*!
    Destroy the QDeclarativeDomValueValueSource.
*/
QDeclarativeDomValueValueSource::~QDeclarativeDomValueValueSource()
{
}

/*!
    Assign \a other to this QDeclarativeDomValueValueSource.
*/
QDeclarativeDomValueValueSource &QDeclarativeDomValueValueSource::operator=(const QDeclarativeDomValueValueSource
      &other)
{
   d = other.d;
   return *this;
}

/*!
    Return the value source object.

    In the example below, an object representing the NumberAnimation will be
    returned.
    \qml
Rectangle {
    x: NumberAnimation {
        from: 0
        to: 100
        loops: Animation.Infinite
    }
}
    \endqml
*/
QDeclarativeDomObject QDeclarativeDomValueValueSource::object() const
{
   QDeclarativeDomObject rv;
   if (d->value) {
      rv.d->object = d->value->object;
      rv.d->object->addref();
   }
   return rv;
}

/*!
    \class QDeclarativeDomValueValueInterceptor
    \internal
    \brief The QDeclarativeDomValueValueInterceptor class represents a value interceptor assignment value.

    In QML, value interceptor are special write-intercepting types that may be
    assigned to properties.  Value interceptor inherit the QDeclarativePropertyValueInterceptor
    class.  In the example below, the "x" property is being assigned the
    Behavior value interceptor.

    \qml
Rectangle {
    Behavior on x { NumberAnimation { duration: 500 } }
}
    \endqml
*/

/*!
    Construct an empty QDeclarativeDomValueValueInterceptor.
*/
QDeclarativeDomValueValueInterceptor::QDeclarativeDomValueValueInterceptor():
   d(new QDeclarativeDomBasicValuePrivate)
{
}

/*!
    Create a copy of \a other QDeclarativeDomValueValueInterceptor.
*/
QDeclarativeDomValueValueInterceptor::QDeclarativeDomValueValueInterceptor(const QDeclarativeDomValueValueInterceptor
      &other)
   : d(other.d)
{
}

/*!
    Destroy the QDeclarativeDomValueValueInterceptor.
*/
QDeclarativeDomValueValueInterceptor::~QDeclarativeDomValueValueInterceptor()
{
}

/*!
    Assign \a other to this QDeclarativeDomValueValueInterceptor.
*/
QDeclarativeDomValueValueInterceptor &QDeclarativeDomValueValueInterceptor::operator=
(const QDeclarativeDomValueValueInterceptor &other)
{
   d = other.d;
   return *this;
}

/*!
    Return the value interceptor object.

    In the example below, an object representing the Behavior will be
    returned.
    \qml
Rectangle {
    Behavior on x { NumberAnimation { duration: 500 } }
}
    \endqml
*/
QDeclarativeDomObject QDeclarativeDomValueValueInterceptor::object() const
{
   QDeclarativeDomObject rv;
   if (d->value) {
      rv.d->object = d->value->object;
      rv.d->object->addref();
   }
   return rv;
}

QDeclarativeDomValuePrivate::QDeclarativeDomValuePrivate()
   : property(0), value(0)
{
}

QDeclarativeDomValuePrivate::~QDeclarativeDomValuePrivate()
{
   if (property) {
      property->release();
   }
   if (value) {
      value->release();
   }
}

/*!
    \class QDeclarativeDomValue
    \internal
    \brief The QDeclarativeDomValue class represents a generic Qml value.

    QDeclarativeDomValue's can be assigned to QML \l {QDeclarativeDomProperty}{properties}.  In
    QML, properties can be assigned various different values, including basic
    literals, property bindings, property value sources, objects and lists of
    values.  The QDeclarativeDomValue class allows a programmer to determine the specific
    value type being assigned and access more detailed information through a
    corresponding value type class.

    For example, in the following example,

    \qml
Text {
    text: "Hello World!"
    y: Other.y
}
    \endqml

    The text property is being assigned a literal, and the y property a property
    binding.  To output the values assigned to the text and y properties in the
    above example from C++,

    \code
    QDeclarativeDomDocument document;
    QDeclarativeDomObject root = document.rootObject();

    QDeclarativeDomProperty text = root.property("text");
    if (text.value().isLiteral()) {
        QDeclarativeDomValueLiteral literal = text.value().toLiteral();
        qDebug() << literal.literal();
    }

    QDeclarativeDomProperty y = root.property("y");
    if (y.value().isBinding()) {
        QDeclarativeDomValueBinding binding = y.value().toBinding();
        qDebug() << binding.binding();
    }
    \endcode
*/

/*!
    Construct an invalid QDeclarativeDomValue.
*/
QDeclarativeDomValue::QDeclarativeDomValue()
   : d(new QDeclarativeDomValuePrivate)
{
}

/*!
    Create a copy of \a other QDeclarativeDomValue.
*/
QDeclarativeDomValue::QDeclarativeDomValue(const QDeclarativeDomValue &other)
   : d(other.d)
{
}

/*!
    Destroy the QDeclarativeDomValue
*/
QDeclarativeDomValue::~QDeclarativeDomValue()
{
}

/*!
    Assign \a other to this QDeclarativeDomValue.
*/
QDeclarativeDomValue &QDeclarativeDomValue::operator=(const QDeclarativeDomValue &other)
{
   d = other.d;
   return *this;
}

/*!
    \enum QDeclarativeDomValue::Type

    The type of the QDeclarativeDomValue node.

    \value Invalid The QDeclarativeDomValue is invalid.
    \value Literal The QDeclarativeDomValue is a literal value assignment.  Use QDeclarativeDomValue::toLiteral() to access the type instance.
    \value PropertyBinding The QDeclarativeDomValue is a property binding.  Use QDeclarativeDomValue::toBinding() to access the type instance.
    \value ValueSource The QDeclarativeDomValue is a property value source.  Use QDeclarativeDomValue::toValueSource() to access the type instance.
    \value ValueInterceptor The QDeclarativeDomValue is a property value interceptor.  Use QDeclarativeDomValue::toValueInterceptor() to access the type instance.
    \value Object The QDeclarativeDomValue is an object assignment.  Use QDeclarativeDomValue::toObject() to access the type instnace.
    \value List The QDeclarativeDomValue is a list of other values.  Use QDeclarativeDomValue::toList() to access the type instance.
*/

/*!
    Returns the type of this QDeclarativeDomValue.
*/
QDeclarativeDomValue::Type QDeclarativeDomValue::type() const
{
   if (d->property)
      if (QDeclarativeMetaType::isList(d->property->type) ||
            (d->property && (d->property->values.count() + d->property->onValues.count()) > 1)) {
         return List;
      }

   QDeclarativeParser::Value *value = d->value;
   if (!value && !d->property) {
      return Invalid;
   }

   switch (value->type) {
      case QDeclarativeParser::Value::Unknown:
         return Invalid;
      case QDeclarativeParser::Value::Literal:
         return Literal;
      case QDeclarativeParser::Value::PropertyBinding:
         return PropertyBinding;
      case QDeclarativeParser::Value::ValueSource:
         return ValueSource;
      case QDeclarativeParser::Value::ValueInterceptor:
         return ValueInterceptor;
      case QDeclarativeParser::Value::CreatedObject:
         return Object;
      case QDeclarativeParser::Value::SignalObject:
         return Invalid;
      case QDeclarativeParser::Value::SignalExpression:
         return Literal;
      case QDeclarativeParser::Value::Id:
         return Literal;
   }
   return Invalid;
}

/*!
    Returns true if this is an invalid value, otherwise false.
*/
bool QDeclarativeDomValue::isInvalid() const
{
   return type() == Invalid;
}

/*!
    Returns true if this is a literal value, otherwise false.
*/
bool QDeclarativeDomValue::isLiteral() const
{
   return type() == Literal;
}

/*!
    Returns true if this is a property binding value, otherwise false.
*/
bool QDeclarativeDomValue::isBinding() const
{
   return type() == PropertyBinding;
}

/*!
    Returns true if this is a value source value, otherwise false.
*/
bool QDeclarativeDomValue::isValueSource() const
{
   return type() == ValueSource;
}

/*!
    Returns true if this is a value interceptor value, otherwise false.
*/
bool QDeclarativeDomValue::isValueInterceptor() const
{
   return type() == ValueInterceptor;
}

/*!
    Returns true if this is an object value, otherwise false.
*/
bool QDeclarativeDomValue::isObject() const
{
   return type() == Object;
}

/*!
    Returns true if this is a list value, otherwise false.
*/
bool QDeclarativeDomValue::isList() const
{
   return type() == List;
}

/*!
    Returns a QDeclarativeDomValueLiteral if this value is a literal type, otherwise
    returns an invalid QDeclarativeDomValueLiteral.

    \sa QDeclarativeDomValue::type()
*/
QDeclarativeDomValueLiteral QDeclarativeDomValue::toLiteral() const
{
   QDeclarativeDomValueLiteral rv;
   if (type() == Literal) {
      rv.d->value = d->value;
      rv.d->value->addref();
   }
   return rv;
}

/*!
    Returns a QDeclarativeDomValueBinding if this value is a property binding type,
    otherwise returns an invalid QDeclarativeDomValueBinding.

    \sa QDeclarativeDomValue::type()
*/
QDeclarativeDomValueBinding QDeclarativeDomValue::toBinding() const
{
   QDeclarativeDomValueBinding rv;
   if (type() == PropertyBinding) {
      rv.d->value = d->value;
      rv.d->value->addref();
   }
   return rv;
}

/*!
    Returns a QDeclarativeDomValueValueSource if this value is a property value source
    type, otherwise returns an invalid QDeclarativeDomValueValueSource.

    \sa QDeclarativeDomValue::type()
*/
QDeclarativeDomValueValueSource QDeclarativeDomValue::toValueSource() const
{
   QDeclarativeDomValueValueSource rv;
   if (type() == ValueSource) {
      rv.d->value = d->value;
      rv.d->value->addref();
   }
   return rv;
}

/*!
    Returns a QDeclarativeDomValueValueInterceptor if this value is a property value interceptor
    type, otherwise returns an invalid QDeclarativeDomValueValueInterceptor.

    \sa QDeclarativeDomValue::type()
*/
QDeclarativeDomValueValueInterceptor QDeclarativeDomValue::toValueInterceptor() const
{
   QDeclarativeDomValueValueInterceptor rv;
   if (type() == ValueInterceptor) {
      rv.d->value = d->value;
      rv.d->value->addref();
   }
   return rv;
}

/*!
    Returns a QDeclarativeDomObject if this value is an object assignment type, otherwise
    returns an invalid QDeclarativeDomObject.

    \sa QDeclarativeDomValue::type()
*/
QDeclarativeDomObject QDeclarativeDomValue::toObject() const
{
   QDeclarativeDomObject rv;
   if (type() == Object) {
      rv.d->object = d->value->object;
      rv.d->object->addref();
   }
   return rv;
}

/*!
    Returns a QDeclarativeDomList if this value is a list type, otherwise returns an
    invalid QDeclarativeDomList.

    \sa QDeclarativeDomValue::type()
*/
QDeclarativeDomList QDeclarativeDomValue::toList() const
{
   QDeclarativeDomList rv;
   if (type() == List) {
      rv.d = d;
   }
   return rv;
}

/*!
    Returns the position in the input data where the property value startd, or -1
 if the value is invalid.
*/
int QDeclarativeDomValue::position() const
{
   if (type() == Invalid) {
      return -1;
   } else {
      return d->value->location.range.offset;
   }
}

/*!
    Returns the length in the input data from where the property value started u
pto the end of it, or -1 if the value is invalid.
*/
int QDeclarativeDomValue::length() const
{
   if (type() == Invalid) {
      return -1;
   } else {
      return d->value->location.range.length;
   }
}

/*!
    \class QDeclarativeDomList
    \internal
    \brief The QDeclarativeDomList class represents a list of values assigned to a QML property.

    Lists of values can be assigned to properties.  For example, the following
    example assigns multiple objects to Item's "children" property
    \qml
Item {
    children: [
        Text { },
        Rectangle { }
    ]
}
    \endqml

    Lists can also be implicitly created by assigning multiple
    \l {QDeclarativeDomValueValueSource}{value sources} or constants to a property.
    \qml
Item {
    x: 10
    x: NumberAnimation {
        running: false
        from: 0
        to: 100
    }
}
    \endqml
*/

/*!
    Construct an empty QDeclarativeDomList.
*/
QDeclarativeDomList::QDeclarativeDomList()
{
}

/*!
    Create a copy of \a other QDeclarativeDomList.
*/
QDeclarativeDomList::QDeclarativeDomList(const QDeclarativeDomList &other)
   : d(other.d)
{
}

/*!
    Destroy the QDeclarativeDomList.
*/
QDeclarativeDomList::~QDeclarativeDomList()
{
}

/*!
    Assign \a other to this QDeclarativeDomList.
*/
QDeclarativeDomList &QDeclarativeDomList::operator=(const QDeclarativeDomList &other)
{
   d = other.d;
   return *this;
}

/*!
    Returns the list of QDeclarativeDomValue's.
*/
QList<QDeclarativeDomValue> QDeclarativeDomList::values() const
{
   QList<QDeclarativeDomValue> rv;
   if (!d->property) {
      return rv;
   }

   for (int ii = 0; ii < d->property->values.count(); ++ii) {
      QDeclarativeDomValue v;
      v.d->value = d->property->values.at(ii);
      v.d->value->addref();
      rv << v;
   }

   for (int ii = 0; ii < d->property->onValues.count(); ++ii) {
      QDeclarativeDomValue v;
      v.d->value = d->property->onValues.at(ii);
      v.d->value->addref();
      rv << v;
   }

   return rv;
}

/*!
    Returns the position in the input data where the list started, or -1 if
 the property is invalid.
*/
int QDeclarativeDomList::position() const
{
   if (d && d->property) {
      return d->property->listValueRange.offset;
   } else {
      return -1;
   }
}

/*!
    Returns the length in the input data from where the list started upto
 the end of it, or 0 if the property is invalid.
*/
int QDeclarativeDomList::length() const
{
   if (d && d->property) {
      return d->property->listValueRange.length;
   } else {
      return -1;
   }
}

/*!
  Returns a list of positions of the commas in the QML file.
*/
QList<int> QDeclarativeDomList:: commaPositions() const
{
   if (d && d->property) {
      return d->property->listCommaPositions;
   } else {
      return QList<int>();
   }
}

/*!
    \class QDeclarativeDomComponent
    \internal
    \brief The QDeclarativeDomComponent class represents sub-component within a QML document.

    Sub-components are QDeclarativeComponents defined within a QML document.  The
    following example shows the definition of a sub-component with the id
    "listDelegate".

    \qml
Item {
    Component {
        id: listDelegate
        Text {
            text: modelData.text
        }
    }
}
    \endqml

    Like QDeclarativeDomDocument's, components contain a single root object.
*/

/*!
    Construct an empty QDeclarativeDomComponent.
*/
QDeclarativeDomComponent::QDeclarativeDomComponent()
{
}

/*!
    Create a copy of \a other QDeclarativeDomComponent.
*/
QDeclarativeDomComponent::QDeclarativeDomComponent(const QDeclarativeDomComponent &other)
   : QDeclarativeDomObject(other)
{
}

/*!
    Destroy the QDeclarativeDomComponent.
*/
QDeclarativeDomComponent::~QDeclarativeDomComponent()
{
}

/*!
    Assign \a other to this QDeclarativeDomComponent.
*/
QDeclarativeDomComponent &QDeclarativeDomComponent::operator=(const QDeclarativeDomComponent &other)
{
   static_cast<QDeclarativeDomObject &>(*this) = other;
   return *this;
}

/*!
    Returns the component's root object.

    In the example below, the root object is the "Text" object.
    \qml
Item {
    Component {
        id: listDelegate
        Text {
            text: modelData.text
        }
    }
}
    \endqml
*/
QDeclarativeDomObject QDeclarativeDomComponent::componentRoot() const
{
   QDeclarativeDomObject rv;
   if (d->object) {
      QDeclarativeParser::Object *obj = 0;
      if (d->object->defaultProperty &&
            d->object->defaultProperty->values.count() == 1 &&
            d->object->defaultProperty->values.at(0)->object) {
         obj = d->object->defaultProperty->values.at(0)->object;
      }

      if (obj) {
         rv.d->object = obj;
         rv.d->object->addref();
      }
   }

   return rv;
}

QDeclarativeDomImportPrivate::QDeclarativeDomImportPrivate()
   : type(File)
{
}

QDeclarativeDomImportPrivate::~QDeclarativeDomImportPrivate()
{
}

/*!
    \class QDeclarativeDomImport
    \internal
    \brief The QDeclarativeDomImport class represents an import statement.
*/

/*!
    Construct an empty QDeclarativeDomImport.
*/
QDeclarativeDomImport::QDeclarativeDomImport()
   : d(new QDeclarativeDomImportPrivate)
{
}

/*!
    Create a copy of \a other QDeclarativeDomImport.
*/
QDeclarativeDomImport::QDeclarativeDomImport(const QDeclarativeDomImport &other)
   : d(other.d)
{
}

/*!
    Destroy the QDeclarativeDomImport.
*/
QDeclarativeDomImport::~QDeclarativeDomImport()
{
}

/*!
    Assign \a other to this QDeclarativeDomImport.
*/
QDeclarativeDomImport &QDeclarativeDomImport::operator=(const QDeclarativeDomImport &other)
{
   d = other.d;
   return *this;
}

/*!
  Returns the type of the import.
  */
QDeclarativeDomImport::Type QDeclarativeDomImport::type() const
{
   return static_cast<QDeclarativeDomImport::Type>(d->type);
}

QString QDeclarativeDomImport::uri() const
{
   return d->uri;
}

/*!
  Returns the version specified by the import. An empty string if no version was specified.
  */
QString QDeclarativeDomImport::version() const
{
   return d->version;
}

/*!
  Returns the (optional) qualifier string (the token following the 'as' keyword) of the import.
  */
QString QDeclarativeDomImport::qualifier() const
{
   return d->qualifier;
}

QT_END_NAMESPACE
