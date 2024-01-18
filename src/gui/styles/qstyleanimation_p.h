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

#ifndef QSTYLEANIMATION_P_H
#define QSTYLEANIMATION_P_H

#include <qabstractanimation.h>
#include <qdatetime.h>
#include <qimage.h>

#ifndef QT_NO_ANIMATION

class QStyleAnimation : public QAbstractAnimation
{
   GUI_CS_OBJECT(QStyleAnimation)

 public:
   QStyleAnimation(QObject *target);
   virtual ~QStyleAnimation();

   QObject *target() const;

   int duration() const override;
   void setDuration(int duration);

   int delay() const;
   void setDelay(int delay);

   QTime startTime() const;
   void setStartTime(const QTime &time);

   enum FrameRate {
      DefaultFps,
      SixtyFps,
      ThirtyFps,
      TwentyFps
   };

   FrameRate frameRate() const;
   void setFrameRate(FrameRate fps);

   void updateTarget();

   GUI_CS_SLOT_1(Public, void start())
   GUI_CS_SLOT_2(start)

 protected:
   virtual bool isUpdateNeeded() const;
   void updateCurrentTime(int time) override;

 private:
   int _delay;
   int _duration;
   QTime _startTime;
   FrameRate _fps;
   int _skip;
};

class QProgressStyleAnimation : public QStyleAnimation
{
   GUI_CS_OBJECT(QProgressStyleAnimation)

 public:
   QProgressStyleAnimation(int speed, QObject *target);

   int animationStep() const;
   int progressStep(int width) const;

   int speed() const;
   void setSpeed(int speed);

 protected:
   bool isUpdateNeeded() const override;

 private:
   int _speed;
   mutable int _step;
};

class QNumberStyleAnimation : public QStyleAnimation
{
   GUI_CS_OBJECT(QNumberStyleAnimation)

 public:
   QNumberStyleAnimation(QObject *target);

   qreal startValue() const;
   void setStartValue(qreal value);

   qreal endValue() const;
   void setEndValue(qreal value);

   qreal currentValue() const;

 protected:
   bool isUpdateNeeded() const override;

 private:
   qreal _start;
   qreal _end;
   mutable qreal _prev;
};

class QBlendStyleAnimation : public QStyleAnimation
{
   GUI_CS_OBJECT(QBlendStyleAnimation)

 public:
   enum Type { Transition, Pulse };

   QBlendStyleAnimation(Type type, QObject *target);

   QImage startImage() const;
   void setStartImage(const QImage &image);

   QImage endImage() const;
   void setEndImage(const QImage &image);

   QImage currentImage() const;

 protected:
   void updateCurrentTime(int time) override;

 private:
   Type _type;
   QImage _start;
   QImage _end;
   QImage _current;
};

class QScrollbarStyleAnimation : public QNumberStyleAnimation
{
   GUI_CS_OBJECT(QScrollbarStyleAnimation)

 public:
   enum Mode { Activating, Deactivating };

   QScrollbarStyleAnimation(Mode mode, QObject *target);

   Mode mode() const;

   bool wasActive() const;
   void setActive(bool active);

 private:
   GUI_CS_SLOT_1(Private, void updateCurrentTime(int time) override)
   GUI_CS_SLOT_2(updateCurrentTime)

   Mode _mode;
   bool _active;
};

#endif // QT_NO_ANIMATION

#endif // QSTYLEANIMATION_P_H
