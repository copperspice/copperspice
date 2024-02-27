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

#ifndef AVFDISPLAYLINK_H
#define AVFDISPLAYLINK_H

#include <qobject.h>
#include <qmutex.h>

#if defined(Q_OS_IOS)
#include <CoreVideo/CVBase.h>
#else
#include <QuartzCore/CVDisplayLink.h>
#endif

class AVFDisplayLink : public QObject
{
   CS_OBJECT(AVFDisplayLink)

 public:
   explicit AVFDisplayLink(QObject *parent = nullptr);
   virtual ~AVFDisplayLink();

   bool isValid() const;
   bool isActive() const;

   CS_SLOT_1(Public, void start())
   CS_SLOT_2(start)

   CS_SLOT_1(Public, void stop())
   CS_SLOT_2(stop)

   CS_SIGNAL_1(Public, void tick(const CVTimeStamp &ts))
   CS_SIGNAL_2(tick, ts)

   void displayLinkEvent(const CVTimeStamp *);

 protected:
   bool event(QEvent *) override;

 private:

#if defined(Q_OS_IOS)
   void *m_displayLink;
#else
   CVDisplayLinkRef m_displayLink;
#endif

   QMutex m_displayLinkMutex;
   bool m_pendingDisplayLinkEvent;
   bool m_isActive;
   CVTimeStamp m_frameTimeStamp;
};

#endif
