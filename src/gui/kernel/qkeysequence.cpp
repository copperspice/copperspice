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

#include <qkeysequence.h>

#include <qplatform_theme.h>

#include <qapplication_p.h>
#include <qkeysequence_p.h>

#ifndef QT_NO_SHORTCUT

#include <qdatastream.h>
#include <qdatastream.h>
#include <qdebug.h>
#include <qhashfunc.h>
#include <qregularexpression.h>
#include <qshortcut.h>
#include <qvariant.h>

#include <algorithm>

#ifdef Q_OS_DARWIN
#include <qcore_mac_p.h>
#include <Carbon/Carbon.h>

static bool qt_sequence_no_mnemonics = true;

struct MacSpecialKey {
   int key;
   ushort macSymbol;
};

static constexpr const int NumEntries = 21;

static const MacSpecialKey entries[NumEntries] = {
   { Qt::Key_Escape,    0x238B },
   { Qt::Key_Tab,       0x21E5 },
   { Qt::Key_Backtab,   0x21E4 },
   { Qt::Key_Backspace, 0x232B },
   { Qt::Key_Return,    0x21B5 },
   { Qt::Key_Enter,     0x2324 },
   { Qt::Key_Delete,    0x2326 },
   { Qt::Key_Home,      0x2196 },
   { Qt::Key_End,       0x2198 },
   { Qt::Key_Left,      0x2190 },
   { Qt::Key_Up,        0x2191 },
   { Qt::Key_Right,     0x2192 },
   { Qt::Key_Down,      0x2193 },
   { Qt::Key_PageUp,    0x21DE },
   { Qt::Key_PageDown,  0x21DF },
   { Qt::Key_Shift,     kShiftUnicode   },
   { Qt::Key_Control,   kCommandUnicode },
   { Qt::Key_Meta,      kControlUnicode },
   { Qt::Key_Alt,       kOptionUnicode  },
   { Qt::Key_CapsLock,  0x21EA },
};

static bool operator<(const MacSpecialKey &entry, int key)
{
   return entry.key < key;
}

static bool operator<(int key, const MacSpecialKey &entry)
{
   return key < entry.key;
}

static const MacSpecialKey *const MacSpecialKeyEntriesEnd = entries + NumEntries;

QChar qt_macSymbolForQtKey(int key)
{
   const MacSpecialKey *i = std::lower_bound(entries, MacSpecialKeyEntriesEnd, key);

   if ((i == MacSpecialKeyEntriesEnd) || (key < *i) ) {
      return QChar();
   }

   ushort macSymbol = i->macSymbol;

   if (qApp->testAttribute(Qt::AA_MacDontSwapCtrlAndMeta)
         && (macSymbol == kControlUnicode || macSymbol == kCommandUnicode)) {

      if (macSymbol == kControlUnicode) {
         macSymbol = kCommandUnicode;
      } else {
         macSymbol = kControlUnicode;
      }
   }

   return QChar(macSymbol);
}

static int qtkeyForMacSymbol(const QChar ch)
{
   const ushort unicode = ch.unicode();

   for (int i = 0; i < NumEntries; ++i) {
      const MacSpecialKey &entry = entries[i];
      if (entry.macSymbol == unicode) {
         int key = entry.key;

         if (qApp->testAttribute(Qt::AA_MacDontSwapCtrlAndMeta)
             && (unicode == kControlUnicode || unicode == kCommandUnicode)) {

            if (unicode == kControlUnicode) {
               key = Qt::Key_Control;
            } else {
               key = Qt::Key_Meta;
            }
         }
         return key;
      }
   }   return -1;
}

#else
static bool qt_sequence_no_mnemonics = false;
#endif

void Q_GUI_EXPORT qt_set_sequence_auto_mnemonic(bool b)
{
   qt_sequence_no_mnemonics = !b;
}

