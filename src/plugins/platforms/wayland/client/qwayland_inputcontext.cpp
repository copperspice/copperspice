/***********************************************************************
*
* Copyright (c) 2012-2026 Barbara Geller
* Copyright (c) 2012-2026 Ansel Sermersheim
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

#include <qwayland_display_p.h>
#include <qwayland_inputdevice_p.h>
#include <qwayland_window_p.h>

#include <xkbcommon/xkbcommon.h>

namespace QtWaylandClient {

static Qt::Key toQtKey(uint32_t sym)
{
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
   if (QApplication::focusObject() == nullptr) {
      return;
   }

   QInputMethodQueryEvent event(Qt::ImQueryAll);
   QCoreApplication::sendEvent(QApplication::focusObject(), &event);

   const QString &text = event.value(Qt::ImSurroundingText).toString();
   const int cursor = event.value(Qt::ImCursorPosition).toInt();
   const int anchor = event.value(Qt::ImAnchorPosition).toInt();

   set_surrounding_text(text, text.leftView(cursor).size_storage(), text.leftView(anchor).size_storage());

   commit_state(++m_serial);
}

void QWaylandTextInput::text_input_preedit_string(uint32_t serial, const QString &text, const QString &commit)
{
   (void) serial;

   if (QApplication::focusObject() == nullptr) {
      return;
   }

   m_commit = commit;
   QList<QInputMethodEvent::Attribute> attributes;
   QInputMethodEvent event(text, attributes);
   QCoreApplication::sendEvent(QApplication::focusObject(), &event);
}

void QWaylandTextInput::text_input_commit_string(uint32_t serial, const QString &text)
{
   (void) serial;

   if (QApplication::focusObject() == nullptr) {
      return;
   }

   QInputMethodEvent event;
   event.setCommitString(text);
   QCoreApplication::sendEvent(QApplication::focusObject(), &event);

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

   if (QApplication::focusObject() == nullptr) {
      return;
   }

   // TODO: Convert modifiers to Qt::KeyboardModifiers
   QKeyEvent event(toQEventType(state), toQtKey(sym), Qt::NoModifier);
   QCoreApplication::sendEvent(qGuiApp->focusWindow(), &event);
}

QWaylandInputContext::QWaylandInputContext(QWaylandDisplay *display)
   : m_display(display), m_textInput()
{
}

bool QWaylandInputContext::isValid() const
{
   return m_display->textInputManager() != nullptr;
}

void QWaylandInputContext::reset()
{
   if (! ensureTextInput()) {
      return;
   }

   m_textInput->reset();
}

void QWaylandInputContext::commit()
{
   if (! ensureTextInput()) {
      return;
   }

   if (QApplication::focusObject() == nullptr) {
      return;
   }

   QInputMethodEvent event;
   event.setCommitString(m_textInput->commitString());

   QCoreApplication::sendEvent(QApplication::focusObject(), &event);

   m_textInput->reset();
}

void QWaylandInputContext::update(Qt::InputMethodQueries queries)
{
   (void) queries;

   if (! ensureTextInput()) {
      return;
   }

   m_textInput->updateState();
}

void QWaylandInputContext::invokeAction(QInputMethod::Action, int cursorPosition)
{
   if (! ensureTextInput()) {
      return;
   }

   m_textInput->invoke_action(0, cursorPosition);
}

void QWaylandInputContext::showInputPanel()
{
   if (! ensureTextInput()) {
      return;
   }

   m_textInput->show_input_panel();
}

void QWaylandInputContext::hideInputPanel()
{
   if (! ensureTextInput()) {
      return;
   }

   m_textInput->hide_input_panel();
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
      m_textInput->deactivate(m_display->defaultInputDevice()->wl_seat());
      return;
   }

   QWindow *window = QApplication::focusWindow();

   if (window == nullptr || window->handle() == nullptr) {
      return;
   }

   struct ::wl_surface *surface = static_cast<QWaylandWindow *>(window->handle())->object();

   m_textInput->activate(m_display->defaultInputDevice()->wl_seat(), surface);
}

bool QWaylandInputContext::ensureTextInput()
{
   if (m_textInput != nullptr) {
      return true;
   }

   if (! isValid()) {
      return false;
   }

   m_textInput.reset(new QWaylandTextInput(m_display->textInputManager()->create_text_input()));

   return true;
}

}
