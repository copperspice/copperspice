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

#ifndef QDECLARATIVECUSTOMPARSER_P_H
#define QDECLARATIVECUSTOMPARSER_P_H

#include <qdeclarativemetatype_p.h>
#include <qdeclarativeerror.h>
#include <qdeclarativeparser_p.h>
#include <qdeclarativebinding_p.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qxmlstream.h>

QT_BEGIN_NAMESPACE

class QDeclarativeCompiler;

class QDeclarativeCustomParserPropertyPrivate;
class Q_DECLARATIVE_EXPORT QDeclarativeCustomParserProperty
{
 public:
   QDeclarativeCustomParserProperty();
   QDeclarativeCustomParserProperty(const QDeclarativeCustomParserProperty &);
   QDeclarativeCustomParserProperty &operator=(const QDeclarativeCustomParserProperty &);
   ~QDeclarativeCustomParserProperty();

   QByteArray name() const;
   QDeclarativeParser::Location location() const;

   bool isList() const;
   // Will be one of QDeclarativeParser::Variant, QDeclarativeCustomParserProperty or
   // QDeclarativeCustomParserNode
   QList<QVariant> assignedValues() const;

 private:
   friend class QDeclarativeCustomParserNodePrivate;
   friend class QDeclarativeCustomParserPropertyPrivate;
   QDeclarativeCustomParserPropertyPrivate *d;
};

class QDeclarativeCustomParserNodePrivate;
class Q_DECLARATIVE_EXPORT QDeclarativeCustomParserNode
{
 public:
   QDeclarativeCustomParserNode();
   QDeclarativeCustomParserNode(const QDeclarativeCustomParserNode &);
   QDeclarativeCustomParserNode &operator=(const QDeclarativeCustomParserNode &);
   ~QDeclarativeCustomParserNode();

   QByteArray name() const;
   QDeclarativeParser::Location location() const;

   QList<QDeclarativeCustomParserProperty> properties() const;

 private:
   friend class QDeclarativeCustomParserNodePrivate;
   QDeclarativeCustomParserNodePrivate *d;
};

class Q_DECLARATIVE_EXPORT QDeclarativeCustomParser
{
 public:
   enum Flag {
      NoFlag                    = 0x00000000,
      AcceptsAttachedProperties = 0x00000001
   };
   using Flags = QFlags<Flag>;

   QDeclarativeCustomParser() : compiler(0), object(0), m_flags(NoFlag) {}
   QDeclarativeCustomParser(Flags f) : compiler(0), object(0), m_flags(f) {}
   virtual ~QDeclarativeCustomParser() {}

   void clearErrors();
   Flags flags() const {
      return m_flags;
   }

   virtual QByteArray compile(const QList<QDeclarativeCustomParserProperty> &) = 0;
   virtual void setCustomData(QObject *, const QByteArray &) = 0;

   QList<QDeclarativeError> errors() const {
      return exceptions;
   }

 protected:
   void error(const QString &description);
   void error(const QDeclarativeCustomParserProperty &, const QString &description);
   void error(const QDeclarativeCustomParserNode &, const QString &description);

   int evaluateEnum(const QByteArray &) const;

   const QMetaObject *resolveType(const QByteArray &) const;

   QDeclarativeBinding::Identifier rewriteBinding(const QString &, const QByteArray &);

 private:
   QList<QDeclarativeError> exceptions;
   QDeclarativeCompiler *compiler;
   QDeclarativeParser::Object *object;
   Flags m_flags;
   friend class QDeclarativeCompiler;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(QDeclarativeCustomParser::Flags);

#if 0
#define QML_REGISTER_CUSTOM_TYPE(URI, VERSION_MAJ, VERSION_MIN, NAME, TYPE, CUSTOMTYPE) \
            qmlRegisterCustomType<TYPE>(#URI, VERSION_MAJ, VERSION_MIN, #NAME, #TYPE, new CUSTOMTYPE)
#endif

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QDeclarativeCustomParserProperty)
Q_DECLARE_METATYPE(QDeclarativeCustomParserNode)

#endif