static const struct {
   int key;
   const char *name;
} keyname[] = {

   //: This and all following "incomprehensible" strings in QShortcut context
   //: are key names. Please use the localized names appearing on actual
   //: keyboards or whatever is commonly used.

   { Qt::Key_Space,        cs_mark_tr("QShortcut", "Space") },
   { Qt::Key_Escape,       cs_mark_tr("QShortcut", "Esc") },
   { Qt::Key_Tab,          cs_mark_tr("QShortcut", "Tab") },
   { Qt::Key_Backtab,      cs_mark_tr("QShortcut", "Backtab") },
   { Qt::Key_Backspace,    cs_mark_tr("QShortcut", "Backspace") },
   { Qt::Key_Return,       cs_mark_tr("QShortcut", "Return") },
   { Qt::Key_Enter,        cs_mark_tr("QShortcut", "Enter") },
   { Qt::Key_Insert,       cs_mark_tr("QShortcut", "Ins") },
   { Qt::Key_Delete,       cs_mark_tr("QShortcut", "Del") },
   { Qt::Key_Pause,        cs_mark_tr("QShortcut", "Pause") },
   { Qt::Key_Print,        cs_mark_tr("QShortcut", "Print") },
   { Qt::Key_SysReq,       cs_mark_tr("QShortcut", "SysReq") },
   { Qt::Key_Home,         cs_mark_tr("QShortcut", "Home") },
   { Qt::Key_End,          cs_mark_tr("QShortcut", "End") },
   { Qt::Key_Left,         cs_mark_tr("QShortcut", "Left") },
   { Qt::Key_Up,           cs_mark_tr("QShortcut", "Up") },
   { Qt::Key_Right,        cs_mark_tr("QShortcut", "Right") },
   { Qt::Key_Down,         cs_mark_tr("QShortcut", "Down") },
   { Qt::Key_PageUp,       cs_mark_tr("QShortcut", "PgUp") },
   { Qt::Key_PageDown,     cs_mark_tr("QShortcut", "PgDown") },
   { Qt::Key_CapsLock,     cs_mark_tr("QShortcut", "CapsLock") },
   { Qt::Key_NumLock,      cs_mark_tr("QShortcut", "NumLock") },
   { Qt::Key_ScrollLock,   cs_mark_tr("QShortcut", "ScrollLock") },
   { Qt::Key_Menu,         cs_mark_tr("QShortcut", "Menu") },
   { Qt::Key_Help,         cs_mark_tr("QShortcut", "Help") },

   // Special keys
   // Includes multimedia, launcher, lan keys ( bluetooth, wireless )
   // window navigation
   { Qt::Key_Back,                       cs_mark_tr("QShortcut", "Back") },
   { Qt::Key_Forward,                    cs_mark_tr("QShortcut", "Forward") },
   { Qt::Key_Stop,                       cs_mark_tr("QShortcut", "Stop") },
   { Qt::Key_Refresh,                    cs_mark_tr("QShortcut", "Refresh") },
   { Qt::Key_VolumeDown,                 cs_mark_tr("QShortcut", "Volume Down") },
   { Qt::Key_VolumeMute,                 cs_mark_tr("QShortcut", "Volume Mute") },
   { Qt::Key_VolumeUp,                   cs_mark_tr("QShortcut", "Volume Up") },
   { Qt::Key_BassBoost,                  cs_mark_tr("QShortcut", "Bass Boost") },
   { Qt::Key_BassUp,                     cs_mark_tr("QShortcut", "Bass Up") },
   { Qt::Key_BassDown,                   cs_mark_tr("QShortcut", "Bass Down") },
   { Qt::Key_TrebleUp,                   cs_mark_tr("QShortcut", "Treble Up") },
   { Qt::Key_TrebleDown,                 cs_mark_tr("QShortcut", "Treble Down") },
   { Qt::Key_MediaPlay,                  cs_mark_tr("QShortcut", "Media Play") },
   { Qt::Key_MediaStop,                  cs_mark_tr("QShortcut", "Media Stop") },
   { Qt::Key_MediaPrevious,              cs_mark_tr("QShortcut", "Media Previous") },
   { Qt::Key_MediaNext,                  cs_mark_tr("QShortcut", "Media Next") },
   { Qt::Key_MediaRecord,                cs_mark_tr("QShortcut", "Media Record") },
   //: Media player pause button
   { Qt::Key_MediaPause,                 cs_mark_tr("QShortcut", "Media Pause") },
   //: Media player button to toggle between playing and paused
   { Qt::Key_MediaTogglePlayPause,       cs_mark_tr("QShortcut", "Toggle Media Play/Pause") },
   { Qt::Key_HomePage,                   cs_mark_tr("QShortcut", "Home Page") },
   { Qt::Key_Favorites,                  cs_mark_tr("QShortcut", "Favorites") },
   { Qt::Key_Search,                     cs_mark_tr("QShortcut", "Search") },
   { Qt::Key_Standby,                    cs_mark_tr("QShortcut", "Standby") },
   { Qt::Key_OpenUrl,                    cs_mark_tr("QShortcut", "Open URL") },
   { Qt::Key_LaunchMail,                 cs_mark_tr("QShortcut", "Launch Mail") },
   { Qt::Key_LaunchMedia,                cs_mark_tr("QShortcut", "Launch Media") },
   { Qt::Key_Launch0,                    cs_mark_tr("QShortcut", "Launch (0)") },
   { Qt::Key_Launch1,                    cs_mark_tr("QShortcut", "Launch (1)") },
   { Qt::Key_Launch2,                    cs_mark_tr("QShortcut", "Launch (2)") },
   { Qt::Key_Launch3,                    cs_mark_tr("QShortcut", "Launch (3)") },
   { Qt::Key_Launch4,                    cs_mark_tr("QShortcut", "Launch (4)") },
   { Qt::Key_Launch5,                    cs_mark_tr("QShortcut", "Launch (5)") },
   { Qt::Key_Launch6,                    cs_mark_tr("QShortcut", "Launch (6)") },
   { Qt::Key_Launch7,                    cs_mark_tr("QShortcut", "Launch (7)") },
   { Qt::Key_Launch8,                    cs_mark_tr("QShortcut", "Launch (8)") },
   { Qt::Key_Launch9,                    cs_mark_tr("QShortcut", "Launch (9)") },
   { Qt::Key_LaunchA,                    cs_mark_tr("QShortcut", "Launch (A)") },
   { Qt::Key_LaunchB,                    cs_mark_tr("QShortcut", "Launch (B)") },
   { Qt::Key_LaunchC,                    cs_mark_tr("QShortcut", "Launch (C)") },
   { Qt::Key_LaunchD,                    cs_mark_tr("QShortcut", "Launch (D)") },
   { Qt::Key_LaunchE,                    cs_mark_tr("QShortcut", "Launch (E)") },
   { Qt::Key_LaunchF,                    cs_mark_tr("QShortcut", "Launch (F)") },
   { Qt::Key_MonBrightnessUp,            cs_mark_tr("QShortcut", "Monitor Brightness Up") },
   { Qt::Key_MonBrightnessDown,          cs_mark_tr("QShortcut", "Monitor Brightness Down") },
   { Qt::Key_KeyboardLightOnOff,         cs_mark_tr("QShortcut", "Keyboard Light On/Off") },
   { Qt::Key_KeyboardBrightnessUp,       cs_mark_tr("QShortcut", "Keyboard Brightness Up") },
   { Qt::Key_KeyboardBrightnessDown,     cs_mark_tr("QShortcut", "Keyboard Brightness Down") },
   { Qt::Key_PowerOff,                   cs_mark_tr("QShortcut", "Power Off") },
   { Qt::Key_WakeUp,                     cs_mark_tr("QShortcut", "Wake Up") },
   { Qt::Key_Eject,                      cs_mark_tr("QShortcut", "Eject") },
   { Qt::Key_ScreenSaver,                cs_mark_tr("QShortcut", "Screensaver") },
   { Qt::Key_WWW,                        cs_mark_tr("QShortcut", "WWW") },
   { Qt::Key_Sleep,                      cs_mark_tr("QShortcut", "Sleep") },
   { Qt::Key_LightBulb,                  cs_mark_tr("QShortcut", "LightBulb") },
   { Qt::Key_Shop,                       cs_mark_tr("QShortcut", "Shop") },
   { Qt::Key_History,                    cs_mark_tr("QShortcut", "History") },
   { Qt::Key_AddFavorite,                cs_mark_tr("QShortcut", "Add Favorite") },
   { Qt::Key_HotLinks,                   cs_mark_tr("QShortcut", "Hot Links") },
   { Qt::Key_BrightnessAdjust,           cs_mark_tr("QShortcut", "Adjust Brightness") },
   { Qt::Key_Finance,                    cs_mark_tr("QShortcut", "Finance") },
   { Qt::Key_Community,                  cs_mark_tr("QShortcut", "Community") },
   { Qt::Key_AudioRewind,                cs_mark_tr("QShortcut", "Media Rewind") },
   { Qt::Key_BackForward,                cs_mark_tr("QShortcut", "Back Forward") },
   { Qt::Key_ApplicationLeft,            cs_mark_tr("QShortcut", "Application Left") },
   { Qt::Key_ApplicationRight,           cs_mark_tr("QShortcut", "Application Right") },
   { Qt::Key_Book,                       cs_mark_tr("QShortcut", "Book") },
   { Qt::Key_CD,                         cs_mark_tr("QShortcut", "CD") },
   { Qt::Key_Calculator,                 cs_mark_tr("QShortcut", "Calculator") },
   { Qt::Key_Clear,                      cs_mark_tr("QShortcut", "Clear") },
   { Qt::Key_ClearGrab,                  cs_mark_tr("QShortcut", "Clear Grab") },
   { Qt::Key_Close,                      cs_mark_tr("QShortcut", "Close") },
   { Qt::Key_Copy,                       cs_mark_tr("QShortcut", "Copy") },
   { Qt::Key_Cut,                        cs_mark_tr("QShortcut", "Cut") },
   { Qt::Key_Display,                    cs_mark_tr("QShortcut", "Display") },
   { Qt::Key_DOS,                        cs_mark_tr("QShortcut", "DOS") },
   { Qt::Key_Documents,                  cs_mark_tr("QShortcut", "Documents") },
   { Qt::Key_Excel,                      cs_mark_tr("QShortcut", "Spreadsheet") },
   { Qt::Key_Explorer,                   cs_mark_tr("QShortcut", "Browser") },
   { Qt::Key_Game,                       cs_mark_tr("QShortcut", "Game") },
   { Qt::Key_Go,                         cs_mark_tr("QShortcut", "Go") },
   { Qt::Key_iTouch,                     cs_mark_tr("QShortcut", "iTouch") },
   { Qt::Key_LogOff,                     cs_mark_tr("QShortcut", "Logoff") },
   { Qt::Key_Market,                     cs_mark_tr("QShortcut", "Market") },
   { Qt::Key_Meeting,                    cs_mark_tr("QShortcut", "Meeting") },
   { Qt::Key_MenuKB,                     cs_mark_tr("QShortcut", "Keyboard Menu") },
   { Qt::Key_MenuPB,                     cs_mark_tr("QShortcut", "Menu PB") },
   { Qt::Key_MySites,                    cs_mark_tr("QShortcut", "My Sites") },
   { Qt::Key_News,                       cs_mark_tr("QShortcut", "News") },
   { Qt::Key_OfficeHome,                 cs_mark_tr("QShortcut", "Home Office") },
   { Qt::Key_Option,                     cs_mark_tr("QShortcut", "Option") },
   { Qt::Key_Paste,                      cs_mark_tr("QShortcut", "Paste") },
   { Qt::Key_Phone,                      cs_mark_tr("QShortcut", "Phone") },
   { Qt::Key_Reply,                      cs_mark_tr("QShortcut", "Reply") },
   { Qt::Key_Reload,                     cs_mark_tr("QShortcut", "Reload") },
   { Qt::Key_RotateWindows,              cs_mark_tr("QShortcut", "Rotate Windows") },
   { Qt::Key_RotationPB,                 cs_mark_tr("QShortcut", "Rotation PB") },
   { Qt::Key_RotationKB,                 cs_mark_tr("QShortcut", "Rotation KB") },
   { Qt::Key_Save,                       cs_mark_tr("QShortcut", "Save") },
   { Qt::Key_Send,                       cs_mark_tr("QShortcut", "Send") },
   { Qt::Key_Spell,                      cs_mark_tr("QShortcut", "Spellchecker") },
   { Qt::Key_SplitScreen,                cs_mark_tr("QShortcut", "Split Screen") },
   { Qt::Key_Support,                    cs_mark_tr("QShortcut", "Support") },
   { Qt::Key_TaskPane,                   cs_mark_tr("QShortcut", "Task Panel") },
   { Qt::Key_Terminal,                   cs_mark_tr("QShortcut", "Terminal") },
   { Qt::Key_Tools,                      cs_mark_tr("QShortcut", "Tools") },
   { Qt::Key_Travel,                     cs_mark_tr("QShortcut", "Travel") },
   { Qt::Key_Video,                      cs_mark_tr("QShortcut", "Video") },
   { Qt::Key_Word,                       cs_mark_tr("QShortcut", "Word Processor") },
   { Qt::Key_Xfer,                       cs_mark_tr("QShortcut", "XFer") },
   { Qt::Key_ZoomIn,                     cs_mark_tr("QShortcut", "Zoom In") },
   { Qt::Key_ZoomOut,                    cs_mark_tr("QShortcut", "Zoom Out") },
   { Qt::Key_Away,                       cs_mark_tr("QShortcut", "Away") },
   { Qt::Key_Messenger,                  cs_mark_tr("QShortcut", "Messenger") },
   { Qt::Key_WebCam,                     cs_mark_tr("QShortcut", "WebCam") },
   { Qt::Key_MailForward,                cs_mark_tr("QShortcut", "Mail Forward") },
   { Qt::Key_Pictures,                   cs_mark_tr("QShortcut", "Pictures") },
   { Qt::Key_Music,                      cs_mark_tr("QShortcut", "Music") },
   { Qt::Key_Battery,                    cs_mark_tr("QShortcut", "Battery") },
   { Qt::Key_Bluetooth,                  cs_mark_tr("QShortcut", "Bluetooth") },
   { Qt::Key_WLAN,                       cs_mark_tr("QShortcut", "Wireless") },
   { Qt::Key_UWB,                        cs_mark_tr("QShortcut", "Ultra Wide Band") },
   { Qt::Key_AudioForward,               cs_mark_tr("QShortcut", "Media Fast Forward") },
   { Qt::Key_AudioRepeat,                cs_mark_tr("QShortcut", "Audio Repeat") },
   { Qt::Key_AudioRandomPlay,            cs_mark_tr("QShortcut", "Audio Random Play") },
   { Qt::Key_Subtitle,                   cs_mark_tr("QShortcut", "Subtitle") },
   { Qt::Key_AudioCycleTrack,            cs_mark_tr("QShortcut", "Audio Cycle Track") },
   { Qt::Key_Time,                       cs_mark_tr("QShortcut", "Time") },
   { Qt::Key_Hibernate,                  cs_mark_tr("QShortcut", "Hibernate") },
   { Qt::Key_View,                       cs_mark_tr("QShortcut", "View") },
   { Qt::Key_TopMenu,                    cs_mark_tr("QShortcut", "Top Menu") },
   { Qt::Key_PowerDown,                  cs_mark_tr("QShortcut", "Power Down") },
   { Qt::Key_Suspend,                    cs_mark_tr("QShortcut", "Suspend") },
   { Qt::Key_MicMute,                    cs_mark_tr("QShortcut", "Microphone Mute") },

   { Qt::Key_Red,                        cs_mark_tr("QShortcut", "Red") },
   { Qt::Key_Green,                      cs_mark_tr("QShortcut", "Green") },
   { Qt::Key_Yellow,                     cs_mark_tr("QShortcut", "Yellow") },
   { Qt::Key_Blue,                       cs_mark_tr("QShortcut", "Blue") },

   { Qt::Key_ChannelUp,                  cs_mark_tr("QShortcut", "Channel Up") },
   { Qt::Key_ChannelDown,                cs_mark_tr("QShortcut", "Channel Down") },
   { Qt::Key_Guide,                      cs_mark_tr("QShortcut", "Guide") },
   { Qt::Key_Info,                       cs_mark_tr("QShortcut", "Info") },
   { Qt::Key_Settings,                   cs_mark_tr("QShortcut", "Settings") },
   { Qt::Key_MicVolumeUp,                cs_mark_tr("QShortcut", "Microphone Volume Up") },
   { Qt::Key_MicVolumeDown,              cs_mark_tr("QShortcut", "Microphone Volume Down") },
   { Qt::Key_New,                        cs_mark_tr("QShortcut", "New") },
   { Qt::Key_Open,                       cs_mark_tr("QShortcut", "Open") },
   { Qt::Key_Find,                       cs_mark_tr("QShortcut", "Find") },
   { Qt::Key_Undo,                       cs_mark_tr("QShortcut", "Undo") },
   { Qt::Key_Redo,                       cs_mark_tr("QShortcut", "Redo") },
   // --------------------------------------------------------------
   // More consistent namings
   { Qt::Key_Print,             cs_mark_tr("QShortcut", "Print Screen") },
   { Qt::Key_PageUp,            cs_mark_tr("QShortcut", "Page Up") },
   { Qt::Key_PageDown,          cs_mark_tr("QShortcut", "Page Down") },
   { Qt::Key_CapsLock,          cs_mark_tr("QShortcut", "Caps Lock") },
   { Qt::Key_NumLock,           cs_mark_tr("QShortcut", "Num Lock") },
   { Qt::Key_NumLock,           cs_mark_tr("QShortcut", "Number Lock") },
   { Qt::Key_ScrollLock,        cs_mark_tr("QShortcut", "Scroll Lock") },
   { Qt::Key_Insert,            cs_mark_tr("QShortcut", "Insert") },
   { Qt::Key_Delete,            cs_mark_tr("QShortcut", "Delete") },
   { Qt::Key_Escape,            cs_mark_tr("QShortcut", "Escape") },
   { Qt::Key_SysReq,            cs_mark_tr("QShortcut", "System Request") },

   // --------------------------------------------------------------
   // Keypad navigation keys
   { Qt::Key_Select,            cs_mark_tr("QShortcut", "Select") },
   { Qt::Key_Yes,               cs_mark_tr("QShortcut", "Yes") },
   { Qt::Key_No,                cs_mark_tr("QShortcut", "No") },

   // --------------------------------------------------------------
   // Device keys
   { Qt::Key_Context1,          cs_mark_tr("QShortcut", "Context1") },
   { Qt::Key_Context2,          cs_mark_tr("QShortcut", "Context2") },
   { Qt::Key_Context3,          cs_mark_tr("QShortcut", "Context3") },
   { Qt::Key_Context4,          cs_mark_tr("QShortcut", "Context4") },

   //: Button to start a call (note: a separate button is used to end the call)
   { Qt::Key_Call,              cs_mark_tr("QShortcut", "Call") },

   //: Button to end a call (note: a separate button is used to start the call)
   { Qt::Key_Hangup,            cs_mark_tr("QShortcut", "Hangup") },

   //: Button that will hang up if we're in call, or make a call if we're not.
   { Qt::Key_ToggleCallHangup,  cs_mark_tr("QShortcut", "Toggle Call/Hangup") },
   { Qt::Key_Flip,              cs_mark_tr("QShortcut", "Flip") },

   //: Button to trigger voice dialing
   { Qt::Key_VoiceDial,         cs_mark_tr("QShortcut", "Voice Dial") },

   //: Button to redial the last number called
   { Qt::Key_LastNumberRedial,  cs_mark_tr("QShortcut", "Last Number Redial") },

   //: Button to trigger the camera shutter (take a picture)
   { Qt::Key_Camera,            cs_mark_tr("QShortcut", "Camera Shutter") },

   //: Button to focus the camera
   { Qt::Key_CameraFocus,       cs_mark_tr("QShortcut", "Camera Focus") },

   // --------------------------------------------------------------
   // Japanese keyboard support
   { Qt::Key_Kanji,             cs_mark_tr("QShortcut", "Kanji") },
   { Qt::Key_Muhenkan,          cs_mark_tr("QShortcut", "Muhenkan") },
   { Qt::Key_Henkan,            cs_mark_tr("QShortcut", "Henkan") },
   { Qt::Key_Romaji,            cs_mark_tr("QShortcut", "Romaji") },
   { Qt::Key_Hiragana,          cs_mark_tr("QShortcut", "Hiragana") },
   { Qt::Key_Katakana,          cs_mark_tr("QShortcut", "Katakana") },
   { Qt::Key_Hiragana_Katakana, cs_mark_tr("QShortcut", "Hiragana Katakana") },
   { Qt::Key_Zenkaku,           cs_mark_tr("QShortcut", "Zenkaku") },
   { Qt::Key_Hankaku,           cs_mark_tr("QShortcut", "Hankaku") },
   { Qt::Key_Zenkaku_Hankaku,   cs_mark_tr("QShortcut", "Zenkaku Hankaku") },
   { Qt::Key_Touroku,           cs_mark_tr("QShortcut", "Touroku") },
   { Qt::Key_Massyo,            cs_mark_tr("QShortcut", "Massyo") },
   { Qt::Key_Kana_Lock,         cs_mark_tr("QShortcut", "Kana Lock") },
   { Qt::Key_Kana_Shift,        cs_mark_tr("QShortcut", "Kana Shift") },
   { Qt::Key_Eisu_Shift,        cs_mark_tr("QShortcut", "Eisu Shift") },
   { Qt::Key_Eisu_toggle,       cs_mark_tr("QShortcut", "Eisu toggle") },
   { Qt::Key_Codeinput,         cs_mark_tr("QShortcut", "Code input") },
   { Qt::Key_MultipleCandidate, cs_mark_tr("QShortcut", "Multiple Candidate") },
   { Qt::Key_PreviousCandidate, cs_mark_tr("QShortcut", "Previous Candidate") },

   // --------------------------------------------------------------
   // Korean keyboard support
   { Qt::Key_Hangul,            cs_mark_tr("QShortcut", "Hangul") },
   { Qt::Key_Hangul_Start,      cs_mark_tr("QShortcut", "Hangul Start") },
   { Qt::Key_Hangul_End,        cs_mark_tr("QShortcut", "Hangul End") },
   { Qt::Key_Hangul_Hanja,      cs_mark_tr("QShortcut", "Hangul Hanja") },
   { Qt::Key_Hangul_Jamo,       cs_mark_tr("QShortcut", "Hangul Jamo") },
   { Qt::Key_Hangul_Romaja,     cs_mark_tr("QShortcut", "Hangul Romaja") },
   { Qt::Key_Hangul_Jeonja,     cs_mark_tr("QShortcut", "Hangul Jeonja") },
   { Qt::Key_Hangul_Banja,      cs_mark_tr("QShortcut", "Hangul Banja") },
   { Qt::Key_Hangul_PreHanja,   cs_mark_tr("QShortcut", "Hangul PreHanja") },
   { Qt::Key_Hangul_PostHanja,  cs_mark_tr("QShortcut", "Hangul PostHanja") },
   { Qt::Key_Hangul_Special,    cs_mark_tr("QShortcut", "Hangul Special") },

   // --------------------------------------------------------------
   // Miscellaenous keys
   { Qt::Key_Cancel,            cs_mark_tr("QShortcut", "Cancel") },
   { Qt::Key_Printer,           cs_mark_tr("QShortcut", "Printer") },
   { Qt::Key_Execute,           cs_mark_tr("QShortcut", "Execute") },
   { Qt::Key_Play,              cs_mark_tr("QShortcut", "Play") },
   { Qt::Key_Zoom,              cs_mark_tr("QShortcut", "Zoom") },
   { Qt::Key_Exit,              cs_mark_tr("QShortcut", "Exit") },
   { Qt::Key_TouchpadToggle,    cs_mark_tr("QShortcut", "Touchpad Toggle") },
   { Qt::Key_TouchpadOn,        cs_mark_tr("QShortcut", "Touchpad On") },
   { Qt::Key_TouchpadOff,       cs_mark_tr("QShortcut", "Touchpad Off") },
};
static constexpr int numKeyNames = sizeof keyname / sizeof * keyname;

