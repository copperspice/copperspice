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

#ifndef QWINDOWSTHEME_H
#define QWINDOWSTHEME_H

#include <qwin_threadpoolrunner.h>
#include <qplatform_theme.h>
#include <qvariant.h>

class QWindow;

class QWindowsTheme : public QPlatformTheme
{
 public:
   QWindowsTheme();
   ~QWindowsTheme();

   static QWindowsTheme *instance() {
      return m_instance;
   }

   bool usePlatformNativeDialog(DialogType type) const override;
   QPlatformDialogHelper *createPlatformDialogHelper(DialogType type) const override;
   QVariant themeHint(ThemeHint) const override;

   const QPalette *palette(Palette type = SystemPalette) const override {
      return m_palettes[type];
   }

   const QFont *font(Font type = SystemFont) const override {
      return m_fonts[type];
   }

   QPixmap standardPixmap(StandardPixmap sp, const QSizeF &size) const override;
   QPixmap fileIconPixmap(const QFileInfo &fileInfo, const QSizeF &size,
      QPlatformTheme::IconOptions iconOptions = Qt::EmptyFlag) const override;

   void windowsThemeChanged(QWindow *window);
   void displayChanged() {
      refreshIconPixmapSizes();
   }

   static QString name;

 private:
   void refresh() {
      refreshPalettes();
      refreshFonts();
   }
   void clearPalettes();
   void refreshPalettes();
   void clearFonts();
   void refreshFonts();
   void refreshIconPixmapSizes();

   static QWindowsTheme *m_instance;
   QPalette *m_palettes[NPalettes];
   QFont *m_fonts[NFonts];
   mutable QWindowsThreadPoolRunner m_threadPoolRunner;
   QVariant m_fileIconSizes;
};

#endif
