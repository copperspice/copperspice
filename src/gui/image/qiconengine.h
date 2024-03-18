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

#ifndef QICONENGINE_H
#define QICONENGINE_H

#include <qglobal.h>
#include <qlist.h>
#include <qicon.h>


class Q_GUI_EXPORT QIconEngine
{
 public:
   enum IconEngineHook {
      AvailableSizesHook = 1,
      IconNameHook
   };

   QIconEngine();

   virtual ~QIconEngine();
   virtual void paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state) = 0;
   virtual QSize actualSize(const QSize &size, QIcon::Mode mode, QIcon::State state);
   virtual QPixmap pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state);

   virtual void addPixmap(const QPixmap &pixmap, QIcon::Mode mode, QIcon::State state);
   virtual void addFile(const QString &fileName, const QSize &size, QIcon::Mode mode, QIcon::State state);

   virtual QString key() const;
   virtual QIconEngine *clone() const = 0;
   virtual bool read(QDataStream &in);
   virtual bool write(QDataStream &out) const;

   struct AvailableSizesArgument {
      QIcon::Mode mode;
      QIcon::State state;
      QList<QSize> sizes;
   };

   virtual QList<QSize> availableSizes(QIcon::Mode mode = QIcon::Normal, QIcon::State state = QIcon::Off) const;
   virtual QString iconName() const;
   virtual void virtual_hook(int id, void *data);
};



#endif
