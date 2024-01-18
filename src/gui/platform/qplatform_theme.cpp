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

#include <qplatform_theme.h>
#include <qplatform_theme_p.h>

#include <qvariant.h>
#include <qstringlist.h>
#include <qfileinfo.h>
#include <qpalette.h>
#include <qplatform_integration.h>
#include <qplatform_dialoghelper.h>
#include <qtextformat.h>

#include <qiconloader_p.h>
#include <qguiapplication_p.h>

#include <algorithm>

// Table of key bindings, must be sorted on key sequence:
// The integer value of VK_KEY | Modifier Keys (e.g., VK_META, and etc.)
// A priority of 1 indicates that this is the primary key binding when multiple are defined.

enum KeyPlatform {
   KB_Win   = (1 << QPlatformTheme::WindowsKeyboardScheme),
   KB_Mac   = (1 << QPlatformTheme::MacKeyboardScheme),
   KB_X11   = (1 << QPlatformTheme::X11KeyboardScheme),
   KB_KDE   = (1 << QPlatformTheme::KdeKeyboardScheme),
   KB_Gnome = (1 << QPlatformTheme::GnomeKeyboardScheme),
   KB_CDE   = (1 << QPlatformTheme::CdeKeyboardScheme),
   KB_All   = 0xffff
};