QKeySequence::QKeySequence(StandardKey key)
{
   const QList <QKeySequence> bindings = keyBindings(key);
   //pick only the first/primary shortcut from current bindings
   if (bindings.size() > 0) {
      d = bindings.first().d;
      d->ref.ref();
   } else {
      d = new QKeySequencePrivate();
   }
}

QKeySequence::QKeySequence()
{
   static QKeySequencePrivate shared_empty;
   d = &shared_empty;
   d->ref.ref();
}


QKeySequence::QKeySequence(const QString &key, QKeySequence::SequenceFormat format)
{
   d = new QKeySequencePrivate();
   assign(key, format);
}

QKeySequence::QKeySequence(int k1, int k2, int k3, int k4)
{
   d = new QKeySequencePrivate();
   d->key[0] = k1;
   d->key[1] = k2;
   d->key[2] = k3;
   d->key[3] = k4;
}

QKeySequence::QKeySequence(const QKeySequence &keysequence)
   : d(keysequence.d)
{
   d->ref.ref();
}

QList<QKeySequence> QKeySequence::keyBindings(StandardKey key)
{
   return QGuiApplicationPrivate::platformTheme()->keyBindings(key);
}

QKeySequence::~QKeySequence()
{
   if (! d->ref.deref()) {
      delete d;
   }
}

