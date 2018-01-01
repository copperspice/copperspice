/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QABSTRACTSLIDER_H
#define QABSTRACTSLIDER_H

#include <QtGui/qwidget.h>

QT_BEGIN_NAMESPACE

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
   explicit QAbstractSlider(QWidget *parent = nullptr);
   ~QAbstractSlider();

   Qt::Orientation orientation() const;

   void setMinimum(int);
   int minimum() const;

   void setMaximum(int);
   int maximum() const;

   void setRange(int min, int max);

   void setSingleStep(int);
   int singleStep() const;

   void setPageStep(int);
   int pageStep() const;

   void setTracking(bool enable);
   bool hasTracking() const;

   void setSliderDown(bool);
   bool isSliderDown() const;

   void setSliderPosition(int);
   int sliderPosition() const;

   void setInvertedAppearance(bool);
   bool invertedAppearance() const;

   void setInvertedControls(bool);
   bool invertedControls() const;

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

   int value() const;

   void triggerAction(SliderAction action);

   GUI_CS_SLOT_1(Public, void setValue(int un_named_arg1))
   GUI_CS_SLOT_2(setValue)
   GUI_CS_SLOT_1(Public, void setOrientation(Qt::Orientation un_named_arg1))
   GUI_CS_SLOT_2(setOrientation)

   GUI_CS_SIGNAL_1(Public, void valueChanged(int value))
   GUI_CS_SIGNAL_2(valueChanged, value)

   GUI_CS_SIGNAL_1(Public, void sliderPressed())
   GUI_CS_SIGNAL_2(sliderPressed)
   GUI_CS_SIGNAL_1(Public, void sliderMoved(int position))
   GUI_CS_SIGNAL_2(sliderMoved, position)
   GUI_CS_SIGNAL_1(Public, void sliderReleased())
   GUI_CS_SIGNAL_2(sliderReleased)

   GUI_CS_SIGNAL_1(Public, void rangeChanged(int min, int max))
   GUI_CS_SIGNAL_2(rangeChanged, min, max)

   GUI_CS_SIGNAL_1(Public, void actionTriggered(int action))
   GUI_CS_SIGNAL_2(actionTriggered, action)

 protected:
   bool event(QEvent *e) override;

   void setRepeatAction(SliderAction action, int thresholdTime = 500, int repeatTime = 50);
   SliderAction repeatAction() const;

   enum SliderChange {
      SliderRangeChange,
      SliderOrientationChange,
      SliderStepsChange,
      SliderValueChange
   };
   virtual void sliderChange(SliderChange change);

   void keyPressEvent(QKeyEvent *ev) override;
   void timerEvent(QTimerEvent *) override;

#ifndef QT_NO_WHEELEVENT
   void wheelEvent(QWheelEvent *e) override;
#endif

   void changeEvent(QEvent *e) override;

   QAbstractSlider(QAbstractSliderPrivate &dd, QWidget *parent = nullptr);

 private:
   Q_DISABLE_COPY(QAbstractSlider)
   Q_DECLARE_PRIVATE(QAbstractSlider)
};

QT_END_NAMESPACE

#endif // QABSTRACTSLIDER_H
