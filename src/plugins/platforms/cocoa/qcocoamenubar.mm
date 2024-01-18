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

#include <Cocoa/Cocoa.h>

#include <qalgorithms.h>
#include <qapplication.h>
#include <qdebug.h>
#include <qcocoamenubar.h>
#include <qcocoawindow.h>
#include <qcocoamenuloader.h>
#include <qcocoaapplication.h>             // for custom application category
#include <qcocoaapplicationdelegate.h>

static QList<QCocoaMenuBar *> static_menubars;

static inline QCocoaMenuLoader *getMenuLoader()
{
   return [NSApp qt_qcocoamenuLoader];
}

QCocoaMenuBar::QCocoaMenuBar()
   : m_window(nullptr)
{
   static_menubars.append(this);

   m_nativeMenu = [[NSMenu alloc] init];

#ifdef QT_COCOA_ENABLE_MENU_DEBUG
   qDebug() << "Construct QCocoaMenuBar" << this << m_nativeMenu;
#endif
}

QCocoaMenuBar::~QCocoaMenuBar()
{
#ifdef QT_COCOA_ENABLE_MENU_DEBUG
   qDebug() << "~QCocoaMenuBar" << this;
#endif

   for (QCocoaMenu *menu : m_menus) {
      if (! menu) {
         continue;
      }

      NSMenuItem *item = nativeItemForMenu(menu);
      if (menu->attachedItem() == item) {
         menu->setAttachedItem(nil);
      }
   }

   [m_nativeMenu release];
   static_menubars.removeOne(this);

   if (m_window && m_window->menubar() == this) {
      m_window->setMenubar(nullptr);

      // Delete the children first so they do not cause
      // the native menu items to be hidden after
      // the menu bar was updated
      qDeleteAll(children());
      updateMenuBarImmediately();
   }
}

bool QCocoaMenuBar::needsImmediateUpdate()
{
   if (m_window && m_window->window()->isActive()) {
      return true;

   } else if (! m_window) {

      // Only update if the focus/active window has no menubar, which means it will be using this menubar.
      // This is to avoid a modification in a parentless menubar to affect a window-assigned menubar.

      QWindow *fw = QApplication::focusWindow();
      if (! fw) {
         // Same if there is no focus window
         return true;

      } else {
         QCocoaWindow *cw = static_cast<QCocoaWindow *>(fw->handle());
         if (cw && !cw->menubar()) {
            return true;
         }
      }
   }

   // Either the menubar is attached to a non-active window,
   // or the application's focus window has its own menubar
   // (which is different from this one)
   return false;
}

void QCocoaMenuBar::insertMenu(QPlatformMenu *platformMenu, QPlatformMenu *before)
{
   QCocoaMenu *menu       = static_cast<QCocoaMenu *>(platformMenu);
   QCocoaMenu *beforeMenu = static_cast<QCocoaMenu *>(before);

#ifdef QT_COCOA_ENABLE_MENU_DEBUG
   qDebug() << "QCocoaMenuBar::insertMenu()" << menu << "before" << before;
#endif

   if (m_menus.contains(QPointer<QCocoaMenu>(menu))) {
      qWarning("Menu already belongs to the menubar remove it first");
      return;
   }

   if (beforeMenu && ! m_menus.contains(QPointer<QCocoaMenu>(beforeMenu))) {
      qWarning("Before menu does not belong to the menubar");
      return;
   }

   int insertionIndex = beforeMenu ? m_menus.indexOf(beforeMenu) : m_menus.size();
   m_menus.insert(insertionIndex, menu);

   {
      QMacAutoReleasePool pool;
      NSMenuItem *item = [[[NSMenuItem alloc] init] autorelease];
      item.tag = reinterpret_cast<NSInteger>(menu);

      if (beforeMenu) {
         // QMenuBar::toNSMenu() exposes the native menubar and
         // the user could have inserted its own items in there.
         // Same remark applies to removeMenu().

         NSMenuItem *beforeItem = nativeItemForMenu(beforeMenu);
         NSInteger nativeIndex = [m_nativeMenu indexOfItem: beforeItem];
         [m_nativeMenu insertItem: item atIndex: nativeIndex];

      } else {
         [m_nativeMenu addItem: item];
      }
   }

   syncMenu(menu);

   if (needsImmediateUpdate()) {
      updateMenuBarImmediately();
   }
}

void QCocoaMenuBar::removeMenu(QPlatformMenu *platformMenu)
{
   QCocoaMenu *menu = static_cast<QCocoaMenu *>(platformMenu);
   if (! m_menus.contains(menu)) {
      qWarning("Trying to remove a menu that does not belong to the menubar");
      return;
   }

   NSMenuItem *item = nativeItemForMenu(menu);
   if (menu->attachedItem() == item) {
      menu->setAttachedItem(nil);
   }
   m_menus.removeOne(menu);

   QMacAutoReleasePool pool;

   // See remark in insertMenu().
   NSInteger nativeIndex = [m_nativeMenu indexOfItem: item];
   [m_nativeMenu removeItemAtIndex: nativeIndex];
}

