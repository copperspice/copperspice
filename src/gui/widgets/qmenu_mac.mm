/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#include <cs_carbon_wrapper_p.h>

#include <qmenu.h>
#include <qhash.h>
#include <qdebug.h>
#include <qapplication.h>
#include <qt_mac_p.h>
#include <qregularexpression.h>
#include <qmainwindow.h>
#include <qdockwidget.h>
#include <qtoolbar.h>
#include <qevent.h>
#include <qstyle.h>
#include <qwidgetaction.h>
#include <qmacnativewidget_mac.h>

#include <qapplication_p.h>
#include <qcocoaapplication_mac_p.h>
#include <qmenu_p.h>
#include <qmenubar_p.h>
#include <qcocoamenuloader_mac_p.h>
#include <qcocoamenu_mac_p.h>
#include <qt_cocoa_helpers_mac_p.h>
#include <Cocoa/Cocoa.h>

bool qt_mac_no_menubar_merge = false;
bool qt_mac_quit_menu_item_enabled = true;
int  qt_mac_menus_open_count = 0;

static OSMenuRef qt_mac_create_menu(QWidget *w);

static struct {
   QPointer<QMenuBar> qmenubar;
   bool modal;
} qt_mac_current_menubar = { 0, false };


extern OSViewRef qt_mac_hiview_for(const QWidget *w);
extern HIViewRef qt_mac_hiview_for(OSWindowRef w);
extern IconRef qt_mac_create_iconref(const QPixmap &px);
extern QWidget *mac_keyboard_grabber;
extern bool qt_sendSpontaneousEvent(QObject *, QEvent *);

RgnHandle qt_mac_get_rgn();
void qt_mac_dispose_rgn(RgnHandle r);

bool qt_mac_watchingAboutToShow(QMenu *menu)
{
   return menu && menu->receivers("aboutToShow()");
}

static int qt_mac_CountMenuItems(OSMenuRef menu)
{
   if (menu) {
      return [menu numberOfItems];
   }
   return 0;
}

static quint32 constructModifierMask(quint32 accel_key)
{
   quint32 ret = 0;
   const bool dontSwap = qApp->testAttribute(Qt::AA_MacDontSwapCtrlAndMeta);

   if ((accel_key & Qt::CTRL) == Qt::CTRL) {
      ret |= (dontSwap ? NSControlKeyMask : NSCommandKeyMask);
   }
   if ((accel_key & Qt::META) == Qt::META) {
      ret |= (dontSwap ? NSCommandKeyMask : NSControlKeyMask);
   }
   if ((accel_key & Qt::ALT) == Qt::ALT) {
      ret |= NSAlternateKeyMask;
   }
   if ((accel_key & Qt::SHIFT) == Qt::SHIFT) {
      ret |= NSShiftKeyMask;
   }

   return ret;
}

static void cancelAllMenuTracking()
{
   QMacCocoaAutoReleasePool pool;
   NSMenu *mainMenu = [[NSApplication sharedApplication] mainMenu];
   [mainMenu cancelTracking];
   for (NSMenuItem * item in [mainMenu itemArray]) {
      if ([item submenu]) {
         [[item submenu] cancelTracking];
      }
   }
}

static bool actualMenuItemVisibility(const QMenuBarPrivate::QMacMenuBarPrivate *mbp,
                                     const QMacMenuAction *action)
{
   bool visible = action->action->isVisible();
   if (visible && action->action->text() == QString(QChar(0x14))) {
      return false;
   }
   if (visible && action->action->menu() && !action->action->menu()->actions().isEmpty() &&
         !qt_mac_CountMenuItems(action->action->menu()->macMenu(mbp->apple_menu)) &&
         !qt_mac_watchingAboutToShow(action->action->menu())) {
      return false;
   }
   return visible;
}

static inline void syncNSMenuItemVisiblity(NSMenuItem *menuItem, bool actionVisibility)
{
   [menuItem setHidden: NO];
   [menuItem setHidden: YES];
   [menuItem setHidden: !actionVisibility];
}

static inline void syncNSMenuItemEnabled(NSMenuItem *menuItem, bool enabled)
{
   [menuItem setEnabled: NO];
   [menuItem setEnabled: YES];
   [menuItem setEnabled: enabled];
}

static inline void syncMenuBarItemsVisiblity(const QMenuBarPrivate::QMacMenuBarPrivate *mac_menubar)
{
   const QList<QMacMenuAction *> &menubarActions = mac_menubar->actionItems;
   for (int i = 0; i < menubarActions.size(); ++i) {
      const QMacMenuAction *action = menubarActions.at(i);
      syncNSMenuItemVisiblity(action->menuItem, actualMenuItemVisibility(mac_menubar, action));
   }
}

static inline QT_MANGLE_NAMESPACE(QCocoaMenuLoader) *getMenuLoader()
{
   return [[NSApplication sharedApplication] QT_MANGLE_NAMESPACE(qt_qcocoamenuLoader)];
}

static NSMenuItem *createNSMenuItem(const QString &title)
{
   NSMenuItem *item = [[NSMenuItem alloc]
                       initWithTitle: qt_mac_QStringToNSString(title)
                       action: @selector(qtDispatcherToQAction:) keyEquivalent: @""];
   [item setTarget: nil];
   return item;
}

