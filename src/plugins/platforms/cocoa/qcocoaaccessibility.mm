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

#include "qcocoaaccessibility.h"
#include "qcocoaaccessibilityelement.h"
#include <qaccessible.h>
#include <qcore_mac_p.h>

#ifndef QT_NO_ACCESSIBILITY

QCocoaAccessibility::QCocoaAccessibility()
{

}

QCocoaAccessibility::~QCocoaAccessibility()
{

}

void QCocoaAccessibility::notifyAccessibilityUpdate(QAccessibleEvent *event)
{
   if (!isActive() || !event->accessibleInterface() || !event->accessibleInterface()->isValid()) {
      return;
   }
   QMacAccessibilityElement *element = [QMacAccessibilityElement elementWithId: event->uniqueId()];
   if (!element) {
      qWarning() << "QCocoaAccessibility::notifyAccessibilityUpdate: invalid element";
      return;
   }

   switch (event->type()) {
      case QAccessible::Focus: {
         NSAccessibilityPostNotification(element, NSAccessibilityFocusedUIElementChangedNotification);
         break;
      }
      case QAccessible::StateChanged:
      case QAccessible::ValueChanged:
      case QAccessible::TextInserted:
      case QAccessible::TextRemoved:
      case QAccessible::TextUpdated:
         NSAccessibilityPostNotification(element, NSAccessibilityValueChangedNotification);
         break;
      case QAccessible::TextCaretMoved:
      case QAccessible::TextSelectionChanged:
         NSAccessibilityPostNotification(element, NSAccessibilitySelectedTextChangedNotification);
         break;
      case QAccessible::NameChanged:
         NSAccessibilityPostNotification(element, NSAccessibilityTitleChangedNotification);
         break;
      default:
         break;
   }
}

void QCocoaAccessibility::setRootObject(QObject *o)
{
   (void) o;
}

void QCocoaAccessibility::initialize()
{

}

void QCocoaAccessibility::cleanup()
{

}

namespace QCocoaAccessible {

using QMacAccessibiltyRoleMap = QMap<QAccessible::Role, NSString *>;

static QMacAccessibiltyRoleMap *qMacAccessibiltyRoleMap()
{
   static QMacAccessibiltyRoleMap retval;
   return &retval;
}

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
   roleMap[QAccessible::Button] = NSAccessibilityButtonRole;
   roleMap[QAccessible::EditableText] = NSAccessibilityTextFieldRole;
   roleMap[QAccessible::Link] = NSAccessibilityLinkRole;
   roleMap[QAccessible::Indicator] = NSAccessibilityValueIndicatorRole;
   roleMap[QAccessible::Splitter] = NSAccessibilitySplitGroupRole;
   roleMap[QAccessible::List] = NSAccessibilityListRole;
   roleMap[QAccessible::ListItem] = NSAccessibilityStaticTextRole;
   roleMap[QAccessible::Cell] = NSAccessibilityStaticTextRole;
   roleMap[QAccessible::Client] = NSAccessibilityGroupRole;
   roleMap[QAccessible::Paragraph] = NSAccessibilityGroupRole;
   roleMap[QAccessible::Section] = NSAccessibilityGroupRole;
   roleMap[QAccessible::WebDocument] = NSAccessibilityGroupRole;
   roleMap[QAccessible::ColorChooser] = NSAccessibilityColorWellRole;
   roleMap[QAccessible::Footer] = NSAccessibilityGroupRole;
   roleMap[QAccessible::Form] = NSAccessibilityGroupRole;
   roleMap[QAccessible::Heading] = @"AXHeading";
   roleMap[QAccessible::Note] = NSAccessibilityGroupRole;
   roleMap[QAccessible::ComplementaryContent] = NSAccessibilityGroupRole;
   roleMap[QAccessible::Graphic] = NSAccessibilityImageRole;
}

