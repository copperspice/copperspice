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

#include <qiconengine.h>
#include <qpainter.h>

QSize QIconEngine::actualSize(const QSize &size, QIcon::Mode, QIcon::State)
{
   return size;
}

QIconEngine::QIconEngine()
{
}

QIconEngine::~QIconEngine()
{
}

QPixmap QIconEngine::pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state)
{
   QPixmap pm(size);
   {
      QPainter p(&pm);
      paint(&p, QRect(QPoint(0, 0), size), mode, state);
   }

   return pm;
}

void QIconEngine::addPixmap(const QPixmap &pixmap, QIcon::Mode mode, QIcon::State state)
{
   (void) pixmap;
   (void) mode;
   (void) state;
}

void QIconEngine::addFile(const QString &fileName, const QSize &size, QIcon::Mode mode, QIcon::State state)
{
   (void) fileName;
   (void) size;
   (void) mode;
   (void) state;
}

QString QIconEngine::key() const
{
   return QString();
}

bool QIconEngine::read(QDataStream &)
{
   return false;
}

bool QIconEngine::write(QDataStream &) const
{
   return false;
}

void QIconEngine::virtual_hook(int id, void *data)
{
   switch (id) {
      case QIconEngine::AvailableSizesHook: {
         QIconEngine::AvailableSizesArgument &arg =
            *reinterpret_cast<QIconEngine::AvailableSizesArgument *>(data);
         arg.sizes.clear();
         break;
      }

      default:
         break;
   }
}

QList<QSize> QIconEngine::availableSizes(QIcon::Mode mode, QIcon::State state) const
{
   AvailableSizesArgument arg;
   arg.mode = mode;
   arg.state = state;
   const_cast<QIconEngine *>(this)->virtual_hook(QIconEngine::AvailableSizesHook, reinterpret_cast<void *>(&arg));

   return arg.sizes;
}

QString QIconEngine::iconName() const
{
   QString name;
   const_cast<QIconEngine *>(this)->virtual_hook(QIconEngine::IconNameHook, reinterpret_cast<void *>(&name));

   return name;
}