// helper that recurses into a menu structure and en/dis-ables them
void qt_mac_set_modal_state_helper_recursive(OSMenuRef menu, OSMenuRef merge, bool on)
{
   bool modalWindowOnScreen = qApp->activeModalWidget() != 0;
   for (NSMenuItem * item in [menu itemArray]) {
      OSMenuRef submenu = [item submenu];
      if (submenu != merge) {
         if (submenu) {
            qt_mac_set_modal_state_helper_recursive(submenu, merge, on);
         }
         if (!on) {
            // The item should follow what the QAction has.
            if ([item tag]) {
               QAction *action = reinterpret_cast<QAction *>([item tag]);
               syncNSMenuItemEnabled(item, action->isEnabled());
            } else {
               syncNSMenuItemEnabled(item, YES);
            }
            // We sneak in some extra code here to handle a menu problem:
            // If there is no window on screen, we cannot set 'nil' as
            // menu item target, because then cocoa will disable the item
            // (guess it assumes that there will be no first responder to
            // catch the trigger anyway?) OTOH, If we have a modal window,
            // then setting the menu loader as target will make cocoa not
            // deliver the trigger because the loader is then seen as modally
            // shaddowed). So either way there are shortcomings. Instead, we
            // decide the target as late as possible:
            [item setTarget: modalWindowOnScreen ? nil : getMenuLoader()];
         } else {
            syncNSMenuItemEnabled(item, NO);
         }
      }
   }
}

//toggling of modal state
static void qt_mac_set_modal_state(OSMenuRef menu, bool on)
{
   OSMenuRef merge = QMenuPrivate::mergeMenuHash.value(menu);
   qt_mac_set_modal_state_helper_recursive(menu, merge, on);
   // I'm ignoring the special items now, since they should get handled via a syncAction()
}

bool qt_mac_menubar_is_open()
{
   return qt_mac_menus_open_count > 0;
}

QMacMenuAction::~QMacMenuAction()
{

   [menu release];
   // Update the menu item if this action still owns it. For some items
   // (like 'Quit') ownership will be transferred between all menu bars...
   if (action && action.data() == reinterpret_cast<QAction *>([menuItem tag])) {
      QAction::MenuRole role = action->menuRole();
      // Check if the item is owned by Qt, and should be hidden to keep it from causing
      // problems. Do it for everything but the quit menu item since that should always
      // be visible.
      if (role > QAction::ApplicationSpecificRole && role < QAction::QuitRole) {
         [menuItem setHidden: YES];
      } else if (role == QAction::TextHeuristicRole
                 && menuItem != [getMenuLoader() quitMenuItem]) {
         [menuItem setHidden: YES];
      }
      [menuItem setTag: 0];
   }
   [menuItem release];

}

static NSMenuItem *qt_mac_menu_merge_action(OSMenuRef merge, QMacMenuAction *action)
{
   if (qt_mac_no_menubar_merge || action->action->menu() || action->action->isSeparator()
         || action->action->menuRole() == QAction::NoRole) {
      return 0;
   }

   QString t = qt_mac_removeMnemonics(action->action->text().toLower());
   int st = t.lastIndexOf('\t');

   if (st != -1) {
      t.remove(st, t.length() - st);
   }

   static QRegularExpression regExp("\\.+$");
   t.replace(regExp, "");                         // no ellipses

   NSMenuItem *ret = 0;
   QT_MANGLE_NAMESPACE(QCocoaMenuLoader) *loader = getMenuLoader();

   switch (action->action->menuRole()) {

      case QAction::NoRole:
         ret = 0;
         break;

      case QAction::ApplicationSpecificRole:
         ret = [loader appSpecificMenuItem: reinterpret_cast<NSInteger>(action)];

         break;
      case QAction::AboutRole:
         ret = [loader aboutMenuItem];
         break;

      case QAction::AboutCsRole:
         ret = [loader aboutCsMenuItem];
         break;

      case QAction::QuitRole:
         ret = [loader quitMenuItem];
         break;

      case QAction::PreferencesRole:
         ret = [loader preferencesMenuItem];
         break;

      case QAction::TextHeuristicRole: {
         QString aboutString = QMenuBar::tr("About").toLower();

         if (t.startsWith(aboutString) || t.endsWith(aboutString)) {
            static QRegularExpression regExp("qt$", QPatternOption::CaseInsensitiveOption);

            if (! t.contains(regExp)) {
               ret = [loader aboutMenuItem];

            } else {
               ret = [loader aboutCsMenuItem];

            }

         } else if (t.startsWith(QMenuBar::tr("Config").toLower())
                 || t.startsWith(QMenuBar::tr("Preference").toLower())
                 || t.startsWith(QMenuBar::tr("Options").toLower())
                 || t.startsWith(QMenuBar::tr("Setting").toLower())
                 || t.startsWith(QMenuBar::tr("Setup").toLower())) {

            ret = [loader preferencesMenuItem];

         } else if (t.startsWith(QMenuBar::tr("Quit").toLower())
                    || t.startsWith(QMenuBar::tr("Exit").toLower())) {

            ret = [loader quitMenuItem];

         }
      }

      break;
   }


   if (QMenuMergeList *list = QMenuPrivate::mergeMenuItemsHash.value(merge)) {
      for (int i = 0; i < list->size(); ++i) {
         const QMenuMergeItem &item = list->at(i);

         if (item.menuItem == ret && item.action) {
            return 0;
         }
      }
   }

   return ret;
}

