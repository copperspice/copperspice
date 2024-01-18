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

#include <qsessionmanager.h>
#include <qplatform_sessionmanager.h>
#include <qplatform_integration.h>

#include <qapplication.h>
#include <qguiapplication_p.h>
#include <qsessionmanager_p.h>

#ifndef QT_NO_SESSIONMANAGER

QSessionManagerPrivate::QSessionManagerPrivate(const QString &id, const QString &key)
{
    platformSessionManager = QGuiApplicationPrivate::platformIntegration()->createPlatformSessionManager(id, key);

    Q_ASSERT_X(platformSessionManager, "Platform session management",
               "No platform session management, should use the default implementation");
}

QSessionManagerPrivate::~QSessionManagerPrivate()
{
    delete platformSessionManager;
    platformSessionManager = nullptr;
}

QSessionManager::QSessionManager(QApplication *app, QString &id, QString &key)
   : QObject(app), d_ptr(new QSessionManagerPrivate(id, key))
{
}

QSessionManager::~QSessionManager()
{
}

QString QSessionManager::sessionId() const
{
   Q_D(const QSessionManager);
   return d->platformSessionManager->sessionId();
}

QString QSessionManager::sessionKey() const
{
   Q_D(const QSessionManager);
   return d->platformSessionManager->sessionKey();
}

bool QSessionManager::allowsInteraction()
{
    Q_D(QSessionManager);
    return d->platformSessionManager->allowsInteraction();
}

bool QSessionManager::allowsErrorInteraction()
{
    Q_D(QSessionManager);
    return d->platformSessionManager->allowsErrorInteraction();
}

void QSessionManager::release()
{
    Q_D(QSessionManager);
    d->platformSessionManager->release();
}

void QSessionManager::cancel()
{
    Q_D(QSessionManager);
    d->platformSessionManager->cancel();
}

void QSessionManager::setRestartHint(QSessionManager::RestartHint hint)
{
   Q_D(QSessionManager);
    d->platformSessionManager->setRestartHint(hint);
}

QSessionManager::RestartHint QSessionManager::restartHint() const
{
   Q_D(const QSessionManager);
    return d->platformSessionManager->restartHint();
}

void QSessionManager::setRestartCommand(const QStringList &command)
{
   Q_D(QSessionManager);
    d->platformSessionManager->setRestartCommand(command);
}

QStringList QSessionManager::restartCommand() const
{
   Q_D(const QSessionManager);
    return d->platformSessionManager->restartCommand();
}

void QSessionManager::setDiscardCommand(const QStringList &command)
{
   Q_D(QSessionManager);
    d->platformSessionManager->setDiscardCommand(command);
}

QStringList QSessionManager::discardCommand() const
{
   Q_D(const QSessionManager);
    return d->platformSessionManager->discardCommand();
}

void QSessionManager::setManagerProperty(const QString &name,
   const QString &value)
{
    Q_D(QSessionManager);
    d->platformSessionManager->setManagerProperty(name, value);
}

void QSessionManager::setManagerProperty(const QString &name,
   const QStringList &value)
{
    Q_D(QSessionManager);
    d->platformSessionManager->setManagerProperty(name, value);
}

bool QSessionManager::isPhase2() const
{
    Q_D(const QSessionManager);
    return d->platformSessionManager->isPhase2();
}

void QSessionManager::requestPhase2()
{
    Q_D(QSessionManager);
    d->platformSessionManager->requestPhase2();
}

#endif // QT_NO_SESSIONMANAGER
