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

#ifndef QICON_P_H
#define QICON_P_H

#include <qglobal.h>
#include <qsize.h>
#include <qlist.h>
#include <qpixmap.h>
#include <qicon.h>
#include <qiconengine.h>

#ifndef QT_NO_ICON


class QIconPrivate
{
 public:
   QIconPrivate();

   ~QIconPrivate() {
      delete engine;
   }
   qreal pixmapDevicePixelRatio(qreal displayDevicePixelRatio, const QSize &requestedSize, const QSize &actualSize);

   QIconEngine *engine;

   QAtomicInt ref;
   int serialNum;
   int detach_no;

   bool is_mask;
};

struct QPixmapIconEngineEntry {
   QPixmapIconEngineEntry(): mode(QIcon::Normal), state(QIcon::Off) {}
   QPixmapIconEngineEntry(const QPixmap &pm, QIcon::Mode m = QIcon::Normal, QIcon::State s = QIcon::Off)
      : pixmap(pm), size(pm.size()), mode(m), state(s) {}

   QPixmapIconEngineEntry(const QString &file, const QSize &sz = QSize(), QIcon::Mode m = QIcon::Normal,
      QIcon::State s = QIcon::Off)

      : fileName(file), size(sz), mode(m), state(s) {}

   QPixmapIconEngineEntry(const QString &file, const QImage &image, QIcon::Mode m = QIcon::Normal, QIcon::State s = QIcon::Off);

   QPixmap pixmap;
   QString fileName;
   QSize size;
   QIcon::Mode mode;
   QIcon::State state;
   bool isNull() const {
      return (fileName.isEmpty() && pixmap.isNull());
   }
};

inline QPixmapIconEngineEntry::QPixmapIconEngineEntry(const QString &file, const QImage &image, QIcon::Mode m, QIcon::State s)
   : fileName(file), size(image.size()), mode(m), state(s)
{
   pixmap.convertFromImage(image);
   // Reset the devicePixelRatio. The pixmap may be loaded from a @2x file,
   // but be used as a 1x pixmap by QIcon.
   pixmap.setDevicePixelRatio(1.0);
}

class Q_GUI_EXPORT QPixmapIconEngine : public QIconEngine
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

   QString key() const override;
   QIconEngine *clone() const override;
   bool read(QDataStream &in) override;
   bool write(QDataStream &out) const override;
   void virtual_hook(int id, void *data) override;

 private:
   QPixmapIconEngineEntry *tryMatch(const QSize &size, QIcon::Mode mode, QIcon::State state);
   QVector<QPixmapIconEngineEntry> pixmaps;

   friend Q_GUI_EXPORT QDataStream &operator<<(QDataStream &stream, const QIcon &icon);
   friend class QIconThemeEngine;
};

#endif //QT_NO_ICON

#endif
