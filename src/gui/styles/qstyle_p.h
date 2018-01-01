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

#ifndef QSTYLE_P_H
#define QSTYLE_P_H

#include <QtGui/qstyle.h>

QT_BEGIN_NAMESPACE

class QStyle;

class QStylePrivate
{
   Q_DECLARE_PUBLIC(QStyle)

 public:
   inline QStylePrivate()
      : layoutSpacingIndex(-1), proxyStyle(0) {}

   virtual ~QStylePrivate() {}

   mutable int layoutSpacingIndex;
   QStyle *proxyStyle;

 protected:
   QStyle *q_ptr;
};


#define BEGIN_STYLE_PIXMAPCACHE(a) \
    QRect rect = option->rect; \
    QPixmap internalPixmapCache; \
    QImage imageCache; \
    QPainter *p = painter; \
    QString unique = QStyleHelper::uniqueName((a), option, option->rect.size()); \
    int txType = painter->deviceTransform().type() | painter->worldTransform().type(); \
    bool doPixmapCache = txType <= QTransform::TxTranslate; \
    if (doPixmapCache && QPixmapCache::find(unique, internalPixmapCache)) { \
        painter->drawPixmap(option->rect.topLeft(), internalPixmapCache); \
    } else { \
        if (doPixmapCache) { \
            rect.setRect(0, 0, option->rect.width(), option->rect.height()); \
            imageCache = QImage(option->rect.size(), QImage::Format_ARGB32_Premultiplied); \
            imageCache.fill(0); \
            p = new QPainter(&imageCache); \
        }


#define END_STYLE_PIXMAPCACHE \
        if (doPixmapCache) { \
            p->end(); \
            delete p; \
            internalPixmapCache = QPixmap::fromImage(imageCache); \
            painter->drawPixmap(option->rect.topLeft(), internalPixmapCache); \
            QPixmapCache::insert(unique, internalPixmapCache); \
        } \
    }

QT_END_NAMESPACE

#endif //QSTYLE_P_H
