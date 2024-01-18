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

#ifndef QPLATFORMTHEME_COCOA_H
#define QPLATFORMTHEME_COCOA_H

#include <qhash.h>
#include <qplatform_theme.h>

class QPalette;

class QCocoaTheme : public QPlatformTheme
{
 public:
   QCocoaTheme();
   ~QCocoaTheme();

   QPlatformMenuItem *createPlatformMenuItem() const override;
   QPlatformMenu *createPlatformMenu() const override;
   QPlatformMenuBar *createPlatformMenuBar() const override;

#ifndef QT_NO_SYSTEMTRAYICON
   QPlatformSystemTrayIcon *createPlatformSystemTrayIcon() const override;
#endif

   bool usePlatformNativeDialog(DialogType dialogType) const override;
   QPlatformDialogHelper *createPlatformDialogHelper(DialogType dialogType) const override;

   const QPalette *palette(Palette type = SystemPalette) const override;
   const QFont *font(Font type = SystemFont) const override;
   QPixmap standardPixmap(StandardPixmap sp, const QSizeF &size) const override;
   QPixmap fileIconPixmap(const QFileInfo &fileInfo, const QSizeF &size,
            QPlatformTheme::IconOptions options = Qt::EmptyFlag) const override;

   QVariant themeHint(ThemeHint hint) const override;
   QString standardButtonText(int button) const override;

   static QString name;

 private:
   mutable QPalette *m_systemPalette;
   mutable QHash<QPlatformTheme::Palette, QPalette *> m_palettes;
   mutable QHash<QPlatformTheme::Font, QFont *> m_fonts;
};

#endif
