/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

/***********************************************************************
** Copyright (C) 2007-2008, Apple, Inc.
***********************************************************************/

#define QT_MAC_SYSTEMTRAY_USE_GROWL

#include <qt_cocoa_helpers_mac_p.h>
#include <qsystemtrayicon_p.h>
#include <qtemporaryfile.h>
#include <qimagewriter.h>
#include <qapplication.h>
#include <qdebug.h>
#include <qstyle.h>

#include <qt_mac_p.h>
#import <AppKit/AppKit.h>

QT_BEGIN_NAMESPACE
extern bool qt_mac_execute_apple_script(const QString &script, AEDesc *ret); //qapplication_mac.cpp
extern void qtsystray_sendActivated(QSystemTrayIcon *i, int r); //qsystemtrayicon.cpp
extern NSString *keySequenceToKeyEqivalent(const QKeySequence &accel); // qmenu_mac.mm
extern NSUInteger keySequenceModifierMask(const QKeySequence &accel);  // qmenu_mac.mm
extern Qt::MouseButton cocoaButton2QtButton(NSInteger buttonNum);
QT_END_NAMESPACE

QT_USE_NAMESPACE

@class QT_MANGLE_NAMESPACE(QNSMenu);
@class QT_MANGLE_NAMESPACE(QNSImageView);

@interface QT_MANGLE_NAMESPACE(QNSStatusItem) : NSObject <NSUserNotificationCenterDelegate>
{
   NSStatusItem *item;
   QSystemTrayIcon *icon;
   QSystemTrayIconPrivate *iconPrivate;
   QT_MANGLE_NAMESPACE(QNSImageView) *imageCell;
}
-(id)initWithIcon: (QSystemTrayIcon *)icon iconPrivate: (QSystemTrayIconPrivate *)iprivate;
-(void)dealloc;
-(QSystemTrayIcon *)icon;
-(NSStatusItem *)item;
-(QRectF)geometry;
- (void)triggerSelector: (id)sender button: (Qt::MouseButton)mouseButton;
- (void)doubleClickSelector: (id)sender;

- (BOOL)userNotificationCenter:(NSUserNotificationCenter *)center shouldPresentNotification:(NSUserNotification *)notification;
- (void)userNotificationCenter:(NSUserNotificationCenter *)center didActivateNotification:(NSUserNotification *)notification;


@end

@interface QT_MANGLE_NAMESPACE(QNSImageView) : NSImageView
{
   BOOL down;
   QT_MANGLE_NAMESPACE(QNSStatusItem) *parent;
}
-(id)initWithParent: (QT_MANGLE_NAMESPACE(QNSStatusItem) *)myParent;
-(QSystemTrayIcon *)icon;
-(void)menuTrackingDone: (NSNotification *)notification;
-(void)mousePressed: (NSEvent *)mouseEvent button: (Qt::MouseButton)mouseButton;
@end


@interface QT_MANGLE_NAMESPACE(QNSMenu) : NSMenu <NSMenuDelegate>
{
   QMenu *qmenu;
}
-(QMenu *)menu;
-(id)initWithQMenu: (QMenu *)qmenu;
-(void)selectedAction: (id)item;
@end

QT_BEGIN_NAMESPACE
class QSystemTrayIconSys
{
 public:
   QSystemTrayIconSys(QSystemTrayIcon *icon, QSystemTrayIconPrivate *d) {
      QMacCocoaAutoReleasePool pool;
      item = [[QT_MANGLE_NAMESPACE(QNSStatusItem) alloc] initWithIcon: icon iconPrivate: d];

      if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_8) {
         [[NSUserNotificationCenter defaultUserNotificationCenter] setDelegate:item];
      }
   }

   ~QSystemTrayIconSys() {
      QMacCocoaAutoReleasePool pool;
      [[[item item] view] setHidden: YES];

        if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_8) {
            [[NSUserNotificationCenter defaultUserNotificationCenter] setDelegate:nil];
        }

      [item release];
   }
    
   void emitMessageClicked() {
      emit [item icon]->messageClicked();
   }

    QT_MANGLE_NAMESPACE(QNSStatusItem) *item;
};

