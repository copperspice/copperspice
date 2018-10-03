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

#include <algorithm>

#include <qkeysequence.h>
#include <qkeysequence_p.h>
#include <qapplication_p.h>

#ifndef QT_NO_SHORTCUT

#include <qshortcut.h>
#include <qdebug.h>
#include <qregularexpression.h>
#include <qdatastream.h>
#include <qvariant.h>

#ifdef Q_OS_MAC
# include <qt_mac_p.h>

static bool qt_sequence_no_mnemonics = true;

struct MacSpecialKey {
   int key;
   ushort macSymbol;
};

enum {
   kShiftUnicode   = 0x21E7,
   kCommandUnicode = 0x2318,
   kControlUnicode = 0x2303,
   kOptionUnicode  = 0x2325,
};

static const int NumEntries = 21;
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
   }
   return -1;
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
   { Qt::Key_Space,        QT_TRANSLATE_NOOP("QShortcut", "Space") },
   { Qt::Key_Escape,       QT_TRANSLATE_NOOP("QShortcut", "Esc") },
   { Qt::Key_Tab,          QT_TRANSLATE_NOOP("QShortcut", "Tab") },
   { Qt::Key_Backtab,      QT_TRANSLATE_NOOP("QShortcut", "Backtab") },
   { Qt::Key_Backspace,    QT_TRANSLATE_NOOP("QShortcut", "Backspace") },
   { Qt::Key_Return,       QT_TRANSLATE_NOOP("QShortcut", "Return") },
   { Qt::Key_Enter,        QT_TRANSLATE_NOOP("QShortcut", "Enter") },
   { Qt::Key_Insert,       QT_TRANSLATE_NOOP("QShortcut", "Ins") },
   { Qt::Key_Delete,       QT_TRANSLATE_NOOP("QShortcut", "Del") },
   { Qt::Key_Pause,        QT_TRANSLATE_NOOP("QShortcut", "Pause") },
   { Qt::Key_Print,        QT_TRANSLATE_NOOP("QShortcut", "Print") },
   { Qt::Key_SysReq,       QT_TRANSLATE_NOOP("QShortcut", "SysReq") },
   { Qt::Key_Home,         QT_TRANSLATE_NOOP("QShortcut", "Home") },
   { Qt::Key_End,          QT_TRANSLATE_NOOP("QShortcut", "End") },
   { Qt::Key_Left,         QT_TRANSLATE_NOOP("QShortcut", "Left") },
   { Qt::Key_Up,           QT_TRANSLATE_NOOP("QShortcut", "Up") },
   { Qt::Key_Right,        QT_TRANSLATE_NOOP("QShortcut", "Right") },
   { Qt::Key_Down,         QT_TRANSLATE_NOOP("QShortcut", "Down") },
   { Qt::Key_PageUp,       QT_TRANSLATE_NOOP("QShortcut", "PgUp") },
   { Qt::Key_PageDown,     QT_TRANSLATE_NOOP("QShortcut", "PgDown") },
   { Qt::Key_CapsLock,     QT_TRANSLATE_NOOP("QShortcut", "CapsLock") },
   { Qt::Key_NumLock,      QT_TRANSLATE_NOOP("QShortcut", "NumLock") },
   { Qt::Key_ScrollLock,   QT_TRANSLATE_NOOP("QShortcut", "ScrollLock") },
   { Qt::Key_Menu,         QT_TRANSLATE_NOOP("QShortcut", "Menu") },
   { Qt::Key_Help,         QT_TRANSLATE_NOOP("QShortcut", "Help") },

   // Special keys
   // Includes multimedia, launcher, lan keys ( bluetooth, wireless )
   // window navigation
   { Qt::Key_Back,                       QT_TRANSLATE_NOOP("QShortcut", "Back") },
   { Qt::Key_Forward,                    QT_TRANSLATE_NOOP("QShortcut", "Forward") },
   { Qt::Key_Stop,                       QT_TRANSLATE_NOOP("QShortcut", "Stop") },
   { Qt::Key_Refresh,                    QT_TRANSLATE_NOOP("QShortcut", "Refresh") },
   { Qt::Key_VolumeDown,                 QT_TRANSLATE_NOOP("QShortcut", "Volume Down") },
   { Qt::Key_VolumeMute,                 QT_TRANSLATE_NOOP("QShortcut", "Volume Mute") },
   { Qt::Key_VolumeUp,                   QT_TRANSLATE_NOOP("QShortcut", "Volume Up") },
   { Qt::Key_BassBoost,                  QT_TRANSLATE_NOOP("QShortcut", "Bass Boost") },
   { Qt::Key_BassUp,                     QT_TRANSLATE_NOOP("QShortcut", "Bass Up") },
   { Qt::Key_BassDown,                   QT_TRANSLATE_NOOP("QShortcut", "Bass Down") },
   { Qt::Key_TrebleUp,                   QT_TRANSLATE_NOOP("QShortcut", "Treble Up") },
   { Qt::Key_TrebleDown,                 QT_TRANSLATE_NOOP("QShortcut", "Treble Down") },
   { Qt::Key_MediaPlay,                  QT_TRANSLATE_NOOP("QShortcut", "Media Play") },
   { Qt::Key_MediaStop,                  QT_TRANSLATE_NOOP("QShortcut", "Media Stop") },
   { Qt::Key_MediaPrevious,              QT_TRANSLATE_NOOP("QShortcut", "Media Previous") },
   { Qt::Key_MediaNext,                  QT_TRANSLATE_NOOP("QShortcut", "Media Next") },
   { Qt::Key_MediaRecord,                QT_TRANSLATE_NOOP("QShortcut", "Media Record") },
   //: Media player pause button
   { Qt::Key_MediaPause,                 QT_TRANSLATE_NOOP("QShortcut", "Media Pause") },
   //: Media player button to toggle between playing and paused
   { Qt::Key_MediaTogglePlayPause,       QT_TRANSLATE_NOOP("QShortcut", "Toggle Media Play/Pause") },
   { Qt::Key_HomePage,                   QT_TRANSLATE_NOOP("QShortcut", "Home Page") },
   { Qt::Key_Favorites,                  QT_TRANSLATE_NOOP("QShortcut", "Favorites") },
   { Qt::Key_Search,                     QT_TRANSLATE_NOOP("QShortcut", "Search") },
   { Qt::Key_Standby,                    QT_TRANSLATE_NOOP("QShortcut", "Standby") },
   { Qt::Key_OpenUrl,                    QT_TRANSLATE_NOOP("QShortcut", "Open URL") },
   { Qt::Key_LaunchMail,                 QT_TRANSLATE_NOOP("QShortcut", "Launch Mail") },
   { Qt::Key_LaunchMedia,                QT_TRANSLATE_NOOP("QShortcut", "Launch Media") },
   { Qt::Key_Launch0,                    QT_TRANSLATE_NOOP("QShortcut", "Launch (0)") },
   { Qt::Key_Launch1,                    QT_TRANSLATE_NOOP("QShortcut", "Launch (1)") },
   { Qt::Key_Launch2,                    QT_TRANSLATE_NOOP("QShortcut", "Launch (2)") },
   { Qt::Key_Launch3,                    QT_TRANSLATE_NOOP("QShortcut", "Launch (3)") },
   { Qt::Key_Launch4,                    QT_TRANSLATE_NOOP("QShortcut", "Launch (4)") },
   { Qt::Key_Launch5,                    QT_TRANSLATE_NOOP("QShortcut", "Launch (5)") },
   { Qt::Key_Launch6,                    QT_TRANSLATE_NOOP("QShortcut", "Launch (6)") },
   { Qt::Key_Launch7,                    QT_TRANSLATE_NOOP("QShortcut", "Launch (7)") },
   { Qt::Key_Launch8,                    QT_TRANSLATE_NOOP("QShortcut", "Launch (8)") },
   { Qt::Key_Launch9,                    QT_TRANSLATE_NOOP("QShortcut", "Launch (9)") },
   { Qt::Key_LaunchA,                    QT_TRANSLATE_NOOP("QShortcut", "Launch (A)") },
   { Qt::Key_LaunchB,                    QT_TRANSLATE_NOOP("QShortcut", "Launch (B)") },
   { Qt::Key_LaunchC,                    QT_TRANSLATE_NOOP("QShortcut", "Launch (C)") },
   { Qt::Key_LaunchD,                    QT_TRANSLATE_NOOP("QShortcut", "Launch (D)") },
   { Qt::Key_LaunchE,                    QT_TRANSLATE_NOOP("QShortcut", "Launch (E)") },
   { Qt::Key_LaunchF,                    QT_TRANSLATE_NOOP("QShortcut", "Launch (F)") },
   { Qt::Key_MonBrightnessUp,            QT_TRANSLATE_NOOP("QShortcut", "Monitor Brightness Up") },
   { Qt::Key_MonBrightnessDown,          QT_TRANSLATE_NOOP("QShortcut", "Monitor Brightness Down") },
   { Qt::Key_KeyboardLightOnOff,         QT_TRANSLATE_NOOP("QShortcut", "Keyboard Light On/Off") },
   { Qt::Key_KeyboardBrightnessUp,       QT_TRANSLATE_NOOP("QShortcut", "Keyboard Brightness Up") },
   { Qt::Key_KeyboardBrightnessDown,     QT_TRANSLATE_NOOP("QShortcut", "Keyboard Brightness Down") },
   { Qt::Key_PowerOff,                   QT_TRANSLATE_NOOP("QShortcut", "Power Off") },
   { Qt::Key_WakeUp,                     QT_TRANSLATE_NOOP("QShortcut", "Wake Up") },
   { Qt::Key_Eject,                      QT_TRANSLATE_NOOP("QShortcut", "Eject") },
   { Qt::Key_ScreenSaver,                QT_TRANSLATE_NOOP("QShortcut", "Screensaver") },
   { Qt::Key_WWW,                        QT_TRANSLATE_NOOP("QShortcut", "WWW") },
   { Qt::Key_Sleep,                      QT_TRANSLATE_NOOP("QShortcut", "Sleep") },
   { Qt::Key_LightBulb,                  QT_TRANSLATE_NOOP("QShortcut", "LightBulb") },
   { Qt::Key_Shop,                       QT_TRANSLATE_NOOP("QShortcut", "Shop") },
   { Qt::Key_History,                    QT_TRANSLATE_NOOP("QShortcut", "History") },
   { Qt::Key_AddFavorite,                QT_TRANSLATE_NOOP("QShortcut", "Add Favorite") },
   { Qt::Key_HotLinks,                   QT_TRANSLATE_NOOP("QShortcut", "Hot Links") },
   { Qt::Key_BrightnessAdjust,           QT_TRANSLATE_NOOP("QShortcut", "Adjust Brightness") },
   { Qt::Key_Finance,                    QT_TRANSLATE_NOOP("QShortcut", "Finance") },
   { Qt::Key_Community,                  QT_TRANSLATE_NOOP("QShortcut", "Community") },
   { Qt::Key_AudioRewind,                QT_TRANSLATE_NOOP("QShortcut", "Audio Rewind") },
   { Qt::Key_BackForward,                QT_TRANSLATE_NOOP("QShortcut", "Back Forward") },
   { Qt::Key_ApplicationLeft,            QT_TRANSLATE_NOOP("QShortcut", "Application Left") },
   { Qt::Key_ApplicationRight,           QT_TRANSLATE_NOOP("QShortcut", "Application Right") },
   { Qt::Key_Book,                       QT_TRANSLATE_NOOP("QShortcut", "Book") },
   { Qt::Key_CD,                         QT_TRANSLATE_NOOP("QShortcut", "CD") },
   { Qt::Key_Calculator,                 QT_TRANSLATE_NOOP("QShortcut", "Calculator") },
   { Qt::Key_Clear,                      QT_TRANSLATE_NOOP("QShortcut", "Clear") },
   { Qt::Key_ClearGrab,                  QT_TRANSLATE_NOOP("QShortcut", "Clear Grab") },
   { Qt::Key_Close,                      QT_TRANSLATE_NOOP("QShortcut", "Close") },
   { Qt::Key_Copy,                       QT_TRANSLATE_NOOP("QShortcut", "Copy") },
   { Qt::Key_Cut,                        QT_TRANSLATE_NOOP("QShortcut", "Cut") },
   { Qt::Key_Display,                    QT_TRANSLATE_NOOP("QShortcut", "Display") },
   { Qt::Key_DOS,                        QT_TRANSLATE_NOOP("QShortcut", "DOS") },
   { Qt::Key_Documents,                  QT_TRANSLATE_NOOP("QShortcut", "Documents") },
   { Qt::Key_Excel,                      QT_TRANSLATE_NOOP("QShortcut", "Spreadsheet") },
   { Qt::Key_Explorer,                   QT_TRANSLATE_NOOP("QShortcut", "Browser") },
   { Qt::Key_Game,                       QT_TRANSLATE_NOOP("QShortcut", "Game") },
   { Qt::Key_Go,                         QT_TRANSLATE_NOOP("QShortcut", "Go") },
   { Qt::Key_iTouch,                     QT_TRANSLATE_NOOP("QShortcut", "iTouch") },
   { Qt::Key_LogOff,                     QT_TRANSLATE_NOOP("QShortcut", "Logoff") },
   { Qt::Key_Market,                     QT_TRANSLATE_NOOP("QShortcut", "Market") },
   { Qt::Key_Meeting,                    QT_TRANSLATE_NOOP("QShortcut", "Meeting") },
   { Qt::Key_MenuKB,                     QT_TRANSLATE_NOOP("QShortcut", "Keyboard Menu") },
   { Qt::Key_MenuPB,                     QT_TRANSLATE_NOOP("QShortcut", "Menu PB") },
   { Qt::Key_MySites,                    QT_TRANSLATE_NOOP("QShortcut", "My Sites") },
   { Qt::Key_News,                       QT_TRANSLATE_NOOP("QShortcut", "News") },
   { Qt::Key_OfficeHome,                 QT_TRANSLATE_NOOP("QShortcut", "Home Office") },
   { Qt::Key_Option,                     QT_TRANSLATE_NOOP("QShortcut", "Option") },
   { Qt::Key_Paste,                      QT_TRANSLATE_NOOP("QShortcut", "Paste") },
   { Qt::Key_Phone,                      QT_TRANSLATE_NOOP("QShortcut", "Phone") },
   { Qt::Key_Reply,                      QT_TRANSLATE_NOOP("QShortcut", "Reply") },
   { Qt::Key_Reload,                     QT_TRANSLATE_NOOP("QShortcut", "Reload") },
   { Qt::Key_RotateWindows,              QT_TRANSLATE_NOOP("QShortcut", "Rotate Windows") },
   { Qt::Key_RotationPB,                 QT_TRANSLATE_NOOP("QShortcut", "Rotation PB") },
   { Qt::Key_RotationKB,                 QT_TRANSLATE_NOOP("QShortcut", "Rotation KB") },
   { Qt::Key_Save,                       QT_TRANSLATE_NOOP("QShortcut", "Save") },
   { Qt::Key_Send,                       QT_TRANSLATE_NOOP("QShortcut", "Send") },
   { Qt::Key_Spell,                      QT_TRANSLATE_NOOP("QShortcut", "Spellchecker") },
   { Qt::Key_SplitScreen,                QT_TRANSLATE_NOOP("QShortcut", "Split Screen") },
   { Qt::Key_Support,                    QT_TRANSLATE_NOOP("QShortcut", "Support") },
   { Qt::Key_TaskPane,                   QT_TRANSLATE_NOOP("QShortcut", "Task Panel") },
   { Qt::Key_Terminal,                   QT_TRANSLATE_NOOP("QShortcut", "Terminal") },
   { Qt::Key_Tools,                      QT_TRANSLATE_NOOP("QShortcut", "Tools") },
   { Qt::Key_Travel,                     QT_TRANSLATE_NOOP("QShortcut", "Travel") },
   { Qt::Key_Video,                      QT_TRANSLATE_NOOP("QShortcut", "Video") },
   { Qt::Key_Word,                       QT_TRANSLATE_NOOP("QShortcut", "Word Processor") },
   { Qt::Key_Xfer,                       QT_TRANSLATE_NOOP("QShortcut", "XFer") },
   { Qt::Key_ZoomIn,                     QT_TRANSLATE_NOOP("QShortcut", "Zoom In") },
   { Qt::Key_ZoomOut,                    QT_TRANSLATE_NOOP("QShortcut", "Zoom Out") },
   { Qt::Key_Away,                       QT_TRANSLATE_NOOP("QShortcut", "Away") },
   { Qt::Key_Messenger,                  QT_TRANSLATE_NOOP("QShortcut", "Messenger") },
   { Qt::Key_WebCam,                     QT_TRANSLATE_NOOP("QShortcut", "WebCam") },
   { Qt::Key_MailForward,                QT_TRANSLATE_NOOP("QShortcut", "Mail Forward") },
   { Qt::Key_Pictures,                   QT_TRANSLATE_NOOP("QShortcut", "Pictures") },
   { Qt::Key_Music,                      QT_TRANSLATE_NOOP("QShortcut", "Music") },
   { Qt::Key_Battery,                    QT_TRANSLATE_NOOP("QShortcut", "Battery") },
   { Qt::Key_Bluetooth,                  QT_TRANSLATE_NOOP("QShortcut", "Bluetooth") },
   { Qt::Key_WLAN,                       QT_TRANSLATE_NOOP("QShortcut", "Wireless") },
   { Qt::Key_UWB,                        QT_TRANSLATE_NOOP("QShortcut", "Ultra Wide Band") },
   { Qt::Key_AudioForward,               QT_TRANSLATE_NOOP("QShortcut", "Audio Forward") },
   { Qt::Key_AudioRepeat,                QT_TRANSLATE_NOOP("QShortcut", "Audio Repeat") },
   { Qt::Key_AudioRandomPlay,            QT_TRANSLATE_NOOP("QShortcut", "Audio Random Play") },
   { Qt::Key_Subtitle,                   QT_TRANSLATE_NOOP("QShortcut", "Subtitle") },
   { Qt::Key_AudioCycleTrack,            QT_TRANSLATE_NOOP("QShortcut", "Audio Cycle Track") },
   { Qt::Key_Time,                       QT_TRANSLATE_NOOP("QShortcut", "Time") },
   { Qt::Key_Select,                     QT_TRANSLATE_NOOP("QShortcut", "Select") },
   { Qt::Key_View,                       QT_TRANSLATE_NOOP("QShortcut", "View") },
   { Qt::Key_TopMenu,                    QT_TRANSLATE_NOOP("QShortcut", "Top Menu") },
   { Qt::Key_Suspend,                    QT_TRANSLATE_NOOP("QShortcut", "Suspend") },
   { Qt::Key_Hibernate,                  QT_TRANSLATE_NOOP("QShortcut", "Hibernate") },

   // --------------------------------------------------------------
   // More consistent namings
   { Qt::Key_Print,        QT_TRANSLATE_NOOP("QShortcut", "Print Screen") },
   { Qt::Key_PageUp,       QT_TRANSLATE_NOOP("QShortcut", "Page Up") },
   { Qt::Key_PageDown,     QT_TRANSLATE_NOOP("QShortcut", "Page Down") },
   { Qt::Key_CapsLock,     QT_TRANSLATE_NOOP("QShortcut", "Caps Lock") },
   { Qt::Key_NumLock,      QT_TRANSLATE_NOOP("QShortcut", "Num Lock") },
   { Qt::Key_NumLock,      QT_TRANSLATE_NOOP("QShortcut", "Number Lock") },
   { Qt::Key_ScrollLock,   QT_TRANSLATE_NOOP("QShortcut", "Scroll Lock") },
   { Qt::Key_Insert,       QT_TRANSLATE_NOOP("QShortcut", "Insert") },
   { Qt::Key_Delete,       QT_TRANSLATE_NOOP("QShortcut", "Delete") },
   { Qt::Key_Escape,       QT_TRANSLATE_NOOP("QShortcut", "Escape") },
   { Qt::Key_SysReq,       QT_TRANSLATE_NOOP("QShortcut", "System Request") },

   // --------------------------------------------------------------
   // Keypad navigation keys
   { Qt::Key_Select,       QT_TRANSLATE_NOOP("QShortcut", "Select") },
   { Qt::Key_Yes,          QT_TRANSLATE_NOOP("QShortcut", "Yes") },
   { Qt::Key_No,           QT_TRANSLATE_NOOP("QShortcut", "No") },

   // --------------------------------------------------------------
   // Device keys
   { Qt::Key_Context1,         QT_TRANSLATE_NOOP("QShortcut", "Context1") },
   { Qt::Key_Context2,         QT_TRANSLATE_NOOP("QShortcut", "Context2") },
   { Qt::Key_Context3,         QT_TRANSLATE_NOOP("QShortcut", "Context3") },
   { Qt::Key_Context4,         QT_TRANSLATE_NOOP("QShortcut", "Context4") },
   //: Button to start a call (note: a separate button is used to end the call)
   { Qt::Key_Call,             QT_TRANSLATE_NOOP("QShortcut", "Call") },
   //: Button to end a call (note: a separate button is used to start the call)
   { Qt::Key_Hangup,           QT_TRANSLATE_NOOP("QShortcut", "Hangup") },
   //: Button that will hang up if we're in call, or make a call if we're not.
   { Qt::Key_ToggleCallHangup, QT_TRANSLATE_NOOP("QShortcut", "Toggle Call/Hangup") },
   { Qt::Key_Flip,             QT_TRANSLATE_NOOP("QShortcut", "Flip") },
   //: Button to trigger voice dialing
   { Qt::Key_VoiceDial,        QT_TRANSLATE_NOOP("QShortcut", "Voice Dial") },
   //: Button to redial the last number called
   { Qt::Key_LastNumberRedial, QT_TRANSLATE_NOOP("QShortcut", "Last Number Redial") },
   //: Button to trigger the camera shutter (take a picture)
   { Qt::Key_Camera,           QT_TRANSLATE_NOOP("QShortcut", "Camera Shutter") },
   //: Button to focus the camera
   { Qt::Key_CameraFocus,      QT_TRANSLATE_NOOP("QShortcut", "Camera Focus") },

   // --------------------------------------------------------------
   // Japanese keyboard support
   { Qt::Key_Kanji,            QT_TRANSLATE_NOOP("QShortcut", "Kanji") },
   { Qt::Key_Muhenkan,         QT_TRANSLATE_NOOP("QShortcut", "Muhenkan") },
   { Qt::Key_Henkan,           QT_TRANSLATE_NOOP("QShortcut", "Henkan") },
   { Qt::Key_Romaji,           QT_TRANSLATE_NOOP("QShortcut", "Romaji") },
   { Qt::Key_Hiragana,         QT_TRANSLATE_NOOP("QShortcut", "Hiragana") },
   { Qt::Key_Katakana,         QT_TRANSLATE_NOOP("QShortcut", "Katakana") },
   { Qt::Key_Hiragana_Katakana, QT_TRANSLATE_NOOP("QShortcut", "Hiragana Katakana") },
   { Qt::Key_Zenkaku,          QT_TRANSLATE_NOOP("QShortcut", "Zenkaku") },
   { Qt::Key_Hankaku,          QT_TRANSLATE_NOOP("QShortcut", "Hankaku") },
   { Qt::Key_Zenkaku_Hankaku,  QT_TRANSLATE_NOOP("QShortcut", "Zenkaku Hankaku") },
   { Qt::Key_Touroku,          QT_TRANSLATE_NOOP("QShortcut", "Touroku") },
   { Qt::Key_Massyo,           QT_TRANSLATE_NOOP("QShortcut", "Massyo") },
   { Qt::Key_Kana_Lock,        QT_TRANSLATE_NOOP("QShortcut", "Kana Lock") },
   { Qt::Key_Kana_Shift,       QT_TRANSLATE_NOOP("QShortcut", "Kana Shift") },
   { Qt::Key_Eisu_Shift,       QT_TRANSLATE_NOOP("QShortcut", "Eisu Shift") },
   { Qt::Key_Eisu_toggle,      QT_TRANSLATE_NOOP("QShortcut", "Eisu toggle") },
   { Qt::Key_Codeinput,        QT_TRANSLATE_NOOP("QShortcut", "Code input") },
   { Qt::Key_MultipleCandidate, QT_TRANSLATE_NOOP("QShortcut", "Multiple Candidate") },
   { Qt::Key_PreviousCandidate, QT_TRANSLATE_NOOP("QShortcut", "Previous Candidate") },

   // --------------------------------------------------------------
   // Korean keyboard support
   { Qt::Key_Hangul,          QT_TRANSLATE_NOOP("QShortcut", "Hangul") },
   { Qt::Key_Hangul_Start,    QT_TRANSLATE_NOOP("QShortcut", "Hangul Start") },
   { Qt::Key_Hangul_End,      QT_TRANSLATE_NOOP("QShortcut", "Hangul End") },
   { Qt::Key_Hangul_Hanja,    QT_TRANSLATE_NOOP("QShortcut", "Hangul Hanja") },
   { Qt::Key_Hangul_Jamo,     QT_TRANSLATE_NOOP("QShortcut", "Hangul Jamo") },
   { Qt::Key_Hangul_Romaja,   QT_TRANSLATE_NOOP("QShortcut", "Hangul Romaja") },
   { Qt::Key_Hangul_Jeonja,   QT_TRANSLATE_NOOP("QShortcut", "Hangul Jeonja") },
   { Qt::Key_Hangul_Banja,    QT_TRANSLATE_NOOP("QShortcut", "Hangul Banja") },
   { Qt::Key_Hangul_PreHanja, QT_TRANSLATE_NOOP("QShortcut", "Hangul PreHanja") },
   { Qt::Key_Hangul_PostHanja, QT_TRANSLATE_NOOP("QShortcut", "Hangul PostHanja") },
   { Qt::Key_Hangul_Special,  QT_TRANSLATE_NOOP("QShortcut", "Hangul Special") },

   // --------------------------------------------------------------
   // Miscellaenous keys
   { Qt::Key_Cancel,  QT_TRANSLATE_NOOP("QShortcut", "Cancel") },
   { Qt::Key_Printer,  QT_TRANSLATE_NOOP("QShortcut", "Printer") },
   { Qt::Key_Execute,  QT_TRANSLATE_NOOP("QShortcut", "Execute") },
   { Qt::Key_Play,  QT_TRANSLATE_NOOP("QShortcut", "Play") },
   { Qt::Key_Zoom,  QT_TRANSLATE_NOOP("QShortcut", "Zoom") },

   { 0, 0 }
};

