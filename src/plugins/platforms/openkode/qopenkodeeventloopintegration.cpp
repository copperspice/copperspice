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

#include "qopenkodeeventloopintegration.h"

#include <QDebug>

#include <KD/kd.h>
#include <KD/ATX_keyboard.h>

QT_BEGIN_NAMESPACE

static const int QT_EVENT_WAKEUP_EVENTLOOP = KD_EVENT_USER + 1;

void kdprocessevent( const KDEvent *event)
{
    switch (event->type) {
    case KD_EVENT_INPUT:
        qDebug() << "KD_EVENT_INPUT";
        break;
    case KD_EVENT_INPUT_POINTER:
        qDebug() << "KD_EVENT_INPUT_POINTER";
        break;
    case KD_EVENT_WINDOW_CLOSE:
        qDebug() << "KD_EVENT_WINDOW_CLOSE";
        break;
    case KD_EVENT_WINDOWPROPERTY_CHANGE:
        qDebug() << "KD_EVENT_WINDOWPROPERTY_CHANGE";
        qDebug() << event->data.windowproperty.pname;
        break;
    case KD_EVENT_WINDOW_FOCUS:
        qDebug() << "KD_EVENT_WINDOW_FOCUS";
        break;
    case KD_EVENT_WINDOW_REDRAW:
        qDebug() << "KD_EVENT_WINDOW_REDRAW";
        break;
    case KD_EVENT_USER:
        qDebug() << "KD_EVENT_USER";
        break;
    case KD_EVENT_INPUT_KEY_ATX:
        qDebug() << "KD_EVENT_INPUT_KEY_ATX";
        break;
    case QT_EVENT_WAKEUP_EVENTLOOP:
        QPlatformEventLoopIntegration::processEvents();
        break;
    default:
        break;
    }

    kdDefaultEvent(event);

}

QOpenKODEEventLoopIntegration::QOpenKODEEventLoopIntegration()
    : m_quit(false)
{
    m_kdThread = kdThreadSelf();
    kdInstallCallback(&kdprocessevent,QT_EVENT_WAKEUP_EVENTLOOP,this);
}

void QOpenKODEEventLoopIntegration::startEventLoop()
{

    while(!m_quit) {
        qint64 msec = nextTimerEvent();
        const KDEvent *event = kdWaitEvent(msec);
        if (event) {
            kdDefaultEvent(event);
            while ((event = kdWaitEvent(0)) != 0) {
                kdDefaultEvent(event);
            }
        }
        QPlatformEventLoopIntegration::processEvents();
    }
}

void QOpenKODEEventLoopIntegration::quitEventLoop()
{
    m_quit = true;
}

void QOpenKODEEventLoopIntegration::qtNeedsToProcessEvents()
{
    KDEvent *event = kdCreateEvent();
    event->type = QT_EVENT_WAKEUP_EVENTLOOP;
    event->userptr = this;
    kdPostThreadEvent(event,m_kdThread);
}

QT_END_NAMESPACE
