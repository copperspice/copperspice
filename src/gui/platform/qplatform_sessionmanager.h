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

#ifndef QPLATFORM_SESSIONMANAGER_H
#define QPLATFORM_SESSIONMANAGER_H

#include <qnamespace.h>
#include <qsessionmanager.h>

#ifndef QT_NO_SESSIONMANAGER

class Q_GUI_EXPORT QPlatformSessionManager
{
 public:
   explicit QPlatformSessionManager(const QString &id, const QString &key);

   QPlatformSessionManager(const QPlatformSessionManager &) = delete;
   QPlatformSessionManager &operator=(const QPlatformSessionManager &) = delete;

   virtual ~QPlatformSessionManager();

   virtual QString sessionId() const;
   virtual QString sessionKey() const;

   virtual bool allowsInteraction();
   virtual bool allowsErrorInteraction();
   virtual void release();

   virtual void cancel();

   virtual void setRestartHint(QSessionManager::RestartHint restartHint);
   virtual QSessionManager::RestartHint restartHint() const;

   virtual void setRestartCommand(const QStringList &command);
   virtual QStringList restartCommand() const;
   virtual void setDiscardCommand(const QStringList &command);
   virtual QStringList discardCommand() const;

   virtual void setManagerProperty(const QString &name, const QString &value);
   virtual void setManagerProperty(const QString &name, const QStringList &value);

   virtual bool isPhase2() const;
   virtual void requestPhase2();

   void appCommitData();
   void appSaveState();

 protected:
   QString m_sessionId;
   QString m_sessionKey;

 private:
   QStringList m_restartCommand;
   QStringList m_discardCommand;
   QSessionManager::RestartHint m_restartHint;
};

#endif // QT_NO_SESSIONMANAGER

#endif
