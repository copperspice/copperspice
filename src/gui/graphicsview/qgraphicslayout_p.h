/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#ifndef QGRAPHICSLAYOUT_P_H
#define QGRAPHICSLAYOUT_P_H

#include <qglobal.h>

#if !defined(QT_NO_GRAPHICSVIEW)

#include <qgraphicslayout.h>
#include <qgraphicslayoutitem_p.h>
#include <QtGui/qstyle.h>
#include <QtGui/qwidget.h>
#include <QtGui/qstyleoption.h>

class QGraphicsLayoutItem;
class QGraphicsWidget;

#ifdef QT_DEBUG
inline bool qt_graphicsLayoutDebug()
{
   static int checked_env = -1;

   if (checked_env == -1) {
      checked_env = qgetenv("QT_GRAPHICSLAYOUT_DEBUG").toInt() != 0;
   }

   return checked_env;
}
#endif


class QLayoutStyleInfo
{
 public:
   inline QLayoutStyleInfo() {
      invalidate();
   }

   inline QLayoutStyleInfo(QStyle *style, QWidget *widget)
      : m_valid(true), m_style(style), m_widget(widget) {
      Q_ASSERT(style);
      if (widget) { //###
         m_styleOption.initFrom(widget);
      }
      m_defaultSpacing[0] = style->pixelMetric(QStyle::PM_LayoutHorizontalSpacing);
      m_defaultSpacing[1] = style->pixelMetric(QStyle::PM_LayoutVerticalSpacing);
   }

   inline void invalidate() {
      m_valid = false;
      m_style = 0;
      m_widget = 0;
   }

   inline QStyle *style() const {
      return m_style;
   }

   inline QWidget *widget() const {
      return m_widget;
   }

   inline bool operator==(const QLayoutStyleInfo &other) const {
      return m_style == other.m_style && m_widget == other.m_widget;
   }

   inline bool operator!=(const QLayoutStyleInfo &other) const {
      return !(*this == other);
   }

   inline void setDefaultSpacing(Qt::Orientation o, qreal spacing) {
      if (spacing >= 0) {
         m_defaultSpacing[o - 1] = spacing;
      }
   }

   inline qreal defaultSpacing(Qt::Orientation o) const {
      return m_defaultSpacing[o - 1];
   }

   inline qreal perItemSpacing(QSizePolicy::ControlType control1, QSizePolicy::ControlType control2, Qt::Orientation orientation) const {
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
   QGraphicsLayoutPrivate() : QGraphicsLayoutItemPrivate(0, true), left(-1.0), top(-1.0), right(-1.0), bottom(-1.0), activated(true)
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
