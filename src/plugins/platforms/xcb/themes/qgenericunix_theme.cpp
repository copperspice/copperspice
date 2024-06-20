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

#include <qgenericunix_theme_p.h>

#include <QPalette>
#include <QFont>
#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QDebug>
#include <QHash>
#include <QSettings>
#include <QVariant>
#include <QStandardPaths>
#include <QStringList>
#include <qplatform_integration.h>
#include <qplatform_services.h>
#include <qplatform_dialoghelper.h>

#include <qapplication_p.h>
#include <qgenericunix_services_p.h>
#include <qplatform_theme_p.h>

#if ! defined(QT_NO_DBUS) && ! defined(QT_NO_SYSTEMTRAYICON)
#include <qdbustrayicon_p.h>
#include <qdbusplatformmenu_p.h>
#endif

#include <algorithm>

ResourceHelper::ResourceHelper()
{
   std::fill(palettes, palettes + QPlatformTheme::NPalettes, static_cast<QPalette *>(nullptr));
   std::fill(fonts, fonts + QPlatformTheme::NFonts, static_cast<QFont *>(nullptr));
}

void ResourceHelper::clear()
{
   qDeleteAll(palettes, palettes + QPlatformTheme::NPalettes);
   qDeleteAll(fonts, fonts + QPlatformTheme::NFonts);
   std::fill(palettes, palettes + QPlatformTheme::NPalettes, static_cast<QPalette *>(nullptr));
   std::fill(fonts, fonts + QPlatformTheme::NFonts, static_cast<QFont *>(nullptr));
}

QString QGenericUnixTheme::m_name = "generic";

// Default system font
// XRender/FontConfig which we can now assume as default.

static const QString defaultSystemFontNameC = "Sans Serif";
constexpr int defaultSystemFontSize = 9;

#if ! defined(QT_NO_DBUS) && ! defined(QT_NO_SYSTEMTRAYICON)
static bool isDBusTrayAvailable()
{
   static bool dbusTrayAvailable = false;
   static bool dbusTrayAvailableKnown = false;

   if (! dbusTrayAvailableKnown) {
      QDBusMenuConnection conn;

      if (conn.isStatusNotifierHostRegistered()) {
         dbusTrayAvailable = true;
      }

      dbusTrayAvailableKnown = true;
   }

   return dbusTrayAvailable;
}
#endif

class QGenericUnixThemePrivate : public QPlatformThemePrivate
{
 public:
   QGenericUnixThemePrivate()
      : QPlatformThemePrivate(), systemFont(defaultSystemFontNameC, defaultSystemFontSize),
        fixedFont("monospace", systemFont.pointSize())
   {
      fixedFont.setStyleHint(QFont::TypeWriter);
   }

   const QFont systemFont;
   QFont fixedFont;
};

QGenericUnixTheme::QGenericUnixTheme()
   : QPlatformTheme(new QGenericUnixThemePrivate())
{
}

const QFont *QGenericUnixTheme::font(Font type) const
{
   Q_D(const QGenericUnixTheme);

   switch (type) {
      case QPlatformTheme::SystemFont:
         return &d->systemFont;

      case QPlatformTheme::FixedFont:
         return &d->fixedFont;

      default:
         return nullptr;
   }
}

// Helper to return the icon theme paths from XDG.
QStringList QGenericUnixTheme::xdgIconThemePaths()
{
   QStringList paths;
   // Add home directory first in search path
   const QFileInfo homeIconDir(QDir::homePath() + "/.icons");

   if (homeIconDir.isDir()) {
      paths.prepend(homeIconDir.absoluteFilePath());
   }

   QString xdgDirString = QFile::decodeName(qgetenv("XDG_DATA_DIRS"));
   if (xdgDirString.isEmpty()) {
      xdgDirString = "/usr/local/share/:/usr/share/";
   }

   for (const QString &xdgDir : xdgDirString.split(':')) {
      const QFileInfo xdgIconsDir(xdgDir + "/icons");

      if (xdgIconsDir.isDir()) {
         paths.append(xdgIconsDir.absoluteFilePath());
      }
   }

   const QFileInfo pixmapsIconsDir("/usr/share/pixmaps");
   if (pixmapsIconsDir.isDir()) {
      paths.append(pixmapsIconsDir.absoluteFilePath());
   }

   return paths;
}

