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

#include <avfdisplaylink.h>

#include <qcoreapplication.h>

#ifdef QT_DEBUG_AVF
#include <qdebug.h>
#endif

#if defined(Q_OS_IOS)
#import <QuartzCore/CADisplayLink.h>
#import <Foundation/NSRunLoop.h>
#define _m_displayLink static_cast<DisplayLinkObserver*>(m_displayLink)
#else
#endif

#if defined(Q_OS_IOS)
@interface DisplayLinkObserver : NSObject
{
   AVFDisplayLink *m_avfDisplayLink;
   CADisplayLink *m_displayLink;
}

- (void)start;
- (void)stop;
- (void)displayLinkNotification: (CADisplayLink *)sender;

@end

@implementation DisplayLinkObserver

- (id)initWithAVFDisplayLink: (AVFDisplayLink *)link
{
   self = [super init];

   if (self) {
      m_avfDisplayLink = link;
      m_displayLink = [[CADisplayLink displayLinkWithTarget: self selector: @selector(displayLinkNotification:)] retain];
   }

   return self;
}

- (void) dealloc
{
   if (m_displayLink) {
      [m_displayLink release];
      m_displayLink = NULL;
   }

   [super dealloc];
}

- (void)start
{
   [m_displayLink addToRunLoop: [NSRunLoop currentRunLoop] forMode: NSDefaultRunLoopMode];
}

- (void)stop
{
   [m_displayLink removeFromRunLoop: [NSRunLoop currentRunLoop] forMode: NSDefaultRunLoopMode];
}

- (void)displayLinkNotification: (CADisplayLink *)sender
{
   (void) sender;
   m_avfDisplayLink->displayLinkEvent(nullptr);
}

@end
#else
static CVReturn CVDisplayLinkCallback(CVDisplayLinkRef displayLink, const CVTimeStamp *inNow,
   const CVTimeStamp *inOutputTime, CVOptionFlags flagsIn,
   CVOptionFlags *flagsOut, void *displayLinkContext)
{
   (void) displayLink;
   (void) inNow;
   (void) flagsIn;
   (void) flagsOut;

   AVFDisplayLink *link = (AVFDisplayLink *)displayLinkContext;

   link->displayLinkEvent(inOutputTime);
   return kCVReturnSuccess;
}
#endif

AVFDisplayLink::AVFDisplayLink(QObject *parent)
   : QObject(parent), m_displayLink(nullptr), m_pendingDisplayLinkEvent(false), m_isActive(false)
{
#if defined(Q_OS_IOS)
   m_displayLink = [[DisplayLinkObserver alloc] initWithAVFDisplayLink: this];
#else
   // create display link for the main display
   CVDisplayLinkCreateWithCGDisplay(kCGDirectMainDisplay, &m_displayLink);
   if (m_displayLink) {
      // set the current display of a display link.
      CVDisplayLinkSetCurrentCGDisplay(m_displayLink, kCGDirectMainDisplay);

      // set the renderer output callback function
      CVDisplayLinkSetOutputCallback(m_displayLink, &CVDisplayLinkCallback, this);
   }
#endif
}

AVFDisplayLink::~AVFDisplayLink()
{
#ifdef QT_DEBUG_AVF
   qDebug() << Q_FUNC_INFO;
#endif

   if (m_displayLink) {
      stop();
#if defined(Q_OS_IOS)
      [_m_displayLink release];
#else
      CVDisplayLinkRelease(m_displayLink);
#endif
      m_displayLink = nullptr;
   }
}

bool AVFDisplayLink::isValid() const
{
   return m_displayLink != nullptr;
}

bool AVFDisplayLink::isActive() const
{
   return m_isActive;
}

void AVFDisplayLink::start()
{
   if (m_displayLink && !m_isActive) {
#if defined(Q_OS_IOS)
      [_m_displayLink start];
#else
      CVDisplayLinkStart(m_displayLink);
#endif
      m_isActive = true;
   }
}

void AVFDisplayLink::stop()
{
   if (m_displayLink && m_isActive) {
#if defined(Q_OS_IOS)
      [_m_displayLink stop];
#else
      CVDisplayLinkStop(m_displayLink);
#endif
      m_isActive = false;
   }
}

void AVFDisplayLink::displayLinkEvent(const CVTimeStamp *ts)
{
   // This function is called from a
   // thread != gui thread. So we post the event.
   // But we need to make sure that we don't post faster
   // than the event loop can eat:
   m_displayLinkMutex.lock();
   bool pending = m_pendingDisplayLinkEvent;
   m_pendingDisplayLinkEvent = true;

#if defined(Q_OS_IOS)
   (void) ts;
   memset(&m_frameTimeStamp, 0, sizeof(CVTimeStamp));
#else
   m_frameTimeStamp = *ts;
#endif

   m_displayLinkMutex.unlock();

   if (!pending) {
      qApp->postEvent(this, new QEvent(QEvent::User), Qt::HighEventPriority);
   }
}

bool AVFDisplayLink::event(QEvent *event)
{
   switch (event->type()) {
      case QEvent::User:  {
         m_displayLinkMutex.lock();
         m_pendingDisplayLinkEvent = false;
         CVTimeStamp ts = m_frameTimeStamp;
         m_displayLinkMutex.unlock();

         Q_EMIT tick(ts);

         return false;
      }
      break;
      default:
         break;
   }
   return QObject::event(event);
}
