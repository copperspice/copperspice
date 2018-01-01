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

#ifndef QSIGNALEVENTGENERATOR_P_H
#define QSIGNALEVENTGENERATOR_P_H

#include <qobject.h>

QT_BEGIN_NAMESPACE

class QStateMachine;

class QSignalEventGenerator : public QObject
{
   CORE_CS_OBJECT(QSignalEventGenerator)

 public:
   QSignalEventGenerator(QStateMachine *parent);

   // BROOM (on hold, statemachine)
   CORE_CS_SLOT_1(Public, void execute())     // const TeaCupAbstract &data))
   CORE_CS_SLOT_2(execute)

 private:
   Q_DISABLE_COPY(QSignalEventGenerator)
};

QT_END_NAMESPACE

#endif
