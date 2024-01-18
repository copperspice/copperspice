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

#ifndef QFINALSTATE_H
#define QFINALSTATE_H

#include <qabstractstate.h>

#ifndef QT_NO_STATEMACHINE

class QFinalStatePrivate;

class Q_CORE_EXPORT QFinalState : public QAbstractState
{
   CORE_CS_OBJECT(QFinalState)

 public:
   QFinalState(QState *parent = nullptr);

   QFinalState(const QFinalState &other) = delete;
   QFinalState &operator=(const QFinalState &other) = delete;

   ~QFinalState();

 protected:
   void onEntry(QEvent *event) override;
   void onExit(QEvent *event) override;

   bool event(QEvent *event) override;

 private:
   Q_DECLARE_PRIVATE(QFinalState)
};

#endif //QT_NO_STATEMACHINE

#endif
