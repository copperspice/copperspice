/***********************************************************************
*
* Copyright (c) 2012-2025 Barbara Geller
* Copyright (c) 2012-2025 Ansel Sermersheim
*
* Copyright (C) 2013 Klarälvdalens Datakonsult AB (KDAB).
*
* This file is part of CopperSpice.
*
* CopperSpice is free software, released under the BSD 2-Clause license.
* For license details refer to LICENSE provided with this project.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* https://opensource.org/licenses/BSD-2-Clause
*
***********************************************************************/

#include <qwayland_inputcontext_p.h>

#include <qapplication.h>
#include <qplatform_window.h>
#include <qwindow.h>

#ifndef QT_NO_WAYLAND_XKB
#include <xkbcommon/xkbcommon.h>
#endif

namespace QtWaylandClient {

static Qt::Key toQtKey(uint32_t sym)
{
#ifndef QT_NO_WAYLAND_XKB

   switch (static_cast < xkb_keysym_t > (sym)) {
      case XKB_KEY_BackSpace:
         return Qt::Key_Backspace;

      case XKB_KEY_Return:
         return Qt::Key_Return;

      case XKB_KEY_Left:
         return Qt::Key_Left;

      case XKB_KEY_Up:
         return Qt::Key_Up;

      case XKB_KEY_Right:
         return Qt::Key_Right;

      case XKB_KEY_Down:
         return Qt::Key_Down;

      default:
         return Qt::Key_unknown;
   }

#else
   (void) sym;
   return Qt::Key_unknown;
#endif
}

static QEvent::Type toQEventType(uint32_t state)
{
   switch (static_cast < wl_keyboard_key_state > (state)) {
      case WL_KEYBOARD_KEY_STATE_RELEASED:
         return QEvent::KeyRelease;

      case WL_KEYBOARD_KEY_STATE_PRESSED:
      default:
         return QEvent::KeyPress;
   }
}

QWaylandTextInput::QWaylandTextInput(struct ::wl_text_input *text_input)
   : QtWayland::wl_text_input(text_input), m_commit(), m_serial(0), m_resetSerial(0)
{
}

QString QWaylandTextInput::commitString() const
{
   return m_commit;
}

void QWaylandTextInput::reset()
{
   wl_text_input::reset();
   updateState();
   m_resetSerial = m_serial;
}

void QWaylandTextInput::updateState()
{
   if (! QGuiApplication::focusObject()) {
      return;
   }

   QInputMethodQueryEvent event(Qt::ImQueryAll);
   QCoreApplication::sendEvent(QGuiApplication::focusObject(), &event);

   const QString &text = event.value(Qt::ImSurroundingText).toString();
   const int cursor = event.value(Qt::ImCursorPosition).toInt();
   const int anchor = event.value(Qt::ImAnchorPosition).toInt();

   set_surrounding_text(text, text.leftView(cursor).size_storage(), text.leftView(anchor).size_storage());

   commit_state(++m_serial);
}

void QWaylandTextInput::text_input_preedit_string(uint32_t serial, const QString &text, const QString &commit)
{
   (void) serial;

   if (! QGuiApplication::focusObject()) {
      return;
   }

   m_commit = commit;
   QList<QInputMethodEvent::Attribute> attributes;
   QInputMethodEvent event(text, attributes);
   QCoreApplication::sendEvent(QGuiApplication::focusObject(), &event);
}

void QWaylandTextInput::text_input_commit_string(uint32_t serial, const QString &text)
{
   (void) serial;

   if (! QGuiApplication::focusObject()) {
      return;
   }

   QInputMethodEvent event;
   event.setCommitString(text);
   QCoreApplication::sendEvent(QGuiApplication::focusObject(), &event);

   m_commit = QString();
}

void QWaylandTextInput::text_input_enter(wl_surface *)
{
   updateState();
   m_resetSerial = m_serial;
}

void QWaylandTextInput::text_input_leave()
{
   if (! m_commit.isEmpty()) {
      text_input_commit_string(0, m_commit);
   }
}

void QWaylandTextInput::text_input_keysym(uint32_t serial, uint32_t time, uint32_t sym, uint32_t state, uint32_t modifiers)
{
   (void) serial;
   (void) time;
   (void) modifiers;

   if (! QGuiApplication::focusObject()) {
      return;
   }

   // TODO: Convert modifiers to Qt::KeyboardModifiers.
   QKeyEvent event(toQEventType(state), toQtKey(sym), Qt::NoModifier);
   QCoreApplication::sendEvent(qGuiApp->focusWindow(), &event);
}

QWaylandInputContext::QWaylandInputContext(QWaylandDisplay *display)
   : mTextInput()
{
}

bool QWaylandInputContext::isValid() const
{
   // pending implementation
   return false;
}

void QWaylandInputContext::reset()
{
   if (! ensureTextInput()) {
      return;
   }

   mTextInput->reset();
}

void QWaylandInputContext::commit()
{
   if (! ensureTextInput()) {
      return;
   }

   if (! QGuiApplication::focusObject()) {
      return;
   }

   QInputMethodEvent event;
   event.setCommitString(mTextInput->commitString());
   QCoreApplication::sendEvent(QGuiApplication::focusObject(), &event);

   mTextInput->reset();
}

void QWaylandInputContext::update(Qt::InputMethodQueries queries)
{
   (void) queries;

   if (! ensureTextInput()) {
      return;
   }

   mTextInput->updateState();
}

void QWaylandInputContext::invokeAction(QInputMethod::Action, int cursorPosition)
{
   if (! ensureTextInput()) {
      return;
   }

   mTextInput->invoke_action(0, cursorPosition);
}

void QWaylandInputContext::showInputPanel()
{
   if (! ensureTextInput()) {
      return;
   }

   mTextInput->show_input_panel();
}

void QWaylandInputContext::hideInputPanel()
{
   if (! ensureTextInput()) {
      return;
   }

   mTextInput->hide_input_panel();
}

bool QWaylandInputContext::isInputPanelVisible() const
{
   return false;
}

void QWaylandInputContext::setFocusObject(QObject *object)
{
   if (! ensureTextInput()) {
      return;
   }

   if (object == nullptr) {
      // pending implementation
      return;
   }

   QWindow *window = QGuiApplication::focusWindow();

   if (window == nullptr || window->handle() == nullptr) {
      return;
   }

}

bool QWaylandInputContext::ensureTextInput()
{
   if (mTextInput) {
      return true;
   }

   if (! isValid()) {
      return false;
   }

   // pending implementation

   return true;
}

}
