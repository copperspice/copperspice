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

#ifndef QDECLARATIVELISTMODELWORKERAGENT_P_H
#define QDECLARATIVELISTMODELWORKERAGENT_P_H

#include "qdeclarative.h"
#include <QtScript/qscriptvalue.h>
#include <QtGui/qevent.h>
#include <QMutex>
#include <QWaitCondition>

QT_BEGIN_NAMESPACE

class QDeclarativeListModel;
class FlatListScriptClass;

class QDeclarativeListModelWorkerAgent : public QObject
{
   DECL_CS_OBJECT(QDeclarativeListModelWorkerAgent)
   DECL_CS_PROPERTY_READ(count, count)

 public:
   QDeclarativeListModelWorkerAgent(QDeclarativeListModel *);
   ~QDeclarativeListModelWorkerAgent();

   void setScriptEngine(QScriptEngine *eng);
   QScriptEngine *scriptEngine() const;

   void addref();
   void release();

   int count() const;

   Q_INVOKABLE void clear();
   Q_INVOKABLE void remove(int index);
   Q_INVOKABLE void append(const QScriptValue &);
   Q_INVOKABLE void insert(int index, const QScriptValue &);
   Q_INVOKABLE QScriptValue get(int index) const;
   Q_INVOKABLE void set(int index, const QScriptValue &);
   Q_INVOKABLE void setProperty(int index, const QString &property, const QVariant &value);
   Q_INVOKABLE void move(int from, int to, int count);
   Q_INVOKABLE void sync();

   struct VariantRef {
      VariantRef() : a(0) {}
      VariantRef(const VariantRef &r) : a(r.a) {
         if (a) {
            a->addref();
         }
      }
      VariantRef(QDeclarativeListModelWorkerAgent *_a) : a(_a) {
         if (a) {
            a->addref();
         }
      }
      ~VariantRef() {
         if (a) {
            a->release();
         }
      }

      VariantRef &operator=(const VariantRef &o) {
         if (o.a) {
            o.a->addref();
         }
         if (a) {
            a->release();
         }
         a = o.a;
         return *this;
      }

      QDeclarativeListModelWorkerAgent *a;
   };
 protected:
   virtual bool event(QEvent *);

 private:
   friend class QDeclarativeWorkerScriptEnginePrivate;
   friend class FlatListScriptClass;
   QScriptEngine *m_engine;

   struct Change {
      enum { Inserted, Removed, Moved, Changed } type;
      int index; // Inserted/Removed/Moved/Changed
      int count; // Inserted/Removed/Moved/Changed
      int to;    // Moved
      QList<int> roles;
   };

   struct Data {
      QList<Change> changes;

      void clearChange();
      void insertChange(int index, int count);
      void removeChange(int index, int count);
      void moveChange(int index, int count, int to);
      void changedChange(int index, int count, const QList<int> &roles);
   };
   Data data;

   struct Sync : public QEvent {
      Sync() : QEvent(QEvent::User) {}
      Data data;
      QDeclarativeListModel *list;
   };

   void changedData(int index, int count, const QList<int> &roles);

   QAtomicInt m_ref;
   QDeclarativeListModel *m_orig;
   QDeclarativeListModel *m_copy;
   QMutex mutex;
   QWaitCondition syncDone;
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QDeclarativeListModelWorkerAgent::VariantRef)

#endif // QDECLARATIVEWORKERSCRIPT_P_H

