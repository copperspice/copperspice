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

#ifndef QICON_P_H
#define QICON_P_H

#include <QtCore/qglobal.h>
#include <QtCore/qsize.h>
#include <QtCore/qlist.h>
#include <QtGui/qpixmap.h>
#include <QtGui/qicon.h>
#include <QtGui/qiconengine.h>

#ifndef QT_NO_ICON
QT_BEGIN_NAMESPACE

class QIconPrivate
{
 public:
   QIconPrivate();

   ~QIconPrivate() {
      if (engine_version == 1) {
         if (!v1RefCount->deref()) {
            delete engine;
            delete v1RefCount;
         }
      } else if (engine_version == 2) {
         delete engine;
      }
   }

   QIconEngine *engine;

   QAtomicInt ref;
   int serialNum;
   int detach_no;
   int engine_version;

   QAtomicInt *v1RefCount;
};


struct QPixmapIconEngineEntry {
   QPixmapIconEngineEntry(): mode(QIcon::Normal), state(QIcon::Off) {}
   QPixmapIconEngineEntry(const QPixmap &pm, QIcon::Mode m = QIcon::Normal, QIcon::State s = QIcon::Off)
      : pixmap(pm), size(pm.size()), mode(m), state(s) {}

   QPixmapIconEngineEntry(const QString &file, const QSize &sz = QSize(), QIcon::Mode m = QIcon::Normal,
                          QIcon::State s = QIcon::Off)

      : fileName(file), size(sz), mode(m), state(s) {}
   QPixmap pixmap;
   QString fileName;
   QSize size;
   QIcon::Mode mode;
   QIcon::State state;
   bool isNull() const {
      return (fileName.isEmpty() && pixmap.isNull());
   }
};

class QPixmapIconEngine : public QIconEngineV2
{
 public:
   QPixmapIconEngine();
   QPixmapIconEngine(const QPixmapIconEngine &);
   ~QPixmapIconEngine();

   void paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state) override;
   QPixmap pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state) override;
   QPixmapIconEngineEntry *bestMatch(const QSize &size, QIcon::Mode mode, QIcon::State state, bool sizeOnly);
   QSize actualSize(const QSize &size, QIcon::Mode mode, QIcon::State state) override;
   void addPixmap(const QPixmap &pixmap, QIcon::Mode mode, QIcon::State state) override;
   void addFile(const QString &fileName, const QSize &size, QIcon::Mode mode, QIcon::State state) override;

   // v2 functions
   QString key() const override;
   QIconEngineV2 *clone() const override;
   bool read(QDataStream &in) override;
   bool write(QDataStream &out) const override;
   void virtual_hook(int id, void *data) override;

 private:
   QPixmapIconEngineEntry *tryMatch(const QSize &size, QIcon::Mode mode, QIcon::State state);
   QVector<QPixmapIconEngineEntry> pixmaps;

   friend QDataStream &operator<<(QDataStream &s, const QIcon &icon);
   friend class QIconThemeEngine;
};

QT_END_NAMESPACE
#endif //QT_NO_ICON
#endif // QICON_P_H
