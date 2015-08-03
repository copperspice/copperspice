/***********************************************************************
*
* Copyright (c) 2012-2015 Barbara Geller
* Copyright (c) 2012-2015 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QSLIDER_H
#define QSLIDER_H

#include <QtGui/qabstractslider.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_SLIDER

class QSliderPrivate;
class QStyleOptionSlider;

class Q_GUI_EXPORT QSlider : public QAbstractSlider
{
   GUI_CS_OBJECT(QSlider)

   GUI_CS_ENUM(TickPosition)

   GUI_CS_PROPERTY_READ(tickPosition, tickPosition)
   GUI_CS_PROPERTY_WRITE(tickPosition, setTickPosition)
   GUI_CS_PROPERTY_READ(tickInterval, tickInterval)
   GUI_CS_PROPERTY_WRITE(tickInterval, setTickInterval)

 public:
   enum TickPosition {
      NoTicks = 0,
      TicksAbove = 1,
      TicksLeft = TicksAbove,
      TicksBelow = 2,
      TicksRight = TicksBelow,
      TicksBothSides = 3
   };

   explicit QSlider(QWidget *parent = 0);
   explicit QSlider(Qt::Orientation orientation, QWidget *parent = 0);

   ~QSlider();

   QSize sizeHint() const;
   QSize minimumSizeHint() const;

   void setTickPosition(TickPosition position);
   TickPosition tickPosition() const;

   void setTickInterval(int ti);
   int tickInterval() const;

   bool event(QEvent *event);

 protected:
   void paintEvent(QPaintEvent *ev);
   void mousePressEvent(QMouseEvent *ev);
   void mouseReleaseEvent(QMouseEvent *ev);
   void mouseMoveEvent(QMouseEvent *ev);
   void initStyleOption(QStyleOptionSlider *option) const;

 private:
   friend Q_GUI_EXPORT QStyleOptionSlider qt_qsliderStyleOption(QSlider *slider);

   Q_DISABLE_COPY(QSlider)
   Q_DECLARE_PRIVATE(QSlider)
};

#endif // QT_NO_SLIDER

QT_END_NAMESPACE

#endif // QSLIDER_H
