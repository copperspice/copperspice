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

#ifndef QDECLARATIVEDOM_P_H
#define QDECLARATIVEDOM_P_H

#include <qdeclarativeerror.h>
#include <QtCore/qlist.h>
#include <QtCore/qshareddata.h>
#include <qdeclarativeglobal_p.h>

QT_BEGIN_NAMESPACE

class QString;
class QByteArray;
class QDeclarativeDomObject;
class QDeclarativeDomList;
class QDeclarativeDomValue;
class QDeclarativeEngine;
class QDeclarativeDomComponent;
class QDeclarativeDomImport;
class QIODevice;

class QDeclarativeDomDocumentPrivate;

class Q_DECLARATIVE_PRIVATE_EXPORT QDeclarativeDomDocument
{
 public:
   QDeclarativeDomDocument();
   QDeclarativeDomDocument(const QDeclarativeDomDocument &);
   ~QDeclarativeDomDocument();
   QDeclarativeDomDocument &operator=(const QDeclarativeDomDocument &);

   QList<QDeclarativeDomImport> imports() const;

   QList<QDeclarativeError> errors() const;
   bool load(QDeclarativeEngine *, const QByteArray &, const QUrl & = QUrl());

   QDeclarativeDomObject rootObject() const;

 private:
   QSharedDataPointer<QDeclarativeDomDocumentPrivate> d;
};

class QDeclarativeDomPropertyPrivate;
class Q_DECLARATIVE_PRIVATE_EXPORT QDeclarativeDomProperty
{
 public:
   QDeclarativeDomProperty();
   QDeclarativeDomProperty(const QDeclarativeDomProperty &);
   ~QDeclarativeDomProperty();
   QDeclarativeDomProperty &operator=(const QDeclarativeDomProperty &);

   bool isValid() const;

   QByteArray propertyName() const;
   QList<QByteArray> propertyNameParts() const;

   bool isDefaultProperty() const;

   QDeclarativeDomValue value() const;

   int position() const;
   int length() const;

 private:
   friend class QDeclarativeDomObject;
   friend class QDeclarativeDomDynamicProperty;
   QSharedDataPointer<QDeclarativeDomPropertyPrivate> d;
};

class QDeclarativeDomDynamicPropertyPrivate;
class Q_DECLARATIVE_PRIVATE_EXPORT QDeclarativeDomDynamicProperty
{
 public:
   QDeclarativeDomDynamicProperty();
   QDeclarativeDomDynamicProperty(const QDeclarativeDomDynamicProperty &);
   ~QDeclarativeDomDynamicProperty();
   QDeclarativeDomDynamicProperty &operator=(const QDeclarativeDomDynamicProperty &);

   bool isValid() const;

   QByteArray propertyName() const;
   int propertyType() const;
   QByteArray propertyTypeName() const;

   bool isDefaultProperty() const;
   QDeclarativeDomProperty defaultValue() const;

   bool isAlias() const;

   int position() const;
   int length() const;

 private:
   friend class QDeclarativeDomObject;
   QSharedDataPointer<QDeclarativeDomDynamicPropertyPrivate> d;
};

class QDeclarativeDomObjectPrivate;
class Q_DECLARATIVE_PRIVATE_EXPORT QDeclarativeDomObject
{
 public:
   QDeclarativeDomObject();
   QDeclarativeDomObject(const QDeclarativeDomObject &);
   ~QDeclarativeDomObject();
   QDeclarativeDomObject &operator=(const QDeclarativeDomObject &);

   bool isValid() const;

   QByteArray objectType() const;
   QByteArray objectClassName() const;

   int objectTypeMajorVersion() const;
   int objectTypeMinorVersion() const;

   QString objectId() const;

   QList<QDeclarativeDomProperty> properties() const;
   QDeclarativeDomProperty property(const QByteArray &) const;

   QList<QDeclarativeDomDynamicProperty> dynamicProperties() const;
   QDeclarativeDomDynamicProperty dynamicProperty(const QByteArray &) const;

   bool isCustomType() const;
   QByteArray customTypeData() const;

   bool isComponent() const;
   QDeclarativeDomComponent toComponent() const;

   int position() const;
   int length() const;

   QUrl url() const;
 private:
   friend class QDeclarativeDomDocument;
   friend class QDeclarativeDomComponent;
   friend class QDeclarativeDomValue;
   friend class QDeclarativeDomValueValueSource;
   friend class QDeclarativeDomValueValueInterceptor;
   QSharedDataPointer<QDeclarativeDomObjectPrivate> d;
};

class QDeclarativeDomValuePrivate;
class QDeclarativeDomBasicValuePrivate;
class Q_DECLARATIVE_PRIVATE_EXPORT QDeclarativeDomValueLiteral
{
 public:
   QDeclarativeDomValueLiteral();
   QDeclarativeDomValueLiteral(const QDeclarativeDomValueLiteral &);
   ~QDeclarativeDomValueLiteral();
   QDeclarativeDomValueLiteral &operator=(const QDeclarativeDomValueLiteral &);