/*
    Returns a Cocoa accessibility role for the given interface, or
    NSAccessibilityUnknownRole if no role mapping is found.
*/
NSString *macRole(QAccessibleInterface *interface)
{
   QAccessible::Role qtRole = interface->role();
   QMacAccessibiltyRoleMap &roleMap = *qMacAccessibiltyRoleMap();

   if (roleMap.isEmpty()) {
      populateRoleMap();
   }

   // MAC_ACCESSIBILTY_DEBUG() << "role for" << interface.object() << "interface role" << hex << qtRole;

   if (roleMap.contains(qtRole)) {
      // MAC_ACCESSIBILTY_DEBUG() << "return" <<  roleMap[qtRole];
      if (roleMap[qtRole] == NSAccessibilityTextFieldRole && interface->state().multiLine) {
         return NSAccessibilityTextAreaRole;
      }
      return roleMap[qtRole];
   }

   // Treat unknown roles as generic group container items. Returning
   // NSAccessibilityUnknownRole is also possible but makes the screen
   // reader focus on the item instead of passing focus to child items.
   // MAC_ACCESSIBILTY_DEBUG() << "return NSAccessibilityGroupRole for unknown role";

   return NSAccessibilityGroupRole;
}

/*
    Returns a Cocoa sub role for the given interface.
*/
NSString *macSubrole(QAccessibleInterface *interface)
{
   QAccessible::State s = interface->state();
   if (s.searchEdit) {
      return NSAccessibilitySearchFieldSubrole;
   }
   if (s.passwordEdit) {
      return NSAccessibilitySecureTextFieldSubrole;
   }
   return nil;
}

/*
    Cocoa accessibility supports ignoring elements, which means that
    the elements are still present in the accessibility tree but is
    not used by the screen reader.
*/
bool shouldBeIgnored(QAccessibleInterface *interface)
{
   // Cocoa accessibility does not have an attribute that corresponds to the Invisible/Offscreen
   // state. Ignore interfaces with those flags set.
   const QAccessible::State state = interface->state();
   if (state.invisible ||
      state.offscreen ||
      state.invalid) {
      return true;
   }

   // Some roles are not interesting. In particular, container roles should be
   // ignored in order to flatten the accessibility tree as seen by the user.
   const QAccessible::Role role = interface->role();
   if (role == QAccessible::Border ||      // QFrame
      role == QAccessible::Application || // We use the system-provided application element.
      role == QAccessible::MenuItem ||    // The system also provides the menu items.
      role == QAccessible::ToolBar ||     // Access the tool buttons directly.
      role == QAccessible::Pane ||        // Scroll areas.
      role == QAccessible::Client) {      // The default for QWidget.
      return true;
   }

   NSString *mac_role = macRole(interface);
   if (mac_role == NSAccessibilityWindowRole || // We use the system-provided window elements.
      mac_role == NSAccessibilityUnknownRole) {
      return true;
   }

   // Client is a generic role returned by plain QWidgets or other
   // widgets that does not have separate QAccessible interface, such
   // as the TabWidget. Return false unless macRole gives the interface
   // a special role.
   if (role == QAccessible::Client && mac_role == NSAccessibilityUnknownRole) {
      return true;
   }

   if (QObject *const object = interface->object()) {
      const QString className = object->metaObject()->className();

      // VoiceOver focusing on tool tips can be confusing. The contents of the
      // tool tip is available through the description attribute anyway, so
      // we disable accessibility for tool tips.

      if (className == "QTipLabel") {
         return true;
      }
   }

   return false;
}

NSArray *unignoredChildren(QAccessibleInterface *interface)
{
   int numKids = interface->childCount();

   NSMutableArray *kids = [NSMutableArray arrayWithCapacity: numKids];
   for (int i = 0; i < numKids; ++i) {
      QAccessibleInterface *child = interface->child(i);
      if (!child || !child->isValid() || child->state().invalid || child->state().invisible) {
         continue;
      }

      QAccessible::Id childId = QAccessible::uniqueId(child);

      QMacAccessibilityElement *element = [QMacAccessibilityElement elementWithId: childId];
      if (element) {
         [kids addObject: element];
      } else {
         qWarning() << "QCocoaAccessibility: invalid child";
      }
   }
   return NSAccessibilityUnignoredChildren(kids);
}