#if !defined(QT_NO_DBUS) && !defined(QT_NO_SYSTEMTRAYICON)
QPlatformSystemTrayIcon *QGenericUnixTheme::createPlatformSystemTrayIcon() const
{
   if (isDBusTrayAvailable()) {
      return new QDBusTrayIcon();
   }
   return nullptr;
}
#endif

QVariant QGenericUnixTheme::themeHint(ThemeHint hint) const
{
   switch (hint) {
      case QPlatformTheme::SystemIconFallbackThemeName:
         return QVariant(QString("hicolor"));

      case QPlatformTheme::IconThemeSearchPaths:
         return xdgIconThemePaths();

      case QPlatformTheme::DialogButtonBoxButtonsHaveIcons:
         return QVariant(true);

      case QPlatformTheme::StyleNames: {
         QStringList styleNames;
         styleNames << "Fusion" << "Windows";

         return QVariant(styleNames);
      }

      case QPlatformTheme::KeyboardScheme:
         return QVariant(int(X11KeyboardScheme));

      default:
         break;
   }

   return QPlatformTheme::themeHint(hint);
}

#ifndef QT_NO_SETTINGS
class QKdeThemePrivate : public QPlatformThemePrivate
{
 public:
   QKdeThemePrivate(const QStringList &kdeDirs, int kdeVersion)
      : kdeDirs(kdeDirs)
      , kdeVersion(kdeVersion)
      , toolButtonStyle(Qt::ToolButtonTextBesideIcon)
      , toolBarIconSize(0)
      , singleClick(true)
      , wheelScrollLines(3)
   { }

   static QString kdeGlobals(const QString &kdeDir, int kdeVersion) {
      if (kdeVersion > 4) {
         return kdeDir + "/kdeglobals";
      }

      return kdeDir + "/share/config/kdeglobals";
   }

   void refresh();
   static QVariant readKdeSetting(const QString &key, const QStringList &kdeDirs, int kdeVersion,
      QHash<QString, QSettings *> &kdeSettings);
   static void readKdeSystemPalette(const QStringList &kdeDirs, int kdeVersion, QHash<QString, QSettings *> &kdeSettings, QPalette *pal);
   static QFont *kdeFont(const QVariant &fontValue);
   static QStringList kdeIconThemeSearchPaths(const QStringList &kdeDirs);

   const QStringList kdeDirs;
   const int kdeVersion;

   ResourceHelper resources;
   QString iconThemeName;
   QString iconFallbackThemeName;
   QStringList styleNames;
   int toolButtonStyle;
   int toolBarIconSize;
   bool singleClick;
   int wheelScrollLines;
};

