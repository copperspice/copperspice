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

#ifndef QPLATFORM_THEME_H
#define QPLATFORM_THEME_H

#include <qglobal.h>
#include <qscopedpointer.h>
#include <qkeysequence.h>

class QIconEngine;
class QMenu;
class QMenuBar;
class QPlatformMenuItem;
class QPlatformMenu;
class QPlatformMenuBar;
class QPlatformDialogHelper;
class QPlatformSystemTrayIcon;
class QPlatformThemePrivate;
class QVariant;
class QPalette;
class QFont;
class QPixmap;
class QSizeF;
class QFileInfo;

class Q_GUI_EXPORT QPlatformTheme
{
   Q_DECLARE_PRIVATE(QPlatformTheme)

 public:
   enum ThemeHint {
      CursorFlashTime,
      KeyboardInputInterval,
      MouseDoubleClickInterval,
      StartDragDistance,
      StartDragTime,
      KeyboardAutoRepeatRate,
      PasswordMaskDelay,
      StartDragVelocity,
      TextCursorWidth,
      DropShadow,
      MaximumScrollBarDragDistance,
      ToolButtonStyle,
      ToolBarIconSize,
      ItemViewActivateItemOnSingleClick,
      SystemIconThemeName,
      SystemIconFallbackThemeName,
      IconThemeSearchPaths,
      StyleNames,
      WindowAutoPlacement,
      DialogButtonBoxLayout,
      DialogButtonBoxButtonsHaveIcons,
      UseFullScreenForPopupMenu,
      KeyboardScheme,
      UiEffects,
      SpellCheckUnderlineStyle,
      TabFocusBehavior,
      IconPixmapSizes,
      PasswordMaskCharacter,
      DialogSnapToDefaultButton,
      ContextMenuOnMouseRelease,
      MousePressAndHoldInterval,
      MouseDoubleClickDistance,
      WheelScrollLines
   };

   enum DialogType {
      FileDialog,
      ColorDialog,
      FontDialog,
      MessageDialog
   };

   enum Palette {
      SystemPalette,
      ToolTipPalette,
      ToolButtonPalette,
      ButtonPalette,
      CheckBoxPalette,
      RadioButtonPalette,
      HeaderPalette,
      ComboBoxPalette,
      ItemViewPalette,
      MessageBoxLabelPelette,
      MessageBoxLabelPalette = MessageBoxLabelPelette,
      TabBarPalette,
      LabelPalette,
      GroupBoxPalette,
      MenuPalette,
      MenuBarPalette,
      TextEditPalette,
      TextLineEditPalette,
      NPalettes
   };

   enum Font {
      SystemFont,
      MenuFont,
      MenuBarFont,
      MenuItemFont,
      MessageBoxFont,
      LabelFont,
      TipLabelFont,
      StatusBarFont,
      TitleBarFont,
      MdiSubWindowTitleFont,
      DockWidgetTitleFont,
      PushButtonFont,
      CheckBoxFont,
      RadioButtonFont,
      ToolButtonFont,
      ItemViewFont,
      ListViewFont,
      HeaderViewFont,
      ListBoxFont,
      ComboMenuItemFont,
      ComboLineEditFont,
      SmallFont,
      MiniFont,
      FixedFont,
      GroupBoxTitleFont,
      TabButtonFont,
      NFonts
   };