static QString qt_mac_menu_merge_text(QMacMenuAction *action)
{
   QString ret;
   extern QString qt_mac_applicationmenu_string(int type);

   QT_MANGLE_NAMESPACE(QCocoaMenuLoader) *loader = getMenuLoader();

   if (action->action->menuRole() == QAction::ApplicationSpecificRole) {
      ret = action->action->text();
   }

   else if (action->menuItem == [loader aboutMenuItem]) {
      ret = qt_mac_applicationmenu_string(6).formatArg(qAppName());

   } else if (action->menuItem == [loader aboutCsMenuItem]) {

      if (action->action->text() == QString("About CopperSpice")) {
         ret = QMenuBar::tr("About CopperSpice");
      } else {
         ret = action->action->text();
      }

   } else if (action->menuItem == [loader preferencesMenuItem]) {
      ret = qt_mac_applicationmenu_string(4);

   } else if (action->menuItem == [loader quitMenuItem]) {
      ret = qt_mac_applicationmenu_string(5).formatArg(qAppName());
   }

   return ret;
}

static QKeySequence qt_mac_menu_merge_accel(QMacMenuAction *action)
{
   QKeySequence ret;

   QT_MANGLE_NAMESPACE(QCocoaMenuLoader) *loader = getMenuLoader();

   if (action->action->menuRole() == QAction::ApplicationSpecificRole) {
      ret = action->action->shortcut();
   }

   else if (action->menuItem == [loader preferencesMenuItem]) {
      ret = QKeySequence(QKeySequence::Preferences);
   } else if (action->menuItem == [loader quitMenuItem]) {
      ret = QKeySequence(QKeySequence::Quit);
   }

   return ret;
}

void Q_GUI_EXPORT qt_mac_set_menubar_icons(bool b)
{
   QApplication::instance()->setAttribute(Qt::AA_DontShowIconsInMenus, !b);
}

void Q_GUI_EXPORT qt_mac_set_native_menubar(bool b)
{
   QApplication::instance()->setAttribute(Qt::AA_DontUseNativeMenuBar, !b);
}

void Q_GUI_EXPORT qt_mac_set_menubar_merge(bool b)
{
   qt_mac_no_menubar_merge = !b;
}

/*****************************************************************************
  QMenu bindings
 *****************************************************************************/
QMenuPrivate::QMacMenuPrivate::QMacMenuPrivate() : menu(0)
{
}

QMenuPrivate::QMacMenuPrivate::~QMacMenuPrivate()
{
   QMacCocoaAutoReleasePool pool;
   while (actionItems.size()) {
      QMacMenuAction *action = actionItems.takeFirst();
      if (QMenuMergeList *list = mergeMenuItemsHash.value(action->menu)) {
         int i = 0;
         while (i < list->size()) {
            const QMenuMergeItem &item = list->at(i);
            if (item.action == action) {
               list->removeAt(i);
            } else {
               ++i;
            }
         }
      }
      delete action;
   }
   mergeMenuHash.remove(menu);
   mergeMenuItemsHash.remove(menu);
   [menu release];
}

void QMenuPrivate::QMacMenuPrivate::addAction(QAction *a, QMacMenuAction *before, QMenuPrivate *qmenu)
{
   QMacMenuAction *action = new QMacMenuAction;
   action->action = a;
   action->ignore_accel = 0;
   action->merged = 0;
   action->menu = 0;

   addAction(action, before, qmenu);
}

