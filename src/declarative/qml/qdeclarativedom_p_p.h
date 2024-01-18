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

#ifndef QDECLARATIVEDOM_P_P_H
#define QDECLARATIVEDOM_P_P_H

#include <qglobal.h>
#include <qdeclarativeparser_p.h>

QT_BEGIN_NAMESPACE

class QDeclarativeDomDocumentPrivate : public QSharedData
{
 public:
   QDeclarativeDomDocumentPrivate();
   QDeclarativeDomDocumentPrivate(const QDeclarativeDomDocumentPrivate &o)
      : QSharedData(o) {
      qFatal("Not impl");
   }
   ~QDeclarativeDomDocumentPrivate();

   QList<QDeclarativeError> errors;
   QList<QDeclarativeDomImport> imports;
   QDeclarativeParser::Object *root;
   QList<int> automaticSemicolonOffsets;
};

class QDeclarativeDomObjectPrivate : public QSharedData
{
 public:
   QDeclarativeDomObjectPrivate();
   QDeclarativeDomObjectPrivate(const QDeclarativeDomObjectPrivate &o)
      : QSharedData(o) {
      qFatal("Not impl");
   }
   ~QDeclarativeDomObjectPrivate();

   typedef QList<QPair<QDeclarativeParser::Property *, QByteArray> > Properties;
   Properties properties() const;
   Properties properties(QDeclarativeParser::Property *) const;

   QDeclarativeParser::Object *object;
};

class QDeclarativeDomPropertyPrivate : public QSharedData
{
 public:
   QDeclarativeDomPropertyPrivate();
   QDeclarativeDomPropertyPrivate(const QDeclarativeDomPropertyPrivate &o)
      : QSharedData(o) {
      qFatal("Not impl");
   }
   ~QDeclarativeDomPropertyPrivate();

   QByteArray propertyName;
   QDeclarativeParser::Property *property;
};

class QDeclarativeDomDynamicPropertyPrivate : public QSharedData
{
 public:
   QDeclarativeDomDynamicPropertyPrivate();
   QDeclarativeDomDynamicPropertyPrivate(const QDeclarativeDomDynamicPropertyPrivate &o)
      : QSharedData(o) {
      qFatal("Not impl");
   }
   ~QDeclarativeDomDynamicPropertyPrivate();

   bool valid;
   QDeclarativeParser::Object::DynamicProperty property;
};

class QDeclarativeDomValuePrivate : public QSharedData
{
 public:
   QDeclarativeDomValuePrivate();
   QDeclarativeDomValuePrivate(const QDeclarativeDomValuePrivate &o)
      : QSharedData(o) {
      qFatal("Not impl");
   }
   ~QDeclarativeDomValuePrivate();

   QDeclarativeParser::Property *property;
   QDeclarativeParser::Value *value;
};

class QDeclarativeDomBasicValuePrivate : public QSharedData
{
 public:
   QDeclarativeDomBasicValuePrivate();
   QDeclarativeDomBasicValuePrivate(const QDeclarativeDomBasicValuePrivate &o)
      : QSharedData(o) {
      qFatal("Not impl");
   }
   ~QDeclarativeDomBasicValuePrivate();

   QDeclarativeParser::Value *value;
};

class QDeclarativeDomImportPrivate : public QSharedData
{
 public:
   QDeclarativeDomImportPrivate();
   QDeclarativeDomImportPrivate(const QDeclarativeDomImportPrivate &o)
      : QSharedData(o) {
      qFatal("Not impl");
   }
   ~QDeclarativeDomImportPrivate();

   enum Type { Library, File };

   Type type;
   QString uri;
   QString version;
   QString qualifier;
};

QT_END_NAMESPACE

#endif // QDECLARATIVEDOM_P_P_H

