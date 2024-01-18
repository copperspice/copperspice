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

#include <qplatform_sessionmanager.h>
#include <qguiapplication_p.h>

#ifndef QT_NO_SESSIONMANAGER

QPlatformSessionManager::QPlatformSessionManager(const QString &id, const QString &key)
   : m_sessionId(id), m_sessionKey(key),
     m_restartHint(QSessionManager::RestartIfRunning)
{
}

QPlatformSessionManager::~QPlatformSessionManager()
{
}

QString QPlatformSessionManager::sessionId() const
{
   return m_sessionId;
}

QString QPlatformSessionManager::sessionKey() const
{
   return m_sessionKey;
}

bool QPlatformSessionManager::allowsInteraction()
{
   return false;
}

bool QPlatformSessionManager::allowsErrorInteraction()
{
   return false;
}

void QPlatformSessionManager::release()
{
}

void QPlatformSessionManager::cancel()
{
}

void QPlatformSessionManager::setRestartHint(QSessionManager::RestartHint restartHint)
{
   m_restartHint = restartHint;
}

QSessionManager::RestartHint QPlatformSessionManager::restartHint() const
{
   return m_restartHint;
}

void QPlatformSessionManager::setRestartCommand(const QStringList &command)
{
   m_restartCommand = command;
}

QStringList QPlatformSessionManager::restartCommand() const
{
   return m_restartCommand;
}

void QPlatformSessionManager::setDiscardCommand(const QStringList &command)
{
   m_discardCommand = command;
}

QStringList QPlatformSessionManager::discardCommand() const
{
   return m_discardCommand;
}

void QPlatformSessionManager::setManagerProperty(const QString &name, const QString &value)
{
   (void) name;
   (void) value;
}

void QPlatformSessionManager::setManagerProperty(const QString &name, const QStringList &value)
{
   (void) name;
   (void) value;
}

bool QPlatformSessionManager::isPhase2() const
{
   return false;
}

void QPlatformSessionManager::requestPhase2()
{
}

void QPlatformSessionManager::appCommitData()
{
   qGuiApp->d_func()->commitData();
}

void QPlatformSessionManager::appSaveState()
{
   qGuiApp->d_func()->saveState();
}

#endif