void QMenuPrivate::QMacMenuPrivate::addAction(QMacMenuAction *action, QMacMenuAction *before, QMenuPrivate *qmenu)
{
   QMacCocoaAutoReleasePool pool;
   Q_UNUSED(qmenu);

   if (!action) {
      return;
   }
   int before_index = actionItems.indexOf(before);
   if (before_index < 0) {
      before = 0;
      before_index = actionItems.size();
   }
   actionItems.insert(before_index, action);

   [menu retain];
   [action->menu release];

   action->menu = menu;

   /* When the action is considered a mergable action it
      will stay that way, until removed.. */
   if (!qt_mac_no_menubar_merge) {

      OSMenuRef merge = QMenuPrivate::mergeMenuHash.value(menu);

      if (merge) {

         if (NSMenuItem *cmd = qt_mac_menu_merge_action(merge, action)) {
            action->merged = 1;
            [merge retain];
            [action->menu release];

            action->menu = merge;

            [cmd retain];
            [cmd setAction: @selector(qtDispatcherToQAction:)];
            [cmd setTarget: nil];
            [action->menuItem release];
            action->menuItem = cmd;

            QMenuMergeList *list = QMenuPrivate::mergeMenuItemsHash.value(merge);

            if (! list) {
               list = new QMenuMergeList;
               QMenuPrivate::mergeMenuItemsHash.insert(merge, list);
            }

            list->append(QMenuMergeItem(cmd, action));
         }

      }
   }


   NSMenuItem *newItem = action->menuItem;

   if (newItem == 0) {

      newItem = createNSMenuItem(action->action->text());
      action->menuItem = newItem;

      if (before) {
         [menu insertItem: newItem atIndex: qMax(before_index, 0)];

      } else {
         [menu addItem: newItem];

      }

      QWidget *widget = qmenu ? qmenu->widgetItems.value(action->action) : 0;

      if (widget) {
         QMacNativeWidget *container = new QMacNativeWidget(0);
         container->resize(widget->sizeHint());
         widget->setAttribute(Qt::WA_LayoutUsesWidgetRect);
         widget->setParent(container);

         NSView *containerView = qt_mac_nativeview_for(container);
         [containerView setAutoresizesSubviews: YES];
         [containerView setAutoresizingMask: NSViewWidthSizable];
         [qt_mac_nativeview_for(widget) setAutoresizingMask: NSViewWidthSizable];

         [newItem setView: containerView];
         container->show();
         widget->show();
      }

   } else {

      [newItem setEnabled: !QApplicationPrivate::modalState()];

   }

   [newItem setTag: long(static_cast<QAction *>(action->action))];
   syncAction(action);
}

// return an autoreleased string given a QKeySequence (currently only looks at the first one).
NSString *keySequenceToKeyEqivalent(const QKeySequence &accel)
{
   char32_t accel_key = (accel[0] & ~(Qt::MODIFIER_MASK | Qt::UNICODE_ACCEL));
   extern QChar qtKey2CocoaKey(Qt::Key key);

   QString cocoa_key = qtKey2CocoaKey(Qt::Key(accel_key));

   if (cocoa_key.isEmpty()) {
      cocoa_key = QChar(accel_key).toLower();
   }

   return cocoa_key.toNSString();
}

// return the cocoa modifier mask for the QKeySequence (currently only looks at the first one).
NSUInteger keySequenceModifierMask(const QKeySequence &accel)
{
   return constructModifierMask(accel[0]);
}

void QMenuPrivate::QMacMenuPrivate::syncAction(QMacMenuAction *action)
{
   if (! action) {
      return;
   }

   NSMenuItem *item = action->menuItem;
   if (!item) {
      return;
   }

   QMacCocoaAutoReleasePool pool;
   NSMenu *menu = [item menu];
   bool actionVisible = action->action->isVisible();
   [item setHidden: !actionVisible];
   if (!actionVisible) {
      return;
   }

   int itemIndex = [menu indexOfItem: item];
   Q_ASSERT(itemIndex != -1);
   if (action->action->isSeparator()) {
      action->menuItem = [NSMenuItem separatorItem];
      [action->menuItem retain];
      [menu insertItem: action->menuItem atIndex: itemIndex];
      [menu removeItem: item];
      [item release];
      item = action->menuItem;
      return;
   } else if ([item isSeparatorItem]) {
      // I'm no longer a separator...
      action->menuItem = createNSMenuItem(action->action->text());
      [menu insertItem: action->menuItem atIndex: itemIndex];
      [menu removeItem: item];
      [item release];
      item = action->menuItem;
   }

   //find text (and accel)
   action->ignore_accel = 0;
   QString text = action->action->text();
   QKeySequence accel = action->action->shortcut();
   {
      int st = text.lastIndexOf(QLatin1Char('\t'));
      if (st != -1) {
         action->ignore_accel = 1;
         accel = QKeySequence(text.right(text.length() - (st + 1)));
         text.remove(st, text.length() - st);
      }
   }
   {
      QString cmd_text = qt_mac_menu_merge_text(action);
      if (!cmd_text.isEmpty()) {
         text = cmd_text;
         accel = qt_mac_menu_merge_accel(action);
      }
   }
   // Show multiple key sequences as part of the menu text.
   if (accel.count() > 1) {
      text += QLatin1String(" (") + accel.toString(QKeySequence::NativeText) + QLatin1String(")");
   }

   QString finalString = qt_mac_removeMnemonics(text);

   // Cocoa Font and title
   if (action->action->font().resolve()) {
      const QFont &actionFont = action->action->font();
      NSFont *customMenuFont = [NSFont fontWithName: qt_mac_QStringToNSString(actionFont.family())
                                size: actionFont.pointSize()];
      NSArray *keys = [NSArray arrayWithObjects: NSFontAttributeName, nil];
      NSArray *objects = [NSArray arrayWithObjects: customMenuFont, nil];
      NSDictionary *attributes = [NSDictionary dictionaryWithObjects: objects forKeys: keys];
      NSAttributedString *str = [[[NSAttributedString alloc] initWithString: qt_mac_QStringToNSString(finalString)
                                  attributes: attributes] autorelease];
      [item setAttributedTitle: str];
   } else {
      [item setTitle: qt_mac_QStringToNSString(finalString)];
   }

   if (action->action->menuRole() == QAction::AboutRole || action->action->menuRole() == QAction::QuitRole) {
      [item setTitle: qt_mac_QStringToNSString(text)];
   } else {
      [item setTitle: qt_mac_QStringToNSString(qt_mac_removeMnemonics(text))];
   }

   // Cocoa Enabled
   [item setEnabled: action->action->isEnabled()];

   // Cocoa icon
   NSImage *nsimage = 0;
   if (!action->action->icon().isNull() && action->action->isIconVisibleInMenu()) {
      nsimage = static_cast<NSImage *>(qt_mac_create_nsimage(action->action->icon().pixmap(16, QIcon::Normal)));
   }
   [item setImage: nsimage];
   [nsimage release];


   if (action->action->menu()) {
      //submenu

      NSMenu *subMenu  = static_cast<NSMenu *>(action->action->menu()->macMenu());
      if ([subMenu supermenu] && [subMenu supermenu] != [item menu]) {
         // The menu is already a sub-menu of another one. Cocoa will throw an exception,
         // in such cases. For the time being, a new QMenu with same set of actions is the
         // only workaround.
         action->action->setEnabled(false);
      } else {
         [item setSubmenu: subMenu];
      }
      [item setAction: nil];

   } else {
      //respect some other items

      [item setSubmenu: 0];
      if ([item action] == nil) {
         [item setAction: @selector(qtDispatcherToQAction:)];
      }
      // No key equivalent set for multiple key QKeySequence.
      if (accel.count() == 1) {
         [item setKeyEquivalent: keySequenceToKeyEqivalent(accel)];
         [item setKeyEquivalentModifierMask: keySequenceModifierMask(accel)];
      } else {
         [item setKeyEquivalent: @""];
         [item setKeyEquivalentModifierMask: NSCommandKeyMask];
      }

   }

   //mark glyph
   [item setState: action->action->isChecked() ?  NSOnState : NSOffState];
}

