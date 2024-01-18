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

#ifndef QLAYOUT_P_H
#define QLAYOUT_P_H

#include <qstyle.h>
#include <qsizepolicy.h>

class QWidgetItem;
class QSpacerItem;
class QLayoutItem;

class Q_GUI_EXPORT QLayoutPrivate
{
   Q_DECLARE_PUBLIC(QLayout)

 public:
   QLayoutPrivate();

   virtual ~QLayoutPrivate()
   {
   }

   void getMargin(int *result, int userMargin, QStyle::PixelMetric pm) const;
   void doResize(const QSize &);
   void reparentChildWidgets(QWidget *mw);
   bool checkWidget(QWidget *widget) const;
   bool checkLayout(QLayout *otherLayout) const;

   static QWidgetItem *createWidgetItem(const QLayout *layout, QWidget *widget);

   static QSpacerItem *createSpacerItem(const QLayout *layout, int w, int h,
      QSizePolicy::Policy hPolicy = QSizePolicy::Minimum, QSizePolicy::Policy vPolicy = QSizePolicy::Minimum);

   virtual QLayoutItem *replaceAt(int index, QLayoutItem *newitem) {
      (void) index;
      (void) newitem;

      return nullptr;
   }

   static QLayout::QWidgetItemFactory widgetItemFactory;
   static QLayout::QSpacerItemFactory spacerItemFactory;

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

#endif
