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

#ifndef QXCBKEYBOARD_H
#define QXCBKEYBOARD_H

#include <qevent.h>
#include <qxcb_object.h>

#include <xcb/xcb_keysyms.h>
#include <xkbcommon/xkbcommon.h>

#ifndef QT_NO_XKB
#include <xkbcommon/xkbcommon-x11.h>
#endif

class QWindow;

class QXcbKeyboard : public QXcbObject
{
 public:
   QXcbKeyboard(QXcbConnection *connection);

   ~QXcbKeyboard();

   void handleKeyPressEvent(const xcb_key_press_event_t *event);
   void handleKeyReleaseEvent(const xcb_key_release_event_t *event);
   void handleMappingNotifyEvent(const void *event);

   Qt::KeyboardModifiers translateModifiers(int s) const;
   void updateKeymap();
   QList<int> possibleKeys(const QKeyEvent *e) const;

   // when XKEYBOARD not present on the X server
   void updateXKBMods();
   quint32 xkbModMask(quint16 state);
   void updateXKBStateFromCore(quint16 state);
#ifdef XCB_USE_XINPUT22
   void updateXKBStateFromXI(void *modInfo, void *groupInfo);
#endif
#ifndef QT_NO_XKB
   // when XKEYBOARD is present on the X server
   int coreDeviceId() const {
      return core_device_id;
   }
   void updateXKBState(xcb_xkb_state_notify_event_t *state);
#endif

 protected:
   void handleKeyEvent(xcb_window_t sourceWindow, QEvent::Type type, xcb_keycode_t code, quint16 state, xcb_timestamp_t time);

   void resolveMaskConflicts();
   QString lookupString(struct xkb_state *state, xcb_keycode_t code) const;
   int keysymToQtKey(xcb_keysym_t keysym) const;
   int keysymToQtKey(xcb_keysym_t keysym, Qt::KeyboardModifiers &modifiers, const QString &text) const;
   void printKeymapError(const char *error) const;

   void readXKBConfig();
   void clearXKBConfig();
   // when XKEYBOARD not present on the X server
   void updateModifiers();
   // when XKEYBOARD is present on the X server
   void updateVModMapping();
   void updateVModToRModMapping();

   xkb_keysym_t lookupLatinKeysym(xkb_keycode_t keycode) const;
   void checkForLatinLayout();

 private:
   void updateXKBStateFromState(struct xkb_state *kb_state, quint16 state);

   bool m_config;
   xcb_keycode_t m_autorepeat_code;

   struct xkb_context *xkb_context;
   struct xkb_keymap *xkb_keymap;
   struct xkb_state *xkb_state;
   struct xkb_rule_names xkb_names;
   mutable struct xkb_keymap *latin_keymap;

   struct _mod_masks {
      uint alt;
      uint altgr;
      uint meta;
      uint super;
      uint hyper;
   };

   _mod_masks rmod_masks;

   // when XKEYBOARD not present on the X server
   xcb_key_symbols_t *m_key_symbols;
   struct _xkb_mods {
      xkb_mod_index_t shift;
      xkb_mod_index_t lock;
      xkb_mod_index_t control;
      xkb_mod_index_t mod1;
      xkb_mod_index_t mod2;
      xkb_mod_index_t mod3;
      xkb_mod_index_t mod4;
      xkb_mod_index_t mod5;
   };
   _xkb_mods xkb_mods;
#ifndef QT_NO_XKB
   // when XKEYBOARD is present on the X server
   _mod_masks vmod_masks;
   int core_device_id;
#endif
   bool m_hasLatinLayout;
};

#endif