const QKeyBinding QPlatformThemePrivate::keyBindings[] = {
   //  StandardKey                      Priority           Key Sequence                          Platforms

   {QKeySequence::HelpContents,            1,          Qt::ControlModifier | Qt::Key_Question,   KB_Mac},
   {QKeySequence::HelpContents,            0,          Qt::Key_F1,                               KB_Win | KB_X11},
   {QKeySequence::WhatsThis,               1,          Qt::ShiftModifier   | Qt::Key_F1,         KB_All},
   {QKeySequence::Open,                    1,          Qt::ControlModifier | Qt::Key_O,          KB_All},
   {QKeySequence::Close,                   0,          Qt::ControlModifier | Qt::Key_F4,         KB_Mac},
   {QKeySequence::Close,                   1,          Qt::ControlModifier | Qt::Key_F4,         KB_Win},
   {QKeySequence::Close,                   1,          Qt::ControlModifier | Qt::Key_W,          KB_Mac},
   {QKeySequence::Close,                   0,          Qt::ControlModifier | Qt::Key_W,          KB_Win | KB_X11},
   {QKeySequence::Save,                    1,          Qt::ControlModifier | Qt::Key_S,          KB_All},
   {QKeySequence::New,                     1,          Qt::ControlModifier | Qt::Key_N,          KB_All},
   {QKeySequence::Delete,                  0,          Qt::ControlModifier | Qt::Key_D,          KB_X11},
   {QKeySequence::Delete,                  1,          Qt::Key_Delete,                           KB_All},
   {QKeySequence::Delete,                  0,          Qt::MetaModifier    | Qt::Key_D,          KB_Mac},
   {QKeySequence::Cut,                     1,          Qt::ControlModifier | Qt::Key_X,          KB_All},
   {QKeySequence::Cut,                     0,          Qt::ShiftModifier   | Qt::Key_Delete,     KB_Win | KB_X11},
   {QKeySequence::Cut,                     0,          Qt::Key_F20,                              KB_X11},
   {QKeySequence::Cut,                     0,          Qt::MetaModifier    | Qt::Key_K,          KB_Mac},
   {QKeySequence::Copy,                    0,          Qt::ControlModifier | Qt::Key_Insert,     KB_X11 | KB_Win},
   {QKeySequence::Copy,                    1,          Qt::ControlModifier | Qt::Key_C,          KB_All},
   {QKeySequence::Copy,                    0,          Qt::Key_F16,                              KB_X11},
   {QKeySequence::Paste,                   0,          Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_Insert, KB_X11},
   {QKeySequence::Paste,                   1,          Qt::ControlModifier | Qt::Key_V,          KB_All},
   {QKeySequence::Paste,                   0,          Qt::ShiftModifier   | Qt::Key_Insert,     KB_Win | KB_X11},
   {QKeySequence::Paste,                   0,          Qt::Key_F18,                              KB_X11},
   {QKeySequence::Paste,                   0,          Qt::MetaModifier    | Qt::Key_Y,          KB_Mac},
   {QKeySequence::Undo,                    0,          Qt::AltModifier     | Qt::Key_Backspace,  KB_Win},
   {QKeySequence::Undo,                    1,          Qt::ControlModifier | Qt::Key_Z,          KB_All},
   {QKeySequence::Undo,                    0,          Qt::Key_F14,                              KB_X11},
   {QKeySequence::Redo,                    0,          Qt::AltModifier     | Qt::ShiftModifier | Qt::Key_Backspace, KB_Win},
   {QKeySequence::Redo,                    0,          Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_Z,  KB_Mac},
   {QKeySequence::Redo,                    0,          Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_Z,  KB_Win | KB_X11},
   {QKeySequence::Redo,                    1,          Qt::ControlModifier | Qt::Key_Y,             KB_Win},
   {QKeySequence::Back,                    1,          Qt::AltModifier     | Qt::Key_Left,          KB_Win | KB_X11},
   {QKeySequence::Back,                    0,          Qt::ControlModifier | Qt::Key_Left,          KB_Mac},
   {QKeySequence::Back,                    1,          Qt::ControlModifier | Qt::Key_BracketLeft,   KB_Mac},
   {QKeySequence::Back,                    0,          Qt::Key_Backspace,                           KB_Win},
   {QKeySequence::Forward,                 1,          Qt::AltModifier     | Qt::Key_Right,         KB_Win | KB_X11},
   {QKeySequence::Forward,                 0,          Qt::ControlModifier | Qt::Key_Right,         KB_Mac},
   {QKeySequence::Forward,                 1,          Qt::ControlModifier | Qt::Key_BracketRight,  KB_Mac},
   {QKeySequence::Forward,                 0,          Qt::ShiftModifier   | Qt::Key_Backspace,     KB_Win},
   {QKeySequence::Refresh,                 1,          Qt::ControlModifier | Qt::Key_R,             KB_Gnome | KB_Mac},
   {QKeySequence::Refresh,                 0,          Qt::Key_F5,                                  KB_Win | KB_X11},
   {QKeySequence::ZoomIn,                  1,          Qt::ControlModifier | Qt::Key_Plus,          KB_All},
   {QKeySequence::ZoomOut,                 1,          Qt::ControlModifier | Qt::Key_Minus,         KB_All},
   {QKeySequence::Print,                   1,          Qt::ControlModifier | Qt::Key_P,             KB_All},
   {QKeySequence::AddTab,                  1,          Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_N, KB_KDE},
   {QKeySequence::AddTab,                  0,          Qt::ControlModifier | Qt::Key_T,             KB_All},
   {QKeySequence::NextChild,               0,          Qt::ControlModifier | Qt::Key_F6,            KB_Win},
   {QKeySequence::NextChild,               0,          Qt::ControlModifier | Qt::Key_Tab,           KB_Mac},
   {QKeySequence::NextChild,               1,          Qt::ControlModifier | Qt::Key_Tab,           KB_Win | KB_X11},
   {QKeySequence::NextChild,               1,          Qt::ControlModifier | Qt::Key_BraceRight,    KB_Mac},
   {QKeySequence::NextChild,               0,          Qt::ControlModifier | Qt::Key_Comma,         KB_KDE},
   {QKeySequence::NextChild,               0,          Qt::Key_Forward,                             KB_All},
   {QKeySequence::PreviousChild,           0,          Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_F6,      KB_Win},
   {QKeySequence::PreviousChild,           0,          Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_Backtab, KB_Mac },
   {QKeySequence::PreviousChild,           1,          Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_Backtab, KB_Win | KB_X11},
   {QKeySequence::PreviousChild,           1,          Qt::ControlModifier | Qt::Key_BraceLeft,     KB_Mac},
   {QKeySequence::PreviousChild,           0,          Qt::ControlModifier | Qt::Key_Period,        KB_KDE},
   {QKeySequence::PreviousChild,           0,          Qt::Key_Back,                                KB_All},
   {QKeySequence::Find,                    0,          Qt::ControlModifier | Qt::Key_F,             KB_All},
   {QKeySequence::FindNext,                0,          Qt::ControlModifier | Qt::Key_G,             KB_Win},
   {QKeySequence::FindNext,                1,          Qt::ControlModifier | Qt::Key_G,             KB_Gnome | KB_Mac},
   {QKeySequence::FindNext,                1,          Qt::Key_F3,                                  KB_Win},
   {QKeySequence::FindNext,                0,          Qt::Key_F3,                                  KB_X11},
   {QKeySequence::FindPrevious,            0,          Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_G, KB_Win},
   {QKeySequence::FindPrevious,            1,          Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_G, KB_Gnome | KB_Mac},
   {QKeySequence::FindPrevious,            1,          Qt::ShiftModifier   | Qt::Key_F3,         KB_Win},
   {QKeySequence::FindPrevious,            0,          Qt::ShiftModifier   | Qt::Key_F3,         KB_X11},
   {QKeySequence::Replace,                 0,          Qt::ControlModifier | Qt::Key_R,          KB_KDE},
   {QKeySequence::Replace,                 0,          Qt::ControlModifier | Qt::Key_H,          KB_Gnome},
   {QKeySequence::Replace,                 0,          Qt::ControlModifier | Qt::Key_H,          KB_Win},
   {QKeySequence::SelectAll,               1,          Qt::ControlModifier | Qt::Key_A,          KB_All},
   {QKeySequence::Bold,                    1,          Qt::ControlModifier | Qt::Key_B,          KB_All},
   {QKeySequence::Italic,                  0,          Qt::ControlModifier | Qt::Key_I,          KB_All},
   {QKeySequence::Underline,               1,          Qt::ControlModifier | Qt::Key_U,          KB_All},
   {QKeySequence::MoveToNextChar,          1,          Qt::Key_Right,                            KB_All},
   {QKeySequence::MoveToNextChar,          0,          Qt::MetaModifier    | Qt::Key_F,          KB_Mac},
   {QKeySequence::MoveToPreviousChar,      1,          Qt::Key_Left,                             KB_All},
   {QKeySequence::MoveToPreviousChar,      0,          Qt::MetaModifier    | Qt::Key_B,          KB_Mac},
   {QKeySequence::MoveToNextWord,          0,          Qt::AltModifier     | Qt::Key_Right,      KB_Mac},
   {QKeySequence::MoveToNextWord,          0,          Qt::ControlModifier | Qt::Key_Right,      KB_Win | KB_X11},
   {QKeySequence::MoveToPreviousWord,      0,          Qt::AltModifier     | Qt::Key_Left,       KB_Mac},
   {QKeySequence::MoveToPreviousWord,      0,          Qt::ControlModifier | Qt::Key_Left,       KB_Win | KB_X11},
   {QKeySequence::MoveToNextLine,          1,          Qt::Key_Down,                             KB_All},
   {QKeySequence::MoveToNextLine,          0,          Qt::MetaModifier    | Qt::Key_N,          KB_Mac},
   {QKeySequence::MoveToPreviousLine,      1,          Qt::Key_Up,                               KB_All},
   {QKeySequence::MoveToPreviousLine,      0,          Qt::MetaModifier    | Qt::Key_P,          KB_Mac},
   {QKeySequence::MoveToNextPage,          0,          Qt::MetaModifier    | Qt::Key_PageDown,   KB_Mac},
   {QKeySequence::MoveToNextPage,          0,          Qt::MetaModifier    | Qt::Key_Down,       KB_Mac},
   {QKeySequence::MoveToNextPage,          0,          Qt::MetaModifier    | Qt::Key_V,          KB_Mac},
   {QKeySequence::MoveToNextPage,          0,          Qt::AltModifier     | Qt::Key_PageDown,   KB_Mac },
   {QKeySequence::MoveToNextPage,          1,          Qt::Key_PageDown,                         KB_All},
   {QKeySequence::MoveToPreviousPage,      0,          Qt::MetaModifier    | Qt::Key_PageUp,     KB_Mac},
   {QKeySequence::MoveToPreviousPage,      0,          Qt::MetaModifier    | Qt::Key_Up,         KB_Mac},
   {QKeySequence::MoveToPreviousPage,      0,          Qt::AltModifier     | Qt::Key_PageUp,     KB_Mac },
   {QKeySequence::MoveToPreviousPage,      1,          Qt::Key_PageUp,                           KB_All},
   {QKeySequence::MoveToStartOfLine,       0,          Qt::MetaModifier    | Qt::Key_Left,       KB_Mac},
   {QKeySequence::MoveToStartOfLine,       0,          Qt::ControlModifier | Qt::Key_Left,       KB_Mac },
   {QKeySequence::MoveToStartOfLine,       0,          Qt::Key_Home,                             KB_Win | KB_X11},
   {QKeySequence::MoveToEndOfLine,         0,          Qt::MetaModifier    | Qt::Key_Right,      KB_Mac},
   {QKeySequence::MoveToEndOfLine,         0,          Qt::ControlModifier | Qt::Key_Right,      KB_Mac },
   {QKeySequence::MoveToEndOfLine,         0,          Qt::Key_End,                              KB_Win | KB_X11},
   {QKeySequence::MoveToEndOfLine,         0,          Qt::ControlModifier + Qt::Key_E,          KB_X11},
   {QKeySequence::MoveToStartOfBlock,      0,          Qt::MetaModifier    | Qt::Key_A,          KB_Mac},
   {QKeySequence::MoveToStartOfBlock,      1,          Qt::AltModifier     | Qt::Key_Up,         KB_Mac}, //mac only
   {QKeySequence::MoveToEndOfBlock,        0,          Qt::MetaModifier    | Qt::Key_E,          KB_Mac},
   {QKeySequence::MoveToEndOfBlock,        1,          Qt::AltModifier     | Qt::Key_Down,       KB_Mac}, //mac only
   {QKeySequence::MoveToStartOfDocument,   1,          Qt::ControlModifier | Qt::Key_Up,         KB_Mac},
   {QKeySequence::MoveToStartOfDocument,   0,          Qt::ControlModifier | Qt::Key_Home,       KB_Win | KB_X11},
   {QKeySequence::MoveToStartOfDocument,   0,          Qt::Key_Home,                             KB_Mac},
   {QKeySequence::MoveToEndOfDocument,     1,          Qt::ControlModifier | Qt::Key_Down,       KB_Mac},
   {QKeySequence::MoveToEndOfDocument,     0,          Qt::ControlModifier | Qt::Key_End,        KB_Win | KB_X11},
   {QKeySequence::MoveToEndOfDocument,     0,          Qt::Key_End,                              KB_Mac},
   {QKeySequence::SelectNextChar,          0,          Qt::ShiftModifier   | Qt::Key_Right,      KB_All},
   {QKeySequence::SelectPreviousChar,      0,          Qt::ShiftModifier   | Qt::Key_Left,       KB_All},
   {QKeySequence::SelectNextWord,          0,          Qt::AltModifier     | Qt::ShiftModifier | Qt::Key_Right, KB_Mac},
   {QKeySequence::SelectNextWord,          0,          Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_Right, KB_Win | KB_X11},
   {QKeySequence::SelectPreviousWord,      0,          Qt::AltModifier     | Qt::ShiftModifier | Qt::Key_Left,  KB_Mac},
   {QKeySequence::SelectPreviousWord,      0,          Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_Left,  KB_Win | KB_X11},
   {QKeySequence::SelectNextLine,          0,          Qt::ShiftModifier   | Qt::Key_Down,         KB_All},
   {QKeySequence::SelectPreviousLine,      0,          Qt::ShiftModifier   | Qt::Key_Up,           KB_All},
   {QKeySequence::SelectNextPage,          0,          Qt::ShiftModifier   | Qt::Key_PageDown,     KB_All},
   {QKeySequence::SelectPreviousPage,      0,          Qt::ShiftModifier   | Qt::Key_PageUp,       KB_All},
   {QKeySequence::SelectStartOfLine,       0,          Qt::MetaModifier    | Qt::ShiftModifier | Qt::Key_Left,    KB_Mac},
   {QKeySequence::SelectStartOfLine,       1,          Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_Left,    KB_Mac },
   {QKeySequence::SelectStartOfLine,       0,          Qt::ShiftModifier   | Qt::Key_Home,                        KB_Win | KB_X11},
   {QKeySequence::SelectEndOfLine,         0,          Qt::MetaModifier    | Qt::ShiftModifier | Qt::Key_Right,   KB_Mac},
   {QKeySequence::SelectEndOfLine,         1,          Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_Right,   KB_Mac },
   {QKeySequence::SelectEndOfLine,         0,          Qt::ShiftModifier   | Qt::Key_End,                         KB_Win | KB_X11},
   {QKeySequence::SelectStartOfBlock,      1,          Qt::AltModifier     | Qt::ShiftModifier | Qt::Key_Up,      KB_Mac},
   {QKeySequence::SelectStartOfBlock,      0,          Qt::MetaModifier    | Qt::ShiftModifier | Qt::Key_A,       KB_Mac},
   {QKeySequence::SelectEndOfBlock,        1,          Qt::AltModifier     | Qt::ShiftModifier | Qt::Key_Down,    KB_Mac},
   {QKeySequence::SelectEndOfBlock,        0,          Qt::MetaModifier    | Qt::ShiftModifier | Qt::Key_E,       KB_Mac},
   {QKeySequence::SelectStartOfDocument,   1,          Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_Up,      KB_Mac},
   {QKeySequence::SelectStartOfDocument,   0,          Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_Home,    KB_Win | KB_X11},
   {QKeySequence::SelectStartOfDocument,   0,          Qt::ShiftModifier   | Qt::Key_Home,                        KB_Mac},
   {QKeySequence::SelectEndOfDocument,     1,          Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_Down,    KB_Mac},
   {QKeySequence::SelectEndOfDocument,     0,          Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_End,     KB_Win | KB_X11},
   {QKeySequence::SelectEndOfDocument,     0,          Qt::ShiftModifier   | Qt::Key_End,        KB_Mac},
   {QKeySequence::DeleteStartOfWord,       0,          Qt::AltModifier     | Qt::Key_Backspace,  KB_Mac},
   {QKeySequence::DeleteStartOfWord,       0,          Qt::ControlModifier | Qt::Key_Backspace,  KB_X11 | KB_Win},
   {QKeySequence::DeleteEndOfWord,         0,          Qt::AltModifier     | Qt::Key_Delete,     KB_Mac},
   {QKeySequence::DeleteEndOfWord,         0,          Qt::ControlModifier | Qt::Key_Delete,     KB_X11 | KB_Win},
   {QKeySequence::DeleteEndOfLine,         0,          Qt::ControlModifier | Qt::Key_K,          KB_X11},
   {QKeySequence::InsertParagraphSeparator, 0,         Qt::Key_Enter,                            KB_All},
   {QKeySequence::InsertParagraphSeparator, 0,         Qt::Key_Return,                           KB_All},
   {QKeySequence::InsertLineSeparator,     0,          Qt::MetaModifier    | Qt::Key_Enter,      KB_Mac},
   {QKeySequence::InsertLineSeparator,     0,          Qt::MetaModifier    | Qt::Key_Return,     KB_Mac},
   {QKeySequence::InsertLineSeparator,     0,          Qt::ShiftModifier   | Qt::Key_Enter,      KB_All},
   {QKeySequence::InsertLineSeparator,     0,          Qt::ShiftModifier   | Qt::Key_Return,     KB_All},
   {QKeySequence::InsertLineSeparator,     0,          Qt::MetaModifier    | Qt::Key_O,          KB_Mac},
   {QKeySequence::SaveAs,                  0,          Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_S,   KB_Gnome | KB_Mac},
   {QKeySequence::Preferences,             0,          Qt::ControlModifier | Qt::Key_Comma,                   KB_Mac},
   {QKeySequence::Quit,                    0,          Qt::ControlModifier | Qt::Key_Q,                       KB_X11 | KB_Gnome | KB_KDE | KB_Mac},
   {QKeySequence::FullScreen,              1,          Qt::MetaModifier    | Qt::ControlModifier | Qt::Key_F, KB_Mac},
   {QKeySequence::FullScreen,              0,          Qt::AltModifier     | Qt::Key_Enter,                   KB_Win},
   {QKeySequence::FullScreen,              0,          Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_F,   KB_KDE},
   {QKeySequence::FullScreen,              1,          Qt::ControlModifier | Qt::Key_F11,                     KB_Gnome},
   {QKeySequence::FullScreen,              1,          Qt::Key_F11,                                           KB_Win | KB_KDE},
   {QKeySequence::Deselect,                0,          Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_A,   KB_X11},
   {QKeySequence::DeleteCompleteLine,      0,          Qt::ControlModifier | Qt::Key_U,                       KB_X11},
   {QKeySequence::Backspace,               0,          Qt::MetaModifier    | Qt::Key_H,          KB_Mac},
   {QKeySequence::Cancel,                  0,          Qt::Key_Escape,                           KB_All},
   {QKeySequence::Cancel,                  0,          Qt::ControlModifier | Qt::Key_Period,     KB_Mac}
};