void
QMenuPrivate::QMacMenuPrivate::removeAction(QMacMenuAction *action)
{
   if (!action) {
      return;
   }

   QMacCocoaAutoReleasePool pool;
   if (action->merged) {
      if (reinterpret_cast<QAction *>([action->menuItem tag]) == action->action) {
         QT_MANGLE_NAMESPACE(QCocoaMenuLoader) *loader = getMenuLoader();
         [action->menuItem setEnabled: false];
         if (action->menuItem != [loader quitMenuItem]
               && action->menuItem != [loader preferencesMenuItem]) {
            [[action->menuItem menu] removeItem: action->menuItem];
         }
         if (QMenuMergeList *list = mergeMenuItemsHash.value(action->menu)) {
            int i = 0;
            while (i < list->size()) {
               const QMenuMergeItem &item = list->at(i);
               if (item.action == action) {
                  list->removeAt(i);
               } else {
                  ++i;
               }
            }
         }
      }
   } else {
      [[action->menuItem menu] removeItem: action->menuItem];
   }

   actionItems.removeAll(action);
}

OSMenuRef
QMenuPrivate::macMenu(OSMenuRef merge)
{
   Q_UNUSED(merge);
   Q_Q(QMenu);
   if (mac_menu && mac_menu->menu) {
      return mac_menu->menu;
   }
   if (!mac_menu) {
      mac_menu = new QMacMenuPrivate;
   }
   mac_menu->menu = qt_mac_create_menu(q);

   if (merge) {
      mergeMenuHash.insert(mac_menu->menu, merge);
   }

   QList<QAction *> items = q->actions();
   for (int i = 0; i < items.count(); i++) {
      mac_menu->addAction(items[i], 0, this);
   }
   syncSeparatorsCollapsible(collapsibleSeparators);
   return mac_menu->menu;
}

/*!
  \internal
*/
void
QMenuPrivate::syncSeparatorsCollapsible(bool collapse)
{
   qt_mac_menu_collapseSeparators(mac_menu->menu, collapse);
}

OSMenuRef QMenu::macMenu(OSMenuRef merge)
{
   return d_func()->macMenu(merge);
}

typedef QHash<QWidget *, QMenuBar *> MenuBarHash;
Q_GLOBAL_STATIC(MenuBarHash, menubars)
static QMenuBar *fallback = 0;

QMenuBarPrivate::QMacMenuBarPrivate::QMacMenuBarPrivate() : menu(0), apple_menu(0)
{
}

QMenuBarPrivate::QMacMenuBarPrivate::~QMacMenuBarPrivate()
{
   for (auto item : actionItems) {
      delete item;
   }

   [apple_menu release];
   [menu release];

}

void QMenuBarPrivate::QMacMenuBarPrivate::addAction(QAction *a, QAction *before)
{
   if (a->isSeparator() || !menu) {
      return;
   }

   QMacMenuAction *action = new QMacMenuAction;
   action->action = a;
   action->ignore_accel = 1;

   addAction(action, findAction(before));
}

