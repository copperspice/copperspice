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

#ifndef QDECLARATIVECONTEXT_H
#define QDECLARATIVECONTEXT_H

#include <QtCore/qurl.h>
#include <QtCore/qobject.h>
#include <QtScript/qscriptvalue.h>
#include <QtCore/qmetatype.h>
#include <QtCore/qvariant.h>

QT_BEGIN_NAMESPACE

class QString;
class QDeclarativeEngine;
class QDeclarativeRefCount;
class QDeclarativeContextPrivate;
class QDeclarativeCompositeTypeData;
class QDeclarativeContextData;

class Q_DECLARATIVE_EXPORT QDeclarativeContext : public QObject
{
   DECL_CS_OBJECT(QDeclarativeContext)
   Q_DECLARE_PRIVATE(QDeclarativeContext)

 public:
   QDeclarativeContext(QDeclarativeEngine *parent, QObject *objParent = 0);
   QDeclarativeContext(QDeclarativeContext *parent, QObject *objParent = 0);
   virtual ~QDeclarativeContext();

   bool isValid() const;

   QDeclarativeEngine *engine() const;
   QDeclarativeContext *parentContext() const;

   QObject *contextObject() const;
   void setContextObject(QObject *);

   QVariant contextProperty(const QString &) const;
   void setContextProperty(const QString &, QObject *);
   void setContextProperty(const QString &, const QVariant &);

   QUrl resolvedUrl(const QUrl &);

   void setBaseUrl(const QUrl &);
   QUrl baseUrl() const;

 private:
   friend class QDeclarativeVME;
   friend class QDeclarativeEngine;
   friend class QDeclarativeEnginePrivate;
   friend class QDeclarativeExpression;
   friend class QDeclarativeExpressionPrivate;
   friend class QDeclarativeContextScriptClass;
   friend class QDeclarativeObjectScriptClass;
   friend class QDeclarativeComponent;
   friend class QDeclarativeComponentPrivate;
   friend class QDeclarativeScriptPrivate;
   friend class QDeclarativeBoundSignalProxy;
   friend class QDeclarativeContextData;

   QDeclarativeContext(QDeclarativeContextData *);
   QDeclarativeContext(QDeclarativeEngine *, bool);
   Q_DISABLE_COPY(QDeclarativeContext)
};
QT_END_NAMESPACE

Q_DECLARE_METATYPE(QList<QObject *>)

#endif // QDECLARATIVECONTEXT_H