void QKeySequence::setKey(int key, int index)
{
   Q_ASSERT_X(index >= 0 && index < QKeySequencePrivate::MaxKeyCount, "QKeySequence::setKey", "index out of range");

   qAtomicDetach(d);
   d->key[index] = key;
}

int QKeySequence::count() const
{
   if (! d->key[0]) {
      return 0;
   }

   if (!d->key[1]) {
      return 1;
   }

   if (! d->key[2]) {
      return 2;
   }

   if (! d->key[3]) {
      return 3;
   }

   return 4;
}

bool QKeySequence::isEmpty() const
{
   return ! d->key[0];
}

QKeySequence QKeySequence::mnemonic(const QString &text)
{
   QKeySequence ret;

   if (qt_sequence_no_mnemonics) {
      return ret;
   }

   bool found = false;
   int p = 0;

   while (p >= 0) {
      p = text.indexOf('&', p) + 1;

      if (p <= 0 || p >= (int)text.length()) {
         break;
      }

      if (text.at(p) != '&') {
         QChar c = text.at(p);

         if (c.isPrint()) {

            if (! found) {
               c   = c.toUpper()[0];
               ret = QKeySequence(c.unicode() + Qt::AltModifier);

               found = true;

            } else {
               qWarning("QKeySequence::mnemonic() \"%s\" contains multiple occurrences of '&'", csPrintable(text));

            }
         }
      }

      p++;
   }

   return ret;
}