void QCocoaMenuBar::syncMenu(QPlatformMenu *menu)
{
   QMacAutoReleasePool pool;

   QCocoaMenu *cocoaMenu = static_cast<QCocoaMenu *>(menu);
   for (QCocoaMenuItem *item : cocoaMenu->items()) {
      cocoaMenu->syncMenuItem(item);
   }

   BOOL shouldHide = YES;
   if (cocoaMenu->isVisible()) {
      // If the NSMenu has no visble items, or only separators, we should hide it
      // on the menubar. This can happen after syncing the menu items since they
      // can be moved to other menus.

      for (NSMenuItem * item in [cocoaMenu->nsMenu() itemArray])
         if (! [item isSeparatorItem] && ![item isHidden]) {
            shouldHide = NO;
            break;
         }
   }

   if (NSMenuItem *attachedItem = cocoaMenu->attachedItem()) {
      // Non-nil attached item means the item's submenu is set
      attachedItem.title = cocoaMenu->nsMenu().title;
      attachedItem.hidden = shouldHide;
   }
}

NSMenuItem *QCocoaMenuBar::nativeItemForMenu(QCocoaMenu *menu) const
{
   if (! menu) {
      return nil;
   }

   return [m_nativeMenu itemWithTag: reinterpret_cast<NSInteger>(menu)];
}

void QCocoaMenuBar::handleReparent(QWindow *newParentWindow)
{
#ifdef QT_COCOA_ENABLE_MENU_DEBUG
   qDebug() << "QCocoaMenuBar::handleReparent()" << newParentWindow;
#endif

   if (m_window) {
      m_window->setMenubar(nullptr);
   }

   if (newParentWindow == nullptr) {
      m_window = nullptr;
   } else {
      newParentWindow->create();
      m_window = static_cast<QCocoaWindow *>(newParentWindow->handle());
      m_window->setMenubar(this);
   }

   updateMenuBarImmediately();
}

QCocoaWindow *QCocoaMenuBar::findWindowForMenubar()
{
   if (qApp->focusWindow()) {
      return static_cast<QCocoaWindow *>(qApp->focusWindow()->handle());
   }

   return nullptr;
}

QCocoaMenuBar *QCocoaMenuBar::findGlobalMenubar()
{
   for (QCocoaMenuBar *mb : static_menubars) {
      if (mb->m_window == nullptr) {
         return mb;
      }
   }

   return nullptr;
}

void QCocoaMenuBar::redirectKnownMenuItemsToFirstResponder()
{
   // QTBUG-17291: http://forums.macrumors.com/showthread.php?t=1249452
   // When a dialog is opened, shortcuts for actions inside the dialog (cut, paste, ...)
   // continue to go through the same menu items which claimed those shortcuts.
   // They are not keystrokes which we can intercept in any other way; the OS intercepts them.
   // The menu items had to be created by the application.  That's why we need roles
   // to identify those "special" menu items which can be useful even when non-Qt
   // native widgets are in focus.  When the native widget is focused it will be the
   // first responder, so the menu item needs to have its target be the first responder;
   // this is done by setting it to nil.

   // This function will find all menu items on all menus which have
   // "special" roles, set the target and also set the standard actions which
   // apply to those roles.  But afterwards it is necessary to call
   // resetKnownMenuItemsToQt() to put back the target and action so that
   // those menu items will go back to invoking their associated QActions.

   for (QCocoaMenuBar *mb : static_menubars) {
      for (QCocoaMenu *m : mb->m_menus) {

         for (QCocoaMenuItem *i : m->items()) {
            bool known = true;

            switch (i->effectiveRole()) {
               case QPlatformMenuItem::CutRole:
                  [i->nsItem() setAction: @selector(cut:)];
                  break;

               case QPlatformMenuItem::CopyRole:
                  [i->nsItem() setAction: @selector(copy:)];
                  break;

               case QPlatformMenuItem::PasteRole:
                  [i->nsItem() setAction: @selector(paste:)];
                  break;

               case QPlatformMenuItem::SelectAllRole:
                  [i->nsItem() setAction: @selector(selectAll:)];
                  break;

               // We may discover later that there are other roles/actions which
               // are meaningful to standard native widgets; they can be added.

               default:
                  known = false;
                  break;
            }

            if (known) {
               [i->nsItem() setTarget: nil];
            }
         }
      }
   }

}

void QCocoaMenuBar::resetKnownMenuItemsToQt()
{
   // Undo the effect of redirectKnownMenuItemsToFirstResponder():
   // set the menu items' actions to itemFired and their targets to
   // the QCocoaMenuDelegate.
   updateMenuBarImmediately();
}

