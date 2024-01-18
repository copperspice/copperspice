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

#ifndef QCOCOAMENU_H
#define QCOCOAMENU_H

#include <QList>
#include <qplatform_menu.h>
#include <qcocoamenuitem.h>

class QCocoaMenuBar;

class QCocoaMenu : public QPlatformMenu, public QCocoaMenuObject
{
 public:
   QCocoaMenu();
   ~QCocoaMenu();

   void setTag(quintptr tag) override {
      m_tag = tag;
   }
   quintptr tag() const override {
      return m_tag;
   }

   void insertMenuItem(QPlatformMenuItem *menuItem, QPlatformMenuItem *before) override;
   void removeMenuItem(QPlatformMenuItem *menuItem) override;
   void syncMenuItem(QPlatformMenuItem *menuItem) override;
   void setEnabled(bool enabled) override;
   bool isEnabled() const override;
   void setVisible(bool visible) override;
   void showPopup(const QWindow *parentWindow, const QRect &targetRect, const QPlatformMenuItem *item) override;
   void dismiss() override;

   void syncSeparatorsCollapsible(bool enable) override;

   void propagateEnabledState(bool enabled);

   void setIcon(const QIcon &icon) override {
      (void) icon;
   }

   void setText(const QString &text) override;
   void setMinimumWidth(int width) override;
   void setFont(const QFont &font) override;

   inline NSMenu *nsMenu() const {
      return m_nativeMenu;
   }

   inline bool isVisible() const {
      return m_visible;
   }

   QPlatformMenuItem *menuItemAt(int position) const override;
   QPlatformMenuItem *menuItemForTag(quintptr tag) const override;

   QList<QCocoaMenuItem *> items() const;
   QList<QCocoaMenuItem *> merged() const;

   void setAttachedItem(NSMenuItem *item);
   NSMenuItem *attachedItem() const;

   bool isOpen() const;
   void setIsOpen(bool isOpen);

 private:
   QCocoaMenuItem *itemOrNull(int index) const;
   void insertNative(QCocoaMenuItem *item, QCocoaMenuItem *beforeItem);

   QList<QCocoaMenuItem *> m_menuItems;
   NSMenu *m_nativeMenu;
   NSMenuItem *m_attachedItem;
   quintptr m_tag;
   bool m_enabled: 1;
   bool m_parentEnabled: 1;
   bool m_visible: 1;
   bool m_isOpen: 1;
};

#endif
