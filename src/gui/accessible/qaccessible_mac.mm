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

#ifndef QT_NO_ACCESSIBILITY
#include <qaccessible_mac_p.h>
#include <qhash.h>
#include <qset.h>
#include <qpointer.h>
#include <qapplication.h>
#include <qmainwindow.h>
#include <qtextdocument.h>
#include <qdebug.h>
#include <qabstractslider.h>
#include <qsplitter.h>
#include <qtabwidget.h>
#include <qlistview.h>
#include <qtableview.h>
#include <qdockwidget.h>

#include <qt_mac_p.h>
#include <qwidget_p.h>
#include <CoreFoundation/CoreFoundation.h>

QT_BEGIN_NAMESPACE

// Set up platform defines
typedef NSString *const QAXRoleType;

#define QAXApplicationRole NSAccessibilityApplicationRole
#define QAXButtonRole NSAccessibilityButtonRole
#define QAXCancelAction NSAccessibilityCancelAction
#define QAXCheckBoxRole NSAccessibilityCheckBoxRole
#define QAXChildrenAttribute NSAccessibilityChildrenAttribute
#define QAXCloseButtonAttribute NSAccessibilityCloseButtonAttribute
#define QAXCloseButtonAttribute NSAccessibilityCloseButtonAttribute
#define QAXColumnRole NSAccessibilityColumnRole
#define QAXConfirmAction NSAccessibilityConfirmAction
#define QAXContentsAttribute NSAccessibilityContentsAttribute
#define QAXDecrementAction NSAccessibilityDecrementAction
#define QAXDecrementArrowSubrole NSAccessibilityDecrementArrowSubrole
#define QAXDecrementPageSubrole NSAccessibilityDecrementPageSubrole
#define QAXDescriptionAttribute NSAccessibilityDescriptionAttribute
#define QAXEnabledAttribute NSAccessibilityEnabledAttribute
#define QAXExpandedAttribute NSAccessibilityExpandedAttribute
#define QAXFocusedAttribute NSAccessibilityFocusedAttribute
#define QAXFocusedUIElementChangedNotification NSAccessibilityFocusedUIElementChangedNotification
#define QAXFocusedWindowChangedNotification NSAccessibilityFocusedWindowChangedNotification
#define QAXGroupRole NSAccessibilityGroupRole
#define QAXGrowAreaAttribute NSAccessibilityGrowAreaAttribute
#define QAXGrowAreaRole NSAccessibilityGrowAreaRole
#define QAXHelpAttribute NSAccessibilityHelpAttribute
#define QAXHorizontalOrientationValue NSAccessibilityHorizontalOrientationValue
#define QAXHorizontalScrollBarAttribute NSAccessibilityHorizontalScrollBarAttribute
#define QAXIncrementAction NSAccessibilityIncrementAction
#define QAXIncrementArrowSubrole NSAccessibilityIncrementArrowSubrole
#define QAXIncrementPageSubrole NSAccessibilityIncrementPageSubrole
#define QAXIncrementorRole NSAccessibilityIncrementorRole
#define QAXLinkedUIElementsAttribute NSAccessibilityLinkedUIElementsAttribute
#define QAXListRole NSAccessibilityListRole
#define QAXMainAttribute NSAccessibilityMainAttribute
#define QAXMaxValueAttribute NSAccessibilityMaxValueAttribute
#define QAXMenuBarRole NSAccessibilityMenuBarRole
#define QAXMenuButtonRole NSAccessibilityMenuButtonRole
#define QAXMenuClosedNotification NSAccessibilityMenuClosedNotification
#define QAXMenuItemRole NSAccessibilityMenuItemRole
#define QAXMenuOpenedNotification NSAccessibilityMenuOpenedNotification
#define QAXMenuRole NSAccessibilityMenuRole
#define QAXMinValueAttribute NSAccessibilityMinValueAttribute
#define QAXMinimizeButtonAttribute NSAccessibilityMinimizeButtonAttribute
#define QAXMinimizedAttribute NSAccessibilityMinimizedAttribute
#define QAXNextContentsAttribute NSAccessibilityNextContentsAttribute
#define QAXOrientationAttribute NSAccessibilityOrientationAttribute
#define QAXParentAttribute NSAccessibilityParentAttribute
#define QAXPickAction NSAccessibilityPickAction
#define QAXPopUpButtonRole NSAccessibilityPopUpButtonRole
#define QAXPositionAttribute NSAccessibilityPositionAttribute
#define QAXPressAction NSAccessibilityPressAction
#define QAXPreviousContentsAttribute NSAccessibilityPreviousContentsAttribute
#define QAXProgressIndicatorRole NSAccessibilityProgressIndicatorRole
#define QAXRadioButtonRole NSAccessibilityRadioButtonRole
#define QAXRoleAttribute NSAccessibilityRoleAttribute
#define QAXRoleDescriptionAttribute NSAccessibilityRoleDescriptionAttribute
#define QAXRowRole NSAccessibilityRowRole
#define QAXRowsAttribute NSAccessibilityRowsAttribute
#define QAXScrollAreaRole NSAccessibilityScrollAreaRole
#define QAXScrollBarRole NSAccessibilityScrollBarRole
#define QAXSelectedAttribute NSAccessibilitySelectedAttribute
#define QAXSelectedChildrenAttribute NSAccessibilitySelectedChildrenAttribute
#define QAXSelectedRowsAttribute NSAccessibilitySelectedRowsAttribute
#define QAXSizeAttribute NSAccessibilitySizeAttribute
#define QAXSliderRole NSAccessibilitySliderRole
#define QAXSplitGroupRole NSAccessibilitySplitGroupRole
#define QAXSplitterRole NSAccessibilitySplitterRole
#define QAXSplittersAttribute NSAccessibilitySplittersAttribute
#define QAXStaticTextRole NSAccessibilityStaticTextRole
#define QAXSubroleAttribute NSAccessibilitySubroleAttribute
#define QAXSubroleAttribute NSAccessibilitySubroleAttribute
#define QAXTabGroupRole NSAccessibilityTabGroupRole
#define QAXTableRole NSAccessibilityTableRole
#define QAXTabsAttribute NSAccessibilityTabsAttribute
#define QAXTextFieldRole NSAccessibilityTextFieldRole
#define QAXTitleAttribute NSAccessibilityTitleAttribute
#define QAXTitleUIElementAttribute NSAccessibilityTitleUIElementAttribute
#define QAXToolbarButtonAttribute NSAccessibilityToolbarButtonAttribute
#define QAXToolbarRole NSAccessibilityToolbarRole
#define QAXTopLevelUIElementAttribute NSAccessibilityTopLevelUIElementAttribute
#define QAXUnknownRole NSAccessibilityUnknownRole
#define QAXValueAttribute NSAccessibilityValueAttribute
#define QAXValueChangedNotification NSAccessibilityValueChangedNotification
#define QAXValueIndicatorRole NSAccessibilityValueIndicatorRole
#define QAXVerticalOrientationValue NSAccessibilityVerticalOrientationValue
#define QAXVerticalScrollBarAttribute NSAccessibilityVerticalScrollBarAttribute
#define QAXVisibleRowsAttribute NSAccessibilityVisibleRowsAttribute
#define QAXWindowAttribute NSAccessibilityWindowAttribute
#define QAXWindowCreatedNotification NSAccessibilityWindowCreatedNotification
#define QAXWindowMovedNotification NSAccessibilityWindowMovedNotification
#define QAXWindowRole NSAccessibilityWindowRole
#define QAXZoomButtonAttribute NSAccessibilityZoomButtonAttribute