void QMenuBarPrivate::QMacMenuBarPrivate::addAction(QMacMenuAction *action, QMacMenuAction *before)
{
   if (!action || !menu) {
      return;
   }

   int before_index = actionItems.indexOf(before);
   if (before_index < 0) {
      before = 0;
      before_index = actionItems.size();
   }
   actionItems.insert(before_index, action);

   MenuItemIndex index = actionItems.size() - 1;

   action->menu = menu;

   QMacCocoaAutoReleasePool pool;
   [action->menu retain];
   NSMenuItem *newItem = createNSMenuItem(action->action->text());
   action->menuItem = newItem;

   if (before) {
      [menu insertItem: newItem atIndex: qMax(1, before_index + 1)];
      index = before_index;

   } else {
      [menu addItem: newItem];

   }

   [newItem setTag: long(static_cast<QAction *>(action->action))];
   syncAction(action);
}

void QMenuBarPrivate::QMacMenuBarPrivate::syncAction(QMacMenuAction *action)
{
   if (!action || !menu) {
      return;
   }

   QMacCocoaAutoReleasePool pool;
   NSMenuItem *item = action->menuItem;

   OSMenuRef submenu = 0;
   bool release_submenu = false;
   if (action->action->menu()) {
      if ((submenu = action->action->menu()->macMenu(apple_menu))) {

         if ([submenu supermenu] && [submenu supermenu] != [item menu]) {
            return;
         } else {
            [item setSubmenu: submenu];
         }

      }

   }

   if (submenu) {
      bool visible = actualMenuItemVisibility(this, action);

      [item setSubmenu: submenu];
      [submenu setTitle: qt_mac_QStringToNSString(qt_mac_removeMnemonics(action->action->text()))];
      syncNSMenuItemVisiblity(item, visible);
      syncNSMenuItemEnabled(item, action->action->isEnabled());

      if (release_submenu) {
         //no pointers to it
         [submenu release];

      }
   } else {
      qWarning("QMenu: No OSMenuRef created for popup menu");
   }
}

void QMenuBarPrivate::QMacMenuBarPrivate::removeAction(QMacMenuAction *action)
{
   if (!action || !menu) {
      return;
   }

   QMacCocoaAutoReleasePool pool;
   [action->menu removeItem: action->menuItem];

   actionItems.removeAll(action);
}

bool QMenuBarPrivate::macWidgetHasNativeMenubar(QWidget *widget)
{
   // This function is different from q->isNativeMenuBar(), as
   // it returns true only if a native menu bar is actually
   // _created_.
   if (!widget) {
      return false;
   }
   return menubars()->contains(widget->window());
}

void QMenuBarPrivate::macCreateMenuBar(QWidget *parent)
{
   Q_Q(QMenuBar);
   static int dontUseNativeMenuBar = -1;

   // We call the isNativeMenuBar function here
   // because that will make sure that local overrides
   // are dealt with correctly. q->isNativeMenuBar() will, if not
   // overridden, depend on the attribute Qt::AA_DontUseNativeMenuBar:

   bool qt_mac_no_native_menubar = !q->isNativeMenuBar();

   if (qt_mac_no_native_menubar == false && dontUseNativeMenuBar < 0) {
      // The menubar is set to be native. Let's check (one time only
      // for all menubars) if this is OK with the rest of the environment.
      // As a result, Qt::AA_DontUseNativeMenuBar is set. NB: the application
      // might still choose to not respect, or change, this flag.
      bool isPlugin = QApplication::testAttribute(Qt::AA_MacPluginApplication);
      bool environmentSaysNo = !qgetenv("QT_MAC_NO_NATIVE_MENUBAR").isEmpty();
      dontUseNativeMenuBar = isPlugin || environmentSaysNo;
      QApplication::instance()->setAttribute(Qt::AA_DontUseNativeMenuBar, dontUseNativeMenuBar);
      qt_mac_no_native_menubar = !q->isNativeMenuBar();
   }

   if (qt_mac_no_native_menubar == false) {
      // INVARIANT: Use native menubar.
      extern void qt_event_request_menubarupdate(); //qapplication_mac.cpp
      qt_event_request_menubarupdate();
      if (!parent && !fallback) {
         fallback = q;
         mac_menubar = new QMacMenuBarPrivate;
      } else if (parent && parent->isWindow()) {
         menubars()->insert(q->window(), q);
         mac_menubar = new QMacMenuBarPrivate;
      }
   }
}

void QMenuBarPrivate::macDestroyMenuBar()
{
   Q_Q(QMenuBar);
   QMacCocoaAutoReleasePool pool;
   if (fallback == q) {
      fallback = 0;
   }
   delete mac_menubar;
   QWidget *tlw = q->window();
   menubars()->remove(tlw);
   mac_menubar = 0;

   if (!qt_mac_current_menubar.qmenubar || qt_mac_current_menubar.qmenubar == q) {
      QT_MANGLE_NAMESPACE(QCocoaMenuLoader) *loader = getMenuLoader();
      [loader removeActionsFromAppMenu];

      extern void qt_event_request_menubarupdate(); //qapplication_mac.cpp
      qt_event_request_menubarupdate();
   }
}

