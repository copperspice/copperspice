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

#ifndef QCOCOAMENUITEM_H
#define QCOCOAMENUITEM_H

#include <qplatform_menu.h>
#include <qimage.h>

#ifdef __OBJC__
@class NSMenuItem;
@class NSMenu;
@class NSObject;
@class NSView;
#else
using NSMenuItem = struct objc_object;
using MSMenu     = struct objc_object;
using NSObject   = struct objc_object;
using NSView     = struct objc_object;
#endif

class QCocoaMenu;

class QCocoaMenuObject
{
 public:
   void setMenuParent(QObject *obj) {
      parent = obj;
   }

   QObject *menuParent() const {
      return parent;
   }

 private:
   QPointer<QObject> parent;
};

class QCocoaMenuItem : public QPlatformMenuItem, public QCocoaMenuObject
{
 public:
   QCocoaMenuItem();
   ~QCocoaMenuItem();

   void setTag(quintptr tag) override {
      m_tag = tag;
   }

   quintptr tag() const override {
      return m_tag;
   }

   void setText(const QString &text) override;
   void setIcon(const QIcon &icon) override;
   void setMenu(QPlatformMenu *menu) override;
   void setVisible(bool isVisible) override;
   void setIsSeparator(bool isSeparator) override;
   void setFont(const QFont &font) override;
   void setRole(MenuRole role) override;
   void setShortcut(const QKeySequence &shortcut) override;

   void setCheckable(bool checkable) override {
      (void) checkable;
   }

   void setChecked(bool isChecked) override;
   void setEnabled(bool isEnabled) override;
   void setIconSize(int size) override;

   void setNativeContents(WId item) override;

   inline QString text() const {
      return m_text;
   }

   inline NSMenuItem *nsItem() {
      return m_native;
   }
   NSMenuItem *sync();

   void syncMerged();
   void setParentEnabled(bool enabled);

   inline bool isMerged() const {
      return m_merged;
   }

   inline bool isEnabled() const {
      return m_enabled && m_parentEnabled;
   }

   inline bool isSeparator() const {
      return m_isSeparator;
   }

   QCocoaMenu *menu() const {
      return m_menu;
   }

   MenuRole effectiveRole() const;

 private:
   QString mergeText();
   QKeySequence mergeAccel();

   NSMenuItem *m_native;
   NSView *m_itemView;

   QString m_text;
   QIcon m_icon;
   QPointer<QCocoaMenu> m_menu;
   QFont m_font;

   MenuRole m_role;
   MenuRole m_detectedRole;

   QKeySequence m_shortcut;
   quintptr m_tag;
   int m_iconSize;

   bool m_textSynced: 1;
   bool m_isVisible: 1;
   bool m_enabled: 1;
   bool m_parentEnabled: 1;
   bool m_isSeparator: 1;
   bool m_checked: 1;
   bool m_merged: 1;
};

#endif