void QSystemTrayIconPrivate::install_sys()
{
   Q_Q(QSystemTrayIcon);
   if (!sys) {
      sys = new QSystemTrayIconSys(q, this);
      updateIcon_sys();
      updateMenu_sys();
      updateToolTip_sys();
   }
}

QRect QSystemTrayIconPrivate::geometry_sys() const
{
   if (sys) {
      const QRectF geom = [sys->item geometry];
      if (!geom.isNull()) {
         return geom.toRect();
      }
   }
   return QRect();
}

void QSystemTrayIconPrivate::remove_sys()
{
   delete sys;
   sys = 0;
}

void QSystemTrayIconPrivate::updateIcon_sys()
{
   if (sys && !icon.isNull()) {
      QMacCocoaAutoReleasePool pool;

      CGFloat hgt = [[[NSApplication sharedApplication] mainMenu] menuBarHeight];
      const short scale = hgt - 4;

      NSImage *nsimage = static_cast<NSImage *>(qt_mac_create_nsimage(icon.pixmap(QSize(scale, scale))));
      [(NSImageView *)[[sys->item item] view] setImage: nsimage];
      [nsimage release];
   }
}

void QSystemTrayIconPrivate::updateMenu_sys()
{
   if (sys) {
      QMacCocoaAutoReleasePool pool;
      if (menu && !menu->isEmpty()) {
         [[sys->item item] setHighlightMode: YES];
      } else {
         [[sys->item item] setHighlightMode: NO];
      }
   }
}

void QSystemTrayIconPrivate::updateToolTip_sys()
{
   if (sys) {
      QMacCocoaAutoReleasePool pool;
      QCFString string(toolTip);
      [[[sys->item item] view] setToolTip: (NSString *)static_cast<CFStringRef>(string)];
   }
}

bool QSystemTrayIconPrivate::isSystemTrayAvailable_sys()
{
   return true;
}

bool QSystemTrayIconPrivate::supportsMessages_sys()
{
   return true;
}

void QSystemTrayIconPrivate::showMessage_sys(const QString &title, const QString &message,
      QSystemTrayIcon::MessageIcon icon, int)
{
    if (!sys)
        return;

    if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_8) {
        NSUserNotification *notification = [[NSUserNotification alloc] init];
        notification.title = [NSString stringWithUTF8String:title.toUtf8().data()];
        notification.informativeText = [NSString stringWithUTF8String:message.toUtf8().data()];

        [[NSUserNotificationCenter defaultUserNotificationCenter] deliverNotification:notification];

        return;
    }


