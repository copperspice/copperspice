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

#ifndef QT_NO_ICON

#include <qiconloader_p.h>

#include <qiconengineplugin.h>
#include <qpixmapcache.h>
#include <qplatform_theme.h>
#include <qiconengine.h>
#include <qpalette.h>
#include <qlist.h>

#include <qdir.h>
#include <qsettings.h>
#include <qpainter.h>

#include <qapplication_p.h>
#include <qicon_p.h>
#include <qhexstring_p.h>

static QIconLoader *iconLoaderInstance()
{
   static QIconLoader retval;
   return &retval;
}

/* Theme to use in last resort, if the theme does not have the icon, neither the parents  */
static QString fallbackTheme()
{
   if (const QPlatformTheme *theme = QGuiApplicationPrivate::platformTheme()) {
      const QVariant themeHint = theme->themeHint(QPlatformTheme::SystemIconFallbackThemeName);
      if (themeHint.isValid()) {
         return themeHint.toString();
      }
   }

   return QString();
}

QIconLoader::QIconLoader() :
   m_themeKey(1), m_supportsSvg(false), m_initialized(false)
{
}

static inline QString systemThemeName()
{
   if (const QPlatformTheme *theme = QGuiApplicationPrivate::platformTheme()) {
      const QVariant themeHint = theme->themeHint(QPlatformTheme::SystemIconThemeName);
      if (themeHint.isValid()) {
         return themeHint.toString();
      }
   }
   return QString();
}

static inline QStringList systemIconSearchPaths()
{
   if (const QPlatformTheme *theme = QGuiApplicationPrivate::platformTheme()) {
      const QVariant themeHint = theme->themeHint(QPlatformTheme::IconThemeSearchPaths);
      if (themeHint.isValid()) {
         return themeHint.toStringList();
      }
   }
   return QStringList();
}

extern QFactoryLoader *cs_internal_iconLoader();    // qicon.cpp

void QIconLoader::ensureInitialized()
{
   if (! m_initialized) {
      m_initialized = true;
      Q_ASSERT(qApp);

      m_systemTheme = systemThemeName();
      if (m_systemTheme.isEmpty()) {
         m_systemTheme = fallbackTheme();
      }

      if (cs_internal_iconLoader()->keySet().contains("svg")) {
         m_supportsSvg = true;
      }
   }
}

QIconLoader *QIconLoader::instance()
{
   iconLoaderInstance()->ensureInitialized();
   return iconLoaderInstance();
}

// Queries the system theme and invalidates existing
// icons if the theme has changed.
void QIconLoader::updateSystemTheme()
{
   // Only change if this is not explicitly set by the user
   if (m_userTheme.isEmpty()) {
      QString theme = systemThemeName();
      if (theme.isEmpty()) {
         theme = fallbackTheme();
      }

      if (theme != m_systemTheme) {
         m_systemTheme = theme;
         invalidateKey();
      }
   }
}

void QIconLoader::setThemeName(const QString &themeName)
{
   m_userTheme = themeName;
   invalidateKey();
}

void QIconLoader::setThemeSearchPath(const QStringList &searchPaths)
{
   m_iconDirs = searchPaths;
   themeList.clear();
   invalidateKey();
}

QStringList QIconLoader::themeSearchPaths() const
{
   if (m_iconDirs.isEmpty()) {
      m_iconDirs = systemIconSearchPaths();

      // Always add resource directory as search path
      m_iconDirs.append(":/icons");
   }

   return m_iconDirs;
}

