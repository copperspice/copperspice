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

#ifndef QICONENGINE_H
#define QICONENGINE_H

#include <QtCore/qglobal.h>
#include <QtCore/qlist.h>
#include <QtGui/qicon.h>

QT_BEGIN_NAMESPACE

class Q_GUI_EXPORT QIconEngine
{

 public:
   virtual ~QIconEngine();
   virtual void paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state) = 0;
   virtual QSize actualSize(const QSize &size, QIcon::Mode mode, QIcon::State state);
   virtual QPixmap pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state);

   virtual void addPixmap(const QPixmap &pixmap, QIcon::Mode mode, QIcon::State state);
   virtual void addFile(const QString &fileName, const QSize &size, QIcon::Mode mode, QIcon::State state);
};

// ### Qt5/move the below into QIconEngine
class Q_GUI_EXPORT QIconEngineV2 : public QIconEngine
{

 public:
   virtual QString key() const;
   virtual QIconEngineV2 *clone() const;
   virtual bool read(QDataStream &in);
   virtual bool write(QDataStream &out) const;
   virtual void virtual_hook(int id, void *data);

   enum IconEngineHook { AvailableSizesHook = 1, IconNameHook };

   struct AvailableSizesArgument {
      QIcon::Mode mode;
      QIcon::State state;
      QList<QSize> sizes;
   };

   // ### Qt5/make this function const and virtual.
   QList<QSize> availableSizes(QIcon::Mode mode = QIcon::Normal, QIcon::State state = QIcon::Off);

   // ### Qt5/make this function const and virtual.
   QString iconName();
};

QT_END_NAMESPACE

#endif // QICONENGINE_H
