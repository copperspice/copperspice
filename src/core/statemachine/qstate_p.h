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

#ifndef QSTATE_P_H
#define QSTATE_P_H

#include <qstate.h>
#include <qlist.h>
#include <qpointer.h>
#include <qvariant.h>

#include <qabstractstate_p.h>

class QAbstractTransition;
class QHistoryState;
class QState;

#ifndef QT_NO_STATEMACHINE

struct QPropertyAssignment {
   QPropertyAssignment()
      : object(nullptr), explicitlySet(true)
   { }

   QPropertyAssignment(QObject *o, const QString &n, const QVariant &v, bool es = true)
      : object(o), propertyName(n), value(v), explicitlySet(es)
   { }

   bool objectDeleted() const {
      return !object;
   }

   void write() const {
      Q_ASSERT(object != nullptr);
      object->setProperty(propertyName, value);
   }

   bool hasTarget(QObject *o, const QString &pn) const {
      return object == o && propertyName == pn;
   }

   QPointer<QObject> object;
   QString propertyName;
   QVariant value;
   bool explicitlySet;
};

class Q_CORE_EXPORT QStatePrivate : public QAbstractStatePrivate
{
   Q_DECLARE_PUBLIC(QState)

 public:
   QStatePrivate();
   ~QStatePrivate();

   static QStatePrivate *get(QState *q) {
      return q ? q->d_func() : nullptr;
   }

   static const QStatePrivate *get(const QState *q) {
      return q ? q->d_func() : nullptr;
   }

   QList<QAbstractState *> childStates() const;
   QList<QHistoryState *> historyStates() const;
   QList<QAbstractTransition *> transitions() const;

   void emitFinished();
   void emitPropertiesAssigned();

   QAbstractState *errorState;
   QAbstractState *initialState;
   QState::ChildMode childMode;

   mutable bool childStatesListNeedsRefresh;
   mutable bool transitionsListNeedsRefresh;
   mutable QList<QAbstractState *> childStatesList;
   mutable QList<QAbstractTransition *> transitionsList;

   QVector<QPropertyAssignment> propertyAssignments;
};

#endif // QT_NO_STATEMACHINE

#endif
