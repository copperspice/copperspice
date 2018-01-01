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

#ifndef QIconLoader_P_H
#define QIconLoader_P_H

#ifndef QT_NO_ICON

#include <QIcon>
#include <QIconEngine>
#include <QPixmapCache>
#include <qicon_p.h>
#include <qfactoryloader_p.h>
#include <QHash>

QT_BEGIN_NAMESPACE

class QIconLoader;

struct QIconDirInfo {
   enum Type { Fixed, Scalable, Threshold };

   QIconDirInfo(const QString &_path = QString()) :
      path(_path),
      size(0),
      maxSize(0),
      minSize(0),
      threshold(0),
      type(Threshold) {}
   QString path;
   short size;
   short maxSize;
   short minSize;
   short threshold;
   Type type : 4;
};

class QIconLoaderEngineEntry
{
 public:
   virtual ~QIconLoaderEngineEntry() {}
   virtual QPixmap pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state) = 0;
   QString filename;
   QIconDirInfo dir;
   static int count;
};

struct ScalableEntry : public QIconLoaderEngineEntry {
   QPixmap pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state) override;
   QIcon svgIcon;
};

struct PixmapEntry : public QIconLoaderEngineEntry {
   QPixmap pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state) override;
   QPixmap basePixmap;
};

typedef QList<QIconLoaderEngineEntry *> QThemeIconEntries;

class QIconLoaderEngine : public QIconEngineV2
{
 public:
   QIconLoaderEngine(const QString &iconName = QString());
   ~QIconLoaderEngine();

   void paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state) override;
   QPixmap pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state) override;
   QSize actualSize(const QSize &size, QIcon::Mode mode, QIcon::State state) override;
   QIconEngineV2 *clone() const override;
   bool read(QDataStream &in) override;
   bool write(QDataStream &out) const override;

 private:
   QString key() const override;
   bool hasIcon() const;
   void ensureLoaded();
   void virtual_hook(int id, void *data) override;
   QIconLoaderEngineEntry *entryForSize(const QSize &size);
   QIconLoaderEngine(const QIconLoaderEngine &other);
   QThemeIconEntries m_entries;
   QString m_iconName;
   uint m_key;

   friend class QIconLoader;
};

class QIconTheme
{
 public:
   QIconTheme(const QString &name);
   QIconTheme() : m_valid(false) {}

   QStringList parents() {
      return m_parents;
   }

   QList <QIconDirInfo> keyList() {
      return m_keyList;
   }

   QString contentDir() {
      return m_contentDir;
   }

   bool isValid() {
      return m_valid;
   }

 private:
   QString m_contentDir;
   QList <QIconDirInfo> m_keyList;
   QStringList m_parents;
   bool m_valid;
};

class QIconLoader : public QObject
{
 public:
   QIconLoader();
   QThemeIconEntries loadIcon(const QString &iconName) const;

   uint themeKey() const {
      return m_themeKey;
   }

   QString themeName() const {
      return m_userTheme.isEmpty() ? m_systemTheme : m_userTheme;
   }

   void setThemeName(const QString &themeName);
   QIconTheme theme() {
      return themeList.value(themeName());
   }

   void setThemeSearchPath(const QStringList &searchPaths);
   QStringList themeSearchPaths() const;
   QIconDirInfo dirInfo(int dirindex);
   static QIconLoader *instance();
   void updateSystemTheme();

   void invalidateKey() {
      m_themeKey++;
   }

   void ensureInitialized();

 private:
   QThemeIconEntries findIconHelper(const QString &themeName, const QString &iconName, QStringList &visited) const;
   uint m_themeKey;
   bool m_supportsSvg;
   bool m_initialized;

   mutable QString m_userTheme;
   mutable QString m_systemTheme;
   mutable QStringList m_iconDirs;
   mutable QHash <QString, QIconTheme> themeList;
};

QT_END_NAMESPACE

#endif // QDESKTOPICON_P_H

#endif //QT_NO_ICON