   QString literal() const;

 private:
   friend class QDeclarativeDomValue;
   QSharedDataPointer<QDeclarativeDomBasicValuePrivate> d;
};

class Q_DECLARATIVE_PRIVATE_EXPORT QDeclarativeDomValueBinding
{
 public:
   QDeclarativeDomValueBinding();
   QDeclarativeDomValueBinding(const QDeclarativeDomValueBinding &);
   ~QDeclarativeDomValueBinding();
   QDeclarativeDomValueBinding &operator=(const QDeclarativeDomValueBinding &);

   QString binding() const;

 private:
   friend class QDeclarativeDomValue;
   QSharedDataPointer<QDeclarativeDomBasicValuePrivate> d;
};

class Q_DECLARATIVE_PRIVATE_EXPORT QDeclarativeDomValueValueSource
{
 public:
   QDeclarativeDomValueValueSource();
   QDeclarativeDomValueValueSource(const QDeclarativeDomValueValueSource &);
   ~QDeclarativeDomValueValueSource();
   QDeclarativeDomValueValueSource &operator=(const QDeclarativeDomValueValueSource &);

   QDeclarativeDomObject object() const;

 private:
   friend class QDeclarativeDomValue;
   QSharedDataPointer<QDeclarativeDomBasicValuePrivate> d;
};

class Q_DECLARATIVE_PRIVATE_EXPORT QDeclarativeDomValueValueInterceptor
{
 public:
   QDeclarativeDomValueValueInterceptor();
   QDeclarativeDomValueValueInterceptor(const QDeclarativeDomValueValueInterceptor &);
   ~QDeclarativeDomValueValueInterceptor();
   QDeclarativeDomValueValueInterceptor &operator=(const QDeclarativeDomValueValueInterceptor &);

   QDeclarativeDomObject object() const;

 private:
   friend class QDeclarativeDomValue;
   QSharedDataPointer<QDeclarativeDomBasicValuePrivate> d;
};


class Q_DECLARATIVE_PRIVATE_EXPORT QDeclarativeDomComponent : public QDeclarativeDomObject
{
 public:
   QDeclarativeDomComponent();
   QDeclarativeDomComponent(const QDeclarativeDomComponent &);
   ~QDeclarativeDomComponent();
   QDeclarativeDomComponent &operator=(const QDeclarativeDomComponent &);

   QDeclarativeDomObject componentRoot() const;
};

class Q_DECLARATIVE_PRIVATE_EXPORT QDeclarativeDomValue
{
 public:
   enum Type {
      Invalid,
      Literal,
      PropertyBinding,
      ValueSource,
      ValueInterceptor,
      Object,
      List
   };

   QDeclarativeDomValue();
   QDeclarativeDomValue(const QDeclarativeDomValue &);
   ~QDeclarativeDomValue();
   QDeclarativeDomValue &operator=(const QDeclarativeDomValue &);

   Type type() const;

   bool isInvalid() const;
   bool isLiteral() const;
   bool isBinding() const;
   bool isValueSource() const;
   bool isValueInterceptor() const;
   bool isObject() const;
   bool isList() const;

   QDeclarativeDomValueLiteral toLiteral() const;
   QDeclarativeDomValueBinding toBinding() const;
   QDeclarativeDomValueValueSource toValueSource() const;
   QDeclarativeDomValueValueInterceptor toValueInterceptor() const;
   QDeclarativeDomObject toObject() const;
   QDeclarativeDomList toList() const;

   int position() const;
   int length() const;

 private:
   friend class QDeclarativeDomProperty;
   friend class QDeclarativeDomList;
   QSharedDataPointer<QDeclarativeDomValuePrivate> d;
};

class Q_DECLARATIVE_PRIVATE_EXPORT QDeclarativeDomList
{
 public:
   QDeclarativeDomList();
   QDeclarativeDomList(const QDeclarativeDomList &);
   ~QDeclarativeDomList();
   QDeclarativeDomList &operator=(const QDeclarativeDomList &);

   QList<QDeclarativeDomValue> values() const;

   int position() const;
   int length() const;

   QList<int> commaPositions() const;

 private:
   friend class QDeclarativeDomValue;
   QSharedDataPointer<QDeclarativeDomValuePrivate> d;
};

class QDeclarativeDomImportPrivate;
class Q_DECLARATIVE_PRIVATE_EXPORT QDeclarativeDomImport
{
 public:
   enum Type { Library, File };

   QDeclarativeDomImport();
   QDeclarativeDomImport(const QDeclarativeDomImport &);
   ~QDeclarativeDomImport();
   QDeclarativeDomImport &operator=(const QDeclarativeDomImport &);

   Type type() const;
   QString uri() const;
   QString version() const;
   QString qualifier() const;

 private:
   friend class QDeclarativeDomDocument;
   QSharedDataPointer<QDeclarativeDomImportPrivate> d;
};

QT_END_NAMESPACE

#endif // QDECLARATIVEDOM_P_H
