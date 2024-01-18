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

#ifndef QCOMMONSTYLE_P_H
#define QCOMMONSTYLE_P_H

#include <qalgorithms.h>
#include <qcommonstyle.h>
#include <qstyle_p.h>
#include <qstyleanimation_p.h>
#include <qstyleoption.h>

class QStringList;

class QCommonStylePrivate : public QStylePrivate
{
   Q_DECLARE_PUBLIC(QCommonStyle)

 public:
   QCommonStylePrivate()

#ifndef QT_NO_ITEMVIEWS
      : cachedOption(nullptr), animationFps(30)
#else
      : animationFps(30)
#endif
   { }

   ~QCommonStylePrivate() {
#ifndef QT_NO_ANIMATION
      qDeleteAll(animations);
#endif

#ifndef QT_NO_ITEMVIEWS
      delete cachedOption;
#endif
   }

#ifndef QT_NO_ITEMVIEWS
   void viewItemDrawText(QPainter *painter, const QStyleOptionViewItem *option, const QRect &rect) const;
   void viewItemLayout(const QStyleOptionViewItem *option,  QRect *checkRect,
      QRect *pixmapRect, QRect *textRect, bool sizehint) const;

   QSize viewItemSize(const QStyleOptionViewItem *option, int role) const;

   mutable QRect decorationRect, displayRect, checkRect;
   mutable QStyleOptionViewItem *cachedOption;

   bool isViewItemCached(const QStyleOptionViewItem &option) const {
      return cachedOption && (option.widget == cachedOption->widget
            && option.index == cachedOption->index
            && option.state == cachedOption->state
            && option.rect == cachedOption->rect
            && option.text == cachedOption->text
            && option.direction == cachedOption->direction
            && option.displayAlignment == cachedOption->displayAlignment
            && option.decorationAlignment == cachedOption->decorationAlignment
            && option.decorationPosition == cachedOption->decorationPosition
            && option.decorationSize == cachedOption->decorationSize
            && option.features == cachedOption->features
            && option.icon.isNull() == cachedOption->icon.isNull()
            && option.font == cachedOption->font
            && option.viewItemPosition == cachedOption->viewItemPosition);
   }
#endif
   mutable QIcon tabBarcloseButtonIcon;

#ifndef QT_NO_TABBAR
   void tabLayout(const QStyleOptionTab *option, const QWidget *widget, QRect *textRect, QRect *pixmapRect) const;
#endif

   int animationFps;

#ifndef QT_NO_ANIMATION
   void _q_removeAnimation(QObject *obj);

   QList<const QObject *> animationKeys() const;
   QStyleAnimation *animationValue(const QObject *target) const;

   void startAnimation(QStyleAnimation *animation) const;
   void stopAnimation(const QObject *target) const;

 private:
   mutable QHash<const QObject *, QStyleAnimation *> animations;
#endif

};

#endif
