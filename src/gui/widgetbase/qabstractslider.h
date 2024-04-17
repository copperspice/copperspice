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

#ifndef QABSTRACTSLIDER_H
#define QABSTRACTSLIDER_H

#include <qwidget.h>

class QAbstractSliderPrivate;

class Q_GUI_EXPORT QAbstractSlider : public QWidget
{
   GUI_CS_OBJECT(QAbstractSlider)

   GUI_CS_PROPERTY_READ(minimum, minimum)
   GUI_CS_PROPERTY_WRITE(minimum, setMinimum)
   GUI_CS_PROPERTY_READ(maximum, maximum)
   GUI_CS_PROPERTY_WRITE(maximum, setMaximum)
   GUI_CS_PROPERTY_READ(singleStep, singleStep)
   GUI_CS_PROPERTY_WRITE(singleStep, setSingleStep)
   GUI_CS_PROPERTY_READ(pageStep, pageStep)
   GUI_CS_PROPERTY_WRITE(pageStep, setPageStep)
   GUI_CS_PROPERTY_READ(value, value)
   GUI_CS_PROPERTY_WRITE(value, setValue)
   GUI_CS_PROPERTY_NOTIFY(value, valueChanged)
   GUI_CS_PROPERTY_USER(value, true)
   GUI_CS_PROPERTY_READ(sliderPosition, sliderPosition)
   GUI_CS_PROPERTY_WRITE(sliderPosition, setSliderPosition)
   GUI_CS_PROPERTY_NOTIFY(sliderPosition, sliderMoved)
   GUI_CS_PROPERTY_READ(tracking, hasTracking)
   GUI_CS_PROPERTY_WRITE(tracking, setTracking)
   GUI_CS_PROPERTY_READ(orientation, orientation)
   GUI_CS_PROPERTY_WRITE(orientation, setOrientation)
   GUI_CS_PROPERTY_READ(invertedAppearance, invertedAppearance)
   GUI_CS_PROPERTY_WRITE(invertedAppearance, setInvertedAppearance)
   GUI_CS_PROPERTY_READ(invertedControls, invertedControls)
   GUI_CS_PROPERTY_WRITE(invertedControls, setInvertedControls)
   GUI_CS_PROPERTY_READ(sliderDown, isSliderDown)
   GUI_CS_PROPERTY_WRITE(sliderDown, setSliderDown)
   GUI_CS_PROPERTY_DESIGNABLE(sliderDown, false)

 public:
   enum SliderAction {
      SliderNoAction,
      SliderSingleStepAdd,
      SliderSingleStepSub,
      SliderPageStepAdd,
      SliderPageStepSub,
      SliderToMinimum,
      SliderToMaximum,
      SliderMove
   };

   explicit QAbstractSlider(QWidget *parent = nullptr);

   QAbstractSlider(const QAbstractSlider &) = delete;
   QAbstractSlider &operator=(const QAbstractSlider &) = delete;

   ~QAbstractSlider();

   Qt::Orientation orientation() const;

   void setMinimum(int value);
   int minimum() const;

   void setMaximum(int value);
   int maximum() const;

   void setSingleStep(int value);
   int singleStep() const;

   void setPageStep(int value);
   int pageStep() const;

   void setTracking(bool enable);
   bool hasTracking() const;

   void setSliderDown(bool enable);
   bool isSliderDown() const;

   void setSliderPosition(int value);
   int sliderPosition() const;

   void setInvertedAppearance(bool enable);
   bool invertedAppearance() const;

   void setInvertedControls(bool enable);
   bool invertedControls() const;

   int value() const;

   void triggerAction(SliderAction action);

   GUI_CS_SLOT_1(Public, void setValue(int value))
   GUI_CS_SLOT_2(setValue)

   GUI_CS_SLOT_1(Public, void setOrientation(Qt::Orientation orientation))
   GUI_CS_SLOT_2(setOrientation)

   GUI_CS_SLOT_1(Public, void setRange(int min, int max))
   GUI_CS_SLOT_2(setRange)

   GUI_CS_SIGNAL_1(Public, void valueChanged(int value))
   GUI_CS_SIGNAL_2(valueChanged, value)

   GUI_CS_SIGNAL_1(Public, void sliderPressed())
   GUI_CS_SIGNAL_2(sliderPressed)

   GUI_CS_SIGNAL_1(Public, void sliderMoved(int pos))
   GUI_CS_SIGNAL_2(sliderMoved, pos)

   GUI_CS_SIGNAL_1(Public, void sliderReleased())
   GUI_CS_SIGNAL_2(sliderReleased)

   GUI_CS_SIGNAL_1(Public, void rangeChanged(int min, int max))
   GUI_CS_SIGNAL_2(rangeChanged, min, max)

   GUI_CS_SIGNAL_1(Public, void actionTriggered(int action))
   GUI_CS_SIGNAL_2(actionTriggered, action)

 protected:
   enum SliderChange {
      SliderRangeChange,
      SliderOrientationChange,
      SliderStepsChange,
      SliderValueChange
   };

   bool event(QEvent *event) override;

   void setRepeatAction(SliderAction action, int thresholdTime = 500, int repeatTime = 50);
   SliderAction repeatAction() const;

   virtual void sliderChange(SliderChange change);

   void keyPressEvent(QKeyEvent *event) override;
   void timerEvent(QTimerEvent *event) override;

#ifndef QT_NO_WHEELEVENT
   void wheelEvent(QWheelEvent *event) override;
#endif

   void changeEvent(QEvent *event) override;

   QAbstractSlider(QAbstractSliderPrivate &dd, QWidget *parent = nullptr);

 private:
   Q_DECLARE_PRIVATE(QAbstractSlider)
};

#endif
