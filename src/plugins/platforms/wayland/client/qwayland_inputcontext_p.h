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

#ifndef QWAYLAND_INPUTCONTEXT_H
#define QWAYLAND_INPUTCONTEXT_H

#include <qplatform_inputcontext.h>

#include <qwayland-text.h>

namespace QtWaylandClient {

class QWaylandDisplay;
class QWaylandWindow;

class QWaylandTextInput : public QtWayland::wl_text_input
{
 public:
   QWaylandTextInput(struct ::wl_text_input *text_input);

   QString commitString() const;

   void reset();
   void updateState();

 protected:
   void text_input_preedit_string(uint32_t serial, const QString &text, const QString &commit) override;
   void text_input_commit_string(uint32_t serial, const QString &text) override;
   void text_input_enter(wl_surface *surface) override;
   void text_input_leave() override;
   void text_input_keysym(uint32_t serial, uint32_t time, uint32_t sym, uint32_t state, uint32_t modifiers) override;

 private:
   QString m_commit;

   uint32_t m_serial;
   uint32_t m_resetSerial;
};

class QWaylandInputContext : public QPlatformInputContext
{
   CS_OBJECT(QWaylandInputContext)

 public:
   explicit QWaylandInputContext(QWaylandDisplay *display);

   bool isValid() const override;

   void reset() override;
   void commit() override;
   void update(Qt::InputMethodQueries) override;
   void invokeAction(QInputMethod::Action, int cursorPosition) override;

   void showInputPanel() override;
   void hideInputPanel() override;
   bool isInputPanelVisible() const override;

   void setFocusObject(QObject *object) override;

 private:
   bool ensureTextInput();

   QWaylandDisplay *m_display;
   QScopedPointer<QWaylandTextInput> m_textInput;
};

}

#endif