const uint QPlatformThemePrivate::numberOfKeyBindings = sizeof(QPlatformThemePrivate::keyBindings) / (sizeof(QKeyBinding));

QPlatformThemePrivate::QPlatformThemePrivate()
   : systemPalette(nullptr)
{
}

QPlatformThemePrivate::~QPlatformThemePrivate()
{
   delete systemPalette;
}

Q_GUI_EXPORT QPalette qt_fusionPalette();

void QPlatformThemePrivate::initializeSystemPalette()
{
   Q_ASSERT(!systemPalette);
   systemPalette = new QPalette(qt_fusionPalette());
}

QPlatformTheme::QPlatformTheme()
   : d_ptr(new QPlatformThemePrivate)
{
}

QPlatformTheme::QPlatformTheme(QPlatformThemePrivate *priv)
   : d_ptr(priv)
{
}

QPlatformTheme::~QPlatformTheme()
{
}

bool QPlatformTheme::usePlatformNativeDialog(DialogType type) const
{
   (void) type;
   return false;
}

QPlatformDialogHelper *QPlatformTheme::createPlatformDialogHelper(DialogType type) const
{
   (void) type;
   return nullptr;
}

const QPalette *QPlatformTheme::palette(Palette type) const
{
   Q_D(const QPlatformTheme);

   if (type == QPlatformTheme::SystemPalette) {
      if (!d->systemPalette) {
         const_cast<QPlatformTheme *>(this)->d_ptr->initializeSystemPalette();
      }
      return d->systemPalette;
   }

   return nullptr;
}

