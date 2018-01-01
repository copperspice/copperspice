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

#ifndef QHISTORYSTATE_H
#define QHISTORYSTATE_H

#include <QtCore/qabstractstate.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_STATEMACHINE

class QHistoryStatePrivate;

class Q_CORE_EXPORT QHistoryState : public QAbstractState
{
   CORE_CS_OBJECT(QHistoryState)

   CORE_CS_PROPERTY_READ(defaultState, defaultState)
   CORE_CS_PROPERTY_WRITE(defaultState, setDefaultState)
   CORE_CS_PROPERTY_READ(historyType, historyType)
   CORE_CS_PROPERTY_WRITE(historyType, setHistoryType)
   
 public:
   enum HistoryType {
      ShallowHistory,
      DeepHistory
   };

   CORE_CS_ENUM(HistoryType)

   QHistoryState(QState *parent = 0);
   QHistoryState(HistoryType type, QState *parent = 0);
   ~QHistoryState();

   QAbstractState *defaultState() const;
   void setDefaultState(QAbstractState *state);

   HistoryType historyType() const;
   void setHistoryType(HistoryType type);

 protected:
   void onEntry(QEvent *event) override;
   void onExit(QEvent *event) override;

   bool event(QEvent *e) override;

 private:
   Q_DISABLE_COPY(QHistoryState)
   Q_DECLARE_PRIVATE(QHistoryState)
};

#endif //QT_NO_STATEMACHINE

QT_END_NAMESPACE

#endif