//  externals
extern bool qt_mac_is_macsheet(const QWidget *w);    // qwidget_mac.cpp
extern bool qt_mac_is_macdrawer(const QWidget *w);   // qwidget_mac.cpp


// QAccessible Bindings - hardcoded bindings between control info and (known) QWidgets
struct QAccessibleTextBinding {
   int qt;
   QAXRoleType mac;
   bool settable;
} text_bindings[][10] = {
   {  { QAccessible::MenuItem, QAXMenuItemRole, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false }
   },
   {  { QAccessible::MenuBar, QAXMenuBarRole, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false }
   },
   {  { QAccessible::ScrollBar, QAXScrollBarRole, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false }
   },
   {  { QAccessible::Grip, QAXGrowAreaRole, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false }
   },
   {  { QAccessible::Window, QAXWindowRole, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false }
   },
   {  { QAccessible::Dialog, QAXWindowRole, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false }
   },
   {  { QAccessible::AlertMessage, QAXWindowRole, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false }
   },
   {  { QAccessible::ToolTip, QAXWindowRole, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false }
   },
   {  { QAccessible::HelpBalloon, QAXWindowRole, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false }
   },
   {  { QAccessible::PopupMenu, QAXMenuRole, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false }
   },
   {  { QAccessible::Application, QAXApplicationRole, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false }
   },
   {  { QAccessible::Pane, QAXGroupRole, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false }
   },
   {  { QAccessible::Grouping, QAXGroupRole, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false }
   },
   {  { QAccessible::Separator, QAXSplitterRole, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false }
   },
   {  { QAccessible::ToolBar, QAXToolbarRole, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false }
   },
   {  { QAccessible::PageTab, QAXRadioButtonRole, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false }
   },
   {  { QAccessible::ButtonMenu, QAXMenuButtonRole, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false }
   },
   {  { QAccessible::ButtonDropDown, QAXPopUpButtonRole, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false }
   },
   {  { QAccessible::SpinBox, QAXIncrementorRole, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false }
   },
   {  { QAccessible::Slider, QAXSliderRole, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false }
   },
   {  { QAccessible::ProgressBar, QAXProgressIndicatorRole, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false }
   },
   {  { QAccessible::ComboBox, QAXPopUpButtonRole, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false }
   },
   {  { QAccessible::RadioButton, QAXRadioButtonRole, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false }
   },
   {  { QAccessible::CheckBox, QAXCheckBoxRole, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false }
   },
   {  { QAccessible::StaticText, QAXStaticTextRole, false },
      { QAccessible::Name, QAXValueAttribute, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false }
   },
   {  { QAccessible::Table, QAXTableRole, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false }
   },
   {  { QAccessible::StatusBar, QAXStaticTextRole, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false }
   },
   {  { QAccessible::Column, QAXColumnRole, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false }
   },
   {  { QAccessible::ColumnHeader, QAXColumnRole, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false }
   },
   {  { QAccessible::Row, QAXRowRole, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false }
   },
   {  { QAccessible::RowHeader, QAXRowRole, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false }
   },
   {  { QAccessible::Cell, QAXTextFieldRole, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false }
   },
   {  { QAccessible::PushButton, QAXButtonRole, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false }
   },
   {  { QAccessible::EditableText, QAXTextFieldRole, true },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false }
   },
   {  { QAccessible::Link, QAXTextFieldRole, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false }
   },
   {  { QAccessible::Indicator, QAXValueIndicatorRole, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false }
   },
   {  { QAccessible::Splitter, QAXSplitGroupRole, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false }
   },
   {  { QAccessible::List, QAXListRole, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false }
   },
   {  { QAccessible::ListItem, QAXStaticTextRole, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false }
   },
   {  { QAccessible::Cell, QAXStaticTextRole, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false }
   },
   {  { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false },
      { -1, 0, false }
   }
};

