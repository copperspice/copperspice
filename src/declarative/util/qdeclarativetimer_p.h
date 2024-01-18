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
   DECL_CS_PROPERTY_READ(interval, interval)
   DECL_CS_PROPERTY_WRITE(interval, setInterval)
   DECL_CS_PROPERTY_NOTIFY(interval, intervalChanged)
   DECL_CS_PROPERTY_READ(running, isRunning)
   DECL_CS_PROPERTY_WRITE(running, setRunning)
   DECL_CS_PROPERTY_NOTIFY(running, runningChanged)
   DECL_CS_PROPERTY_READ(repeat, isRepeating)
   DECL_CS_PROPERTY_WRITE(repeat, setRepeating)
   DECL_CS_PROPERTY_NOTIFY(repeat, repeatChanged)
   DECL_CS_PROPERTY_READ(triggeredOnStart, triggeredOnStart)
   DECL_CS_PROPERTY_WRITE(triggeredOnStart, setTriggeredOnStart)
   DECL_CS_PROPERTY_NOTIFY(triggeredOnStart, triggeredOnStartChanged)
   DECL_CS_PROPERTY_READ(*parent, parent)
   DECL_CS_PROPERTY_CONSTANT(*parent)

 public:
   QDeclarativeTimer(QObject *parent = nullptr);

   void setInterval(int interval);
   int interval() const;

   bool isRunning() const;
   void setRunning(bool running);

   bool isRepeating() const;
   void setRepeating(bool repeating);

   bool triggeredOnStart() const;
   void setTriggeredOnStart(bool triggeredOnStart);

   DECL_CS_SLOT_1(Public, void start())
   DECL_CS_SLOT_2(start)
   DECL_CS_SLOT_1(Public, void stop())
   DECL_CS_SLOT_2(stop)
   DECL_CS_SLOT_1(Public, void restart())
   DECL_CS_SLOT_2(restart)

   DECL_CS_SIGNAL_1(Public, void triggered())
   DECL_CS_SIGNAL_2(triggered)
   DECL_CS_SIGNAL_1(Public, void runningChanged())
   DECL_CS_SIGNAL_2(runningChanged)
   DECL_CS_SIGNAL_1(Public, void intervalChanged())
   DECL_CS_SIGNAL_2(intervalChanged)
   DECL_CS_SIGNAL_1(Public, void repeatChanged())
   DECL_CS_SIGNAL_2(repeatChanged)
   DECL_CS_SIGNAL_1(Public, void triggeredOnStartChanged())
   DECL_CS_SIGNAL_2(triggeredOnStartChanged)

 protected:
   void classBegin();
   void componentComplete();

 private:
   void update();

   DECL_CS_SLOT_1(Private, void ticked())
   DECL_CS_SLOT_2(ticked)
   DECL_CS_SLOT_1(Private, void finished())
   DECL_CS_SLOT_2(finished)
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeTimer)

#endif
