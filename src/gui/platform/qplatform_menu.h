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

#ifndef QPLATFORM_MENU_H
#define QPLATFORM_MENU_H

#include <qglobal.h>
#include <qpointer.h>
#include <qfont.h>
#include <qkeysequence.h>
#include <qicon.h>

class QPlatformMenu;

class Q_GUI_EXPORT QPlatformMenuItem : public QObject
{
   GUI_CS_OBJECT(QPlatformMenuItem)

   GUI_CS_ENUM(MenuRole)

 public:
   // must stay in sync with, QAction menu roles
   enum MenuRole { NoRole = 0,
      TextHeuristicRole,
      ApplicationSpecificRole,
      AboutCsRole,
      AboutRole,
      PreferencesRole,
      QuitRole,

      // following are private, might be added as public QAction roles if necessary
      CutRole,
      CopyRole,
      PasteRole,
      SelectAllRole,
      RoleCount
   };

   virtual void setTag(quintptr tag) = 0;
   virtual quintptr tag()const = 0;

   virtual void setText(const QString &text) = 0;
   virtual void setIcon(const QIcon &icon) = 0;
   virtual void setMenu(QPlatformMenu *menu) = 0;
   virtual void setVisible(bool isVisible) = 0;
   virtual void setIsSeparator(bool isSeparator) = 0;
   virtual void setFont(const QFont &font) = 0;
   virtual void setRole(MenuRole role) = 0;
   virtual void setCheckable(bool checkable) = 0;
   virtual void setChecked(bool checked) = 0;
   virtual void setShortcut(const QKeySequence &shortcut) = 0;
   virtual void setEnabled(bool enabled) = 0;
   virtual void setIconSize(int size) = 0;

   virtual void setNativeContents(WId item) {
      (void) item;
   }

   GUI_CS_SIGNAL_1(Public, void activated())
   GUI_CS_SIGNAL_2(activated)

   GUI_CS_SIGNAL_1(Public, void hovered())
   GUI_CS_SIGNAL_2(hovered)
};

class Q_GUI_EXPORT QPlatformMenu : public QObject
{
   GUI_CS_OBJECT(QPlatformMenu)

   GUI_CS_ENUM(MenuType)

 public:
   enum MenuType { DefaultMenu = 0, EditMenu };

   virtual void insertMenuItem(QPlatformMenuItem *menuItem, QPlatformMenuItem *before) = 0;
   virtual void removeMenuItem(QPlatformMenuItem *menuItem) = 0;
   virtual void syncMenuItem(QPlatformMenuItem *menuItem) = 0;
   virtual void syncSeparatorsCollapsible(bool enable) = 0;

   virtual void setTag(quintptr tag) = 0;
   virtual quintptr tag()const = 0;

   virtual void setText(const QString &text) = 0;
   virtual void setIcon(const QIcon &icon) = 0;
   virtual void setEnabled(bool enabled) = 0;
   virtual bool isEnabled() const {
      return true;
   }
   virtual void setVisible(bool visible) = 0;
   virtual void setMinimumWidth(int width) {
      (void) width;
   }
   virtual void setFont(const QFont &font) {
      (void) font;
   }
   virtual void setMenuType(MenuType type) {
      (void) type;
   }

   virtual void showPopup(const QWindow *parentWindow, const QRect &targetRect, const QPlatformMenuItem *item) {
      (void) parentWindow;
      (void) targetRect;
      (void) item;

      setVisible(true);
   }

   virtual void dismiss() { } // Closes this and all its related menu popups

   virtual QPlatformMenuItem *menuItemAt(int position) const = 0;
   virtual QPlatformMenuItem *menuItemForTag(quintptr tag) const = 0;

   virtual QPlatformMenuItem *createMenuItem() const;
   virtual QPlatformMenu *createSubMenu() const;

   GUI_CS_SIGNAL_1(Public, void aboutToShow())
   GUI_CS_SIGNAL_2(aboutToShow)

   GUI_CS_SIGNAL_1(Public, void aboutToHide())
   GUI_CS_SIGNAL_2(aboutToHide)
};

class Q_GUI_EXPORT QPlatformMenuBar : public QObject
{
   GUI_CS_OBJECT(QPlatformMenuBar)

 public:
   virtual void insertMenu(QPlatformMenu *menu, QPlatformMenu *before) = 0;
   virtual void removeMenu(QPlatformMenu *menu) = 0;
   virtual void syncMenu(QPlatformMenu *menuItem) = 0;
   virtual void handleReparent(QWindow *newParentWindow) = 0;

   virtual QPlatformMenu *menuForTag(quintptr tag) const = 0;
};

#endif