// The root of the Qt accessible hiearchy
static QObject *rootObject = 0;

class QAInterface;
static CFStringRef macRole(const QAInterface &interface);

QDebug operator<<(QDebug debug, const QAInterface &interface)
{
   if (interface.isValid() == false) {
      debug << "invalid interface";
   } else {
      debug << interface.object() << "id" << interface.id() << "role" << hex << interface.role();
   }
   return debug;
}

// ***
bool QAInterface::operator==(const QAInterface &other) const
{
   if (isValid() == false || other.isValid() == false) {
      return (isValid() && other.isValid());
   }

   // walk up the parent chain, comparing child indexes, until we reach an interface with a QObject
   QAInterface currentThis = *this;
   QAInterface currentOther = other;

   while (currentThis.object() == 0) {
      if (currentOther.object() != 0) {
         return false;
      }

      // fail if the child indexes in the two hirearchies don't match.
      if (currentThis.parent().indexOfChild(currentThis) !=
            currentOther.parent().indexOfChild(currentOther)) {
         return false;
      }

      currentThis = currentThis.parent();
      currentOther = currentOther.parent();
   }

   return (currentThis.object() == currentOther.object() && currentThis.id() == currentOther.id());
}

bool QAInterface::operator!=(const QAInterface &other) const
{
   return !operator==(other);
}