/*
    Translates a predefined QAccessibleActionInterface action to a Mac action constant.
    Returns 0 if the Action has no mac equivalent. Ownership of the NSString is not transferred
*/
NSString *getTranslatedAction(const QString &qtAction)
{
   if (qtAction == QAccessibleActionInterface::pressAction()) {
      return NSAccessibilityPressAction;
   } else if (qtAction == QAccessibleActionInterface::increaseAction()) {
      return NSAccessibilityIncrementAction;
   } else if (qtAction == QAccessibleActionInterface::decreaseAction()) {
      return NSAccessibilityDecrementAction;
   } else if (qtAction == QAccessibleActionInterface::showMenuAction()) {
      return NSAccessibilityShowMenuAction;
   } else if (qtAction == QAccessibleActionInterface::setFocusAction()) { // Not 100% sure on this one
      return NSAccessibilityRaiseAction;
   } else if (qtAction == QAccessibleActionInterface::toggleAction()) {
      return NSAccessibilityPressAction;
   }

   // Not translated:
   //
   // Qt:
   //     static const QString &checkAction();
   //     static const QString &uncheckAction();
   //
   // Cocoa:
   //      NSAccessibilityConfirmAction;
   //      NSAccessibilityPickAction;
   //      NSAccessibilityCancelAction;
   //      NSAccessibilityDeleteAction;

   return nullptr;
}

/*
    Translates between a Mac action constant and a QAccessibleActionInterface action
    Returns an empty QString if there is no predefined equivalent
*/
QString translateAction(NSString *nsAction, QAccessibleInterface *interface)
{
   if ([nsAction compare: NSAccessibilityPressAction] == NSOrderedSame) {
      if (interface->role() == QAccessible::CheckBox || interface->role() == QAccessible::RadioButton) {
         return QAccessibleActionInterface::toggleAction();
      }
      return QAccessibleActionInterface::pressAction();
   } else if ([nsAction compare: NSAccessibilityIncrementAction] == NSOrderedSame) {
      return QAccessibleActionInterface::increaseAction();
   } else if ([nsAction compare: NSAccessibilityDecrementAction] == NSOrderedSame) {
      return QAccessibleActionInterface::decreaseAction();
   } else if ([nsAction compare: NSAccessibilityShowMenuAction] == NSOrderedSame) {
      return QAccessibleActionInterface::showMenuAction();
   } else if ([nsAction compare: NSAccessibilityRaiseAction] == NSOrderedSame) {
      return QAccessibleActionInterface::setFocusAction();
   }

   // See getTranslatedAction for not matched translations.

   return QString();
}

bool hasValueAttribute(QAccessibleInterface *interface)
{
   Q_ASSERT(interface);
   const QAccessible::Role qtrole = interface->role();
   if (qtrole == QAccessible::EditableText
      || interface->valueInterface()
      || interface->state().checkable) {
      return true;
   }

   return false;
}

id getValueAttribute(QAccessibleInterface *interface)
{
   const QAccessible::Role qtrole = interface->role();
   if (qtrole == QAccessible::EditableText) {
      if (QAccessibleTextInterface *textInterface = interface->textInterface()) {

         int begin = 0;
         int end = textInterface->characterCount();
         QString text;
         if (interface->state().passwordEdit) {
            // return round password replacement chars
            text = QString(end, QChar(kBulletUnicode));
         } else {
            // VoiceOver will read out the entire text string at once when returning
            // text as a value. For large text edits the size of the returned string
            // needs to be limited and text range attributes need to be used instead.
            // NSTextEdit returns the first sentence as the value, Do the same here:
            // ### call to textAfterOffset hangs. Booo!
            //if (textInterface->characterCount() > 0)
            //    textInterface->textAfterOffset(0, QAccessible2::SentenceBoundary, &begin, &end);
            text = textInterface->text(begin, end);
         }
         return QCFString::toNSString(text);
      }
   }

   if (QAccessibleValueInterface *valueInterface = interface->valueInterface()) {
      return QCFString::toNSString(valueInterface->currentValue().toString());
   }

   if (interface->state().checkable) {
      return [NSNumber numberWithInt: (interface->state().checked ? 1 : 0)];
   }

   return nil;
}

} // namespace QCocoaAccessible

#endif // QT_NO_ACCESSIBILITY


