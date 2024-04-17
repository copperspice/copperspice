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

#ifndef QSYSTEMTRAYICON_H
#define QSYSTEMTRAYICON_H

#include <qobject.h>

#ifndef QT_NO_SYSTEMTRAYICON

#include <qicon.h>
#include <qplatform_systemtrayicon.h>
#include <qscopedpointer.h>

class QEvent;
class QMenu;
class QMouseEvent;
class QPoint;
class QSystemTrayIconPrivate;
class QWheelEvent;

class Q_GUI_EXPORT QSystemTrayIcon : public QObject
{
   GUI_CS_OBJECT(QSystemTrayIcon)

   GUI_CS_PROPERTY_READ(toolTip, toolTip)
   GUI_CS_PROPERTY_WRITE(toolTip, setToolTip)

   GUI_CS_PROPERTY_READ(icon, icon)
   GUI_CS_PROPERTY_WRITE(icon, setIcon)

   GUI_CS_PROPERTY_READ(visible, isVisible)
   GUI_CS_PROPERTY_WRITE(visible, setVisible)

   GUI_CS_PROPERTY_DESIGNABLE(visible, false)

 public:
   enum MessageIcon {
      NoIcon,
      Information,
      Warning,
      Critical
   };

   QSystemTrayIcon(QObject *parent = nullptr);
   QSystemTrayIcon(const QIcon &icon, QObject *parent = nullptr);

   QSystemTrayIcon(const QSystemTrayIcon &) = delete;
   QSystemTrayIcon &operator=(const QSystemTrayIcon &) = delete;

   ~QSystemTrayIcon();

   enum ActivationReason {
      Unknown,
      Context,
      DoubleClick,
      Trigger,
      MiddleClick
   };

#ifndef QT_NO_MENU
   void setContextMenu(QMenu *menu);
   QMenu *contextMenu() const;
#endif

   QIcon icon() const;
   void setIcon(const QIcon &icon);

   QString toolTip() const;
   void setToolTip(const QString &tip);

   static bool isSystemTrayAvailable();
   static bool supportsMessages();

   QRect geometry() const;
   bool isVisible() const;

   GUI_CS_SLOT_1(Public, void setVisible(bool visible))
   GUI_CS_SLOT_2(setVisible)

   GUI_CS_SLOT_1(Public, void show())
   GUI_CS_SLOT_2(show)

   GUI_CS_SLOT_1(Public, void hide())
   GUI_CS_SLOT_2(hide)

   GUI_CS_SLOT_1(Public, void showMessage(const QString &title, const QString &msg,
               QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::Information, int msecs = 10000))
   GUI_CS_SLOT_2(showMessage)

   GUI_CS_SIGNAL_1(Public, void activated(QSystemTrayIcon::ActivationReason reason))
   GUI_CS_SIGNAL_2(activated, reason)

   GUI_CS_SIGNAL_1(Public, void messageClicked())
   GUI_CS_SIGNAL_2(messageClicked)

 protected:
   bool event(QEvent *event) override;

   QScopedPointer<QSystemTrayIconPrivate> d_ptr;

 private:
   Q_DECLARE_PRIVATE(QSystemTrayIcon)

   GUI_CS_SLOT_1(Private, void _q_emitActivated(QPlatformSystemTrayIcon::ActivationReason))
   GUI_CS_SLOT_2(_q_emitActivated)

   friend class QSystemTrayIconSys;
   friend class QBalloonTip;
};

inline void QSystemTrayIcon::show()
{
   setVisible(true);
}

inline void QSystemTrayIcon::hide()
{
   setVisible(false);
}

#endif // QT_NO_SYSTEMTRAYICON
#endif // QSYSTEMTRAYICON_H