void QCocoaMenuBar::updateMenuBarImmediately()
{
   QMacAutoReleasePool pool;
   QCocoaMenuBar *mb = findGlobalMenubar();
   QCocoaWindow *cw  = findWindowForMenubar();

   QWindow *win = cw ? cw->window() : nullptr;

   if (win && (win->flags() & Qt::Popup) == Qt::Popup) {
      // context menus, comboboxes, etc. don't need to update the menubar,
      // but if an application has only Qt::Tool window(s) on start,
      // we still have to update the menubar.
      if ((win->flags() & Qt::WindowType_Mask) != Qt::Tool) {
         return;
      }

      typedef QCocoaApplicationDelegate AppDelegate;

      NSApplication *app = [NSApplication sharedApplication];
      if (! [app.delegate isKindOfClass: [AppDelegate class]]) {
         return;
      }

      // We apply this logic _only_ during the startup.
      AppDelegate *appDelegate = app.delegate;
      if (! appDelegate.inLaunch) {
         return;
      }
   }

   if (cw && cw->menubar()) {
      mb = cw->menubar();
   }

   if (! mb) {
      return;
   }

#ifdef QT_COCOA_ENABLE_MENU_DEBUG
   qDebug() << "QCocoaMenuBar::updateMenuBarImmediately" << cw << ", number of menus" << mb->m_menus.size();
#endif

   bool disableForModal = mb->shouldDisable(cw);

   for (QCocoaMenu *menu : mb->m_menus) {
      if (! menu) {
         continue;
      }

      NSMenuItem *item = mb->nativeItemForMenu(menu);
      menu->setAttachedItem(item);
      menu->setMenuParent(mb);

      // force a sync?
      mb->syncMenu(menu);
      menu->propagateEnabledState(! disableForModal);
   }

#ifdef QT_COCOA_ENABLE_MENU_DEBUG
   qDebug() << "Updated menus\n" << mb->m_nativeMenu;
#endif

   QCocoaMenuLoader *loader = getMenuLoader();
   [loader ensureAppMenuInMenu: mb->nsMenu()];

   NSMutableSet *mergedItems = [[NSMutableSet setWithCapacity: 0] retain];
   for (QCocoaMenuItem *m : mb->merged()) {
      [mergedItems addObject: m->nsItem()];
      m->syncMerged();
   }

   // hide+disable all mergeable items we are not currently using
   for (NSMenuItem * mergeable in [loader mergeable]) {
      if (! [mergedItems containsObject: mergeable]) {
         [mergeable setHidden: YES];
         [mergeable setEnabled: NO];
      }
   }

   [mergedItems release];
   [NSApp setMainMenu: mb->nsMenu()];
   [loader qtTranslateApplicationMenu];
}

QList<QCocoaMenuItem *> QCocoaMenuBar::merged() const
{
   QList<QCocoaMenuItem *> r;
   for (QCocoaMenu *menu : m_menus) {
      r.append(menu->merged());
   }

   return r;
}

bool QCocoaMenuBar::shouldDisable(QCocoaWindow *active) const
{
   if (active && (active->window()->modality() == Qt::NonModal)) {
      return false;
   }

   if (m_window == active) {
      // modal window owns us, we should be enabled!
      return false;
   }

   QWindowList topWindows(qApp->topLevelWindows());

   // When there is an application modal window on screen, the entries of
   // the menubar should be disabled. The exception in CS is that if the
   // modal window is the only window on screen then we enable the menu bar.

   for (QWindow *w : topWindows) {
      if (w->isVisible() && w->modality() == Qt::ApplicationModal) {

         // check for other visible windows
         for (QWindow *other : topWindows) {
            if ((w != other) && (other->isVisible())) {
               // INVARIANT: we found another visible window
               // on screen other than our modalWidget. We therefore
               // disable the menu bar to follow normal modality logic:
               return true;
            }
         }

         // INVARIANT: We have only one window on screen that happends
         // to be application modal. We choose to enable the menu bar
         // in that case to e.g. enable the quit menu item.
         return false;
      }
   }

   return true;
}

QPlatformMenu *QCocoaMenuBar::menuForTag(quintptr tag) const
{
   for (QCocoaMenu *menu : m_menus) {
      if (menu->tag() ==  tag) {
         return menu;
      }
   }

   return nullptr;
}

NSMenuItem *QCocoaMenuBar::itemForRole(QPlatformMenuItem::MenuRole r)
{
   for (QCocoaMenu *m : m_menus) {
      for (QCocoaMenuItem *i : m->items()) {
         if (i->effectiveRole() == r) {
            return i->nsItem();
         }
      }
   }

   return nullptr;
}

