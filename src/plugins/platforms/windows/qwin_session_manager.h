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

#ifndef QWINDOWSSESSIONMANAGER_H
#define QWINDOWSSESSIONMANAGER_H

#include <qplatform_sessionmanager.h>

class QWindowsSessionManager : public QPlatformSessionManager
{
 public:
   explicit QWindowsSessionManager(const QString &id, const QString &key);

   QWindowsSessionManager(const QWindowsSessionManager &) = delete;
   QWindowsSessionManager &operator=(const QWindowsSessionManager &) = delete;

   bool allowsInteraction() override;
   bool allowsErrorInteraction() override;

   void blocksInteraction() {
      m_blockUserInput = true;
   }

   bool isInteractionBlocked() const {
      return m_blockUserInput;
   }

   void release() override;

   void cancel() override;
   void clearCancellation() {
      m_canceled = false;
   }

   bool wasCanceled() const {
      return m_canceled;
   }

   void setActive(bool active) {
      m_isActive = active;
   }

   bool isActive() const {
      return m_isActive;
   }

 private:
   bool m_isActive;
   bool m_blockUserInput;
   bool m_canceled;
};

#endif
