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

#ifndef QCOCOASYSTEMTRAYICON_P_H
#define QCOCOASYSTEMTRAYICON_P_H

#include <qglobal.h>

#ifndef QT_NO_SYSTEMTRAYICON

#include <qstring.h>
#include <qplatform_systemtrayicon.h>

class QSystemTrayIconSys;

class QCocoaSystemTrayIcon : public QPlatformSystemTrayIcon
{
 public:
   QCocoaSystemTrayIcon()
      : m_sys(nullptr)
   {
   }

   void init() override;
   void cleanup() override;
   void updateIcon(const QIcon &icon) override;
   void updateToolTip(const QString &toolTip) override;
   void updateMenu(QPlatformMenu *menu) override;
   QRect geometry() const override;
   void showMessage(const QString &title, const QString &msg,
      const QIcon &icon, MessageIcon iconType, int secs) override;

   bool isSystemTrayAvailable() const override;
   bool supportsMessages() const override;

 private:
   QSystemTrayIconSys *m_sys;
};

#endif

#endif