void QKdeThemePrivate::refresh()
{
   resources.clear();

   toolButtonStyle = Qt::ToolButtonTextBesideIcon;
   toolBarIconSize = 0;
   styleNames.clear();

   if (kdeVersion >= 5) {
      styleNames << "breeze";
   }

   styleNames << "Oxygen" << "fusion" << "windows";

   if (kdeVersion >= 5) {
      iconFallbackThemeName = iconThemeName = "breeze";
   } else {
      iconFallbackThemeName = iconThemeName = "oxygen";
   }

   QHash<QString, QSettings *> kdeSettings;

   QPalette systemPalette = QPalette();
   readKdeSystemPalette(kdeDirs, kdeVersion, kdeSettings, &systemPalette);
   resources.palettes[QPlatformTheme::SystemPalette] = new QPalette(systemPalette);
   //## TODO tooltip color

   const QVariant styleValue = readKdeSetting("widgetStyle", kdeDirs, kdeVersion, kdeSettings);

   if (styleValue.isValid()) {
      const QString style = styleValue.toString();

      if (style != styleNames.front()) {
         styleNames.push_front(style);
      }
   }

   const QVariant singleClickValue = readKdeSetting("KDE/SingleClick", kdeDirs, kdeVersion, kdeSettings);
   if (singleClickValue.isValid()) {
      singleClick = singleClickValue.toBool();
   }

   const QVariant themeValue = readKdeSetting("Icons/Theme", kdeDirs, kdeVersion, kdeSettings);
   if (themeValue.isValid()) {
      iconThemeName = themeValue.toString();
   }

   const QVariant toolBarIconSizeValue = readKdeSetting("ToolbarIcons/Size", kdeDirs, kdeVersion, kdeSettings);
   if (toolBarIconSizeValue.isValid()) {
      toolBarIconSize = toolBarIconSizeValue.toInt();
   }

   const QVariant toolbarStyleValue = readKdeSetting("Toolbar style/ToolButtonStyle", kdeDirs, kdeVersion, kdeSettings);
   if (toolbarStyleValue.isValid()) {
      const QString toolBarStyle = toolbarStyleValue.toString();
      if (toolBarStyle == QLatin1String("TextBesideIcon")) {
         toolButtonStyle =  Qt::ToolButtonTextBesideIcon;
      }

      else if (toolBarStyle == QLatin1String("TextOnly")) {
         toolButtonStyle = Qt::ToolButtonTextOnly;
      }

      else if (toolBarStyle == QLatin1String("TextUnderIcon")) {
         toolButtonStyle = Qt::ToolButtonTextUnderIcon;
      }
   }

   const QVariant wheelScrollLinesValue = readKdeSetting("KDE/WheelScrollLines", kdeDirs, kdeVersion, kdeSettings);
   if (wheelScrollLinesValue.isValid()) {
      wheelScrollLines = wheelScrollLinesValue.toInt();
   }

   // Read system font, ignore 'smallestReadableFont'
   if (QFont *systemFont = kdeFont(readKdeSetting("font", kdeDirs, kdeVersion, kdeSettings))) {
      resources.fonts[QPlatformTheme::SystemFont] = systemFont;
   } else {
      resources.fonts[QPlatformTheme::SystemFont] = new QFont(QLatin1String(defaultSystemFontNameC), defaultSystemFontSize);
   }

   if (QFont *fixedFont = kdeFont(readKdeSetting("fixed", kdeDirs, kdeVersion, kdeSettings))) {
      resources.fonts[QPlatformTheme::FixedFont] = fixedFont;
   } else {
      fixedFont = new QFont(QLatin1String(defaultSystemFontNameC), defaultSystemFontSize);
      fixedFont->setStyleHint(QFont::TypeWriter);
      resources.fonts[QPlatformTheme::FixedFont] = fixedFont;
   }

   qDeleteAll(kdeSettings);
}

QVariant QKdeThemePrivate::readKdeSetting(const QString &key, const QStringList &kdeDirs, int kdeVersion,
   QHash<QString, QSettings *> &kdeSettings)
{
   for (const QString &kdeDir : kdeDirs) {
      QSettings *settings = kdeSettings.value(kdeDir);
      if (!settings) {
         const QString kdeGlobalsPath = kdeGlobals(kdeDir, kdeVersion);
         if (QFileInfo(kdeGlobalsPath).isReadable()) {
            settings = new QSettings(kdeGlobalsPath, QSettings::IniFormat);
            kdeSettings.insert(kdeDir, settings);
         }
      }
      if (settings) {
         const QVariant value = settings->value(key);
         if (value.isValid()) {
            return value;
         }
      }
   }
   return QVariant();
}

// Reads the color from the KDE configuration, and store it in the
// palette with the given color role if found.
static inline bool kdeColor(QPalette *pal, QPalette::ColorRole role, const QVariant &value)
{
   if (!value.isValid()) {
      return false;
   }

   const QStringList values = value.toStringList();

   if (values.size() != 3) {
      return false;
   }

   pal->setBrush(role, QColor(values.at(0).toInteger<int>(), values.at(1).toInteger<int>(), values.at(2).toInteger<int>()));

   return true;
}