const QFont *QPlatformTheme::font(Font type) const
{
   (void) type;
   return nullptr;
}

QPixmap QPlatformTheme::standardPixmap(StandardPixmap sp, const QSizeF &size) const
{
   (void) sp;
   (void) size;

   // TODO Should return QCommonStyle pixmaps?
   return QPixmap();
}

QPixmap QPlatformTheme::fileIconPixmap(const QFileInfo &fileInfo, const QSizeF &size,
   QPlatformTheme::IconOptions iconOptions) const
{
   (void) fileInfo;
   (void) size;
   (void) iconOptions;

   // TODO Should return QCommonStyle pixmaps?
   return QPixmap();
}

QVariant QPlatformTheme::themeHint(ThemeHint hint) const
{
   // For theme hints which mirror platform integration style hints, query
   // the platform integration. The base QPlatformIntegration::styleHint()
   // function will in turn query QPlatformTheme::defaultThemeHint() if there
   // is no custom value.

   switch (hint) {
      case QPlatformTheme::CursorFlashTime:
         return QGuiApplicationPrivate::platformIntegration()->styleHint(QPlatformIntegration::CursorFlashTime);

      case QPlatformTheme::KeyboardInputInterval:
         return QGuiApplicationPrivate::platformIntegration()->styleHint(QPlatformIntegration::KeyboardInputInterval);

      case QPlatformTheme::KeyboardAutoRepeatRate:
         return QGuiApplicationPrivate::platformIntegration()->styleHint(QPlatformIntegration::KeyboardAutoRepeatRate);

      case QPlatformTheme::MouseDoubleClickInterval:
         return QGuiApplicationPrivate::platformIntegration()->styleHint(QPlatformIntegration::MouseDoubleClickInterval);

      case QPlatformTheme::StartDragDistance:
         return QGuiApplicationPrivate::platformIntegration()->styleHint(QPlatformIntegration::StartDragDistance);

      case QPlatformTheme::StartDragTime:
         return QGuiApplicationPrivate::platformIntegration()->styleHint(QPlatformIntegration::StartDragTime);

      case QPlatformTheme::StartDragVelocity:
         return QGuiApplicationPrivate::platformIntegration()->styleHint(QPlatformIntegration::StartDragVelocity);

      case QPlatformTheme::PasswordMaskDelay:
         return QGuiApplicationPrivate::platformIntegration()->styleHint(QPlatformIntegration::PasswordMaskDelay);

      case QPlatformTheme::PasswordMaskCharacter:
         return QGuiApplicationPrivate::platformIntegration()->styleHint(QPlatformIntegration::PasswordMaskCharacter);

      case QPlatformTheme::MousePressAndHoldInterval:
         return QGuiApplicationPrivate::platformIntegration()->styleHint(QPlatformIntegration::MousePressAndHoldInterval);

      case QPlatformTheme::ItemViewActivateItemOnSingleClick:
         return QGuiApplicationPrivate::platformIntegration()->styleHint(QPlatformIntegration::ItemViewActivateItemOnSingleClick);

      default:
         return QPlatformTheme::defaultThemeHint(hint);
   }
}

