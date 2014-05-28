/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include "qaccessible.h"

#ifndef QT_NO_ACCESSIBILITY
#include "qaccessible_mac_p.h"
#include "qhash.h"
#include "qset.h"
#include "qpointer.h"
#include "qapplication.h"
#include "qmainwindow.h"
#include "qtextdocument.h"
#include "qdebug.h"
#include "qabstractslider.h"
#include "qsplitter.h"
#include "qtabwidget.h"
#include "qlistview.h"
#include "qtableview.h"
#include "qdockwidget.h"

#include <qt_mac_p.h>
#include <qwidget_p.h>
#include <CoreFoundation/CoreFoundation.h>

QT_BEGIN_NAMESPACE

/*
    Set up platform defines. There is a one-to-one correspondence between the
    Carbon and Cocoa roles and attributes, but the prefix and type changes.
*/
typedef NSString * const QAXRoleType;
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

/*****************************************************************************
  Externals
 *****************************************************************************/
extern bool qt_mac_is_macsheet(const QWidget *w); //qwidget_mac.cpp
extern bool qt_mac_is_macdrawer(const QWidget *w); //qwidget_mac.cpp

/*****************************************************************************
  QAccessible Bindings
 *****************************************************************************/
//hardcoded bindings between control info and (known) QWidgets
struct QAccessibleTextBinding {
    int qt;
    QAXRoleType mac;
    bool settable;
} text_bindings[][10] = {
    { { QAccessible::MenuItem, QAXMenuItemRole, false },
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
    { { QAccessible::MenuBar, QAXMenuBarRole, false },
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
    { { QAccessible::ScrollBar, QAXScrollBarRole, false },
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
    { { QAccessible::Grip, QAXGrowAreaRole, false },
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
    { { QAccessible::Window, QAXWindowRole, false },
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
    { { QAccessible::Dialog, QAXWindowRole, false },
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
    { { QAccessible::AlertMessage, QAXWindowRole, false },
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
    { { QAccessible::ToolTip, QAXWindowRole, false },
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
    { { QAccessible::HelpBalloon, QAXWindowRole, false },
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
    { { QAccessible::PopupMenu, QAXMenuRole, false },
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
    { { QAccessible::Application, QAXApplicationRole, false },
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
    { { QAccessible::Pane, QAXGroupRole, false },
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
    { { QAccessible::Grouping, QAXGroupRole, false },
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
    { { QAccessible::Separator, QAXSplitterRole, false },
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
    { { QAccessible::ToolBar, QAXToolbarRole, false },
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
    { { QAccessible::PageTab, QAXRadioButtonRole, false },
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
    { { QAccessible::ButtonMenu, QAXMenuButtonRole, false },
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
    { { QAccessible::ButtonDropDown, QAXPopUpButtonRole, false },
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
    { { QAccessible::SpinBox, QAXIncrementorRole, false },
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
    { { QAccessible::Slider, QAXSliderRole, false },
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
    { { QAccessible::ProgressBar, QAXProgressIndicatorRole, false },
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
    { { QAccessible::ComboBox, QAXPopUpButtonRole, false },
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
    { { QAccessible::RadioButton, QAXRadioButtonRole, false },
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
    { { QAccessible::CheckBox, QAXCheckBoxRole, false },
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
    { { QAccessible::StaticText, QAXStaticTextRole, false },
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
    { { QAccessible::Table, QAXTableRole, false },
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
    { { QAccessible::StatusBar, QAXStaticTextRole, false },
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
    { { QAccessible::Column, QAXColumnRole, false },
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
    { { QAccessible::ColumnHeader, QAXColumnRole, false },
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
    { { QAccessible::Row, QAXRowRole, false },
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
    { { QAccessible::RowHeader, QAXRowRole, false },
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
    { { QAccessible::Cell, QAXTextFieldRole, false },
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
    { { QAccessible::PushButton, QAXButtonRole, false },
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
    { { QAccessible::EditableText, QAXTextFieldRole, true },
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
    { { QAccessible::Link, QAXTextFieldRole, false },
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
    { { QAccessible::Indicator, QAXValueIndicatorRole, false },
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
    { { QAccessible::Splitter, QAXSplitGroupRole, false },
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
    { { QAccessible::List, QAXListRole, false },
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
    { { QAccessible::ListItem, QAXStaticTextRole, false },
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
    { { QAccessible::Cell, QAXStaticTextRole, false },
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
    { { -1, 0, false },
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

class QAInterface;
static CFStringRef macRole(const QAInterface &interface);

QDebug operator<<(QDebug debug, const QAInterface &interface)
{
    if (interface.isValid() == false)
        debug << "invalid interface";
    else 
        debug << interface.object() << "id" << interface.id() << "role" << hex << interface.role();
    return debug;
}

// The root of the Qt accessible hiearchy.
static QObject *rootObject = 0;


bool QAInterface::operator==(const QAInterface &other) const
{
    if (isValid() == false || other.isValid() == false)
        return (isValid() && other.isValid());
    
    // walk up the parent chain, comparing child indexes, until we reach
    // an interface that has a QObject.
    QAInterface currentThis = *this;
    QAInterface currentOther = other;
    
    while (currentThis.object() == 0) {
        if (currentOther.object() != 0)
            return false;

        // fail if the child indexes in the two hirearchies don't match.
        if (currentThis.parent().indexOfChild(currentThis) !=
            currentOther.parent().indexOfChild(currentOther))
            return false;

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
    if (item.isValid())
        return qHash(item.object()) + qHash(item.id());
    else
        return qHash(item.cachedObject()) + qHash(item.id());
}

QAInterface QAInterface::navigate(RelationFlag relation, int entry) const
{
        if (!checkValid())
            return QAInterface();

    // On a QAccessibleInterface that handles its own children we can short-circut
    // the navigation if this QAInterface refers to one of the children:
    if (child != 0) {
        // The Ancestor interface will always be the same QAccessibleInterface with
        // a child value of 0.
        if (relation == QAccessible::Ancestor)
            return QAInterface(*this, 0);

        // The child hiearchy is only one level deep, so navigating to a child
        // of a child is not possible.
        if (relation == QAccessible::Child) {
            return QAInterface();
        }
    }
    QAccessibleInterface *child_iface = 0;

    const int status = base.interface->navigate(relation, entry, &child_iface);

    if (status == -1)
        return QAInterface(); // not found;

    // Check if target is a child of this interface.
    if (!child_iface) {
        return QAInterface(*this, status);
    } else {
        // Target is child_iface or a child of that (status decides).
        return QAInterface(child_iface, status);
    }
}

QAElement::QAElement()
:elementRef(0)
{}

QAElement::QAElement(AXUIElementRef elementRef)
:elementRef(elementRef)
{
    if (elementRef != 0) {
        CFRetain(elementRef);
        CFRetain(object());
    }
}

QAElement::QAElement(const QAElement &element)
:elementRef(element.elementRef)
{
    if (elementRef != 0) {
        CFRetain(elementRef);
        CFRetain(object());
    }
}

QAElement::QAElement(HIObjectRef object, int child)
{
    Q_UNUSED(object);
    Q_UNUSED(child);
}

QAElement::~QAElement()
{
    if (elementRef != 0) {
        CFRelease(object());
        CFRelease(elementRef);
    }
}

void QAElement::operator=(const QAElement &other)
{
    if (*this == other)
        return;

    if (elementRef != 0) {
        CFRelease(object());
        CFRelease(elementRef);
    }

    elementRef = other.elementRef;

    if (elementRef != 0) {
        CFRetain(elementRef);
        CFRetain(object());
    }
}

bool QAElement::operator==(const QAElement &other) const
{
    if (elementRef == 0 || other.elementRef == 0)
        return (elementRef == other.elementRef);

    return CFEqual(elementRef, other.elementRef);
}

uint qHash(QAElement element)
{
    return qHash(element.object()) + qHash(element.id());
}

Q_GLOBAL_STATIC(QAccessibleHierarchyManager, accessibleHierarchyManager);

/*
    Reomves all accessibility info accosiated with the sender object.
*/
void QAccessibleHierarchyManager::objectDestroyed(QObject *object)
{
    HIObjectRef hiObject = qobjectHiobjectHash.value(object);
    delete qobjectElementHash.value(object);
    qobjectElementHash.remove(object);
    hiobjectInterfaceHash.remove(hiObject);
}

/*
    Removes all stored items.
*/
void QAccessibleHierarchyManager::reset()
{
    qDeleteAll(qobjectElementHash);
    qobjectElementHash.clear();
    hiobjectInterfaceHash.clear();
    qobjectHiobjectHash.clear();
}

QAccessibleHierarchyManager *QAccessibleHierarchyManager::instance()
{
    return accessibleHierarchyManager();
}

static bool isTabWidget(const QAInterface &interface)
{
    if (QObject *object = interface.object())
        return (object->inherits("QTabWidget") && interface.id() == 0);
    return false;
}

static bool isStandaloneTabBar(const QAInterface &interface)
{
    QObject *object = interface.object();
    if (interface.role() == QAccessible::PageTabList && object)
        return (qobject_cast<QTabWidget *>(object->parent()) == 0);

    return false;
}

static bool isEmbeddedTabBar(const QAInterface &interface)
{
    QObject *object = interface.object();
    if (interface.role() == QAccessible::PageTabList && object)
        return (qobject_cast<QTabWidget *>(object->parent()));

    return false;
}

/*
    Decides if a QAInterface is interesting from an accessibility users point of view.
*/
bool isItInteresting(const QAInterface &interface)
{
    // Mac accessibility does not have an attribute that corresponds to the Invisible/Offscreen
    // state, so we disable the interface here.
    const QAccessible::State state = interface.state();
    if (state & QAccessible::Invisible ||
        state & QAccessible::Offscreen )
        return false;

    const QAccessible::Role role = interface.role();

    if (QObject * const object = interface.object()) {
        const QString className = QLatin1String(object->metaObject()->className());

        // VoiceOver focusing on tool tips can be confusing. The contents of the
        // tool tip is avalible through the description attribute anyway, so
        // we disable accessibility for tool tips.
        if (className == QLatin1String("QTipLabel"))
            return false;

        // Hide TabBars that has a QTabWidget parent (the tab widget handles the accessibility)
        if (isEmbeddedTabBar(interface))
            return false;

         // Hide docked dockwidgets. ### causes infinitie loop in the apple accessibility code.
     /*    if (QDockWidget *dockWidget = qobject_cast<QDockWidget *>(object)) {
            if (dockWidget->isFloating() == false)
                return false;        
         }
    */
    }

    // Client is a generic role returned by plain QWidgets or other
    // widgets that does not have separate QAccessible interface, such
    // as the TabWidget. Return false unless macRole gives the interface
    // a special role.
    if (role == QAccessible::Client && macRole(interface) == CFStringRef(QAXUnknownRole))
        return false;

    // Some roles are not interesting:
    if (role == QAccessible::Border ||    // QFrame
        role == QAccessible::Application || // We use the system-provided application element.
        role == QAccessible::MenuItem)      // The system also provides the menu items.
        return false;

    // It is probably better to access the toolbar buttons directly than having
    // to navigate through the toolbar.
    if (role == QAccessible::ToolBar)
        return false;

    return true;
}

QAElement QAccessibleHierarchyManager::registerInterface(QObject *object, int child)
{
    Q_UNUSED(object);
    Q_UNUSED(child);
    return QAElement();
}

/*
    Creates a QAXUIelement that corresponds to the given QAInterface.
*/
QAElement QAccessibleHierarchyManager::registerInterface(const QAInterface &interface)
{
    Q_UNUSED(interface);
    return QAElement();
}

void QAccessibleHierarchyManager::registerInterface(QObject * qobject, HIObjectRef hiobject, QInterfaceFactory *interfaceFactory)
{
    Q_UNUSED(qobject);
    Q_UNUSED(hiobject);
    Q_UNUSED(interfaceFactory);
}

void QAccessibleHierarchyManager::registerChildren(const QAInterface &interface)
{
    QObject * const object = interface.object();
    if (object == 0)
        return;

    QInterfaceFactory *interfaceFactory = qobjectElementHash.value(object);
    
    if (interfaceFactory == 0)
        return;

    interfaceFactory->registerChildren();
}

QAInterface QAccessibleHierarchyManager::lookup(const AXUIElementRef &element)
{
     if (element == 0)
        return QAInterface();

    return QAInterface();
}

QAInterface QAccessibleHierarchyManager::lookup(const QAElement &element)
{
    return lookup(element.element());
}

QAElement QAccessibleHierarchyManager::lookup(const QAInterface &interface)
{
    if (interface.isValid() == false)
        return QAElement();

    QInterfaceFactory *factory = qobjectElementHash.value(interface.objectInterface().object());
    if (factory == 0)
        return QAElement();

    return factory->element(interface);
}

QAElement QAccessibleHierarchyManager::lookup(QObject * const object, int id)
{
    QInterfaceFactory *factory = qobjectElementHash.value(object);
    if (factory == 0)
        return QAElement();

    return factory->element(id);
}

/*
    Standard interface mapping, return the stored interface
    or HIObjectRef, and there is an one-to-one mapping between
    the identifier and child.
*/
class QStandardInterfaceFactory : public QInterfaceFactory
{
public:
    QStandardInterfaceFactory(const QAInterface &interface)
    : m_interface(interface), object(interface.hiObject())
    {
        CFRetain(object);
    }
    
    ~QStandardInterfaceFactory()
    {
         CFRelease(object);
    }

    
    QAInterface interface(UInt64 identifier)
    {
        const int child = identifier;
        return QAInterface(m_interface, child);
    }

    QAElement element(int id)
    {
        return QAElement(object, id);
    }

    QAElement element(const QAInterface &interface)
    {
        if (interface.object() == 0)
            return QAElement();
        return QAElement(object, interface.id());
    }

    void registerChildren()
    {
        const int childCount = m_interface.childCount();
        for (int i = 1; i <= childCount; ++i) {
            accessibleHierarchyManager()->registerInterface(m_interface.navigate(QAccessible::Child, i));
        }
    }

private:
    QAInterface m_interface;
    HIObjectRef object;
};

/*
    Interface mapping where that creates one HIObject for each interface child.
*/
class QMultipleHIObjectFactory : public QInterfaceFactory
{
public:
    QMultipleHIObjectFactory(const QAInterface &interface)
    : m_interface(interface)
    {  }
    
    ~QMultipleHIObjectFactory()
    {
        foreach (HIObjectRef object, objects) {
            CFRelease(object);
        }
    }

    QAInterface interface(UInt64 identifier)
    {
        const int child = identifier;
        return QAInterface(m_interface, child);
    }

    QAElement element(int child)
    {
        if (child == 0)
            return QAElement(m_interface.hiObject(), 0);
        
        if (child > objects.count())
            return QAElement();

        return QAElement(objects.at(child - 1), child);
    }

    void registerChildren()
    {
    }

private:
    QAInterface m_interface;
    QList<HIObjectRef> objects;
};

class QItemViewInterfaceFactory : public QInterfaceFactory
{
public:
    QItemViewInterfaceFactory(const QAInterface &interface)
    : m_interface(interface), object(interface.hiObject())
    {
        CFRetain(object);
        columnCount = 0;
        if (QTableView * tableView = qobject_cast<QTableView *>(interface.parent().object())) {
            if (tableView->model())
                columnCount = tableView->model()->columnCount();
            if (tableView->verticalHeader())
                ++columnCount;
        }
    }
    
    ~QItemViewInterfaceFactory()
    {
        CFRelease(object);
    }

    QAInterface interface(UInt64 identifier)
    {
        if (identifier == 0)
            return m_interface;

        if (m_interface.role() == QAccessible::List)
            return m_interface.childAt(identifier);
        
        if (m_interface.role() == QAccessible::Table) {
            const int index = identifier;
            if (index == 0)
                return m_interface; // return the item view interface.
           
            const int rowIndex = (index - 1) / (columnCount + 1);
            const int cellIndex = (index - 1)  % (columnCount + 1);
/*
            qDebug() << "index" << index;
            qDebug() << "rowIndex" << rowIndex;
            qDebug() << "cellIndex" << cellIndex;
*/
            const QAInterface rowInterface = m_interface.childAt(rowIndex + 1);

            if ((cellIndex) == 0) // Is it a row?
                return rowInterface;
            else {
                return rowInterface.childAt(cellIndex);
            }
        }

        return QAInterface();
    }

    QAElement element(int id)
    {
        if (id != 0) {
            return QAElement();
        }
        return QAElement(object, 0);
    }

    QAElement element(const QAInterface &interface)
    {
        if (interface.object() && interface.object() == m_interface.object()) {
            return QAElement(object, 0);
        } else if (m_interface.role() == QAccessible::List) {
            if (interface.parent().object() && interface.parent().object() == m_interface.object())
                return QAElement(object, m_interface.indexOfChild(interface));
        } else if (m_interface.role() == QAccessible::Table) {
            QAInterface currentInterface = interface;
            int index = 0;

            while (currentInterface.isValid() && currentInterface.object() == 0) {
                const QAInterface parentInterface = currentInterface.parent();
/*
                qDebug() << "current index" << index;
                qDebug() << "current interface" << interface;

                qDebug() << "parent interface" << parentInterface;
                qDebug() << "grandparent interface" << parentInterface.parent();
                qDebug() << "childCount" << interface.childCount();
                qDebug() << "index of child" << parentInterface.indexOfChild(currentInterface);
*/
                index += ((parentInterface.indexOfChild(currentInterface) - 1) * (currentInterface.childCount() + 1)) + 1;
                currentInterface = parentInterface;
//                qDebug() << "new current interface" << currentInterface;
            }
            if (currentInterface.object() == m_interface.object())
                return QAElement(object, index);


        }
        return QAElement();
    }

    void registerChildren()
    {
        // Item view child interfraces don't have their own qobjects, so there is nothing to register here.
    }

private:
    QAInterface m_interface;
    HIObjectRef object;
    int columnCount; // for table views;
};

QList<QAElement> lookup(const QList<QAInterface> &interfaces)
{
    QList<QAElement> elements;
    foreach (const QAInterface &interface, interfaces)
        if (interface.isValid()) {
            const QAElement element = accessibleHierarchyManager()->lookup(interface);
            if (element.isValid())
                elements.append(element);
        }
    return elements;
}

// Debug output helpers:
/*
static QString nameForEventKind(UInt32 kind)
{
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


//  Gets the AccessibleObject parameter from an event.
static inline AXUIElementRef getAccessibleObjectParameter(EventRef event)
{
    AXUIElementRef element;
    GetEventParameter(event, kEventParamAccessibleObject, typeCFTypeRef, 0,
                        sizeof(element), 0, &element);
    return element;
}

/*
    Translates a QAccessible::Role into a mac accessibility role.
*/
static CFStringRef macRole(const QAInterface &interface)
{
    const QAccessible::Role qtRole = interface.role();

//    qDebug() << "role for" << interface.object() << "interface role" << hex << qtRole;

    // Qt accessibility:  QAccessible::Splitter contains QAccessible::Grip.
    // Mac accessibility: AXSplitGroup contains AXSplitter.
    if (qtRole == QAccessible::Grip) {
        const QAInterface parent = interface.parent();
        if (parent.isValid() && parent.role() == QAccessible::Splitter)
            return CFStringRef(QAXSplitterRole);
    }

    // Tab widgets and standalone tab bars get the kAXTabGroupRole. Accessibility
    // for tab bars emebedded in a tab widget is handled by the tab widget.
    if (isTabWidget(interface) || isStandaloneTabBar(interface))
        return kAXTabGroupRole;

    if (QObject *object = interface.object()) {
        // ### The interface for an abstract scroll area returns the generic "Client"
        // role, so we have to to an extra detect on the QObject here.
        if (object->inherits("QAbstractScrollArea") && interface.id() == 0)
            return CFStringRef(QAXScrollAreaRole);

        if (object->inherits("QDockWidget"))
            return CFStringRef(QAXUnknownRole);
    }

    int i = 0;
    int testRole = text_bindings[i][0].qt;
    while (testRole != -1) {
        if (testRole == qtRole)
            return CFStringRef(text_bindings[i][0].mac);
        ++i;
        testRole = text_bindings[i][0].qt;
    }

//    qDebug() << "got unknown role!" << interface << interface.parent();

    return CFStringRef(QAXUnknownRole);
}

struct IsWindowTest
{
    static inline bool test(const QAInterface &interface)
    {
        return (interface.role() == QAccessible::Window);
    }
};

struct IsWindowAndNotDrawerOrSheetTest
{
    static inline bool test(const QAInterface &interface)
    {
        QWidget * const widget = qobject_cast<QWidget*>(interface.object());
        return (interface.role() == QAccessible::Window &&
                widget && widget->isWindow() &&
                !qt_mac_is_macdrawer(widget) &&
                !qt_mac_is_macsheet(widget));
    }
};

/*
    Navigates up the iterfaces ancestor hierachy until a QAccessibleInterface that
    passes the Test is found. If we reach a interface that is a HIView we stop the
    search and call AXUIElementCopyAttributeValue.
*/
template <typename TestType>
OSStatus navigateAncestors(EventHandlerCallRef next_ref, EventRef event, const QAInterface &interface, CFStringRef attribute)
{
    if (interface.isHIView())
        return CallNextEventHandler(next_ref, event);

    QAInterface current = interface;
    QAElement element;
    while (current.isValid()) {
        if (TestType::test(interface)) {
            element = accessibleHierarchyManager()->lookup(current);
            break;
        }

        // If we reach an InterfaceItem that is a HiView we can hand of the search to
        // the system event handler. This is the common case.
        if (current.isHIView()) {
            CFTypeRef value = 0;
            const QAElement currentElement = accessibleHierarchyManager()->lookup(current);
            AXError err = AXUIElementCopyAttributeValue(currentElement.element(), attribute, &value);
            AXUIElementRef newElement = (AXUIElementRef)value;

            if (err == noErr)
                element = QAElement(newElement);

            if (newElement != 0)
                CFRelease(newElement);
            break;
        }

        QAInterface next = current.parent();
        if (next.isValid() == false)
            break;
        if (next == current)
            break;
        current = next;
    }

    if (element.isValid() == false)
        return eventNotHandledErr;


    AXUIElementRef elementRef = element.element();
    SetEventParameter(event, kEventParamAccessibleAttributeValue, typeCFTypeRef,
                                      sizeof(elementRef), &elementRef);
    return noErr;
}

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