int QKeySequence::assign(const QString &ks)
{
   return assign(ks, NativeText);
}

int QKeySequence::assign(const QString &ks, QKeySequence::SequenceFormat format)
{
   QString keyseq = ks;
   QString part;
   int n = 0;
   int p = 0;
   int diff = 0;

   // Run through the whole string, but stop if we have 4 keys before the end.

   while (keyseq.length() && n < QKeySequencePrivate::MaxKeyCount) {
      // We MUST use something to separate each sequence, and space does not work since some of the key names
      // have space in them
      p = keyseq.indexOf(',');

      if (-1 != p) {
         if (p == keyseq.count() - 1) {       // Last comma 'Ctrl+,'
            p = -1;

         } else {
            if (',' == keyseq.at(p + 1)) {    // e.g. 'Ctrl+,, Shift+,,'
               p++;
            }

            if (' ' == keyseq.at(p + 1)) {    // Space after comma
               diff = 1;
               p++;
            } else {
               diff = 0;
            }
         }
      }

      part = keyseq.left(-1 == p ? keyseq.length() : p - diff);
      keyseq = keyseq.right(-1 == p ? 0 : keyseq.length() - (p + 1));
      d->key[n] = QKeySequencePrivate::decodeString(part, format);
      ++n;
   }

   return n;
}