void QKdeThemePrivate::readKdeSystemPalette(const QStringList &kdeDirs, int kdeVersion, QHash<QString, QSettings *> &kdeSettings,
   QPalette *pal)
{
   if (! kdeColor(pal, QPalette::Button, readKdeSetting("Colors:Button/BackgroundNormal", kdeDirs, kdeVersion, kdeSettings))) {
      // kcolorscheme.cpp: SetDefaultColors
      const QColor defaultWindowBackground(214, 210, 208);
      const QColor defaultButtonBackground(223, 220, 217);
      *pal = QPalette(defaultButtonBackground, defaultWindowBackground);
      return;
   }

   kdeColor(pal, QPalette::Window, readKdeSetting("Colors:Window/BackgroundNormal", kdeDirs, kdeVersion, kdeSettings));
   kdeColor(pal, QPalette::Text, readKdeSetting("Colors:View/ForegroundNormal",    kdeDirs, kdeVersion, kdeSettings));
   kdeColor(pal, QPalette::WindowText, readKdeSetting("Colors:Window/ForegroundNormal", kdeDirs, kdeVersion, kdeSettings));
   kdeColor(pal, QPalette::Base, readKdeSetting("Colors:View/BackgroundNormal", kdeDirs, kdeVersion, kdeSettings));
   kdeColor(pal, QPalette::Highlight, readKdeSetting("Colors:Selection/BackgroundNormal", kdeDirs, kdeVersion, kdeSettings));
   kdeColor(pal, QPalette::HighlightedText, readKdeSetting("Colors:Selection/ForegroundNormal", kdeDirs, kdeVersion, kdeSettings));
   kdeColor(pal, QPalette::AlternateBase, readKdeSetting("Colors:View/BackgroundAlternate", kdeDirs, kdeVersion, kdeSettings));
   kdeColor(pal, QPalette::ButtonText, readKdeSetting("Colors:Button/ForegroundNormal", kdeDirs, kdeVersion, kdeSettings));
   kdeColor(pal, QPalette::Link, readKdeSetting("Colors:View/ForegroundLink", kdeDirs, kdeVersion, kdeSettings));
   kdeColor(pal, QPalette::LinkVisited, readKdeSetting("Colors:View/ForegroundVisited", kdeDirs, kdeVersion, kdeSettings));
   kdeColor(pal, QPalette::ToolTipBase, readKdeSetting("Colors:Tooltip/BackgroundNormal", kdeDirs, kdeVersion, kdeSettings));
   kdeColor(pal, QPalette::ToolTipText, readKdeSetting("Colors:Tooltip/ForegroundNormal", kdeDirs, kdeVersion, kdeSettings));

   // The above code sets _all_ color roles to "normal" colors. In KDE, the disabled
   // color roles are calculated by applying various effects described in kdeglobals.
   // We use a bit simpler approach here, similar logic than in qt_palette_from_color().
   const QColor button = pal->color(QPalette::Button);
   int h, s, v;
   button.getHsv(&h, &s, &v);

   const QBrush whiteBrush = QBrush(Qt::white);
   const QBrush buttonBrush = QBrush(button);
   const QBrush buttonBrushDark = QBrush(button.darker(v > 128 ? 200 : 50));
   const QBrush buttonBrushDark150 = QBrush(button.darker(v > 128 ? 150 : 75));
   const QBrush buttonBrushLight150 = QBrush(button.lighter(v > 128 ? 150 : 75));
   const QBrush buttonBrushLight = QBrush(button.lighter(v > 128 ? 200 : 50));

   pal->setBrush(QPalette::Disabled, QPalette::WindowText, buttonBrushDark);
   pal->setBrush(QPalette::Disabled, QPalette::ButtonText, buttonBrushDark);
   pal->setBrush(QPalette::Disabled, QPalette::Button, buttonBrush);
   pal->setBrush(QPalette::Disabled, QPalette::Text, buttonBrushDark);
   pal->setBrush(QPalette::Disabled, QPalette::BrightText, whiteBrush);
   pal->setBrush(QPalette::Disabled, QPalette::Base, buttonBrush);
   pal->setBrush(QPalette::Disabled, QPalette::Window, buttonBrush);
   pal->setBrush(QPalette::Disabled, QPalette::Highlight, buttonBrushDark150);
   pal->setBrush(QPalette::Disabled, QPalette::HighlightedText, buttonBrushLight150);

   // set calculated colors for all groups
   pal->setBrush(QPalette::Light, buttonBrushLight);
   pal->setBrush(QPalette::Midlight, buttonBrushLight150);
   pal->setBrush(QPalette::Mid, buttonBrushDark150);
   pal->setBrush(QPalette::Dark, buttonBrushDark);
}