#ifdef QT_MAC_SYSTEMTRAY_USE_GROWL
    // Make sure that we have Growl installed on the machine we are running on.
    QCFType<CFURLRef> cfurl;
    OSStatus status = LSGetApplicationForInfo(kLSUnknownType, kLSUnknownCreator,
                                              CFSTR("growlTicket"), kLSRolesAll, 0, &cfurl);
    if (status == kLSApplicationNotFoundErr)
        return;
    QCFType<CFBundleRef> bundle = CFBundleCreate(0, cfurl);

    if (CFStringCompare(CFBundleGetIdentifier(bundle), CFSTR("com.Growl.GrowlHelperApp"),
                kCFCompareCaseInsensitive |  kCFCompareBackwards) != kCFCompareEqualTo)
        return;

    QPixmap notificationIconPixmap;
    if (icon == QSystemTrayIcon::Information)
        notificationIconPixmap = QApplication::style()->standardPixmap(QStyle::SP_MessageBoxInformation);
    else if (icon == QSystemTrayIcon::Warning)
        notificationIconPixmap = QApplication::style()->standardPixmap(QStyle::SP_MessageBoxWarning);
    else if (icon == QSystemTrayIcon::Critical)
        notificationIconPixmap = QApplication::style()->standardPixmap(QStyle::SP_MessageBoxCritical);

    QTemporaryFile notificationIconFile;
    QString notificationType(QLatin1String("Notification")), notificationIcon, notificationApp(QApplication::applicationName());
    if (notificationApp.isEmpty())
        notificationApp = QLatin1String("Application");
    if (!notificationIconPixmap.isNull() && notificationIconFile.open()) {
        QImageWriter writer(&notificationIconFile, "PNG");
        if (writer.write(notificationIconPixmap.toImage()))
            notificationIcon = QLatin1String("image from location \"file://") + notificationIconFile.fileName() + QLatin1String("\"");
    }
    const QString script(QLatin1String(
        "tell application \"System Events\"\n"
        "set isRunning to (count of (every process whose bundle identifier is \"com.Growl.GrowlHelperApp\")) > 0\n"
        "end tell\n"
        "if isRunning\n"
        "tell application id \"com.Growl.GrowlHelperApp\"\n"
        "-- Make a list of all the notification types (all)\n"
        "set the allNotificationsList to {\"") + notificationType + QLatin1String("\"}\n"

        "-- Make a list of the notifications (enabled)\n"
        "set the enabledNotificationsList to {\"") + notificationType + QLatin1String("\"}\n"

        "-- Register our script with growl.\n"
        "register as application \"") + notificationApp + QLatin1String("\" all notifications allNotificationsList default notifications enabledNotificationsList\n"

        "-- Send a Notification...\n") +
        QLatin1String("notify with name \"") + notificationType +
        QLatin1String("\" title \"") + title +
        QLatin1String("\" description \"") + message +
        QLatin1String("\" application name \"") + notificationApp +
        QLatin1String("\" ")  + notificationIcon +
        QLatin1String("\nend tell\nend if"));
    qt_mac_execute_apple_script(script, 0);
#else
    Q_UNUSED(icon);
    Q_UNUSED(title);
    Q_UNUSED(message);
#endif

}
QT_END_NAMESPACE

@implementation NSStatusItem (Qt)
@end

@implementation QT_MANGLE_NAMESPACE(QNSImageView)
-(id)initWithParent: (QT_MANGLE_NAMESPACE(QNSStatusItem) *)myParent
{
   self = [super init];
   parent = myParent;
   down = NO;
   return self;
}

-(QSystemTrayIcon *)icon
{
   return [parent icon];
}

-(void)menuTrackingDone: (NSNotification *)notification
{
   Q_UNUSED(notification);
   down = NO;

   if ( ![self icon]->icon().isNull() ) {

      CGFloat hgt = [[[NSApplication sharedApplication] mainMenu] menuBarHeight];
      const short scale = hgt - 4;

      NSImage *nsimage = static_cast<NSImage *>(qt_mac_create_nsimage([self icon]->icon().pixmap(QSize(scale, scale))));
      [self setImage: nsimage];
      [nsimage release];
   }

   if ([self icon]->contextMenu()) {
      [self icon]->contextMenu()->hide();
   }

   [self setNeedsDisplay: YES];
}

-(void)mousePressed: (NSEvent *)mouseEvent button: (Qt::MouseButton)mouseButton
{
   down = YES;
   int clickCount = [mouseEvent clickCount];
   [self setNeedsDisplay: YES];

   CGFloat hgt = [[[NSApplication sharedApplication] mainMenu] menuBarHeight];
   const short scale = hgt - 4;

   if (![self icon]->icon().isNull() ) {
      NSImage *nsaltimage = static_cast<NSImage *>(qt_mac_create_nsimage([self icon]->icon().pixmap(QSize(scale, scale),
                            QIcon::Selected)));
      [self setImage: nsaltimage];
      [nsaltimage release];
   }

   if (clickCount == 2) {
      [self menuTrackingDone: nil];
      [parent doubleClickSelector: self];
   } else {
      [parent triggerSelector: self button: mouseButton];
   }
}