OSMenuRef QMenuBarPrivate::macMenu()
{
   Q_Q(QMenuBar);

   if (!q->isNativeMenuBar() || !mac_menubar) {
      return 0;

   } else if (!mac_menubar->menu) {
      mac_menubar->menu = qt_mac_create_menu(q);
      [mac_menubar->menu setAutoenablesItems: NO];
      ProcessSerialNumber mine, front;
      if (GetCurrentProcess(&mine) == noErr && GetFrontProcess(&front) == noErr) {
         if (!qt_mac_no_menubar_merge && !mac_menubar->apple_menu) {
            mac_menubar->apple_menu = qt_mac_create_menu(q);

            [mac_menubar->apple_menu setTitle: qt_mac_QStringToNSString(QString(QChar(0x14)))];
            NSMenuItem *apple_menuItem = [[NSMenuItem alloc] init];
            [apple_menuItem setSubmenu: mac_menubar->menu];
            [mac_menubar->apple_menu addItem: apple_menuItem];
            [apple_menuItem release];

         }
         if (mac_menubar->apple_menu) {
            QMenuPrivate::mergeMenuHash.insert(mac_menubar->menu, mac_menubar->apple_menu);
         }

         QList<QAction *> items = q->actions();
         for (int i = 0; i < items.count(); i++) {
            mac_menubar->addAction(items[i]);
         }
      }
   }
   return mac_menubar->menu;
}


OSMenuRef QMenuBar::macMenu()
{
   return d_func()->macMenu();
}

static bool qt_mac_is_ancestor(QWidget *possibleAncestor, QWidget *child)
{
   if (!possibleAncestor) {
      return false;
   }

   QWidget *current = child->parentWidget();
   while (current != 0) {
      if (current == possibleAncestor) {
         return true;
      }
      current = current->parentWidget();
   }
   return false;
}