struct QModifKeyName {
   QModifKeyName() { }
   QModifKeyName(int q, QChar n) : qt_key(q), name(n) { }
   QModifKeyName(int q, const QString &n) : qt_key(q), name(n) { }
   int qt_key;
   QString name;
};

static QVector<QModifKeyName> *globalModifs()
{
   static QVector<QModifKeyName> retval;
   return &retval;
}

static QVector<QModifKeyName> *globalPortableModifs()
{
   static QVector<QModifKeyName> retval;
   return &retval;
}

int QKeySequence::decodeString(const QString &str)
{
   return QKeySequencePrivate::decodeString(str, NativeText);
}

int QKeySequencePrivate::decodeString(const QString &str, QKeySequence::SequenceFormat format)
{
   int ret = 0;

   QString accel   = str.toLower();
   bool nativeText = (format == QKeySequence::NativeText);

   QVector<QModifKeyName> *gmodifs;

   if (nativeText) {
      gmodifs = globalModifs();

      if (gmodifs->isEmpty()) {

#ifdef Q_OS_DARWIN
         const bool dontSwap = qApp->testAttribute(Qt::AA_MacDontSwapCtrlAndMeta);

         if (dontSwap) {
            *gmodifs << QModifKeyName(Qt::MetaModifier, kCommandUnicode);
         } else {
            *gmodifs << QModifKeyName(Qt::ControlModifier, kCommandUnicode);
         }

         *gmodifs << QModifKeyName(Qt::AltModifier, kOptionUnicode);

         if (dontSwap) {
            *gmodifs << QModifKeyName(Qt::ControlModifier, kControlUnicode);
         } else {
            *gmodifs << QModifKeyName(Qt::MetaModifier, kControlUnicode);
         }

         *gmodifs << QModifKeyName(Qt::ShiftModifier, kShiftUnicode);
#endif

         *gmodifs << QModifKeyName(Qt::ControlModifier, "ctrl+")
                  << QModifKeyName(Qt::ShiftModifier,   "shift+")
                  << QModifKeyName(Qt::AltModifier,     "alt+")
                  << QModifKeyName(Qt::MetaModifier,    "meta+")
                  << QModifKeyName(Qt::KeypadModifier,  "num+");
      }

   } else {
      gmodifs = globalPortableModifs();

      if (gmodifs->isEmpty()) {
         *gmodifs << QModifKeyName(Qt::ControlModifier,  "ctrl+")
                  << QModifKeyName(Qt::ShiftModifier,    "shift+")
                  << QModifKeyName(Qt::AltModifier,      "alt+")
                  << QModifKeyName(Qt::MetaModifier,     "meta+")
                  << QModifKeyName(Qt::KeypadModifier,   "num+");
      }
   }

   if (! gmodifs) {
      return ret;
   }

   QVector<QModifKeyName> modifs;
   if (nativeText) {
      modifs << QModifKeyName(Qt::ControlModifier, QCoreApplication::translate("QShortcut", "Ctrl").toLower().append(QChar('+')))
             << QModifKeyName(Qt::ShiftModifier,   QCoreApplication::translate("QShortcut", "Shift").toLower().append(QChar('+')))
             << QModifKeyName(Qt::AltModifier,     QCoreApplication::translate("QShortcut", "Alt").toLower().append(QChar('+')))
             << QModifKeyName(Qt::MetaModifier,    QCoreApplication::translate("QShortcut", "Meta").toLower().append(QChar('+')))
             << QModifKeyName(Qt::KeypadModifier,  QCoreApplication::translate("QShortcut", "Num").toLower().append(QChar('+')));
   }
   modifs += *gmodifs; // Test non-translated ones last

   QString sl = accel;

#ifdef Q_OS_DARWIN
   for (int i = 0; i < modifs.size(); ++i) {
      const QModifKeyName &mkf = modifs.at(i);
      if (sl.contains(mkf.name)) {
         ret |= mkf.qt_key;
         accel.remove(mkf.name);
         sl = accel;
      }
   }

#endif

   int i     = 0;
   int lastI = 0;

   while ((i = sl.indexOf(QChar('+'), i + 1)) != -1) {
      const QStringView sub = sl.midView(lastI, i - lastI + 1);

      // If we get here the shortcuts contains at least one '+'. We break up
      // along the following strategy:
      //      Meta+Ctrl++   ( "Meta+", "Ctrl+", "+" )
      //      Super+Shift+A ( "Super+", "Shift+" )
      //      4+3+2=1       ( "4+", "3+" )
      // In other words, everything we try to handle HAS to be a modifier
      // except for a single '+' at the end of the string.

      // Only '+' can have length 1.
      if (sub.length() == 1) {
         // Make sure we only encounter a single '+' at the end of the accel
         if (accel.lastIndexOf(QChar('+')) != accel.length() - 1) {
            return Qt::Key_unknown;
         }

      } else {
         // Identify the modifier
         bool validModifier = false;
         for (int j = 0; j < modifs.size(); ++j) {
            const QModifKeyName &mkf = modifs.at(j);

            if (sub == mkf.name) {
               ret |= mkf.qt_key;
               validModifier = true;
               break; // Shortcut, since if we find an other it would/should just be a dup
            }
         }

         if (!validModifier) {
            return Qt::Key_unknown;
         }
      }
      lastI = i + 1;
   }

   int p = accel.lastIndexOf('+', str.length() - 2); // -2 so that Ctrl++ works
   if (p > 0) {
      accel = accel.mid(p + 1);
   }

   int fnum = 0;
   if (accel.length() == 1) {

#ifdef Q_OS_DARWIN
      int qtKey = qtkeyForMacSymbol(accel[0]);

      if (qtKey != -1) {
         ret |= qtKey;
      } else
#endif

      {
         ret |= accel[0].toUpper()[0].unicode();
      }

   } else if (accel[0] == 'f' && (fnum = accel.mid(1).toInteger<int>()) && (fnum >= 1) && (fnum <= 35)) {
      ret |= Qt::Key_F1 + fnum - 1;

   } else {
      // For NativeText, check the traslation table first,
      // if we don't find anything then try it out with just the untranlated stuff.
      // PortableText will only try the untranlated table.
      bool found = false;

      for (int tran = 0; tran < 2; ++tran) {
         if (! nativeText) {
            ++tran;
         }

         for (int i = 0; i < numKeyNames; ++i) {
            QString keyName;

            if (tran == 0) {
               keyName = QCoreApplication::translate("QShortcut", keyname[i].name);
            } else {
               keyName = QString::fromLatin1(keyname[i].name);
            }

            if (accel == keyName.toLower()) {
               ret |= keyname[i].key;
               found = true;
               break;
            }
         }

         if (found) {
            break;
         }
      }

      // unable to translate the key
      if (!found) {
         return Qt::Key_unknown;
      }
   }

   return ret;
}