QIconTheme::QIconTheme(const QString &themeName)
   : m_valid(false)
{
   QFile themeIndex;

   QStringList iconDirs = QIcon::themeSearchPaths();

   for ( int i = 0 ; i < iconDirs.size() ; ++i) {
      QDir iconDir(iconDirs[i]);

      QString themeDir = iconDir.path() + QLatin1Char('/') + themeName;
      QFileInfo themeDirInfo(themeDir);

      if (themeDirInfo.isDir()) {
         m_contentDirs << themeDir;
      }

      if (!m_valid) {
         themeIndex.setFileName(themeDir + QLatin1String("/index.theme"));
         if (themeIndex.exists()) {
            m_valid = true;
         }
      }
   }

#ifndef QT_NO_SETTINGS
   if (themeIndex.exists()) {
      const QSettings indexReader(themeIndex.fileName(), QSettings::IniFormat);
      QStringListIterator keyIterator(indexReader.allKeys());
      while (keyIterator.hasNext()) {

         const QString key = keyIterator.next();
         if (key.endsWith(QLatin1String("/Size"))) {
            // Note the QSettings ini-format does not accept
            // slashes in key names, hence we have to cheat
            if (int size = indexReader.value(key).toInt()) {
               QString directoryKey = key.left(key.size() - 5);
               QIconDirInfo dirInfo(directoryKey);
               dirInfo.size = size;
               QString type = indexReader.value(directoryKey +
                     QLatin1String("/Type")
                  ).toString();

               if (type == QLatin1String("Fixed")) {
                  dirInfo.type = QIconDirInfo::Fixed;
               } else if (type == QLatin1String("Scalable")) {
                  dirInfo.type = QIconDirInfo::Scalable;
               } else {
                  dirInfo.type = QIconDirInfo::Threshold;
               }

               dirInfo.threshold = indexReader.value(directoryKey +
                     QLatin1String("/Threshold"),
                     2).toInt();

               dirInfo.minSize = indexReader.value(directoryKey +
                     QLatin1String("/MinSize"),
                     size).toInt();

               dirInfo.maxSize = indexReader.value(directoryKey +
                     QLatin1String("/MaxSize"),
                     size).toInt();
               m_keyList.append(dirInfo);
            }
         }
      }

      // Parent themes provide fallbacks for missing icons
      m_parents = indexReader.value(
            QLatin1String("Icon Theme/Inherits")).toStringList();

      m_parents.removeAll(QString());

      // Ensure a default platform fallback for all themes
      if (m_parents.isEmpty()) {
         const QString fallback = fallbackTheme();

         if (! fallback.isEmpty()) {
            m_parents.append(fallback);
         }
      }

      // Ensure that all themes fall back to hicolor
      if (! m_parents.contains(QLatin1String("hicolor"))) {
         m_parents.append(QLatin1String("hicolor"));
      }
   }
#endif //QT_NO_SETTINGS
}

QThemeIconInfo QIconLoader::findIconHelper(const QString &themeName,
   const QString &iconName,
   QStringList &visited) const
{
   QThemeIconInfo info;
   Q_ASSERT(!themeName.isEmpty());

   // Used to protect against potential recursions
   visited << themeName;

   QIconTheme theme = themeList.value(themeName);
   if (!theme.isValid()) {
      theme = QIconTheme(themeName);
      if (!theme.isValid()) {
         theme = QIconTheme(fallbackTheme());
      }

      themeList.insert(themeName, theme);
   }

   const QStringList contentDirs = theme.contentDirs();
   const QVector<QIconDirInfo> subDirs = theme.keyList();

   QString iconNameFallback = iconName;
   // Iterate through all icon's fallbacks in current theme
   while (info.entries.isEmpty()) {
      const QString svgIconName = iconNameFallback + ".svg";
      const QString pngIconName = iconNameFallback + ".png";

      // Add all relevant files
      for (int i = 0; i < contentDirs.size(); ++i) {
         QString contentDir = contentDirs.at(i) + '/';

         for (int j = 0; j < subDirs.size() ; ++j) {

            const QIconDirInfo &dirInfo = subDirs.at(j);
            QString subdir = dirInfo.path;
            QDir currentDir(contentDir + subdir);

            if (currentDir.exists(pngIconName)) {
               PixmapEntry *iconEntry = new PixmapEntry;
               iconEntry->dir = dirInfo;
               iconEntry->filename = currentDir.filePath(pngIconName);
               // Notice we ensure that pixmap entries always come before
               // scalable to preserve search order afterwards
               info.entries.prepend(iconEntry);

            } else if (m_supportsSvg &&
               currentDir.exists(svgIconName)) {
               ScalableEntry *iconEntry = new ScalableEntry;
               iconEntry->dir = dirInfo;
               iconEntry->filename = currentDir.filePath(svgIconName);
               info.entries.append(iconEntry);
            }
         }
      }

      if (!info.entries.isEmpty()) {
         info.iconName = iconNameFallback;
         break;
      }
      // If it's possible - find next fallback for the icon
      const int indexOfDash = iconNameFallback.lastIndexOf(QLatin1Char('-'));
      if (indexOfDash == -1) {
         break;
      }

      iconNameFallback.truncate(indexOfDash);
   }
   if (info.entries.isEmpty()) {
      const QStringList parents = theme.parents();
      // Search recursively through inherited themes
      for (int i = 0 ; i < parents.size() ; ++i) {

         const QString parentTheme = parents.at(i).trimmed();

         if (!visited.contains(parentTheme)) {
            // guard against recursion
            info = findIconHelper(parentTheme, iconName, visited);
         }

         if (! info.entries.isEmpty()) {
            // success
            break;
         }
      }
   }

   return info;
}

