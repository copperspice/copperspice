/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#include <qcocoamenu.h>
#include <qcocoahelpers.h>

#include <qdebug.h>
#include <qmetaobject.h>
#include <qvarlengtharray.h>

#include <qthread_p.h>
#include <qapplication_p.h>

#include <qcocoaapplication.h>
#include <qcocoamenuloader.h>
#include <qcocoamenubar.h>
#include <qcocoawindow.h>

#import <qnsview.h>

NSString *qt_mac_removePrivateUnicode(NSString *string)
{
   int len = [string length];

   if (len) {
      QVarLengthArray <unichar, 10> characters(len);
      bool changed = false;

      for (int i = 0; i < len; i++) {
         characters[i] = [string characterAtIndex: i];
         // check if they belong to key codes in private unicode range
         // currently we need to handle only the NSDeleteFunctionKey

         if (characters[i] == NSDeleteFunctionKey) {
            characters[i] = NSDeleteCharacter;
            changed = true;
         }
      }

      if (changed) {
         return [NSString stringWithCharacters: characters.data() length: len];
      }
   }

   return string;
}

static inline QCocoaMenuLoader *getMenuLoader()
{
   return [NSApp qt_qcocoamenuLoader];
}

@interface QCocoaMenuDelegate : NSObject <NSMenuDelegate>
{
   QCocoaMenu *m_menu;
}

- (id) initWithMenu: (QCocoaMenu *) m;
- (NSMenuItem *)findItem: (NSMenu *)menu forKey: (NSString *)key forModifiers: (NSUInteger)modifier;

@end

@implementation QCocoaMenuDelegate

- (id) initWithMenu: (QCocoaMenu *) m
{
   if ((self = [super init])) {
      m_menu = m;
   }

   return self;
}

- (NSInteger)numberOfItemsInMenu: (NSMenu *)menu
{
   Q_ASSERT(m_menu->nsMenu() == menu);
   return menu.numberOfItems;
}

- (BOOL)menu: (NSMenu *)menu updateItem: (NSMenuItem *)item atIndex: (NSInteger)index shouldCancel: (BOOL)shouldCancel
{
   Q_ASSERT(m_menu->nsMenu() == menu);

   if (shouldCancel) {
      // TODO detach all submenus
      return NO;
   }

   QCocoaMenuItem *menuItem = reinterpret_cast<QCocoaMenuItem *>(item.tag);
   if (m_menu->items().contains(menuItem)) {
      if (QCocoaMenu *itemSubmenu = menuItem->menu()) {
         itemSubmenu->setAttachedItem(item);
      }
   }
   return YES;
}

- (void)menu: (NSMenu *)menu willHighlightItem: (NSMenuItem *)item
{
   Q_UNUSED(menu);
   if (item && [item tag]) {
      QCocoaMenuItem *cocoaItem = reinterpret_cast<QCocoaMenuItem *>([item tag]);
      cocoaItem->hovered();
   }
}

- (void) menuWillOpen: (NSMenu *)m
{
   m_menu->setIsOpen(true);
   emit m_menu->aboutToShow();
}

- (void) menuDidClose: (NSMenu *)m
{
   m_menu->setIsOpen(false);

   // may be incorrect, but it is the best we can do
   emit m_menu->aboutToHide();
}

- (void) itemFired: (NSMenuItem *) item
{
   QCocoaMenuItem *cocoaItem = reinterpret_cast<QCocoaMenuItem *>([item tag]);
   QScopedLoopLevelCounter loopLevelCounter(QApplicationPrivate::instance()->getThreadData());

   QApplicationPrivate::modifier_buttons = [QNSView convertKeyModifiers: [NSEvent modifierFlags]];
   static QMetaMethod activatedSignal = QMetaMethod::fromSignal(&QCocoaMenuItem::activated);
   activatedSignal.invoke(cocoaItem, Qt::QueuedConnection);
}

- (BOOL)validateMenuItem: (NSMenuItem *)menuItem
{
   QCocoaMenuItem *cocoaItem = reinterpret_cast<QCocoaMenuItem *>(menuItem.tag);

   if (! cocoaItem) {
      return YES;
   }

   return cocoaItem->isEnabled();
}

