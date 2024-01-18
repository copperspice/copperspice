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

#include "private/qdeclarativeevents_p_p.h"

QT_BEGIN_NAMESPACE
/*!
    \qmlclass KeyEvent QDeclarativeKeyEvent
    \since 4.7
    \ingroup qml-event-elements

    \brief The KeyEvent object provides information about a key event.

    For example, the following changes the Item's state property when the Enter
    key is pressed:
    \qml
Item {
    focus: true
    Keys.onPressed: { if (event.key == Qt.Key_Enter) state = 'ShowDetails'; }
}
    \endqml
*/

/*!
    \qmlproperty int KeyEvent::key

    This property holds the code of the key that was pressed or released.

    See \l {Qt::Key}{Qt.Key} for the list of keyboard codes. These codes are
    independent of the underlying window system. Note that this
    function does not distinguish between capital and non-capital
    letters, use the text() function (returning the Unicode text the
    key generated) for this purpose.

    A value of either 0 or \l {Qt::Key_unknown}{Qt.Key_Unknown} means that the event is not
    the result of a known key; for example, it may be the result of
    a compose sequence, a keyboard macro, or due to key event
    compression.
*/

/*!
    \qmlproperty string KeyEvent::text

    This property holds the Unicode text that the key generated.
    The text returned can be an empty string in cases where modifier keys,
    such as Shift, Control, Alt, and Meta, are being pressed or released.
    In such cases \c key will contain a valid value
*/

/*!
    \qmlproperty bool KeyEvent::isAutoRepeat

    This property holds whether this event comes from an auto-repeating key.
*/

/*!
    \qmlproperty int KeyEvent::count

    This property holds the number of keys involved in this event. If \l KeyEvent::text
    is not empty, this is simply the length of the string.
*/

/*!
    \qmlproperty bool KeyEvent::accepted

    Setting \a accepted to true prevents the key event from being
    propagated to the item's parent.

    Generally, if the item acts on the key event then it should be accepted
    so that ancestor items do not also respond to the same event.
*/

/*!
    \qmlproperty int KeyEvent::modifiers

    This property holds the keyboard modifier flags that existed immediately
    before the event occurred.

    It contains a bitwise combination of:
    \list
    \o Qt.NoModifier - No modifier key is pressed.
    \o Qt.ShiftModifier - A Shift key on the keyboard is pressed.
    \o Qt.ControlModifier - A Ctrl key on the keyboard is pressed.
    \o Qt.AltModifier - An Alt key on the keyboard is pressed.
    \o Qt.MetaModifier - A Meta key on the keyboard is pressed.
    \o Qt.KeypadModifier - A keypad button is pressed.
    \endlist

    For example, to react to a Shift key + Enter key combination:
    \qml
    Item {
        focus: true
        Keys.onPressed: {
            if ((event.key == Qt.Key_Enter) && (event.modifiers & Qt.ShiftModifier))
                doSomething();
        }
    }
    \endqml
*/


/*!
    \qmlclass MouseEvent QDeclarativeMouseEvent
    \since 4.7
    \ingroup qml-event-elements

    \brief The MouseEvent object provides information about a mouse event.

    The position of the mouse can be found via the \l x and \l y properties.
    The button that caused the event is available via the \l button property.

    \sa MouseArea
*/

/*!
    \internal
    \class QDeclarativeMouseEvent
*/

/*!
    \qmlproperty int MouseEvent::x
    \qmlproperty int MouseEvent::y

    These properties hold the coordinates of the position supplied by the mouse event.
*/


/*!
    \qmlproperty bool MouseEvent::accepted

    Setting \a accepted to true prevents the mouse event from being
    propagated to items below this item.

    Generally, if the item acts on the mouse event then it should be accepted
    so that items lower in the stacking order do not also respond to the same event.
*/

/*!
    \qmlproperty enumeration MouseEvent::button

    This property holds the button that caused the event.  It can be one of:
    \list
    \o Qt.LeftButton
    \o Qt.RightButton
    \o Qt.MiddleButton
    \endlist
*/

/*!
    \qmlproperty bool MouseEvent::wasHeld

    This property is true if the mouse button has been held pressed longer the
    threshold (800ms).
*/

/*!
    \qmlproperty int MouseEvent::buttons

    This property holds the mouse buttons pressed when the event was generated.
    For mouse move events, this is all buttons that are pressed down. For mouse
    press and double click events this includes the button that caused the event.
    For mouse release events this excludes the button that caused the event.

    It contains a bitwise combination of:
    \list
    \o Qt.LeftButton
    \o Qt.RightButton
    \o Qt.MiddleButton
    \endlist
*/

/*!
    \qmlproperty int MouseEvent::modifiers

    This property holds the keyboard modifier flags that existed immediately
    before the event occurred.

    It contains a bitwise combination of:
    \list
    \o Qt.NoModifier - No modifier key is pressed.
    \o Qt.ShiftModifier - A Shift key on the keyboard is pressed.
    \o Qt.ControlModifier - A Ctrl key on the keyboard is pressed.
    \o Qt.AltModifier - An Alt key on the keyboard is pressed.
    \o Qt.MetaModifier - A Meta key on the keyboard is pressed.
    \o Qt.KeypadModifier - A keypad button is pressed.
    \endlist

    For example, to react to a Shift key + Left mouse button click:
    \qml
    MouseArea {
        onClicked: {
            if ((mouse.button == Qt.LeftButton) && (mouse.modifiers & Qt.ShiftModifier))
                doSomething();
        }
    }
    \endqml
*/

QT_END_NAMESPACE
