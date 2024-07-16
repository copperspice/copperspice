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

#ifndef QGRAPHICS_LAYOUT_P_H
#define QGRAPHICS_LAYOUT_P_H

#include <qglobal.h>

#if ! defined(QT_NO_GRAPHICSVIEW)

#include <qgraphicslayout.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qwidget.h>

#include <qgraphics_layoutitem_p.h>

class QGraphicsLayoutItem;
class QGraphicsWidget;

class QLayoutStyleInfo
{
 public:
   QLayoutStyleInfo()
   {
      invalidate();
   }

   QLayoutStyleInfo(QStyle *style, QWidget *widget)
      : m_valid(true), m_style(style), m_widget(widget)
   {
      Q_ASSERT(style);

      if (widget) {
         m_styleOption.initFrom(widget);
      }

      m_defaultSpacing[0] = style->pixelMetric(QStyle::PM_LayoutHorizontalSpacing);
      m_defaultSpacing[1] = style->pixelMetric(QStyle::PM_LayoutVerticalSpacing);
   }

   void invalidate() {
      m_valid  = false;
      m_style  = nullptr;
      m_widget = nullptr;
   }

   QStyle *style() const {
      return m_style;
   }

   QWidget *widget() const {
      return m_widget;
   }

   bool operator==(const QLayoutStyleInfo &other) const {
      return m_style == other.m_style && m_widget == other.m_widget;
   }

   bool operator!=(const QLayoutStyleInfo &other) const {
      return !(*this == other);
   }

   void setDefaultSpacing(Qt::Orientation o, qreal spacing) {
      if (spacing >= 0) {
         m_defaultSpacing[o - 1] = spacing;
      }
   }

   qreal defaultSpacing(Qt::Orientation o) const {
      return m_defaultSpacing[o - 1];
   }

   qreal perItemSpacing(QSizePolicy::ControlType control1, QSizePolicy::ControlType control2,
         Qt::Orientation orientation) const {
      Q_ASSERT(style());
      return style()->layoutSpacing(control1, control2, orientation, &m_styleOption, widget());
   }

 private:
   bool m_valid;
   QStyle *m_style;
   QWidget *m_widget;
   QStyleOption m_styleOption;
   qreal m_defaultSpacing[2];
};

class QGraphicsLayoutPrivate : public QGraphicsLayoutItemPrivate
{
   Q_DECLARE_PUBLIC(QGraphicsLayout)

 public:
   QGraphicsLayoutPrivate()
      : QGraphicsLayoutItemPrivate(nullptr, true), left(-1.0), top(-1.0), right(-1.0), bottom(-1.0), activated(true)
   { }

   void reparentChildItems(QGraphicsItem *newParent);
   void getMargin(qreal *result, qreal userMargin, QStyle::PixelMetric pm) const;
   Qt::LayoutDirection visualDirection() const;

   void addChildLayoutItem(QGraphicsLayoutItem *item);
   void activateRecursive(QGraphicsLayoutItem *item);

   qreal left, top, right, bottom;
   bool activated;
};

#endif //QT_NO_GRAPHICSVIEW

#endif
