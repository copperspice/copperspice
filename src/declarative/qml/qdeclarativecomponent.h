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

#ifndef QDECLARATIVECOMPONENT_H
#define QDECLARATIVECOMPONENT_H

#include <QtDeclarative/qdeclarative.h>
#include <QtDeclarative/qdeclarativeerror.h>
#include <QtCore/qobject.h>
#include <QtCore/qstring.h>
#include <QtScript/qscriptvalue.h>

QT_BEGIN_NAMESPACE

class QDeclarativeCompiledData;
class QByteArray;
class QDeclarativeComponentPrivate;
class QDeclarativeEngine;
class QDeclarativeComponentAttached;

class Q_DECLARATIVE_EXPORT QDeclarativeComponent : public QObject
{
   DECL_CS_OBJECT(QDeclarativeComponent)
   Q_DECLARE_PRIVATE(QDeclarativeComponent)

   DECL_CS_PROPERTY_READ(progress, progress)
   DECL_CS_PROPERTY_NOTIFY(progress, progressChanged)
   DECL_CS_PROPERTY_READ(status, status)
   DECL_CS_PROPERTY_NOTIFY(status, statusChanged)
   DECL_CS_PROPERTY_READ(url, url)
   DECL_CS_PROPERTY_CONSTANT(url)

 public:
   QDeclarativeComponent(QObject *parent = nullptr);
   QDeclarativeComponent(QDeclarativeEngine *, QObject *parent = nullptr);
   QDeclarativeComponent(QDeclarativeEngine *, const QString &fileName, QObject *parent = nullptr);
   QDeclarativeComponent(QDeclarativeEngine *, const QUrl &url, QObject *parent = nullptr);
   virtual ~QDeclarativeComponent();

   CS_ENUM(Status)
   enum Status { Null, Ready, Loading, Error };
   Status status() const;

   bool isNull() const;
   bool isReady() const;
   bool isError() const;
   bool isLoading() const;

   QList<QDeclarativeError> errors() const;

   DECL_CS_INVOKABLE_METHOD_1(Public, QString errorString() const)
   DECL_CS_INVOKABLE_METHOD_2(errorString)

   qreal progress() const;

   QUrl url() const;

   virtual QObject *create(QDeclarativeContext *context = 0);
   virtual QObject *beginCreate(QDeclarativeContext *);
   virtual void completeCreate();

   void loadUrl(const QUrl &url);
   void setData(const QByteArray &, const QUrl &baseUrl);

   QDeclarativeContext *creationContext() const;

   static QDeclarativeComponentAttached *qmlAttachedProperties(QObject *);

   DECL_CS_SIGNAL_1(Public, void statusChanged(QDeclarativeComponent::Status un_named_arg1))
   DECL_CS_SIGNAL_2(statusChanged, un_named_arg1)

   DECL_CS_SIGNAL_1(Public, void progressChanged(qreal un_named_arg1))
   DECL_CS_SIGNAL_2(progressChanged, un_named_arg1)

 protected:
   QDeclarativeComponent(QDeclarativeComponentPrivate &dd, QObject *parent);

   CS_INVOKABLE_METHOD_1(Protected, QScriptValue createObject(QObject *parent))
   CS_INVOKABLE_METHOD_OVERLOAD(createObject)

   CS_INVOKABLE_METHOD_1(Protected, createObject(QObject *parent, const QScriptValue &valuemap))
   CS_INVOKABLE_METHOD_OVERLOAD(createObject)
   CS_REVISION_OVERLOAD(createObject, 1, (QObject *, const QScriptValue &))

 private:
   QDeclarativeComponent(QDeclarativeEngine *, QDeclarativeCompiledData *, int, int, QObject *parent);

   Q_DISABLE_COPY(QDeclarativeComponent)
   friend class QDeclarativeVME;
   friend class QDeclarativeCompositeTypeData;
   friend class QDeclarativeTypeData;
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QDeclarativeComponent::Status)
QML_DECLARE_TYPE(QDeclarativeComponent)
QML_DECLARE_TYPEINFO(QDeclarativeComponent, QML_HAS_ATTACHED_PROPERTIES)

#endif // QDECLARATIVECOMPONENT_H
