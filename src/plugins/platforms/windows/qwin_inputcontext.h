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

#ifndef QWINDOWSINPUTCONTEXT_H
#define QWINDOWSINPUTCONTEXT_H

#include <qlocale.h>
#include <qpointer.h>
#include <qplatform_inputcontext.h>
#include <qwin_additional.h>

class QInputMethodEvent;
class QWindowsWindow;

class QWindowsInputContext : public QPlatformInputContext
{
   CS_OBJECT(QWindowsInputContext)

   struct CompositionContext {
      CompositionContext();

      HWND hwnd;
      bool haveCaret;
      QString composition;
      int position;
      bool isComposing;
      QPointer<QObject> focusObject;
      qreal factor;
   };

 public:
   explicit QWindowsInputContext();
   ~QWindowsInputContext();

   static void setWindowsImeEnabled(QWindowsWindow *platformWindow, bool enabled);

   bool hasCapability(Capability capability) const override;
   QLocale locale() const override {
      return m_locale;
   }

   void reset() override;
   void update(Qt::InputMethodQueries) override;
   void invokeAction(QInputMethod::Action, int cursorPosition) override;
   void setFocusObject(QObject *object) override;

   bool startComposition(HWND hwnd);
   bool composition(HWND hwnd, LPARAM lParam);
   bool endComposition(HWND hwnd);

   inline bool isComposing() const {
      return m_compositionContext.isComposing;
   }

   int reconvertString(RECONVERTSTRING *reconv);

   bool handleIME_Request(WPARAM wparam, LPARAM lparam, LRESULT *result);
   void handleInputLanguageChanged(WPARAM wparam, LPARAM lparam);

 private :
   CS_SLOT_1(Private, void cursorRectChanged())
   CS_SLOT_2(cursorRectChanged)

 private:
   void initContext(HWND hwnd, qreal factor, QObject *focusObject);
   void doneContext();
   void startContextComposition();
   void endContextComposition();
   void updateEnabled();

   const DWORD m_WM_MSIME_MOUSE;
   static HIMC m_defaultContext;
   CompositionContext m_compositionContext;
   bool m_endCompositionRecursionGuard;
   LCID m_languageId;
   QLocale m_locale;
};

#endif
