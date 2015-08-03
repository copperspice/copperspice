/***********************************************************************
*
* Copyright (c) 2012-2015 Barbara Geller
* Copyright (c) 2012-2015 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QDECLARATIVETIMER_P_H
#define QDECLARATIVETIMER_P_H

#include <qdeclarative.h>
#include <QtCore/qobject.h>
#include <QtCore/qabstractanimation.h>
#include <qdeclarativeglobal_p.h>

QT_BEGIN_NAMESPACE

class QDeclarativeTimerPrivate;

class Q_DECLARATIVE_PRIVATE_EXPORT QDeclarativeTimer : public QObject, public QDeclarativeParserStatus
{
   DECL_CS_OBJECT(QDeclarativeTimer)

   Q_DECLARE_PRIVATE(QDeclarativeTimer)

   CS_INTERFACES(QDeclarativeParserStatus)
   CS_PROPERTY_READ(interval, interval)
   CS_PROPERTY_WRITE(interval, setInterval)
   CS_PROPERTY_NOTIFY(interval, intervalChanged)
   CS_PROPERTY_READ(running, isRunning)
   CS_PROPERTY_WRITE(running, setRunning)
   CS_PROPERTY_NOTIFY(running, runningChanged)
   CS_PROPERTY_READ(repeat, isRepeating)
   CS_PROPERTY_WRITE(repeat, setRepeating)
   CS_PROPERTY_NOTIFY(repeat, repeatChanged)
   CS_PROPERTY_READ(triggeredOnStart, triggeredOnStart)
   CS_PROPERTY_WRITE(triggeredOnStart, setTriggeredOnStart)
   CS_PROPERTY_NOTIFY(triggeredOnStart, triggeredOnStartChanged)
   CS_PROPERTY_READ(*parent, parent)
   CS_PROPERTY_CONSTANT(*parent)

 public:
   QDeclarativeTimer(QObject *parent = 0);

   void setInterval(int interval);
   int interval() const;

   bool isRunning() const;
   void setRunning(bool running);

   bool isRepeating() const;
   void setRepeating(bool repeating);

   bool triggeredOnStart() const;
   void setTriggeredOnStart(bool triggeredOnStart);

   CS_SLOT_1(Public, void start())
   CS_SLOT_2(start)
   CS_SLOT_1(Public, void stop())
   CS_SLOT_2(stop)
   CS_SLOT_1(Public, void restart())
   CS_SLOT_2(restart)

   CS_SIGNAL_1(Public, void triggered())
   CS_SIGNAL_2(triggered)
   CS_SIGNAL_1(Public, void runningChanged())
   CS_SIGNAL_2(runningChanged)
   CS_SIGNAL_1(Public, void intervalChanged())
   CS_SIGNAL_2(intervalChanged)
   CS_SIGNAL_1(Public, void repeatChanged())
   CS_SIGNAL_2(repeatChanged)
   CS_SIGNAL_1(Public, void triggeredOnStartChanged())
   CS_SIGNAL_2(triggeredOnStartChanged)

 protected:
   void classBegin();
   void componentComplete();

 private:
   void update();

   CS_SLOT_1(Private, void ticked())
   CS_SLOT_2(ticked)
   CS_SLOT_1(Private, void finished())
   CS_SLOT_2(finished)
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeTimer)

#endif