QVariant QPlatformTheme::defaultThemeHint(ThemeHint hint)
{
   switch (hint) {
      case QPlatformTheme::CursorFlashTime:
         return QVariant(1000);
      case QPlatformTheme::KeyboardInputInterval:
         return QVariant(400);
      case QPlatformTheme::KeyboardAutoRepeatRate:
         return QVariant(30);
      case QPlatformTheme::MouseDoubleClickInterval:
         return QVariant(400);
      case QPlatformTheme::StartDragDistance:
         return QVariant(10);
      case QPlatformTheme::StartDragTime:
         return QVariant(500);
      case QPlatformTheme::PasswordMaskDelay:
         return QVariant(int(0));
      case QPlatformTheme::PasswordMaskCharacter:
         return QVariant(QChar(0x25CF));
      case QPlatformTheme::StartDragVelocity:
         return QVariant(int(0)); // no limit
      case QPlatformTheme::UseFullScreenForPopupMenu:
         return QVariant(false);
      case QPlatformTheme::WindowAutoPlacement:
         return QVariant(false);
      case QPlatformTheme::DialogButtonBoxLayout:
         return QVariant(int(0));
      case QPlatformTheme::DialogButtonBoxButtonsHaveIcons:
         return QVariant(false);
      case QPlatformTheme::ItemViewActivateItemOnSingleClick:
         return QVariant(false);
      case QPlatformTheme::ToolButtonStyle:
         return QVariant(int(Qt::ToolButtonIconOnly));
      case QPlatformTheme::ToolBarIconSize:
         return QVariant(int(0));
      case QPlatformTheme::SystemIconThemeName:
      case QPlatformTheme::SystemIconFallbackThemeName:
         return QVariant(QString());
      case QPlatformTheme::IconThemeSearchPaths:
         return QVariant(QStringList());
      case QPlatformTheme::StyleNames:
         return QVariant(QStringList());

      case TextCursorWidth:
         return QVariant(1);

      case DropShadow:
         return QVariant(false);

      case MaximumScrollBarDragDistance:
         return QVariant(-1);

      case KeyboardScheme:
         return QVariant(int(WindowsKeyboardScheme));

      case UiEffects:
         return QVariant(int(0));

      case SpellCheckUnderlineStyle:
         return QVariant(int(QTextCharFormat::SpellCheckUnderline));

      case TabFocusBehavior:
         return QVariant(int(Qt::TabFocusAllControls));

      case IconPixmapSizes:
         return QVariant::fromValue(QList<int>());

      case DialogSnapToDefaultButton:
      case ContextMenuOnMouseRelease:
         return QVariant(false);

      case MousePressAndHoldInterval:
         return QVariant(800);

      case MouseDoubleClickDistance: {
         bool ok = false;
         const int dist = qgetenv("QT_DBL_CLICK_DIST").toInt(&ok);
         return QVariant(ok ? dist : 5);
      }

      case WheelScrollLines:
         return QVariant(3);
   }
   return QVariant();
}

