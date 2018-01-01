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

#include <qaccessible.h>
#include <qaccessible_mac_p.h>
#include <qdebug.h>
#include <qtabwidget.h>
#include <qt_mac_p.h>
#include <qcocoaview_mac_p.h>
#include <qwidget_p.h>

#ifndef QT_NO_ACCESSIBILITY

QT_BEGIN_NAMESPACE

#ifdef MAC_ACCESSIBILTY_DEVELOPER_MODE
#define MAC_ACCESSIBILTY_DEBUG QT_PREPEND_NAMESPACE(qDebug)
#else
#define MAC_ACCESSIBILTY_DEBUG if (0) QT_PREPEND_NAMESPACE(qDebug)
#endif

typedef QMap<QAccessible::Role, NSString *> QMacAccessibiltyRoleMap;
Q_GLOBAL_STATIC(QMacAccessibiltyRoleMap, qMacAccessibiltyRoleMap);

static QAInterface interfaceForView(QT_MANGLE_NAMESPACE(QCocoaView) *view)
{
   return QAInterface(QAccessible::queryAccessibleInterface([view qt_qwidget]));
}

// set up mappings from Qt accessibilty roles to Mac accessibilty roles
static void populateRoleMap()
{
   QMacAccessibiltyRoleMap &roleMap = *qMacAccessibiltyRoleMap();
   roleMap[QAccessible::MenuItem] = NSAccessibilityMenuItemRole;
   roleMap[QAccessible::MenuBar] = NSAccessibilityMenuBarRole;
   roleMap[QAccessible::ScrollBar] = NSAccessibilityScrollBarRole;
   roleMap[QAccessible::Grip] = NSAccessibilityGrowAreaRole;
   roleMap[QAccessible::Window] = NSAccessibilityWindowRole;
   roleMap[QAccessible::Dialog] = NSAccessibilityWindowRole;
   roleMap[QAccessible::AlertMessage] = NSAccessibilityWindowRole;
   roleMap[QAccessible::ToolTip] = NSAccessibilityWindowRole;
   roleMap[QAccessible::HelpBalloon] = NSAccessibilityWindowRole;
   roleMap[QAccessible::PopupMenu] = NSAccessibilityMenuRole;
   roleMap[QAccessible::Application] = NSAccessibilityApplicationRole;
   roleMap[QAccessible::Pane] = NSAccessibilityGroupRole;
   roleMap[QAccessible::Grouping] = NSAccessibilityGroupRole;
   roleMap[QAccessible::Separator] = NSAccessibilitySplitterRole;
   roleMap[QAccessible::ToolBar] = NSAccessibilityToolbarRole;
   roleMap[QAccessible::PageTab] = NSAccessibilityRadioButtonRole;
   roleMap[QAccessible::ButtonMenu] = NSAccessibilityMenuButtonRole;
   roleMap[QAccessible::ButtonDropDown] = NSAccessibilityPopUpButtonRole;
   roleMap[QAccessible::SpinBox] = NSAccessibilityIncrementorRole;
   roleMap[QAccessible::Slider] = NSAccessibilitySliderRole;
   roleMap[QAccessible::ProgressBar] = NSAccessibilityProgressIndicatorRole;
   roleMap[QAccessible::ComboBox] = NSAccessibilityPopUpButtonRole;
   roleMap[QAccessible::RadioButton] = NSAccessibilityRadioButtonRole;
   roleMap[QAccessible::CheckBox] = NSAccessibilityCheckBoxRole;
   roleMap[QAccessible::StaticText] = NSAccessibilityStaticTextRole;
   roleMap[QAccessible::Table] = NSAccessibilityTableRole;
   roleMap[QAccessible::StatusBar] = NSAccessibilityStaticTextRole;
   roleMap[QAccessible::Column] = NSAccessibilityColumnRole;
   roleMap[QAccessible::ColumnHeader] = NSAccessibilityColumnRole;
   roleMap[QAccessible::Row] = NSAccessibilityRowRole;
   roleMap[QAccessible::RowHeader] = NSAccessibilityRowRole;
   roleMap[QAccessible::Cell] = NSAccessibilityTextFieldRole;
   roleMap[QAccessible::PushButton] = NSAccessibilityButtonRole;
   roleMap[QAccessible::EditableText] = NSAccessibilityTextFieldRole;
   roleMap[QAccessible::Link] = NSAccessibilityTextFieldRole;
   roleMap[QAccessible::Indicator] = NSAccessibilityValueIndicatorRole;
   roleMap[QAccessible::Splitter] = NSAccessibilitySplitGroupRole;
   roleMap[QAccessible::List] = NSAccessibilityListRole;
   roleMap[QAccessible::ListItem] = NSAccessibilityStaticTextRole;
   roleMap[QAccessible::Cell] = NSAccessibilityStaticTextRole;
}


