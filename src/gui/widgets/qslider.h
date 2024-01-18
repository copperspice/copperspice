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

#ifndef QSLIDER_H
#define QSLIDER_H

#include <qabstractslider.h>

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
   GUI_CS_REGISTER_ENUM(
      enum TickPosition {
         NoTicks = 0,
         TicksAbove = 1,
         TicksLeft = TicksAbove,
         TicksBelow = 2,
         TicksRight = TicksBelow,
         TicksBothSides = 3
      };
   )

   explicit QSlider(QWidget *parent = nullptr);
   explicit QSlider(Qt::Orientation orientation, QWidget *parent = nullptr);

   QSlider(const QSlider &) = delete;
   QSlider &operator=(const QSlider &) = delete;

   ~QSlider();

   QSize sizeHint() const override;
   QSize minimumSizeHint() const override;

   void setTickPosition(TickPosition position);
   TickPosition tickPosition() const;

   void setTickInterval(int ti);
   int tickInterval() const;

   bool event(QEvent *event) override;

 protected:
   void paintEvent(QPaintEvent *event) override;
   void mousePressEvent(QMouseEvent *event) override;
   void mouseReleaseEvent(QMouseEvent *event) override;
   void mouseMoveEvent(QMouseEvent *event) override;
   void initStyleOption(QStyleOptionSlider *option) const;

 private:
   Q_DECLARE_PRIVATE(QSlider)

   friend Q_GUI_EXPORT QStyleOptionSlider qt_qsliderStyleOption(QSlider *slider);
};

#endif // QT_NO_SLIDER

#endif