- (BOOL)menuHasKeyEquivalent: (NSMenu *)menu forEvent: (NSEvent *)event target: (id *)target action: (SEL *)action
{
   /*
      Check if the menu actually has a keysequence defined for this key event.
      If it does, then we will first send the key sequence to the QWidget that has focus
      since (in Qt's eyes) it needs to a chance at the key event first (QEvent::ShortcutOverride).
      If the widget accepts the key event, we then return YES, but set the target and action to be nil,
      which means that the action should not be triggered, and instead dispatch the event ourselves.
      In every other case we return NO, which means that Cocoa can do as it pleases
      (i.e., fire the menu action).
   */

   // Change the private unicode keys to the ones used in setting the "Key Equivalents"
   NSString *characters = qt_mac_removePrivateUnicode([event characters]);

   // Interested only in Shift, Cmd, Ctrl & Alt Keys, so ignoring masks like, Caps lock, Num Lock
   const NSUInteger mask = NSEventModifierFlagShift | NSEventModifierFlagControl | NSEventModifierFlagCommand | NSEventModifierFlagOption;

   if (NSMenuItem *menuItem = [self findItem: menu forKey: characters forModifiers: ([event modifierFlags] & mask)]) {
      if (!menuItem.target) {
         // This item was modified by QCocoaMenuBar::redirectKnownMenuItemsToFirstResponder
         // and it looks like we're running a modal session for NSOpenPanel/NSSavePanel.
         // QCocoaFileDialogHelper is actually the only place we use this and we run NSOpenPanel modal
         // (modal sheet, window modal, application modal).
         // Whatever the current first responder is, let's give it a chance
         // and do not touch the Qt's focusObject (which is different from some native view
         // having a focus inside NSSave/OpenPanel.
         *target = nil;
         *action = menuItem.action;
         return YES;
      }

      QObject *object = qApp->focusObject();

      if (object) {
         QChar ch;
         int keyCode;
         ulong nativeModifiers = [event modifierFlags];
         Qt::KeyboardModifiers modifiers = [QNSView convertKeyModifiers: nativeModifiers];
         NSString *charactersIgnoringModifiers = [event charactersIgnoringModifiers];
         NSString *characters = [event characters];

         if ([charactersIgnoringModifiers length] > 0) { // convert the first character into a key code
            if ((modifiers & Qt::ControlModifier) && ([characters length] != 0)) {
               ch = QChar([characters characterAtIndex: 0]);
            } else {
               ch = QChar([charactersIgnoringModifiers characterAtIndex: 0]);
            }
            keyCode = qt_mac_cocoaKey2QtKey(ch);

         } else {
            // might be a dead key
            ch = QChar::ReplacementCharacter;
            keyCode = Qt::Key_unknown;
         }

         QKeyEvent accel_ev(QEvent::ShortcutOverride, (keyCode & (~Qt::KeyboardModifierMask)),
            Qt::KeyboardModifiers(modifiers & Qt::KeyboardModifierMask));
         accel_ev.ignore();

         QCoreApplication::sendEvent(object, &accel_ev);
         if (accel_ev.isAccepted()) {
            [[NSApp keyWindow] sendEvent: event];
            *target = nil;
            *action = nil;
            return YES;
         }
      }
   }
   return NO;
}

- (NSMenuItem *)findItem: (NSMenu *)menu forKey: (NSString *)key forModifiers: (NSUInteger)modifier
{
   for (NSMenuItem * item in [menu itemArray]) {
      if (![item isEnabled] || [item isHidden] || [item isSeparatorItem]) {
         continue;
      }

      if ([item hasSubmenu]) {
         if (NSMenuItem *nested = [self findItem: [item submenu] forKey: key forModifiers: modifier]) {
            return nested;
         }
      }

      NSString *menuKey = [item keyEquivalent];
      if (menuKey && NSOrderedSame == [menuKey compare: key] && modifier == [item keyEquivalentModifierMask]) {
         return item;
      }
   }
   return nil;
}

@end

QCocoaMenu::QCocoaMenu() :
   m_attachedItem(0),
   m_tag(0),
   m_enabled(true),
   m_parentEnabled(true),
   m_visible(true),
   m_isOpen(false)
{
   QMacAutoReleasePool pool;

   m_nativeMenu = [[NSMenu alloc] initWithTitle: @"Untitled"];
   [m_nativeMenu setAutoenablesItems: YES];
   m_nativeMenu.delegate = [[QCocoaMenuDelegate alloc] initWithMenu: this];
}

QCocoaMenu::~QCocoaMenu()
{
   for (QCocoaMenuItem *item : m_menuItems) {
      if (item->menuParent() == this) {
         item->setMenuParent(0);
      }
   }

   QMacAutoReleasePool pool;
   NSObject *delegate = m_nativeMenu.delegate;
   m_nativeMenu.delegate = nil;
   [delegate release];
   [m_nativeMenu release];
}