QString QKdeTheme::m_name = "kde";

QKdeTheme::QKdeTheme(const QStringList &kdeDirs, int kdeVersion)
   : QPlatformTheme(new QKdeThemePrivate(kdeDirs, kdeVersion))
{
   d_func()->refresh();
}

QFont *QKdeThemePrivate::kdeFont(const QVariant &fontValue)
{
   if (fontValue.isValid()) {
      // Read font value: Might be a QStringList as KDE stores fonts without quotes.
      // Also retrieve the family for the constructor since we cannot use the
      // default constructor of QFont, which accesses QApplication::systemFont()
      // causing recursion.
      QString fontDescription;
      QString fontFamily;

      if (fontValue.type() == QVariant::StringList) {
         const QStringList list = fontValue.toStringList();

         if (! list.isEmpty()) {
            fontFamily = list.first();
            fontDescription = list.join(QLatin1Char(','));
         }

      } else {
         fontDescription = fontFamily = fontValue.toString();
      }

      if (!fontDescription.isEmpty()) {
         QFont font(fontFamily);
         if (font.fromString(fontDescription)) {
            return new QFont(font);
         }
      }
   }

   return nullptr;
}


QStringList QKdeThemePrivate::kdeIconThemeSearchPaths(const QStringList &kdeDirs)
{
   QStringList paths = QGenericUnixTheme::xdgIconThemePaths();
   const QString iconPath = "/share/icons";

   for (const QString &candidate : kdeDirs) {
      const QFileInfo fi(candidate + iconPath);
      if (fi.isDir()) {
         paths.append(fi.absoluteFilePath());
      }
   }

   return paths;
}

QVariant QKdeTheme::themeHint(QPlatformTheme::ThemeHint hint) const
{
   Q_D(const QKdeTheme);

   switch (hint) {
      case QPlatformTheme::UseFullScreenForPopupMenu:
         return QVariant(true);

      case QPlatformTheme::DialogButtonBoxButtonsHaveIcons:
         return QVariant(true);

      case QPlatformTheme::DialogButtonBoxLayout:
         return QVariant(QPlatformDialogHelper::KdeLayout);

      case QPlatformTheme::ToolButtonStyle:
         return QVariant(d->toolButtonStyle);

      case QPlatformTheme::ToolBarIconSize:
         return QVariant(d->toolBarIconSize);

      case QPlatformTheme::SystemIconThemeName:
         return QVariant(d->iconThemeName);

      case QPlatformTheme::SystemIconFallbackThemeName:
         return QVariant(d->iconFallbackThemeName);

      case QPlatformTheme::IconThemeSearchPaths:
         return QVariant(d->kdeIconThemeSearchPaths(d->kdeDirs));

      case QPlatformTheme::StyleNames:
         return QVariant(d->styleNames);

      case QPlatformTheme::KeyboardScheme:
         return QVariant(int(KdeKeyboardScheme));

      case QPlatformTheme::ItemViewActivateItemOnSingleClick:
         return QVariant(d->singleClick);

      case QPlatformTheme::WheelScrollLines:
         return QVariant(d->wheelScrollLines);

      default:
         break;
   }

   return QPlatformTheme::themeHint(hint);
}

const QPalette *QKdeTheme::palette(Palette type) const
{
   Q_D(const QKdeTheme);
   return d->resources.palettes[type];
}

const QFont *QKdeTheme::font(Font type) const
{
   Q_D(const QKdeTheme);
   return d->resources.fonts[type];
}

