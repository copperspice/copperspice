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

#ifndef QLAYOUT_P_H
#define QLAYOUT_P_H

#include <qstyle.h>
#include <qsizepolicy.h>

QT_BEGIN_NAMESPACE

class QWidgetItem;
class QSpacerItem;

class Q_GUI_EXPORT QLayoutPrivate
{
   Q_DECLARE_PUBLIC(QLayout)

 public:
   typedef QWidgetItem *(*QWidgetItemFactoryMethod)(const QLayout *layout, QWidget *widget);
   typedef QSpacerItem *(*QSpacerItemFactoryMethod)(const QLayout *layout, int w, int h, QSizePolicy::Policy hPolicy,
         QSizePolicy::Policy);

   QLayoutPrivate();
   virtual ~QLayoutPrivate() {}

   void getMargin(int *result, int userMargin, QStyle::PixelMetric pm) const;
   void doResize(const QSize &);
   void reparentChildWidgets(QWidget *mw);

   static QWidgetItem *createWidgetItem(const QLayout *layout, QWidget *widget);
   static QSpacerItem *createSpacerItem(const QLayout *layout, int w, int h,
                                        QSizePolicy::Policy hPolicy = QSizePolicy::Minimum, QSizePolicy::Policy vPolicy = QSizePolicy::Minimum);

   static QWidgetItemFactoryMethod widgetItemFactoryMethod;
   static QSpacerItemFactoryMethod spacerItemFactoryMethod;

   int insideSpacing;
   int userLeftMargin;
   int userTopMargin;
   int userRightMargin;
   int userBottomMargin;
   uint topLevel : 1;
   uint enabled : 1;
   uint activated : 1;
   uint autoNewChild : 1;
   QLayout::SizeConstraint constraint;
   QRect rect;
   QWidget *menubar;

 protected:
   QLayout *q_ptr;

};

QT_END_NAMESPACE

#endif // QLAYOUT_P_H
