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

#ifndef QDECLARATIVECOMPONENT_P_H
#define QDECLARATIVECOMPONENT_P_H

#include <qdeclarativecomponent.h>
#include <qdeclarativeengine_p.h>
#include <qdeclarativetypeloader_p.h>
#include <qbitfield_p.h>
#include <qdeclarativeerror.h>
#include <qdeclarative.h>

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QList>

QT_BEGIN_NAMESPACE

class QDeclarativeComponent;
class QDeclarativeEngine;
class QDeclarativeCompiledData;

class QDeclarativeComponentAttached;
class QDeclarativeComponentPrivate : public QDeclarativeTypeData::TypeDataCallback
{
   Q_DECLARE_PUBLIC(QDeclarativeComponent)

 public:
   QDeclarativeComponentPrivate() : typeData(0), progress(0.), start(-1), count(-1), cc(0), engine(0),
      creationContext(0) {}

   QObject *beginCreate(QDeclarativeContextData *, const QBitField &);
   void completeCreate();

   QDeclarativeTypeData *typeData;
   virtual void typeDataReady(QDeclarativeTypeData *);
   virtual void typeDataProgress(QDeclarativeTypeData *, qreal);

   void fromTypeData(QDeclarativeTypeData *data);

   QUrl url;
   qreal progress;

   int start;
   int count;
   QDeclarativeCompiledData *cc;

   struct ConstructionState {
      ConstructionState() : componentAttached(0), completePending(false) {}
      QList<QDeclarativeEnginePrivate::SimpleList<QDeclarativeAbstractBinding> > bindValues;
      QList<QDeclarativeEnginePrivate::SimpleList<QDeclarativeParserStatus> > parserStatus;
      QList<QPair<QDeclarativeGuard<QObject>, int> > finalizedParserStatus;
      QDeclarativeComponentAttached *componentAttached;
      QList<QDeclarativeError> errors;
      bool completePending;
   };
   ConstructionState state;

   static QObject *begin(QDeclarativeContextData *parentContext, QDeclarativeContextData *componentCreationContext,
                         QDeclarativeCompiledData *component, int start, int count,
                         ConstructionState *state, QList<QDeclarativeError> *errors,
                         const QBitField &bindings = QBitField());
   static void beginDeferred(QDeclarativeEnginePrivate *enginePriv, QObject *object,
                             ConstructionState *state);
   static void complete(QDeclarativeEnginePrivate *enginePriv, ConstructionState *state);

   QScriptValue createObject(QObject *publicParent, const QScriptValue valuemap);

   QDeclarativeEngine *engine;
   QDeclarativeGuardedContextData creationContext;

   void clear();

   static QDeclarativeComponentPrivate *get(QDeclarativeComponent *c) {
      return static_cast<QDeclarativeComponentPrivate *>(QObjectPrivate::get(c));
   }
};

class QDeclarativeComponentAttached : public QObject
{
   DECL_CS_OBJECT(QDeclarativeComponentAttached)

 public:
   QDeclarativeComponentAttached(QObject *parent = nullptr);
   ~QDeclarativeComponentAttached();

   void add(QDeclarativeComponentAttached **a) {
      prev = a;
      next = *a;
      *a = this;
      if (next) {
         next->prev = &next;
      }
   }
   void rem() {
      if (next) {
         next->prev = prev;
      }
      *prev = next;
      next = 0;
      prev = 0;
   }
   QDeclarativeComponentAttached **prev;
   QDeclarativeComponentAttached *next;

 public:
   DECL_CS_SIGNAL_1(Public, void completed())
   DECL_CS_SIGNAL_2(completed)
   DECL_CS_SIGNAL_1(Public, void destruction())
   DECL_CS_SIGNAL_2(destruction)

 private:
   friend class QDeclarativeContextData;
   friend class QDeclarativeComponentPrivate;
};

QT_END_NAMESPACE

#endif // QDECLARATIVECOMPONENT_P_H