QPlatformMenuItem *QPlatformTheme::createPlatformMenuItem() const
{
   return nullptr;
}

QPlatformMenu *QPlatformTheme::createPlatformMenu() const
{
   return nullptr;
}

QPlatformMenuBar *QPlatformTheme::createPlatformMenuBar() const
{
   return nullptr;
}

#ifndef QT_NO_SYSTEMTRAYICON

QPlatformSystemTrayIcon *QPlatformTheme::createPlatformSystemTrayIcon() const
{
   return nullptr;
}
#endif

QIconEngine *QPlatformTheme::createIconEngine(const QString &iconName) const
{
   return new QIconLoaderEngine(iconName);
}

#if defined(Q_OS_DARWIN)
static inline int maybeSwapShortcut(int shortcut)
{
   if (qApp->testAttribute(Qt::AA_MacDontSwapCtrlAndMeta)) {
      uint oldshortcut = shortcut;
      shortcut &= ~(Qt::ControlModifier | Qt::MetaModifier);

      if (oldshortcut & Qt::ControlModifier) {
         shortcut |= Qt::MetaModifier;
      }

      if (oldshortcut & Qt::MetaModifier) {
         shortcut |= Qt::ControlModifier;
      }
   }

   return shortcut;
}
#endif

// mixed-mode predicate: all of these overloads are actually needed (but not all for every compiler)
struct ByStandardKey {
   typedef bool result_type;