QString QKeySequence::encodeString(int key)
{
   return QKeySequencePrivate::encodeString(key, NativeText);
}

static inline void addKey(QString &str, const QString &theKey, QKeySequence::SequenceFormat format)
{
   if (! str.isEmpty()) {
      str += (format == QKeySequence::NativeText) ? QCoreApplication::translate("QShortcut", "+")
         : QString::fromLatin1("+");
   }

   str += theKey;
}

QString QKeySequencePrivate::encodeString(int key, QKeySequence::SequenceFormat format)
{
   bool nativeText = (format == QKeySequence::NativeText);
   QString s;

   if (key == -1 || key == Qt::Key_unknown) {
      return s;
   }

#if defined(Q_OS_DARWIN)
   if (nativeText) {
      // On Mac OS X the order (by default) is Meta, Alt, Shift, Control.
      // If the AA_MacDontSwapCtrlAndMeta is enabled, then the order
      // is Ctrl, Alt, Shift, Meta. The macSymbolForQtKey does this swap
      // for us, which means that we have to adjust our order here.
      // The upshot is a lot more infrastructure to keep the number of
      // if tests down and the code relatively clean.

      static const int ModifierOrder[]         = {
            Qt::MetaModifier, Qt::AltModifier, Qt::ShiftModifier, Qt::ControlModifier, 0 };

      static const int QtKeyOrder[]            = {
            Qt::Key_Meta, Qt::Key_Alt, Qt::Key_Shift, Qt::Key_Control, 0 };

      static const int DontSwapModifierOrder[] = {
            Qt::ControlModifier, Qt::AltModifier, Qt::ShiftModifier, Qt::MetaModifier, 0 };

      static const int DontSwapQtKeyOrder[]    = {
            Qt::Key_Control, Qt::Key_Alt, Qt::Key_Shift, Qt::Key_Meta, 0 };

      const int *modifierOrder;
      const int *qtkeyOrder;

      if (qApp->testAttribute(Qt::AA_MacDontSwapCtrlAndMeta)) {
         modifierOrder = DontSwapModifierOrder;
         qtkeyOrder = DontSwapQtKeyOrder;
      } else {
         modifierOrder = ModifierOrder;
         qtkeyOrder = QtKeyOrder;
      }

      for (int i = 0; modifierOrder[i] != 0; ++i) {
         if (key & modifierOrder[i]) {
            s += qt_macSymbolForQtKey(qtkeyOrder[i]);
         }
      }

   } else

#endif

   {
      // On other systems the order is Meta, Control, Alt, Shift
      if ((key & Qt::MetaModifier) == Qt::MetaModifier) {
         s = nativeText ? QCoreApplication::translate("QShortcut", "Meta") : QString::fromLatin1("Meta");
      }

      if ((key & Qt::ControlModifier) == Qt::ControlModifier) {
         addKey(s, nativeText ? QCoreApplication::translate("QShortcut", "Ctrl") : QString::fromLatin1("Ctrl"), format);
      }

      if ((key & Qt::AltModifier) == Qt::AltModifier) {
         addKey(s, nativeText ? QCoreApplication::translate("QShortcut", "Alt") : QString::fromLatin1("Alt"), format);
      }

      if ((key & Qt::ShiftModifier) == Qt::ShiftModifier) {
         addKey(s, nativeText ? QCoreApplication::translate("QShortcut", "Shift") : QString::fromLatin1("Shift"), format);
      }
   }

   if ((key & Qt::KeypadModifier) == Qt::KeypadModifier) {
      addKey(s, nativeText ? QCoreApplication::translate("QShortcut", "Num") : QString::fromLatin1("Num"), format);
   }

   QString p = keyName(key, format);

#if defined(Q_OS_DARWIN)
   if (nativeText) {
      s += p;
   } else
#endif

      addKey(s, p, format);
   return s;
}

