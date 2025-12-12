/***********************************************************************
*
* Copyright (c) 2012-2025 Barbara Geller
* Copyright (c) 2012-2025 Ansel Sermersheim
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

// ** Copyright (C) 2015 Jolla Ltd

#include <qwayland_xkb.h>

#include <qstring.h>

#include <xkbcommon/xkbcommon-keysyms.h>

static const uint32_t KeyTbl[] = {
   XKB_KEY_Escape,                  Qt::Key_Escape,
   XKB_KEY_Tab,                     Qt::Key_Tab,
   XKB_KEY_ISO_Left_Tab,            Qt::Key_Backtab,
   XKB_KEY_BackSpace,               Qt::Key_Backspace,
   XKB_KEY_Return,                  Qt::Key_Return,
   XKB_KEY_Insert,                  Qt::Key_Insert,
   XKB_KEY_Delete,                  Qt::Key_Delete,
   XKB_KEY_Clear,                   Qt::Key_Delete,
   XKB_KEY_Pause,                   Qt::Key_Pause,
   XKB_KEY_Print,                   Qt::Key_Print,

   XKB_KEY_Home,                    Qt::Key_Home,
   XKB_KEY_End,                     Qt::Key_End,
   XKB_KEY_Left,                    Qt::Key_Left,
   XKB_KEY_Up,                      Qt::Key_Up,
   XKB_KEY_Right,                   Qt::Key_Right,
   XKB_KEY_Down,                    Qt::Key_Down,
   XKB_KEY_Prior,                   Qt::Key_PageUp,
   XKB_KEY_Next,                    Qt::Key_PageDown,

   XKB_KEY_Shift_L,                 Qt::Key_Shift,
   XKB_KEY_Shift_R,                 Qt::Key_Shift,
   XKB_KEY_Shift_Lock,              Qt::Key_Shift,
   XKB_KEY_Control_L,               Qt::Key_Control,
   XKB_KEY_Control_R,               Qt::Key_Control,
   XKB_KEY_Meta_L,                  Qt::Key_Meta,
   XKB_KEY_Meta_R,                  Qt::Key_Meta,
   XKB_KEY_Alt_L,                   Qt::Key_Alt,
   XKB_KEY_Alt_R,                   Qt::Key_Alt,
   XKB_KEY_Caps_Lock,               Qt::Key_CapsLock,
   XKB_KEY_Num_Lock,                Qt::Key_NumLock,
   XKB_KEY_Scroll_Lock,             Qt::Key_ScrollLock,
   XKB_KEY_Super_L,                 Qt::Key_Super_L,
   XKB_KEY_Super_R,                 Qt::Key_Super_R,
   XKB_KEY_Menu,                    Qt::Key_Menu,
   XKB_KEY_Hyper_L,                 Qt::Key_Hyper_L,
   XKB_KEY_Hyper_R,                 Qt::Key_Hyper_R,
   XKB_KEY_Help,                    Qt::Key_Help,

   XKB_KEY_KP_Space,                Qt::Key_Space,
   XKB_KEY_KP_Tab,                  Qt::Key_Tab,
   XKB_KEY_KP_Enter,                Qt::Key_Enter,
   XKB_KEY_KP_Home,                 Qt::Key_Home,
   XKB_KEY_KP_Left,                 Qt::Key_Left,
   XKB_KEY_KP_Up,                   Qt::Key_Up,
   XKB_KEY_KP_Right,                Qt::Key_Right,
   XKB_KEY_KP_Down,                 Qt::Key_Down,
   XKB_KEY_KP_Prior,                Qt::Key_PageUp,
   XKB_KEY_KP_Next,                 Qt::Key_PageDown,
   XKB_KEY_KP_End,                  Qt::Key_End,
   XKB_KEY_KP_Begin,                Qt::Key_Clear,
   XKB_KEY_KP_Insert,               Qt::Key_Insert,
   XKB_KEY_KP_Delete,               Qt::Key_Delete,
   XKB_KEY_KP_Equal,                Qt::Key_Equal,
   XKB_KEY_KP_Multiply,             Qt::Key_Asterisk,
   XKB_KEY_KP_Add,                  Qt::Key_Plus,
   XKB_KEY_KP_Separator,            Qt::Key_Comma,
   XKB_KEY_KP_Subtract,             Qt::Key_Minus,
   XKB_KEY_KP_Decimal,              Qt::Key_Period,
   XKB_KEY_KP_Divide,               Qt::Key_Slash,

   XKB_KEY_ISO_Level3_Shift,        Qt::Key_AltGr,
   XKB_KEY_Multi_key,               Qt::Key_Multi_key,
   XKB_KEY_Codeinput,               Qt::Key_Codeinput,
   XKB_KEY_SingleCandidate,         Qt::Key_SingleCandidate,
   XKB_KEY_MultipleCandidate,       Qt::Key_MultipleCandidate,
   XKB_KEY_PreviousCandidate,       Qt::Key_PreviousCandidate,

   XKB_KEY_Mode_switch,             Qt::Key_Mode_switch,
   XKB_KEY_script_switch,           Qt::Key_Mode_switch,

   XKB_KEY_XF86AudioPlay,           Qt::Key_MediaTogglePlayPause, // there is not a PlayPause keysym, play keys are not common
   XKB_KEY_XF86AudioPause,          Qt::Key_MediaPause,
   XKB_KEY_XF86AudioStop,           Qt::Key_MediaStop,
   XKB_KEY_XF86AudioPrev,           Qt::Key_MediaPrevious,
   XKB_KEY_XF86AudioNext,           Qt::Key_MediaNext,
   XKB_KEY_XF86AudioRewind,         Qt::Key_MediaPrevious,
   XKB_KEY_XF86AudioForward,        Qt::Key_MediaNext,
   XKB_KEY_XF86AudioRecord,         Qt::Key_MediaRecord,

   XKB_KEY_XF86AudioMute,           Qt::Key_VolumeMute,
   XKB_KEY_XF86AudioLowerVolume,    Qt::Key_VolumeDown,
   XKB_KEY_XF86AudioRaiseVolume,    Qt::Key_VolumeUp,

   XKB_KEY_XF86AudioRandomPlay,     Qt::Key_AudioRandomPlay,
   XKB_KEY_XF86AudioRepeat,         Qt::Key_AudioRepeat,

   XKB_KEY_XF86ZoomIn,              Qt::Key_ZoomIn,
   XKB_KEY_XF86ZoomOut,             Qt::Key_ZoomOut,

   XKB_KEY_XF86Eject,               Qt::Key_Eject,

   XKB_KEY_XF86Phone,               Qt::Key_ToggleCallHangup,

   0,                               0
};

static int lookupKeysym(xkb_keysym_t key)
{
   int code = 0;
   int i    = 0;

   while (KeyTbl[i]) {
      if (key == KeyTbl[i]) {
         code = (int)KeyTbl[i + 1];
         break;
      }

      i += 2;
   }

   return code;
}

int QWaylandXkb::keysymToQtKey(xkb_keysym_t keysym, Qt::KeyboardModifiers &modifiers, const QString &text)
{
   int code = 0;

   if (keysym >= XKB_KEY_F1 && keysym <= XKB_KEY_F35) {
      code =  Qt::Key_F1 + (int(keysym) - XKB_KEY_F1);

   } else if (keysym >= XKB_KEY_KP_Space && keysym <= XKB_KEY_KP_9) {
      if (keysym >= XKB_KEY_KP_0) {
         // numeric keypad keys
         code = Qt::Key_0 + ((int)keysym - XKB_KEY_KP_0);
      } else {
         code = lookupKeysym(keysym);
      }

      modifiers |= Qt::KeypadModifier;

   } else if (text.length() == 1 && text[0].unicode() > 0x1f
         && text[0].unicode() != 0x7f && ! (keysym >= XKB_KEY_dead_grave && keysym <= XKB_KEY_dead_currency)) {

      code = text[0].toUpper()[0].unicode();

   } else {
      // any other keys
      code = lookupKeysym(keysym);
   }

   return code;
}

Qt::KeyboardModifiers QWaylandXkb::modifiers(struct xkb_state *state)
{
   Qt::KeyboardModifiers modifiers = Qt::NoModifier;

   xkb_state_component cstate = static_cast<xkb_state_component>(XKB_STATE_DEPRESSED | XKB_STATE_LATCHED | XKB_STATE_LOCKED);

   if (xkb_state_mod_name_is_active(state, XKB_MOD_NAME_SHIFT, cstate)) {
      modifiers |= Qt::ShiftModifier;
   }

   if (xkb_state_mod_name_is_active(state, XKB_MOD_NAME_CTRL, cstate)) {
      modifiers |= Qt::ControlModifier;
   }

   if (xkb_state_mod_name_is_active(state, XKB_MOD_NAME_ALT, cstate)) {
      modifiers |= Qt::AltModifier;
   }

   if (xkb_state_mod_name_is_active(state, XKB_MOD_NAME_LOGO, cstate)) {
      modifiers |= Qt::MetaModifier;
   }

   return modifiers;
}