void QCocoaMenu::setText(const QString &text)
{
   QMacAutoReleasePool pool;

   QString stripped = qt_mac_removeAmpersandEscapes(text);
   [m_nativeMenu setTitle: QCFString::toNSString(stripped)];
}

void QCocoaMenu::setMinimumWidth(int width)
{
   m_nativeMenu.minimumWidth = width;
}

void QCocoaMenu::setFont(const QFont &font)
{
   if (font.resolve()) {
      NSFont *customMenuFont = [NSFont fontWithName: QCFString::toNSString(font.family())
                                               size: font.pointSize()];
      m_nativeMenu.font = customMenuFont;
   }
}

void QCocoaMenu::insertMenuItem(QPlatformMenuItem *menuItem, QPlatformMenuItem *before)
{
   QMacAutoReleasePool pool;
   QCocoaMenuItem *cocoaItem = static_cast<QCocoaMenuItem *>(menuItem);
   QCocoaMenuItem *beforeItem = static_cast<QCocoaMenuItem *>(before);

   cocoaItem->sync();
   if (beforeItem) {
      int index = m_menuItems.indexOf(beforeItem);
      // if a before item is supplied, it should be in the menu
      if (index < 0) {
         qWarning("Before menu item not found");
         return;
      }
      m_menuItems.insert(index, cocoaItem);
   } else {
      m_menuItems.append(cocoaItem);
   }

   insertNative(cocoaItem, beforeItem);
}

void QCocoaMenu::insertNative(QCocoaMenuItem *item, QCocoaMenuItem *beforeItem)
{
   NSMenuItem *nativeItem = item->nsItem();
   nativeItem.target = m_nativeMenu.delegate;
   if (! item->menu()) {
      nativeItem.action = @selector(itemFired:);
   } else if (isOpen() && nativeItem) { // Someone's adding new items after aboutToShow() was emitted
      item->menu()->setAttachedItem(nativeItem);
   }

   item->setParentEnabled(isEnabled());

   if (item->isMerged()) {
      return;
   }

   // if the item we're inserting before is merged, skip along until
   // we find a non-merged real item to insert ahead of.
   while (beforeItem && beforeItem->isMerged()) {
      beforeItem = itemOrNull(m_menuItems.indexOf(beforeItem) + 1);
   }

   if (nativeItem.menu) {
      qWarning() << "Menu item" << item->text() << "already in menu" << QString::fromNSString(nativeItem.menu.title);
      return;
   }

   if (beforeItem) {
      if (beforeItem->isMerged()) {
         qWarning("No non-merged before menu item found");
         return;
      }
      const NSInteger nativeIndex = [m_nativeMenu indexOfItem: beforeItem->nsItem()];
      [m_nativeMenu insertItem: nativeItem atIndex: nativeIndex];
   } else {
      [m_nativeMenu addItem: nativeItem];
   }
   item->setMenuParent(this);
}

bool QCocoaMenu::isOpen() const
{
   return m_isOpen;
}

void QCocoaMenu::setIsOpen(bool isOpen)
{
   m_isOpen = isOpen;
}

void QCocoaMenu::removeMenuItem(QPlatformMenuItem *menuItem)
{
   QMacAutoReleasePool pool;
   QCocoaMenuItem *cocoaItem = static_cast<QCocoaMenuItem *>(menuItem);
   if (!m_menuItems.contains(cocoaItem)) {
      qWarning("Menu does not contain the item to be removed");
      return;
   }

   if (cocoaItem->menuParent() == this) {
      cocoaItem->setMenuParent(0);
   }

   // Ignore any parent enabled state
   cocoaItem->setParentEnabled(true);

   m_menuItems.removeOne(cocoaItem);
   if (!cocoaItem->isMerged()) {
      if (m_nativeMenu != [cocoaItem->nsItem() menu]) {
         qWarning("Item to remove does not belong to this menu");
         return;
      }
      [m_nativeMenu removeItem: cocoaItem->nsItem()];
   }
}

QCocoaMenuItem *QCocoaMenu::itemOrNull(int index) const
{
   if ((index < 0) || (index >= m_menuItems.size())) {
      return 0;
   }

   return m_menuItems.at(index);
}

