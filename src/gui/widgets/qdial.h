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

#ifndef QDIAL_H
#define QDIAL_H

#include <qabstractslider.h>

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
   explicit QDial(QWidget *parent = nullptr);

   QDial(const QDial &) = delete;
   QDial &operator=(const QDial &) = delete;

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

   GUI_CS_SLOT_1(Public, void setWrapping(bool enable))
   GUI_CS_SLOT_2(setWrapping)

 protected:
   bool event(QEvent *event) override;
   void resizeEvent(QResizeEvent *event) override;
   void paintEvent(QPaintEvent *event) override;

   void mousePressEvent(QMouseEvent *event) override;
   void mouseReleaseEvent(QMouseEvent *event) override;
   void mouseMoveEvent(QMouseEvent *event) override;

   void sliderChange(SliderChange change) override;
   void initStyleOption(QStyleOptionSlider *option) const;

 private:
   Q_DECLARE_PRIVATE(QDial)
};

#endif  // QT_NO_DIAL

#endif
