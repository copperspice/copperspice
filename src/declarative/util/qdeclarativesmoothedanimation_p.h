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

#ifndef QDECLARATIVESMOOTHEDANIMATION_P_H
#define QDECLARATIVESMOOTHEDANIMATION_P_H

#include <qdeclarative.h>
#include <qdeclarativeanimation_p.h>
#include <QtCore/qobject.h>

QT_BEGIN_NAMESPACE

class QDeclarativeProperty;
class QDeclarativeSmoothedAnimationPrivate;

class QDeclarativeSmoothedAnimation : public QDeclarativeNumberAnimation
{
   CS_OBJECT(QDeclarativeSmoothedAnimation)

   Q_DECLARE_PRIVATE(QDeclarativeSmoothedAnimation)
   CS_ENUM(ReversingMode)

   CS_PROPERTY_READ(velocity, velocity)
   CS_PROPERTY_WRITE(velocity, setVelocity)
   CS_PROPERTY_NOTIFY(velocity, velocityChanged)
   CS_PROPERTY_READ(reversingMode, reversingMode)
   CS_PROPERTY_WRITE(reversingMode, setReversingMode)
   CS_PROPERTY_NOTIFY(reversingMode, reversingModeChanged)
   CS_PROPERTY_READ(maximumEasingTime, maximumEasingTime)
   CS_PROPERTY_WRITE(maximumEasingTime, setMaximumEasingTime)
   CS_PROPERTY_NOTIFY(maximumEasingTime, maximumEasingTimeChanged)

 public:
   enum ReversingMode { Eased, Immediate, Sync };

   QDeclarativeSmoothedAnimation(QObject *parent = 0);
   ~QDeclarativeSmoothedAnimation();

   ReversingMode reversingMode() const;
   void setReversingMode(ReversingMode);

   virtual int duration() const;
   virtual void setDuration(int);

   qreal velocity() const;
   void setVelocity(qreal);

   int maximumEasingTime() const;
   void setMaximumEasingTime(int);

   virtual void transition(QDeclarativeStateActions &actions, QDeclarativeProperties &modified,
                           TransitionDirection direction);
   QAbstractAnimation *qtAnimation();

   CS_SIGNAL_1(Public, void velocityChanged())
   CS_SIGNAL_2(velocityChanged)
   CS_SIGNAL_1(Public, void reversingModeChanged())
   CS_SIGNAL_2(reversingModeChanged)
   CS_SIGNAL_1(Public, void maximumEasingTimeChanged())
   CS_SIGNAL_2(maximumEasingTimeChanged)
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeSmoothedAnimation)

#endif // QDECLARATIVESMOOTHEDANIMATION_H
