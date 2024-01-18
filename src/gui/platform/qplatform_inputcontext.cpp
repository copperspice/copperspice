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

#include <qplatform_inputcontext.h>
#include <qguiapplication.h>
#include <qrect.h>

#include <qkeymapper_p.h>
#include <qplatform_inputcontext_p.h>

bool QPlatformInputContextPrivate::s_inputMethodAccepted = false;

QPlatformInputContext::QPlatformInputContext()
   : d_ptr(new QPlatformInputContextPrivate)
{
   d_ptr->q_ptr = this;
}

QPlatformInputContext::~QPlatformInputContext()
{
}

bool QPlatformInputContext::isValid() const
{
   return false;
}

bool QPlatformInputContext::hasCapability(Capability capability) const
{
   (void) capability;

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
   (void) cursorPosition;

   // Default behavior for simple ephemeral input contexts. Some
   // complex input contexts should not be reset here.
   if (action == QInputMethod::Click) {
      reset();
   }
}

bool QPlatformInputContext::filterEvent(const QEvent *event)
{
   (void) event;

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

bool QPlatformInputContext::isInputPanelVisible() const
{
   return false;
}

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

void QPlatformInputContext::setFocusObject(QObject *object)
{
   (void) object;
}

bool QPlatformInputContext::inputMethodAccepted() const
{
   return QPlatformInputContextPrivate::s_inputMethodAccepted;
}

void QPlatformInputContextPrivate::setInputMethodAccepted(bool accepted)
{
   QPlatformInputContextPrivate::s_inputMethodAccepted = accepted;
}

