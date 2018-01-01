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

#ifndef QPIXMAPCACHE_H
#define QPIXMAPCACHE_H

#include <QtGui/qpixmap.h>

#ifdef Q_TEST_QPIXMAPCACHE
#include <QtCore/qpair.h>
#endif

QT_BEGIN_NAMESPACE

class Q_GUI_EXPORT QPixmapCache
{

 public:
   class KeyData;
   class Q_GUI_EXPORT Key
   {
    public:
      Key();
      Key(const Key &other);
      ~Key();
      bool operator ==(const Key &key) const;
      inline bool operator !=(const Key &key) const {
         return !operator==(key);
      }
      Key &operator =(const Key &other);

    private:
      KeyData *d;
      friend class QPMCache;
      friend class QPixmapCache;
   };

   static int cacheLimit();
   static void setCacheLimit(int);
   static QPixmap *find(const QString &key);
   static bool find(const QString &key, QPixmap &pixmap);
   static bool find(const QString &key, QPixmap *pixmap);
   static bool find(const Key &key, QPixmap *pixmap);
   static bool insert(const QString &key, const QPixmap &pixmap);
   static Key insert(const QPixmap &pixmap);
   static bool replace(const Key &key, const QPixmap &pixmap);
   static void remove(const QString &key);
   static void remove(const Key &key);
   static void clear();

#ifdef Q_TEST_QPIXMAPCACHE
   static void flushDetachedPixmaps();
   static int totalUsed();
   static QList< QPair<QString, QPixmap> > allPixmaps();
#endif
};

QT_END_NAMESPACE

#endif // QPIXMAPCACHE_H