uint qHash(const QAInterface &item)
{
   if (item.isValid()) {
      return qHash(item.object()) + qHash(item.id());
   } else {
      return qHash(item.cachedObject()) + qHash(item.id());
   }
}

QAInterface QAInterface::navigate(RelationFlag relation, int entry) const
{
   if (!checkValid()) {
      return QAInterface();
   }

   // On a QAccessibleInterface that handles its own children we can short-circut
   // the navigation if this QAInterface refers to one of the children:
   if (child != 0) {
      // The Ancestor interface will always be the same QAccessibleInterface with
      // a child value of 0.
      if (relation == QAccessible::Ancestor) {
         return QAInterface(*this, 0);
      }

      // The child hiearchy is only one level deep, so navigating to a child
      // of a child is not possible.
      if (relation == QAccessible::Child) {
         return QAInterface();
      }
   }
   QAccessibleInterface *child_iface = 0;

   const int status = base.interface->navigate(relation, entry, &child_iface);

   if (status == -1) {
      return QAInterface();   // not found;
   }

   // Check if target is a child of this interface
   if (! child_iface) {
      return QAInterface(*this, status);
   } else {
      // Target is child_iface or a child of that (status decides).
      return QAInterface(child_iface, status);
   }
}

static bool isTabWidget(const QAInterface &interface)
{
   if (QObject *object = interface.object()) {
      return (object->inherits("QTabWidget") && interface.id() == 0);
   }
   return false;
}

static bool isStandaloneTabBar(const QAInterface &interface)
{
   QObject *object = interface.object();

   if (interface.role() == QAccessible::PageTabList && object) {
      return (qobject_cast<QTabWidget *>(object->parent()) == 0);
   }

   return false;
}

static bool isEmbeddedTabBar(const QAInterface &interface)
{
   QObject *object = interface.object();
   if (interface.role() == QAccessible::PageTabList && object) {
      return (qobject_cast<QTabWidget *>(object->parent()));
   }

   return false;
}