QString QKeySequencePrivate::keyName(int key, QKeySequence::SequenceFormat format)
{
   bool nativeText = (format == QKeySequence::NativeText);

   key &= ~(Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier | Qt::KeypadModifier);
   QString p;

   if (key && key < Qt::Key_Escape && key != Qt::Key_Space) {
      p = QChar(char32_t(key)).toUpper();

   } else if (key >= Qt::Key_F1 && key <= Qt::Key_F35) {
      p = nativeText ? QCoreApplication::translate("QShortcut", "F%1").formatArg(key - Qt::Key_F1 + 1)
         : QString::fromLatin1("F%1").formatArg(key - Qt::Key_F1 + 1);

   } else if (key) {
      int i = 0;

#if defined(Q_OS_DARWIN)
      if (nativeText) {
         QChar ch = qt_macSymbolForQtKey(key);
         if (!ch.isNull()) {
            p = ch;
         } else {
            goto NonSymbol;
         }
      } else
#endif

      {

#ifdef Q_OS_DARWIN
      NonSymbol:
#endif

         while (i < numKeyNames) {
            if (key == keyname[i].key) {
               p = nativeText ? QCoreApplication::translate("QShortcut", keyname[i].name)
                  : QString::fromLatin1(keyname[i].name);
               break;
            }
            ++i;
         }

         // If we can't find the actual translatable keyname,
         // fall back on the unicode representation of it...
         // Or else characters like Qt::Key_aring may not get displayed
         // (Really depends on you locale)

         if (i >= numKeyNames) {
            p = QChar(char32_t(key)).toUpper();
         }
      }
   }

   return p;
}

QKeySequence::SequenceMatch QKeySequence::matches(const QKeySequence &seq) const
{
   uint userN = count();
   uint seqN  = seq.count();

   if (userN > seqN) {
      return NoMatch;
   }

   // If equal in length, we have a potential ExactMatch sequence,
   // else we already know it can only be partial.
   SequenceMatch match = (userN == seqN ? ExactMatch : PartialMatch);

   for (uint i = 0; i < userN; ++i) {
      int userKey = (*this)[i];
      int sequenceKey = seq[i];

      if (userKey != sequenceKey) {
         return NoMatch;
      }
   }

   return match;
}

QKeySequence::operator QVariant() const
{
   return QVariant(QVariant::KeySequence, this);
}

/*!
    Returns a reference to the element at position \a index in the key
    sequence. This can only be used to read an element.
 */
int QKeySequence::operator[](uint index) const
{
   Q_ASSERT_X(index < QKeySequencePrivate::MaxKeyCount, "QKeySequence::operator[]", "index out of range");
   return d->key[index];
}

/*!
    Assignment operator. Assigns the \a other key sequence to this object.
 */
QKeySequence &QKeySequence::operator=(const QKeySequence &other)
{
   qAtomicAssign(d, other.d);
   return *this;
}

bool QKeySequence::operator==(const QKeySequence &other) const
{
   return (d->key[0] == other.d->key[0] &&
         d->key[1] == other.d->key[1] &&
         d->key[2] == other.d->key[2] &&
         d->key[3] == other.d->key[3]);
}

uint qHash(const QKeySequence &key, uint seed)
{

   seed = qHash(key.d->key[0], seed);
   seed = qHash(key.d->key[1], seed);
   seed = qHash(key.d->key[2], seed);
   seed = qHash(key.d->key[3], seed);

   return seed;
}

bool QKeySequence::operator< (const QKeySequence &other) const
{
   for (int i = 0; i < 4; ++i) {
      if (d->key[i] != other.d->key[i]) {
         return d->key[i] < other.d->key[i];
      }
   }

   return false;
}

// internal
bool QKeySequence::isDetached() const
{
   return d->ref.load() == 1;
}

QString QKeySequence::toString(SequenceFormat format) const
{
   QString finalString;

   int end = count();

   for (int i = 0; i < end; ++i) {
      finalString += d->encodeString(d->key[i], format);
      finalString += ", ";
   }

   finalString.truncate(finalString.length() - 2);

   return finalString;
}

QKeySequence QKeySequence::fromString(const QString &str, SequenceFormat format)
{
   return QKeySequence(str, format);
}

QList<QKeySequence> QKeySequence::listFromString(const QString &str, SequenceFormat format)
{
   QList<QKeySequence> retval;

   QStringList list = str.split("; ");

   for (const QString &item : list) {
      retval.append(fromString(item, format));
   }

   return retval;
}

QString QKeySequence::listToString(const QList<QKeySequence> &list, SequenceFormat format)
{
   QString retval;

   for (const QKeySequence &item : list) {
      retval.append(item.toString(format));
      retval.append("; ");
   }

   retval.chop(2);

   return retval;
}

QDataStream &operator<<(QDataStream &s, const QKeySequence &keysequence)
{
   QList<quint32> list;
   list << keysequence.d->key[0];

   if (keysequence.count() > 1) {
      list << keysequence.d->key[1];
      list << keysequence.d->key[2];
      list << keysequence.d->key[3];
   }

   s << list;
   return s;
}

QDataStream &operator>>(QDataStream &s, QKeySequence &keysequence)
{
   qAtomicDetach(keysequence.d);

   QList<quint32> list;
   s >> list;

   for (int i = 0; i < 4; ++i) {
      keysequence.d->key[i] = list.value(i);
   }
   return s;
}

QDebug operator<<(QDebug dbg, const QKeySequence &p)
{
   QDebugStateSaver saver(dbg);
   dbg.nospace() << "QKeySequence(" << p.toString() << ')';

   return dbg.space();
}

#endif // QT_NO_SHORTCUT
