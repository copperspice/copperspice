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

/*
*
* Copyright (c) 2006 Nikolas Zimmermann <zimmermann@kde.org>
*
*/

#include "config.h"
#include "MainThread.h"

#include <QObject>
#include <QCoreApplication>
#include <QThread>

namespace WTF {

class MainThreadInvoker : public QObject
{
    WEB_CS_OBJECT(MainThreadInvoker)

public:
    MainThreadInvoker();

private:
    WEB_CS_SLOT_1(Private,void dispatch())
    WEB_CS_SLOT_2(dispatch)
};

MainThreadInvoker::MainThreadInvoker()
{
    moveToThread(QCoreApplication::instance()->thread());
}

void MainThreadInvoker::dispatch()
{
    dispatchFunctionsFromMainThread();
}

Q_GLOBAL_STATIC(MainThreadInvoker, webkit_main_thread_invoker)

void initializeMainThreadPlatform()
{
}

void scheduleDispatchFunctionsOnMainThread()
{
    QMetaObject::invokeMethod(webkit_main_thread_invoker(), "dispatch", Qt::QueuedConnection);
}

bool isMainThread()
{
    return QThread::currentThread() == QCoreApplication::instance()->thread();
}

} // namespace WTF