// Decides if a QAInterface is interesting from an accessibility users point of view
bool isItInteresting(const QAInterface &interface)
{
   // Mac accessibility does not have an attribute that corresponds to the Invisible/Offscreen
   // state, so we disable the interface here

   const QAccessible::State state = interface.state();
   if (state & QAccessible::Invisible ||
         state & QAccessible::Offscreen ) {
      return false;
   }

   const QAccessible::Role role = interface.role();

   if (QObject *const object = interface.object()) {
      const QString className = QLatin1String(object->metaObject()->className());

      // VoiceOver focusing on tool tips can be confusing. The contents of the tool tip is avalible
      // through the description attribute anyway, so we disable accessibility for tool tips.
      if (className == QLatin1String("QTipLabel")) {
         return false;
      }

      // Hide TabBars has a QTabWidget parent (the tab widget handles the accessibility)
      if (isEmbeddedTabBar(interface)) {
         return false;
      }

      // Hide docked dockwidgets. ### causes infinitie loop in the apple accessibility code.
      /*    if (QDockWidget *dockWidget = qobject_cast<QDockWidget *>(object)) {
             if (dockWidget->isFloating() == false)
                 return false;
          }
      */

   }

   // Client is a generic role returned by plain QWidgets or other widgets which does not have separate
   //  QAccessible interface, such as the TabWidget. Return false unless macRole gives the interface a special role.
   if (role == QAccessible::Client && macRole(interface) == CFStringRef(QAXUnknownRole)) {
      return false;
   }

   // Some roles are not interesting

   if (role == QAccessible::Border ||       // QFrame
         role == QAccessible::Application ||  // We use the system-provided application element.
         role == QAccessible::MenuItem) {     // The system also provides the menu items.
      return false;
   }

   // It is probably better to access the toolbar buttons directly than having to navigate through the toolbar
   if (role == QAccessible::ToolBar) {
      return false;
   }

   return true;
}

// ***
QAElement::QAElement()
   : elementRef(0)
{
}

QAElement::QAElement(AXUIElementRef elementRef)
   : elementRef(elementRef)
{
   if (elementRef != 0) {
      CFRetain(elementRef);
   }
}

QAElement::QAElement(const QAElement &element)
   : elementRef(element.elementRef)
{
   if (elementRef != 0) {
      CFRetain(elementRef);
   }
}

QAElement::~QAElement()
{
   if (elementRef != 0) {
      CFRelease(elementRef);
   }
}

void QAElement::operator=(const QAElement &other)
{
   if (*this == other) {
      return;
   }

   if (elementRef != 0) {
      CFRelease(elementRef);
   }

   elementRef = other.elementRef;

   if (elementRef != 0) {
      CFRetain(elementRef);
   }
}

bool QAElement::operator==(const QAElement &other) const
{
   if (elementRef == 0 || other.elementRef == 0) {
      return (elementRef == other.elementRef);
   }

   return CFEqual(elementRef, other.elementRef);
}

uint qHash(QAElement element)
{
   return qHash(element.id());
}

// ***
Q_GLOBAL_STATIC(QAccessibleHierarchyManager, accessibleHierarchyManager);

// removes all accessibility info associated with the sender object
void QAccessibleHierarchyManager::objectDestroyed(QObject *object)
{
   delete qobjectElementHash.value(object);
   qobjectElementHash.remove(object);
}

// Removes all stored items
void QAccessibleHierarchyManager::reset()
{
   qDeleteAll(qobjectElementHash);
   qobjectElementHash.clear();
}

QAccessibleHierarchyManager *QAccessibleHierarchyManager::instance()
{
   return accessibleHierarchyManager();
}

void QAccessibleHierarchyManager::registerChildren(const QAInterface &interface)
{
   QObject *const object = interface.object();
   if (object == 0) {
      return;
   }

   QInterfaceFactory *interfaceFactory = qobjectElementHash.value(object);

   if (interfaceFactory == 0) {
      return;
   }

   interfaceFactory->registerChildren();
}

QAElement QAccessibleHierarchyManager::lookup(const QAInterface &interface)
{
   if (interface.isValid() == false) {
      return QAElement();
   }

   QInterfaceFactory *factory = qobjectElementHash.value(interface.objectInterface().object());
   if (factory == 0) {
      return QAElement();
   }

   return factory->element(interface);
}

QAElement QAccessibleHierarchyManager::lookup(QObject *const object, int id)
{
   QInterfaceFactory *factory = qobjectElementHash.value(object);
   if (factory == 0) {
      return QAElement();
   }

   return factory->element(id);
}

QList<QAElement> lookup(const QList<QAInterface> &interfaces)
{
   QList<QAElement> elements;

   for (const QAInterface & interface : interfaces) {
      if (interface.isValid()) {
         const QAElement element = accessibleHierarchyManager()->lookup(interface);
         if (element.isValid()) {
            elements.append(element);
         }
      }
   }

   return elements;
}

