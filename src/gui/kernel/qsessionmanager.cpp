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

#include <qsessionmanager.h>

#include <qapplication.h>

#ifndef QT_NO_SESSIONMANAGER

QT_BEGIN_NAMESPACE

class QSessionManagerPrivate
{

 public:
   QSessionManagerPrivate(QSessionManager *m, const QString &id, const QString &key);
   virtual ~QSessionManagerPrivate() {};

   QStringList restartCommand;
   QStringList discardCommand;
   const QString sessionId;
   const QString sessionKey;
   QSessionManager::RestartHint restartHint;
};

QSessionManagerPrivate::QSessionManagerPrivate(QSessionManager *, const QString &id, const QString &key)
   : sessionId(id), sessionKey(key)
{
}

QSessionManager::QSessionManager(QApplication *app, QString &id, QString &key)
   : QObject(*(new QSessionManagerPrivate(this, id, key)), app)
{
   Q_D(QSessionManager);
   d->restartHint = RestartIfRunning;
}

QSessionManager::~QSessionManager()
{
}

QString QSessionManager::sessionId() const
{
   Q_D(const QSessionManager);
   return d->sessionId;
}

QString QSessionManager::sessionKey() const
{
   Q_D(const QSessionManager);
   return d->sessionKey;
}


bool QSessionManager::allowsInteraction()
{
   return false;
}

bool QSessionManager::allowsErrorInteraction()
{
   return false;
}

void QSessionManager::release()
{
}

void QSessionManager::cancel()
{
}

void QSessionManager::setRestartHint(QSessionManager::RestartHint hint)
{
   Q_D(QSessionManager);
   d->restartHint = hint;
}

QSessionManager::RestartHint QSessionManager::restartHint() const
{
   Q_D(const QSessionManager);
   return d->restartHint;
}

void QSessionManager::setRestartCommand(const QStringList &command)
{
   Q_D(QSessionManager);
   d->restartCommand = command;
}

QStringList QSessionManager::restartCommand() const
{
   Q_D(const QSessionManager);
   return d->restartCommand;
}

void QSessionManager::setDiscardCommand(const QStringList &command)
{
   Q_D(QSessionManager);
   d->discardCommand = command;
}

QStringList QSessionManager::discardCommand() const
{
   Q_D(const QSessionManager);
   return d->discardCommand;
}

void QSessionManager::setManagerProperty(const QString &name,
      const QString &value)
{
   Q_UNUSED(name);
   Q_UNUSED(value);
}

void QSessionManager::setManagerProperty(const QString &name,
      const QStringList &value)
{
   Q_UNUSED(name);
   Q_UNUSED(value);
}

bool QSessionManager::isPhase2() const
{
   return false;
}

void QSessionManager::requestPhase2()
{
}

QT_END_NAMESPACE

#endif // QT_NO_SESSIONMANAGER