-(void)mouseDown: (NSEvent *)mouseEvent
{
   [self mousePressed: mouseEvent button: Qt::LeftButton];
}

-(void)mouseUp: (NSEvent *)mouseEvent
{
   Q_UNUSED(mouseEvent);
   [self menuTrackingDone: nil];
}

- (void)rightMouseDown: (NSEvent *)mouseEvent
{
   [self mousePressed: mouseEvent button: Qt::RightButton];
}

-(void)rightMouseUp: (NSEvent *)mouseEvent
{
   Q_UNUSED(mouseEvent);
   [self menuTrackingDone: nil];
}

- (void)otherMouseDown: (NSEvent *)mouseEvent
{
   [self mousePressed: mouseEvent button: cocoaButton2QtButton([mouseEvent buttonNumber])];
}

-(void)otherMouseUp: (NSEvent *)mouseEvent
{
   Q_UNUSED(mouseEvent);
   [self menuTrackingDone: nil];
}

-(void)drawRect: (NSRect)rect
{
   [[parent item] drawStatusBarBackgroundInRect: rect withHighlight: down];
   [super drawRect: rect];
}
@end

@implementation QT_MANGLE_NAMESPACE(QNSStatusItem)

-(id)initWithIcon: (QSystemTrayIcon *)i iconPrivate: (QSystemTrayIconPrivate *)iPrivate
{
   self = [super init];
   if (self) {
      icon = i;
      iconPrivate = iPrivate;
      item = [[[NSStatusBar systemStatusBar] statusItemWithLength: NSSquareStatusItemLength] retain];
      imageCell = [[QT_MANGLE_NAMESPACE(QNSImageView) alloc] initWithParent: self];
      [item setView: imageCell];
   }
   return self;
}
-(void)dealloc
{
   [[NSStatusBar systemStatusBar] removeStatusItem: item];
   [imageCell release];
   [item release];
   [super dealloc];

}

-(QSystemTrayIcon *)icon
{
   return icon;
}

-(NSStatusItem *)item
{
   return item;
}
-(QRectF)geometry
{
   if (NSWindow *window = [[item view] window]) {
      NSRect screenRect = [[window screen] frame];
      NSRect windowRect = [window frame];
      return QRectF(windowRect.origin.x, screenRect.size.height - windowRect.origin.y - windowRect.size.height,
                    windowRect.size.width, windowRect.size.height);
   }
   return QRectF();
}

- (void)triggerSelector: (id)sender button: (Qt::MouseButton)mouseButton
{
   Q_UNUSED(sender);
   if (!icon) {
      return;
   }

   if (mouseButton == Qt::MiddleButton) {
      qtsystray_sendActivated(icon, QSystemTrayIcon::MiddleClick);
   } else {
      qtsystray_sendActivated(icon, QSystemTrayIcon::Trigger);
   }

   if (icon->contextMenu()) {

      NSMenu *m = [[QT_MANGLE_NAMESPACE(QNSMenu) alloc] initWithQMenu: icon->contextMenu()];
      [m setAutoenablesItems: NO];
      [[NSNotificationCenter defaultCenter] addObserver: imageCell
       selector: @selector(menuTrackingDone:)
       name: NSMenuDidEndTrackingNotification
       object: m];
      [item popUpStatusItemMenu: m];
      [m release];
   }
}

- (void)doubleClickSelector: (id)sender
{
   Q_UNUSED(sender);
   if (!icon) {
      return;
   }
   qtsystray_sendActivated(icon, QSystemTrayIcon::DoubleClick);
}


- (BOOL)userNotificationCenter:(NSUserNotificationCenter *)center shouldPresentNotification:(NSUserNotification *)notification {
    Q_UNUSED(center);
    Q_UNUSED(notification);
    return YES;
}