void QCocoaMenu::syncMenuItem(QPlatformMenuItem *menuItem)
{
   QMacAutoReleasePool pool;
   QCocoaMenuItem *cocoaItem = static_cast<QCocoaMenuItem *>(menuItem);
   if (!m_menuItems.contains(cocoaItem)) {
      qWarning("Item does not belong to this menu");
      return;
   }

   bool wasMerged = cocoaItem->isMerged();
   NSMenu *oldMenu = wasMerged ? [getMenuLoader() applicationMenu] : m_nativeMenu;
   NSMenuItem *oldItem = [oldMenu itemWithTag: (NSInteger) cocoaItem];

   if (cocoaItem->sync() != oldItem) {
      // native item was changed for some reason
      if (oldItem) {
         if (wasMerged) {
            [oldItem setEnabled: NO];
            [oldItem setHidden: YES];
         } else {
            [m_nativeMenu removeItem: oldItem];
         }
      }

      QCocoaMenuItem *beforeItem = itemOrNull(m_menuItems.indexOf(cocoaItem) + 1);
      insertNative(cocoaItem, beforeItem);
   }
}

void QCocoaMenu::syncSeparatorsCollapsible(bool enable)
{
   QMacAutoReleasePool pool;
   if (enable) {
      bool previousIsSeparator = true; // setting to true kills all the separators placed at the top.
      NSMenuItem *previousItem = nil;

      NSArray *itemArray = [m_nativeMenu itemArray];
      for (unsigned int i = 0; i < [itemArray count]; ++i) {
         NSMenuItem *item = reinterpret_cast<NSMenuItem *>([itemArray objectAtIndex: i]);
         if ([item isSeparatorItem]) {
            QCocoaMenuItem *cocoaItem = reinterpret_cast<QCocoaMenuItem *>([item tag]);
            if (cocoaItem) {
               cocoaItem->setVisible(!previousIsSeparator);
            }
            [item setHidden: previousIsSeparator];
         }

         if (![item isHidden]) {
            previousItem = item;
            previousIsSeparator = ([previousItem isSeparatorItem]);
         }
      }

      // We now need to check the final item since we don't want any separators at the end of the list.
      if (previousItem && previousIsSeparator) {
         QCocoaMenuItem *cocoaItem = reinterpret_cast<QCocoaMenuItem *>([previousItem tag]);
         if (cocoaItem) {
            cocoaItem->setVisible(false);
         }
         [previousItem setHidden: YES];
      }

   } else {
      for (QCocoaMenuItem *item : m_menuItems) {
         if (! item->isSeparator()) {
            continue;
         }

         // sync the visiblity directly
         item->sync();
      }
   }
}

void QCocoaMenu::setEnabled(bool enabled)
{
   if (m_enabled == enabled) {
      return;
   }

   m_enabled = enabled;
   const bool wasParentEnabled = m_parentEnabled;
   propagateEnabledState(m_enabled);
   m_parentEnabled = wasParentEnabled; // Reset to the parent value
}

bool QCocoaMenu::isEnabled() const
{
   return m_attachedItem ? [m_attachedItem isEnabled] : m_enabled && m_parentEnabled;
}

void QCocoaMenu::setVisible(bool visible)
{
   m_visible = visible;
}