QThemeIconInfo QIconLoader::loadIcon(const QString &name) const
{
   if (!themeName().isEmpty()) {
      QStringList visited;
      return findIconHelper(themeName(), name, visited);
   }

   return QThemeIconInfo();
}


// Icon Loader Engine

QIconLoaderEngine::QIconLoaderEngine(const QString &iconName)
   : m_iconName(iconName), m_key(0)
{
}

QIconLoaderEngine::~QIconLoaderEngine()
{
   qDeleteAll(m_info.entries);
}

QIconLoaderEngine::QIconLoaderEngine(const QIconLoaderEngine &other)
   : QIconEngine(other),
     m_iconName(other.m_iconName),
     m_key(0)
{
}

QIconEngine *QIconLoaderEngine::clone() const
{
   return new QIconLoaderEngine(*this);
}

bool QIconLoaderEngine::read(QDataStream &in)
{
   in >> m_iconName;
   return true;
}

bool QIconLoaderEngine::write(QDataStream &out) const
{
   out << m_iconName;
   return true;
}

bool QIconLoaderEngine::hasIcon() const
{
   return !(m_info.entries.isEmpty());
}

// Lazily load the icon
void QIconLoaderEngine::ensureLoaded()
{

   if (!(QIconLoader::instance()->themeKey() == m_key)) {
      qDeleteAll(m_info.entries);
      m_info.entries.clear();
      m_info.iconName.clear();

      Q_ASSERT(m_info.entries.size() == 0);
      m_info = QIconLoader::instance()->loadIcon(m_iconName);
      m_key = QIconLoader::instance()->themeKey();
   }
}

void QIconLoaderEngine::paint(QPainter *painter, const QRect &rect,
   QIcon::Mode mode, QIcon::State state)
{
   QSize pixmapSize = rect.size();

   painter->drawPixmap(rect, pixmap(pixmapSize, mode, state));
}

/*
 * This algorithm is defined by the freedesktop spec:
 * http://standards.freedesktop.org/icon-theme-spec/icon-theme-spec-latest.html
 */
static bool directoryMatchesSize(const QIconDirInfo &dir, int iconsize)
{
   if (dir.type == QIconDirInfo::Fixed) {
      return dir.size == iconsize;

   } else if (dir.type == QIconDirInfo::Scalable) {
      return iconsize <= dir.maxSize &&
         iconsize >= dir.minSize;

   } else if (dir.type == QIconDirInfo::Threshold) {
      return iconsize >= dir.size - dir.threshold &&
         iconsize <= dir.size + dir.threshold;
   }

   Q_ASSERT(1); // Not a valid value
   return false;
}

/*
 * This algorithm is defined by the freedesktop spec:
 * http://standards.freedesktop.org/icon-theme-spec/icon-theme-spec-latest.html
 */
static int directorySizeDistance(const QIconDirInfo &dir, int iconsize)
{
   if (dir.type == QIconDirInfo::Fixed) {
      return qAbs(dir.size - iconsize);

   } else if (dir.type == QIconDirInfo::Scalable) {
      if (iconsize < dir.minSize) {
         return dir.minSize - iconsize;
      } else if (iconsize > dir.maxSize) {
         return iconsize - dir.maxSize;
      } else {
         return 0;
      }

   } else if (dir.type == QIconDirInfo::Threshold) {
      if (iconsize < dir.size - dir.threshold) {
         return dir.minSize - iconsize;

      } else if (iconsize > dir.size + dir.threshold) {
         return iconsize - dir.maxSize;

      } else {
         return 0;
      }
   }

   Q_ASSERT(1); // Not a valid value
   return INT_MAX;
}

