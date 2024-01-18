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

#ifndef QCOCOAMENUBAR_H
#define QCOCOAMENUBAR_H

#include <QList>
#include <qplatform_menu.h>
#include "qcocoamenu.h"

class QCocoaWindow;

class QCocoaMenuBar : public QPlatformMenuBar
{
   CS_OBJECT(QCocoaMenuBar)

 public:
   QCocoaMenuBar();
   ~QCocoaMenuBar();

   void insertMenu(QPlatformMenu *menu, QPlatformMenu *before) override;
   void removeMenu(QPlatformMenu *menu) override;
   void syncMenu(QPlatformMenu *menuItem) override;
   void handleReparent(QWindow *newParentWindow) override;
   QPlatformMenu *menuForTag(quintptr tag) const override;

   inline NSMenu *nsMenu() const {
      return m_nativeMenu;
   }

   static void redirectKnownMenuItemsToFirstResponder();
   static void resetKnownMenuItemsToQt();
   static void updateMenuBarImmediately();

   QList<QCocoaMenuItem *> merged() const;
   NSMenuItem *itemForRole(QPlatformMenuItem::MenuRole r);

 private:
   static QCocoaWindow *findWindowForMenubar();
   static QCocoaMenuBar *findGlobalMenubar();

   bool needsImmediateUpdate();
   bool shouldDisable(QCocoaWindow *active) const;

   NSMenuItem *nativeItemForMenu(QCocoaMenu *menu) const;

   QList<QPointer<QCocoaMenu>> m_menus;
   NSMenu *m_nativeMenu;
   QCocoaWindow *m_window;
};

#endif
