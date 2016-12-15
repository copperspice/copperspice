/***********************************************************************
*
* Copyright (c) 2012-2016 Barbara Geller
* Copyright (c) 2012-2016 Ansel Sermersheim
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

#ifndef QDIAL_H
#define QDIAL_H

#include <QtGui/qabstractslider.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_DIAL

class QDialPrivate;
class QStyleOptionSlider;

class Q_GUI_EXPORT QDial: public QAbstractSlider
{
   GUI_CS_OBJECT(QDial)

   GUI_CS_PROPERTY_READ(wrapping, wrapping)
   GUI_CS_PROPERTY_WRITE(wrapping, setWrapping)
   GUI_CS_PROPERTY_READ(notchSize, notchSize)
   GUI_CS_PROPERTY_READ(notchTarget, notchTarget)
   GUI_CS_PROPERTY_WRITE(notchTarget, setNotchTarget)
   GUI_CS_PROPERTY_READ(notchesVisible, notchesVisible)
   GUI_CS_PROPERTY_WRITE(notchesVisible, setNotchesVisible)

 public:
   explicit QDial(QWidget *parent = 0);

   ~QDial();

   bool wrapping() const;
   int notchSize() const;

   void setNotchTarget(double target);
   qreal notchTarget() const;
   bool notchesVisible() const;

   QSize sizeHint() const override;
   QSize minimumSizeHint() const override;

   GUI_CS_SLOT_1(Public, void setNotchesVisible(bool visible))
   GUI_CS_SLOT_2(setNotchesVisible)
   GUI_CS_SLOT_1(Public, void setWrapping(bool on))
   GUI_CS_SLOT_2(setWrapping)

 protected:
   bool event(QEvent *e) override;
   void resizeEvent(QResizeEvent *re) override;
   void paintEvent(QPaintEvent *pe) override;

   void mousePressEvent(QMouseEvent *me) override;
   void mouseReleaseEvent(QMouseEvent *me) override;
   void mouseMoveEvent(QMouseEvent *me) override;

   void sliderChange(SliderChange change) override;
   void initStyleOption(QStyleOptionSlider *option) const;

 private:
   Q_DECLARE_PRIVATE(QDial)
   Q_DISABLE_COPY(QDial)
};

#endif  // QT_NO_DIAL

QT_END_NAMESPACE

#endif // QDIAL_H