   bool operator()(QKeySequence::StandardKey lhs, QKeySequence::StandardKey rhs) const {
      return lhs < rhs;
   }

   bool operator()(const QKeyBinding &lhs, const QKeyBinding &rhs) const {
      return operator()(lhs.standardKey, rhs.standardKey);
   }

   bool operator()(QKeySequence::StandardKey lhs, const QKeyBinding &rhs) const {
      return operator()(lhs, rhs.standardKey);
   }

   bool operator()(const QKeyBinding &lhs, QKeySequence::StandardKey rhs) const {
      return operator()(lhs.standardKey, rhs);
   }
};

QList<QKeySequence> QPlatformTheme::keyBindings(QKeySequence::StandardKey key) const
{
   const uint platform = QPlatformThemePrivate::currentKeyPlatforms();
   QList <QKeySequence> list;

   std::pair<const QKeyBinding *, const QKeyBinding *> range =
      std::equal_range(QPlatformThemePrivate::keyBindings,
         QPlatformThemePrivate::keyBindings + QPlatformThemePrivate::numberOfKeyBindings,
         key, ByStandardKey());

   for (const QKeyBinding *it = range.first; it < range.second; ++it) {
      if (!(it->platform & platform)) {
         continue;
      }

      uint shortcut =
#if defined(Q_OS_DARWIN)
         maybeSwapShortcut(it->shortcut);
#else
         it->shortcut;
#endif

      if (it->priority > 0) {
         list.prepend(QKeySequence(shortcut));
      } else {
         list.append(QKeySequence(shortcut));
      }
   }

   return list;
}