void QCocoaMenu::showPopup(const QWindow *parentWindow, const QRect &targetRect, const QPlatformMenuItem *item)
{
   QMacAutoReleasePool pool;

   QPoint pos =  QPoint(targetRect.left(), targetRect.top() + targetRect.height());
   QCocoaWindow *cocoaWindow = parentWindow ? static_cast<QCocoaWindow *>(parentWindow->handle()) : 0;
   NSView *view = cocoaWindow ? cocoaWindow->contentView() : nil;
   NSMenuItem *nsItem = item ? ((QCocoaMenuItem *)item)->nsItem() : nil;

   QScreen *screen = nullptr;
   if (parentWindow) {
      screen = parentWindow->screen();
   }

   if (!screen && ! QApplication::screens().isEmpty()) {
      screen = QApplication::screens().at(0);
   }
   Q_ASSERT(screen);

   // Ideally, we would call -popUpMenuPositioningItem:atLocation:inView:.
   // However, this showed not to work with modal windows where the menu items
   // would appear disabled. So, we resort to a more artisanal solution. Note
   // that this implies several things.
   if (nsItem) {
      // If we want to position the menu popup so that a specific item lies under
      // the mouse cursor, we resort to NSPopUpButtonCell to do that. This is the
      // typical use-case for a choice list, or non-editable combobox. We can't
      // re-use the popUpContextMenu:withEvent:forView: logic below since it won't
      // respect the menu's minimum width.
      NSPopUpButtonCell *popupCell = [[[NSPopUpButtonCell alloc] initTextCell: @"" pullsDown: NO]
                                       autorelease];
      [popupCell setAltersStateOfSelectedItem: NO];
      [popupCell setTransparent: YES];
      [popupCell setMenu: m_nativeMenu];
      [popupCell selectItem: nsItem];

      int availableHeight = screen->availableSize().height();
      const QPoint &globalPos = parentWindow->mapToGlobal(pos);
      int menuHeight = m_nativeMenu.size.height;
      if (globalPos.y() + menuHeight > availableHeight) {
         // Maybe we need to fix the vertical popup position but we don't know the
         // exact popup height at the moment (and Cocoa is just guessing) nor its
         // position. So, instead of translating by the popup's full height, we need
         // to estimate where the menu will show up and translate by the remaining height.
         float idx = ([m_nativeMenu indexOfItem: nsItem] + 1.0f) / m_nativeMenu.numberOfItems;
         float heightBelowPos = (1.0 - idx) * menuHeight;
         if (globalPos.y() + heightBelowPos > availableHeight) {
            pos.setY(pos.y() - globalPos.y() + availableHeight - heightBelowPos);
         }
      }

      NSRect cellFrame = NSMakeRect(pos.x(), pos.y(), m_nativeMenu.minimumWidth, 10);
      [popupCell performClickWithFrame: cellFrame inView: view];

   } else {
      // Else, we need to transform 'pos' to window or screen coordinates.
      NSPoint nsPos = NSMakePoint(pos.x() - 1, pos.y());

      if (view) {
         // convert coordinates from view to the view's window
         nsPos = [view convertPoint: nsPos toView: nil];
      } else {
         nsPos.y = screen->availableVirtualSize().height() - nsPos.y;
      }

      if (view) {
         // Finally, we need to synthesize an event.
         NSEvent *menuEvent = [NSEvent mouseEventWithType: NSEventTypeRightMouseDown
                                                 location: nsPos
                                            modifierFlags: 0
                                                timestamp: 0
                                             windowNumber: view ? view.window.windowNumber : 0
                                                 context : nil
                                             eventNumber : 0
                                              clickCount : 1
                                                pressure : 1.0];
         [NSMenu popUpContextMenu: m_nativeMenu withEvent: menuEvent forView: view];

      } else {
         [m_nativeMenu popUpMenuPositioningItem: nsItem atLocation: nsPos inView: 0];
      }
   }

   // The calls above block, and also swallow any mouse release event,
   // so we need to clear any mouse button that triggered the menu popup.
   if ([view isKindOfClass: [QNSView class]]) {
      [(QNSView *)view resetMouseButtons];
   }
}

void QCocoaMenu::dismiss()
{
   [m_nativeMenu cancelTracking];
}

QPlatformMenuItem *QCocoaMenu::menuItemAt(int position) const
{
   if (0 <= position && position < m_menuItems.count()) {
      return m_menuItems.at(position);
   }

   return 0;
}

QPlatformMenuItem *QCocoaMenu::menuItemForTag(quintptr tag) const
{
   for (QCocoaMenuItem *item : m_menuItems) {
      if (item->tag() ==  tag) {
         return item;
      }
   }

   return 0;
}

QList<QCocoaMenuItem *> QCocoaMenu::items() const
{
   return m_menuItems;
}

QList<QCocoaMenuItem *> QCocoaMenu::merged() const
{
   QList<QCocoaMenuItem *> result;

   for (QCocoaMenuItem *item : m_menuItems) {
      if (item->menu()) {
         // recurse into submenus
         result.append(item->menu()->merged());
         continue;
      }

      if (item->isMerged()) {
         result.append(item);
      }
   }

   return result;
}

void QCocoaMenu::propagateEnabledState(bool enabled)
{
   QMacAutoReleasePool pool; // FIXME Is this still needed for Creator? See 6a0bb4206a2928b83648

   m_parentEnabled = enabled;
   if (! m_enabled && enabled) {
      // Some ancestor was enabled, but this menu is not
      return;
   }

   for (QCocoaMenuItem *item : m_menuItems) {
      if (QCocoaMenu *menu = item->menu()) {
         menu->propagateEnabledState(enabled);
      } else {
         item->setParentEnabled(enabled);
      }
   }
}

void QCocoaMenu::setAttachedItem(NSMenuItem *item)
{
   if (item == m_attachedItem) {
      return;
   }

   if (m_attachedItem) {
      m_attachedItem.submenu = nil;
   }

   m_attachedItem = item;

   if (m_attachedItem) {
      m_attachedItem.submenu = m_nativeMenu;
   }

}

NSMenuItem *QCocoaMenu::attachedItem() const
{
   return m_attachedItem;
}

