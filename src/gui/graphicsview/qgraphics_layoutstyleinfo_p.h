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

#ifndef QGRAPHICS_LAYOUTSTYLEINFO_P_H
#define QGRAPHICS_LAYOUTSTYLEINFO_P_H

#include <qabstractlayoutstyleinfo_p.h>
#include <qstyleoption.h>

class QStyle;
class QWidget;
class QGraphicsLayoutPrivate;

class QGraphicsLayoutStyleInfo : public QAbstractLayoutStyleInfo
{
 public:
   QGraphicsLayoutStyleInfo(const QGraphicsLayoutPrivate *layout);
   ~QGraphicsLayoutStyleInfo();

   qreal combinedLayoutSpacing(QLayoutPolicy::ControlTypes controls1,
      QLayoutPolicy::ControlTypes controls2, Qt::Orientation orientation) const override;

   qreal perItemSpacing(QLayoutPolicy::ControlType control1,
      QLayoutPolicy::ControlType control2, Qt::Orientation orientation) const override;

   qreal spacing(Qt::Orientation orientation) const override;

   qreal windowMargin(Qt::Orientation orientation) const override;

   void invalidate() override {
      m_style = nullptr;
      QAbstractLayoutStyleInfo::invalidate();
   }

   QWidget *widget() const;
   QStyle *style() const;

 private:
   const QGraphicsLayoutPrivate *m_layout;
   mutable QStyle *m_style;
   QStyleOption m_styleOption;
   QWidget *m_widget;
};

#endif