static bool qt_mac_should_disable_menu(QMenuBar *menuBar)
{
   QWidget *modalWidget = qApp->activeModalWidget();
   if (!modalWidget) {
      return false;
   }

   if (menuBar && menuBar == menubars()->value(modalWidget))
      // The menu bar is owned by the modal widget.
      // In that case we should enable it:
   {
      return false;
   }

   // When there is an application modal window on screen, the entries of
   // the menubar should be disabled. The exception in Qt is that if the
   // modal window is the only window on screen, then we enable the menu bar.
   QWidget *w = modalWidget;
   QWidgetList topLevelWidgets = QApplication::topLevelWidgets();

   while (w) {
      if (w->isVisible() && w->windowModality() == Qt::ApplicationModal) {
         for (int i = 0; i < topLevelWidgets.size(); ++i) {
            QWidget *top = topLevelWidgets.at(i);
            if (w != top && top->isVisible()) {
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
      w = w->parentWidget();
   }

   // INVARIANT: modalWidget is window modal. Disable menu entries
   // if the menu bar belongs to an ancestor of modalWidget. If menuBar
   // is nil, we understand it as the default menu bar set by the nib:
   return menuBar ? qt_mac_is_ancestor(menuBar->parentWidget(), modalWidget) : false;
}

static QWidget *findWindowThatShouldDisplayMenubar()
{
   QWidget *w = qApp->activeWindow();
   if (!w) {
      // We have no active window on screen. Try to
      // find a window from the list of top levels:
      QWidgetList tlws = QApplication::topLevelWidgets();
      for (int i = 0; i < tlws.size(); ++i) {
         QWidget *tlw = tlws.at(i);
         if ((tlw->isVisible() && tlw->windowType() != Qt::Tool &&
               tlw->windowType() != Qt::Popup)) {
            w = tlw;
            break;
         }
      }
   }
   return w;
}

static QMenuBar *findMenubarForWindow(QWidget *w)
{
   QMenuBar *mb = 0;
   if (w) {
      mb = menubars()->value(w);
#ifndef QT_NO_MAINWINDOW
      QDockWidget *dw = qobject_cast<QDockWidget *>(w);
      if (!mb && dw) {
         QMainWindow *mw = qobject_cast<QMainWindow *>(dw->parentWidget());
         if (mw && (mb = menubars()->value(mw))) {
            w = mw;
         }
      }
#endif
      while (w && !mb) {
         mb = menubars()->value((w = w->parentWidget()));
      }
   }

   if (!mb) {
      // We could not find a menu bar for the window. Lets
      // check if we have a global (parentless) menu bar instead:
      mb = fallback;
   }

   return mb;
}

void qt_mac_clear_menubar()
{
   if (QApplication::testAttribute(Qt::AA_MacPluginApplication)) {
      return;
   }

   QMacCocoaAutoReleasePool pool;
   QT_MANGLE_NAMESPACE(QCocoaMenuLoader) *loader = getMenuLoader();
   NSMenu *menu = [loader menu];
   [loader ensureAppMenuInMenu: menu];
   [[NSApplication sharedApplication] setMainMenu: menu];
   const bool modal = qt_mac_should_disable_menu(0);
   if (qt_mac_current_menubar.qmenubar || modal != qt_mac_current_menubar.modal) {
      qt_mac_set_modal_state(menu, modal);
   }
   qt_mac_current_menubar.qmenubar = 0;
   qt_mac_current_menubar.modal = modal;

}


bool QMenuBar::macUpdateMenuBar()
{
   QMacCocoaAutoReleasePool pool;
   qt_cocoaPostMessage(getMenuLoader(), @selector(qtUpdateMenubar));
   return true;
}

bool QMenuBarPrivate::macUpdateMenuBarImmediatly()
{
   bool ret = false;
   cancelAllMenuTracking();
   QWidget *w = findWindowThatShouldDisplayMenubar();
   QMenuBar *mb = findMenubarForWindow(w);
   extern bool qt_mac_app_fullscreen; //qapplication_mac.mm

   // We need to see if we are in full screen mode, if so we need to
   // switch the full screen mode to be able to show or hide the menubar.
   if (w && mb) {
      // This case means we are creating a menubar, check if full screen
      if (w->isFullScreen()) {
         // Ok, switch to showing the menubar when hovering over it.
         CS_SetSystemUIMode(kUIModeAllHidden, kUIOptionAutoShowMenuBar);
         qt_mac_app_fullscreen = true;
      }
   } else if (w) {
      // Removing a menubar
      if (w->isFullScreen()) {
         // Ok, switch to not showing the menubar when hovering on it
         CS_SetSystemUIMode(kUIModeAllHidden, 0);
         qt_mac_app_fullscreen = true;
      }
   }

   if (mb && mb->isNativeMenuBar()) {
      bool modal = QApplicationPrivate::modalState();
      QMacCocoaAutoReleasePool pool;

      if (OSMenuRef menu = mb->macMenu()) {

         QT_MANGLE_NAMESPACE(QCocoaMenuLoader) *loader = getMenuLoader();
         [loader ensureAppMenuInMenu: menu];
         [[NSApplication sharedApplication] setMainMenu: menu];
         syncMenuBarItemsVisiblity(mb->d_func()->mac_menubar);

         if (OSMenuRef tmpMerge = QMenuPrivate::mergeMenuHash.value(menu)) {
            if (QMenuMergeList * mergeList
                  = QMenuPrivate::mergeMenuItemsHash.value(tmpMerge)) {
               const int mergeListSize = mergeList->size();

               for (int i = 0; i < mergeListSize; ++i) {
                  const QMenuMergeItem &mergeItem = mergeList->at(i);
                  // Ideally we would call QMenuPrivate::syncAction, but that requires finding
                  // the original QMen and likely doing more work than we need.
                  // For example, enabled is handled below.
                  [mergeItem.menuItem setTag: reinterpret_cast<long>(
                      static_cast<QAction *>(mergeItem.action->action))];
                  [mergeItem.menuItem setHidden: !(mergeItem.action->action->isVisible())];
               }
            }
         }

         // Check if menu is modally shaddowed and should  be disabled:
         modal = qt_mac_should_disable_menu(mb);
         if (mb != qt_mac_current_menubar.qmenubar || modal != qt_mac_current_menubar.modal) {
            qt_mac_set_modal_state(menu, modal);
         }
      }
      qt_mac_current_menubar.qmenubar = mb;
      qt_mac_current_menubar.modal = modal;
      ret = true;
   } else if (qt_mac_current_menubar.qmenubar && qt_mac_current_menubar.qmenubar->isNativeMenuBar()) {
      // INVARIANT: The currently active menu bar (if any) is not native. But we do have a
      // native menu bar from before. So we need to decide whether or not is should be enabled:
      const bool modal = qt_mac_should_disable_menu(qt_mac_current_menubar.qmenubar);
      if (modal != qt_mac_current_menubar.modal) {
         ret = true;
         if (OSMenuRef menu = qt_mac_current_menubar.qmenubar->macMenu()) {

            QT_MANGLE_NAMESPACE(QCocoaMenuLoader) *loader = getMenuLoader();
            [loader ensureAppMenuInMenu: menu];
            [[NSApplication sharedApplication] setMainMenu: menu];
            syncMenuBarItemsVisiblity(qt_mac_current_menubar.qmenubar->d_func()->mac_menubar);

            qt_mac_set_modal_state(menu, modal);
         }
         qt_mac_current_menubar.modal = modal;
      }
   }

   if (!ret) {
      qt_mac_clear_menubar();
   }
   return ret;
}

QHash<OSMenuRef, OSMenuRef> QMenuPrivate::mergeMenuHash;
QHash<OSMenuRef, QMenuMergeList *> QMenuPrivate::mergeMenuItemsHash;

bool QMenuPrivate::QMacMenuPrivate::merged(const QAction *action) const
{
   if (OSMenuRef merge = mergeMenuHash.value(menu)) {
      if (QMenuMergeList *list = mergeMenuItemsHash.value(merge)) {
         for (int i = 0; i < list->size(); ++i) {
            const QMenuMergeItem &item = list->at(i);
            if (item.action->action == action) {
               return true;
            }
         }
      }
   }

   return false;
}

//creation of the OSMenuRef
static OSMenuRef qt_mac_create_menu(QWidget *w)
{
   OSMenuRef ret;

   if (QMenu *qmenu = qobject_cast<QMenu *>(w)) {
      ret = [[QT_MANGLE_NAMESPACE(QCocoaMenu) alloc] initWithQMenu: qmenu];
   } else {
      ret = [[NSMenu alloc] init];
   }

   return ret;
}