- (void)userNotificationCenter:(NSUserNotificationCenter *)center didActivateNotification:(NSUserNotification *)notification {
    Q_UNUSED(center);
    Q_UNUSED(notification);
    emit iconPrivate->sys->emitMessageClicked();
}


@end

class QSystemTrayIconQMenu : public QMenu
{
 public:
   void doAboutToShow() {
      emit aboutToShow();
   }
 private:
   QSystemTrayIconQMenu();
};

@implementation QT_MANGLE_NAMESPACE(QNSMenu)
-(id)initWithQMenu: (QMenu *)qm
{
   self = [super init];
   if (self) {
      self->qmenu = qm;
      [self setDelegate: self];
   }
   return self;
}
-(QMenu *)menu
{
   return qmenu;
}
-(void)menuNeedsUpdate: (NSMenu *)nsmenu
{
   QT_MANGLE_NAMESPACE(QNSMenu) *menu = static_cast<QT_MANGLE_NAMESPACE(QNSMenu) *>(nsmenu);
   emit static_cast<QSystemTrayIconQMenu *>(menu->qmenu)->doAboutToShow();
   for (int i = [menu numberOfItems] - 1; i >= 0; --i) {
      [menu removeItemAtIndex: i];
   }
   QList<QAction *> actions = menu->qmenu->actions();;
   for (int i = 0; i < actions.size(); ++i) {
      const QAction *action = actions[i];
      if (!action->isVisible()) {
         continue;
      }

      NSMenuItem *item = 0;
      bool needRelease = false;
      if (action->isSeparator()) {
         item = [NSMenuItem separatorItem];
      } else {
         item = [[NSMenuItem alloc] init];
         needRelease = true;
         QString text = action->text();
         QKeySequence accel = action->shortcut();
         {
            int st = text.lastIndexOf(QLatin1Char('\t'));
            if (st != -1) {
               accel = QKeySequence(text.right(text.length() - (st + 1)));
               text.remove(st, text.length() - st);
            }
         }
         if (accel.count() > 1) {
            text += QLatin1String(" (****)");   //just to denote a multi stroke shortcut
         }

         [item setTitle: (NSString *)QCFString::toCFStringRef(qt_mac_removeMnemonics(text))];
         [item setEnabled: menu->qmenu->isEnabled() && action->isEnabled()];
         [item setState: action->isChecked() ? NSOnState : NSOffState];
         [item setToolTip: (NSString *)QCFString::toCFStringRef(action->toolTip())];
         const QIcon icon = action->icon();
         if (!icon.isNull() && action->isIconVisibleInMenu()) {

            const short scale = [[[NSApplication sharedApplication] mainMenu] menuBarHeight];

            NSImage *nsimage = static_cast<NSImage *>(qt_mac_create_nsimage(icon.pixmap(QSize(scale, scale))));
            [item setImage: nsimage];
            [nsimage release];
         }
         if (action->menu()) {
            QT_MANGLE_NAMESPACE(QNSMenu) *sub = [[QT_MANGLE_NAMESPACE(QNSMenu) alloc] initWithQMenu: action->menu()];
            [item setSubmenu: sub];
         } else {
            [item setAction: @selector(selectedAction:)];
            [item setTarget: self];
         }
         if (!accel.isEmpty()) {
            [item setKeyEquivalent: keySequenceToKeyEqivalent(accel)];
            [item setKeyEquivalentModifierMask: keySequenceModifierMask(accel)];
         }
      }
      if (item) {
         [menu addItem: item];
      }
      if (needRelease) {
         [item release];
      }
   }
}
-(void)selectedAction: (id)a
{
   const int activated = [self indexOfItem: a];
   QAction *action = 0;
   QList<QAction *> actions = qmenu->actions();
   for (int i = 0, cnt = 0; i < actions.size(); ++i) {
      if (actions.at(i)->isVisible() && (cnt++) == activated) {
         action = actions.at(i);
         break;
      }
   }
   if (action) {
      action->activate(QAction::Trigger);
   }
}
@end