   enum StandardPixmap {  // Keep in sync with QStyle::StandardPixmap
      TitleBarMenuButton,
      TitleBarMinButton,
      TitleBarMaxButton,
      TitleBarCloseButton,
      TitleBarNormalButton,
      TitleBarShadeButton,
      TitleBarUnshadeButton,
      TitleBarContextHelpButton,
      DockWidgetCloseButton,
      MessageBoxInformation,
      MessageBoxWarning,
      MessageBoxCritical,
      MessageBoxQuestion,
      DesktopIcon,
      TrashIcon,
      ComputerIcon,
      DriveFDIcon,
      DriveHDIcon,
      DriveCDIcon,
      DriveDVDIcon,
      DriveNetIcon,
      DirOpenIcon,
      DirClosedIcon,
      DirLinkIcon,
      DirLinkOpenIcon,
      FileIcon,
      FileLinkIcon,
      ToolBarHorizontalExtensionButton,
      ToolBarVerticalExtensionButton,
      FileDialogStart,
      FileDialogEnd,
      FileDialogToParent,
      FileDialogNewFolder,
      FileDialogDetailedView,
      FileDialogInfoView,
      FileDialogContentsView,
      FileDialogListView,
      FileDialogBack,
      DirIcon,
      DialogOkButton,
      DialogCancelButton,
      DialogHelpButton,
      DialogOpenButton,
      DialogSaveButton,
      DialogCloseButton,
      DialogApplyButton,
      DialogResetButton,
      DialogDiscardButton,
      DialogYesButton,
      DialogNoButton,
      ArrowUp,
      ArrowDown,
      ArrowLeft,
      ArrowRight,
      ArrowBack,
      ArrowForward,
      DirHomeIcon,
      CommandLink,
      VistaShield,
      BrowserReload,
      BrowserStop,
      MediaPlay,
      MediaStop,
      MediaPause,
      MediaSkipForward,
      MediaSkipBackward,
      MediaSeekForward,
      MediaSeekBackward,
      MediaVolume,
      MediaVolumeMuted,
      // do not add any values below/greater than this
      CustomBase = 0xf0000000
   };

   enum KeyboardSchemes {
      WindowsKeyboardScheme,
      MacKeyboardScheme,
      X11KeyboardScheme,
      KdeKeyboardScheme,
      GnomeKeyboardScheme,
      CdeKeyboardScheme
   };

   enum UiEffect {
      GeneralUiEffect        = 0x1,
      AnimateMenuUiEffect    = 0x2,
      FadeMenuUiEffect       = 0x4,
      AnimateComboUiEffect   = 0x8,
      AnimateTooltipUiEffect = 0x10,
      FadeTooltipUiEffect    = 0x20,
      AnimateToolBoxUiEffect = 0x40
   };

   enum IconOption {
      DontUseCustomDirectoryIcons = 0x01
   };
   using IconOptions = QFlags<IconOption>;

   explicit QPlatformTheme();
   QPlatformTheme(const QPlatformTheme &) = delete;

   virtual ~QPlatformTheme();

   virtual QPlatformMenuItem *createPlatformMenuItem() const;
   virtual QPlatformMenu *createPlatformMenu() const;
   virtual QPlatformMenuBar *createPlatformMenuBar() const;
   virtual void showPlatformMenuBar() {}

   virtual bool usePlatformNativeDialog(DialogType type) const;
   virtual QPlatformDialogHelper *createPlatformDialogHelper(DialogType type) const;

#ifndef QT_NO_SYSTEMTRAYICON
   virtual QPlatformSystemTrayIcon *createPlatformSystemTrayIcon() const;
#endif

   virtual const QPalette *palette(Palette type = SystemPalette) const;

   virtual const QFont *font(Font type = SystemFont) const;

   virtual QVariant themeHint(ThemeHint hint) const;

   virtual QPixmap standardPixmap(StandardPixmap sp, const QSizeF &size) const;
   virtual QPixmap fileIconPixmap(const QFileInfo &fileInfo, const QSizeF &size,
      QPlatformTheme::IconOptions iconOptions = Qt::EmptyFlag) const;

   virtual QIconEngine *createIconEngine(const QString &iconName) const;

   virtual QList<QKeySequence> keyBindings(QKeySequence::StandardKey key) const;

   virtual QString standardButtonText(int button) const;

   static QVariant defaultThemeHint(ThemeHint hint);
   static QString defaultStandardButtonText(int button);

 protected:
   explicit QPlatformTheme(QPlatformThemePrivate *priv);
   QScopedPointer<QPlatformThemePrivate> d_ptr;
};

#endif