//  returns a Mac accessibility role for the given interface, or NSAccessibilityUnknownRole if no role mapping is found
static NSString *macRoleForInterface(QAInterface interface)
{
   const QAccessible::Role qtRole = interface.role();
   QMacAccessibiltyRoleMap &roleMap = *qMacAccessibiltyRoleMap();

   if (roleMap.isEmpty()) {
      populateRoleMap();
   }

   MAC_ACCESSIBILTY_DEBUG() << "role for" << interface.object() << "interface role" << hex << qtRole;

   if (roleMap.contains(qtRole)) {
      MAC_ACCESSIBILTY_DEBUG() << "return" <<  roleMap[qtRole];
      return roleMap[qtRole];
   }

   MAC_ACCESSIBILTY_DEBUG() << "return NSAccessibilityUnknownRole";
   return NSAccessibilityUnknownRole;
}

/*
    Is the interface a QTabBar embedded in a QTabWidget?
    (as opposed to a stand-alone tab bar)
*/
static bool isEmbeddedTabBar(const QAInterface &interface)
{
   QObject *object = interface.object();
   if (interface.role() == QAccessible::PageTabList && object) {
      return (qobject_cast<QTabWidget *>(object->parent()));
   }

   return false;
}

static bool isInterfaceIgnored(QAInterface interface)
{
   // Mac accessibility does not have an attribute that corresponds to the
   // Invisible/Offscreen state. Use the ignore facility to disable them.
   const QAccessible::State state = interface.state();
   if (state & QAccessible::Invisible ||
         state & QAccessible::Offscreen ) {
      return false;
   }

   // Hide QTabBars that has a QTabWidget parent (the QTabWidget handles the accessibility)
   if (isEmbeddedTabBar(interface)) {
      return false;
   }

   if (QObject *const object = interface.object()) {
      const QString className = QLatin1String(object->metaObject()->className());

      // Prevent VoiceOver from focusing on tool tips by ignoring those
      // interfaces. Shifting VoiceOver focus to the tool tip is confusing
      // and the contents of the tool tip is avalible through the description
      // attribute anyway.
      if (className == QLatin1String("QTipLabel")) {
         return false;
      }
   }

   // Hide interfaces with an unknown role. When developing it's often useful to disable
   // this check to see all interfaces in the hierarchy.

#ifndef MAC_ACCESSIBILTY_DEVELOPER_MODE
   return [macRoleForInterface(interface) isEqualToString: NSAccessibilityUnknownRole];
#else
   return NO;
#endif

}

QT_END_NAMESPACE

@implementation QT_MANGLE_NAMESPACE(QCocoaView) (Accessibility)

- (BOOL)accessibilityIsIgnored
{
   QT_PREPEND_NAMESPACE(QAInterface) interface = QT_PREPEND_NAMESPACE(interfaceForView)(self);
   return isInterfaceIgnored(interface);
}

- (NSArray *)accessibilityAttributeNames
{
   QT_PREPEND_NAMESPACE(QAInterface) interface = QT_PREPEND_NAMESPACE(interfaceForView)(self);

   static NSArray *attributes = nil;
   if (attributes == nil) {
      attributes = [super accessibilityAttributeNames];

   }
   return attributes;
}

- (id)accessibilityAttributeValue: (NSString *)attribute
{
   MAC_ACCESSIBILTY_DEBUG() << "accessibilityAttributeValue" << self <<
                            QT_PREPEND_NAMESPACE(QCFString)::toQString(reinterpret_cast<CFStringRef>(attribute));

   QT_PREPEND_NAMESPACE(QAInterface) interface = QT_PREPEND_NAMESPACE(interfaceForView)(self);

   // Switch on the attribute name and call the appropriate handler function.
   // Pass the call on to the NSView class for attributes we don't handle.
   if ([attribute isEqualToString: @"AXRole"]) {
      return macRoleForInterface(interface);
   } else {
      return [super accessibilityAttributeValue: attribute];
   }
}

@end

#endif // QT_NO_ACCESSIBILITY

