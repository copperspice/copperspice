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

#ifndef QICON_H
#define QICON_H

#include <QtCore/qglobal.h>
#include <QtCore/qsize.h>
#include <QtCore/qlist.h>
#include <QtGui/qpixmap.h>

QT_BEGIN_NAMESPACE

class QIconPrivate;
class QIconEngine;
class QIconEngineV2;

class Q_GUI_EXPORT QIcon
{

 public:
   enum Mode { Normal, Disabled, Active, Selected };
   enum State { On, Off };

   QIcon();
   QIcon(const QPixmap &pixmap);
   QIcon(const QIcon &other);
   explicit QIcon(const QString &fileName); // file or resource name
   explicit QIcon(QIconEngine *engine);
   explicit QIcon(QIconEngineV2 *engine);
   ~QIcon();
   QIcon &operator=(const QIcon &other);

   inline QIcon &operator=(QIcon && other) {
      qSwap(d, other.d);
      return *this;
   }

   inline void swap(QIcon &other) {
      qSwap(d, other.d);
   }

   operator QVariant() const;

   QPixmap pixmap(const QSize &size, Mode mode = Normal, State state = Off) const;
   inline QPixmap pixmap(int w, int h, Mode mode = Normal, State state = Off) const {
      return pixmap(QSize(w, h), mode, state);
   }
   inline QPixmap pixmap(int extent, Mode mode = Normal, State state = Off) const {
      return pixmap(QSize(extent, extent), mode, state);
   }

   QSize actualSize(const QSize &size, Mode mode = Normal, State state = Off) const;

   QString name() const;

   void paint(QPainter *painter, const QRect &rect, Qt::Alignment alignment = Qt::AlignCenter, Mode mode = Normal,
              State state = Off) const;

   inline void paint(QPainter *painter, int x, int y, int w, int h,
                     Qt::Alignment alignment = Qt::AlignCenter, Mode mode = Normal, State state = Off) const {
      paint(painter, QRect(x, y, w, h), alignment, mode, state);
   }

   bool isNull() const;
   bool isDetached() const;
   void detach();

   int serialNumber() const;
   qint64 cacheKey() const;

   void addPixmap(const QPixmap &pixmap, Mode mode = Normal, State state = Off);
   void addFile(const QString &fileName, const QSize &size = QSize(), Mode mode = Normal, State state = Off);

   QList<QSize> availableSizes(Mode mode = Normal, State state = Off) const;

   static QIcon fromTheme(const QString &name, const QIcon &fallback = QIcon());
   static bool hasThemeIcon(const QString &name);

   static QStringList themeSearchPaths();
   static void setThemeSearchPaths(const QStringList &searchpath);

   static QString themeName();
   static void setThemeName(const QString &path);
   
   typedef QIconPrivate *DataPtr;
   inline DataPtr &data_ptr() {
      return d;
   }

 private:
   QIconPrivate *d;

#if !defined(QT_NO_DATASTREAM)
   friend Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QIcon &);
   friend Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QIcon &);
#endif

};

Q_DECLARE_SHARED(QIcon)
Q_DECLARE_TYPEINFO(QIcon, Q_MOVABLE_TYPE);

#if !defined(QT_NO_DATASTREAM)
Q_GUI_EXPORT QDataStream &operator<<(QDataStream &, const QIcon &);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &, QIcon &);
#endif

QT_END_NAMESPACE

#endif // QICON_H
