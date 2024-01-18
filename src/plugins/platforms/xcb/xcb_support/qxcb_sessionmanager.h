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

#ifndef QXCB_SESSIONMANAGER_H
#define QXCB_SESSIONMANAGER_H

#include <qplatform_sessionmanager.h>

class QEventLoop;

class QXcbSessionManager : public QPlatformSessionManager
{
 public:
   QXcbSessionManager(const QString &id, const QString &key);

   QXcbSessionManager(const QXcbSessionManager &) = delete;
   QXcbSessionManager &operator=(const QXcbSessionManager &) = delete;

   virtual ~QXcbSessionManager();

   void *handle() const;

   void setSessionId(const QString &id) {
      m_sessionId = id;
   }
   void setSessionKey(const QString &key) {
      m_sessionKey = key;
   }

   bool allowsInteraction() override;
   bool allowsErrorInteraction() override;
   void release() override;

   void cancel() override;

   void setManagerProperty(const QString &name, const QString &value) override;
   void setManagerProperty(const QString &name, const QStringList &value) override;

   bool isPhase2() const override;
   void requestPhase2() override;

   void exitEventLoop();

 private:
   QEventLoop *m_eventLoop;
};

#endif
