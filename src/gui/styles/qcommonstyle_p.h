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

#ifndef QCOMMONSTYLE_P_H
#define QCOMMONSTYLE_P_H

#include <qcommonstyle.h>
#include <qstyle_p.h>
#include <qstyleoption.h>

QT_BEGIN_NAMESPACE

class QStringList;

class QCommonStylePrivate : public QStylePrivate
{
   Q_DECLARE_PUBLIC(QCommonStyle)

 public:
   inline QCommonStylePrivate()
#ifndef QT_NO_ITEMVIEWS
      : cachedOption(0)
#endif
   { }

#ifndef QT_NO_ITEMVIEWS
   ~QCommonStylePrivate() {
      delete cachedOption;
   }
   void viewItemDrawText(QPainter *p, const QStyleOptionViewItemV4 *option, const QRect &rect) const;
   void viewItemLayout(const QStyleOptionViewItemV4 *opt,  QRect *checkRect, QRect *pixmapRect, QRect *textRect, bool sizehint) const;
   QSize viewItemSize(const QStyleOptionViewItemV4 *option, int role) const;

   mutable QRect decorationRect, displayRect, checkRect;
   mutable QStyleOptionViewItemV4 *cachedOption;

   bool isViewItemCached(const QStyleOptionViewItemV4 &option) const {
      return cachedOption && (option.rect == cachedOption->rect
                              && option.direction == cachedOption->direction
                              && option.state == cachedOption->state
                              && option.displayAlignment == cachedOption->displayAlignment
                              && option.decorationAlignment == cachedOption->decorationAlignment
                              && option.decorationPosition == cachedOption->decorationPosition
                              && option.decorationSize == cachedOption->decorationSize
                              && option.font == cachedOption->font
                              && option.features == cachedOption->features
                              && option.widget == cachedOption->widget
                              && option.index == cachedOption->index
                              && option.icon.isNull() == cachedOption->icon.isNull()
                              && option.text == cachedOption->text
                              && option.viewItemPosition == cachedOption->viewItemPosition);
   }
#endif
   mutable QIcon tabBarcloseButtonIcon;

#ifndef QT_NO_TABBAR
   void tabLayout(const QStyleOptionTabV3 *opt, const QWidget *widget, QRect *textRect, QRect *pixmapRect) const;
#endif
};

QT_END_NAMESPACE

#endif //QCOMMONSTYLE_P_H
