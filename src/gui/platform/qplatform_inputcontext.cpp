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

#include <qplatform_inputcontext.h>
#include <qguiapplication.h>
#include <qrect.h>

#include <qkeymapper_p.h>
#include <qplatform_inputcontext_p.h>

/*!
    \internal
 */
QPlatformInputContext::QPlatformInputContext()
    : QObject(*(new QPlatformInputContextPrivate))
{
}

/*!
    \internal
 */
QPlatformInputContext::~QPlatformInputContext()
{
}


bool QPlatformInputContext::isValid() const
{
    return false;
}


bool QPlatformInputContext::hasCapability(Capability capability) const
{
    Q_UNUSED(capability)
    return true;
}


void QPlatformInputContext::reset()
{
}

void QPlatformInputContext::commit()
{
}


void QPlatformInputContext::update(Qt::InputMethodQueries)
{
}


void QPlatformInputContext::invokeAction(QInputMethod::Action action, int cursorPosition)
{
    Q_UNUSED(cursorPosition)
    // Default behavior for simple ephemeral input contexts. Some
    // complex input contexts should not be reset here.
    if (action == QInputMethod::Click)
        reset();
}

bool QPlatformInputContext::filterEvent(const QEvent *event)
{
    Q_UNUSED(event)
    return false;
}


QRectF QPlatformInputContext::keyboardRect() const
{
    return QRectF();
}

void QPlatformInputContext::emitKeyboardRectChanged()
{
    emit QGuiApplication::inputMethod()->keyboardRectangleChanged();
}


bool QPlatformInputContext::isAnimating() const
{
    return false;
}


void QPlatformInputContext::emitAnimatingChanged()
{
    emit QGuiApplication::inputMethod()->animatingChanged();
}

void QPlatformInputContext::showInputPanel()
{
}


void QPlatformInputContext::hideInputPanel()
{
}

/*!
    Returns input panel visibility status. Default implementation returns \c false.
 */
bool QPlatformInputContext::isInputPanelVisible() const
{
    return false;
}

/*!
    Active QPlatformInputContext is responsible for providing visible property to QInputMethod.
    In addition of providing the value in isInputPanelVisible function, it also needs to call this emit
    function whenever the property changes.
 */
void QPlatformInputContext::emitInputPanelVisibleChanged()
{
    emit QGuiApplication::inputMethod()->visibleChanged();
}

QLocale QPlatformInputContext::locale() const
{
    return qt_keymapper_private()->keyboardInputLocale;
}

void QPlatformInputContext::emitLocaleChanged()
{
    emit QGuiApplication::inputMethod()->localeChanged();
}

Qt::LayoutDirection QPlatformInputContext::inputDirection() const
{
    return qt_keymapper_private()->keyboardInputDirection;
}

void QPlatformInputContext::emitInputDirectionChanged(Qt::LayoutDirection newDirection)
{
    emit QGuiApplication::inputMethod()->inputDirectionChanged(newDirection);
}

/*!
    This virtual method gets called to notify updated focus to \a object.
    \warning Input methods must not call this function directly.
 */
void QPlatformInputContext::setFocusObject(QObject *object)
{
    Q_UNUSED(object)
}

/*!
    Returns \c true if current focus object supports input method events.
 */
bool QPlatformInputContext::inputMethodAccepted() const
{
    return QPlatformInputContextPrivate::s_inputMethodAccepted;
}

bool QPlatformInputContextPrivate::s_inputMethodAccepted = false;

void QPlatformInputContextPrivate::setInputMethodAccepted(bool accepted)
{
    QPlatformInputContextPrivate::s_inputMethodAccepted = accepted;
}