/*
static QString nameForEventKind(UInt32 kind)
{
   // debug output helpers
    switch(kind) {
        case kEventAccessibleGetChildAtPoint:       return QString("GetChildAtPoint");      break;
        case kEventAccessibleGetAllAttributeNames:  return QString("GetAllAttributeNames"); break;
        case kEventAccessibleGetNamedAttribute:     return QString("GetNamedAttribute");    break;
        case kEventAccessibleSetNamedAttribute:     return QString("SetNamedAttribute");    break;
        case kEventAccessibleGetAllActionNames:     return QString("GetAllActionNames");    break;
        case kEventAccessibleGetFocusedChild:       return QString("GetFocusedChild");      break;
        default:
            return QString("Unknown accessibility event type: %1").arg(kind);
        break;
    };
}
*/

//  Translates a QAccessible::Role into a mac accessibility role.
static CFStringRef macRole(const QAInterface &interface)
{
   const QAccessible::Role qtRole = interface.role();

   //    qDebug() << "role for" << interface.object() << "interface role" << hex << qtRole;

   // Qt accessibility:  QAccessible::Splitter contains QAccessible::Grip.
   // Mac accessibility: AXSplitGroup contains AXSplitter.
   if (qtRole == QAccessible::Grip) {
      const QAInterface parent = interface.parent();
      if (parent.isValid() && parent.role() == QAccessible::Splitter) {
         return CFStringRef(QAXSplitterRole);
      }
   }

   // Tab widgets and standalone tab bars get the kAXTabGroupRole. Accessibility
   // for tab bars emebedded in a tab widget is handled by the tab widget.
   if (isTabWidget(interface) || isStandaloneTabBar(interface)) {
      return kAXTabGroupRole;
   }

   if (QObject *object = interface.object()) {
      // ### The interface for an abstract scroll area returns the generic "Client"
      // role, so we have to to an extra detect on the QObject here.
      if (object->inherits("QAbstractScrollArea") && interface.id() == 0) {
         return CFStringRef(QAXScrollAreaRole);
      }

      if (object->inherits("QDockWidget")) {
         return CFStringRef(QAXUnknownRole);
      }
   }

   int i = 0;
   int testRole = text_bindings[i][0].qt;
   while (testRole != -1) {
      if (testRole == qtRole) {
         return CFStringRef(text_bindings[i][0].mac);
      }
      ++i;
      testRole = text_bindings[i][0].qt;
   }

   //    qDebug() << "got unknown role!" << interface << interface.parent();

   return CFStringRef(QAXUnknownRole);
}

struct IsWindowTest {
   static inline bool test(const QAInterface &interface) {
      return (interface.role() == QAccessible::Window);
   }
};

struct IsWindowAndNotDrawerOrSheetTest {
   static inline bool test(const QAInterface &interface) {
      QWidget *const widget = qobject_cast<QWidget *>(interface.object());
      return (interface.role() == QAccessible::Window &&
              widget && widget->isWindow() &&
              !qt_mac_is_macdrawer(widget) &&
              !qt_mac_is_macsheet(widget));
   }
};


// ***
void QAccessible::initialize()
{
}

// Sets thre root object for the application
void QAccessible::setRootObject(QObject *object)
{
   // Call installed root object handler if we have one
   if (rootObjectHandler) {
      rootObjectHandler(object);
      return;
   }

   rootObject = object;
}

void QAccessible::cleanup()
{
   accessibleHierarchyManager()->reset();
}

void QAccessible::updateAccessibility(QObject *object, int child, Event reason)
{
   // Call installed update handler if we have one.
   if (updateHandler) {
      updateHandler(object, child, reason);
      return;
   }
}

QT_END_NAMESPACE

#endif // QT_NO_ACCESSIBILITY