// Table of key bindings. It must be sorted on key sequence.
// A priority of 1 indicates that this is the primary key binding when multiple are defined.

const QKeyBinding QKeySequencePrivate::keyBindings[] = {
   //   StandardKey                        Priority    Key Sequence                            Platforms
   {QKeySequence::Back,                     0,         Qt::Key_Backspace,                      QApplicationPrivate::KB_Win},
   {QKeySequence::InsertParagraphSeparator, 0,         Qt::Key_Return,                         QApplicationPrivate::KB_All},
   {QKeySequence::InsertParagraphSeparator, 0,         Qt::Key_Enter,                          QApplicationPrivate::KB_All},
   {QKeySequence::Delete,                  1,          Qt::Key_Delete,                         QApplicationPrivate::KB_All},
   {QKeySequence::MoveToStartOfLine,       0,          Qt::Key_Home,                           QApplicationPrivate::KB_Win | QApplicationPrivate::KB_X11},
   {QKeySequence::MoveToStartOfDocument,   0,          Qt::Key_Home,                           QApplicationPrivate::KB_Mac},
   {QKeySequence::MoveToEndOfLine,         0,          Qt::Key_End,                            QApplicationPrivate::KB_Win | QApplicationPrivate::KB_X11},
   {QKeySequence::MoveToEndOfDocument,     0,          Qt::Key_End,                            QApplicationPrivate::KB_Mac},
   {QKeySequence::MoveToPreviousChar,      0,          Qt::Key_Left,                           QApplicationPrivate::KB_All},
   {QKeySequence::MoveToPreviousLine,      0,          Qt::Key_Up,                             QApplicationPrivate::KB_All},
   {QKeySequence::MoveToNextChar,          0,          Qt::Key_Right,                          QApplicationPrivate::KB_All},
   {QKeySequence::MoveToNextLine,          0,          Qt::Key_Down,                           QApplicationPrivate::KB_All},
   {QKeySequence::MoveToPreviousPage,      1,          Qt::Key_PageUp,                         QApplicationPrivate::KB_All},
   {QKeySequence::MoveToNextPage,          1,          Qt::Key_PageDown,                       QApplicationPrivate::KB_All},
   {QKeySequence::HelpContents,            0,          Qt::Key_F1,                             QApplicationPrivate::KB_Win | QApplicationPrivate::KB_X11},
   {QKeySequence::FindNext,                0,          Qt::Key_F3,                             QApplicationPrivate::KB_X11},
   {QKeySequence::FindNext,                1,          Qt::Key_F3,                             QApplicationPrivate::KB_Win},
   {QKeySequence::Refresh,                 0,          Qt::Key_F5,                             QApplicationPrivate::KB_Win | QApplicationPrivate::KB_X11},
   {QKeySequence::Undo,                    0,          Qt::Key_F14,                            QApplicationPrivate::KB_X11}, // Undo on sun keyboards
   {QKeySequence::Copy,                    0,          Qt::Key_F16,                            QApplicationPrivate::KB_X11}, // Copy on sun keyboards
   {QKeySequence::Paste,                   0,          Qt::Key_F18,                            QApplicationPrivate::KB_X11}, // Paste on sun keyboards
   {QKeySequence::Cut,                     0,          Qt::Key_F20,                            QApplicationPrivate::KB_X11}, // Cut on sun keyboards
   {QKeySequence::PreviousChild,           0,          Qt::Key_Back,                           QApplicationPrivate::KB_All},
   {QKeySequence::NextChild,               0,          Qt::Key_Forward,                        QApplicationPrivate::KB_All},
   {QKeySequence::Forward,                 0,          Qt::SHIFT | Qt::Key_Backspace,          QApplicationPrivate::KB_Win},
   {QKeySequence::InsertLineSeparator,     0,          Qt::SHIFT | Qt::Key_Return,             QApplicationPrivate::KB_All},
   {QKeySequence::InsertLineSeparator,     0,          Qt::SHIFT | Qt::Key_Enter,              QApplicationPrivate::KB_All},
   {QKeySequence::Paste,                   0,          Qt::SHIFT | Qt::Key_Insert,             QApplicationPrivate::KB_Win | QApplicationPrivate::KB_X11},
   {QKeySequence::Cut,                     0,          Qt::SHIFT | Qt::Key_Delete,             QApplicationPrivate::KB_Win | QApplicationPrivate::KB_X11}, // mac ?
   {QKeySequence::SelectStartOfLine,       0,          Qt::SHIFT | Qt::Key_Home,               QApplicationPrivate::KB_Win | QApplicationPrivate::KB_X11},
   {QKeySequence::SelectStartOfDocument,   0,          Qt::SHIFT | Qt::Key_Home,               QApplicationPrivate::KB_Mac},
   {QKeySequence::SelectEndOfLine,         0,          Qt::SHIFT | Qt::Key_End,                QApplicationPrivate::KB_Win | QApplicationPrivate::KB_X11},
   {QKeySequence::SelectEndOfDocument,     0,          Qt::SHIFT | Qt::Key_End,                QApplicationPrivate::KB_Mac},
   {QKeySequence::SelectPreviousChar,      0,          Qt::SHIFT | Qt::Key_Left,               QApplicationPrivate::KB_All},
   {QKeySequence::SelectPreviousLine,      0,          Qt::SHIFT | Qt::Key_Up,                 QApplicationPrivate::KB_All},
   {QKeySequence::SelectNextChar,          0,          Qt::SHIFT | Qt::Key_Right,              QApplicationPrivate::KB_All},
   {QKeySequence::SelectNextLine,          0,          Qt::SHIFT | Qt::Key_Down,               QApplicationPrivate::KB_All},
   {QKeySequence::SelectPreviousPage,      0,          Qt::SHIFT | Qt::Key_PageUp,             QApplicationPrivate::KB_All},
   {QKeySequence::SelectNextPage,          0,          Qt::SHIFT | Qt::Key_PageDown,           QApplicationPrivate::KB_All},
   {QKeySequence::WhatsThis,               1,          Qt::SHIFT | Qt::Key_F1,                 QApplicationPrivate::KB_All},
   {QKeySequence::FindPrevious,            0,          Qt::SHIFT | Qt::Key_F3,                 QApplicationPrivate::KB_X11},
   {QKeySequence::FindPrevious,            1,          Qt::SHIFT | Qt::Key_F3,                 QApplicationPrivate::KB_Win},
   {QKeySequence::ZoomIn,                  1,          Qt::CTRL | Qt::Key_Plus,                QApplicationPrivate::KB_All},
   {QKeySequence::NextChild,               0,          Qt::CTRL | Qt::Key_Comma,               QApplicationPrivate::KB_KDE},
   {QKeySequence::Preferences,             0,          Qt::CTRL | Qt::Key_Comma,               QApplicationPrivate::KB_Mac},
   {QKeySequence::ZoomOut,                 1,          Qt::CTRL | Qt::Key_Minus,               QApplicationPrivate::KB_All},
   {QKeySequence::PreviousChild,           0,          Qt::CTRL | Qt::Key_Period,              QApplicationPrivate::KB_KDE},
   {QKeySequence::HelpContents,            1,          Qt::CTRL | Qt::Key_Question,            QApplicationPrivate::KB_Mac},
   {QKeySequence::SelectAll,               1,          Qt::CTRL | Qt::Key_A,                   QApplicationPrivate::KB_All},
   {QKeySequence::Bold,                    1,          Qt::CTRL | Qt::Key_B,                   QApplicationPrivate::KB_All},
   {QKeySequence::Copy,                    1,          Qt::CTRL | Qt::Key_C,                   QApplicationPrivate::KB_All},
   {QKeySequence::Delete,                  0,          Qt::CTRL | Qt::Key_D,                   QApplicationPrivate::KB_X11}, //emacs (line edit only)
   {QKeySequence::Find,                    0,          Qt::CTRL | Qt::Key_F,                   QApplicationPrivate::KB_All},
   {QKeySequence::FindNext,                1,          Qt::CTRL | Qt::Key_G,                   QApplicationPrivate::KB_Gnome | QApplicationPrivate::KB_Mac},
   {QKeySequence::FindNext,                0,          Qt::CTRL | Qt::Key_G,                   QApplicationPrivate::KB_Win},
   {QKeySequence::Replace,                 0,          Qt::CTRL | Qt::Key_H,                   QApplicationPrivate::KB_Win},
   {QKeySequence::Replace,                 0,          Qt::CTRL | Qt::Key_H,                   QApplicationPrivate::KB_Gnome},
   {QKeySequence::Italic,                  0,          Qt::CTRL | Qt::Key_I,                   QApplicationPrivate::KB_All},
   {QKeySequence::DeleteEndOfLine,         0,          Qt::CTRL | Qt::Key_K,                   QApplicationPrivate::KB_X11}, //emacs (line edit only)
   {QKeySequence::New,                     1,          Qt::CTRL | Qt::Key_N,                   QApplicationPrivate::KB_All},
   {QKeySequence::Open,                    1,          Qt::CTRL | Qt::Key_O,                   QApplicationPrivate::KB_All},
   {QKeySequence::Print,                   1,          Qt::CTRL | Qt::Key_P,                   QApplicationPrivate::KB_All},

   {QKeySequence::Quit,                    0,          Qt::CTRL | Qt::Key_Q,                   QApplicationPrivate::KB_X11 | QApplicationPrivate::KB_Gnome |
                                                                                               QApplicationPrivate::KB_KDE | QApplicationPrivate::KB_Mac},

   {QKeySequence::Refresh,                 1,          Qt::CTRL | Qt::Key_R,                   QApplicationPrivate::KB_Gnome | QApplicationPrivate::KB_Mac},
   {QKeySequence::Replace,                 0,          Qt::CTRL | Qt::Key_R,                   QApplicationPrivate::KB_KDE},
   {QKeySequence::Save,                    1,          Qt::CTRL | Qt::Key_S,                   QApplicationPrivate::KB_All},
   {QKeySequence::AddTab,                  0,          Qt::CTRL | Qt::Key_T,                   QApplicationPrivate::KB_All},
   {QKeySequence::Underline,               1,          Qt::CTRL | Qt::Key_U,                   QApplicationPrivate::KB_All},
   {QKeySequence::Paste,                   1,          Qt::CTRL | Qt::Key_V,                   QApplicationPrivate::KB_All},
   {QKeySequence::Close,                   0,          Qt::CTRL | Qt::Key_W,                   QApplicationPrivate::KB_Win | QApplicationPrivate::KB_X11},
   {QKeySequence::Close,                   1,          Qt::CTRL | Qt::Key_W,                   QApplicationPrivate::KB_Mac},
   {QKeySequence::Cut,                     1,          Qt::CTRL | Qt::Key_X,                   QApplicationPrivate::KB_All},
   {QKeySequence::Redo,                    1,          Qt::CTRL | Qt::Key_Y,                   QApplicationPrivate::KB_Win},
   {QKeySequence::Undo,                    1,          Qt::CTRL | Qt::Key_Z,                   QApplicationPrivate::KB_All},
   {QKeySequence::Back,                    1,          Qt::CTRL | Qt::Key_BracketLeft,         QApplicationPrivate::KB_Mac},
   {QKeySequence::Forward,                 1,          Qt::CTRL | Qt::Key_BracketRight,        QApplicationPrivate::KB_Mac},
   {QKeySequence::PreviousChild,           1,          Qt::CTRL | Qt::Key_BraceLeft,           QApplicationPrivate::KB_Mac},
   {QKeySequence::NextChild,               1,          Qt::CTRL | Qt::Key_BraceRight,          QApplicationPrivate::KB_Mac},
   {QKeySequence::NextChild,               1,          Qt::CTRL | Qt::Key_Tab,                 QApplicationPrivate::KB_Win | QApplicationPrivate::KB_X11},
   {QKeySequence::NextChild,               0,          Qt::CTRL | Qt::Key_Tab,                 QApplicationPrivate::KB_Mac}, //different priority from above
   {QKeySequence::DeleteStartOfWord,       0,          Qt::CTRL | Qt::Key_Backspace,           QApplicationPrivate::KB_X11 | QApplicationPrivate::KB_Win},
   {QKeySequence::Copy,                    0,          Qt::CTRL | Qt::Key_Insert,              QApplicationPrivate::KB_X11 | QApplicationPrivate::KB_Win},
   {QKeySequence::DeleteEndOfWord,         0,          Qt::CTRL | Qt::Key_Delete,              QApplicationPrivate::KB_X11 | QApplicationPrivate::KB_Win},
   {QKeySequence::MoveToStartOfDocument,   0,          Qt::CTRL | Qt::Key_Home,                QApplicationPrivate::KB_Win | QApplicationPrivate::KB_X11},
   {QKeySequence::MoveToEndOfDocument,     0,          Qt::CTRL | Qt::Key_End,                 QApplicationPrivate::KB_Win | QApplicationPrivate::KB_X11},
   {QKeySequence::Back,                    0,          Qt::CTRL | Qt::Key_Left,                QApplicationPrivate::KB_Mac},
   {QKeySequence::MoveToPreviousWord,      0,          Qt::CTRL | Qt::Key_Left,                QApplicationPrivate::KB_Win | QApplicationPrivate::KB_X11},
   {QKeySequence::MoveToStartOfLine,       0,          Qt::CTRL | Qt::Key_Left,                QApplicationPrivate::KB_Mac },
   {QKeySequence::MoveToStartOfDocument,   1,          Qt::CTRL | Qt::Key_Up,                  QApplicationPrivate::KB_Mac},
   {QKeySequence::Forward,                 0,          Qt::CTRL | Qt::Key_Right,               QApplicationPrivate::KB_Mac},
   {QKeySequence::MoveToEndOfLine,         0,          Qt::CTRL | Qt::Key_Right,               QApplicationPrivate::KB_Mac },
   {QKeySequence::MoveToNextWord,          0,          Qt::CTRL | Qt::Key_Right,               QApplicationPrivate::KB_Win | QApplicationPrivate::KB_X11},
   {QKeySequence::MoveToEndOfDocument,     1,          Qt::CTRL | Qt::Key_Down,                QApplicationPrivate::KB_Mac},
   {QKeySequence::Close,                   1,          Qt::CTRL | Qt::Key_F4,                  QApplicationPrivate::KB_Win},
   {QKeySequence::Close,                   0,          Qt::CTRL | Qt::Key_F4,                  QApplicationPrivate::KB_Mac},
   {QKeySequence::NextChild,               0,          Qt::CTRL | Qt::Key_F6,                  QApplicationPrivate::KB_Win},
   {QKeySequence::FindPrevious,            1,          Qt::CTRL | Qt::SHIFT | Qt::Key_G,       QApplicationPrivate::KB_Gnome | QApplicationPrivate::KB_Mac},
   {QKeySequence::FindPrevious,            0,          Qt::CTRL | Qt::SHIFT | Qt::Key_G,       QApplicationPrivate::KB_Win},
   {QKeySequence::AddTab,                  1,          Qt::CTRL | Qt::SHIFT | Qt::Key_N,       QApplicationPrivate::KB_KDE},
   {QKeySequence::SaveAs,                  0,          Qt::CTRL | Qt::SHIFT | Qt::Key_S,       QApplicationPrivate::KB_Gnome | QApplicationPrivate::KB_Mac},
   {QKeySequence::Redo,                    0,          Qt::CTRL | Qt::SHIFT | Qt::Key_Z,       QApplicationPrivate::KB_Win | QApplicationPrivate::KB_X11  },
   {QKeySequence::Redo,                    0,          Qt::CTRL | Qt::SHIFT | Qt::Key_Z,       QApplicationPrivate::KB_Mac},
   {QKeySequence::PreviousChild,           1,          Qt::CTRL | Qt::SHIFT | Qt::Key_Backtab, QApplicationPrivate::KB_Win | QApplicationPrivate::KB_X11},
   {QKeySequence::PreviousChild,           0,          Qt::CTRL | Qt::SHIFT | Qt::Key_Backtab, QApplicationPrivate::KB_Mac },//different priority from above
   {QKeySequence::Paste,                   0,          Qt::CTRL | Qt::SHIFT | Qt::Key_Insert,  QApplicationPrivate::KB_X11},
   {QKeySequence::SelectStartOfDocument,   0,          Qt::CTRL | Qt::SHIFT | Qt::Key_Home,    QApplicationPrivate::KB_Win | QApplicationPrivate::KB_X11},
   {QKeySequence::SelectEndOfDocument,     0,          Qt::CTRL | Qt::SHIFT | Qt::Key_End,     QApplicationPrivate::KB_Win | QApplicationPrivate::KB_X11},
   {QKeySequence::SelectPreviousWord,      0,          Qt::CTRL | Qt::SHIFT | Qt::Key_Left,    QApplicationPrivate::KB_Win | QApplicationPrivate::KB_X11},
   {QKeySequence::SelectStartOfLine,       1,          Qt::CTRL | Qt::SHIFT | Qt::Key_Left,    QApplicationPrivate::KB_Mac },
   {QKeySequence::SelectStartOfDocument,   1,          Qt::CTRL | Qt::SHIFT | Qt::Key_Up,      QApplicationPrivate::KB_Mac},
   {QKeySequence::SelectNextWord,          0,          Qt::CTRL | Qt::SHIFT | Qt::Key_Right,   QApplicationPrivate::KB_Win | QApplicationPrivate::KB_X11},
   {QKeySequence::SelectEndOfLine,         1,          Qt::CTRL | Qt::SHIFT | Qt::Key_Right,   QApplicationPrivate::KB_Mac },
   {QKeySequence::SelectEndOfDocument,     1,          Qt::CTRL | Qt::SHIFT | Qt::Key_Down,    QApplicationPrivate::KB_Mac},
   {QKeySequence::PreviousChild,           0,          Qt::CTRL | Qt::SHIFT | Qt::Key_F6,      QApplicationPrivate::KB_Win},
   {QKeySequence::Undo,                    0,          Qt::ALT  | Qt::Key_Backspace,           QApplicationPrivate::KB_Win},
   {QKeySequence::DeleteStartOfWord,       0,          Qt::ALT  | Qt::Key_Backspace,           QApplicationPrivate::KB_Mac},
   {QKeySequence::DeleteEndOfWord,         0,          Qt::ALT  | Qt::Key_Delete,              QApplicationPrivate::KB_Mac},
   {QKeySequence::Back,                    1,          Qt::ALT  | Qt::Key_Left,                QApplicationPrivate::KB_Win | QApplicationPrivate::KB_X11},
   {QKeySequence::MoveToPreviousWord,      0,          Qt::ALT  | Qt::Key_Left,                QApplicationPrivate::KB_Mac},
   {QKeySequence::MoveToStartOfBlock,      0,          Qt::ALT  | Qt::Key_Up,                  QApplicationPrivate::KB_Mac}, //mac only
   {QKeySequence::MoveToNextWord,          0,          Qt::ALT  | Qt::Key_Right,               QApplicationPrivate::KB_Mac},
   {QKeySequence::Forward,                 1,          Qt::ALT  | Qt::Key_Right,               QApplicationPrivate::KB_Win | QApplicationPrivate::KB_X11},
   {QKeySequence::MoveToEndOfBlock,        0,          Qt::ALT  | Qt::Key_Down,                QApplicationPrivate::KB_Mac}, //mac only
   {QKeySequence::MoveToPreviousPage,      0,          Qt::ALT  | Qt::Key_PageUp,              QApplicationPrivate::KB_Mac },
   {QKeySequence::MoveToNextPage,          0,          Qt::ALT  | Qt::Key_PageDown,            QApplicationPrivate::KB_Mac },
   {QKeySequence::Redo,                    0,          Qt::ALT  | Qt::SHIFT | Qt::Key_Backspace, QApplicationPrivate::KB_Win},
   {QKeySequence::SelectPreviousWord,      0,          Qt::ALT  | Qt::SHIFT | Qt::Key_Left,    QApplicationPrivate::KB_Mac},
   {QKeySequence::SelectStartOfBlock,      0,          Qt::ALT  | Qt::SHIFT | Qt::Key_Up,      QApplicationPrivate::KB_Mac}, //mac only
   {QKeySequence::SelectNextWord,          0,          Qt::ALT  | Qt::SHIFT | Qt::Key_Right,   QApplicationPrivate::KB_Mac},
   {QKeySequence::SelectEndOfBlock,        0,          Qt::ALT  | Qt::SHIFT | Qt::Key_Down,    QApplicationPrivate::KB_Mac}, //mac only
   {QKeySequence::MoveToStartOfBlock,      0,          Qt::META | Qt::Key_A,                   QApplicationPrivate::KB_Mac},
   {QKeySequence::Delete,                  0,          Qt::META | Qt::Key_D,                   QApplicationPrivate::KB_Mac},
   {QKeySequence::MoveToEndOfBlock,        0,          Qt::META | Qt::Key_E,                   QApplicationPrivate::KB_Mac},
   {QKeySequence::InsertLineSeparator,     0,          Qt::META | Qt::Key_Return,              QApplicationPrivate::KB_Mac},
   {QKeySequence::InsertLineSeparator,     0,          Qt::META | Qt::Key_Enter,               QApplicationPrivate::KB_Mac},
   {QKeySequence::MoveToStartOfLine,       0,          Qt::META | Qt::Key_Left,                QApplicationPrivate::KB_Mac},
   {QKeySequence::MoveToPreviousPage,      0,          Qt::META | Qt::Key_Up,                  QApplicationPrivate::KB_Mac},
   {QKeySequence::MoveToEndOfLine,         0,          Qt::META | Qt::Key_Right,               QApplicationPrivate::KB_Mac},
   {QKeySequence::MoveToNextPage,          0,          Qt::META | Qt::Key_Down,                QApplicationPrivate::KB_Mac},
   {QKeySequence::MoveToPreviousPage,      0,          Qt::META | Qt::Key_PageUp,              QApplicationPrivate::KB_Mac},
   {QKeySequence::MoveToNextPage,          0,          Qt::META | Qt::Key_PageDown,            QApplicationPrivate::KB_Mac},
   {QKeySequence::SelectStartOfBlock,      0,          Qt::META | Qt::SHIFT | Qt::Key_A,       QApplicationPrivate::KB_Mac},
   {QKeySequence::SelectEndOfBlock,        0,          Qt::META | Qt::SHIFT | Qt::Key_E,       QApplicationPrivate::KB_Mac},
   {QKeySequence::SelectStartOfLine,       0,          Qt::META | Qt::SHIFT | Qt::Key_Left,    QApplicationPrivate::KB_Mac},
   {QKeySequence::SelectEndOfLine,         0,          Qt::META | Qt::SHIFT | Qt::Key_Right,   QApplicationPrivate::KB_Mac}
};