QString QPlatformTheme::standardButtonText(int button) const
{
   return QPlatformTheme::defaultStandardButtonText(button);
}

QString QPlatformTheme::defaultStandardButtonText(int button)
{
   switch (button) {
      case QPlatformDialogHelper::Ok:
         return QCoreApplication::translate("QPlatformTheme", "OK");
      case QPlatformDialogHelper::Save:
         return QCoreApplication::translate("QPlatformTheme", "Save");
      case QPlatformDialogHelper::SaveAll:
         return QCoreApplication::translate("QPlatformTheme", "Save All");
      case QPlatformDialogHelper::Open:
         return QCoreApplication::translate("QPlatformTheme", "Open");
      case QPlatformDialogHelper::Yes:
         return QCoreApplication::translate("QPlatformTheme", "&Yes");
      case QPlatformDialogHelper::YesToAll:
         return QCoreApplication::translate("QPlatformTheme", "Yes to &All");
      case QPlatformDialogHelper::No:
         return QCoreApplication::translate("QPlatformTheme", "&No");
      case QPlatformDialogHelper::NoToAll:
         return QCoreApplication::translate("QPlatformTheme", "N&o to All");
      case QPlatformDialogHelper::Abort:
         return QCoreApplication::translate("QPlatformTheme", "Abort");
      case QPlatformDialogHelper::Retry:
         return QCoreApplication::translate("QPlatformTheme", "Retry");
      case QPlatformDialogHelper::Ignore:
         return QCoreApplication::translate("QPlatformTheme", "Ignore");
      case QPlatformDialogHelper::Close:
         return QCoreApplication::translate("QPlatformTheme", "Close");
      case QPlatformDialogHelper::Cancel:
         return QCoreApplication::translate("QPlatformTheme", "Cancel");
      case QPlatformDialogHelper::Discard:
         return QCoreApplication::translate("QPlatformTheme", "Discard");
      case QPlatformDialogHelper::Help:
         return QCoreApplication::translate("QPlatformTheme", "Help");
      case QPlatformDialogHelper::Apply:
         return QCoreApplication::translate("QPlatformTheme", "Apply");
      case QPlatformDialogHelper::Reset:
         return QCoreApplication::translate("QPlatformTheme", "Reset");
      case QPlatformDialogHelper::RestoreDefaults:
         return QCoreApplication::translate("QPlatformTheme", "Restore Defaults");
      default:
         break;
   }

   return QString();
}

unsigned QPlatformThemePrivate::currentKeyPlatforms()
{
   const uint keyboardScheme = QGuiApplicationPrivate::platformTheme()->themeHint(QPlatformTheme::KeyboardScheme).toInt();
   unsigned result = 1u << keyboardScheme;

   if (keyboardScheme == QPlatformTheme::KdeKeyboardScheme
      || keyboardScheme == QPlatformTheme::GnomeKeyboardScheme
      || keyboardScheme == QPlatformTheme::CdeKeyboardScheme) {
      result |= KB_X11;
   }

   return result;
}

