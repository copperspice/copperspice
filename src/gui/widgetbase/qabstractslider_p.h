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

#ifndef QABSTRACTSLIDER_P_H
#define QABSTRACTSLIDER_P_H

#include <qbasictimer.h>
#include <qelapsedtimer.h>
#include <qstyle.h>

#include <qwidget_p.h>

class QAbstractSliderPrivate : public QWidgetPrivate
{
   Q_DECLARE_PUBLIC(QAbstractSlider)

 public:
   QAbstractSliderPrivate();
   ~QAbstractSliderPrivate();

   void setSteps(int single, int page);

   int minimum;
   int maximum;
   int pageStep;
   int value;
   int position;
   int pressValue;
   int singleStep;

   float offset_accumulated;
   uint tracking : 1;
   uint blocktracking : 1;
   uint pressed : 1;
   uint invertedAppearance : 1;
   uint invertedControls : 1;
   Qt::Orientation orientation;

   QBasicTimer repeatActionTimer;
   int repeatActionTime;
   QAbstractSlider::SliderAction repeatAction;

#ifdef QT_KEYPAD_NAVIGATION
   int origValue;
   bool isAutoRepeating;

   // When auto repeating, multiply singleStep with this value to get our effective step
   qreal repeatMultiplier;


   // time of when the first auto repeating key press event occurs
   QElapsedTimer firstRepeat;

#endif

   int effectiveSingleStep() const {

#ifdef QT_KEYPAD_NAVIGATION
      return singleStep * repeatMultiplier;
#else
      return singleStep;
#endif

   }

   virtual int bound(int val) const {
      return qMax(minimum, qMin(maximum, val));
   }

   int overflowSafeAdd(int add) const {
      int newValue = value + add;

      if (add > 0 && newValue < value) {
         newValue = maximum;
      } else if (add < 0 && newValue > value) {
         newValue = minimum;
      }

      return newValue;
   }

   void setAdjustedSliderPosition(int position) {
      Q_Q(QAbstractSlider);

      if (q->style()->styleHint(QStyle::SH_Slider_StopMouseOverSlider, nullptr, q)) {
         if ((position > pressValue - 2 * pageStep) && (position < pressValue + 2 * pageStep)) {
            repeatAction = QAbstractSlider::SliderNoAction;
            q->setSliderPosition(pressValue);
            return;
         }
      }

      q->triggerAction(repeatAction);
   }

   bool scrollByDelta(Qt::Orientation orientation, Qt::KeyboardModifiers modifiers, int delta);
};

#endif // QABSTRACTSLIDER_P_H
