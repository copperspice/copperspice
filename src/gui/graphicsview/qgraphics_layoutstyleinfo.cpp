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

#include <qgraphics_layoutstyleinfo_p.h>

#ifndef QT_NO_GRAPHICSVIEW
#include <qgraphicswidget.h>
#include <qstyle.h>
#include <qwidget.h>
#include <qapplication.h>

#include <qgraphics_layout_p.h>

QGraphicsLayoutStyleInfo::QGraphicsLayoutStyleInfo(const QGraphicsLayoutPrivate *layout)
   : m_layout(layout), m_style(nullptr)
{
   m_widget = new QWidget; // pixelMetric might need a widget ptr

   if (m_widget) {
      m_styleOption.initFrom(m_widget);
   }

   m_isWindow = m_styleOption.state & QStyle::State_Window;
}

QGraphicsLayoutStyleInfo::~QGraphicsLayoutStyleInfo()
{
   delete m_widget;
}

qreal QGraphicsLayoutStyleInfo::combinedLayoutSpacing(QLayoutPolicy::ControlTypes controls1,
   QLayoutPolicy::ControlTypes controls2,
   Qt::Orientation orientation) const
{
   Q_ASSERT(style());
   return style()->combinedLayoutSpacing(QSizePolicy::ControlTypes(int(controls1)), QSizePolicy::ControlTypes(int(controls2)),
         orientation, const_cast<QStyleOption *>(&m_styleOption), widget());
}

qreal QGraphicsLayoutStyleInfo::perItemSpacing(QLayoutPolicy::ControlType control1,
   QLayoutPolicy::ControlType control2,
   Qt::Orientation orientation) const
{
   Q_ASSERT(style());
   return style()->layoutSpacing(QSizePolicy::ControlType(control1), QSizePolicy::ControlType(control2),
         orientation, const_cast<QStyleOption *>(&m_styleOption), widget());
}

qreal QGraphicsLayoutStyleInfo::spacing(Qt::Orientation orientation) const
{
   Q_ASSERT(style());
   return style()->pixelMetric(orientation == Qt::Horizontal ? QStyle::PM_LayoutHorizontalSpacing : QStyle::PM_LayoutVerticalSpacing);
}

qreal QGraphicsLayoutStyleInfo::windowMargin(Qt::Orientation orientation) const
{
   return style()->pixelMetric(orientation == Qt::Vertical
         ? QStyle::PM_LayoutBottomMargin
         : QStyle::PM_LayoutRightMargin,
         const_cast<QStyleOption *>(&m_styleOption), widget());
}

QWidget *QGraphicsLayoutStyleInfo::widget() const
{
   return m_widget;
}

QStyle *QGraphicsLayoutStyleInfo::style() const
{
   if (!m_style) {
      Q_ASSERT(m_layout);
      QGraphicsItem *item = m_layout->parentItem();
      m_style = (item && item->isWidget()) ? static_cast<QGraphicsWidget *>(item)->style() : QApplication::style();
   }
   return m_style;
}

#endif // QT_NO_GRAPHICSVIEW
