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

#include <qwin_session_manager.h>

QWindowsSessionManager::QWindowsSessionManager(const QString &id, const QString &key)
   : QPlatformSessionManager(id, key), m_isActive(false)
   , m_blockUserInput(false)
   , m_canceled(false)
{
}

bool QWindowsSessionManager::allowsInteraction()
{
   m_blockUserInput = false;
   return true;
}

bool QWindowsSessionManager::allowsErrorInteraction()
{
   m_blockUserInput = false;
   return true;
}

void QWindowsSessionManager::release()
{
   if (m_isActive) {
      m_blockUserInput = true;
   }
}

void QWindowsSessionManager::cancel()
{
   m_canceled = true;
}


