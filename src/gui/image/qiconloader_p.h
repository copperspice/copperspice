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

#ifndef QIconLoader_P_H
#define QIconLoader_P_H

#include <qglobal.h>

#ifndef QT_NO_ICON

#include <qicon.h>
#include <qiconengine.h>
#include <qpixmapcache.h>
#include <qhash.h>
#include <qvector.h>

#include <qicon_p.h>
#include <qfactoryloader_p.h>

class QIconLoader;

struct QIconDirInfo {
   enum Type {
      Fixed,
      Scalable,
      Threshold
   };

   QIconDirInfo(const QString &_path = QString())
      : path(_path), size(0), maxSize(0), minSize(0), threshold(0), type(Threshold)
   { }

   QString path;
   short size;
   short maxSize;
   short minSize;
   short threshold;
   Type type;
};

class QIconLoaderEngineEntry
{
 public:
   virtual ~QIconLoaderEngineEntry() {}
   virtual QPixmap pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state) = 0;
   QString filename;
   QIconDirInfo dir;
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

struct QThemeIconInfo {
   QThemeIconEntries entries;
   QString iconName;
};
class QIconLoaderEngine : public QIconEngine
{
 public:
   QIconLoaderEngine(const QString &iconName = QString());
   ~QIconLoaderEngine();

   void paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state) override;
   QPixmap pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state) override;
   QSize actualSize(const QSize &size, QIcon::Mode mode, QIcon::State state) override;
   QIconEngine *clone() const override;
   bool read(QDataStream &in) override;
   bool write(QDataStream &out) const override;

 private:
   QString key() const override;
   bool hasIcon() const;
   void ensureLoaded();
   void virtual_hook(int id, void *data) override;
   QIconLoaderEngineEntry *entryForSize(const QSize &size);
   QIconLoaderEngine(const QIconLoaderEngine &other);
   QThemeIconInfo m_info;
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

   QVector<QIconDirInfo> keyList() {
      return m_keyList;
   }

   QStringList contentDirs() {
      return m_contentDirs;
   }

   bool isValid() {
      return m_valid;
   }

 private:
   QStringList m_contentDirs;
   QVector <QIconDirInfo> m_keyList;
   QStringList m_parents;
   bool m_valid;
};

class QIconLoader
{
 public:
   QIconLoader();
   QThemeIconInfo loadIcon(const QString &iconName) const;

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
   bool hasUserTheme() const {
      return !m_userTheme.isEmpty();
   }

 private:
   QThemeIconInfo findIconHelper(const QString &themeName, const QString &iconName, QStringList &visited) const;
   uint m_themeKey;
   bool m_supportsSvg;
   bool m_initialized;

   mutable QString m_userTheme;
   mutable QString m_systemTheme;
   mutable QStringList m_iconDirs;
   mutable QHash <QString, QIconTheme> themeList;
};

#endif // QDESKTOPICON_P_H

#endif
