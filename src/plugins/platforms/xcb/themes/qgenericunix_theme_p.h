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

#ifndef QGENERICUNIX_THEME_P_H
#define QGENERICUNIX_THEME_P_H

#include <qplatform_theme.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qfont.h>

class QGenericUnixThemePrivate;
class QGnomeThemePrivate;

class ResourceHelper
{
 public:
   ResourceHelper();
   ~ResourceHelper() {
      clear();
   }

   void clear();

   QPalette *palettes[QPlatformTheme::NPalettes];
   QFont *fonts[QPlatformTheme::NFonts];
};

class QGenericUnixTheme : public QPlatformTheme
{
   Q_DECLARE_PRIVATE(QGenericUnixTheme)

 public:
   QGenericUnixTheme();

   static QPlatformTheme *createUnixTheme(const QString &name);
   static QStringList themeNames();

   const QFont *font(Font type) const override;
   QVariant themeHint(ThemeHint hint) const override;

   static QStringList xdgIconThemePaths();

#if ! defined(QT_NO_DBUS) && ! defined(QT_NO_SYSTEMTRAYICON)
   QPlatformSystemTrayIcon *createPlatformSystemTrayIcon() const override;
#endif

   static QString m_name;
};

#ifndef QT_NO_SETTINGS
class QKdeThemePrivate;

class QKdeTheme : public QPlatformTheme
{
   Q_DECLARE_PRIVATE(QKdeTheme)

 public:
   QKdeTheme(const QStringList &kdeDirs, int kdeVersion);

   static QPlatformTheme *createKdeTheme();
   QVariant themeHint(ThemeHint hint) const override;

   const QPalette *palette(Palette type = SystemPalette) const override;

   const QFont *font(Font type) const override;

#if ! defined(QT_NO_DBUS) && !defined(QT_NO_SYSTEMTRAYICON)
   QPlatformSystemTrayIcon *createPlatformSystemTrayIcon() const override;
#endif

   static QString m_name;
};
#endif // QT_NO_SETTINGS

class QGnomeTheme : public QPlatformTheme
{
   Q_DECLARE_PRIVATE(QGnomeTheme)

 public:
   QGnomeTheme();
   QVariant themeHint(ThemeHint hint) const override;
   const QFont *font(Font type) const override;
   QString standardButtonText(int button) const override;

   virtual QString gtkFontName() const;

#if ! defined(QT_NO_DBUS) && !defined(QT_NO_SYSTEMTRAYICON)
   QPlatformSystemTrayIcon *createPlatformSystemTrayIcon() const override;
#endif

   static QString m_name;
};

QPlatformTheme *qt_createUnixTheme();

#endif