QPlatformTheme *QKdeTheme::createKdeTheme()
{
   const QByteArray kdeVersionBA = qgetenv("KDE_SESSION_VERSION");
   const int kdeVersion = kdeVersionBA.toInt();
   if (kdeVersion < 4) {
      return nullptr;
   }

   if (kdeVersion > 4) {
      // Plasma 5 follows XDG spec, but uses the same config file format:
      return new QKdeTheme(QStandardPaths::standardLocations(QStandardPaths::GenericConfigLocation), kdeVersion);
   }

   // Determine KDE prefixes in the following priority order:
   // - KDEHOME and KDEDIRS environment variables
   // - ~/.kde(<version>)
   // - read prefixes from /etc/kde<version>rc
   // - fallback to /etc/kde<version>

   QStringList kdeDirs;
   const QString kdeHomePathVar = QFile::decodeName(qgetenv("KDEHOME"));
   if (! kdeHomePathVar.isEmpty()) {
      kdeDirs += kdeHomePathVar;
   }

   const QString kdeDirsVar = QFile::decodeName(qgetenv("KDEDIRS"));
   if (! kdeDirsVar.isEmpty()) {
      kdeDirs += kdeDirsVar.split(':', QStringParser::SkipEmptyParts);
   }

   const QString kdeVersionHomePath = QDir::homePath() + "/.kde" + kdeVersionBA;
   if (QFileInfo(kdeVersionHomePath).isDir()) {
      kdeDirs += kdeVersionHomePath;
   }

   const QString kdeHomePath = QDir::homePath() + "/.kde";
   if (QFileInfo(kdeHomePath).isDir()) {
      kdeDirs += kdeHomePath;
   }

   const QString kdeRcPath = "/etc/kde" + kdeVersionBA + "rc";

   if (QFileInfo(kdeRcPath).isReadable()) {
      QSettings kdeSettings(kdeRcPath, QSettings::IniFormat);
      kdeSettings.beginGroup("Directories-default");
      kdeDirs += kdeSettings.value("prefixes").toStringList();
   }

   const QString kdeVersionPrefix = "/etc/kde" + kdeVersionBA;
   if (QFileInfo(kdeVersionPrefix).isDir()) {
      kdeDirs += kdeVersionPrefix;
   }

   kdeDirs.removeDuplicates();
   if (kdeDirs.isEmpty()) {
      qWarning("Unable to determine KDE dirs");
      return nullptr;
   }

   return new QKdeTheme(kdeDirs, kdeVersion);
}

#if !defined(QT_NO_DBUS) && !defined(QT_NO_SYSTEMTRAYICON)
QPlatformSystemTrayIcon *QKdeTheme::createPlatformSystemTrayIcon() const
{
   if (isDBusTrayAvailable()) {
      return new QDBusTrayIcon();
   }

   return nullptr;
}
#endif

#endif // QT_NO_SETTINGS

QString QGnomeTheme::m_name = "gnome";

class QGnomeThemePrivate : public QPlatformThemePrivate
{
 public:
   QGnomeThemePrivate() : systemFont(nullptr), fixedFont(nullptr)
   {
   }

   ~QGnomeThemePrivate() {
      delete systemFont;
      delete fixedFont;
   }

   void configureFonts(const QString &gtkFontName) const {
      Q_ASSERT(!systemFont);

      const int split = gtkFontName.lastIndexOf(QChar(QChar::Space));
      float size = gtkFontName.mid(split + 1).toFloat();
      QString fontName = gtkFontName.left(split);

      systemFont = new QFont(fontName, size);
      fixedFont = new QFont("monospace", systemFont->pointSize());
      fixedFont->setStyleHint(QFont::TypeWriter);
   }

   mutable QFont *systemFont;
   mutable QFont *fixedFont;
};

QGnomeTheme::QGnomeTheme()
   : QPlatformTheme(new QGnomeThemePrivate())
{
}

QVariant QGnomeTheme::themeHint(QPlatformTheme::ThemeHint hint) const
{
   switch (hint) {
      case QPlatformTheme::DialogButtonBoxButtonsHaveIcons:
         return QVariant(true);

      case QPlatformTheme::DialogButtonBoxLayout:
         return QVariant(QPlatformDialogHelper::GnomeLayout);

      case QPlatformTheme::SystemIconThemeName:
         return QVariant(QString("Adwaita"));

      case QPlatformTheme::SystemIconFallbackThemeName:
         return QVariant(QString("gnome"));

      case QPlatformTheme::IconThemeSearchPaths:
         return QVariant(QGenericUnixTheme::xdgIconThemePaths());

      case QPlatformTheme::StyleNames: {
         QStringList styleNames;
         styleNames << "GTK+" << "fusion" << "windows";
         return QVariant(styleNames);
      }

      case QPlatformTheme::KeyboardScheme:
         return QVariant(int(GnomeKeyboardScheme));

      case QPlatformTheme::PasswordMaskCharacter:
         return QVariant(QChar(0x2022));

      default:
         break;
   }

   return QPlatformTheme::themeHint(hint);
}