const uint QKeySequencePrivate::numberOfKeyBindings = sizeof(QKeySequencePrivate::keyBindings) / (sizeof(QKeyBinding));

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

QKeySequence::QKeySequence(const QString &key)
{
   d = new QKeySequencePrivate();
   assign(key);
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

#ifdef Q_OS_MAC
static inline int maybeSwapShortcut(int shortcut)
{
   if (qApp->testAttribute(Qt::AA_MacDontSwapCtrlAndMeta)) {
      uint oldshortcut = shortcut;
      shortcut &= ~(Qt::CTRL | Qt::META);
      if (oldshortcut & Qt::CTRL) {
         shortcut |= Qt::META;
      }
      if (oldshortcut & Qt::META) {
         shortcut |= Qt::CTRL;
      }
   }
   return shortcut;
}
#endif


QList<QKeySequence> QKeySequence::keyBindings(StandardKey key)
{
   uint platform = QApplicationPrivate::currentPlatform();
   QList <QKeySequence> list;

   for (uint i = 0; i < QKeySequencePrivate::numberOfKeyBindings ; ++i) {
      QKeyBinding keyBinding = QKeySequencePrivate::keyBindings[i];

      if (keyBinding.standardKey == key && (keyBinding.platform & platform)) {
         uint shortcut =

#ifdef Q_OS_MAC
            maybeSwapShortcut(QKeySequencePrivate::keyBindings[i].shortcut);
#else
            QKeySequencePrivate::keyBindings[i].shortcut;
#endif

         if (keyBinding.priority > 0) {
            list.prepend(QKeySequence(shortcut));
         } else {
            list.append(QKeySequence(shortcut));
         }
      }
   }
   return list;
}

QKeySequence::~QKeySequence()
{
   if (!d->ref.deref()) {
      delete d;
   }
}

/*!
    \internal
    KeySequences should never be modified, but rather just created.
    Internally though we do need to modify to keep pace in event
    delivery.
*/

void QKeySequence::setKey(int key, int index)
{
   Q_ASSERT_X(index >= 0 && index < 4, "QKeySequence::setKey", "index out of range");
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
               ret = QKeySequence(c.unicode() + Qt::ALT);

               found = true;

            } else {
               qWarning("QKeySequence::mnemonic: \"%s\" contains multiple occurrences of '&'", qPrintable(text));

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

   while (keyseq.length() && n < 4) {
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

Q_GLOBAL_STATIC(QList<QModifKeyName>, globalModifs)
Q_GLOBAL_STATIC(QList<QModifKeyName>, globalPortableModifs)

int QKeySequence::decodeString(const QString &str)
{
   return QKeySequencePrivate::decodeString(str, NativeText);
}

int QKeySequencePrivate::decodeString(const QString &str, QKeySequence::SequenceFormat format)
{
   int ret = 0;
   QString accel = str.toLower();
   bool nativeText = (format == QKeySequence::NativeText);

   QList<QModifKeyName> *gmodifs;
   if (nativeText) {
      gmodifs = globalModifs();

      if (gmodifs->isEmpty()) {

#ifdef Q_OS_MAC
         const bool dontSwap = qApp->testAttribute(Qt::AA_MacDontSwapCtrlAndMeta);
         if (dontSwap) {
            *gmodifs << QModifKeyName(Qt::META, QChar(kCommandUnicode));
         } else {
            *gmodifs << QModifKeyName(Qt::CTRL, QChar(kCommandUnicode));
         }
         *gmodifs << QModifKeyName(Qt::ALT, QChar(kOptionUnicode));
         if (dontSwap) {
            *gmodifs << QModifKeyName(Qt::CTRL, QChar(kControlUnicode));
         } else {
            *gmodifs << QModifKeyName(Qt::META, QChar(kControlUnicode));
         }
         *gmodifs << QModifKeyName(Qt::SHIFT, QChar(kShiftUnicode));
#endif
         *gmodifs << QModifKeyName(Qt::CTRL, QLatin1String("ctrl+"))
                  << QModifKeyName(Qt::SHIFT, QLatin1String("shift+"))
                  << QModifKeyName(Qt::ALT, QLatin1String("alt+"))
                  << QModifKeyName(Qt::META, QLatin1String("meta+"))
                  << QModifKeyName(Qt::KeypadModifier, QLatin1String("numpad+"));
      }

   } else {
      gmodifs = globalPortableModifs();
      if (gmodifs->isEmpty()) {
         *gmodifs << QModifKeyName(Qt::CTRL, QLatin1String("ctrl+"))
                  << QModifKeyName(Qt::SHIFT, QLatin1String("shift+"))
                  << QModifKeyName(Qt::ALT, QLatin1String("alt+"))
                  << QModifKeyName(Qt::META, QLatin1String("meta+"))
                  << QModifKeyName(Qt::KeypadModifier, QLatin1String("numpad+"));
      }
   }
   if (!gmodifs) {
      return ret;
   }


   QList<QModifKeyName> modifs;
   if (nativeText) {
      modifs << QModifKeyName(Qt::CTRL, QShortcut::tr("Ctrl").toLower().append(QLatin1Char('+')))
             << QModifKeyName(Qt::SHIFT, QShortcut::tr("Shift").toLower().append(QLatin1Char('+')))
             << QModifKeyName(Qt::ALT, QShortcut::tr("Alt").toLower().append(QLatin1Char('+')))
             << QModifKeyName(Qt::META, QShortcut::tr("Meta").toLower().append(QLatin1Char('+')))
             << QModifKeyName(Qt::KeypadModifier, QShortcut::tr("Numpad").toLower().append(QLatin1Char('+')));
   }
   modifs += *gmodifs; // Test non-translated ones last

   QString sl = accel;
#ifdef Q_OS_MAC
   for (int i = 0; i < modifs.size(); ++i) {
      const QModifKeyName &mkf = modifs.at(i);
      if (sl.contains(mkf.name)) {
         ret |= mkf.qt_key;
         accel.remove(mkf.name);
         sl = accel;
      }
   }
#else
   int i     = 0;
   int lastI = 0;

   while ((i = sl.indexOf(QLatin1Char('+'), i + 1)) != -1) {
      const QString sub = sl.mid(lastI, i - lastI + 1);

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
         if (accel.lastIndexOf(QLatin1Char('+')) != accel.length() - 1) {
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
#endif

   int p = accel.lastIndexOf('+', str.length() - 2); // -2 so that Ctrl++ works
   if (p > 0) {
      accel = accel.mid(p + 1);
   }

   int fnum = 0;
   if (accel.length() == 1) {

#ifdef Q_OS_MAC
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
         if (!nativeText) {
            ++tran;
         }

         for (int i = 0; keyname[i].name; ++i) {
            QString keyName(tran == 0 ? QShortcut::tr(keyname[i].name) : QString::fromLatin1(keyname[i].name));

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
   }
   return ret;
}

/*!
    Creates a shortcut string for \a key. For example,
    Qt::CTRL+Qt::Key_O gives "Ctrl+O". The strings, "Ctrl", "Shift", etc. are
    translated (using QObject::tr()) in the "QShortcut" context.
 */
QString QKeySequence::encodeString(int key)
{
   return QKeySequencePrivate::encodeString(key, NativeText);
}

static inline void addKey(QString &str, const QString &theKey, QKeySequence::SequenceFormat format)
{
   if (!str.isEmpty())
      str += (format == QKeySequence::NativeText) ? QShortcut::tr("+")
             : QString::fromLatin1("+");
   str += theKey;
}

QString QKeySequencePrivate::encodeString(int key, QKeySequence::SequenceFormat format)
{
   bool nativeText = (format == QKeySequence::NativeText);
   QString s;

#if defined(Q_OS_MAC)
   if (nativeText) {
      // On Mac OS X the order (by default) is Meta, Alt, Shift, Control.
      // If the AA_MacDontSwapCtrlAndMeta is enabled, then the order
      // is Ctrl, Alt, Shift, Meta. The macSymbolForQtKey does this swap
      // for us, which means that we have to adjust our order here.
      // The upshot is a lot more infrastructure to keep the number of
      // if tests down and the code relatively clean.

      static const int ModifierOrder[] = { Qt::META, Qt::ALT, Qt::SHIFT, Qt::CTRL, 0 };
      static const int QtKeyOrder[] = { Qt::Key_Meta, Qt::Key_Alt, Qt::Key_Shift, Qt::Key_Control, 0 };
      static const int DontSwapModifierOrder[] = { Qt::CTRL, Qt::ALT, Qt::SHIFT, Qt::META, 0 };
      static const int DontSwapQtKeyOrder[] = { Qt::Key_Control, Qt::Key_Alt, Qt::Key_Shift, Qt::Key_Meta, 0 };

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
      if ((key & Qt::META) == Qt::META) {
         s = nativeText ? QShortcut::tr("Meta") : QString("Meta");
      }

      if ((key & Qt::CTRL) == Qt::CTRL) {
         addKey(s, nativeText ? QShortcut::tr("Ctrl") : QString("Ctrl"), format);
      }

      if ((key & Qt::ALT) == Qt::ALT) {
         addKey(s, nativeText ? QShortcut::tr("Alt") : QString("Alt"), format);
      }

      if ((key & Qt::SHIFT) == Qt::SHIFT) {
         addKey(s, nativeText ? QShortcut::tr("Shift") : QString("Shift"), format);
      }

      if ((key & Qt::KeypadModifier) == Qt::KeypadModifier) {
         addKey(s, nativeText ? QShortcut::tr("Numpad") : QString("Numpad"), format);
      }
   }

   key &= ~(Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier | Qt::KeypadModifier);
   QString p;

   if (key && key < Qt::Key_Escape && key != Qt::Key_Space) {
      p = QChar(char32_t(key)).toUpper();

   } else if (key >= Qt::Key_F1 && key <= Qt::Key_F35) {
      p = nativeText ? QShortcut::tr("F%1").formatArg(key - Qt::Key_F1 + 1)
          : QString::fromLatin1("F%1").formatArg(key - Qt::Key_F1 + 1);

   } else if (key) {
      int i = 0;

#if defined(Q_OS_MAC)
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

#ifdef Q_OS_MAC
      NonSymbol:
#endif

         while (keyname[i].name) {
            if (key == keyname[i].key) {
               p = nativeText ? QShortcut::tr(keyname[i].name)
                   : QString::fromLatin1(keyname[i].name);
               break;
            }
            ++i;
         }

         // If we can't find the actual translatable keyname,
         // fall back on the unicode representation of it...
         // Or else characters like Qt::Key_aring may not get displayed
         // (Really depends on you locale)

         if (! keyname[i].name) {
            p = QChar(char32_t(key)).toUpper();
         }
      }
   }

#ifdef Q_OS_MAC
   if (nativeText) {
      s += p;
   } else
#endif
      addKey(s, p, format);
   return s;
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
   Q_ASSERT_X(index < 4, "QKeySequence::operator[]", "index out of range");
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
   QList<QKeySequence> result;

   QStringList strings = str.split(QLatin1String("; "));
   for (const QString & string : strings) {
      result << fromString(string, format);
   }

   return result;
}

QString QKeySequence::listToString(const QList<QKeySequence> &list, SequenceFormat format)
{
   QString result;

   for (const QKeySequence & sequence : list) {
      result += sequence.toString(format);
      result += "; ";
   }

   result.truncate(result.length() - 2);

   return result;
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
   dbg.nospace() << "QKeySequence(" << p.toString() << ')';
   return dbg.space();
}

#endif // QT_NO_SHORTCUT



