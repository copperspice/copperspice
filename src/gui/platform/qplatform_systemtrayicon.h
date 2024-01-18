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

#ifndef QPLATFORM_SYSTEMTRAYICON_H
#define QPLATFORM_SYSTEMTRAYICON_H

#include <qobject.h>
#include <qstring.h>

#ifndef QT_NO_SYSTEMTRAYICON

class QPlatformMenu;
class QIcon;
class QRect;

class Q_GUI_EXPORT QPlatformSystemTrayIcon : public QObject
{
   GUI_CS_OBJECT(QPlatformSystemTrayIcon)

   GUI_CS_ENUM(ActivationReason)
   GUI_CS_ENUM(MessageIcon)

 public:
   enum ActivationReason {
      Unknown,
      Context,
      DoubleClick,
      Trigger,
      MiddleClick
   };

   enum MessageIcon { NoIcon, Information, Warning, Critical };

   QPlatformSystemTrayIcon();
   ~QPlatformSystemTrayIcon();

   virtual void init() = 0;
   virtual void cleanup() = 0;
   virtual void updateIcon(const QIcon &icon) = 0;
   virtual void updateToolTip(const QString &tooltip) = 0;
   virtual void updateMenu(QPlatformMenu *menu) = 0;
   virtual QRect geometry() const = 0;
   virtual void showMessage(const QString &title, const QString &msg,
      const QIcon &icon, MessageIcon iconType, int msecs) = 0;

   virtual bool isSystemTrayAvailable() const = 0;
   virtual bool supportsMessages() const = 0;

   virtual QPlatformMenu *createMenu() const;

   GUI_CS_SIGNAL_1(Public, void activated(QPlatformSystemTrayIcon::ActivationReason reason))
   GUI_CS_SIGNAL_2(activated, reason)

   GUI_CS_SIGNAL_1(Public, void messageClicked())
   GUI_CS_SIGNAL_2(messageClicked)
};


#endif // QT_NO_SYSTEMTRAYICON

#endif