const QFont *QGnomeTheme::font(Font type) const
{
   Q_D(const QGnomeTheme);

   if (!d->systemFont) {
      d->configureFonts(gtkFontName());
   }

   switch (type) {
      case QPlatformTheme::SystemFont:
         return d->systemFont;

      case QPlatformTheme::FixedFont:
         return d->fixedFont;

      default:
         return nullptr;
   }
}

QString QGnomeTheme::gtkFontName() const
{
   return QString("%1 %2").formatArg(defaultSystemFontNameC).formatArg(defaultSystemFontSize);
}

#if !defined(QT_NO_DBUS) && ! defined(QT_NO_SYSTEMTRAYICON)
QPlatformSystemTrayIcon *QGnomeTheme::createPlatformSystemTrayIcon() const
{
   if (isDBusTrayAvailable()) {
      return new QDBusTrayIcon();
   }

   return nullptr;
}
#endif

QString QGnomeTheme::standardButtonText(int button) const
{
   switch (button) {
      case QPlatformDialogHelper::Ok:
         return QCoreApplication::translate("QGnomeTheme", "&OK");

      case QPlatformDialogHelper::Save:
         return QCoreApplication::translate("QGnomeTheme", "&Save");

      case QPlatformDialogHelper::Cancel:
         return QCoreApplication::translate("QGnomeTheme", "&Cancel");

      case QPlatformDialogHelper::Close:
         return QCoreApplication::translate("QGnomeTheme", "&Close");

      case QPlatformDialogHelper::Discard:
         return QCoreApplication::translate("QGnomeTheme", "Close without Saving");

      default:
         break;
   }

   return QPlatformTheme::standardButtonText(button);
}


QPlatformTheme *QGenericUnixTheme::createUnixTheme(const QString &name)
{
   if (name == QGenericUnixTheme::m_name) {
      return new QGenericUnixTheme;
   }

#ifndef QT_NO_SETTINGS
   if (name == QKdeTheme::m_name)

      if (QPlatformTheme *kdeTheme = QKdeTheme::createKdeTheme()) {
         return kdeTheme;
      }
#endif

   if (name == QGnomeTheme::m_name) {
      return new QGnomeTheme;
   }

   return nullptr;
}

QStringList QGenericUnixTheme::themeNames()
{
   QStringList result;

   if (QApplication::desktopSettingsAware()) {
      const QByteArray desktopEnvironment = QApplicationPrivate::platformIntegration()->services()->desktopEnvironment();
      QList<QByteArray> gtkBasedEnvironments;

      gtkBasedEnvironments << "GNOME"
         << "X-CINNAMON"
         << "UNITY"
         << "MATE"
         << "XFCE"
         << "LXDE";

      QList<QByteArray> desktopNames = desktopEnvironment.split(':');

      for (const QByteArray &desktopName : desktopNames) {
         if (desktopEnvironment == "KDE") {

#ifndef QT_NO_SETTINGS
            result.push_back(QKdeTheme::m_name);
#endif
         } else if (gtkBasedEnvironments.contains(desktopName)) {
            // prefer the GTK2 theme implementation with native dialogs etc.
            result.push_back("gtk2");

            // fallback to the generic Gnome theme if loading the GTK2 theme fails
            result.push_back(QGnomeTheme::m_name);
         }
      }

      const QString session = QString::fromUtf8(qgetenv("DESKTOP_SESSION"));
      if (!session.isEmpty() && session != "default" && ! result.contains(session)) {
         result.push_back(session);
      }

   } // desktopSettingsAware

   result.append(QGenericUnixTheme::m_name);

   return result;
}