QIconLoaderEngineEntry *QIconLoaderEngine::entryForSize(const QSize &size)
{
   int iconsize = qMin(size.width(), size.height());

   // Note that m_entries are sorted so that png-files
   // come first

   const int numEntries = m_info.entries.size();
   // Search for exact matches first
   for (int i = 0; i < numEntries; ++i) {
      QIconLoaderEngineEntry *entry = m_info.entries.at(i);
      if (directoryMatchesSize(entry->dir, iconsize)) {
         return entry;
      }
   }

   // Find the minimum distance icon
   int minimalSize = INT_MAX;
   QIconLoaderEngineEntry *closestMatch = nullptr;
   for (int i = 0; i < numEntries; ++i) {
      QIconLoaderEngineEntry *entry = m_info.entries.at(i);
      int distance = directorySizeDistance(entry->dir, iconsize);
      if (distance < minimalSize) {
         minimalSize  = distance;
         closestMatch = entry;
      }
   }
   return closestMatch;
}


QSize QIconLoaderEngine::actualSize(const QSize &size, QIcon::Mode mode, QIcon::State state)
{
   ensureLoaded();

   QIconLoaderEngineEntry *entry = entryForSize(size);
   if (entry) {
      const QIconDirInfo &dir = entry->dir;
      if (dir.type == QIconDirInfo::Scalable) {
         return size;

      } else {
         int result = qMin(dir.size, qMin(size.width(), size.height()));
         return QSize(result, result);
      }
   }
   return QIconEngine::actualSize(size, mode, state);
}

QPixmap PixmapEntry::pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state)
{
   (void) state;

   // Ensure that basePixmap is lazily initialized before generating the
   // key, otherwise the cache key is not unique
   if (basePixmap.isNull()) {
      basePixmap.load(filename);
   }

   QSize actualSize = basePixmap.size();
   if (!actualSize.isNull() && (actualSize.width() > size.width() || actualSize.height() > size.height())) {
      actualSize.scale(size, Qt::KeepAspectRatio);
   }

   QString key = "$cs_theme_"
      + HexString<qint64>(basePixmap.cacheKey())
      + HexString<int>(mode)
      + HexString<qint64>(QGuiApplication::palette().cacheKey())
      + HexString<int>(actualSize.width())
      + HexString<int>(actualSize.height());

   QPixmap cachedPixmap;
   if (QPixmapCache::find(key, &cachedPixmap)) {
      return cachedPixmap;

   } else {
      if (basePixmap.size() != actualSize) {
         cachedPixmap = basePixmap.scaled(actualSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
      } else {
         cachedPixmap = basePixmap;
      }

      if (QGuiApplication *guiApp = qobject_cast<QGuiApplication *>(qApp)) {
         cachedPixmap = guiApp->cs_internal_applyQIconStyle(mode, cachedPixmap);
      }

      QPixmapCache::insert(key, cachedPixmap);
   }

   return cachedPixmap;
}

QPixmap ScalableEntry::pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state)
{
   if (svgIcon.isNull()) {
      svgIcon = QIcon(filename);
   }

   // Simply reuse svg icon engine
   return svgIcon.pixmap(size, mode, state);
}

QPixmap QIconLoaderEngine::pixmap(const QSize &size, QIcon::Mode mode,
   QIcon::State state)
{
   ensureLoaded();

   QIconLoaderEngineEntry *entry = entryForSize(size);
   if (entry) {
      return entry->pixmap(size, mode, state);
   }

   return QPixmap();
}

QString QIconLoaderEngine::key() const
{
   return QLatin1String("QIconLoaderEngine");
}

void QIconLoaderEngine::virtual_hook(int id, void *data)
{
   ensureLoaded();

   switch (id) {
      case QIconEngine::AvailableSizesHook: {
         QIconEngine::AvailableSizesArgument &arg
            = *reinterpret_cast<QIconEngine::AvailableSizesArgument *>(data);

         const int infoSize = m_info.entries.size();
         QList<QSize> sizes;

         // Gets all sizes from the DirectoryInfo entries
         for (int i = 0 ; i < infoSize ; ++i) {
            int size = m_info.entries.at(i)->dir.size;
            sizes.append(QSize(size, size));
         }

         arg.sizes.swap(sizes); // commit
      }

      break;
      case QIconEngine::IconNameHook: {
         QString &name = *reinterpret_cast<QString *>(data);
         name = m_iconName;
      }
      break;
      default:
         QIconEngine::virtual_hook(id, data);
   }
}


#endif //QT_NO_ICON
