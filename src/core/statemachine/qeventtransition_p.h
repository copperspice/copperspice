/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#ifndef QEVENTTRANSITION_P_H
#define QEVENTTRANSITION_P_H

#include <qabstracttransition_p.h>

QT_BEGIN_NAMESPACE

class QEventTransition;

class Q_CORE_EXPORT QEventTransitionPrivate : public QAbstractTransitionPrivate
{
   Q_DECLARE_PUBLIC(QEventTransition)

 public:
   QEventTransitionPrivate();

   static QEventTransitionPrivate *get(QEventTransition *q);

   void unregister();
   void maybeRegister();

   bool registered;
   QObject *object;
   QEvent::Type eventType;
};

QT_END_NAMESPACE

#endif
