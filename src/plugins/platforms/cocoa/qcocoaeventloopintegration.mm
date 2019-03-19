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

#include "qcocoaeventloopintegration.h"

#import <Cocoa/Cocoa.h>

#include "qcocoaautoreleasepool.h"

#include <QtCore/QElapsedTimer>

#include <QDebug>
#include <QApplication>

void wakeupCallback ( void * ) {
    QPlatformEventLoopIntegration::processEvents();
}

void timerCallback( CFRunLoopTimerRef timer, void *info)
{
    QPlatformEventLoopIntegration::processEvents();
    QCocoaEventLoopIntegration *eventLoopIntegration =
            static_cast<QCocoaEventLoopIntegration *>(info);
    qint64 nextTime = eventLoopIntegration->nextTimerEvent();
    CFAbsoluteTime nexttime = CFAbsoluteTimeGetCurrent();
    nexttime = nexttime + (double(nextTime)/1000);
    CFRunLoopTimerSetNextFireDate(timer,nexttime);
}

QCocoaEventLoopIntegration::QCocoaEventLoopIntegration() :
    QPlatformEventLoopIntegration()
{
    [NSApplication sharedApplication];
    m_sourceContext.version = 0;
    m_sourceContext.info = this;
    m_sourceContext.retain = 0;
    m_sourceContext.release = 0;
    m_sourceContext.copyDescription = 0;
    m_sourceContext.equal = 0;
    m_sourceContext.hash = 0;
    m_sourceContext.schedule = 0;
    m_sourceContext.cancel = 0;
    m_sourceContext.perform = wakeupCallback;

    m_source = CFRunLoopSourceCreate(0,0,&m_sourceContext);
    CFRunLoopAddSource(CFRunLoopGetMain(),m_source,kCFRunLoopCommonModes);

    m_timerContext.version = 0;
    m_timerContext.info = this;
    m_timerContext.retain = 0;
    m_timerContext.release = 0;
    m_timerContext.copyDescription = 0;
    CFAbsoluteTime fireDate = CFAbsoluteTimeGetCurrent ();
    CFTimeInterval interval = 30;

    CFRunLoopTimerRef m_timerSource = CFRunLoopTimerCreate(0,fireDate,interval,0,0,timerCallback,&m_timerContext);
    CFRunLoopAddTimer(CFRunLoopGetMain(),m_timerSource,kCFRunLoopCommonModes);
}

void QCocoaEventLoopIntegration::startEventLoop()
{
    [[NSApplication sharedApplication] run];
}

void QCocoaEventLoopIntegration::quitEventLoop()
{
    [[NSApplication sharedApplication] terminate:nil];
}

void QCocoaEventLoopIntegration::qtNeedsToProcessEvents()
{
    CFRunLoopSourceSignal(m_source);
}

