/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QSTATE_P_H
#define QSTATE_P_H

#include <qabstractstate_p.h>
#include <QtCore/qlist.h>
#include <QtCore/qbytearray.h>
#include <QtCore/qvariant.h>

QT_BEGIN_NAMESPACE

struct QPropertyAssignment {
   QPropertyAssignment()
      : object(0), explicitlySet(true) {}

   QPropertyAssignment(QObject *o, const QByteArray &n,
                       const QVariant &v, bool es = true)
      : object(o), propertyName(n), value(v), explicitlySet(es) {
   }

   QObject *object;
   QByteArray propertyName;
   QVariant value;
   bool explicitlySet;
};

class QAbstractTransition;
class QHistoryState;
class QState;

class QStatePrivate : public QAbstractStatePrivate
{
   Q_DECLARE_PUBLIC(QState)

 public:
   QStatePrivate();
   ~QStatePrivate();

   static QStatePrivate *get(QState *q) {
      return q ? q->d_func() : 0;
   }
   static const QStatePrivate *get(const QState *q) {
      return q ? q->d_func() : 0;
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
   mutable QList<QAbstractState *> childStatesList;
   mutable bool transitionsListNeedsRefresh;
   mutable QList<QAbstractTransition *> transitionsList;

   QList<QPropertyAssignment> propertyAssignments;
};

QT_END_NAMESPACE

#endif
